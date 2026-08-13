unit page_06;

interface

uses crt, dos, graph, ifpglobl, ifpcomon, ifpextrn;

procedure page06;

implementation

procedure page06;
const
  atividmons: array[0..15] of string[24] =
               ('EGA', 'analog monochrome', 'TTL monochrome', 'analog color',
                'RGB color', 'Multisync or compatible', '(unknown)',
                'PS/2 8514 or compatible', 'Seiko 1430', 'MultiSync 2A',
                'Tatung OmniScan', 'NEC 3D or compatible', 'TVM 3M',
                'NEC MultiSync XL/+/4D/5D', 'TVM 2A', 'TVM 3A');
  trividmons: array[0..7] of string[17] =
               ('MDA', 'CGA', 'EGA', 'Digital multisync', 'VGA', '8514',
                'SuperVGA', 'Analog multisync');
  parachips: array[1..4] of string[7] =
               ('PVGA1A', 'WD90C00', 'WD90C10', 'WD90C11');


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
                imagepages: byte;
                reserved0a: byte;
                {The following is part of VESA 1.2 and newer}
                RedMaskSize: byte;
                RedFieldPos: byte;
                GrnMaskSize: byte;
                GrnFieldPos: byte;
                BluMaskSize: byte;
                BluFieldPos: byte;
                RsrvdMaskSize: byte;
                RsrvdMaskPos: byte;
                DirectColorInfo: byte;
                Reserved0b: byte;
                reserved: array[0..215] of byte;
              end;

ATIvidtype = record
               columns: byte;
               maxrow: byte;
               lines_a_row: byte;
               buffer_size: word;
               seqreg: array[1..4] of byte;
               miscreg: byte;
               crtsreg: array[0..$18] of byte;
               attrreg: array[0..$13] of byte;
               graphreg: array[0..8] of byte;
             end;

var
  i : byte;
  VGAbuf : array[$00..$10] of byte;
  VESAinfo: VESAitype;
  VESAmode: VESAmtype;
  xbyte, xbyte2, xbyte3, paralock1, paralock2: byte;
  xword1 : word;
  xword2 : word;
  xword3 : word;
  xword4 : word;
  vgacard: cardtype;
  vidmem : word;
  s: string;
  c: char;
  saveattr, savex, savey: byte;
  foundone, foundit: boolean;

procedure captfont;
  begin
  caption1('Font           Address');
  Writeln;
  Write('INT 1FH        ');
  segofs(longint(intvec[$1F]) shr 16, longint(intvec[$1F]) and $0000FFFF);
  Writeln
  end; {captfont}

procedure showfont(a : byte);
  const
    fontnames: array [0..7] of string[12] = (
                 'INT 1FH     ',
                 'INT 43H     ',
                 'ROM 8x14    ',
                 'ROM 8x8 (lo)',
                 'ROM 8x8 (hi)',
                 'ROM 9x14    ',
                 'ROM 8x16    ',
                 'ROM 9x16    ');

  begin
  with regs do
    begin
    Write(fontnames[a], '   ');
    AX:=$1130;
    BH:=a;
    intr($10, regs);
    segofs(ES, BP);
    Writeln
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
      $00 : Writeln('color');
      $01 : Writeln('monochrome')
      else
        unknown('display', BH, 2)
    end;
    caption2('Memory');
    if vidmem > 0 then
      Writeln(vidmem, 'K')
    else
      if BL < 4 then
        Writeln(memnames[BL], ' as determined from standard BIOS call')
      else
        unknown('size', BL, 2);
    if vgacard = none then
      begin
      caption2('Feature bits');
      Writeln(bin4(CH and $0F));
      caption2('DIP switches (EGA)');
      Writeln(bin4(CL and $0F))
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

  procedure checking(s: string);
    var
      x, y: byte;
    begin
    x:=WhereX;
    y:=WhereY;
    ClrEol;
    Write('Checking for ', s);
    GotoXY(x, y);
    end; {checking}

  procedure d8or16bit(b: boolean);
    begin
    if b then
      Write('8-bit')
    else
      Write('16-bit');
    end;

  procedure cli;
    inline($FA);

  procedure sti;
    inline($FB);

  function isXGA: word;
    var
      POSport, cardID, tmpw: word;
      tmp, tmp1, tmp2, tmp3, tmp4: byte;
      slot: byte;
      regs: registers;
      foundit: boolean;

    begin
    isXGA:=0;
    foundit:=false;
    with regs do
      begin
      DX:=$FFFF;
      AX:=$C400;
      Intr($15, regs);
      if (not nocarry(regs)) or (DX = -1) then
        Exit;
      posport:=DX;
      slot:=0;
      repeat
        cli;
        if slot = 0 then
          Port[$94]:=$DF
        else
          begin
          AX:=$C401;
          BX:=slot;
          Intr($15, regs)
          end;
        cardID:=PortW[POSport];
        tmp1:=Port[POSport + 2];
        tmp2:=Port[POSport + 3];
        tmp3:=Port[POSport + 4];
        tmp4:=Port[POSport + 5];
        if slot = 0 then
          Port[$94]:=$FF
        else
          begin
          AX:=$C402;
          BX:=slot;
          Intr($15, regs);
          end;
        cli;
        if (cardID >= $8FD8) and (cardID <= $8FDB) then
          begin
          tmpw:=tmp1 and $E;
          POSport:=(tmpw shl 3) + $2100;
          Port[POSport + $A]:=$52;
          tmp:=Port[POSport + $B] and $F;
          if (tmp <> 0) and (tmp <> $F) then
            foundit:=true
          else
            Inc(slot);
          end
        else
          Inc(slot);
      until foundit or (slot > 9);
      end;
    if foundit then
      isXGA:=POSport;
    end; {isXGA}

  procedure isport2(var regs: registers; var foundit: boolean);
    var
      savebx, saveax: word;
      tmp: byte;

    begin
    with regs do
      begin
      savebx:=BX;
      BX:=AX;
      Port[DX]:=AL;
      AH:=AL;
      AL:=Port[DX + 1];
      tmp:=AH;
      AH:=AL;
      AL:=tmp;
      saveax:=AX;
      AX:=BX;
      PortW[DX]:=AX;
      Port[DX]:=AL;
      AH:=AL;
      AL:=Port[DX + 1];
      AL:=AL and BH;
      foundit:=(AL = BH);
      if AL = BH then
        begin
        AL:=AH;
        AH:=0;
        Port[DX]:=AX;
        Port[DX]:=AL;
        AH:=AL;
        AL:=Port[DX + 1];
        AL:=AL and BH;
        foundit:=(AL = 0);
        end;
      AX:=saveax;
      PortW[DX]:=AX;
      BX:=savebx;
      end;
    end;

  begin (* procedure page_06 *)
  vgacard:=none;
  caption2('Display adapter');
  checking('VESA');
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
        ClrEol;
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
          Writeln(s)
        end;
      caption1('Video modes supported:');
      Writeln;
      xword2:=VESAinfo.modesSeg;
      xword3:=VESAinfo.modesOfs;
      with VESAmode do
        while MemW[xword2:xword3] <> $FFFF do
          begin
          xword1:=MemW[xword2:xword3];
          pause3(3);
          if endit then
            Exit;
          caption2('Number');
          Write(hex(xword1, 4));
          AX:=$4F01;
          CX:=xword1;
          ES:=Seg(VESAmode);
          DI:=Ofs(VESAmode);
          Intr($10, regs);
          if (AX = $004F) and (modeattr and 1 = 1) then
            begin
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
                6: Write('direct color');
                7: Write('YUV');
                8..$0F: Write('(reserved)');
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
              end
            end
          else
            Caption1('  No available information on this mode.');
          Inc(xword3, 2);
          Writeln;
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
  vidmem:=0;
  ClrEol;
  case graphdriver of
    CGA : begin
          Writeln('CGA');
          captfont
          end;
    MCGA : begin
           Writeln('MCGA');
           captfont;
           showfont($01);
           showfont($03);
           showfont($04);
           showfont($06)
           end;
    EGA..EGAmono : begin
                 Writeln('EGA');
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
                 Writeln;
           (*  PC Tech Journal 3:4 p.65  *)
                 caption2('Video parameter table        ');
                 segofs(memw[xword1 : xword2 +  2], memw[xword1 : xword2]);
                 Writeln;
                 caption2('Dynamic save area            ');
                 xword3:=memw[xword1 : xword2 +  6];
                 xword4:=memw[xword1 : xword2 +  4];
                 if (xword3 > $0000) or (xword4 > $0000) then
                   begin
                   segofs(xword3, xword4);
                   Writeln
                   end
                 else
                   Writeln('(none)');
                 caption2('Auxiliary character generator');
                 xword3:=memw[xword1 : xword2 + 10];
                 xword4:=memw[xword1 : xword2 +  8];
                 if (xword3 > $0000) or (xword4 > $0000) then
                   begin
                   segofs(xword3, xword4);
                   Writeln
                   end
                 else
                   Writeln('(none)');
                 caption2('Graphics mode auxiliary table');
                 xword3:=memw[xword1 : xword2 + 14];
                 xword4:=memw[xword1 : xword2 + 12];
                 if (xword3 > $0000) or (xword4 > $0000) then
                   segofs(xword3, xword4)
                 else
                   Write('(none)')
           (*  PC Tech Journal 3:4 p.67  *)
               end;
    hercmono : begin
               Writeln('Hercules or MDA');
               captfont
               end;
    IBM8514 : begin
              Writeln('IBM 8514');
              captfont
              end;
    ATT400 : begin
             Writeln('AT&T 400');
             captfont
             end;
    VGA : begin
          if novgacheck then
            begin
            Writeln('VGA');
            caption3('Chipset');
            Writeln('Detection blocked by NV command-line switch!');
            end
          else
            begin
            xword1:=isXGA;
            if xword1 > 0 then
              if Port[xword1] and 1 = 1 then
                Writeln('XGA')
              else
                Writeln('VGA, XGA on other monitor')
            else
              Writeln('VGA');
            vidmem:=0;
            caption3('Chipset');
            vgacard:=standard;
            if vgacard = standard then
              begin
              checking('Video 7');
              if Port[$3CC] and 1 = 1 then
                xword1:=$3D0
              else
                xword1:=$3B0;
              Port[xword1 + 4]:=$C;
              i:=Port[xword1 + 5];
              Port[xword1 + 5]:=$55;
              xbyte:=Port[xword1 + 5];
              Port[xword1 + 4]:=$1F;
              xbyte2:=Port[xword1 + 5];
              Port[xword1 + 4]:=$C;
              Port[xword1 + 5]:=i;
              if xbyte2 = $55 xor $EA then
                begin
                vgacard:=video7;
                ClrEol;
                Write('Video 7 - ');
                port[$3C4]:=$8E;
                xbyte:=Port[$3C5];
                case xbyte of
                  $80..$FF: Writeln('Vega VGA');
                  $70..$7F: with regs do
                              begin
                              AX:=$6F07;
                              Intr($10, regs);
                              if (AH and $80) = $80 then
                                Writeln('VRAM')
                              else
                                Writeln('FastWrite');
                              end;
                  $50..$59: Writeln('VGA Version 5');
                  $40..$49: Writeln('1024i')
                else
                  Writeln('unknown value $', hex(xbyte, 2));
                end;
                caption3('  Chip revision');
                Port[$3C4]:=$8E;
                Write(Port[$3C5]);
                Port[$3C4]:=$FF;
                xbyte:=Port[$3C5];
                with regs do
                  begin
                  AX:=$6F07;
                  Intr($10, regs);
                  if AL = $6F then
                    begin
                    vidmem:=256 * (AH and $7F);
                    caption3('Memory type');
                    if AH and $80 = $80 then
                      Writeln('VRAM')
                    else
                      Writeln('DRAM')
                    end
                  else
                    vidmem:=256;
                  end;
                caption3('  Memory');
                d8or16bit((xbyte and 1) = 0);
                caption3('I/O');
                d8or16bit((xbyte and 2) = 0);
                caption3('BIOS');
                d8or16bit((xbyte and 8) = 0);
                caption3('bus');
                caption3('Fast-Write');
                yesorno((xbyte and 4) = 4);
                end;
              end;
            if vgacard = standard then
              begin
              checking('AHEAD');
              s:=readROM($C000, $25, 5);
              if s = 'AHEAD' then
                begin
                vgacard:=ahead;
                ClrEol;
                Writeln('Ahead');
                end
              end;
            if vgacard = standard then
              begin
              checking('Genoa');
              s:=readROM($C000, MemW[$C000:$37], 4);
              if (s[1] = #$77) and (Copy(s, 3, 2) = #$99#$66) then
                begin
                vgacard:=genoa;
                ClrEol;
                Write('Genoa ');
                case Ord(s[2]) of
                  $33: Write('5100/5200 (Tseng ET3000 based)');
                  $55: Write('5300/5400 (Tseng ET3000 based)');
                  $22: Write('6100');
                  $00: Write('6200/6300');
                  $11: Write('6400/6600')
                else
                  Write('(unknown type - $', hex(Ord(s[2]), 2), ')');
                end;
                Writeln;
                if (s[2] = #$33) or (s[2] = #$55) then
                  begin
                  end
                else
                  begin
                  Port[$3C4]:=5;
                  xbyte:=Port[$3C5];
                  caption3('  Bus');
                  if (xbyte and 1) = 1 then
                    Write('PC')
                  else
                    Write('MCA');
                  caption3('Video width');
                  d8or16bit((xbyte and 2) = 2);
                  caption3('BIOS width');
                  d8or16bit((xbyte and 4) = 4);
                  caption3('I/O ports at');
                  if (xbyte and $10) = $10 then
                    Writeln('$3xx')
                  else
                    Writeln('$2xx');
                  caption3('  BIOS size');
                  case (xbyte and $60) shr 5 of
                    0, 3: Write('24K');
                    1: Write('30K');
                    2: Write('32K')
                  end;
                  Port[$3C4]:=7;
                  xbyte:=Port[$3C5];
                  caption3('Monitor type');
                  if (xbyte and $20) = $20 then
                    Write('TTL digital')
                  else
                    Write('analog');
                  caption3('Chipset on');
                  if (xbyte and 8) = 8 then
                    Writeln('motherboard')
                  else
                    Writeln('adapter card');
                  Port[$3C4]:=$10;
                  xbyte:=Port[$3C5];
                  caption3('  Fast scroll');
                  yesorno2((xbyte and 1) = 1);
                  caption3('Fast address');
                  yesorno2((xbyte and 2) = 2);
                  caption3('Fast write');
                  yesorno((xbyte and $40) = $40);
                  Port[$3C4]:=8;
                  xbyte:=Port[$3C5];
                  caption3('  70Hz vertical retrace');
                  yesorno2((xbyte and $10) = $10);
                  xword1:=MemW[$40:$63];
                  Port[xword1]:=$2F;
                  xbyte:=Port[xword1 + 1];
                  caption3('Interlaced');
                  yesorno((xbyte and 1) = 1);
                  end
                end
              end;
            if vgacard = standard then
              begin
              checking('Cirrus');
              xword1:=CirrusCK;
              if xword1 <> 0 then
                begin
                vgacard:=cirrus;
                ClrEol;
                Write('Cirrus');
                caption3('chipset type');
                case xword1 of
                  $EC: Write('510/520');
                  $CA: Write('610/620');
                  $EA: Write('Video Seven')
                else
                  Write('unknown - $', hex(xword1, 2))
                end;
                caption3('Cirrus BIOS');
                s:=readROM($C000, 6, 2);
                yesorno(s = 'CL');
                end;
              end;
            if vgacard = standard then
              begin
              checking('CTI');
              Port[$46E8]:=$1E;
              xbyte:=Port[$104];
              Port[$46E8]:=$E;
              if xbyte = $A5 then
                with regs do
                  begin
                  AH:=$5F;
                  AL:=0;
                  Intr($10, regs);
                  ClrEol;
                  Write('CTI 82C45');
                  Port[$46E8]:=$1E;
                  Port[$103]:=$80;
                  Port[$46E8]:=$E;
                  Port[$3D6]:=0;
                  xbyte:=Port[$3D7];
                  xbyte3:=(xbyte and $F0) shr 4;
                  if xbyte3 = 1 then
                    begin
                    Port[$3D6]:=$3A;
                    i:=Port[$3D7];
                    Port[$3D7]:=$AA;
                    xbyte2:=Port[$3D7];
                    Port[$3D7]:=i;
                    if xbyte2 = $AA then
                      xbyte3:=2
                    else
                      xbyte3:=1;
                    end;
                  Port[$46E8]:=$1E;
                  Port[$103]:=0;
                  Port[$46E8]:=$E;
                  Port[$3D6]:=0;
                  Str(xbyte3:0, s);
                  Writeln(s);
                  vgacard:=CTI;
                  case BH of
                    0: vidmem:=256;
                    1: vidmem:=512;
                    2: vidmem:=1024;
                  else
                    vidmem:=0;
                  end;
                  caption3('  Chip revision');
                  Writeln(xbyte and $0F);
                  caption3('micro-channel');
                  yesorno2(CX and 2 = 2);
                  caption3('DAC size');
                  if CX and 1 = 1 then
                    Writeln('8-bit')
                  else
                    Writeln('6-bit');
                  end;
              end;
            if vgacard = standard then
              begin
              checking('Trident');
              Port[$3C4]:=$0B;
              xbyte:=Port[$3C5];
              if (xbyte <= 6) and (xbyte >= 2) then
  {
                Port[$3C5]:=Port[$3C5];
                Port[$3C4]:=$0E;
                i:=Port[$3C5];
                Port[$3C5]:=0;
                xbyte:=Port[$3C5] and $0F;
                Port[$3C5]:=i;
                if xbyte = 2 then
  }
                begin
                vgacard:=trident;
                ClrEol;
                Write('Trident ');
  {
                Port[$3C4]:=$0B;
                Port[$3C5]:=0;
                xbyte3:=Port[$3C5];
  }
                case xbyte of
                  1: Writeln('8800BR');
                  2: Writeln('8800CS')
                else
                  begin
                  Writeln('8900');
                  Port[$3C4]:=$1F;
                  xbyte:=Port[$3C5] and 3;
                  case xbyte of
                    0: vidmem:=256;
                    1: vidmem:=512;
                    2, 3: vidmem:=1024;
                  end; {case}
                  end;
                end;
                caption3('  BIOS');
                Port[$3C4]:=$F;
                xbyte:=Port[$3C5];
                d8or16bit((xbyte and $80) = 0);
                caption3('interlaced');
                Port[$3C4]:=$1E;
                xbyte:=Port[$3C5];
                yesorno((xbyte and $20) = $20);
                with regs do
                  begin
                  AX:=$7000;
                  BX:=0;
                  Intr($10, regs);
                  if AL = $70 then
                    begin
                    caption3('Everex Card');
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
                end;
              end;
            if vgacard = standard then
              begin
              checking('Tseng');
              xbyte:=tsengCK;
              if xbyte = 1 then
                begin
                ClrEol;
                Write('Tseng ET');
                if Port[$3CC] and 1 = 1 then
                  xword:=$3D0
                else
                  xword:=$3B0;
                Port[xword + 4]:=$33;
                xbyte:=Port[xword + 5];
                Port[xword + 5]:=xbyte xor $F;
                xbyte2:=Port[xword + 5];
                Port[xword + 5]:=xbyte;
                if xbyte2 = xbyte xor $F then
                  begin
                  Write('4000');
                  Port[$3BF]:=3;
                  Port[$3D8]:=$A0;
                  with regs do
                    begin
                    AX:=$10F1;
                    BL:=0;
                    Intr($10, regs);
                    if AL=$10 then
                      case BL of
                        0: Writeln(' w/ normal DAC');
                        1: Writeln(' w/ SC1148x HiColor DAC');
                        2: Writeln(' w/ new HiColor DAC')
                      else
                        Writeln(' w/ unknown HiColor DAC');
                      end;
                    end;
                  Port[xword + 4]:=$37;
                  xbyte:=Port[xword + 5];
                  if xbyte and 8 = 0 then
                    vidmem:=256
                  else
                    case xbyte and 3 of
                      0,1: vidmem:=256;
                      2: vidmem:=512;
                      3: vidmem:=1024;
                    end
                  end
                else
                  Write('3000');
                caption3('ROM');
                d8or16bit((xbyte and $10) = 0);
                caption3('Video');
                Port[xword + 4]:=$36;
                xbyte:=Port[xword + 5];
                d8or16bit((xbyte and $40) = 0);
                caption3('I/O');
                d8or16bit((xbyte and $80) = 0);
                Writeln;
                Port[xword + 4]:=$37;
                xbyte:=Port[xword + 5];
                caption3('Memory type');
                if xbyte and $80 = $80 then
                  Write('VRAM')
                else
                  Write('DRAM');
                Port[$3C4]:=7;
                xbyte:=Port[$3C5];
                caption3('Compatibility');
                if xbyte and $80 = $80 then
                  Write('VGA')
                else
                  Write('EGA');
                caption3('ROM address');
                if xbyte and $20 = 0 then
                  if xbyte and 8 = 0 then
                    Writeln('C000-C3FF')
                  else
                    Writeln('disabled')
                else
                  if xbyte and 8 = 0 then
                    Writeln('C000-C5FF and C680 - C7FF')
                  else
                    Writeln('C000-C7FF');
                vgacard:=tseng
                end;
              end;
            if vgacard = standard then
              begin
              checking('ZyMOS');
              if zymosCK = 2 then
                begin
                ClrEol;
                Writeln('ZyMOS');
                vgacard:=zymos
                end;
              end;
            if vgacard = standard then
              begin
              checking('Oak');
              with regs do
                begin
                DX:=$3DE;
                AX:=$FF11;
                isport2(regs, foundit);
                if foundit then
                  begin
                  ClrEol;
                  Writeln('Oak');
                  Port[$3DE]:=$D;
                    asm
                       jmp  @1
                    @1:
                    end;
                  xbyte:=Port[$3DF];
                  if xbyte and $80 = $80 then
                    vidmem:=512
                  else
                    vidmem:=256;
                  end;
                end;
              end;
            if vgacard = standard then
              with regs do
                begin
                checking('ATI');
                s:=readROM($C000, $31, 9);
                if s = '761295520' then
                  begin
                  vgacard:=ati;
                  ClrEol;
                  Write('ATI ');
                  c:=Chr(Mem[$C000:$43]);
                  case c of
                    '1': Write('18800');
                    '2': Write('18800-1');
                    '3': Write('28800-2');
                    '4': Write('28800-4');
                    '5': Write('28800-5');
                    'a': Write('68875')
                   else
                     Write('"', c, '"???')
                  end;
                  caption3('Board');
                  s:=readROM($C000, $40, 2);
                  if s = '31' then
                    if c = 'a' then
                      Write('Ultra Pro')
                    else
                      Write('VGAWonder');
                  if s = '32' then
                    Write('EGAWonder 800+');
                  caption3('Revision');
                  Write(Mem[$C000:$4D], decimal);
                  ZeroPad(Mem[$C000:$4C]);
                  xbyte:=Mem[$C000:$42];
                  caption3('mouse port');
                  yesorno(xbyte and 2 = 2);
                  caption3('programmable video clock');
                  yesorno2(xbyte and $10 = $10);
                  xword1:=MemW[$C000:$10];
                  xbyte:=ATIinfo($BB, xword1);
                  caption3('monitor');
                  Writeln(atividmons[xbyte and $0F]);
                  if c > '0' then
                    begin
                    xbyte:=Mem[$C000:$44];
                    caption3('70Hz non-interlace');
                    yesorno2(xbyte and 1 = 1);
                    caption3('Korean chars');
                    yesorno2(xbyte and 2 = 2);
                    caption3('Memory clock');
                    if (xbyte and 4 = 4) then
                      Writeln('45MHz')
                    else
                      Writeln('40MHz');
                    caption3('Zero wait state');
                    yesorno2(xbyte and 8 = 8);
                    caption3('Paged ROMs');
                    yesorno2(xbyte and $10 = $10);
                    caption3('8514/A');
                    yesorno2(xbyte and $40 <> $40);
                    caption3('HiColor DAC');
                    yesorno(xbyte and $80 = $80);
                    end;
                  caption3('Video modes');
                  xbyte2:=0;
                  foundone:=false;
                  for xbyte:=0 to 255 do
                    begin
                    AH:=$12;
                    AL:=xbyte;
                    BX:=$5506;
                    BP:=$FFFF;
                    Intr($10, regs);
                    if BP <> $FFFF then
                      begin
                      Write(hex(xbyte, 2), ' ');
                      Inc(xbyte2);
                      foundone:=true;
                      if xbyte2 = 21 then
                        begin
                        Writeln;
                        Write('               ');
                        xbyte2:=0;
                        end;
                      end;
                    end;
                  if (not foundone) or (foundone and (xbyte <> 0)) then
                    Writeln;
                  vidmem:=256;
                  if (c = '1') or (c = '2') then
                    begin
                    if ATIinfo($BB, xword1) and $20 = $20 then
                      vidmem:=512;
                    end
                  else
                    begin
                    xbyte:=ATIinfo($B0, xword1);
                    if xbyte and $10 = $10 then
                      vidmem:=512;
                    if c <> '3' then
                      if xbyte and 8 = 8 then
                        vidmem:=1024;
                    end;
                  end;
                end;
            if vgacard = standard then
              begin
              checking('Paradise');
              s:=readROM($C000, $7D, 4);
              if s = 'VGA=' then
                begin
                vgacard:=paradise;
                ClrEol;
                Write('Western Digital/Paradise ');
                Port[$3CE]:=$F;
                paralock1:=Port[$3CF];
                Port[$3CF]:=5;
                Port[$3C4]:=7;
                if Port[$3CC] and 1 = 1 then
                  xword:=$3D0
                else
                  xword:=$3B0;
                Port[xword + 4]:=$29;
                paralock2:=Port[xword + 5];
                Port[xword + 5]:=$85;
                PortW[$3C4]:=$4806;
                Port[xword + 4]:=$2B;
                xbyte:=Port[xword + 5];
                Port[xword + 5]:=$AA;
                xbyte2:=Port[xword + 5];
                Port[xword + 5]:=xbyte;
                if xbyte2 <> $AA then
                  xbyte3:=1
                else
                  begin
                  Port[$3C4]:=$12;
                  xbyte:=Port[$3C5];
                  Port[$3C5]:=xbyte and $BF;
                  xbyte2:=Port[$3C5] and $40;
                  if xbyte2 <> 0 then
                    begin
                    Port[$3C5]:=xbyte;
                    xbyte3:=2;
                    end
                  else
                    begin
                    Port[$3C5]:=xbyte or $40;
                    xbyte2:=Port[$3C] and $40;
                    if xbyte2 = 0 then
                      begin
                      Port[$3C5]:=xbyte;
                      xbyte3:=2;
                      end
                    else
                      begin
                      Port[$3C5]:=xbyte;
                      xbyte3:=4;
                      Port[$3C4]:=$10;
                      xbyte:=Port[$3C5];
                      Port[$3C5]:=xbyte and $FB;
                      xbyte2:=Port[$3C5] and 4;
                      if xbyte2 <> 0 then
                        xbyte3:=3;
                      Port[$3C5]:=xbyte or 4;
                      xbyte2:=Port[$3C5] and 4;
                      if xbyte2 = 0 then
                        xbyte3:=3;
                      Port[$3C5]:=xbyte;
                      end
                    end
                  end;
                Writeln(parachips[xbyte3]);
                Port[$3CE]:=$0B;
                for xbyte:=1 to 2 do;
                xbyte:=Port[$3CF];
                vidmem:=word(64) * (xbyte shr 4);
                caption3('Video');
                d8or16bit((xbyte and 4) = 0);
                caption3('ROM');
                d8or16bit((xbyte and 2) = 0);
                caption3('Frequencies are');
                Port[$3CE]:=$0F;
                xbyte:=Port[$3CF];
                if (xbyte and $80) = $80 then
                  Writeln('multi-sync')
                else
                  Writeln('fixed-sync');
                Port[xword + 4]:=$29;
                Port[xword + 5]:=paralock2;
                Port[$3CE]:=$F;
                Port[$3CF]:=paralock1;
                end;
              end;
            if vgacard = standard then
              begin
              ClrEol;
              dontknow;
              end;
            end;
          captfont;
          showfont($01);
          showfont($02);
          showfont($03);
          showfont($04);
          showfont($05);
          showfont($06);
          showfont($07);
          int101210;
          saveattr:=TextAttr;
          savex:=WhereX;
          savey:=WhereY;
          TextColor(LightRed + Blink);
          Write('**Retrieving palette information**');
          with regs do
            begin
            AX:=$1009;
            ES:=seg(VGAbuf);
            DX:=ofs(VGAbuf);
            intr($10, regs)
            end;
          GotoXY(savex, savey);
          Write('                                  ');
          GotoXY(savex, savey);
          TextAttr:=saveattr;
          caption2('Palette registers');
          for i:=$00 to $0F do
            Write(hex(VGAbuf[i], 2), ' ');
          Writeln;
          caption2('Border color');
          Write(hex(VGAbuf[$10], 2));
          caption3('Color page');
          with regs do
            begin
            AX:=$101A;
            intr($10, regs);
            Write('$', hex(BH, 2));
            caption3('Paging mode');
            case BL of
              $00 : Writeln('4 pages of 64 registers');
              $01 : Writeln('16 pages of 16 registers')
              else
                unknown('mode', BL, 2)
            end
            end
        end;
    PC3270 : begin
             Writeln('3270 PC');
             captfont
             end
  else
    unknown('adapter', graphdriver, 4)
  end {case}
  end; {page06}
end.
