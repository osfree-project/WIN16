procedure page_15;

var
  i : byte;
  j : 0..3;
  k : byte;
  part : array[0..secsiz - 1] of byte;
  xbool1 : boolean;
  xbool2 : boolean;
  xbool3 : boolean;
  xbool4 : boolean;
  xbyte : byte;
  xlong : longint;
  xword : word;

  begin
  caption2('Partition table data');
  i:=$80;
  xbool1:=false;
  xbool2:=false;
  repeat
    with regs do
      begin
      AX:=$0201;
      CX:=$0001;
      DX:=i;
      ES:=seg(part);
      BX:=ofs(part);
      intr($13, regs);
      if nocarry then
        begin
        xbool1:=true;
        xbool2:=true
        end
      else
        if i < $99 then
          inc(i)
        else
          xbool1:=true
      end
  until xbool1;
  if xbool2 then
    begin
    writeln;
    caption3('Unit');
    writeln;
    caption3('Partition');
    writeln;
    caption3('Bootable');
    writeln;
    caption3('Starting head');
    writeln;
    caption3('Starting sector');
    writeln;
    caption3('Starting cylinder');
    writeln;
    caption3('System ID');
    writeln;
    caption3('Ending head');
    writeln;
    caption3('Ending sector');
    writeln;
    caption3('Ending cylinder');
    writeln;
    caption3('First partition sector');
    writeln;
    caption3('Sectors in partition');
    writeln;
    xbool3:=false;
    repeat
      window(9, 4, twidth, tlength - 2);
      writeln(i);
      window(27, 5, twidth, tlength - 2);
      clrscr;
      for j:=0 to 3 do
        begin
        window(27 + 12 * j, 5, 38 + 12 * j, tlength - 2);
        writeln(j + 1);
        xword:=$01BE + j shl 4;
        xbyte:=part[xword];
        case xbyte of
          $00 : writeln('no');
          $80 : writeln('yes')
        else
            writeln('(', hex(xbyte, 2), ')')
        end;
        xbyte:=part[xword + 4];
        if xbyte > $00 then
          begin
          writeln(part[xword + 1]);
          writeln(part[xword + 2] and $3F);
          writeln(part[xword + 3] +
            word(part[xword + 2] shr 6 * 256));
          case xbyte of
            $01 : write('DOS-12');
            $02,$03 : Write('XENIX');
            $04 : write('DOS-16');
            $05 : write('Ext DOS-16');
            $06 : Write('Big DOS-16');
            $07 : Write('QNX');
            $08 : Write('AIX boot');
            $09 : Write('AIX data');
            $51 : Write('Dsk Managr');
            $52 : Write('CP/M');
            $56 : Write('GB');
            $61 : Write('Speedstor');
            $63 : Write('SysV/386');
            $64 : Write('NOVELL');
            $75 : Write('PC/IX');
            $80 : Write('Minix v1.3-');
            $81 : Write('Minix v1.5+');
            $DB : Write('CP/M');
            $E1 : Write('SpdStr-12');
            $E4 : Write('SpdStr-16');
            $FF : Write('BBT')
          else
            write('(', hex(xbyte, 2), ')')
          end;
          Writeln;
          writeln(part[xword + 5]);
          xbyte:=part[xword + 6];
          writeln(xbyte and $3F);
          writeln(cbw(part[xword + 7], xbyte shr 6));
          xlong:=0;
          for k:=11 downto 8 do
            xlong:=xlong shl 8 + part[xword + k];
          writeln(xlong);
          xlong:=0;
          for k:=15 downto 12 do
            xlong:=xlong shl 8 + part[xword + k];
          writeln(xlong)
        end
      else
         for k:=1 to 9 do
           writeln('-')
      end;
      xbool4:=false;
      repeat
        if i < $99 then
          begin
          inc(i);
          with regs do
            begin
            AX:=$0201;
            CX:=$0001;
            DX:=i;
            ES:=seg(part);
            BX:=ofs(part);
            intr($13, regs);
            if nocarry then
              begin
              xbool4:=true;
              pause1;
              if endit then
                Exit;
              end
            end
          end
        else
          begin
          xbool3:=true;
          xbool4:=true
          end
      until xbool4
    until xbool3
    end
  else
    writeln('(no fixed disks)')
  end;
