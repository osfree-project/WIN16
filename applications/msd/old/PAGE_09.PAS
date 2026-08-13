unit page_09;

interface

uses crt, dos, ifpglobl, ifpcomon;

procedure page09;

implementation

procedure page09;
  const
    weekday: array[0..6] of string[9] = ('Sunday', 'Monday', 'Tuesday',
        'Wednesday', 'Thursday', 'Friday', 'Saturday');

  var
    foundit, xbool, wasone: boolean;
    xbyte: byte;
    xchar: char;
    xstring1: string;
    xstring2: string;
    xword1: word;
    xword2: word;
    xword3: word;
    xword4: word;
    xword5: word;
    xword6: word;
    xword7: word;
    xword8: word;
    filecount, usedfiles, tablesize: word;
    dt: DateTime;
    s: string;

  procedure showecho(a: word);
    var
      xbyte : byte;

    begin
    xbyte:=Mem[DOScseg : a];
    case xbyte of
      $00 : Writeln('off');
      $FF : Writeln('on')
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
    if (OSMajor < 4) or (OSMajor >= 10) then
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
            xword1:=MemW[xword2 : xword3];
            xword2:=MemW[xword2 : xword3 + 2]
            end
          else
            begin
            xbool:=true;
            Writeln(i)
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
        Caption3('Read-ahead');
        Writeln(MemW[ES:BX + $41]);
        if OSMajor = 4 then
          begin
          Caption3('in EMS');
          xword2:=MemW[ES:BX + $12];
          xword1:=MemW[ES:BX + $14];
          case Mem[xword1: xword2 + $C] of
            $00: Writeln('no');
            $FF: begin
                 Write('yes');
                 Caption3('handle');
                 Writeln(MemW[xword1: xword2 + $D])
                 end;
            $01: if Mem[xword1: xword2 + $18] = 0 then
                   Writeln('no')
                 else
                   begin
                   Write('yes');
                   Caption3('handle');
                   Writeln(MemW[xword1: xword2 + $18])
                   end;
          else
            dontknow
          end; {case}
          end
        end
    end; {showbufs}
    (*  BIX ms.dos/secrets #2  *)

  procedure ShowAttrib(s: string; value, mask: byte);
    begin
    if value and mask = mask then
      begin
      if wasone then
        Write('/');
      Write(s);
      wasone:=true
      end;
    end; {ShowAttrib}

  begin (* procedure page_09 *)
  Window(1, 3, twidth div 2, tlength - 2);
  Caption2('DOS version');
  with regs do
    begin
    AX:=$4452;
    Flags:=Flags and FCarry;
    MsDos(regs);
    if NoCarry(regs) then
      begin
      Write('DR-DOS ');
      case AX of
        $1063: Writeln('3', decimal, '41');
        $1065: Writeln('5', decimal, '00');
        $1067: Writeln('6', decimal, '00')
      else
        Writeln('? code ', Hex(AX, 4));
      end;
      end
    else
      begin
      if OSMajor >= 10 then
        begin
        Write('OS/2 ', OSMajor div 10, decimal, AddZero(OSMinor));
        Writeln(' DOS session');
        end
      else
        Write(OSMajor, decimal, addzero(OSMinor));
      if (OSMajor < 5) or (OSMajor = 10) then
        Writeln
      else
        begin
        Caption3('revision');
        AX:=$3306;
        MsDos(regs);
        Writeln(Chr((DL and 7) + Ord('A')));
        Caption3('in HMA');
        YesOrNo2(DH and $10 = $10);
        Caption3('in ROM');
        YesOrNo(DH and 8 = 8);
        end;
      end
    end;
  with regs do
    begin
    AX:=$3000;
    MsDos(regs);
    if (AL <> OSMajor) or (AH <> OSMinor) then
      begin
      Caption3('SETVER''d to');
      Write(AL, decimal);
      ZeroPad(AH);
      Writeln
      end
    end;
  Caption2('OEM serial number');
  with regs do
    begin
    AX:=$3000;
    BX:=0;
    MsDos(regs);
    Writeln(Hex(BH, 2))
    end;
  Caption2('System date');
  getdate(xword1, xword2, xword3, xword4);
  if xword4 < 7 then
    Write(weekday[xword4])
  else
    Write('(', Hex(xword4, 4), ')');
  Write(', ');
  xword5:=CBW(Country[0], Country[1]);
  xchar:=Chr(Country[11]);
  case xword5 of
    $0001: Writeln(xword3, xchar, xword2, xchar, xword1);
    $0002: Writeln(xword1, xchar, xword2, xchar, xword3)
  else
    Writeln(xword2, xchar, xword3, xchar, xword1)
  end;
  Caption2('System time');
  gettime(xword1, xword2, xword3, xword4);
  if Country[17] and 1 = 0 then
    case xword1 of
      0: Write('12');
      1..12: ZeroPad(xword1);
      13..23: Write(xword1 - 12)
    end
  else
    ZeroPad(xword1);
  Write(Chr(Country[13]));
  ZeroPad(xword2);
  Write(Chr(Country[13]));
  ZeroPad(xword3);
  Write(decimal);
  ZeroPad(xword4);
  if Country[17] and 1 = 0 then
    if xword1 > 11 then
      Write(' pm')
    else
      Write(' am');
  Writeln;
  Caption2('Command load paragraph');
  Writeln(Hex(PrefixSeg, 4));
  getcbreak(xbool);
  offoron('Ctrl-C check', xbool);
  Writeln;
  getverify(xbool);
  offoron('Disk verify', xbool);
  Writeln;
  Caption2('Switch prefix character');
  Writeln(switchar);
  Caption2('\DEV\ prefix for devices');
  with regs do begin
    AX:=$3702;
    MSDOS(regs);
    if DL = $00 then
      Writeln('required')
    else
      Writeln('optional')
  end;
  Caption2('Reset boot');
  xword1:=MemW[BIOSdseg : $72];
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
  Caption2('Boot disk was');
  if OSMajor >= 4 then
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
    Caption2('DOS critical flag');
    AX:=$5D06;
    MSDOS(regs);
    segofs(DS, SI);
    Writeln
  end;
  Caption2('DOS busy flag    ');
  segofs(DOScseg, DOScofs);
  Writeln;
  Caption2('Printer echo');
  case OSMajor of
    3 : case OSMinor div 10 of
      0 : dontknow;
      1..3 : showecho($02AC)
      else
        dontknow
    end;
      4,5,6 : showecho($02FE);
  else
      dontknow
  end;
(*  BIX ms.dos/secrets #501  *)
  Caption2('PrtSc status');
  xbyte:=Mem[BIOSdseg : $0100];
  case xbyte of
    $00 : Writeln('ready');
    $01 : Writeln('busy');
    $FF : Writeln('error on last PrtSc')
    else
      unknown('status', xbyte, 2)
  end;
  Caption2('Memory allocation');
  with regs do begin
    AX:=$5800;
    MSDOS(regs);
    case AL of
      0: Writeln('first fit');
      1: Writeln('best fit');
      2..$3F,$43..$7F,$83..$FF: Writeln('last fit');
      $40: Writeln('hi mem first fit');
      $41: Writeln('hi mem best fit');
      $42: Writeln('hi mem last fit');
      $80: Writeln('frst fit,hi then low');
      $81: Writeln('best fit,hi then low');
      $82: Writeln('last fit,hi then low');
    else
      dontknow
    end
  end;
  Caption2('DOS buffers');
  case OSMajor of
    3 : case OSMinor div 10 of
          0 : showbufs($013F);
          1..3 : showbufs($0038)
        else
          dontknow
        end;
    4,5,6 : showbufs(0)
  else
    dontknow
  end;
  Caption2('File handle table ');
  xword1:=MemW[PrefixSeg : $0036];
  xword2:=MemW[PrefixSeg : $0034];
  SegOfs(xword1, xword2);
  Writeln;
  Caption3('length');
  xword2:=MemW[DevSeg:DevOfs + 4];
  xword1:=MemW[DevSeg:DevOfs + 6];
  xbool:=false;
  filecount:=0;
  if (xword1 = $FFFF) and (xword2 = $FFFF) then
    filecount:=MemW[PrefixSeg: $32]
  else
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
  Caption3('used');
  usedfiles:=0;
  xword1:=MemW[PrefixSeg: $36];
  xword2:=MemW[PrefixSeg: $34];
  while Mem[xword1 : xword2] < $FF do begin
    inc(usedfiles);
    inc(xword2)
  end;
  Write(usedfiles:3);
  Window(1 + twidth div 2, 3, twidth, tlength - 2);
  Caption2('File Control Blocks');
  if OSMajor >= 10 then
    dontknow
  else
    begin
    Writeln;
    Caption3('amount');
    if (OSMajor >= 4) or ((OSMajor = 3) and (OSMinor > 0)) then
      begin
      xword3:=MemW[DevSeg:DevOfs + $1E];
      xword2:=MemW[DevSeg:DevOfs + $1A];
      xword1:=MemW[DevSeg:DevOfs + $1C]
      end
    else
      begin
      xword3:=MemW[DevSeg:DevOfs + $26];
      xword2:=MemW[DevSeg:DevOfs + $22];
      xword1:=MemW[DevSeg:DevOfs + $24]
      end;
    Write(MemW[xword1:xword2 + 4]:3);
    if (OSMajor >= 5) and (OSMajor < 10) then
      Writeln
    else
      begin
      Caption3('protected');
      Writeln(xword3:3);
      end;
    end;
  Caption2('Stacks');
  if (OSMajor = 3) or (OSMajor >= 10) then
    dontknow
  else
    begin
    xword1:=MemW[DevSeg:DevOfs - 2];
    xword4:=0; {# of stacks}
    xword5:=0; {size of stacks}
    if (Mem[xword1:0] <> $4D) or (MemW[xword1:1] <> 8) then
      dontknow
    else
      begin
      xword3:=xword1 + MemW[xword1:3] + 1;
      xword2:=xword1 + 1;
      xbool:=false;
      repeat
        xchar:=Chr(Mem[xword2:0]);
        if xchar = 'S' then
          begin
          xword4:=MemW[xword2 + 1:2];
          xword5:=MemW[xword2 + 1:6];
          xbool:=true;
          end;
        if (xchar = 'M') or (xchar = 'Z') then
          xbool:=true;
        xword2:=xword2 + MemW[xword2:3] + 1;
        if xword2 >= xword3 then
         xbool:=true;
      until xbool;
      Writeln;
      Caption3('amount');
      Write(xword4:3);
      Caption3('size each (bytes)');
      Writeln(xword5:3);
      end
    end;
  if (OSMajor = 5) or (OSMajor >= 20) then
    with regs do
      begin
      Caption2('UMBs');
      AH:=$58;
      AL:=2;
      MsDos(regs);
      if AL = 0 then
        Write('NOT ');
      Writeln('in DOS memory chain');
      end;
  Writeln;
  TextColor(LightCyan);
  Write('------ International Information -----');
  Writeln;
  Caption2('Global code page');
  with regs do
    begin
    AX:=$6601;
    MsDos(Regs);
    if AL = $01 then
      begin
      Writeln;
      Caption3('Active');
      Write(BX);
      Caption3('Default');
      Writeln(DX)
      end
    else
      Writeln('N/A')
    end;
  Caption2('Country code');
  Writeln(ccode);
  case ccode of
      1: s:='United States';
      2: s:='French Canada';
      3: s:='Latin America';
     31: s:='Netherlands';
     32: s:='Belgium';
     33: s:='France';
     34: s:='Spain';
     36: s:='Hungary';
     38: s:='Yugoslavia';
     39: s:='Italy';
     41: s:='Switzerland';
     42: s:='Czechoslovakia';
     44: s:='United Kingdom';
     45: s:='Denmark';
     46: s:='Sweden';
     47: s:='Norway';
     48: s:='Poland';
     49: s:='Germany';
     55: s:='Brazil';
     61: s:='International English';
     81: s:='Japan';
     82: s:='Korea';
     86: s:='China';
     90: s:='Turkey';
    351: s:='Portugal';
    354: s:='Iceland';
    358: s:='Finland';
    785: s:='Saudi Arabia';
    972: s:='Israel';
  else
    s:='Unknown';
  end;
  Caption3('Country');
  Writeln(s);
  Caption2('Thousands separator character');
  Writeln(Chr(Country[7]));
  Caption2('Decimal separator character');
  Writeln(decimal);
  Caption2('Data-list separator character');
  Writeln(Chr(Country[22]));
  Caption2('Date format');
  xword1:=CBW(Country[0], Country[1]);
  xchar:=Chr(Country[11]);
  case xword1 of
    0: Writeln('USA (mm', xchar, 'dd', xchar, 'yy)');
    1: Writeln('Europe (dd', xchar, 'mm', xchar, 'yy)');
    2: Writeln('Japan (yy', xchar, 'mm', xchar, 'dd)')
  else
    unknown('format', xword1, 4)
  end;
  Caption3('Separator character');
  Writeln(xchar);
  Caption2('Time format');
  if (Country[17] and $01) = $00 then
    Write('12')
  else
    Write('24');
  Writeln('-hour');
  Caption3('Separator character');
  Writeln(Chr(Country[13]));
  Caption2('Currency format');
  xstring1:='xxxx';
  insert(Chr(Country[7]), xstring1, 2);
  xstring1:=xstring1 + decimal;
  for i:=1 to Country[16] do
    xstring1:=xstring1 + 'y';
  xstring2:='';
  i:=2;
  xchar:=Chr(Country[i]);
  while (i <= 6) and (xchar > #0) do
    begin
    xstring2:=xstring2 + xchar;
    Inc(i);
    xchar:=Chr(Country[i])
    end;
  case Country[15] and $03 of
    $00 : xstring1:=xstring2 + xstring1;
    $01 : xstring1:=xstring1 + xstring2;
    $02 : xstring1:=xstring2 + ' ' + xstring1;
    $03 : xstring1:=xstring1 + ' ' + xstring2;
    $04 : begin
          Delete(xstring1, 6, 1);
          Insert(xstring2, xstring1, 6)
          end
  end {case};
  Writeln(xstring1);
  Caption2('Case map call address');
  segofs(CBW(Country[20], Country[21]), CBW(Country[18], Country[19]));
  Writeln;
  pause1;
  if endit then
    Exit;
  Window(1, 3, twidth, tlength - 2);
  ClrScr;
  Caption2('Open file handles');
  Writeln;
  if OSMajor >= 10 then
    Writeln('  Unable to determine under OS/2!')
  else
    begin
    xword2:=MemW[DevSeg:DevOfs + 4];
    xword1:=MemW[DevSeg:DevOfs + 6];
    xbool:=false;
    if OSMajor = 3 then
      tablesize:=$35
    else
      tablesize:=$3B;
    repeat
      pause3(-2);
      if endit then
        Exit;
      xword4:=MemW[xword1:xword2];
      xword3:=MemW[xword1:xword2 + 2];
      if xword4 = $FFFF then
        xbool:=true;
      filecount:=MemW[xword1:xword2 + 4];
      usedfiles:=0;
      Caption3('Table at');
      segofs(xword1, xword2);
      Caption3('table size (handles)');
      Writeln(filecount);
      foundit:=false;
      xword2:=xword2 + 6;
      repeat
        if MemW[xword1:xword2] <> 0 then
          begin
          pause3(-3);
          if endit then
            Exit;
          foundit:=true;
          xstring1:='';
          for xword8:=xword2 + $20 to xword2 + $2A do
            xstring1:=xstring1 + Chr(Mem[xword1:xword8]);
          if Copy(xstring1, 9, 3) <> '   ' then
            Insert('.', xstring1, 9)
          else
            Insert(' ', xstring1, 9);
          Write('  ', xstring1);
          Caption3('open mode');
          case MemW[xword1:xword2 + 2] and 7 of
            0: Write('read');
            1: Write('write');
            2: Write('read/write');
            3..7: Write('????');
          end;
          Caption3('sharing mode');
          case (MemW[xword1:xword2 + 2] and $70) shr 4 of
            0: Write('compatible');
            1: Write('deny all');
            2: Write('deny write');
            3: Write('deny read');
            4: Write('deny none');
            5..7: Write('????');
          end;
          Caption3('inherit');
          YesOrNo((MemW[xword1:xword2 + 2] and $80) = $80);
          Caption3('              attributes');
          xbyte:=Mem[xword1:xword2 + 4];
          wasone:=false;
          ShowAttrib('read-only', xbyte, 1);
          ShowAttrib('hidden', xbyte, 2);
          ShowAttrib('system', xbyte, 4);
          ShowAttrib('volume label', xbyte, 8);
          ShowAttrib('directory', xbyte, $10);
          ShowAttrib('archive', xbyte, $20);
          if not wasone then
            Write('(none)');
          Writeln;
          Caption3('              remote');
          YesOrNo2((MemW[xword1:xword2 + 5] and $8000) = $8000);
          Caption3('date');
          UnPackTime(MemL[xword1:xword2 + $D], dt);
          xword5:=CBW(Country[0], Country[1]);
          xchar:=Chr(Country[11]);
          case xword5 of
            $0001: Write(dt.day, xchar, dt.month, xchar, dt.year);
            $0002: Write(dt.year, xchar, dt.month, xchar, dt.day)
          else
            Write(dt.month, xchar, dt.day, xchar, dt.year)
          end;
          Caption3('time');
          if Country[17] and 1 = 0 then
            case dt.hour of
              0: Write('12');
              1..12: ZeroPad(dt.hour);
              13..23: Write(dt.hour - 12)
            end
          else
            ZeroPad(dt.hour);
          Write(Chr(Country[13]), AddZero(dt.min), Chr(Country[13]),
                 AddZero(dt.sec));
          if Country[17] and 1 = 0 then
            if dt.hour > 11 then
              Write(' pm')
            else
              Write(' am');
          Writeln;
          Caption3('              size (bytes)');
          Write(MemL[xword1:xword2 + $11], ' (', (MemL[xword1:xword2 + $11] / 1024.0):0:1, 'K)');
          if (xstring1 <> 'AUX         ') and (xstring1 <> 'CON         ') and
            (xstring1 <> 'PRN         ') then
              begin
              Caption3('owner PSP (Hex)');
              Write(Hex(MemW[xword1:xword2 + $31], 4));
              end
          else
            Write(' DOS device');
          Inc(usedfiles);
          xword2:=xword2 + tablesize;
          Writeln;
          end
        else
          Inc(usedfiles);
      until usedfiles = filecount;
      if not foundit then
        Writeln('  (none used)');
      if not xbool then
        begin
        xword1:=xword3;
        xword2:=xword4
        end;
    until xbool;
    end;
  end;
end.
