unit page_04;

interface

uses crt, ifpglobl, ifpcomon;

procedure page04;

implementation

procedure page04;

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
  xword:=MemW[ownerPID:$002C];
  if ownerPID = $0008 then
    if osmajor = 5 then
      if MemW[MCB:8] = $4353 then
        xstring:='DOS code'
      else
        if MemW[MCB:8] = $4453 then
          xstring:='DOS data'
        else
          xstring:='DOS'
    else
      xstring:='DOS'
  else
    if ownerPID=$0006 then
      xstring:='DRDOS UMB'
    else
      if ownerPID=$0007 then
        xstring:='DRDOS UMB hole'
  else
    if ownerPID = parent then
      with regs do
        begin
        AX:=$D44D;
        BX:=0;
        Intr($2F, regs);
        if AX = $44DD then
          xstring:='4DOS.COM'
        else
          begin
          AX:=$E44D;
          BX:=0;
          Intr($2F, regs);
          if AX = $44EE then
            xstring:='NDOS.COM'
          else
            xstring:='COMMAND.COM';
          end
        end
(*  BIX ms.dos/secrets #1496  *)
(*  Software Tools #145, p. 56  *)
    else
      if (ownerPID = $0000) {or (ownerPID = PrefixSeg)} then
        xstring:='(free)'
      else
        begin
        i:=0;
        while MemW[xword:i] > $0000 do
          Inc(i);
        Inc(i, 4);
        xstring:='';
        xbool:=false;
        repeat
          xchar:=Chr(Mem[xword:i]);
          if xchar in pchar then
            begin
            if xchar in dirsep then
              xstring:=''
            else
              xstring:=xstring + xchar;
            Inc(i)
            end
          else
            begin
            xbool:=true;
            if xchar > #0 then
              xstring:=''
            end
        until xbool;
        end;
  Write(hex(MCB, 4), '   ', hex(ownerPID, 4), '   ', hex(parent, 4), '  '
    , '   ', xlong1 : 6, '   ');
  if xword = MCB + 1 then
    write(' þ ')
  else
    write('   ');
  Write('   ', xstring);
  if MCB + 1 = ownerPID then
    begin
    for i:=length(xstring) + 1 to 12 do
      Write(' ');
    Write('  ');
    xlong2:=longint(ownerPID) shl 4;
    for i:=$00 to $FF do
      begin
      xlong3:=longint(intvec[i]) and $FFFF0000 shr 12
        + longint(intvec[i]) and $0000FFFF;
      if (xlong2 <= xlong3) and (xlong3 <= xlong2 + xlong1) then
        begin
        if wherex > twidth - 3 then
          begin
          writeln;
          pause2;
          if endit then
            Exit;
          Write('                                                  '
            , '  ');
          end;
        Write(' ', hex(i, 2))
        end
      end
    end;
  writeln
  end;

  begin (* procedure page_04 *)
  caption1('MCB    PSP    Parent     Size   Env   Owner'
    + '          Interrupts');
  window(1, 4, twidth, tlength - 2);
  xword1:=MemW[devseg : devofs - $0002];
  xbool:=false;
  repeat
    xbyte:=Mem[xword1 : $0000];
    xword2:=MemW[xword1 : $0001];
    xword3:=MemW[xword2 : $0016];
    pause2;
    if endit then
      Exit;
    case xbyte of
      $4D : begin
            xword4:=MemW[xword1 : $0003];
            showMCB(xword1, xword2, xword3, xword4);
            Inc(xword1, 1 + xword4)
            end;
      $5A : begin
            xword4:=DOSmem shr 4 - xword1 - 1;
            showMCB(xword1, xword2, xword3, xword4);
            xbool:=true
            end
    else
      begin
      unknown('MCB status', xbyte, 2);
      xbool:=true
      end
    end {case}
  until xbool
  (*  PC Magazine 6:14 p.425  *)
  end;
end.