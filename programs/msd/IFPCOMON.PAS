unit ifpcomon;

interface

uses Crt, Dos, ifpglobl, ifpextrn;

function getkey2: char2;
function getnum: word;
procedure caption1(a: string);
procedure caption2(a: string);
procedure caption3(a : string);
function nocarry(regs: registers) : boolean;
function hex(a : word; b : byte) : string;
procedure unknown(a: string; b: word; c: byte);
procedure yesorno(a : boolean);
procedure yesorno2(a: boolean);
procedure yesorno3(a: boolean);
procedure dontknow;
procedure dontknow2;
procedure segofs(a, b : word);
function showchar(a : char) : char;
function power2(y: word): longint;
procedure pause1;
procedure pause2;
procedure pause3(extra: integer);
procedure pause4(direc: directions; var ch2: char2);
procedure pause5(direc: directions; var ch2: char2);
function bin4(a: byte) : string;
procedure offoron(a: string; b: boolean);
procedure zeropad(a: word);
procedure zeropad3(a: word);
procedure showvers;
function cbw(a, b: byte) : word;
function bin16(a: word) : string;
procedure drvname(a: byte);
procedure media(a, b: byte);
procedure pagenameclr;
procedure Intr(intno: byte; var regs: registers);
procedure MsDos(var regs: registers);
procedure TextColor(color: byte);
procedure TextBackground(color: byte);
function unBCD(b: byte): byte;
function addzero(b: byte): string;
procedure modeinfo(var vidmode, vidlen, vidpg: byte; var vidwid: word);
procedure box;
procedure center(s: string);
function EMSOK: boolean;

implementation

uses ifpscrpt, ifphelp;

function getkey2: char2;
  var
    c: char;
    c2: char2;

  begin
  c:=ReadKey;
  if c = #0 then
    getkey2:=c + ReadKey
  else
    getkey2:=c;
  end; {getkey2}

{^Make sure number entered, not any letters}
function getnum: word;
  var
    inpchar: char;
    number_string: string[2];
    temp, position, code: word;
    row, col: byte;
    finish: boolean;

  begin
  row:=WhereY;
  col:=WhereX;
  Write(' ':3);
  GotoXY(col, row);
  temp:=99;
  finish:=false;
  position:=0;
  number_string:='';
  TextColor(LightGray);
  repeat
    inpchar:=ReadKey;
    case inpchar of
      '0'..'9':if position < 2 then
        begin
        Inc(position);
        Inc(number_string[0]);
        number_string[position]:=inpchar;
        Write(inpchar)
        end;
      #8: if position > 0 then
        begin
        Dec(position);
        Dec(number_string[0]);
        Write(^H' '^H)
        end;
      #27: if number_string = '' then
          finish:=true
        else
          begin
          number_string:='';
          GotoXY(col, row);
          ClrEol;
          position:=0
          end;
      #13: finish:=true
    end {case}
  until finish;
  if number_string <> '' then
    Val(number_string, temp, code)
  else
    temp:=999;
  getnum:=temp
  end; {getnum}

procedure caption1(a: string);
  begin
  textcolor(LightGray);
  Write(a);
  textcolor(LightCyan)
  end; {caption1}

procedure caption2(a: string);
  const
    capterm = ': ';

  var
    i: byte;
    xbool: boolean;

  begin
  i:=length(a);
  while (i > 0) and (a[i] = ' ') do
    dec(i);
  insert(capterm, a, i + 1);
  caption1(a)
  end; {caption2}

procedure caption3(a : string);
  begin
  caption2('  ' + a)
  end; {caption3}

function nocarry(regs: registers) : boolean;
  begin
  nocarry:=regs.flags and fcarry = $0000
  end; {nocarry}

function hex(a : word; b : byte) : string;
  const
    digit : array[$0..$F] of char = '0123456789ABCDEF';

  var
    i : byte;
    xstring : string;

  begin
  xstring:='';
  for i:=1 to b do
    begin
    insert(digit[a and $000F], xstring, 1);
    a:=a shr 4
    end;
  hex:=xstring
  end; {hex}

procedure unknown(a: string; b: word; c: byte);
  begin
  Writeln('(unknown', ' ', a, ' ', hex(b, c), ')')
  end; {unknown}

procedure yesorno(a : boolean);
  begin
  if a then
    Writeln('yes')
  else
    Writeln('no')
  end; {yesorno}

procedure yesorno2(a: boolean);
  begin
  if a then
    Write('yes')
  else
    Write('no')
  end; {yesorno2}

procedure YesOrNo3(a: boolean);
  begin
  YesOrNo2(a);
  if not a then
    Write(' ');
  end;

procedure dontknow;
  begin
  Writeln('(unknown)')
  end; {dontknow}

procedure dontknow2;
  begin
  Write('(unknown)')
  end; {dontknow2}

procedure segofs(a, b : word);
  begin
  Write(hex(a, 4), ':', hex(b, 4))
  end; {segofs}

function showchar(a : char) : char;
  begin
  if a in pchar then
    showchar:=a
  else
    showchar:='.'
  end; {showchar}

function power2(y: word): longint;
  begin
  power2:=Trunc(exp((y * 1.0) * ln(2.0)))
  end;

procedure pause1;
  var
    xbyte : byte;
    xchar : char2;
    SaveX, SaveY: byte;

  begin
  xbyte:=TextAttr;
  endit:=false;
  TextColor(Cyan);
  SaveX:=WhereX;
  SaveY:=WhereY;
  Write('( for more)');
  if PrinterRec.Mode = 'A' then
    ScreenPrint(Pg, PgNames[Pg], VerNum)
  else
    begin
    repeat
      xchar:=getkey2;
      if xchar = #0#25 then
        begin
        ScreenPrint(Pg, PgNames[Pg], VerNum);
        xchar:=#0#0
        end;
      if xchar = #0#$3B then
        begin
        HelpScreen(Pg, HelpVersion);
        xchar:=#0#0
        end;
    until xchar <> #0#0;
    if xchar <> #0#80 then
      begin
      endit:=true;
      c2:=xchar
      end;
    end;
  TextAttr:=xbyte;
  GotoXY(SaveX, SaveY);
  Write('            ')
  end; {pause1}

procedure pause2;
  var
    xbyte : byte;

  begin
  if WhereY + hi(WindMin) > hi(WindMax) then
    begin
    xbyte:=TextAttr;
    TextColor(Cyan);
    pause1;
    if not endit then
      begin
      Clrscr;
      Writeln('(continued)');
      end;
    TextAttr:=xbyte
    end
  end; {pause2}

procedure pause3(extra: integer);
  var
    xbyte: byte;
  begin
  endit:=false;
  if WhereY + Hi(WindMin) + Abs(extra) > Hi(WindMax) then
    begin
    xbyte:=TextAttr;
    TextColor(Cyan);
    pause1;
    if not endit then
      begin
      ClrScr;
      if extra < 0 then
        Writeln('(continued)');
      end;
    TextAttr:=xbyte
    end
  end; {pause3}

procedure pause4(Direc: Directions; var ch2: char2);
  var
    xbyte : byte;
    xchar : char2;
    SaveX, SaveY: byte;

  begin
  xbyte:=TextAttr;
  endit:=false;
  TextColor(Cyan);
  SaveX:=WhereX;
  SaveY:=WhereY;
  case Direc of
    none:   Write('(any key)');
    up:     Write('( for more)');
    down:   Write('( for more)');
    updown: Write('( or  for more)')
  end;
  repeat
    if PrinterRec.Mode = 'A' then
      if Direc = up then
        xchar:=#0#81
      else
        begin
        ScreenPrint(Pg, PgNames[Pg], VerNum);
        xchar:=#0#80;
        end
    else
      begin
      xchar:=getkey2;
      if xchar = #0#25 then
        begin
        ScreenPrint(Pg, Pgnames[Pg], VerNum);
        xchar:=#0#0
        end;
      if xchar = #0#$3B then
        begin
        HelpScreen(Pg, HelpVersion);
        xchar:=#0#0
        end;
      end;
  until xchar <> #0#0;
  if (xchar[1] <> #0) or
    ((xchar[1] = #0) and (not (xchar[2] in [#80, #72]))) then
    begin
    endit:=true;
    c2:=xchar;
    end;
  TextAttr:=xbyte;
  GotoXY(SaveX, SaveY);
  Write('                 ');
  ch2:=xchar;
  end; {pause4}

procedure pause5(direc: directions; var ch2: char2);
  var
    xbyte : byte;

  begin
  ch2:=#0#0;
  if WhereY + Hi(WindMin) > Hi(WindMax) then
    begin
    xbyte:=TextAttr;
    TextColor(Cyan);
    Pause4(direc, ch2);
    if not endit then
      Clrscr;
    TextAttr:=xbyte
    end
  end; {pause5}

function bin4(a : byte) : string;
  const
    digit : array[0..1] of char = '01';

  var
    xstring : string;
    i : byte;

  begin
  xstring:='';
  for i:=3 downto 0 do
    begin
    insert(digit[a mod 2], xstring, 1);
    a:=a shr 1
    end;
  bin4:=xstring
  end; {bin4}

procedure offoron(a : string; b : boolean);
  begin
  caption3(a);
  if b then
    Write('on')
  else
    Write('off')
  end; {offoron}

procedure zeropad(a : word);
  begin
  if a < 10 then
    Write('0');
  Write(a)
  end; {zeropad}

 procedure zeropad3(a: word);
   begin
   if a < 10 then
     Write('00')
   else
     if a < 100 then
       Write('0');
   Write(a)
   end; {zeropad3}

procedure showvers;
  begin
  if osmajor > 0 then
    Writeln(osmajor, decimal, addzero(osminor))
  else
    Writeln('1', decimal, 'x')
  end; {showvers}

function cbw(a, b : byte) : word;
  begin
  cbw:=word(b) shl 8 + a
  end; {cbw}

function bin16(a : word) : string;
  function bin8(a : byte) : string;
    begin
    bin8:=bin4(a shr 4) + '_' + bin4(a and $0F)
    end; {bin8}

  begin {bin16}
  bin16:=bin8(hi(a)) + '_' + bin8(lo(a))
  end; {bin16}

procedure drvname(a : byte);
  begin
  Write(chr(ord('A') + a), ': ')
  end; {drvname}

procedure media(a, b : byte);
  procedure diskette(a, b, c : byte);
    begin
    Writeln('floppy ', a, ' side, ', b, ' sctr, ', c, ' trk')
    end; {diskette}

  begin {media}
  caption3('Media');
  case a of
    $FF : diskette(2, 8, 40);
    $FE : diskette(1, 8, 40);
    $FD : diskette(2, 9, 40);
    $FC : diskette(1, 9, 40);
    $F9 : if b = 1 then
      diskette(2, 15, 80)
    else
      diskette(2, 9, 80);
    $F8 : Writeln('fixed disk');
    $F0 : diskette(2, 18, 80)
    else
      unknown('media', a, 2)
  end
  end; {media}

procedure pagenameclr;
  var
    xbyte: byte;

  begin
  xbyte:=TextAttr;
  Window(x1, tlength, x2 - 1, tlength);
  TextColor((TextAttr and $70) shr 4);
  ClrScr;
  TextAttr:=xbyte;
  Window(1, 1, twidth, tlength)
  end; {pagenameclr}

procedure Intr(intno: byte; var regs: registers);
  begin
  AltIntr(intno, regs)
  end;

procedure MsDos(var regs: registers);
  begin
  AltMsDos(regs)
  end;

{These first two procedures filter the color commands to allow Black&White}
procedure TextColor(color: byte);
  var
    temp: byte;
  begin
  if mono then
    begin
    case (color and $0F) of
      0: temp:=0;
      1..7: temp:=7;
      8..15: temp:=15
      end;
    if color > 15 then
      temp:=temp + Blink;
    end
  else
    temp:=color;
  Crt.TextColor(temp)
  end; {TextColor}

procedure TextBackground(color: byte);
  var
    temp: byte;
  begin
  temp:=color;
  if mono and (color < 7) then
    temp:=0;
  Crt.TextBackground(temp);
  end; {TextBackground}

function unBCD(b: byte): byte;
  begin
  unBCD:=(b and $0F) + ((b shr 4) * 10)
  end; {unBCD}

function addzero(b: byte): string;
  var
    c2: string[2];
  begin
  Str(b:0, c2);
  if b < 10 then
    c2:='0' + c2;
  addzero:=c2
  end; {addzero}

procedure modeinfo(var vidmode, vidlen, vidpg: byte; var vidwid: word);
  var
    regs: registers;

  begin
  with regs do
    begin
    AH:=$0F;
    Intr($10, regs);
    vidmode:=AL;
    vidwid:=AH;
    vidpg:=BH;
    AX:=$1200;
    BL:=$10;
    Intr($10, regs);
    if BL = $10 then
      vidlen:=25
    else
      vidlen:=Mem[$40:$84] + 1;
    end
  end; {modeinfo}

procedure box;
  const
    frame: array[1..8] of char = '�ͻ���ͼ';
  var
    h, w, x, y: word;

  begin
  w:=Lo(WindMax) - Lo(WindMin) + 1;
  h:=Hi(WindMax) - Hi(WindMin) + 1;
  Inc(WindMax, $0101);
  GotoXY(1, 1);
  Write(frame[1]);
  for x:=2 to w - 1 do
    Write(frame[2]);
  GotoXY(w, 1);
  Write(frame[3]);
  for y:=2 to h - 1 do
    begin
    GotoXY(1, y);
    Write(frame[4]);
    GotoXY(w, y);
    Write(frame[5]);
    end;
  GotoXY(1, h);
  Write(frame[6]);
  GotoXY(2, h);
  for x:=2 to w-1 do
    Write(frame[7]);
  GotoXY(w, h);
  Write(frame[8]);
  Dec(WindMax, $0202);
  Inc(WindMin, $0101);
  end;

procedure center(s: string);
  var
    x, halfwidth, halfstr: integer;

  begin
  halfwidth:=(Lo(WindMax) - Lo(WindMin)) div 2;
  halfstr:=Length(s) div 2;
  if (halfwidth - halfstr) > 0 then
    for x:=1 to (halfwidth - halfstr) do
      Write(' ');
  Write(s);
  end;

function EMSOK: boolean;
  var
    S: string;
    EMSSeg, Address: word;
    Regs: Registers;

  begin
  EMSOK:=false;
  if longint(IntVec[$67]) <> 0 then
    begin
    EMSSeg:=longint(IntVec[$67]) shr 16;
    S:='';
    for Address:=$A to $11 do
      S:=S + Chr(Mem[EMSSeg:Address]);
    if S = 'EMMXXXX0' then
      with Regs do
        begin
        AH:=$40;
        Intr($67, regs);
        if AH = 0 then
          EMSOK:=true;
        end;
    end;
  end;

end.