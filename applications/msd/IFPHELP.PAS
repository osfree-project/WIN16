unit ifphelp;
{$V-}
interface

Uses
    Crt, Dos, ifpglobl, ifpcomon;

procedure helpscreen(pg: byte; helpver: longint);

implementation

type
  tabletype = array[0..63] of longint;
  helpptrtype = ^helptextrec;
  helptextrec = record
                  before, after: helpptrtype;
                  lineno: word;
                  helptext: string[79];
                end;

var
  scrbuf: array[0..9599] of byte;
  monoscrn: array[0..3999] of byte absolute $B000:0;
  colorscrn: array[0..9599] of byte absolute $B800:0;
  vidmode, vidlen, vidpg, oldattr, oldx, oldy: byte;
  vidsize, vidwid, oldwindmin, oldwindmax: word;
  thetable: tabletype;
  filefound: boolean;
  helphead: helpptrtype;
  c: char;

procedure textseek(var thefile: text; position: longint);
  var
    segment, offset: word;
    regs: registers;

  begin
  segment:=Seg(thefile);
  offset:=Ofs(thefile);
  MemW[segment:offset + 8]:=0;
  MemW[segment:offset + 10]:=0;
  with regs do
    begin
    BX:=MemW[segment:offset];
    CX:=position shr 16;
    DX:=position and $0000FFFF;
    AH:=$42;
    AL:=0;
    MsDos(regs);
    end;
  end;

procedure setup;
  var
    x, y: byte;
    regs: registers;
    position: word;

  begin
  oldattr:=TextAttr;
  oldwindmin:=WindMin;
  oldwindmax:=WindMax;
  oldx:=WhereX;
  oldy:=WhereY;
  filefound:=false;
  position:=0;
  modeinfo(vidmode, vidlen, vidpg, vidwid);
  vidsize:=(vidwid * vidlen) * 2;
  if DirectVideo then
    if vidmode = 7 then
      Move(monoscrn, scrbuf, vidsize)
    else
      Move(colorscrn, scrbuf, vidsize)
  else
    for y:=0 to vidlen - 1 do
      for x:=0 to vidwid -1 do
        with regs do
          begin
          AH:=2;
          BH:=vidpg;
          DH:=y;
          DL:=x;
          Intr($10, regs);
          AH:=8;
          BH:=vidpg;
          Intr($10, regs);
          scrbuf[position]:=AL;
          scrbuf[position + 1]:=AH;
          Inc(position, 2);
          end;
  end;

procedure cleanup;
  var
    x, y: byte;
    regs: registers;
    position: word;

  begin
  position:=0;
  if DirectVideo then
    if vidmode = 7 then
      Move(scrbuf, monoscrn, vidsize)
    else
      Move(scrbuf, colorscrn, vidsize)
  else
    for y:=0 to vidlen - 1 do
      for x:=0 to vidwid -1 do
        with regs do
          begin
          AH:=2;
          BH:=vidpg;
          DH:=y;
          DL:=x;
          Intr($10, regs);
          AH:=9;
          AL:=scrbuf[position];
          BH:=vidpg;
          BL:=scrbuf[position + 1];
          CX:=1;
          Intr($10, regs);
          Inc(position, 2);
          end;
  TextAttr:=oldattr;
  WindMin:=oldwindmin;
  WindMax:=oldwindmax;
  GotoXY(oldx, oldy);
  end;

procedure anykey;
  var
    c: char;

  begin
  center('Press <any key> to continue');
  repeat until KeyPressed;
  c:=ReadKey;
  if c = #0 then
    c:=ReadKey;
  end;

procedure clearheap;
  var
    nowptr, nextptr: helpptrtype;

  begin
  nowptr:=helphead;
  repeat
    nextptr:=nowptr^.after;
    Dispose(nowptr);
    nowptr:=nextptr
  until nowptr = nil
  end;

procedure readfile(pg: byte; helpver: longint);
  var
    filename: string[127];
    c:char;
    tablefile: file of tabletype;
    infile: text;
    dir, s: string;
    extension: string[3];
    linecount: word;
    previousptr, nowptr: helpptrtype;
    endread: boolean;

  begin
  if GetEnv('INFOPLUS') <> '' then
    begin
    filename:=GetEnv('INFOPLUS');
    if Pos('.', filename) = 0 then
      begin
      c:=filename[Length(filename)];
      if (filename <> '') and (c <> ':') and (c <> '\') and (c <> '/') then
        filename:=filename + '\';
      filename:=filename + 'INFOPLUS.HLP';
      end;
    Assign(tablefile, filename);
    {$I-} Reset(tablefile); {$I+}
    if IOResult <> 0 then
      begin
      TextColor(White);
      TextBackground(Red);
      s:='INFOPLUS environment variable does not point';
      Window((vidwid div 2) - (Length(s) div 2) - 2, (vidlen div 2) - 3,
             (vidwid div 2) + (Length(s) div 2) + 2, (vidlen div 2) + 3);
      box;
      ClrScr;
      center(s);
      Writeln;
      center('to a valid help file directory.');
      Writeln;
      center('INFOPLUS=' + GetEnv('INFOPLUS'));
      Writeln;
      Writeln;
      anykey;
      cleanup;
      Exit;
      end
    else
      filefound:=true;
    end;
  if not filefound then
    begin
    FSplit(FExpand(ParamStr(0)), dir, filename, extension);
    filename:=FSearch('INFOPLUS.HLP', '.;' + dir + ';' + GetEnv('PATH'));
    if filename = '' then
      begin
      TextColor(White);
      TextBackground(Red);
      s:='Unable to find INFOPLUS.HLP!';
      Window((vidwid div 2) - (Length(s) div 2) - 2, (vidlen div 2) - 2,
             (vidwid div 2) + (Length(s) div 2) + 2, (vidlen div 2) + 2);
      box;
      ClrScr;
      center(s);
      Writeln;
      Writeln;
      anykey;
      cleanup;
      Exit;
      end
    else
      begin
      Assign(tablefile, filename);
      {$I-} Reset(tablefile); {$I+}
      if IOResult <> 0 then
        begin
        TextColor(White);
        TextBackground(Red);
        s:='Unable to open ' + filename;
        Window((vidwid div 2) - (Length(s) div 2) - 2, (vidlen div 2) - 2,
               (vidwid div 2) + (Length(s) div 2) + 2, (vidlen div 2) + 2);
        box;
        ClrScr;
        center(s);
        Writeln;
        Writeln;
        anykey;
        cleanup;
        Exit;
        end
      else
        filefound:=true;
      end;
    end;
  Read(tablefile, thetable);
  Close(tablefile);
  if thetable[63] <> helpver then
    begin
    TextColor(White);
    TextBackground(Red);
    s:='Incorrect version of INFOPLUS.HLP!';
    Window((vidwid div 2) - (Length(s) div 2) - 2, (vidlen div 2) - 2,
           (vidwid div 2) + (Length(s) div 2) + 2, (vidlen div 2) + 2);
    box;
    ClrScr;
    center(s);
    Writeln;
    Writeln('Found version: ', (thetable[63] / 100.0):0:2);
    anykey;
    cleanup;
    filefound:=false;
    Exit;
    end;
  Assign(infile, filename);
  Reset(infile);
  textseek(infile, thetable[pg]);
  helphead:=nil;
  previousptr:=nil;
  nowptr:=nil;
  endread:=false;
  linecount:=0;
  repeat
    Readln(infile, s);
    if s = '$END' then
      endread:=true
    else
      if MaxAvail < SizeOf(helptextrec) then
        begin
        TextColor(White);
        TextBackground(Red);
        s:='Insufficient memory to read the ';
        Window((vidwid div 2) - (Length(s) div 2) - 2, (vidlen div 2) - 3,
               (vidwid div 2) + (Length(s) div 2) + 2, (vidlen div 2) + 3);
        box;
        ClrScr;
        center(s);
        Writeln;
        center('full help page');
        Writeln;
        Writeln;
        anykey;
        endread:=true;
        end
      else
        begin
        New(nowptr);
        if helphead = nil then
          helphead:=nowptr
        else
          previousptr^.after:=nowptr;
        nowptr^.before:=previousptr;
        nowptr^.helptext:=s;
        Inc(linecount);
        nowptr^.lineno:=linecount;
        nowptr^.after:=nil;
        previousptr:=nowptr;
        end;
  until endread;
  Close(infile);
  end;

procedure showhelp;
  var
    c2: char2;
    nowptr: helpptrtype;
    height, helplength, topline, btmline: word;
    endhelp: boolean;

  procedure showscreen(first, last: word);
    var
      nowptr: helpptrtype;

    begin
    nowptr:=helphead;
    GotoXY(1, 1);
    while nowptr^.lineno <> first do
      nowptr:=nowptr^.after;
    while (nowptr^.lineno <= last) and (nowptr <> nil) do
      begin
      ClrEol;
      if WhereY = height then
        Write(nowptr^.helptext)
      else
        Writeln(nowptr^.helptext);
      nowptr:=nowptr^.after
      end;
    end;

  begin
  TextColor(White);
  TextBackground(Blue);
  Window(x2, tlength, twidth, tlength);
  ClrScr;
  Write('   PgUp PgDn Home End ESC');
  Window(1, 3, twidth, tlength - 2);
  ClrScr;
  height:=Hi(WindMax) - Hi(WindMin) + 1;
  nowptr:=helphead;
  helplength:=0;
  while nowptr <> nil do
    begin
    helplength:=nowptr^.lineno;
    nowptr:=nowptr^.after
    end;
  nowptr:=helphead;
  topline:=1;
  if height >= helplength then
    btmline:=helplength
  else
    btmline:=height;
  showscreen(topline, btmline);
  endhelp:=false;
  repeat
    c2:=getkey2;
    case c2[1] of
      #27: endhelp:=true;
      #0: case c2[2] of
            #$50: if btmline < helplength then {dn arrow}
                    begin
                    Inc(btmline);
                    Inc(topline);
                    showscreen(topline, btmline);
                    end;
            #$48: if topline > 1 then {up arrow}
                    begin
                    Dec(topline);
                    Dec(btmline);
                    showscreen(topline, btmline);
                    end;
            #$51: if btmline < helplength then {PgDn}
                    begin
                    Inc(btmline, height);
                    Inc(topline, height);
                    if btmline > helplength then
                      begin
                      btmline:=helplength;
                      topline:=btmline - height + 1;
                      end;
                    showscreen(topline, btmline);
                    end;
            #$49: if topline > 1 then {PgUp}
                    begin
                    if topline <= height then
                      begin
                      topline:=1;
                      btmline:=height;
                      end
                    else
                      begin
                      Dec(topline, height);
                      Dec(btmline, height)
                      end;
                    showscreen(topline, btmline);
                    end;
            #$47: begin {Home}
                  topline:=1;
                  btmline:=height;
                  showscreen(topline, btmline);
                  end;
            #$4F: begin {End}
                  btmline:=helplength;
                  topline:=btmline - height + 1;
                  showscreen(topline, btmline)
                  end;
          end; {case c2[2]}
    end; {case c2[1]}
  until endhelp;
  end;

procedure helpscreen(pg: byte; helpver: longint);
  begin
  setup;
  readfile(pg, helpver);
  if not filefound then
    Exit;
  showhelp;
  clearheap;
  cleanup;
  end;
end.
