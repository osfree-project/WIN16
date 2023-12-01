procedure page_16;
  var
    bootrec : array[0..secsiz - 1] of byte;
    i : 1..26;
    j : word;
    l : longint;
    xbool : boolean;
    xbyte : byte;
    xchar : char;
    xword1 : word;
    xword2 : word;
    xword3 : word;
    xword4 : word;
    xword5 : word;

  begin
  window(1, 3, twidth div 2, tlength - 2);
  caption1('Boot record of ');
  drvname(currdrv);
  writeln;
  xword1:=diskread(currdrv, 0, 1, bootrec);
  if xword1 = $0000 then
    begin
    media(bootrec[$15], bootrec[$0D]);
    caption3('Sectors/cluster');
    writeln(bootrec[$0D]);
    caption3('Bytes/sector');
    writeln(cbw(bootrec[$0B], bootrec[$0C]));
    caption3('Reserved sectors');
    writeln(cbw(bootrec[$0E], bootrec[$0F]));
    caption3('FAT''s');
    writeln(bootrec[$10]);
    caption3('Sectors/FAT');
    writeln(cbw(bootrec[$16], bootrec[$17]));
    caption3('Root directory entries');
    writeln(cbw(bootrec[$11], bootrec[$12]));
    writeln;
    caption3('Heads');
    writeln(cbw(bootrec[$1A], bootrec[$1B]));
    caption3('Total sectors');
    xword1:=bootrec[$13] + word(bootrec[$14] shl 8);
    if xword1 = 0 then
      begin
      l:=bootrec[$20] + (bootrec[$21] * $100) +
        (bootrec[$22] * $10000) + (bootrec[$23] * $1000000);
      Writeln(l)
      end
    else
      writeln(cbw(bootrec[$13], bootrec[$14]));
    caption3('Sectors/track');
    writeln(cbw(bootrec[$18], bootrec[$17]));
    caption3('Hidden sectors');
    if xword1 = 0 then
      begin
      l:=bootrec[$1C] + (bootrec[$1D] * $100) +
        (bootrec[$1E] * $10000) + (bootrec[$1F] * $1000000);
      Writeln(l)
      end
    else
      writeln(cbw(bootrec[$1C], bootrec[$1D]));
    caption3('OEM name and version');
    for i:=$03 to $0A do
      write(showchar(chr(bootrec[i])));
    writeln;
    caption3('Extended boot record');
    if (osmajor >= 4) and (bootrec[$26] = $29) then
      begin
      Writeln('yes');
      caption3('Physical drive number');
      Writeln(bootrec[$24]);
      caption3('Volume label');
      for j:=$2B to $35 do
        Write(showchar(Chr(bootrec[j])));
      Writeln;
      caption3('Serial Number');
      Writeln(hex(cbw(bootrec[$29], bootrec[$2A]), 4), '-',
        hex(cbw(bootrec[$27], bootrec[$28]), 4));
      caption3('FAT type');
      for j:=$36 to $3D do
        Write(showchar(Chr(bootrec[j])))
      end
    else
      Writeln('no')
    end
  else
    begin
    writeln('  Can''t read boot record');
    write('  ');
    xbyte:=hi(xword1);
    case xbyte of
      $80 : writeln('Attachment failed to respond');
      $40 : writeln('Seek operation failed');
      $20 : writeln('Controller failed');
      $10 : writeln('Data error (bad CRC)');
      $08 : writeln('DMA failure');
      $04 : writeln('Sector not found');
      $03 : writeln('Write-protect fault');
      $02 : writeln('Bad address mark');
      $01 : writeln('Bad command');
      $00 : writeln
      else
        unknown('error', xbyte, 2)
    end;
    write('  ');
    xbyte:=lo(xword1);
    case xbyte of
      $00 : writeln('Write-protect error');
      $01 : writeln('Unknown unit');
      $02 : writeln('Drive not ready');
      $03 : writeln('Unknown command');
      $04 : writeln('Data error (bad CRC)');
      $05 : writeln('Bad request structure length');
      $06 : writeln('Seek error');
      $07 : writeln('Unknown media type');
      $08 : writeln('Sector not found');
      $09 : writeln('Printer out of paper');
      $0A : writeln('Write fault');
      $0B : writeln('Read fault');
      $0C : writeln('General failure')
      else
        unknown('error', xbyte, 2)
    end
    end;
  window(1 + twidth div 2, 3, twidth, tlength - 2);
  i:=1;
  xbool:=false;
  xword1:=memw[devseg : devofs + $0018];
  xword2:=memw[devseg : devofs + $0016];
  repeat
    if osmajor >= 4 then
      xbyte:=1
    else
      xbyte:=0;
    caption1('DOS disk parameter block for ');
    drvname(i - 1);
    writeln;
    xword3:=memw[xword1 : xword2 + $0047];
    xword4:=memw[xword1 : xword2 + $0045];
    media(mem[xword3 : xword4 + $0016 + xbyte],
          mem[xword3 : xword4 + $0004] + 1);
    caption3('Sectors/cluster');
    writeln(mem[xword3 : xword4 + $0004] + 1);
    caption3('Bytes/sector');
    writeln(memw[xword3 : xword4 + $0002]);
    caption3('Reserved sectors');
    writeln(memw[xword3 : xword4 + $0006]);
    caption3('FAT''s');
    writeln(mem[xword3 : xword4 + $0008]);
    caption3('Sectors/FAT');
    if osmajor >= 4 then
      Writeln(Mem[xword3:xword4 + $000F] +
          (Mem[xword3:xword4 + $0010] * 256))
    else
      writeln(mem[xword3 : xword4 + $000F]);
    caption3('Root directory entries');
    writeln(memw[xword3 : xword4 + $0009]);
    writeln;
    caption3('DPB valid');
    yesorno(mem[xword3 : xword4 + $0017 + xbyte] < $FF);
    caption3('Current directory');
    j:=xword2;
    xchar:=chr(mem[xword1 : j]);
    while xchar > #0 do begin
      write(xchar);
      inc(j);
      xchar:=chr(mem[xword1 : j])
    end;
    writeln;
    caption3('Device header');
    segofs(memw[xword3 : xword4 + $0014 + xbyte],
      memw[xword3 : xword4 + $0012 + xbyte]);
    writeln;
    caption3('Unit within driver');
    writeln(mem[xword3 : xword4 + $0001]);
    caption3('Clusters');
    writeln(memw[xword3 : xword4 + $000D] - 1);
    caption3('Cluster to sector shift');
    writeln(mem[xword3 : xword4 + $0005]);
    caption3('Root directory sector');
    writeln(memw[xword3 : xword4 + $0010 + xbyte]);
    caption3('First data sector');
    writeln(memw[xword3 : xword4 + $000B]);
    caption3('Next DPB');
    xword5:=memw[xword3 : xword4 + $0018 + xbyte];
    segofs(memw[xword3 : xword4 + $001A + xbyte], xword5);
    writeln;
    if (i < lastdrv) and (xword5 < $FFFF) then
      begin
      write('  ');
      pause1;
      if endit then
        Exit;
      clrscr;
      inc(i);
      if osmajor >= 4 then
        Inc(xword2, $58)
      else
        inc(xword2, $51)
      end
    else
      xbool:=true
  until xbool
  end;
