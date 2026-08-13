unit page_10;
interface

uses crt, dos, ifpglobl, ifpcomon;

procedure page10;

implementation

procedure page10;

var
  i : 1..63;
  xbool1 : boolean;
  xbool2 : boolean;
  xbool3 : boolean;
  xchar : char;
  xword1: word;
  xword2: word;
  regs: registers;
  xbyte: byte;
  count: word;
  s: string;

procedure muxint(a : string; b : byte);
  begin
  caption3(a);
  with regs do
    begin
    AH:=b;
    AL:=0;
    BX:=0;
    CX:=0;
    DX:=0;
    Intr($2F, regs);
    case AL of
      $00 : Writeln('no; OK to load');
      $01 : Writeln('no; not OK to load');
      $FF, 2 : Writeln('yes')
      else
        unknown('status', AL, 2)
    end
    end
  end;

function windev(device: byte): word;
  var
    regs: registers;
    saveit: word;

  begin
  with regs do
    begin
    AX:=$1682;
    Intr($2F, regs);
    AX:=$170A;
    DX:=device;
    Intr($2F, regs);
    windev:=AX
    end
  end;

  begin (* procedure page_10 *)
  caption1('');
  Writeln('Multiplex interrupt ($2F)');
  Window(1, 4, twidth div 2, tlength - 2);
  muxint('DOS            ', $12);
  muxint('DRIVER.SYS     ', $08);
  muxint('DISPLAY.SYS    ', $B0);
  muxint('ANSI.SYS       ', $1A);
  muxint('EGA.SYS        ', $BC);
  muxint('PRINT          ', $01);
  muxint('ASSIGN         ', $06);
(*
**  Byte 12:12 p. 176C, Duncan, and many others, all of whom mistakenly give
**  AH = $02
*)
  muxint('SHARE          ', $10);
(*  muxint('FASTOPEN       ', $xx);*)
  muxint('NLSFUNC        ', $14);
  muxint('GRAFTABL (4.0-)', $B0);
  caption3('GRAFTABL (5.0+)');
  with regs do
    begin
    AX:=$2300;
    BX:=0;
    CX:=0;
    DX:=0;
    Intr($2F, regs);
    if AH=$FF then
      Writeln('yes')
    else
      Writeln('no; OK to load');
    end;
(*  muxint('KEYB           ', $B8);*)
  muxint('NETBIOS append ', $87);
  muxint('NETBIOS network', $88);
(*  Byte 12:12 p. 180.  PC Tech Journal 3:11 p.104 gives AH = $BB  *)
  muxint('SHELLB         ', $19);
  muxint('XMA2EMS        ', $1B);
  muxint('APPEND         ', $B7);
  muxint('GRAPHICS.COM   ', $15);
  muxint('Crit.err.handlr', $05);
  pause3(-2);
  if endit then
    Exit;
  caption3('CDROM          ');
  with regs do
    begin
    AX:=$1500;
    BX:=0;
    Intr($2F, regs);
    if BX = 0 then
      Writeln('no; OK to load')
    else
      begin
      Writeln('yes');
      caption3('  on drives');
      Write(Chr(CX + 65));
      caption3('through');
      Writeln(Chr(CX + BX + 65 - 1))
      end;
    end;
  caption3('Network        ');
  with regs do
    begin
    AX:=$B800;
    Intr($2F, regs);
    if AL = 0 then
      Writeln('no; OK to load')
    else
      begin
      Write('yes');
      caption3('is a');
      if BX and $0040 = $40 then
        Writeln('server')
      else
        if BX and $0004 = $4 then
          Writeln('messenger')
        else
          if BX and $0080 = $80 then
            Writeln('receiver')
          else
            if BX and $0008 = $8 then
              Writeln('redirector');
      end;
    end;
  muxint('DOSKEY         ', $48);
  caption3('DOS Extender   ');
  with regs do
    begin
    AX:=$F100;
    BX:=0;
    CX:=0;
    DX:=0;
    Intr($2F, regs);
    if (AL = $FF) and (SI =$444F {DO}) and (DI = $5358 {SX}) then
      Writeln('yes')
    else
      Writeln('no; OK to load');
    end;
  window(1 + twidth div 2, 3, twidth, tlength - 3);
  if osmajor >=4 then
    with regs do
      begin
      AX:=$B700;
      Intr($2F, regs);
      if AL=$FF then
        begin
        caption2('APPEND ');
        AX:=$B706;
        Intr($2F, regs);
        if (BX and 1) = 1 then
          Write('enabled ');
        if (BX and $2000) = $2000 then
          Write('/PATH ');
        if (BX and $4000) = $4000 then
          Write('/E ');
        if (BX and $8000) = $8000 then
          Write('/X');
        Writeln;
        caption2('APPEND path');
        AX:=$B704;
        Intr($2F, regs);
        while Mem[ES:DI] <> 0 do
          begin
          Write(Chr(mem[ES:DI]));
          Inc(DI)
          end;
        Writeln
        end
      end;
  with regs do
    begin
    AX:=$0100;
    intr($2F, regs);
    if AL = $FF then
      begin
      caption2('PRINT queue');
      AX:=$0104;
      intr($2F, regs);
      xbool1:=true;
      xbool2:=false;
      repeat
        xchar:=char(mem[DS : SI]);
        if xchar > #0 then
          begin
          if xbool1 then
            begin
            xbool1:=false;
            Writeln;
            window(2 + twidth div 2, wherey + hi(windmin), twidth, tlength - 3)
            end;
          pause2;
          if endit then
            Exit;
          Write(xchar);
          i:=1;
          xbool3:=false;
          repeat
            xchar:=char(mem[DS : SI + i]);
            if xchar > #0 then
              begin
              Write(xchar);
              inc(i)
              end
            else
              begin
              Writeln;
              xbool3:=true
              end
          until xbool3;
          inc(SI, 64)
          end
        else
          xbool2:=true
      until xbool2;
      if xbool1 then
        Writeln('(empty)');
      AX:=$0105;
      Intr($2F, regs)
      end;
    end;
  if osmajor = 5 then
    begin
    xword1:=MemW[devseg:devofs + $39];
    xword2:=MemW[devseg:devofs + $37];
    if (xword1 <> 0) and (xword2 <> 0) then
      begin
      caption2('SETVER list at ');
      segofs(xword1, xword2);
      Writeln;
      while Mem[xword1:xword2] <> 0 do
        begin
        xbyte:=Mem[xword1:xword2];
        Inc(xword2);
        s:='';
        for count:=xword2 to xword2 + (xbyte - 1) do
          s:=s + Chr(Mem[xword1: count]);
        Inc(xword2, xbyte);
        Write(s, ' ':(14 - Length(s)), Mem[xword1:xword2], decimal);
        zeropad(mem[xword1:xword2 + 1]);
        Writeln;
        pause2;
        if endit then
          Exit;
        Inc(xword2, 2);
        end;
      end
   end;
   end;
end.
