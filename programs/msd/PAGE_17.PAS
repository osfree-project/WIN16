unit page_17;

interface

uses crt, ifpglobl, ifpcomon;

procedure page17;

implementation

procedure page17;
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
    count, ComputedChecksum, CMOSChecksum: word;
    bad, pm, PS2, Phoenix: boolean;
    Floppy, Hd0, Hd1, Date, Month, Century, Year, Hour, Min, Sec: byte;
    TimeSep: char;
    xbyte: byte;
    s: string;
    Cylinders, Heads, Precomp, LZ, Sectors: word;

  function ReadCMOS(adr: byte): byte;
    var
      i: byte;

    begin
    inline($FA);
    Port[CMOSport]:=adr;
    for i:=1 to 10 do;
    ReadCMOS:=Port[CMOSport + 1];
    inline($FB)
    end; {ReadCMOS}

{
  procedure writeCMOS(adr, data: byte);
    var
      i: byte;

    begin
    inline($FA);
    Port[CMOSport]:=adr;
    for i:=1 to 10 do;
    Port[CMOSport + 1]:=data;
    inline($FB)
    end;
 }

  procedure GetHDValues(HDtype: byte;
            var Cylinders, Heads, PreComp, LZ, Sectors: word);
    var
      HDOffset: word;
    begin
    HDOffset:=$E401 + ((HDtype - 1) * 16);
    Cylinders:=MemW[$F000:HDOffset];
    Heads:=Mem[$F000:HDOffset + 2];
    PreComp:=MemW[$F000:HDOffset + 5];
    LZ:=MemW[$F000:HDOffset + $C];
    Sectors:=Mem[$F000:HDOffset + $E];
    end;

  procedure ShowHDValues(Cylinders, Heads, PreComp, LZ, Sectors: word);
    begin
    Caption3('Cyl');
    Write(Cylinders:4);
    Caption3('Hds');
    Write(Heads:2);
    Caption3('Sec');
    Write(Sectors:2);
    Caption3('LZ');
    Write(integer(LZ):4);
    Caption3('PreComp');
    Writeln(integer(PreComp):4);
    end;

  begin
  Caption2('CMOS');
  regs.AH:=$C0;
  Intr($15, regs);
  if NoCarry(regs) or (Mem[$FFFF:$E] < $FD) then
    begin
    PS2:=False;
    if Mem[regs.ES:regs.BX + 2] = $F8 then
      PS2:=true;
    Phoenix:=false;
    s:='';
    with regs do
      for xword:=BX + $D to BX + $F do
        s:=s + Chr(Mem[ES:xword]);
    if s = 'PTL' then
      Phoenix:=true;
    CMOSport:=$70;
    Writeln;
    Caption3('Power status');
    if ReadCMOS($D) and $80 = $80 then
      Writeln('OK')
    else
      Writeln('No power!');
    Caption3(' Diagnostics');
    xbyte:=ReadCMOS($E);
    if xbyte = 0 then
      Writeln('No problems detected')
    else
      begin
      if xbyte and $80 = $80 then
        Writeln('Clock lost power');
      if xbyte and $40 = $40 then
        Writeln('Incorrect checksum');
      if xbyte and $20 = $20 then
        Writeln('Bad equipment configuration');
      if xbyte and $10 = $10 then
        Writeln('Memory size error');
      if xbyte and 8 = 8 then
        Writeln('Disk drive initialization failure');
      if xbyte and 4 = 4 then
        Writeln('Invalid time');
      if xbyte and 2 = 2 then
        Writeln('Bad adaptor configuration (EISA)');
      if xbyte and 1 = 1 then
        Writeln('Timeout reading adaptor ID (EISA)');
      end;
    Writeln;
    Caption3('Date');
    Date:=ReadCMOS(7);
    if PS2 then
      Century:=unBCD(ReadCMOS($37))
    else
      Century:=unBCD(ReadCMOS($32));
    Year:=ReadCMOS(9);
    Month:=ReadCMOS(8);
    if ReadCMOS($B) and 4 = 0 then
      begin
      Date:=unBCD(Date);
      Year:=unBCD(Year);
      Month:=unBCD(Month);
      end;
{ Most BIOS's do not set the Day of Week byte. Commented out and left for info}
{    Write(DayName[ReadCMOS(6)], ', ');}
    case Country[0] of
      0, 3..255: Writeln(Monthname[Month], ' ', Date, ', ',
                         Century, AddZero(Year));
      1: Writeln(Date, ' ', Monthname[Month], ', ', Century, AddZero(Year));
      2: Writeln(Century, AddZero(Year), ', ', Monthname[Month], ' ', Date);
    end; {case}
    Caption3('Time');
    TimeSep:=Chr(Country[$0D]);
    Hour:=ReadCMOS(4);
    Min:=ReadCMOS(2);
    Sec:=ReadCMOS(0);
    if ReadCMOS($B) and 4 = 0 then
      begin
      Hour:=unBCD(Hour);
      Min:=unBCD(Min);
      Sec:=unBCD(Sec);
      end;
    if ReadCMOS($B) and 2 = 0 then
      if Hour > 12 then
        Hour:=(Hour - 128) + 11
      else
        Dec(Hour);
    if Country[$11] and 1 = 1 then
      Writeln(Hour, TimeSep, AddZero(Min), TimeSep, AddZero(Sec))
    else
      begin
      pm:=false;
      case Hour of
        0: Hour:=12;
        1..11: Hour:=Hour;
        12: pm:=true;
        13..23: begin
                pm:=true;
                Hour:=Hour - 12
                end;
      end; {case}
      Write(Hour, TimeSep, AddZero(Min), TimeSep, AddZero(Sec), ' ');
      if pm then
        Writeln('PM')
      else
        Writeln('AM');
      end;
    Writeln;
    Caption3('Video type ');
    Writeln(ScreenName[(ReadCMOS($14) shr 4) and 3]);
    Caption3('Coprocessor');
    YesOrNo((ReadCMOS($14) and 2) = 2);
    Writeln;
    Caption3('Floppy disk A');
    Floppy:=ReadCMOS($10);
    if (Floppy shr 4) < 6 then
      Writeln(FloppyName[Floppy shr 4])
    else
      Writeln('Unknown value -> ', Hex(Floppy shr 4, 2));
    Caption3('Floppy disk B');
    if (Floppy and $0F) < 6 then
      Writeln(FloppyName[Floppy and $0F])
    else
      Writeln('Unknown value -> ', Hex(Floppy and $0F, 2));
    Writeln;
    Caption3('Hard disk 0');
    if not PS2 then
      begin
      Hd0:=ReadCMOS($12) shr 4;
      Hd1:=ReadCMOS($12) and $0F;
      if Hd0 = $F then
        Hd0:=ReadCMOS($19);
      if Hd1 = $F then
        Hd1:=ReadCMOS($1A);
      end
    else
      begin
      Hd0:=ReadCMOS($11);
      Hd1:=ReadCMOS($12);
      end;
    if Hd0 = 0 then
      Writeln('None')
    else
      begin
      Write('Type ', Hd0);
      if Hd0 < 47 then
        begin
        GetHDValues(Hd0, Cylinders, Heads, PreComp, LZ, Sectors);
        ShowHDValues(Cylinders, Heads, PreComp, LZ, Sectors)
        end
      else
        if (Phoenix) and (Hd0 >= 48) then
          begin
          Cylinders:=word(ReadCMOS($21) shl 8) + ReadCMOS($20);
          Heads:=ReadCMOS($22);
          PreComp:=word(ReadCMOS($24) shl 8) + ReadCMOS($23);
          LZ:=word(ReadCMOS($26) shl 8) + ReadCMOS($25);
          Sectors:=ReadCMOS($27);
          ShowHDValues(Cylinders, Heads, PreComp, LZ, Sectors)
          end
        else
          Writeln;
      end;
    Caption3('Hard disk 1');
    if Hd1 = 0 then
      Writeln('None')
    else
      begin
      Write('Type ', Hd1);
      if Hd1 < 47 then
        begin
        GetHDValues(Hd1, Cylinders, Heads, PreComp, LZ, Sectors);
        ShowHDValues(Cylinders, Heads, PreComp, LZ, Sectors)
        end
      else
        if (Phoenix) and (Hd1 >= 48) then
          begin
          Cylinders:=word(ReadCMOS($36) shl 8) + ReadCMOS($35);
          Heads:=ReadCMOS($37);
          PreComp:=word(ReadCMOS($39) shl 8) + ReadCMOS($38);
          LZ:=word(ReadCMOS($3B) shl 8) + ReadCMOS($3A);
          Sectors:=ReadCMOS($3C);
          ShowHDValues(Cylinders, Heads, PreComp, LZ, Sectors)
          end
        else
          Writeln;
      end;
    Writeln;
    Caption3('Conventional RAM');
    Writeln(word(ReadCMOS($16) shl 8) + ReadCMOS($15):6, 'K');
    Caption3('    Extended RAM');
    Writeln(word(ReadCMOS($18) shl 8) + ReadCMOS($17):6, 'K');
    Writeln;
    Caption3('CMOS checksum');
    if not PS2 then
      begin
      ComputedChecksum:=0;
      for count:=$10 to $2D do
        Inc(ComputedChecksum, ReadCMOS(count));
      CMOSChecksum:=word(ReadCMOS($2E) shl 8) + ReadCMOS($2F);
      end
    else
      begin
      ComputedChecksum:=0;
      for count:=$10 to $31 do
        Inc(ComputedChecksum, ReadCMOS(count));
      CMOSChecksum:=word(ReadCMOS($33) shl 8) + ReadCMOS($32);
      end;
    if ComputedChecksum = CMOSChecksum then
      Writeln('OK')
    else
      Writeln('Error!  Says ', Hex(CMOSChecksum, 4), ' should be ',
               Hex(ComputedChecksum, 4));
    end
  else
    Writeln('No standard CMOS detected!!')
  end;
end.