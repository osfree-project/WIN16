procedure page_01;
  const
    BIOScseg = $C000;
    BIOSext = $AA55;
    PCROMseg = $F000;
    dells: array [2..$11] of string[5] = ('200', '300', '?', '220', '310', '325',
             '?', '310A', '316', '220E', '210', '316SX', '316LT', '320LX',
             '?', '425E');
    dellnums: set of 0..$FF = [2, 3, 5..7, 9..$0F, $11];

  var
    xbool : boolean;
    xbyte : byte;
    xchar : char;
    xlong : longint;
    xword1 : word;
    xword2 : word;
    s: string;

  function BIOSscan(a, b, c : word; var d : word) : boolean;
    const
      max = 3;
      notice : array[1..max] of string = ('(C)', 'COPR.', 'COPYRIGHT');

    var
      i : 1..max;
      len : byte;
      target : string;
      xbool : boolean;
      xlong : longint;
      xword : word;

    function scan(a : string; b, c, d : word; var e : word) : boolean;
      var
        i : longint;
        j : byte;
        len : byte;
        xbool1 : boolean;
        xbool2 : boolean;

      begin
      i:=c;
      len:=length(a);
      xbool1:=false;
      repeat
        if i <= longint(d) - len + 1 then
          begin
          j:=0;
          xbool2:=false;
          repeat
            if j < len then
              if upcase(chr(mem[b : i + j])) = a[j + 1] then
                inc(j)
              else
                begin
                xbool2:=true;
                inc(i)
                end
            else
              begin
              xbool2:=true;
              xbool1:=true;
              e:=i;
              scan:=true
              end
          until xbool2
          end
        else
          begin
          xbool1:=true;
          scan:=false
          end
      until xbool1
      end; {scan}

    begin (* function BIOSscan *)
    xlong:=c;
    xbool:=false;
    for i:=1 to max do
      begin
      target:=notice[i];
      len:=length(target);
      if xbool then
        xlong:=longint(xword) - 2 + len;
      if (xlong >= b) and (xlong <= c) and (scan(target, a, b, xlong, xword))
        then
          xbool:=true
      end;
    if xbool then
      begin
      while (xword > b) and (chr(mem[a : xword - 1]) in pchar) do
        dec(xword);
      d:=xword
      end;
    BIOSscan:=xbool
    end; {biosscan}

  procedure showBIOS(a, b : word);
    var
      xbool : boolean;
      xchar : char;

    begin
    xbool:=false;
    repeat
      xchar:=chr(mem[a : b]);
      if xchar in pchar then
        begin
        write(xchar);
        if b < $FFFF then
          inc(b)
        else
          xbool:=true
        end
      else
        xbool:=true
    until xbool;
    writeln
    end; {showbios}

  begin (* procedure page_01 *)
  caption2('Machine type');
  if UpCase(Chr(Mem[$F000:$E076])) = 'D' then
    begin
    s:='';
    for xword1:=$E077 to $E079 do
      s:=s + UpCase(Chr(Mem[$F000:xword1]));
    if s = 'ELL' then
      begin
      Write('Dell ');
      xbool:=true;
      xbyte:=Mem[$F000:$E845];
      if xbyte in dellnums then
        Write(dells[xbyte])
      else
        begin
        Write('(unknown - ID is ', hex(xbyte, 2));
        xbool:=false
        end;
      if xbool then
        begin
        caption3('BIOS Revision');
        for xword1:=$E845 to $E847 do
          Write(Chr(Mem[$F000:xword1]))
        end;
      Writeln;
      caption2('Standard BIOS call says');
      Writeln
      end
    end;
  with regs do
    begin
    AH:=$C0;
    intr($15, regs);
    if nocarry then
      begin
      xword1:=memw[ES : BX + 2];
      xbyte:=MemW[ES:BX + 4];
      case xword1 of
        $00FC:        if xbyte = 1 then
                        Writeln('PC-AT 2x9')
                      else
                        Writeln('Industrial AT 7531/2');
        $01FC:        case xbyte of
                        $00: Writeln('PC-AT 3x9 or clone');
                        $30: Writeln('Tandy 3000NL')
                      else
                        Writeln('Compaq 286/386 or clone');
                      end;
        $00FB:        if xbyte = 1 then
                        Writeln('PC-XT w/ Enh kbd, 3.5" support')
                      else
                        Writeln('PC-XT');
        $01FB:        Writeln('PC-XT/2');
        $02FC:        Writeln('PC-XT/286');
        $00F9:        Writeln('PC-Convertible');
        $00FA:        Writeln('PS/2 Model 30');
        $01FA:        Writeln('PS/2 Model 25');
        $09FC:        Writeln('PS/2 Model 30-286');
        $04FC:        if xbyte = 3 then
                        Writeln('PS/2 Model 50Z')
                      else
                        Writeln('PS/2 Model 50');
        $05FC:        Writeln('PS/2 Model 60');
        $04F8, $09F8: Writeln('PS/2 Model 70');
        $0CF8:        Writeln('PS/2 Model 55SX');
        $1BF8:        Writeln('PS2 Model 70-486');
        $00F8:        Writeln('PS/2 Model 80 16MHz');
        $01F8:        Writeln('PS/2 Model 80 20MHz');
        $06FC:        Writeln('7552 Gearbox');
        $81FC:        Writeln('AT clone with Phoenix 386 BIOS');
      else
        unknown('machine - model/type word', xword1, 4);
      end; {case}
      caption3('BIOS revision level');
      writeln(mem[ES : BX + 4]);
      xbyte:=mem[ES : BX + 5];
      caption3('DMA channel 3 used');
      yesorno(xbyte and $80 = $80);
      caption3('Slave 8259 present');
      yesorno(xbyte and $40 = $40);
      caption3('Real-time clock');
      yesorno(xbyte and $20 = $20);
      caption3('Keyboard intercept available');
      yesorno(xbyte and $10 = $10);
      caption3('Wait for external event available');
      yesorno(xbyte and $08 = $08);
      caption3('Extended BIOS data area segment');
      if xbyte and $04 = $04 then
        begin
        AH:=$C1;
        intr($15, regs);
        if nocarry then
          writeln(hex(ES, 4))
        else
          dontknow
        end
      else
        writeln('(none)');
      caption3('Micro Channel');
      yesorno(xbyte and $02 = $02)
      end
    else
      begin
      xbyte:=mem[$FFFF : $000E];
      s:='';
      for xword1:=$FFF5 to $FFFC do
        s:=s + Chr(Mem[$F000:xword1]);
      case xbyte of
        $FF : begin
              Write('PC ');
              if s = '04/24/81' then
                Write('(original)');
              if s = '10/19/81' then
                Write('(revised BIOS)');
              if s = '10/27/82' then
                Write('(HD, 640K, EGA supported)');
              Writeln;
              end;
        $FE : begin
              Write('PC-XT');
              if s = '11/08/82' then
                Write(' or Portable');
              Writeln;
              end;
        $FD : writeln('PCjr');
        $FC : writeln('PC-AT');
        $9A : Writeln('Compaq XT or Compaq Plus');
        $30 : Writeln('Sperry PC');
        $2D : Writeln('Compaq PC or Compaq Deskpro')
        else
          unknown('machine - model byte', xbyte, 2)
      end
      end
  end;
(*  Byte 12:12 p. 174  *)
  caption2('BIOS source');
  if BIOSscan(PCROMseg, $C000, $FFFF, xword1) then
    showBIOS(PCROMseg, xword1)
  else
    dontknow;
  caption2('BIOS date');
  i:=$0005;
  xbool:=false;
  xchar:=chr(mem[$FFFF : i]);
  while (i < $0010) and (xchar in pchar) do
    begin
    xbool:=true;
    write(xchar);
    inc(i);
    xchar:=chr(mem[$FFFF : i])
    end;
  if xbool then
    writeln
  else
    dontknow;
  caption2('BIOS extensions');
  xword1:=BIOScseg;
  xbool:=false;
  for i:=0 to 94 do
    begin
    if (memw[xword1 : 0] = BIOSext) then
      begin
      if not xbool then
        begin
        writeln;
        window(3, wherey + hi(windmin), twidth, tlength - 2);
        caption1('Segment Size  Copyright notice');
        writeln;
        xbool:=true
        end;
      pause2;
      if endit then
        Exit;
      Write(hex(xword1, 4), '    ', ((longint(512) * Mem[xword1: 2]) div 1024):3, 'K  ');
      if BIOSscan(xword1, $0000, $1FFF, xword2) then
        showBIOS(xword1, xword2)
      else
        dontknow
      end;
    inc(xword1, $0080)
    end;
  if not xbool then
    writeln('(none)')
  end;
