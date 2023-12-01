procedure page_07;

const
  mouseint = $33;
  mouselang: array [0..8] of string[10] = ('English', 'French', 'Dutch',
          'German', 'Swedish', 'Finnish', 'Spanish', 'Portuguese', 'Italian');

var
  xbyte : byte;
  xword1 : word;
  xword2 : word;

  begin
  caption2('Keyboard');
  writeln;
  caption3('BIOS support for enhanced keyboard');
  with regs do
    begin
    AH:=$02;
    intr($16, regs);
    xbyte:=AL;
    AX:=$1200 + xbyte xor $FF;
    intr($16, regs);
    if AL = xbyte then
      begin
      write('yes');
      caption3('Enhanced keyboard present');
      yesorno(mem[BIOSdseg : $0096] and $10 = $10)
      end
    else
      writeln('no');
(*  PC Magazine 6:15 p.378  *)
    AH:=$02;
    intr($16, regs);
    offoron('Insert     ', AL and $80 = $80);
    offoron('Caps Lock  ', AL and $40 = $40);
    offoron('Num Lock   ', AL and $20 = $20);
    offoron('Scroll Lock', AL and $10 = $10)
    end;
  caption3('Buffer start');
  xword1:=memw[BIOSdseg : $0080];
  segofs(BIOSdseg, xword1);
  caption3(' end');
  xword2:=memw[BIOSdseg : $0082];
  segofs(BIOSdseg, xword2);
  caption3(' size (keystrokes)');
  writeln((xword2 - xword1) shr 1 - 1);
  caption2('Internal modem/serial printer');
  yesorno(equip and $2000 = $2000);
  caption2('Game port');
  yesorno(equip and $1000 = $1000);
  caption2('Mouse');
  with regs do
    begin
    AX:=$0000;
    intr(mouseint, regs);
    if AX = $FFFF then
      begin
      writeln('yes');
      caption3('Buttons');
      writeln(BX);
      caption3('EGA register support');
      AH:=$FA;
      BX:=0;
      Intr($10, regs);
        if BX <> 0 then
        begin
        Write('yes');
        caption3('version');
        Writeln(Mem[ES:BX], decimal, Mem[ES:BX+1])
        end
      else
        Writeln('no');
      caption3('Save state buffer size (bytes)');
      AX:=$0015;
      intr(mouseint, regs);
      writeln(BX);
      caption3('Mickeys/pixel (horizontal)');
      AX:=$001B;
      intr(mouseint, regs);
      write(BX : 5);
      caption3(' (vertical)');
      writeln(CX : 5);
      caption3('Double speed threshold');
      writeln(DX);
      caption3('Current display page');
      AX:=$001E;
      intr(mouseint, regs);
      writeln(BX);
      caption3('Language');
      AX:=$0023;
      intr(mouseint, regs);
      if AX < $FFFF then
        if BX < 9 then
          Writeln(mouselang[BX])
        else
          unknown('language', BX, 4)
      else
        writeln('N/A');
      caption3('Driver version');
      AX:=$0024;
      intr(mouseint, regs);
      if AX < $FFFF then
        begin
        write(BH, decimal);
        zeropad(BL);
        writeln;
        caption3('Type');
        case CH of
          $01 : writeln('bus');
          $02 : writeln('serial');
          $03 : writeln('InPort');
          $04 : writeln('PS/2');
          $05 : writeln('HP')
          else
            unknown('mouse', CH, 2)
        end;
        caption3('Interrupt');
        case CL of
          $00 : writeln('PS/2');
          $02..$05, $07 : writeln('IRQ', CL)
          else
            unknown('interrupt', CL, 2)
        end
        end
      else
        dontknow
      end
    else
      writeln('no')
    end
  end;
