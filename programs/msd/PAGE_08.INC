procedure page_08;
  const
    tick2 = 115200;

  var
    i : byte;
    xbyte1 : byte;
    xbyte2 : byte;
    xword : word;
    xword2: word;

  begin
  window(1, 3, 30, tlength - 2);
  caption2('Printers');
  xbyte1:=equip and $C000 shr 14;
  writeln(xbyte1);
  if xbyte1 > 0 then
    begin
    caption3('Device');
    writeln;
    caption3('Base port');
    writeln;
    caption3('Timeout');
    writeln;
    caption3('Busy');
    writeln;
    caption3('ACK');
    writeln;
    caption3('Paper out');
    writeln;
    caption3('Selected');
    writeln;
    caption3('I/O error');
    writeln;
    caption3('Timed out');
    for i:=1 to xbyte1 do
      begin
      window(9 + 6 * i, 4, 15 + 6 * i, tlength - 2);
      writeln('LPT', i);
      writeln('$', hex(memw[BIOSdseg : 2 * i + 6], 3));
      writeln(mem[BIOSdseg : $0077 + i]);
      with regs do
        begin
        AH:=$02;
        DX:=0;
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
  window(twidth - 42, 3, twidth, tlength - 2);
  caption2('Serial ports');
  xbyte1:=equip and $0E00 shr 9;
  writeln(xbyte1);
  if xbyte1 > 0 then
    begin
    if xbyte1 > 4 then
      xbyte1:=4;
    caption3('Device');
    writeln;
    caption3('Base port');
    writeln;
    caption3('Timeout');
    writeln;
    caption3('Baud rate');
    writeln;
    caption3('Data bits');
    writeln;
    caption3('Parity');
    writeln;
    caption3('Stop bits');
    writeln;
    caption3('Break');
    writeln;
    caption3('RLSD');
    writeln;
    caption3('RI');
    writeln;
    caption3('DSR');
    writeln;
    caption3('CTS');
    writeln;
    caption3('dRLSD');
    writeln;
    caption3('-dRI');
    writeln;
    caption3('dDSR');
    writeln;
    caption3('dCTS');
    for i:=1 to xbyte1 do
      begin
      window(twidth - 35 + 7 * i, 4, twidth - 28 + 7 * i, tlength - 2);
      writeln('COM', i);
      xword:=memw[BIOSdseg : 2 * i - 2];
      writeln('$', hex(xword, 3));
      writeln(mem[BIOSdseg : $007B + i]);
      xbyte2:=port[xword + 3];
      port[xword + 3]:=xbyte2 or $80;
      xword2:=cbw(Port[xword], Port[xword + 1]);
      if xword2 = 0 then
        Writeln('N/A')
      else
        Writeln(tick2 / xword2:0:0);
      port[xword + 3]:=xbyte2;
      Writeln((xbyte2 and $03) + 5);
      case xbyte2 and $38 of
        $00, $10, $20, $30 : writeln('none');
        $08 : writeln('odd');
        $18 : writeln('even');
        $28 : writeln('mark');
        $38 : writeln('space')
      end;
      case xbyte2 and $07 of
        $00..$03 : writeln('1');
        $04 : writeln('1.5');
        $05..$07 : writeln('2')
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
        end
      end
    end
  end;
