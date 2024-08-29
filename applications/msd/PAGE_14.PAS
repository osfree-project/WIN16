unit page_14;

interface

uses crt, ifpglobl, ifpcomon;

procedure page14;

implementation

procedure page14;
  var
    drive : byte;
    NoDisks : boolean;
    xbyte1 : byte;
    xbyte2 : byte;

  begin
  Caption2('BIOS disk parameters');
  NoDisks:=true;
  for drive:=$00 to $FF do
    with regs do
      begin
      AH:=$08;
      DL:=drive;
      Intr($13, regs);
      if nocarry(regs) and ((BL > $00) or (drive >= $80)) and (DL > 0) then
        begin
        if NoDisks then
          begin
          NoDisks:=false;
          Writeln;
          Caption3('Unit');
          Writeln;
          Caption3('Type');
          Writeln;
          Caption3('Drives');
          Writeln;
          Caption3('Heads');
          Writeln;
          Caption3('Cylinders');
          Writeln;
          Caption3('Sectors/track');
          Writeln;
          Caption3('Specify bytes');
          Writeln;
          Caption3('Off time (s)');
          Writeln;
          Caption3('Bytes/sector');
          Writeln;
          Caption3('Sectors/track');
          Writeln;
          Caption3('Gap length');
          Writeln;
          Caption3('Data length');
          Writeln;
          Caption3('Gap length for format');
          Writeln;
          Caption3('Fill byte for format');
          Writeln;
          Caption3('Head settle time (ms)');
          Writeln;
          Caption3('On time (ms)');
          Writeln;
          xbyte1:=27;
          end;
        if xbyte1 + 10 > twidth then
          begin
          pause1;
          if endit then
            Exit;
          xbyte1:=27;
          Window(xbyte1, 4, twidth, tlength - 2);
          ClrScr
          end;
        Window(xbyte1, 4, xbyte1 + 11, tlength - 2);
        Writeln(drive);
        if drive < $80 then
          case BL of
            $01 : Writeln('360KB 5¬"');
            $02 : Writeln('1.2MB 5¬"');
            $03 : Writeln('720KB 3«"');
	    $04 : Writeln('1.44MB 3«"');
	    $05 : Writeln('2.88MB 3«"')
	  else
	    Writeln('(', Hex(BL, 2), ')')
	  end
	else
	  Writeln('fixed disk');
	Writeln(DL);
        Writeln(DH + 1);
        Writeln(cbw(CH, CL shr 6) + 1);
        Writeln(CL and $3F);
        if drive < $80 then
          begin
          Writeln('$', Hex(Mem[ES : DI], 2), ' $'
            , Hex(Mem[ES : DI + $0001], 2));
          Writeln(longint(Mem[ES : DI + $0002]) shl 16 / tick1 : 0
            : 1);
          xbyte2:=Mem[ES : DI + $0003];
          case xbyte2 of
            $00 : Writeln('128');
            $01 : Writeln('256');
            $02 : Writeln('512');
            $03 : Writeln('1024')
          else
            Writeln('(', Hex(xbyte2, 4), ')')
          end;
          Writeln(Mem[ES : DI + $0004]);
          Writeln(Mem[ES : DI + $0005]);
          Writeln(Mem[ES : DI + $0006]);
          Writeln(Mem[ES : DI + $0007]);
          Writeln('$', Hex(Mem[ES : DI + $0008], 2));
          Writeln(Mem[ES : DI + $0009]);
          Writeln(125 * Mem[ES : DI + $000A])
          end;
        inc(xbyte1, 13)
        end
      end;
  if NoDisks then
    Writeln('(no disks)')
  end;
end.