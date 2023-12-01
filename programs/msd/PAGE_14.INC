procedure page_14;

var
  i : byte;
  xbool : boolean;
  xbyte1 : byte;
  xbyte2 : byte;

  begin
  caption2('BIOS disk parameters');
  xbool:=true;
  for i:=$00 to $FF do
    with regs do
      begin
      AH:=$08;
      DL:=i;
      intr($13, regs);
      if nocarry and ((BL > $00) or (i >= $80)) then
        begin
        if xbool then
          begin
          xbool:=false;
          writeln;
          caption3('Unit');
          writeln;
          caption3('Type');
          writeln;
          caption3('Drives');
          writeln;
          caption3('Heads');
          writeln;
          caption3('Cylinders');
          writeln;
          caption3('Sectors/track');
          writeln;
          caption3('Specify bytes');
          writeln;
          caption3('Off time (s)');
          writeln;
          caption3('Bytes/sector');
          writeln;
          caption3('Sectors/track');
          writeln;
          caption3('Gap length');
          writeln;
          caption3('Data length');
          writeln;
          caption3('Gap length for format');
          writeln;
          caption3('Fill byte for format');
          writeln;
          caption3('Head settle time (ms)');
          writeln;
          caption3('On time (ms)');
          writeln;
          xbyte1:=27;
          end;
        if xbyte1 + 10 > twidth then
          begin
          pause1;
          if endit then
            Exit;
          xbyte1:=27;
          window(xbyte1, 4, twidth, tlength - 2);
          clrscr
          end;
        window(xbyte1, 4, xbyte1 + 11, tlength - 2);
        writeln(i);
        if i < $80 then
          case BL of
            $01 : writeln('360KB 5¬"');
            $02 : writeln('1.2MB 5¬"');
            $03 : writeln('720KB 3«"');
            $04 : writeln('1.44MB 3«"')
          else
            writeln('(', hex(BL, 2), ')')
          end
        else
          writeln('fixed disk');
        writeln(DL);
        writeln(DH + 1);
        writeln(cbw(CH, CL shr 6) + 1);
        writeln(CL and $3F);
        if i < $80 then
          begin
          writeln('$', hex(mem[ES : DI], 2), ' $'
            , hex(mem[ES : DI + $0001], 2));
          writeln(longint(mem[ES : DI + $0002]) shl 16 / tick1 : 0
            : 1);
          xbyte2:=mem[ES : DI + $0003];
          case xbyte2 of
            $00 : writeln('128');
            $01 : writeln('256');
            $02 : writeln('512');
            $03 : writeln('1024')
          else
            writeln('(', hex(xbyte2, 4), ')')
          end;
          writeln(mem[ES : DI + $0004]);
          writeln(mem[ES : DI + $0005]);
          writeln(mem[ES : DI + $0006]);
          writeln(mem[ES : DI + $0007]);
          writeln('$', hex(mem[ES : DI + $0008], 2));
          writeln(mem[ES : DI + $0009]);
          writeln(125 * mem[ES : DI + $000A])
          end;
        inc(xbyte1, 13)
        end
      end;
  if xbool then
    writeln('(no disks)')
  end;
