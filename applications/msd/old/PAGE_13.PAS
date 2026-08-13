unit page_13;

interface

uses crt, dos, ifpglobl, ifpcomon;

procedure page13;

implementation

procedure page13;
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
  Caption2('LASTDRIVE');
  if osmajor >= 20 then
    DrvName(lastdrv)
  else
    DrvName(lastdrv - 1);
  Writeln;
  Caption2('Logical drives');
  with regs do
    begin
    xbyte:=0;
    for xchar:='A' to 'Z' do
      begin
      AH:=$0E;
      DL:=ord(xchar) - ord('A');
      MSDOS(regs);
      AH:=$19;
      MSDOS(regs);
      if AL = DL then
        begin
        DrvName(AL);
        Inc(xbyte);
        if xbyte = 20 then
          begin
          Writeln;
          Write('                ');
          xbyte:=0;
          end;
        end;
      end;
    Writeln;
    AH:=$0E;
    DL:=currdrv;
    MSDOS(regs)
    end;
  Caption2('Diskette drives');
  if equip and $0001 = $0001 then
    Writeln(1 + equip and $00C0 shr 6)
  else
    Writeln(0);
  xword1:=longint(intvec[$1E]) shr 16;
  xword2:=longint(intvec[$1E]) and $0000FFFF;
  Caption3('Sectors/track');
  Writeln(Mem[xword1 : xword2 + 4]);
  Caption3('Bytes/sector');
  Writeln(Mem[xword1 : xword2 + 3] shl 8);
  Caption3('On time (ms)');
  Writeln(125 * Mem[xword1 : xword2 + 10]);
  Caption3('Off time (s)');
  Writeln(longint(Mem[xword1 : xword2 + 2]) shl 16 / tick1:0:1);
  Caption3('Head settle time (ms)');
  Writeln(Mem[xword1 : xword2 + 9]);
  if 1 + ((equip and $00C0) shr 6) = 1 then
    begin
    Caption1('  Single drive is now ');
    xbyte:=Mem[BIOSdseg : $0104];
    if xbyte <= ord('Z') - ord('A') then
      begin
      drvname(xbyte);
      Writeln
      end
    else
      if xbyte = $FF then
        Writeln('N/A')
      else
        unknown('status', xbyte, 2);
    end;
(*  Byte 12:12 p.178  *)
  Caption2('Current drive and path');
  GetDir(0, xstring);
  Writeln(xstring);
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
    if (osmajor >= 4) and (osmajor < 10) then
      xword5:=$58
    else
      xword5:=$51;
    if not ((xword1 = $FFFF) and (xword2 = $FFFF)) then
      begin
      xword3:=xword2 + (xword5 * currdrv);
      Caption3('Drive type is');
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
              Caption3('removable');
              if devattr and 1 = 0 then
                Write('yes')
              else
                Write('no');
              Caption3('door lock');
              yesorno(devattr and 2 = 2);
              end
          else
            begin
            AX:=$4408;
            BL:=0;
            MsDos(regs);
            if Flags and FCarry = 0 then
              begin
              Caption3('removable');
              YesOrNo(AL = 0);
              end;
            end;

          end;
      Caption3('JOIN''d ');
      if MemW[xword1:xword3 + $43] and $2000 = $2000 then
        begin
        Write('yes');
        Caption3('actually');
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
      Caption3('SUBST''d');
      if MemW[xword1:xword3 + $43] and $1000 = $1000 then
        begin
        Write('yes');
        Caption3('actually');
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
    end;
  Caption3('Volume label');
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
            xchar:=char(Mem[ES : BX + i]);
            while (i <= $12) and (xchar > #0) do
              begin
              Write(showchar(xchar));
              inc(i);
              xchar:=char(Mem[ES : BX + i])
              end;
            Writeln
            end;
      $FF : Writeln('(none)')
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
    media(Mem[DS : BX], AL);
    Caption3('Clusters');
    Writeln(DX);
    Caption3('Sectors/cluster');
    Writeln(AL);
    Caption3('Bytes/sector');
    Writeln(CX)
    end;
  Caption3('Total space (bytes)');
  xlong:=disksize(0);
  if xlong <> -1 then
    Writeln(xlong:9, ' (', (xlong / 1024.0):9:0, 'K)')
  else
    Writeln('(invalid drive)');
  Caption3('Free space (bytes) ');
  xlong:=diskfree(0);
  if xlong <> -1 then
    Write(xlong:9, ' (', (xlong / 1024.0):9:0, 'K)')
  else
    Write('(invalid drive)')
end;
end.