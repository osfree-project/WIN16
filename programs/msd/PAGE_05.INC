procedure page_05;
  const
    vidmodes: array [$00..$13] of string[36] = (
                '(40 x 25 b/w text)',
                '(40 x 25 color text)',
                '(80 x 25 b/w text)',
                '(80 x 25 color text)',
                '(320 x 200 4 colors)',
                '(320 x 200 4 colors, no color burst)',
                '(640 x 200 2 colors)',
                '(MDA text)',
                '(160 x 200 16 colors)',
                '(320 x 200 16 colors)',
                '(640 x 200 4 colors)',
                '(reserved)',
                '(reserved)',
                '(320 x 200 16 colors)',
                '(640 x 200 16 colors)',
                '(640 x 350 monochrome)',
                '(640 x 350 16 colors)',
                '(640 x 480 2 colors)',
                '(640 x 480 16 colors)',
                '(320 x 200 256 colors)');


  var
    i : byte;
    xbyte : byte;
    xint1 : integer;
    xint2 : integer;
    xword : word;
    mono2: boolean;

procedure showdisp(a : string; b : byte);
  const
    dispmons: array [0..$0C] of string[25] = (
               '(none)',
               'MDA + 5151',
               'CGA + 5153/5154',
               '(reserved)',
               'EGA + 5153/5154',
               'EGA 5151',
               'PGA + 5175',
               'VGA + analog monochrome',
               'VGA + analog color',
               '(reserved)',
               'MCGA + digital color',
               'MCGA + digital monochrome',
               'MCGA + analog color');

begin
  caption2(a);
  case b of
    $00..$0C: Writeln(dispmons[b]);
    $0D..$FE: Writeln('(reserved)');
    $FF: dontknow
  end;
end; {showdisp}

procedure showcolor(a : byte);
  const
    colors: array[black..white] of string[13] = (
             'black', 'blue', 'green', 'cyan', 'red', 'magenta', 'brown',
             'light gray', 'dark gray', 'light blue', 'light green',
             'light cyan', 'light red', 'light magenta', 'yelow', 'white');

  begin
  if a <= white then
    Write(colors[a])
  else
    unknown('color', a, 2)
  end; {showcolor}

  begin (* procedure page_05 *)
  with regs do
    begin
    AX:=$1A00;
    intr($10, regs);
    if AL = $1A then
      begin
      showdisp('Active video subsystem  ', BL);
      showdisp('Inactive video subsystem', BH)
      end
    end;
  caption2('Initial video mode');
  case equip and $0030 of
    $0000 : writeln('No display');
    $0010 : writeln('40 x 25 color');
    $0020 : writeln('80 x 25 color');
    $0030 : writeln('80 x 25 monochrome')
  end;
  caption2('Current video mode');
  xbyte:=lo(lastmode);
  Write(xbyte, ' ');
  if xbyte <= $13 then
    Writeln(vidmodes[xbyte])
  else
    unknown('video mode', xbyte, 2);
  caption2('Current display page');
  writeln(vidpg);
  caption2('Graphics modes');
  getmoderange(graphdriver, xint1, xint2);
  if graphresult = grok then
    writeln(xint2 + 1 - xint1)
  else
    writeln(0);
  caption2('Video buffer (offset)');
  writeln(hex(memw[BIOSdseg : $004E], 4));
  caption2('Video buffer size (bytes)');
  writeln(memw[BIOSdseg : $004C]);
  caption2('Active display port');
  xword:=memw[BIOSdseg : $0063];
  write('$', hex(xword, 3), ' ');
  if xword = $03B4 then
    writeln('(monochrome)')
  else
     if xword = $03D4 then
       writeln('(color)')
     else
       dontknow;
  caption2('CRT mode register');
  writeln('$', hex(mem[BIOSdseg : $0065], 2));
  caption2('Current palette');
  writeln('$', hex(mem[BIOSdseg : $0066], 2));
  caption2('Colors');
  caption1('ú');
  mono2:=mono;
  mono:=false;
  for i:=black to white do
    begin
    textcolor(i);
    write('Û')
    end;
  mono:=mono2;
  caption1('ú');
  writeln;
  caption2('Current colors');
  if (attrsave and $80) = $80 then
    write('blinking ');
  showcolor(attrsave and $0F);
  write(' on ');
  showcolor(attrsave and $70 shr 4);
  writeln;
  caption2('Text rows');
  writeln(tlength);
  caption2('Text columns');
  writeln(twidth);
  if graphdriver in [EGA, MCGA, VGA] then
    begin
    caption2('Scan lines/character');
    with regs do
      begin
      AX:=$1130;
      BH:=$00;
      intr($10, regs);
      writeln(CX)
      end
    end;
  caption2('Cursor scan lines');
  with regs do
    begin
    AH:=$03;
    BH:=vidpg;
    intr($10, regs);
    writeln(CH, '-', CL)
    end
  end;
