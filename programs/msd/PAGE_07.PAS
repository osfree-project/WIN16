unit page_07;

interface

uses crt, ifpglobl, ifpcomon;

procedure page07;

implementation

procedure page07;

const
  mouseint = $33;
  mouselang: array [0..8] of string[10] = ('English', 'French', 'Dutch',
          'German', 'Swedish', 'Finnish', 'Spanish', 'Portuguese', 'Italian');

var
  xbyte : byte;
  xword1 : word;
  xword2 : word;
  jsfound: boolean;
  eqjs: boolean;
  oldx, oldy, oldattr: byte;
  xlong: longint;
  MSysMouse: boolean;
  MSVersion: word;

  begin
  Caption2('Keyboard');
  Writeln;
  Caption3('BIOS support for enhanced keyboard');
  with regs do
    begin
    AH:=$02;
    Intr($16, regs);
    xbyte:=AL;
    AX:=$1200 + xbyte xor $FF;
    Intr($16, regs);
    if AL = xbyte then
      begin
      Write('yes');
      Caption3('Enhanced keyboard present');
      yesorno(Mem[BIOSdseg : $0096] and $10 = $10)
      end
    else
      Writeln('no');
(*  PC Magazine 6:15 p.378  *)
    AH:=$02;
    Intr($16, regs);
    OffOrOn('Insert', AL and $80 = $80);
    OffOrOn('Caps Lock', AL and $40 = $40);
    OffOrOn('Num Lock', AL and $20 = $20);
    OffOrOn('Scroll Lock', AL and $10 = $10);
    Writeln;
    end;
  Caption3('Buffer start');
  xword1:=Memw[BIOSdseg : $0080];
  segofs(BIOSdseg, xword1);
  Caption3(' end');
  xword2:=Memw[BIOSdseg : $0082];
  segofs(BIOSdseg, xword2);
  Caption3(' size (keystrokes)');
  Writeln((xword2 - xword1) shr 1 - 1);
  Caption2('Internal modem [IBM Convertible]/serial printer [PCjr]');
  yesorno(equip and $2000 = $2000);
  Caption2('Game port');
  xword:=0;
  jsfound:=false;
  Port[$201]:=1;
  if Port[$201] and $F <> $F then
    jsfound:=false
  else
    repeat
      if (Port[$201] and $F) <> $F then
        jsfound:=true;
      Inc(xword);
    until (xword = $F00) or jsfound;
  if jsfound then
    Write('yes, and ')
  else
    Write('no, and ');
  if (equip and $1000) <> $1000 then
    Write('NOT')
  else
    Write('IS');
  Writeln(' set in BIOS equipment word');
  Caption2('Mouse');
  with regs do
    begin
    oldx:=WhereX;
    oldy:=WhereY;
    oldattr:=TextAttr;
    TextColor(LightRed + Blink);
    Write('** Searching for Mouse **');
    TextAttr:=oldattr;
    GotoXY(oldx, oldy);
    AX:=$0021;
    Intr(mouseint, regs);
    ClrEol;
    if AX = $FFFF then
      begin
      Write('yes');
      Caption3('Buttons');
      if BX = $FFFF then
        BX:=2;
      Writeln(BX);
      MSysMouse:=false;
      AX:=$0070;
      BX:=$ABCD;
      Intr(mouseint, regs);
      if (AX = $ABCD) and (MemW[BX:CX] = $ABCD) and
        (MemW[BX:CX + 8] = $ABCD) then
        begin
        MSysMouse:=true;
        Caption3('Mouse Systems version');
        xword:=MemW[MemW[BX:CX + 4]:MemW[BX:CX + 2]];
        Writeln(Hi(xword), decimal, addzero(Lo(xword)));
        end;
      AX:=$266C;
      BX:=0;
      CX:=0;
      Intr(mouseint, regs);
      if BX = $5353 {SS} then
        begin
        Caption3('Logitech version');
        Write(Chr(CH), decimal, Chr(CL), 'x');
        if Chr(CH) >= '4' then
          begin
          Caption3('Ballistics');
          AX:=$1F6C;
          BX:=0;
          CX:=0;
          Intr(mouseint, regs);
          if BX = 0 then
            Write('off')
          else
            case CX of
              1: Write('low');
              2: Write('high')
            else
              Write('???');
            end;
          Caption3('Hand');
          AX:=$216C;
          BX:=0;
          Intr(mouseint, regs);
          if BX = 0 then
            Writeln('right')
          else
            Writeln('left')
          end;
        end;
      Caption3('Microsoft version standard');
      AX:=$0024;
      Intr(mouseint, regs);
      if AX < $FFFF then
        begin
        MSVersion:=BX;
        Writeln(BH, decimal, addzero(unbcd(BL)));
        Caption3('Type');
        case CH of
          $01 : Write('bus');
          $02 : Write('serial');
          $03 : Write('InPort');
          $04 : Write('PS/2');
          $05 : Write('HP')
          else
            unknown('mouse', CH, 2)
        end;
        Caption3('Interrupt');
        case CL of
          $00 : Writeln('PS/2');
          $02..$05, $07 : Writeln('IRQ', CL)
          else
            unknown('interrupt', CL, 2)
        end;
        if (Hi(MSVersion) >= 7) and (not MSysMouse) then
          begin
          Caption3('driver type');
          AX:=$25;
          Intr(MouseInt, Regs);
          if (AX and $8000) = $8000 then
            Write('device driver')
          else
            Write('TSR');
          Caption3('cursor type');
          case (AX and $3000) shr 12 of
            0: Write('software');
            1: Write('hardware');
            2,3: Write('graphics')
          end;
          Caption3('no. of MDD''s');
          Writeln(AX and $FF);
          Caption3('interrupt rate');
          case (AX and $0F00) shr 8 of
            0: Write('none');
            1: Write('30Hz');
            2: Write('50Hz');
            3: Write('100Hz');
            4: Write('200Hz')
          else
            Write('???');
          end;
          Caption3('acceleration profile');
          AX:=$2C;
          Intr(MouseInt, Regs);
          Write(BX);
          if (Hi(MSVersion) > 7) or
            ((Hi(MSVersion) = 7) and (Lo(MSVersion) >= 4)) then
            begin
            Caption3('BallPoint');
            AX:=$30;
            CX:=0;
            Intr(MouseInt, Regs);
            yesorno2(AX <> $FFFF);
            end;
          Writeln;
          if Hi(MSVersion) >= 8 then
            begin
            caption3('.INI file path');
            AX:=$34;
            Intr(MouseInt, Regs);
            while Mem[ES:DX] <> 0 do
              begin
              Write(Chr(Mem[ES:DX]));
              Inc(DX);
              end;
            Writeln;
            end;
          If (Hi(MSVersion) > 8) or
            ((Hi(MSVersion) = 8) and (Lo(MSVersion) >= $10)) then
            begin
            caption3('pointer - style');
            AX:=$35;
            BX:=$FFFF;
            Intr(MouseInt, Regs);
            case BH of
              0: Write('normal');
              1: Write('reverse');
              2: Write('transparent')
            else
              Write('???');
            end;
            caption3('size');
            case BL of
              0: Write('small');
              1: Write('medium');
              2: Write('large')
            else
              Write('???');
            end;
            caption3('size change');
            yesorno(CL = 1);
            end;
          end;
        Caption3('EGA register support');
        AH:=$FA;
        BX:=0;
        Intr($10, regs);
          if BX <> 0 then
          begin
          Write('yes');
          Caption3('version');
          Writeln(unbcd(Mem[ES:BX]), decimal, addzero(unbcd(Mem[ES:BX+1])))
          end
        else
          Writeln('no');
        Caption3('Save state buffer size (bytes)');
        AX:=$0015;
        Intr(mouseint, regs);
        Writeln(BX);
        Caption3('Mickeys/pixel (horizontal)');
        AX:=$001B;
        Intr(mouseint, regs);
        Write(BX : 5);
        Caption3(' (vertical)');
        Writeln(CX : 5);
        Caption3('Double speed threshold');
        Writeln(DX);
        Caption3('Current display page');
        AX:=$001E;
        Intr(mouseint, regs);
        Writeln(BX);
        Caption3('Language');
        AX:=$0023;
        Intr(mouseint, regs);
        if AX < $FFFF then
          if BX < 9 then
            Writeln(mouselang[BX])
          else
            unknown('language', BX, 4)
        else
          Writeln('N/A');
        end
      else
        dontknow
      end
    else
      Writeln('no')
    end
  end;
end.