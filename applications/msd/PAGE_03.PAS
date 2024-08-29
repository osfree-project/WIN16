unit page_03;

interface

uses Crt, Dos, ifpglobl, ifpcomon, ifpextrn;

procedure page03;

implementation

procedure page03;

const
  EMMint = $67;
  qEMMdrvr = 'EMMXXXX0';
  EMMerrs : array [$80..$A4] of string[55] = (
        {80} 'internal error in EMM software',
             'malfunction in expanded memory hardware',
             'memory manager busy',
             'invalid handle',
             'undefined function',
             'no more handles available',
             'error in save or restore of mapping context',
             'not enough physical pages available',
        {88} 'not enough free pages available',
             'no pages requested',
             'logical page outside range assigned to handle',
             'invalid physical page number',
             'page map hardware state save area full',
             'mapping context already in save area',
             'mapping context not in save area',
             'undefined subfunction parameter',
        {90} 'attribute type not defined',
             'feature not supported',
             'src & dest overlap;move done, but source overwritten',
             'length for src or dest longer than allocated',
             'conventional and EMS memory overlap',
             'offset outside logical page',
             'region length >1M',
             'src & dest overlap;not moved',
        {98} 'src & dest types undefined',
             'unused erro code',
             'Alt map or DMA supported, but specified set isn''t',
             'Alt map or DMA supported, but all allocated',
             'Alt map or DMA not suported, specified set <> 0',
             'Alt map or DMA suported, specified set <> 0',
             'Dedicated DMA channels not supported',
             'Dedicated DMA channels supported, but not specified one',
        {A0} 'No handle found for specified name',
             'handle with same name already exists',
             '???',
             'invalid pointer passed, or contents of source corrupted',
             'access to function denied');

var
  EMMarray : array[$000..$3FF] of word;
  xlong : longint;
  xword1 : word;
  xword2 : word;
  numhandles: word;
  xstring : string;
  EMMver, j: byte;
  EMMname: array[1..8] of char;
  isdpmi: boolean;
  direc: directions;
  ch2: char2;

  procedure EMMerr(a : byte);
    begin
    if (a >= $80) and (a <= $8F) then
      Writeln(EMMerrs[a])
    else
      unknown('expanded memory error', a, 2)
    end; {EMMerr}

  procedure showbcd(x: word);
    var
      c: char;

    begin
    c:=Chr((x shr 12) + 48);
    if c <> '0' then
      Write(c);
    Write(Chr(((x and $0F00) shr 8) + 48), decimal,
      Chr(((x and $00F0) shr 4) + 48), Chr((x and $000F) + 48))
    end; {showbcd}

  begin (* procedure page_03 *)
  Caption2('Total conventional memory (bytes)  ');
  Writeln(DOSmem: 6, ' (', DOSmem div 1024, 'K)');
  Caption2('Free conventional memory (bytes)   ');
  xlong:=DOSmem - (longint(PrefixSeg) shl 4);
  Writeln(xlong: 6, ' (', xlong div 1024, 'K)');
  Caption2('Extended memory (from BIOS call) ');
  with regs do begin
    AH:=$88;
    Flags:=Flags and FCarry;
    Intr($15, regs);
    if nocarry(regs) then
      Writeln(longint(AX) shl 10:8, ' (', AX, 'K)')
    else
      Writeln('     N/A')
  end;
  Caption2('XMS driver present ');
  with regs do
    begin
    AX:=$4300;
    Intr($2F, regs);
    if AL <> $80 then
      Writeln('no')
    else
      begin
      Writeln('yes');
      AX:=$4310;
      Intr($2F, regs);
      xlong:=longint(ES) shl 16 + BX;
      Caption3('XMS version');
      AX:=0;
      LongCall(xlong, regs);
      if AX <> 0 then
        begin
        showbcd(AX);
        Caption3('XMM version');
        showbcd(BX);
        end
      else
        Write('ERROR');
      Caption3('A20 is');
      AX:=$0700;
      LongCall(xlong, regs);
      if (AX <> 0) or ((AX = 0) and (BL = 0)) then
        case AX of
          0: Writeln('disabled');
          1: Writeln('enabled');
        else
          Writeln('unknown');
        end
      else
        Write('ERROR');
      Caption3('Total free XMS memory');
      AX:=$0800;
      LongCall(xlong, regs);
      if (AX <> 0) or ((AX = 0) and ((BL = 0) or (BL = $A0))) then
        begin
        Write(DX, 'K');
        Caption3('Largest available block');
        Writeln(AX, 'K');
        end
      else
        Writeln('ERROR');
      Caption3('Upper memory Blocks');
      AX:=$1000;
      DX:=1;
      LongCall(xlong, regs);
      if (AX = 0) and (BL <> $B1) then
        Writeln('no')
      else
        if (AX = 0) and (BL = $B1) then
          Writeln('supported, but none available')
        else
          begin
          Write('yes');
          Caption3('Largest available size');
          AX:=$1100;
          DX:=BX;
          LongCall(xlong, regs);
          AX:=$1000;
          DX:=$FFFF;
          LongCall(xlong, regs);
          Writeln(DX * longint(16), ' (', ((DX * 16.0) / 1024):1:1, 'K)');
          end;
      AX:=0;
      LongCall(xlong, regs);
      Caption3('HMA');
      YesOrNo2(DX = 1);
      AX:=$0100;
      DX:=$FFFF;
      LongCall(xlong, regs);
      if AX = 0 then
        Write(' (in use)')
      else
        Write(' (free)');
      if (OSMajor >= 5) and (OSMajor < 10) then
        begin
        Caption3('Used by DOS');
        AX:=$4A01;
        BX:=0;
        ES:=0;
        DI:=0;
        Intr($2F, regs);
        YesOrNo2(BX <> 0);
        if BX <> 0 then
          begin
          Caption3('bytes free');
          Write(BX);
          Caption3('at');
          SegOfs(ES, DI);
          end;
        end;
      Writeln;
      end;
    end;
  isdpmi:=false;
  Caption2('DPMI driver present');
  with regs do
    begin
    AX:=$1687;
    Intr($2F, regs);
    if AX <> 0 then
      Writeln('no')
    else
      begin
      Writeln('yes');
      isdpmi:=true;
      Caption3('version');
      Write(DH, decimal, DL);
      Caption3('CPU');
      case CL of
        2: Write('286');
        3: Write('386');
        4: Write('486');
        5: Write('P5')
      else
        Write('???')
      end;
      Caption3('switch mode entry');
      SegOfs(ES, DI);
      Writeln
      end
    end;
  pause3(-12);
  if endit then
    Exit;
  Caption2('Expanded memory');
  if longint(intvec[EMMint]) <> $00000000 then
    begin
    Writeln;
    Caption3('Interrupt vector');
    xlong:=longint(intvec[EMMint]);
    xword1:=xlong shr 16;
    xword2:=xlong and $0000FFFF;
    SegOfs(xword1, xword2);
    Writeln;
    Caption3('Driver');
    xstring:='';
    for i:=$000A to $0011 do
      xstring:=xstring + showchar(Chr(Mem[xword1 : i]));
    Write(xstring);
    if xstring = qEMMdrvr then
      begin
      Caption3('status');
      with regs do
        begin
        AH:=$40;
        Intr(EMMint, regs);
        if AH = $00 then
          Write('available')
        else
          EMMerr(AH);
        Caption3('version');
        AH:=$46;
        Intr(EMMint, regs);
        if AH = $00 then
          Writeln(AL shr 4, decimal, AL and $0F)
        else
          EMMerr(AH);
        EMMver:=AL shr 4;
        Caption3('Page frame segment');
        AH:=$41;
        Intr(EMMint, regs);
        if AH = $00 then
          Writeln(Hex(BX, 4))
        else
          EMMerr(AH);
        Caption3('Total EMS memory');
        AH:=$42;
        Intr(EMMint, regs);
        if AH = $00 then
          begin
          Write(longint(16) * DX, 'K');
          Caption3('available');
          if AH = $00 then
            Writeln(longint(16) * BX, 'K')
          else
            EMMerr(AH)
          end
        else
          EMMerr(AH);
        if EMMver >= 4 then
          begin
          Caption3('VCPI capable');
          {Check if Windows in enhanced mode. Don't do VCPI check if found.}
          AX:=$1600;
          Intr($2F, regs);
          if AL in [0, 1, $80, $FF] then
{          if not isdpmi then}
            begin
            {must make sure 1 page is allocated to be sure that EMS}
            {driver is ON. VCPI is not detectable if EMS driver is OFF}
            {16K of EMS needed for this test to work properly}
            AH:=$43;
            BX:=1;
            Intr(EMMint, regs);
            if AH <> 0 then
              Writeln('error: need 16K available EMS to detect')
            else
              begin
              xword1:=DX; {handle}
              AX:=$DE00;
              Intr(EMMint, regs);
              if AH <> 0 then
                Writeln('no')
              else
                begin
                Write('yes');
                Caption3('VCPI version');
                Writeln(BH, decimal, BL);
                end;
              AH:=$45; {release our handle}
              DX:=xword1;
              Intr(EMMint, regs)
              end
            end
          else
            Writeln('no');
          end;
        Caption1('  Handle   Size  Name');
        Writeln;
        AH:=$4D;
        ES:=seg(EMMarray);
        DI:=ofs(EMMarray);
        Intr(EMMint, regs);
        if AH = $00 then
          if BX > $0000 then
            begin
            Window(3, WhereY + Hi(WindMin), twidth, tlength - 2);
            numhandles:=BX;
            for i:=1 to numhandles do
              begin
              pause2;
              if endit then
                Exit;
              xlong:=longint(16) * EMMarray[2 * i - 1];
              if xlong > 0 then
                begin
                Write(Hex(EMMarray[2 * i - 2], 4), '   ', xlong:5, 'K  ');
                if EMMver >= 4 then
                  begin
                  AX:=$5300;
                  DX:=EMMarray[2 * i - 2];
                  ES:=Seg(EMMname);
                  DI:=Ofs(EMMname);
                  Intr(EMMint, regs);
                  if AH = 0 then
                    for j:=1 to 8 do
                      if EMMname[j] <> #0 then
                        Write(EMMname[j]);
                  end;
                Writeln;
                end;
              end;
            end
          else
            Writeln('  (no active handles)')
        else
          EMMerr(AH)
        end
      end
    else
      begin
      Writeln;
      dontknow
      end
    end
  else
    Writeln('(none)')
  end;
end.