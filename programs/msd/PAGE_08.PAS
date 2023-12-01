unit page_08;

interface

uses crt, ifpglobl, ifpcomon;

procedure page08;

implementation

procedure page08;
  const
    tick2 = 115200;

  var
    i : byte;
    xbyte1 : byte;
    xbyte2 : byte;
    xbyte3: byte;
    xbyte4: byte;
    xbyte5: byte;
    xbyte6: byte;
    xbyte7: byte;
    xword : word;
    xword1: word;
    xword2: word;
    temp: word;
    sbport: word;
    sbfound: boolean;
    portok: boolean;
    midifound: boolean;
    soundvect: pointer;
    GusFound: boolean;
    GusPort: word;
    foundit: boolean;
    s: string;

procedure GusDelay;
  begin
  asm
    mov dx,$300
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
  end;
  end;

procedure GusPoke(high: byte; low: word; value: byte);
  begin
  Port[GusPort + $103]:=$43;
  PortW[GusPort + $104]:=low;
  Port[GusPort + $103]:=$44;
  Port[GusPort + $105]:=high;
  Port[GusPort + $107]:=value;
  end;

function GusPeek(high: byte; low: word): byte;
  begin
  Port[GusPort + $103]:=$43;
  PortW[GusPort + $104]:=low;
  Port[GusPort + $103]:=$44;
  Port[GusPort + $105]:=high;
  GusPeek:=Port[GusPort + $107];
  end;

  begin
  window(1, 3, 30, tlength - 2);
  caption2('Printers');
  xbyte1:=equip and $C000 shr 14;
  Writeln(xbyte1);
  if xbyte1 > 0 then
    begin
    caption3('Device');
    Writeln;
    caption3('Base Port');
    Writeln;
    caption3('Timeout');
    Writeln;
    caption3('Busy');
    Writeln;
    caption3('ACK');
    Writeln;
    caption3('Paper out');
    Writeln;
    caption3('Selected');
    Writeln;
    caption3('I/O error');
    Writeln;
    caption3('Timed out');
    for i:=1 to xbyte1 do
      begin
      Window(9 + 6 * i, 4, 15 + 6 * i, tlength - 2);
      Writeln('LPT', i);
      Writeln('$', hex(MemW[BIOSdseg : 2 * i + 6], 4));
      Writeln(Mem[BIOSdseg : $0077 + i]);
      with regs do
        begin
        AH:=$02;
        DX:=i;
        Intr($17, regs);
        yesorno(AH and $80 = $00);
        yesorno(AH and $40 = $40);
        yesorno(AH and $20 = $20);
        yesorno(AH and $10 = $10);
        yesorno(AH and $08 = $08);
        yesorno(AH and $01 = $01)
        end
      end
    end;
  Window(twidth - 42, 3, twidth, tlength - 2);
  caption2('Serial ports');
  xbyte1:=equip and $0E00 shr 9;
  Writeln(xbyte1);
  if xbyte1 > 0 then
    begin
    if xbyte1 > 4 then
      xbyte1:=4;
    caption3('Device');
    Writeln;
    caption3('Base port');
    Writeln;
    caption3('UART');
    Writeln;
    caption3('Timeout');
    Writeln;
    caption3('Baud rate');
    Writeln;
    caption3('Data bits');
    Writeln;
    caption3('Parity');
    Writeln;
    caption3('Stop bits');
    Writeln;
    caption3('Break');
    Writeln;
    caption3('RLSD');
    Writeln;
    caption3('RI');
    Writeln;
    caption3('DSR');
    Writeln;
    caption3('CTS');
    Writeln;
    caption3('dRLSD');
    Writeln;
    caption3('-dRI');
    Writeln;
    caption3('dDSR');
    Writeln;
    caption3('dCTS');
    for i:=1 to xbyte1 do
      begin
      window(twidth - 35 + 7 * i, 4, twidth - 28 + 7 * i, tlength - 2);
      Writeln('COM', i);
      xword:=MemW[BIOSdseg : 2 * i - 2];
      if xword = 0 then
        Writeln('N/A')
      else
        begin
        Writeln('$', hex(xword, 4));
        xbyte2:=Port[xword + 7];
        Port[xword + 7]:=$5A;
        if Port[xword + 7] <> $5A then
          Write('8250')
        else
          begin
          Port[xword + 7]:=$A5;
          if Port[xword + 7] <> $A5 then
            Write('8250')
          else
            begin
            Port[xword + 7]:=xbyte2;
            xbyte2:=Port[xword + 2];
            Port[xword + 2]:=1;
            xbyte3:=Port[xword + 2];
            if xbyte3 and $C0 = $C0 then
              Write('16550A')
            else
              if xbyte3 and $80 = $80 then
                Write('16550')
              else
                Write('16450');
            end;
          end;
          if not FifoOn then
            Port[xword + 2]:=0;
{        xbyte2:=Port[xword + 2];
        Port[xword + 2]:=$C1;
        xbyte3:=Port[xword + 2];
        Port[xword + 2]:=xbyte2;
        case ((xbyte3 and $C0) shr 6) of
          0: begin
             xbyte2:=Port[xword + 7];
             Port[xword + 7]:=$FA;
             for temp:=1 to 2 do;
             if Port[xword + 7] = $FA then
               begin
               Port[xword + 7]:=$AF;
               for temp:=1 to 2 do;
               if Port[xword + 7] = $AF then
                 begin
                 Port[xword + 7]:=xbyte2;
                 Write('16450')
                 end
               else
                 Write('8250')
               end
             else
               Write('8250')
             end;
          1: Write('???');
          2: Write('16550');
          3: Write('16550A')
        end;
 }       Writeln;
        Writeln(Mem[BIOSdseg : $007B + i]);
        xbyte2:=Port[xword + 3];
        Port[xword + 3]:=xbyte2 or $80;
        xword2:=cbw(Port[xword], Port[xword + 1]);
        if xword2 = 0 then
          Writeln('N/A')
        else
          Writeln(tick2 / xword2:0:0);
        Port[xword + 3]:=xbyte2;
        Writeln((xbyte2 and $03) + 5);
        case xbyte2 and $38 of
          $00, $10, $20, $30 : Writeln('none');
          $08 : Writeln('odd');
          $18 : Writeln('even');
          $28 : Writeln('mark');
          $38 : Writeln('space')
        end;
        case xbyte2 and $07 of
          $00..$03 : Writeln('1');
          $04 : Writeln('1.5');
          $05..$07 : Writeln('2')
        end;
        yesorno(xbyte2 and $40 = $40);
        with regs do
          begin
          AH:=$03;
          DX:=i - 1;
          Intr($14, regs);
          yesorno(AL and $80 = $80);
          yesorno(AL and $40 = $40);
          yesorno(AL and $20 = $20);
          yesorno(AL and $10 = $10);
          yesorno(AL and $08 = $08);
          yesorno(AL and $04 = $04);
          yesorno(AL and $02 = $02);
          yesorno(AL and $01 = $01)
          end;
        end
      end
    end;
  Window(1, 14, twidth - 43, tlength - 2);
  caption2('Sound cards');
  Writeln;
  caption3('Ad Lib (or compatible)');
  xbyte2:=Port[$388];
  Port[$388]:=$BD;
  xbyte1:=Port[$388];
  xbyte1:=Port[$388];
  xbyte1:=Port[$388];
  xbyte1:=Port[$388];
  xbyte3:=Port[$389];
  Port[$389]:=0;
  for xbyte4:=1 to 36 do
    xbyte1:=Port[$388];
  xbyte1:=xbyte1 and 7;
  Port[$388]:=xbyte2;
  Port[$389]:=xbyte3;
  yesorno(xbyte1 = 6);
  if xbyte1 = 6 then
    begin
    caption3('  driver');
    with regs do
      begin
      AX:=$3565;
      MsDos(regs);
      s:='';
      for xword:=(BX - $16) to (BX - 4) do
        s:=s + Chr(Mem[ES:xword]);
      if s = 'SOUND-DRIVER-AD-LIB' then
        begin
        Write('yes');
        caption3('version');
        Writeln(unBCD(Mem[ES:BX - $17]), decimal, addzero(unBCD(Mem[ES:BX - $18])));
        caption3('  address');
        Writeln(hex(ES, 4), ':', hex(BX, 4));
        end
      else
        Writeln('no');
      end
    end;
  caption3('Sound Blaster');
  sbfound:=false;
  xbyte1:=1;
  while (xbyte1 < 7) and (not sbfound) do
    begin
    sbport:=$200 + ($10 * xbyte1);
    xword1:=0;
    portok:=false;
{    xword2:=sbport + $0C;}
    while (xword1 < $201) and (not portok) do
      begin
      if (Port[sbport + $0C] and $80) = 0 then
        portok:=true;
      Inc(xword1)
      end;
    if portok then
      begin
      xbyte3:=Port[sbport + $0C];
      Port[sbport + $0C]:=$D3;
      for xword2:=1 to $1000 do {nothing};
{      xword2:=sbport + 6;}
      xbyte4:=Port[sbport + 6];
      Port[sbport + 6]:=1;
      xbyte2:=Port[sbport + 6];
      xbyte2:=Port[sbport + 6];
      xbyte2:=Port[sbport + 6];
      xbyte2:=Port[sbport + 6];
      Port[sbport + 6]:=0;
{      xword2:=sbport + $0E;}
      xbyte2:=0;
      repeat
        xword1:=0;
        portok:=false;
        while (xword1 < $201) and (not portok) do
          begin
          if (Port[sbport + $0E] and $80) = $80 then
            portok:=true;
          Inc(xword1)
          end;
        if portok then
          if Port[sbport + $0A] = $AA then
            sbfound:=true;
        Inc(xbyte2);
      until (xbyte2 = $10) or (portok);
      if not portok then
        begin
        Port[sbport + $0C]:=xbyte3;
        Port[sbport + 6]:=xbyte4;
        end;
      end;
    if sbfound then
      begin
      Write('yes');
      Caption3('port');
      Writeln('$', Hex(sbport, 3));
      end
    else
      Inc(xbyte1);
    end;
  if not sbfound then
    Writeln('no');
  caption3('Roland MPU-401');
  portok:=false;
  midifound:=false;
  xbyte1:=0;
  repeat
    if (Port[$331] and $40) = 0 then
      portok:=true;
    Inc(xbyte1);
  until (xbyte1 = 255) or portok;
  if portok then
    begin
    inline($FA); {CLI}
    xbyte2:=Port[$331];
    Port[$331]:=$FF;
    portok:=false;
    xbyte1:=0;
    repeat
      if (Port[$331] and $80) = 0 then
        portok:=true;
      Inc(xbyte1);
    until (xbyte1 = 255) or portok;
    xbyte1:=Port[$330];
    inline($FB); {STI}
    if portok and (xbyte1 = $FE) then
      midifound:=true
    else
      Port[$331]:=xbyte2;
    end;
  yesorno(midifound);
{
  caption3('Tandy digital');
  with regs do
    begin
    AX:=$8100;
    Intr($1A, regs);
    yesorno(AH > $80);
    end;
}
  caption3('Gravis Ultrasound');
  GusFound:=false;
  xbyte1:=1;
  while (xbyte1 < 7) and (not GusFound) do
    begin
    GusPort:=$200 + ($10 * xbyte1);
    xbyte3:=Port[GusPort + $103];
    xbyte4:=Port[GusPort + $104];
    xbyte5:=Port[GusPort + $105];
    xbyte6:=Port[GusPort + $106];
    xbyte7:=Port[GusPort + $107];
    Port[GusPort + $103]:=$4C;
    Port[GusPort + $105]:=0;
    GusDelay;
    GusDelay;
    Port[GusPort + $103]:=$4C;
    Port[GusPort + $105]:=1;
    GusPoke(0, 0, $AA);
    GusPoke(1, 0, $55);
    xbyte2:=GusPeek(0, 0);
    GusPoke(0, 0, 0);
    Port[GusPort + $103]:=$4C;
    Port[GusPort + $105]:=0;
    if xbyte2 = $AA then
      begin
      gusfound:=true;
      Writeln('yes');
      Caption3('  port');
      Write('$', Hex(GusPort, 3));
      Caption3('memory');
      xbyte3:=0;
      foundit:=false;
      while (xbyte3 <= 4) and (not foundit) do
        begin
        if xbyte3 <> 4 then
          begin
          GusPoke(xbyte3 * 4, 0, $AA);
          xbyte2:=GusPeek(xbyte3 * 4, 0);
          end
        else
          begin
          GusPoke($F, $FFFF, $AA);
          xbyte2:=GusPeek($F, $FFFF);
          end;
        if xbyte2 <> $AA then
          begin
          foundit:=true;
          Writeln(xbyte3 * 256, 'K');
          end;
        end;
      end
    else
      begin
      Port[GusPort + $103]:=xbyte3;
      Port[GusPort + $104]:=xbyte4;
      Port[GusPort + $105]:=xbyte5;
      Port[GusPort + $106]:=xbyte6;
      Port[GusPort + $107]:=xbyte7;
      Inc(xbyte1);
      end;
    end;
  if not gusfound then
    Writeln('no');
  end;
end.
