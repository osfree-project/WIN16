procedure page_13;
type
  iotbltype = record
                spclfunc: byte;
                devtype: byte;
                devattr: word;
                cylcount: word;
                medtype: byte;
                bpsec: word;
                secpclus: byte;
                resvsec: word;
                fats: byte;
                rootentries: word;
                numsecs: word;
                meddescr: byte;
                secpfat: word;
                secptrk: word;
                numheads: word;
                numhidden: longint;
                largesec: longint;
                reserved: array[$19..$1E] of byte;
              end;

var
  i : $00..$2B;
  xbyte : byte;
  xchar : char;
  xFCB : array[$00..$2B] of byte;
  xlong : longint;
  xstring : string;
  xword1 : word;
  xword2 : word;
  xword3, xword4, xword5: word;
  iotable: iotbltype;
  saveX, saveY: byte;

  begin
  caption2('LASTDRIVE');
  drvname(lastdrv - 1);
  writeln;
  caption2('Logical drives');
  with regs do
    begin
    for xchar:='A' to 'Z' do
      begin
      AH:=$0E;
      DL:=ord(xchar) - ord('A');
      MSDOS(regs);
      AH:=$19;
      MSDOS(regs);
      if AL = DL then
        drvname(AL)
      end;
    writeln;
    AH:=$0E;
    DL:=currdrv;
    MSDOS(regs)
    end;
  caption2('Diskette drives');
  if equip and $0001 = $0001 then
    writeln(1 + equip and $00C0 shr 6)
  else
    writeln(0);
  xword1:=longint(intvec[$1E]) shr 16;
  xword2:=longint(intvec[$1E]) and $0000FFFF;
  caption3('Sectors/track');
  writeln(mem[xword1 : xword2 + 4]);
  caption3('Bytes/sector');
  writeln(mem[xword1 : xword2 + 3] shl 8);
  caption3('On time (ms)');
  writeln(125 * mem[xword1 : xword2 + 10]);
  caption3('Off time (s)');
  writeln(longint(mem[xword1 : xword2 + 2]) shl 16 / tick1:0:1);
  caption3('Head settle time (ms)');
  writeln(mem[xword1 : xword2 + 9]);
  caption1('  Single drive is now ');
  xbyte:=mem[BIOSdseg : $0104];
  if xbyte <= ord('Z') - ord('A') then
    begin
    drvname(xbyte);
    writeln
    end
  else
    if xbyte = $FF then
      writeln('N/A')
    else
      unknown('status', xbyte, 2);
(*  Byte 12:12 p.178  *)
  caption2('Current drive and path');
  getdir(0, xstring);
  writeln(xstring);
  with regs do
    begin
    AH:=$52;
    MsDos(regs);
    if (osmajor = 3) and (osminor = 0) then
      begin
      xword1:=MemW[ES:BX + $19];
      xword2:=MemW[ES:BX + $17]
      end
    else
      begin
      xword1:=MemW[ES:BX + $18];
      xword2:=MemW[ES:BX + $16]
      end;
    if osmajor = 4 then
      xword5:=$58
    else
      xword5:=$51;
    xword3:=xword2 + (xword5 * currdrv);
    caption3('Drive type is');
    case MemW[xword1:xword3 + $43] shr 14 of
      0: Writeln('invalid');
      1: Writeln('physical');
      2: Writeln('network');
      3: Writeln('Installable File System')
    end;
    if (osmajor >= 4) or ((osmajor = 3) and (osminor >= 20)) then
      with regs do
        begin
        AH:=$44;
        AL:=$0D;
        BL:=0;
        CH:=8;
        CL:=$60;
        DS:=Seg(iotable);
        DX:=Ofs(iotable);
        MsDos(regs);
        if Flags and FCarry = 0 then
          with iotable do
            begin
            caption3('removable');
            if devattr and 1 = 0 then
              Write('yes')
            else
              Write('no');
            caption3('door lock');
            yesorno(devattr and 2 = 2);
            end
        end;
    caption3('JOIN''d ');
    if MemW[xword1:xword3 + $43] and $2000 = $2000 then
      begin
      Write('yes');
      caption3('actually');
      xword4:=xword3;
      while Mem[xword1:xword4] <> 0 do
        begin
        Write(Chr(Mem[xword1:xword4]));
        Inc(xword4)
        end;
      Writeln;
      end
    else
      Writeln('no');
    caption3('SUBST''d');
    if MemW[xword1:xword3 + $43] and $1000 = $1000 then
      begin
      Write('yes');
      caption3('actually');
      xword4:=xword3;
      while Mem[xword1:xword4] <> 0 do
        begin
        Write(Chr(Mem[xword1:xword4]));
        Inc(xword4)
        end;
      Writeln;
      end
    else
      Writeln('no')
    end;
  caption3('Volume label');
  for i:=$00 to $2B do
    xFCB[i]:=$00;
  xFCB[$00]:=$FF;         (* extended FCB *)
  xFCB[$06]:=$08;         (* volume ID attribute *)
  for i:=$08 to $12 do
    xFCB[i]:=ord('?');
  with regs do
    begin
    AH:=$11;
    DS:=seg(xFCB);
    DX:=ofs(xFCB);
    MSDOS(regs);
    case AL of
      $00 : begin
            AH:=$2F;
            MSDOS(regs);
            i:=$08;
            xchar:=char(mem[ES : BX + i]);
            while (i <= $12) and (xchar > #0) do
              begin
              write(showchar(xchar));
              inc(i);
              xchar:=char(mem[ES : BX + i])
              end;
            writeln
            end;
      $FF : writeln('(none)')
    else
      unknown('status', AL, 2)
    end {case}
    end;
  with regs do
    begin
    saveX:=WhereX;
    saveY:=WhereY;
    TextColor(LightRed+Blink);
    Write('  *retrieving information*');
    AH:=$1B;
    MSDOS(regs);
    GotoXY(saveX, saveY);
    Write('                          ');
    GotoXY(saveX, saveY);
    media(mem[DS : BX], AL);
    caption3('Clusters');
    writeln(DX);
    caption3('Sectors/cluster');
    writeln(AL);
    caption3('Bytes/sector');
    writeln(CX)
    end;
  caption3('Total space (bytes)');
  xlong:=disksize(0);
  if xlong <> -1 then
    writeln(xlong : 8)
  else
    writeln('(invalid drive)');
  caption3('Free space (bytes) ');
  xlong:=diskfree(0);
  if xlong <> -1 then
    write(xlong : 8)
  else
    write('(invalid drive)')
end;
