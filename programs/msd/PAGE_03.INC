procedure page_03;
type
  string2 = string[2];

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
  caption2('Total conventional memory (bytes)  ');
  Writeln(DOSmem: 6, ' (', DOSmem div 1024, 'K)');
  caption2('Free conventional memory (bytes)   ');
  xlong:=DOSmem - (longint(PrefixSeg) shl 4);
  Writeln(xlong: 6, ' (', xlong div 1024, 'K)');
  caption2('Extended memory (from BIOS call) ');
  with regs do begin
    AH:=$88;
    intr($15, regs);
    if nocarry then
      writeln(longint(AX) shl 10: 8, ' (', (longint(AX) shl 10) div 1024, 'K)')
    else
      writeln('     N/A')
  end;
  caption2('XMS driver present ');
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
      caption3('XMS version');
      AX:=0;
      longcall(xlong, regs);
      if AX <> 0 then
        begin
        showbcd(AX);
        caption3('XMM version');
        showbcd(BX);
        caption3('HMA');
        yesorno2(DX = 1);
        end
      else
        Write('ERROR');
      caption3('A20 is');
      AX:=$0700;
      longcall(xlong, regs);
      if (AX <> 0) or ((AX = 0) and (BL = 0)) then
        case AX of
          0: Writeln('disabled');
          1: Writeln('enabled');
        else
          Writeln('unknown');
        end
      else
        Writeln('ERROR');
      caption3('Total free XMS memory');
      AX:=$0800;
      longcall(xlong, regs);
      if (AX <> 0) or ((AX = 0) and ((BL = 0) or (BL = $A0))) then
        begin
        Write(DX, 'K');
        caption3('Largest available block');
        Writeln(AX, 'K');
        end
      else
        Writeln('ERROR');
      end
    end;
  isdpmi:=false;
  caption2('DPMI driver present');
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
      caption3('version');
      Write(DH, decimal, DL);
      caption3('CPU');
      case CL of
        2: Write('286');
        3: Write('386');
        4: Write('486')
      else
        Write('???')
      end;
      caption3('switch mode entry');
      segofs(ES, DI);
      Writeln
      end
    end;
  caption2('Expanded memory');
  if longint(intvec[EMMint]) <> $00000000 then
    begin
    writeln;
    caption3('Interrupt vector');
    xlong:=longint(intvec[EMMint]);
    xword1:=xlong shr 16;
    xword2:=xlong and $0000FFFF;
    segofs(xword1, xword2);
    writeln;
    caption3('Driver');
    xstring:='';
    for i:=$000A to $0011 do
      xstring:=xstring + showchar(chr(mem[xword1 : i]));
    write(xstring);
    if xstring = qEMMdrvr then
      begin
      caption3('status');
      with regs do
        begin
        AH:=$40;
        intr(EMMint, regs);
        if AH = $00 then
          writeln('available')
        else
          EMMerr(AH);
        caption3('EMM version');
        AH:=$46;
        intr(EMMint, regs);
        if AH = $00 then
          writeln(AL shr 4, decimal, AL and $0F)
        else
          EMMerr(AH);
        EMMver:=AL shr 4;
        caption3('Page frame segment');
        AH:=$41;
        intr(EMMint, regs);
        if AH = $00 then
          writeln(hex(BX, 4))
        else
          EMMerr(AH);
        caption3('Total EMS memory');
        AH:=$42;
        intr(EMMint, regs);
        if AH = $00 then
          begin
          write(longint(16) * DX, 'K');
          caption3('available');
          if AH = $00 then
            writeln(longint(16) * BX, 'K')
          else
            EMMerr(AH)
          end
        else
          EMMerr(AH);
        if EMMver >= 4 then
          begin
          caption3('VCPI capable');
          {skip VCPI if DPMI found, DPMI takes precedence over VCPI}
          if not isdpmi then
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
                caption3('VCPI version');
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
        caption1('  Handle   Size  Name');
        Writeln;
        AH:=$4D;
        ES:=seg(EMMarray);
        DI:=ofs(EMMarray);
        Intr(EMMint, regs);
        if AH = $00 then
          if BX > $0000 then
            begin
            window(3, wherey + hi(windmin), twidth, tlength - 2);
            numhandles:=BX;
            for i:=1 to numhandles do
              begin
              pause2;
              if endit then
                Exit;
              xlong:=longint(16) * EMMarray[2 * i - 1];
              if xlong > 0 then
                begin
                write(hex(EMMarray[2 * i - 2], 4), '   ', xlong:5, 'K  ');
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
                end
              end
            end
          else
            writeln('  (no active handles)')
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
    writeln('(none)')
  end;
