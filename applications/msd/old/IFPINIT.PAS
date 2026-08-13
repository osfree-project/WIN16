unit ifpinit;

interface

uses crt, graph, dos, ifpglobl, ifpcomon, IFPScrPt;

procedure init;

implementation

procedure init;
  var
    xint: integer;

  procedure rjustify(a : string);
    begin
    GotoXY(1 + lo(WindMax) - length(a), wherey);
    x2:=WhereX;
    Write(a)
    end; {rjustify}

  procedure border(ch: char);
    var
      i : byte;

    begin
    TextColor(LightCyan);
    for i:=1 to twidth do
      Write(ch);
    TextColor(LightGray);
    end; {border}

  procedure checkparams;
    var
      s: string;
      parm: word;

    function deletejunk(s: string): string;
      var
        x: byte;

      begin
      while (Length(s) > 0) and ((s[1] = ' ') or (s[1] = '-') or (s[1] = '/')) do
        s:=Copy(s, 2, 255);
      while (Length(s) > 0) and (s[Length(s)] = ' ') do
        Dec(s[0]);
      if Length(s) > 0 then
        for x:=1 to Length(s) do
          s[x]:=UpCase(s[x]);
      deletejunk:=s;
      end;

    begin {checkparams}
    for parm:=1 to ParamCount do
      begin
      s:=deletejunk(ParamStr(parm));
      if s = 'B' then
        DirectVideo:=false;
      if s = 'D' then
        DirectVideo:=true;
      if s = 'M' then
        mono:=true;
      if s = 'C' then
        mono:=false;
      if s = 'F' then
        FifoOn:=true;
      if s = 'NP' then
        ReadPartitionTable:=false;
      if s = 'NV' then
        novgacheck:=true;
      if Copy(s, 1, 2) = 'AP' then
        begin
        PrinterRec.Mode:='A';
        if s[3] = ':' then
          with PrinterRec do
            begin
            Destination:='F';
            Filename:=deletejunk(Copy(s, 4, 255));
            HiStrip:=true;
            HeaderStr:='';
            if tlength = 25 then
              ScreensPerPage:=2
            else
              ScreensPerPage:=1;
            ScreenCount:=0;
            end
        end;
      if (s = '?') or (s = 'H') then
        begin
        Assign(output, '');
        ReWrite(output);
        Assign(input, '');
        Reset(input);
        Writeln('INFOPLUS ', qversion, '  Compiled: ', qdate);
        Writeln;
        Writeln('Syntax:');
        Writeln('  INFOPLUS [B][D][M][C][F][NP][NV][AP[:filename]][H][?]');
        Writeln('where:');
        Writeln('  B  = Write to screen using BIOS (default under Desqview)');
        Writeln('  D  = Write directly to screen memory (default)');
        Writeln('  M  = Use monochrome colors (default under MODE BWxx)');
        Writeln('  C  = Use normal colors (default)');
        Writeln('  F  = Leave 16550 FIFO''s enabled');
        Writeln('  NV = Do not perform VGA chipset detection');
        Writeln('  NP = Do not read Partition Table');
        Writeln('  AP = AutoPrint all screens, ask for setup');
        Writeln('  AP:filename = AutoPrint to a file or device');
        Writeln('  H or ? = This help screen');
        Writeln;
        Writeln('ex: INFOPLUS B M NV');
        Writeln('      Run infoplus using BIOS, monochrome colors, no VGA check.');
        Writeln;
        Halt
        end;
      end;
    end;

  begin {init}
  mono:=false;
  vidmode:=LastMode;
  attrsave:=TextAttr;
  CheckBreak:=False;
  resetvideo:=false;
  novgacheck:=false;
  ReadPartitionTable:=true;
  FifoOn:=false;
  if (Lo(LastMode) = 0) or (Lo(LastMode) = 1) then
    begin
    TextMode(LastMode + 2);
    resetvideo:=true
    end;
  with regs do
    begin
    AH:=$0F;
    intr($10, regs);
    twidth:=AH;
    vidpg:=BH
    end;
  detectgraph(graphdriver, xint);
  if (graphdriver = EGA) or (graphdriver = MCGA) or (graphdriver = VGA) then
    with regs do
      begin
      AX:=$1130;
      BH:=$00;
      intr($10, regs);
      tlength:=DL + 1;
      CheckSnow:=False;
      end
  else
    tlength:=25;
  with regs do
    begin
    Intr($11, regs);
    equip:=AX;
    Intr($12, regs);
    DOSmem:=longint(AX) shl 10;
    AH:=$19;
    MsDos(regs);
    currdrv:=AL;
    AH:=$34;
    MsDos(regs);
    DOScseg:=ES;
    DOScofs:=BX
    end;
  SwapVectors;
  for i:=$00 to $FF do
    GetIntVec(i, intvec[i]);
  SwapVectors;
{
  intvec[$00]:=saveint00;
  intvec[$02]:=saveint02;
  intvec[$1B]:=saveint1B;
  intvec[$23]:=saveint23;
  intvec[$24]:=saveint24;
  intvec[$34]:=saveint34;
  intvec[$35]:=saveint35;
  intvec[$36]:=saveint36;
  intvec[$37]:=saveint37;
  intvec[$38]:=saveint38;
  intvec[$39]:=saveint39;
  intvec[$3A]:=saveint3A;
  intvec[$3B]:=saveint3B;
  intvec[$3C]:=saveint3C;
  intvec[$3D]:=saveint3D;
  intvec[$3E]:=saveint3E;
  intvec[$3F]:=saveint3F;
  intvec[$75]:=saveint75;
}
  with regs do
    begin
    AX:=$3700;
    MsDos(regs);
    switchar:=Chr(DL)
    end;
  dirsep:=['\'];
  if switchar <> '/' then
    dirsep:=dirsep + ['/'];
  with regs do
    begin
    AH:=$52;
    MsDos(regs);
    devseg:=ES;
    devofs:=BX
    end;
  lastdrv:=Mem[devseg:devofs + $0021];
  if (Lo(LastMode) = 2) or (Lo(LastMode) = 7) then
    mono:=true;
  with regs do
    begin
    AX:=$2B01;
    CX:=$4445; {DE}
    DX:=$5351; {SQ}
    MsDos(regs);
    if AL <> $FF then
      DirectVideo:=False;
    end;
  if ParamCount > 0 then
    checkparams;
  Textcolor(LightGreen);
  TextBackground(Blue);
  Clrscr;
  Write('INFO+');
  Textcolor(lightgray);
  Write(' - Information on all computer functions');
  rjustify(qversion);
  Writeln;
  border(#223);
  GotoXY(1, tlength - 1);
  border(#220);
  Write('Page ');
  x1:=wherex;
  Textcolor(Lightgreen);
  rjustify('F1 Enter PgUp PgDn Home End Esc Alt-P');
  pg:=0;
  endit:=false;
  if osmajor >= 3 then
    with regs do
      begin
      AX:=$3800;
      DS:=seg(country);
      DX:=ofs(country);
      MsDos(regs);
      ccode:=BX;
      decimal:=Chr(country[9]);
      end;
  end; {init}
end.
