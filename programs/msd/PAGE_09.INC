procedure page_09;
  const
    weekday: array[0..6] of string[9] = ('Sunday', 'Monday', 'Tuesday',
        'Wednesday', 'Thursday', 'Friday', 'Saturday');

  var
    xbool: boolean;
    xbyte: byte;
    xchar: char;
    xstring1: string;
    xstring2: string;
    xword1: word;
    xword2: word;
    xword3: word;
    xword4: word;
    xword5: word;
    listseg, listofs: word;
    filecount, usedfiles: word;

  procedure showecho(a: word);
    var
      xbyte : byte;

    begin
    xbyte:=mem[DOScseg : a];
    case xbyte of
      $00 : writeln('off');
      $FF : writeln('on')
    else
      unknown('status', xbyte, 2)
    end
    end; {showecho}

  procedure showbufs(a : word);
    const
      bufsmax = 99;

    var
      i : 0..bufsmax + 1;
      xbool : boolean;
      xword1 : word;
      xword2 : word;
      xword3 : word;

    begin
    if osmajor < 4 then
      begin
      i:=0;
      xword1:=MemW[DOScseg : a];
      xword2:=MemW[DOScseg : a + 2];
      xbool:=false;
      repeat
        if i <= bufsmax then
          begin
          if xword1 < $FFFF then
            begin
            inc(i);
            xword3:=xword1;
            xword1:=memw[xword2 : xword3];
            xword2:=memw[xword2 : xword3 + 2]
            end
          else
            begin
            xbool:=true;
            writeln(i)
            end
          end
        else
          begin
          xbool:=true;
          dontknow
          end
      until xbool
      end
    else
      with regs do
        begin
        AX:=$5200;
        MsDos(regs);
        Write(MemW[ES:BX + $3F]:5);
        caption3('Read-ahead');
        Writeln(MemW[ES:BX + $41])
        end
    end; {showbufs}
    (*  BIX ms.dos/secrets #2  *)

  begin (* procedure page_09 *)
  listseg:=devseg;
  listofs:=devofs;
  window(1, 3, twidth div 2, tlength - 2);
  caption2('DOS version');
  showvers;
  caption2('OEM serial number');
  with regs do
    begin
    AX:=$3000;
    BX:=0;
    MsDos(regs);
    Writeln(hex(BH, 2))
    end;
  caption2('System date');
  getdate(xword1, xword2, xword3, xword4);
  if xword4 < 7 then
    Write(weekday[xword4])
  else
    write('(', hex(xword4, 4), ')');
  write(', ');
  xword5:=cbw(country[0], country[1]);
  xchar:=chr(country[11]);
  case xword5 of
    $0001: writeln(xword3, xchar, xword2, xchar, xword1);
    $0002: writeln(xword1, xchar, xword2, xchar, xword3)
  else
    writeln(xword2, xchar, xword3, xchar, xword1)
  end;
  caption2('System time');
  gettime(xword1, xword2, xword3, xword4);
  if country[17] and 1 = 0 then
    case xword1 of
      0: Write('12');
      1..12: zeropad(xword1);
      13..23: Write(xword1 - 12)
    end
  else
    zeropad(xword1);
  write(chr(country[13]));
  zeropad(xword2);
  write(chr(country[13]));
  zeropad(xword3);
  write(decimal);
  zeropad(xword4);
  if country[17] and 1 = 0 then
    if xword1 > 11 then
      Write(' pm')
    else
      Write(' am');
  writeln;
  caption2('Command load paragraph');
  writeln(hex(prefixseg, 4));
  getcbreak(xbool);
  offoron('Ctrl-C check', xbool);
  getverify(xbool);
  offoron('Disk verify', xbool);
  caption2('Switch prefix character');
  writeln(switchar);
  caption2('\DEV\ prefix for devices');
  with regs do begin
    AX:=$3702;
    MSDOS(regs);
    if DL = $00 then
      writeln('required')
    else
      writeln('optional')
  end;
  caption2('Reset boot');
  xword1:=memw[BIOSdseg : $72];
  case xword1 of
    $0000: Writeln('cold');
    $1234, $1200, $EDCB: Writeln('bypass memory test');
    $4321: Writeln('preserve memory');
    $5678: Writeln('system suspended');
    $9ABC{-25924}: Writeln('manufacturing test mode'); (*!$9ABC*)
    $ABCD{-21555}: Writeln('system POST loop mode') (*!$ABCD*)
  else
    unknown('flag', xword1, 4)
  end;
  caption2('Boot disk was');
  if osmajor >=4 then
    with regs do
      begin
      AX:=$3305;
      MsDos(regs);
      Writeln(Chr(DL+$40), ':')
      end
  else
    dontknow;
(*  Byte 12:12 p.178  *)
  with regs do begin
    caption2('DOS critical flag');
    AX:=$5D06;
    MSDOS(regs);
    segofs(DS, SI);
    writeln
  end;
  caption2('DOS busy flag    ');
  segofs(DOScseg, DOScofs);
  writeln;
  caption2('Printer echo');
  case osmajor of
    3 : case osminor div 10 of
      0 : dontknow;
      1..3 : showecho($02AC)
      else
        dontknow
    end;
      4 : showecho($02FE);
  else
      dontknow
  end;
(*  BIX ms.dos/secrets #501  *)
  caption2('PrtSc status');
  xbyte:=mem[BIOSdseg : $0100];
  case xbyte of
    $00 : writeln('ready');
    $01 : writeln('busy');
    $FF : writeln('error on last PrtSc')
    else
      unknown('status', xbyte, 2)
  end;
  caption2('Memory allocation');
  with regs do begin
    AX:=$5800;
    MSDOS(regs);
    case AL of
      0: Writeln('first fit');
      1: Writeln('best fit');
      2: Writeln('last fit')
    else
      dontknow
    end
  end;
  caption2('DOS buffers');
  case osmajor of
    3 : case osminor div 10 of
          0 : showbufs($013F);
          1..3 : showbufs($0038)
        else
          dontknow
        end;
    4 : showbufs(0)
  else
    dontknow
  end;
  caption2('File handle table ');
  xword1:=memw[prefixseg : $0036];
  xword2:=memw[prefixseg : $0034];
  segofs(xword1, xword2);
  Writeln;
  caption3('length');
  xword2:=MemW[listseg:listofs + 4];
  xword1:=MemW[listseg:listofs + 6];
  xbool:=false;
  filecount:=0;
  repeat
    xword4:=MemW[xword1:xword2];
    xword3:=MemW[xword1:xword2 + 2];
    filecount:=filecount + MemW[xword1:xword2 + 4];
    if xword4 = $FFFF then
      xbool:=true
    else
      begin
      xword1:=xword3;
      xword2:=xword4
      end
  until xbool;
  Write(filecount:3);
  caption3('used');
  usedfiles:=0;
  xword1:=MemW[PrefixSeg: $36];
  xword2:=MemW[PrefixSeg: $34];
  while Mem[xword1 : xword2] < $FF do begin
    inc(usedfiles);
    inc(xword2)
  end;
  writeln(usedfiles:3);
  caption2('File Control Blocks');
  Writeln;
  caption3('amount');
  if (osmajor = 4) or ((osmajor = 3) and (osminor > 0)) then
    begin
    xword3:=MemW[listseg:listofs + $1E];
    xword2:=MemW[listseg:listofs + $1A];
    xword1:=MemW[listseg:listofs + $1C]
    end
  else
    begin
    xword3:=MemW[listseg:listofs + $26];
    xword2:=MemW[listseg:listofs + $22];
    xword1:=MemW[listseg:listofs + $24]
    end;
  Write(MemW[xword1:xword2 + 4]:3);
  caption3('protected');
  Write(xword3:3);
  window(1 + twidth div 2, 3, twidth, tlength - 2);
  caption2('Stacks');
  if osmajor = 3 then
    dontknow
  else
    begin
    xword1:=MemW[listseg:listofs - 2];
    xword4:=0; {# of stacks}
    xword5:=0; {size of stacks}
    if (Mem[xword1:0] <> $4D) or (MemW[xword1:1] <> 8) then
      dontknow
    else
      begin
      xword2:=$10;
      xbool:=false;
      repeat
        xchar:=Chr(Mem[xword1:xword2]);
        if xchar = 'S' then
          begin
          xword3:=MemW[xword1:xword2 + 1];
          xword4:=Mem[xword3:2];
          xword5:=Mem[xword3:6]
          end;
        if (xchar = 'M') or (xchar = 'Z') then
          xbool:=true;
        xword2:=xword2 + (MemW[xword1:xword2 + 3] * $10) + $10;
      until xbool;
      Writeln;
      caption3('amount');
      Write(xword4:3);
      caption3('size each (bytes)');
      Writeln(xword5:3);
      end
  end;
  Writeln;
  TextColor(LightCyan);
  Write('------ International Information -----');
  Writeln;
  caption2('Global code page');
  with regs do begin
    AX:=$6601;
    MSDOS(regs);
    if AL = $01 then begin
      writeln;
      caption3('Active ');
      writeln(BX : 5);
      caption3('Default');
      writeln(DX : 5)
    end else
      writeln('N/A')
  end;
  caption2('Country code');
  writeln(ccode);
  caption2('Thousands separator character');
  writeln(chr(country[7]));
  caption2('Decimal separator character');
  writeln(decimal);
  caption2('Data-list separator character');
  writeln(chr(country[22]));
  caption2('Date format');
  xword1:=cbw(country[0], country[1]);
  xchar:=chr(country[11]);
  case xword1 of
    0: writeln('USA (mm', xchar, 'dd', xchar, 'yy)');
    1: writeln('Europe (dd', xchar, 'mm', xchar, 'yy)');
    2: writeln('Japan (yy', xchar, 'mm', xchar, 'dd)')
  else
    unknown('format', xword1, 4)
  end;
  caption3('Separator character');
  writeln(xchar);
  caption2('Time format');
  if (country[17] and $01) = $00 then
    write('12')
  else
    write('24');
  writeln('-hour');
  caption3('Separator character');
  writeln(chr(country[13]));
  caption2('Currency format');
  xstring1:='xxxx';
  insert(chr(country[7]), xstring1, 2);
  xstring1:=xstring1 + decimal;
  for i:=1 to country[16] do
    xstring1:=xstring1 + 'y';
  xstring2:='';
  i:=2;
  xchar:=chr(country[i]);
  while (i <= 6) and (xchar > #0) do begin
    xstring2:=xstring2 + xchar;
    inc(i);
    xchar:=chr(country[i])
  end;
  case country[15] and $03 of
    $00 : xstring1:=xstring2 + xstring1;
    $01 : xstring1:=xstring1 + xstring2;
    $02 : xstring1:=xstring2 + ' ' + xstring1;
    $03 : xstring1:=xstring1 + ' ' + xstring2;
    $04 : begin
      delete(xstring1, 6, 1);
      insert(xstring2, xstring1, 6)
    end
  end;
  writeln(xstring1);
  caption2('Case map call address');
  segofs(cbw(country[20], country[21]), cbw(country[18], country[19]));
  writeln
end;
