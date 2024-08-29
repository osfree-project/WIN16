unit page_16;

interface

uses crt, ifpglobl, ifpcomon, ifpextrn;

procedure page16;

implementation

procedure page16;
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
    bpbsize: word;
    ch2: char2;
    direc: directions;

  begin
  Window(1, 3, twidth div 2, tlength - 2);
  Caption1('Boot record of ');
  drvname(currdrv);
  Writeln;
  xword1:=diskread(currdrv, 0, 1, bootrec);
  if xword1 = $0000 then
    begin
    media(bootrec[$15], bootrec[$0D]);
    Caption3('Sectors/cluster');
    Writeln(bootrec[$0D]);
    Caption3('Bytes/sector');
    Writeln(cbw(bootrec[$0B], bootrec[$0C]));
    Caption3('Reserved sectors');
    Writeln(cbw(bootrec[$0E], bootrec[$0F]));
    Caption3('FAT''s');
    Writeln(bootrec[$10]);
    Caption3('Sectors/FAT');
    Writeln(cbw(bootrec[$16], bootrec[$17]));
    Caption3('Root directory entries');
    Writeln(cbw(bootrec[$11], bootrec[$12]));
    Writeln;
    Caption3('Heads');
    Writeln(cbw(bootrec[$1A], bootrec[$1B]));
    Caption3('Total sectors');
    xword1:=bootrec[$13] + word(bootrec[$14] shl 8);
    if xword1 = 0 then
      begin
      l:=bootrec[$20] + (bootrec[$21] * $100) +
        (bootrec[$22] * $10000) + (bootrec[$23] * $1000000);
      Writeln(l)
      end
    else
      Writeln(cbw(bootrec[$13], bootrec[$14]));
    Caption3('Sectors/track');
    Writeln(cbw(bootrec[$18], bootrec[$19]));
    Caption3('Hidden sectors');
    if xword1 = 0 then
      begin
      l:=bootrec[$1C] + (bootrec[$1D] * $100) +
        (bootrec[$1E] * $10000) + (bootrec[$1F] * $1000000);
      Writeln(l)
      end
    else
      Writeln(cbw(bootrec[$1C], bootrec[$1D]));
    Caption3('OEM name and version');
    for i:=$03 to $0A do
      Write(showchar(Chr(bootrec[i])));
    Writeln;
    Caption3('Extended boot record');
    if (osmajor >= 4) and (bootrec[$26] = $29) then
      begin
      Writeln('yes');
      Caption3('Physical drive number');
      Writeln(bootrec[$24]);
      Caption3('Volume label');
      for j:=$2B to $35 do
        Write(showchar(Chr(bootrec[j])));
      Writeln;
      Caption3('Serial Number');
      Writeln(hex(cbw(bootrec[$29], bootrec[$2A]), 4), '-',
        hex(cbw(bootrec[$27], bootrec[$28]), 4));
      Caption3('FAT type');
      for j:=$36 to $3D do
        Write(showchar(Chr(bootrec[j])))
      end
    else
      Writeln('no')
    end
  else
    begin
    Writeln('  Can''t read boot record');
    Write('  ');
    xbyte:=hi(xword1);
    case xbyte of
      $80 : Writeln('Attachment failed to respond');
      $40 : Writeln('Seek operation failed');
      $20 : Writeln('Controller failed');
      $10 : Writeln('Data error (bad CRC)');
      $08 : Writeln('DMA failure');
      $04 : Writeln('Sector not found');
      $03 : Writeln('Write-protect fault');
      $02 : Writeln('Bad address mark');
      $01 : Writeln('Bad command');
      $00 : Writeln
      else
        unknown('error', xbyte, 2)
    end;
    Write('  ');
    xbyte:=lo(xword1);
    case xbyte of
      $00 : Writeln('Write-protect error');
      $01 : Writeln('Unknown unit');
      $02 : Writeln('Drive not ready');
      $03 : Writeln('Unknown command');
      $04 : Writeln('Data error (bad CRC)');
      $05 : Writeln('Bad request structure length');
      $06 : Writeln('Seek error');
      $07 : Writeln('Unknown media type');
      $08 : Writeln('Sector not found');
      $09 : Writeln('Printer out of paper');
      $0A : Writeln('Write fault');
      $0B : Writeln('Read fault');
      $0C : Writeln('General failure')
      else
        unknown('error', xbyte, 2)
    end;
    if osmajor >= 10 then
      begin
      Writeln;
      xbyte:=TextAttr;
      TextColor(LightRed);
      Writeln('**NOTICE**');
      TextAttr:=xbyte;
      Writeln('Information for SUBST''d drives');
      Writeln('is not available under OS/2.');
      Writeln('If you recieved an Unknown unit');
      Writeln('error, change to a real drive');
      Writeln('and try INFOPLUS again.');
      end;
    end;
  Window(1 + twidth div 2, 3, twidth, tlength - 2);
  i:=1;
  xbool:=false;
  xword1:=MemW[devseg : devofs + $0018];
  xword2:=MemW[devseg : devofs + $0016];
  if not ((xword1 = $FFFF) and (xword2 = $FFFF)) then
    begin
    if (osmajor >= 4) and (osmajor < 10) then
      begin
      xbyte:=1;
      bpbsize:=$58
      end
    else
      begin
      xbyte:=0;
      bpbsize:=$51
      end;
    repeat
      Caption1('DOS disk parameter block for ');
      xword2:=MemW[devseg : devofs + $0016] + ((i - 1) * bpbsize);
      drvname(i - 1);
      Writeln;
      xword3:=MemW[xword1 : xword2 + $0047];
      xword4:=MemW[xword1 : xword2 + $0045];
      media(Mem[xword3 : xword4 + $0016 + xbyte],
            Mem[xword3 : xword4 + $0004] + 1);
      Caption3('Sectors/cluster');
      Writeln(Mem[xword3 : xword4 + $0004] + 1);
      Caption3('Bytes/sector');
      Writeln(MemW[xword3 : xword4 + $0002]);
      Caption3('Reserved sectors');
      Writeln(MemW[xword3 : xword4 + $0006]);
      Caption3('FAT''s');
      Writeln(Mem[xword3 : xword4 + $0008]);
      Caption3('Sectors/FAT');
      if (osmajor >= 4) and (osmajor <10) then
        Writeln(Mem[xword3:xword4 + $000F] +
            (Mem[xword3:xword4 + $0010] * 256))
      else
        Writeln(Mem[xword3 : xword4 + $000F]);
      Caption3('Root directory entries');
      Writeln(MemW[xword3 : xword4 + $0009]);
      Writeln;
      Caption3('DPB valid');
      yesorno(Mem[xword3 : xword4 + $0017 + xbyte] < $FF);
      Caption3('Current directory');
      j:=xword2;
      xchar:=Chr(Mem[xword1 : j]);
      while xchar > #0 do begin
        Write(xchar);
        inc(j);
        xchar:=Chr(Mem[xword1 : j])
      end;
      Writeln;
      Caption3('Device header');
      segofs(MemW[xword3 : xword4 + $0014 + xbyte],
        MemW[xword3 : xword4 + $0012 + xbyte]);
      Writeln;
      Caption3('Unit within driver');
      Writeln(Mem[xword3 : xword4 + $0001]);
      Caption3('Clusters');
      Writeln(MemW[xword3 : xword4 + $000D] - 1);
      Caption3('Cluster to sector shift');
      Writeln(Mem[xword3 : xword4 + $0005]);
      Caption3('Root directory sector');
      Writeln(MemW[xword3 : xword4 + $0010 + xbyte]);
      Caption3('First data sector');
      Writeln(MemW[xword3 : xword4 + $000B]);
      Caption3('Next DPB');
      xword5:=MemW[xword3 : xword4 + $0018 + xbyte];
      segofs(MemW[xword3 : xword4 + $001A + xbyte], xword5);
      Writeln;
      if i = 1 then
        direc:=down
      else
        if (i = lastdrv) or (xword5 = $FFFF) then
          direc:=up
        else
          direc:=updown;
      Write('  ');
      pause4(direc, ch2);
      if endit then
        Exit;
      if (ch2 = #0#72) and (i > 1) then
        Dec(i)
      else
        if (ch2 = #0#80) and (i < lastdrv) and (xword5 < $FFFF) then
          Inc(i);
      ClrScr;
    until xbool
    end;
  end;
end.