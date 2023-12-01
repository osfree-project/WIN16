procedure page_17;
  const
    DayName: array[0..7] of string[9] = ('Sunday', 'Monday', 'Tuesday',
                                         'Wednesday', 'Thursday', 'Friday',
                                         'Saturday', 'Sunday');
    MonthName: array[0..12] of string[9] = ('???', 'January', 'February', 'March',
                                            'April', 'May', 'June', 'July',
                                            'August', 'September', 'October',
                                            'November', 'December');
    ScreenName: array[0..3] of string[10] = ('EGA/VGA', 'CGA 40col',
                                                 'CGA 80col', 'Monochrome');
    FloppyName: array[0..5] of string[11] = ('none', '5.25" 360K',
                                             '5.25" 1.2M', '3.5"  720K',
                                             '3.5"  1.44M', '3.5"  2.88M');

  var
    CMOSport: word;
    count, checksum1, checksum2: word;
    bad, pm: boolean;
    floppy, hd, hdc, hdd, date, month, century, year, hour, min, sec: byte;
    c: char;

  function readCMOS(adr: byte): byte;
    var
      i: byte;

    begin
    bad:=false;
    if CMOSport = $70 then
      begin
      inline($FA);
      Port[CMOSport]:=adr;
      readCMOS:=Port[CMOSport + 1];
      inline($FB)
      end
    else
      begin
      inline($FA);
      i:=Port[CMOSport + adr];
      if i <> Port[CMOSport + adr] then
        bad:=true;
      inline($FB);
      readCMOS:=i
      end
    end; {readCMOS}

  function addzero(b: byte): string;
    var
      c2: string[2];
    begin
    Str(b:0, c2);
    if b < 10 then
      c2:='0' + c2;
    addzero:=c2
    end; {addzero}

  function unBCD(b: byte): byte;
    begin
    unBCD:=(b and $0F) + ((b shr 4) * 10)
    end; {unBCD}

  begin
  caption2('CMOS');
  regs.AH:=$C0;
  Intr($15, regs);
  if nocarry or (Mem[$F000:$FFFE] = $FC) then
    begin
    CMOSport:=$70;
    Writeln;
    caption3('Date');
    date:=unBCD(readCMOS(7));
    century:=unBCD(readCMOS($32));
    year:=unBCD(readCMOS(9));
    month:=unBCD(readCMOS(8));
    Write(DayName[readCMOS(6)], ', ');
    case country[0] of
      0, 3..255: Writeln(Monthname[month], ' ', date, ', ', century, addzero(year));
      1: Writeln(date, ' ', Monthname[month], ', ', century, addzero(year));
      2: Writeln(century, addzero(year), ', ', Monthname[month], ' ', date);
    end; {case}
    caption3('Time');
    c:=Chr(country[$0D]);
    hour:=unBCD(readCMOS(4));
    min:=unBCD(readCMOS(2));
    sec:=unBCD(readCMOS(0));
    if country[$11] and 1 = 1 then
      Writeln(hour, c, addzero(min), c, addzero(sec))
    else
      begin
      pm:=false;
      case hour of
        0: hour:=12;
        1..11: hour:=hour;
        12: pm:=true;
        13..23: begin
                pm:=true;
                hour:=hour - 12
                end;
      end; {case}
      Write(hour, c, addzero(min), c, addzero(sec), ' ');
      if pm then
        Writeln('PM')
      else
        Writeln('AM');
      end;
    Writeln;
    caption3('Video type ');
    Writeln(ScreenName[(readCMOS($14) shr 4) and 3]);
    caption3('Coprocessor');
    yesorno((readCMOS($14) and 2) = 2);
    Writeln;
    caption3('Floppy disk A');
    floppy:=readCMOS($10);
    if (floppy shr 4) < 5 then
      Writeln(FloppyName[floppy shr 4])
    else
      Writeln('Unknown value -> ', hex(floppy shr 4, 2));
    caption3('Floppy disk B');
    if (floppy and $0F) < 5 then
      Writeln(FloppyName[floppy and $0F])
    else
      Writeln('Unknown value -> ', hex(floppy and $0F, 2));
    Writeln;
    caption3('Hard disk 0');
    hd:=readCMOS($12);
    hdc:=hd shr 4;
    hdd:=hd and $0F;
    if hdc = $F then
      hdc:=readCMOS($19);
    if hdd = $F then
      hdd:=readCMOS($1A);
    if hdc = 0 then
      Writeln('None')
    else
      Writeln('Type ', hdc);
    caption3('Hard disk 1');
    if hdd = 0 then
      Writeln('None')
    else
      Writeln('Type ', hdd);
    Writeln;
    caption3('Conventional RAM');
    Writeln((word(256) * readCMOS($16)) + readCMOS($15):5, 'K');
    caption3('    Extended RAM');
    Writeln((word(256) * readCMOS($18)) + readCMOS($17):5, 'K');
    Writeln;
    caption3('CMOS checksum');
    checksum1:=0;
    for count:=$10 to $2D do
      Inc(checksum1, readCMOS(count));
    checksum2:=(word(256) * readCMOS($2E)) + readCMOS($2F);
    if checksum1 = checksum2 then
      Writeln('OK')
    else
      Writeln('Error!  Says ', hex(checksum2, 4), ' should be ', hex(checksum1, 4));
    end
  else
    Writeln('No standard CMOS detected!!')
  end;
