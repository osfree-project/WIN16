procedure page_06;
const
  atividmons: array[0..15] of string[24] =
               ('EGA', 'analog monochrome', 'TTL monochrome', 'analog color',
                'RGB color', 'Multisync or compatible', '(unknown)',
                'PS/2 8514 or compatible', '(unknown)', 'NEC MultiSync 2A',
                'Tatung OmniScan', 'NEC 3D or compatible', 'TVM 3M',
                'NEC MultiSync XL/+/4D/5D', 'TVM 3A', 'TVM 2A');
  trividmons: array [0..7] of string[17] =
               ('MDA', 'CGA', 'EGA', 'Digital multisync', 'VGA', '8514',
                'SuperVGA', 'Analog multisync');

type
  cardtype = (none, vesa, standard, paradise, video7, ati, ahead, cirrus,
               cti, genoa, trident, tseng, zymos);
  VESAitype = record
                signature: array[0..3] of char;
                version: word;
                OEMnameOfs: word;
                OEMnameSeg: word;
                capabilities: array[0..3] of byte;
                modesOfs: word;
                modesSeg: word;
                reserved: array[0..237] of byte;
              end;
  VESAmtype = record
                modeattr: word;
                winaattr: byte; {Window A attributes}
                winbattr: byte; {Window B attributes}
                wingran: word;  {Window Granularity}
                winsize: word;  {Window Size}
                winaseg: word;  {Window A segment}
                winbseg: word;  {Window B segment}
                posOfs: word;   {Offset of Far call to positioning function}
                posSeg: word;   {Segment ..}
                scansize: word; {Bytes per scan line}
                {The following information is optional for VESA modes,
                 required for OEM modes}
                pixwidth: word;
                pixheight: word;
                charwidth: byte;
                charheight: byte;
                memplanes: byte;
                pixelbits: byte;
                banks: byte;
                memmodel: byte;
                banksize: byte;
                reserved: array[0..227] of byte;
              end;

var
  i : byte;
  VGAbuf : array[$00..$10] of byte;
  VESAinfo: VESAitype;
  VESAmode: VESAmtype;
  xbyte : byte;
  xword1 : word;
  xword2 : word;
  xword3 : word;
  xword4 : word;
  vgacard: cardtype;
  vidmem : word;
  s: string;
  c: char;

procedure captfont;
  begin
  caption1('Font           Address');
  writeln;
  write('INT 1FH        ');
  segofs(longint(intvec[$1F]) shr 16, longint(intvec[$1F]) and $0000FFFF);
  writeln
  end; {captfont}

procedure showfont(a : byte);
  begin
  with regs do
    begin
    case a of
      $00 : write('INT 1FH     ');
      $01 : write('INT 43H     ');
      $02 : write('ROM 8x14    ');
      $03 : write('ROM 8x8 (lo)');
      $04 : write('ROM 8x8 (hi)');
      $05 : write('ROM 9x14    ');
      $06 : write('ROM 8x16    ');
      $07 : write('ROM 9x16    ')
    end;
    write('   ');
    AX:=$1130;
    BH:=a;
    intr($10, regs);
    segofs(ES, BP);
    writeln
    end
  end; {showfont}

procedure int101210;
  const
    memnames: array[0..3] of string[4] = ('64K', '128K', '192K', '256K');

  begin
  with regs do
    begin
    AH:=$12;
    BL:=$10;
    intr($10, regs);
    caption2('Display type');
    case BH of
      $00 : writeln('color');
      $01 : writeln('monochrome')
      else
        unknown('display', BH, 2)
    end;
    caption2('Memory');
    if vgacard <> standard then
      Writeln(vidmem, 'K')
    else
      if BL < 4 then
        Writeln(memnames[BL])
      else
        unknown('size', BL, 2);
    if vgacard = none then
      begin
      caption2('Feature bits');
      writeln(bin4(CH and $0F));
      caption2('DIP switches (EGA)');
      writeln(bin4(CL and $0F))
      end
    end
  end;

  function readROM(seg, ofs: word; length: byte): string;
    var
      x: word;
      s: string;

    begin
    s:='';
    for x:=ofs to ofs + (length - 1) do
      s:=s + Chr(Mem[seg:x]);
    readROM:=s
    end; {readROM}

  begin (* procedure page_06 *)
  vgacard:=none;
  caption2('Display adapter');
  with regs do
    begin
    AX:=$4F00;
    ES:=Seg(VESAinfo);
    DI:=Ofs(VESAinfo);
    Intr($10, regs);
    s:='';
    if (AL = $4F) and (AH = 0) and (VESAinfo.signature = 'VESA') then
      begin
      with VESAinfo do
        begin
        vgacard:=vesa;
        Writeln('VESA version ', Hi(version), decimal, Lo(version));
        caption2('OEM ID');
        xword1:=OEMnameSeg;
        xword2:=OEMnameOfs;
        s:='';
        c:=Chr(Mem[xword1:xword2]);
        while c <> #0 do
          begin
          Write(c);
          s:=s + c;
          Inc(xword2);
          c:=Chr(Mem[xword1:xword2])
          end;
        caption3('Manufacturer');
        if s = '761295520' then
          Writeln('ATI')
        else
          dontknow
        end;
      caption1('Video modes supported:');
      Writeln;
      xword2:=VESAinfo.modesSeg;
      xword3:=VESAinfo.modesOfs;
      with VESAmode do
        while MemW[xword2:xword3] <> $FFFF do
          begin
          xword1:=MemW[xword2:xword3];
          AX:=$4F01;
          CX:=xword1;
          ES:=Seg(VESAmode);
          DI:=Ofs(VESAmode);
          Intr($10, regs);
          if (AX = $004F) and (modeattr and 1 = 1) then
            begin
            pause3(4);
            if endit then
              Exit;
            caption2('Number');
            Write(hex(xword1, 4));
            caption3('Mode');
            if modeattr and 8 = 8 then
              Write('Color ')
            else
              Write('Monochrome ');
            if modeattr and $10 = $10 then
              Write('graphics')
            else
              Write('text');
            caption3('BIOS output support');
            yesorno(modeattr and 4 = 4);
            if modeattr and 2 = 2 then
              begin
              caption3('Screen size');
              Write(pixwidth, 'x', pixheight);
              caption3('Character size');
              Write(charwidth, 'x', charheight);
              caption3('Colors');
              Writeln(exp((pixelbits * 1.0) * ln(2.0)):1:0);
              caption3('Memory model');
              case memmodel of
                0: Write('Text');
                1: Write('CGA');
                2: Write('Hercules');
                3: Write('4-plane');
                4: Write('Packed-pixel');
                5: Write('Nonchain 4');
                6..$0F: Write('(reserved)');
              else
                Write('(unknown)')
              end;
              caption3('Memory planes');
              Write(memplanes);
              caption3('Memory banks');
              Write(banks);
              if banks > 1 then
                begin
                caption3('Bank size');
                Write(banksize, 'K')
                end;
              Writeln;
              end;
            Writeln;
            Inc(xword3, 2);
            end;
          end;
      TextColor(LightGreen);
      Write('The next screen will show standard information, so ');
      pause1;
      if endit then
        Exit;
      ClrScr;
      caption2('Display adapter');
      end;
    end;
  vgacard:=none;
  case graphdriver of
    CGA : begin
          writeln('CGA');
          captfont
          end;
    MCGA : begin
           writeln('MCGA');
           captfont;
           showfont($01);
           showfont($03);
           showfont($04);
           showfont($06)
           end;
    EGA..EGAmono : begin
                 writeln('EGA');
                 captfont;
                 showfont($01);
                 showfont($02);
                 showfont($03);
                 showfont($04);
                 showfont($05);
                 int101210;
                 xbyte:=mem[BIOSdseg : $0087];
                 caption2('Mode change preserves screen buffer');
                 yesorno(xbyte and $80 = $80);
                 caption2('EGA active');
                 yesorno(xbyte and $08 = $00);
                 caption2('Wait for display enable');
                 yesorno(xbyte and $04 = $04);
                 caption2('CGA cursor emulation');
                 yesorno(xbyte and $01 = $00);
           (*  PC Magazine 6:12 p.326  *)
                 caption2('Save area                    ');
                 xword1:=memw[BIOSdseg : $00AA];
                 xword2:=memw[BIOSdseg : $00A8];
                 segofs(xword1, xword2);
                 writeln;
           (*  PC Tech Journal 3:4 p.65  *)
                 caption2('Video parameter table        ');
                 segofs(memw[xword1 : xword2 +  2], memw[xword1 : xword2]);
                 writeln;
                 caption2('Dynamic save area            ');
                 xword3:=memw[xword1 : xword2 +  6];
                 xword4:=memw[xword1 : xword2 +  4];
                 if (xword3 > $0000) or (xword4 > $0000) then
                   begin
                   segofs(xword3, xword4);
                   writeln
                   end
                 else
                   writeln('(none)');
                 caption2('Auxiliary character generator');
                 xword3:=memw[xword1 : xword2 + 10];
                 xword4:=memw[xword1 : xword2 +  8];
                 if (xword3 > $0000) or (xword4 > $0000) then
                   begin
                   segofs(xword3, xword4);
                   writeln
                   end
                 else
                   writeln('(none)');
                 caption2('Graphics mode auxiliary table');
                 xword3:=memw[xword1 : xword2 + 14];
                 xword4:=memw[xword1 : xword2 + 12];
                 if (xword3 > $0000) or (xword4 > $0000) then
                   segofs(xword3, xword4)
                 else
                   write('(none)')
           (*  PC Tech Journal 3:4 p.67  *)
               end;
    hercmono : begin
               writeln('Hercules or MDA');
               captfont
               end;
    IBM8514 : begin
              writeln('IBM 8514');
              captfont
              end;
    ATT400 : begin
             writeln('AT&T 400');
             captfont
             end;
    VGA : begin
          writeln('VGA');
          vidmem:=256;
          caption3('Chipset');
          vgacard:=standard;
          with regs do
            begin
            s:=readROM($C000, $7D, 4);
            if s = 'VGA=' then
              begin
              vgacard:=paradise;
              Write('Paradise');
              vidmem:=word(64) * CH;
              caption3('Frequencies are');
              AX:=$7F;
              BH:=$1F;
              Intr($10, regs);
              if (BL and $80) = $80 then
                Writeln('multi-sync')
              else
                Writeln('fixed-sync')
              end;
            end;
          if vgacard = standard then
            with regs do
              begin
              AX:=$6F00;
              BX:=0;
              Intr($10, regs);
              if BX = $5637 then
                begin
                vgacard:=video7;
                Write('Video 7');
                AX:=$6F07;
                Intr($10, regs);
                if AL = $6F then
                  begin
                  vidmem:=256 * (AH and $7F);
                  caption3('memory type');
                  if AH and $80 = $80 then
                    Writeln('VRAM')
                  else
                    Writeln('DRAM')
                  end
                else
                  vidmem:=256;
                end;
              end;
          if vgacard = standard then
            with regs do
              begin
              s:=readROM($C000, $31, 9);
              if s = '761295520' then
                begin
                vgacard:=ati;
                Write('ATI');
                caption3('Board');
                s:=readROM($C000, $40, 2);
                if s = '31' then
                  Write('VGAWonder');
                if s = '32' then
                  Write('EGAWonder 800+');
                caption3('Revision');
                Write(Mem[$C000:$4C], decimal, Mem[$C000:$4D]);
                caption3('gate version');
                c:=Chr(Mem[$C000:$43]);
                if c = '3' then
                  Writeln('2 w/ VRAM')
                else
                  Writeln(c);
                xbyte:=Mem[$C000:$42];
                caption3('mouse port');
                yesorno2(xbyte and 2 = 2);
                caption3('microchannel');
                yesorno2(xbyte and 8 = 8);
                caption3('non-interlaced available');
                yesorno(xbyte and 4 = 4);
                xword1:=MemW[$C000:$10];
                xbyte:=ATIinfo($BB, xword1);
                caption3('monitor');
                Writeln(atividmons[xbyte and $0F]);
                xbyte:=xbyte and $20;
                if xbyte = $20 then
                  vidmem:=512
                else
                  vidmem:=256;
                end;
              end;
          if vgacard = standard then
            begin
            s:=readROM($C000, $25, 5);
            if s = 'AHEAD' then
              begin
              Writeln('Ahead');
              vgacard:=ahead;
              end
            end;
          if (vgacard = standard) and (CirrusCK = 1) then
            begin
            Write('Cirrus');
            vgacard:=cirrus;
            end;
          if (vgacard = standard) and (CTICK <> 0) then
            with regs do
              begin
              Write('CTI 82C45', CTICK);
              AH:=$5F;
              AL:=0;
              Intr($10, regs);
              if AL = $5F then
                begin
                vgacard:=CTI;
                vidmem:=(BH * 256) + 256;
                caption3('micro-channel');
                yesorno2(CX and 2 = 2);
                caption3('DAC size');
                if CX and 1 = 1 then
                  Writeln('8-bit')
                else
                  Writeln('6-bit');
                end
              end;
          if vgacard = standard then
            begin
            xword2:=MemW[$C000:$37];
            xword1:=MemW[$C000:$39];
            s:=readROM(xword1, xword2, 4);
            if s = #$77#$11#$99#$66 then
              begin
              Writeln('Genoa');
              vgacard:=genoa
              end
            end;
          if vgacard = standard then
            with regs do
              begin
              AX:=$7000;
              BX:=0;
              Intr($10, regs);
              if AL = $70 then
                begin
                Write('Trident');
                vgacard:=trident;
                caption3('Card');
                DX:=(DX and $FFF0) shr 4;
                case DX of
                  $678: Write('Viewpoint');
                  $236: Write('Ultragraphics II');
                  $620: Write('Vision VGA');
                  $673: Write('EVGA')
                else
                  dontknow2;
                end; {case}
                vidmem:=((CH shr 6) * 256) + 256;
                caption3('Monitor');
                if CL < 8 then
                  Writeln(trividmons[CL])
                else
                  Writeln('(unknown) - ', CL);
                end
              end;
          if (vgacard = standard) and (tsengCK = 1) then
            begin
            Writeln('Tseng');
            vgacard:=tseng
            end;
          if (vgacard = standard) and (zymosCK = 1) then
            begin
            Writeln('ZyMOS');
            vgacard:=zymos
            end;
          if vgacard = standard then
            dontknow;
          captfont;
          showfont($01);
          showfont($02);
          showfont($03);
          showfont($04);
          showfont($05);
          showfont($06);
          showfont($07);
          int101210;
          with regs do
            begin
            AX:=$1009;
            ES:=seg(VGAbuf);
            DX:=ofs(VGAbuf);
            intr($10, regs)
            end;
          caption2('Palette registers');
          for i:=$00 to $0F do
            write(hex(VGAbuf[i], 2), ' ');
          writeln;
          caption2('Border color');
          writeln(hex(VGAbuf[$10], 2));
          caption2('Color page');
          with regs do
            begin
            AX:=$101A;
            intr($10, regs);
            writeln('$', hex(BH, 2));
            caption2('Paging mode');
            case BL of
              $00 : writeln('4 pages of 64 registers');
              $01 : writeln('16 pages of 16 registers')
              else
                unknown('mode', BL, 2)
            end
            end
        end;
    PC3270 : begin
             writeln('3270 PC');
             captfont
             end
  else
    unknown('adapter', graphdriver, 4)
  end {case}
  end; {page_06}
