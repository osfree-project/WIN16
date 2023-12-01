(*
**  INFOPLUS.PAS
**
**  Version 1.35 by Andrew Rossmann 10/7/90
*)

(*$A-,B-,D-,L-,F-,I-,N-,O-,R-,S-,V-*)
(*$M 16384, 0, 0*)
program INFOPLUS;

uses
  crt, dos, graph;

const
  qversion = 'Version 1.35';
  qdate = 'October 7, 1990';
  BIOSdseg = $0040;
  pgmax = 18;
  pchar = [' '..'~'];
  secsiz = 1024;
  tick1 = 1193180;
  pgnames: array [0..18] of string[32] = (
             'Table of Contents',
             'Machine & ROM Identification',
             'CPU Identification',
             'RAM Identification',
             'Memory Block Listing',
             'Video Identification',
             'Video Information',
             'Keyboard & Mouse Information',
             'Parallel/Serial Port Information',
             'DOS Information',
         {10}'Multiplex Programs',
             'Environment Variables',
             'Device Drivers',
             'DOS Drive Information',
             'BIOS Drive Information',
             'Partition Table Listing',
             'Boot info & DOS drive parameters',
             'CMOS information',
             'Thanks');

type
  cpu_info_t = record
    cpu_type : byte;
    MSW : word;
    GDT : array[1..6] of byte;
    IDT : array[1..6] of byte;
    intflag : boolean;
    ndp_type : byte;
    ndp_cw : word;
    weitek: byte;
    test_type: char
  end;
  char2 = string[2];

var
  attrsave : byte;
  country : array[0..33] of byte;
  currdrv : byte;
  devofs : word;
  devseg : word;
  dirsep : set of char;
  DOScofs : word;
  DOScseg : word;
  DOSmem : longint;
  equip : word;
  graphdriver : integer;
  i : word;
  intvec : array[$00..$FF] of pointer;
  lastdrv : byte;
  osmajor : byte;
  osminor : byte;
  pg : 0..pgmax;
  regs : registers;
  switchar : char;
  tlength : byte;
  twidth : byte;
  vidpg : byte;
  x1 : byte;
  x2 : byte;
  xbool1 : boolean;
  xbool2 : boolean;
  xchar1 : char;
  xchar2 : char;
  xword : word;
  gotcountry: boolean;
  c2: char2;
  endit: boolean;
  ccode: word;
  mono: boolean;
  vidmode: word;
  decimal: char;

(*$L c:\tasm\INFOPLUS*)

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
    Val(number_string, temp, code);
  getnum:=temp
  end; {getnum}

procedure caption1(a: string);
  begin
  textcolor(LightGray);
  write(a);
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

function nocarry : boolean;
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
  writeln('(unknown', ' ', a, ' ', hex(b, c), ')')
  end; {unknown}

procedure caption3(a : string);
  begin
  caption2('  ' + a)
  end; {caption3}

procedure yesorno(a : boolean);
  begin
  if a then
    writeln('yes')
  else
    writeln('no')
  end; {yesorno}

procedure yesorno2(a: boolean);
  begin
  if a then
    Write('yes')
  else
    Write('no')
  end; {yesorno2}

procedure dontknow;
  begin
  writeln('(unknown)')
  end; {dontknow}

procedure dontknow2;
  begin
  Write('(unknown)')
  end; {dontknow2}

procedure segofs(a, b : word);
  begin
  write(hex(a, 4), ':', hex(b, 4))
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
    savex, savey: byte;

  begin
  xbyte:=textattr;
  endit:=false;
  textcolor(Cyan);
  savex:=WhereX;
  savey:=WhereY;
  Write('( for more)');
  xchar:=getkey2;
  if xchar <> #0#80 then
    begin
    endit:=true;
    c2:=xchar
    end;
  textattr:=xbyte;
  GotoXY(savex, savey);
  Write('            ')
  end; {pause1}

procedure pause2;
  var
    xbyte : byte;

  begin
  if wherey + hi(windmin) > hi(windmax) then
    begin
    xbyte:=TextAttr;
    TextColor(Cyan);
    pause1;
    clrscr;
    writeln('(continued)');
    textattr:=xbyte
    end
  end; {pause2}

procedure pause3(extra: byte);
  var
    xbyte: byte;
  begin
  if WhereY + Hi(WindMin) + extra > Hi(WindMax) then
    begin
    xbyte:=TextAttr;
    TextColor(Cyan);
    pause1;
    ClrScr;
    Writeln('(continued)');
    TextAttr:=xbyte
    end
  end; {pause3}

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
    writeln('on')
  else
    writeln('off')
  end; {offoron}

procedure zeropad(a : word);
  begin
  if a < 10 then
    write('0');
  write(a)
  end; {zeropad}

procedure showvers;
  begin
  if osmajor > 0 then
    begin
    Write(osmajor, decimal);
    zeropad(osminor);
    writeln
    end
  else
    writeln('1', decimal, 'x')
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
  write(chr(ord('A') + a), ': ')
  end; {drvname}

procedure media(a, b : byte);
  procedure diskette(a, b, c : byte);
    begin
    writeln('floppy ', a, ' side, ', b, ' sctr, ', c, ' trk')
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
    $F8 : writeln('fixed disk');
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

{$F+}
procedure CPUID(var a: cpu_info_t); external;

function diskread(drive: byte; starting_sector: longint;
  number_of_sectors: word; var buffer): word; external;

procedure longcall(addr: longint; var regs: registers); external;

function ATIinfo(data_in: byte; register: word): byte; external;

procedure AltIntr(intno: byte; var regs: registers); external;

procedure AltMsDos(var regs: registers); external;

function CirrusCK: byte; external;

function CTICK: byte; external;

function TsengCK: byte; external;

function ZyMOSCK: byte; external;
{$F-}

procedure Intr(intno: byte; var regs: registers);
  begin
  AltIntr(intno, regs)
  end;

procedure MsDos(var regs: registers);
  begin
  AltMsDos(regs)
  end;

procedure init;
  var
    xint : integer;

  procedure rjustify(a : string);
    begin
    gotoxy(1 + lo(windmax) - length(a), wherey);
    x2:=WhereX;
    write(a)
    end; {rjustify}

  procedure border(ch: char);
    var
      i : byte;

    begin
    TextColor(LightCyan);
    for i:=1 to twidth do
      write(ch);
    TextColor(LightGray);
    end; {border}

  begin {init}
  mono:=false;
  vidmode:=LastMode;
  attrsave:=textattr;
  if (Lo(LastMode) = 0) or (Lo(LastMode) = 1) then
    TextMode(LastMode + 2);
  with regs do
    begin
    AH:=$0F;
    intr($10, regs);
    twidth:=AH;
    vidpg:=BH
    end;
  detectgraph(graphdriver, xint);
  if (graphdriver = EGA) or (graphdriver = MCGA) or (graphdriver = VGA) then
    with regs do
      begin
      AX:=$1130;
      BH:=$00;
      intr($10, regs);
      tlength:=DL + 1;
      CheckSnow:=False;
      end
  else
    tlength:=25;
  with regs do
    begin
    intr($11, regs);
    equip:=AX;
    intr($12, regs);
    DOSmem:=longint(AX) shl 10;
    AH:=$19;
    MSDOS(regs);
    currdrv:=AL;
    AH:=$34;
    MSDOS(regs);
    DOScseg:=ES;
    DOScofs:=BX
    end;
  for i:=$00 to $FF do
    getintvec(i, intvec[i]);
  intvec[$00]:=saveint00;
  intvec[$02]:=saveint02;
  intvec[$1B]:=saveint1B;
  intvec[$23]:=saveint23;
  intvec[$24]:=saveint24;
  intvec[$34]:=saveint34;
  intvec[$35]:=saveint35;
  intvec[$36]:=saveint36;
  intvec[$37]:=saveint37;
  intvec[$38]:=saveint38;
  intvec[$39]:=saveint39;
  intvec[$3A]:=saveint3A;
  intvec[$3B]:=saveint3B;
  intvec[$3C]:=saveint3C;
  intvec[$3D]:=saveint3D;
  intvec[$3E]:=saveint3E;
  intvec[$3F]:=saveint3F;
  intvec[$75]:=saveint75;
  with regs do
    begin
    AX:=$3700;
    MSDOS(regs);
    switchar:=chr(DL)
    end;
  dirsep:=['\'];
  if switchar <> '/' then
    dirsep:=dirsep + ['/'];
  with regs do
    begin
    AH:=$52;
    MSDOS(regs);
    devseg:=ES;
    devofs:=BX
    end;
  lastdrv:=mem[devseg : devofs + $0021];
  if (Lo(LastMode) = 2) or (Lo(LastMode) = 7) then
    mono:=true;
  TextBackground(Blue);
  clrscr;
  textcolor(LightGreen);
  write('INFO+');
  textcolor(lightgray);
  write(' - Information on all computer functions');
  rjustify(qversion);
  writeln;
  border(#223);
  gotoxy(1, tlength - 1);
  border(#220);
  write('Page ');
  x1:=wherex;
  textcolor(Lightgreen);
  rjustify('Enter PgUp PgDn Home End Esc');
  pg:=0;
  endit:=false;
  if osmajor >= 3 then
    with regs do
      begin
      AX:=$3800;
      DS:=seg(country);
      DX:=ofs(country);
      MSDOS(regs);
      ccode:=BX;
      decimal:=Chr(country[9]);
      end;
  end; {init}

{$I PAGE_00.INC}
{$I PAGE_01.INC}
{$I PAGE_02.INC}
{$I PAGE_03.INC}
{$I PAGE_04.INC}
{$I PAGE_05.INC}
{$I PAGE_06.INC}
{$I PAGE_07.INC}
{$I PAGE_08.INC}
{$I PAGE_09.INC}
{$I PAGE_10.INC}
{$I PAGE_11.INC}
{$I PAGE_12.INC}
{$I PAGE_13.INC}
{$I PAGE_14.INC}
{$I PAGE_15.INC}
{$I PAGE_16.INC}
{$I PAGE_17.INC}
{$I PAGE_18.INC}

begin
  xword:=dosversion;
  osmajor:=lo(xword);
  osminor:=hi(xword);
  if osmajor >= 3 then
    begin
    init;
    xbool1:=false;
    repeat
      pagenameclr;
      gotoxy(x1, tlength);
      textcolor(lightgray);
      write(pg:2, ' - ');
      Write(pgnames[pg]);
      window(1, 3, twidth, tlength - 2);
      clrscr;
      case pg of
        0 : page_00;
        1 : page_01;
        2 : page_02;
        3 : page_03;
        4 : page_04;
        5 : page_05;
        6 : page_06;
        7 : page_07;
        8 : page_08;
        9 : page_09;
        10 : page_10;
        11 : page_11;
        12 : page_12;
        13 : page_13;
        14 : page_14;
        15 : page_15;
        16 : page_16;
        17 : page_17;
        18 : page_18;
      end;
      window(1, 1, twidth, tlength);
      gotoxy(x2 - 1, tlength);
      xbool2:=false;
      repeat
        if not endit then
          begin
          repeat
          until keypressed;
          xchar1:=readkey;
          if keypressed then
            xchar2:=readkey
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
          pagenameclr;
          gotoxy(x1, tlength);
          textcolor(lightgray);
          write(pg:2, ' - ');
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
          end;
      if not xbool2 then
        begin
        Sound(220);
        Delay(100);
        NoSound
        end
      until xbool2
    until xbool1;
    textattr:=attrsave;
    TextMode(vidmode);
    clrscr
  end
else
  begin
  writeln;
  decimal:='.';
  writeln('INFOPLUS requires DOS version 3.0 or later');
  write('Your DOS version is ');
  showvers
  end
end.
