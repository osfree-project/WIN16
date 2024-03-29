unit ifprun;

interface

uses crt, dos, ifpglobl, ifpcomon, ifpinit, ifpscrpt, ifphelp,
  page_00, page_01, page_02, page_03, page_04, page_05, page_06, page_07,
  page_08, page_09, page_10, page_11, page_12, page_13, page_14, page_15,
  page_16, page_17, page_18, page_19, page_20, page_21;

procedure runit;

implementation

procedure runit;
  begin
  with regs do
    begin
    AX:=$3306;
    MsDos(regs);
    if (AL = $06) and (BH < 100) and (BL >= 5) then
      xword:=BX
    else
      xword:=DosVersion;
    end;
  osmajor:=Lo(xword);
  osminor:=Hi(xword);
  if osmajor >= 3 then
    begin
    init;
    xbool1:=false;
    repeat
      pagenameclr;
      GotoXY(x1, tlength);
      TextColor(lightgray);
      Write(pg:2, ' - ');
      Write(pgnames[pg]);
      Window(1, 3, twidth, tlength - 2);
      Clrscr;
      case pg of
	0 : page00;
	1 : page01;
	2 : page02;
	3 : page03;
	4 : page04;
	5 : page05;
	6 : page06;
	7 : page07;
	8 : page08;
	9 : page09;
	10 : page10;
	11 : page11;
	12 : page12;
	13 : page13;
	14 : page14;
	15 : page15;
	16 : page16;
	17 : page17;
	18 : page18;
        19 : page19;
        20 : page20;
        21 : page21
      end;
      xbool2:=false;
      repeat
        Window(1, 1, twidth, tlength);
        GotoXY(x2 - 1, tlength);
        quiet:=false;
        if PrinterRec.Mode <> 'A' then
          begin
          if not endit then
            begin
            repeat
            until KeyPressed;
            xchar1:=ReadKey;
            if KeyPressed then
              xchar2:=ReadKey
            else
              xchar2:=#0;
            end
          else
            begin
            endit:=false;
            xchar1:=c2[1];
            if Length(c2) = 1 then
              xchar2:=#0
            else
              xchar2:=c2[2]
            end;
          end
        else
          begin
          ScreenPrint(Pg, PgNames[Pg], VerNum);
          xchar1:=#0;
          if Pg = PgMax then
            begin
            PrinterRec.Mode:='S';
            xchar2:=#71;
            end
          else
            xchar2:=#81
          end;
        if (xchar1 = #27) and (xchar2 = #0) then
          begin
          xbool2:=true;
          xbool1:=true
          end;
        if (xchar1 = #13) and (xchar2 = #0) then
          begin
          pagenameclr;
          GotoXY(x1, tlength);
          TextColor(White);
          Write('Go to page no.=> ');
          i:=getnum;
          if (i >= 0 ) and (i <= pgmax) then
            begin
            pg:=i;
            xbool2:=true
            end;
          if i = 999 then
            xbool2:=true;
          pagenameclr;
          GotoXY(x1, tlength);
          TextColor(lightgray);
          Write(pg:2, ' - ');
          Write(pgnames[pg]);
          end;
        if xchar1 = #0 then
          case xchar2 of
            #71: begin
                 xbool2:=true;
                 pg:=0
                 end;
            #73: if pg > 0 then
                   begin
                   xbool2:=true;
                   Dec(pg)
                   end;
            #79: begin
                 xbool2:=true;
                 pg:=pgmax
                 end;
            #81: if pg < pgmax then
                   begin
                   xbool2:=true;
                   Inc(pg)
                   end;
            #25: begin
                 screenprint(pg, pgnames[pg], vernum);
                 xbool2:=false;
                 quiet:=true;
                 end;
            #59: begin
                 helpscreen(pg, helpversion);
                 xbool2:=false;
                 quiet:=true;
                 end;
          end;
      if (not xbool2) and (not quiet) then
        begin
        Sound(220);
        Delay(100);
        NoSound
        end
      until xbool2
    until xbool1;
    TextAttr:=attrsave;
    if resetvideo then
      TextMode(vidmode);
    ClrScr
    end
  else
    begin
    Writeln;
    decimal:='.';
    Writeln('INFOPLUS requires DOS version 3.0 or later');
    Write('Your DOS version is ');
    showvers
    end
  end;
end.
