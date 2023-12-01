procedure page_04;

var
  xbool : boolean;
  xbyte : byte;
  xword1 : word;
  xword2 : word;
  xword3 : word;
  xword4 : word;

procedure showMCB(MCB, ownerPID, parent, size : word);

var
  i : word;
  xbool : boolean;
  xchar : char;
  xlong1 : longint;
  xlong2 : longint;
  xlong3 : longint;
  xstring : string;
  xword : word;

begin
  xlong1:=longint(size) shl 4;
  xword:=memw[ownerPID : $002C];
  if ownerPID = $0008 then
    xstring:='IBMDOS.COM'
  else if ownerPID = parent then
    xstring:='COMMAND.COM'
(*  BIX ms.dos/secrets #1496  *)
(*  Software Tools #145, p. 56  *)
  else if (ownerPID = $0000) or (ownerPID = prefixseg) then
    xstring:='(free)'
  else begin
    i:=0;
    while memw[xword : i] > $0000 do
      inc(i);
    inc(i, 4);
    xstring:='';
    xbool:=false;
    repeat
      xchar:=chr(mem[xword : i]);
      if xchar in pchar then begin
        if xchar in dirsep then
          xstring:=''
        else
          xstring:=xstring + xchar;
        inc(i)
      end else begin
        xbool:=true;
        if xchar > #0 then
          xstring:=''
      end
    until xbool;
  end;
  write(hex(MCB, 4), '   ', hex(ownerPID, 4), '   ', hex(parent, 4), '  '
    , '   ', xlong1 : 6, '   ');
  if xword = MCB + 1 then
    write(' þ ')
  else
    write('   ');
  write('   ', xstring);
  if MCB + 1 = ownerPID then begin
    for i:=length(xstring) + 1 to 12 do
      write(' ');
    write('  ');
    xlong2:=longint(ownerPID) shl 4;
    for i:=$00 to $FF do begin
      xlong3:=longint(intvec[i]) and $FFFF0000 shr 12
        + longint(intvec[i]) and $0000FFFF;
      if (xlong2 <= xlong3) and (xlong3 <= xlong2 + xlong1) then begin
        if wherex > twidth - 3 then begin
          writeln;
          pause2;
          if endit then
            Exit;
          write('                                                  '
            , '  ');
        end;
        write(' ', hex(i, 2))
      end
    end
  end;
  writeln
end;

begin (* procedure page_04 *)
  caption1('MCB    PSP    Parent     Size   Env   Owner'
    + '          Interrupts');
  window(1, 4, twidth, tlength - 2);
  xword1:=memw[devseg : devofs - $0002];
  xbool:=false;
  repeat
    xbyte:=mem[xword1 : $0000];
    xword2:=memw[xword1 : $0001];
    xword3:=memw[xword2 : $0016];
    pause2;
    if endit then
      Exit;
    case xbyte of
      $4D : begin
        xword4:=memw[xword1 : $0003];
        showMCB(xword1, xword2, xword3, xword4);
        inc(xword1, 1 + xword4)
      end;
      $5A : begin
        xword4:=DOSmem shr 4 - xword1 - 1;
        showMCB(xword1, xword2, xword3, xword4);
        xbool:=true
      end else begin
        unknown('MCB status', xbyte, 2);
        xbool:=true
      end
    end
  until xbool
(*  PC Magazine 6:14 p.425  *)
end;
