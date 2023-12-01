procedure page_10;

var
  i : 1..63;
  xbool1 : boolean;
  xbool2 : boolean;
  xbool3 : boolean;
  xchar : char;
  xword1: word;

procedure muxint(a : string; b : byte);
  begin
  caption3(a);
  with regs do
    begin
    AH:=b;
    AL:=0;
    intr($2F, regs);
    case AL of
      $00 : writeln('no; OK to load');
      $01 : writeln('no; not OK to load');
      $FF : writeln('yes')
      else
        unknown('status', AL, 2)
    end
    end
  end;

  begin (* procedure page_10 *)
  caption2('Multiplex interrupt ($2F)');
  Window(1, 4, twidth div 2, tlength - 2);
  muxint('DOS            ', $12);
{ muxint('DRIVER.SYS     ', $08);}
  muxint('DISPLAY.SYS    ', $B0);
  muxint('ANSI.SYS       ', $1A);
  muxint('PRINT          ', $01);
  muxint('ASSIGN         ', $06);
(*
**  Byte 12:12 p. 176C, Duncan, and many others, all of whom mistakenly give
**  AH = $02
*)
  muxint('SHARE          ', $10);
(*  muxint('FASTOPEN       ', $xx);*)
  muxint('NLSFUNC        ', $14);
  muxint('GRAFTABL       ', $B0);
(*  muxint('KEYB           ', $B8);*)
  muxint('NETBIOS append ', $87);
  muxint('NETBIOS network', $88);
(*  Byte 12:12 p. 180.  PC Tech Journal 3:11 p.104 gives AH = $BB  *)
  muxint('SHELLB         ', $19);
  muxint('XMA2EMS        ', $1B);
  muxint('APPEND         ', $B7);
  caption3('CDROM          ');
  with regs do
    begin
    AX:=$1500;
    BX:=0;
    Intr($2F, regs);
    case AL of
      $00 : writeln('no; OK to load');
      $01 : writeln('no; not OK to load');
      $FF : begin
            Write('yes');
            caption3('on drives');
            Write(Chr(CX + 65));
            caption3('through');
            Write(Chr(CX + BX + 65 - 1))
            end;
    end; {case}
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
      caption3('this is a');
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
  pause3(3);
  caption3('JP Software''s 4DOS');
  with regs do
    begin
    AX:=$D44D;
    BX:=0;
    Intr($2F, regs);
    if AX <> $44DD then
      Writeln('no')
    else
      begin
      Writeln('yes');
      caption3('  version');
      xword1:=BH;
      Write(BL, decimal);
      zeropad(xword1);
      caption3('shell no.');
      Writeln(DL);
      caption3('  4DOS PSP segment');
      Writeln(hex(CX, 4))
      end;
    end;
  pause3(2);
  caption3('Microsoft Windows');
  with regs do
    begin
    AX:=$1600;
    Intr($2F, regs);
    case AL of
    $01,$FF: begin
             Writeln('yes');
             caption3('  version');
             Writeln('Windows/386 2.x')
             end;
    $00,$80: begin
             AX:=$4680;
             Intr($2F, regs);
             if AX = 0 then
               begin
               Writeln('yes');
               caption3('  mode');
               Writeln('Real or Standard')
               end
             else
               Writeln('no');
             end;
    $02..$7F,$81..$FE: begin
                       Writeln('yes');
                       caption3('  version');
                       Writeln(AL, decimal, AH, ' enhanced mode');
                       end;
    end; {case}
    end;
  caption3('Quarterdeck''s Desqview');
  with regs do
    begin
    AX:=$2B01;
    CX:=$4445; {DE}
    DX:=$5351; {SQ}
    MsDos(regs);
    if AL = $FF then
      Writeln('no')
    else
      begin
      Writeln('yes');
      caption3('  version');
      if BX = $0002 then
        Writeln('2', decimal, '00')
      else
        begin
        Write(BH, decimal);
        zeropad(BL);
        end;
      end;
    end;
  window(1 + twidth div 2, 3, twidth, tlength - 2);
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
  with regs do begin
    AX:=$0100;
    intr($2F, regs);
    if AL = $FF then begin
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
            writeln;
            window(2 + twidth div 2, wherey + hi(windmin), twidth, tlength - 2)
            end;
          pause2;
          if endit then
            Exit;
          write(xchar);
          i:=1;
          xbool3:=false;
          repeat
            xchar:=char(mem[DS : SI + i]);
            if xchar > #0 then
              begin
              write(xchar);
              inc(i)
              end
            else
              begin
              writeln;
              xbool3:=true
              end
          until xbool3;
          inc(SI, 64)
          end
        else
          xbool2:=true
      until xbool2;
      if xbool1 then
        writeln('(empty)');
      AX:=$0105;
      intr($2F, regs)
    end
  end
end;
