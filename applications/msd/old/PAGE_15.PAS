unit page_15;

interface

uses crt, ifpglobl, ifpcomon;

procedure page15;

implementation

procedure page15;

var
  drive : byte;
  j : 0..3;
  k : byte;
  Part : array[0..secsiz - 1] of byte;
  EndOfDrives : boolean;
  ValidDrive : boolean;
  LastDrive : boolean;
  AnotherDrive : boolean;
  xbyte : byte;
  xlong : longint;
  xword : word;
  s: string;

  begin
  caption2('Partition table data');
  if ReadPartitionTable then
    begin
    drive:=$80;
    EndOfDrives:=false;
    ValidDrive:=false;
    repeat
      with regs do
        begin
        AX:=$0201;
        CX:=$0001;
        DX:=drive;
        ES:=Seg(Part);
        BX:=Ofs(Part);
        Intr($13, regs);
        if NoCarry(regs) then
          begin
          EndOfDrives:=true;
          ValidDrive:=true
          end
        else
          if drive < $99 then
            Inc(drive)
          else
            EndOfDrives:=true
        end
    until EndOfDrives;
    if ValidDrive then
      begin
      Writeln;
      caption3('Unit');
      Writeln;
      caption3('Partition');
      Writeln;
      caption3('Bootable');
      Writeln;
      caption3('Starting head');
      Writeln;
      caption3('Starting sector');
      Writeln;
      caption3('Starting cylinder');
      Writeln;
      caption3('System ID');
      Writeln;
      caption3('Ending head');
      Writeln;
      caption3('Ending sector');
      Writeln;
      caption3('Ending cylinder');
      Writeln;
      caption3('First partition sector');
      Writeln;
      caption3('Sectors in partition');
      Writeln;
      LastDrive:=false;
      repeat
        Window(9, 4, twidth, tlength - 2);
        Writeln(drive);
        Window(27, 5, twidth, tlength - 2);
        ClrScr;
        for j:=0 to 3 do
          begin
          Window(27 + 12 * j, 5, 38 + 12 * j, tlength - 2);
          Writeln(j + 1);
          xword:=$01BE + j shl 4;
          xbyte:=Part[xword];
          case xbyte of
            $00 : Writeln('no');
            $80 : Writeln('yes')
          else
              Writeln('(', hex(xbyte, 2), ')')
          end;
          xbyte:=Part[xword + 4];
          if xbyte > $00 then
            begin
            Writeln(Part[xword + 1]);
            Writeln(Part[xword + 2] and $3F);
            Writeln(Part[xword + 3] +
              word(Part[xword + 2] shr 6 * 256));
            s:='';
            case xbyte of
              $00 : s:='None';
              $01 : s:='DOS-12';
              $02 : s:='XENIX root';
              $03 : s:='XENIX /usr';
              $04 : s:='DOS-16';
              $05 : s:='Ext DOS-16';
              $06 : s:='Big DOS-16';
              $07 : s:='OS/2 HPFS';
              $08 : s:='AIX data';
              $09 : s:='AIX boot';
              $0A : s:='OS/2 BtMngr';
              $10 : s:='OPUS';
              $24 : s:='NEC DOS 3.x';
              $40 : s:='VENIX 286';
              $44 : s:='386BSD';
              $50 : s:='DskMngrR/O';
              $51 : s:='Dsk Managr';
              $52 : s:='CP/M';
              $56 : s:='GB Vfeatre';
              $61 : s:='Speedstor';
              $63 : s:='SysV/386';
              $64 : s:='NOVELL';
              $75 : s:='PC/IX';
              $80 : s:='Minix v1.3-';
              $81 : s:='Minix v1.4+';
              $82 : s:='Minix Swap';
              $83 : s:='Linux extd';
              $84 : s:='OS/2 hidden';
              $93 : s:='Amoeba file';
              $94 : s:='Amoeba BBT';
              $B7 : s:='BSDI file';
              $B8 : s:='BSDI swap';
              $C1 : s:='DRDOSscr12b';
              $C4 : s:='DRDOSscr16b';
              $C6 : s:='DRDOSscrHug';
              $C7 : s:='Cyrnix boot';
              $DB : s:='CP/M';
              $E1 : s:='SpdStr-12';
              $E3 : s:='SpdStr R/O';
              $E4 : s:='SpdStr-16';
              $F2 : s:='DOS secndry';
              $FE : s:='LANstep';
              $FF : s:='Xenix BBT';
            end;
            if s = '' then
              Write('(', hex(xbyte, 2), ')')
            else
              Write(s);
            Writeln;
            Writeln(Part[xword + 5]);
            xbyte:=Part[xword + 6];
            Writeln(xbyte and $3F);
            Writeln(cbw(Part[xword + 7], xbyte shr 6));
            xlong:=0;
            for k:=11 downto 8 do
              xlong:=xlong shl 8 + Part[xword + k];
            Writeln(xlong);
            xlong:=0;
            for k:=15 downto 12 do
              xlong:=xlong shl 8 + Part[xword + k];
            Writeln(xlong)
          end
        else
           for k:=1 to 9 do
             Writeln('-')
        end;
        AnotherDrive:=false;
        repeat
          if drive < $99 then
            begin
            Inc(drive);
            with regs do
              begin
              AX:=$0201;
              CX:=$0001;
              DX:=drive;
              ES:=Seg(Part);
              BX:=Ofs(Part);
              Intr($13, regs);
              if NoCarry(regs) then
                begin
                AnotherDrive:=true;
                Pause1;
                if endit then
                  Exit;
                end
              end
            end
          else
            begin
            LastDrive:=true;
            AnotherDrive:=true
            end
        until AnotherDrive
      until LastDrive
      end
    else
      Writeln('(no fixed disks)')
    end
  else
    begin
    Writeln;
    Writeln('Reading of Partition Table blocked by NP command-line switch!!');
    end;
  end;
end.
