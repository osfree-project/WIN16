program printhlp;

uses
  Dos, ifpglobl;

const
  minpage = 0;
  maxpage = pgmax;
  infodate = '6/25/93';
  dashes: string[79] = '----------------------------------------' +
                       '---------------------------------------';

type
  tabletype = array[0..63] of longint;

var
  filename, s: string;
  infile, outfile: text;
  maxlines: byte;
  printpage, x: word;
  thetable: tabletype;
  tablefile: file of tabletype;

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

procedure printapage(page: word);
  var
    endit: boolean;
    linecount: byte;
    s: string;

  begin
  Str(page, s);
  if page < 10 then
    s:='0' + s;
  Assign(infile, 'INFOPLUS.HLP');
  Reset(infile);
  textseek(infile, thetable[page]);
  Writeln(outfile, dashes);
  Writeln(outfile, 'Infoplus ', vernum, '   Page ', page,
                   ' - ', pgnames[page]);
  Writeln(outfile, dashes);
  Writeln(outfile);
  linecount:=4;
  endit:=false;
  repeat
    Readln(infile, s);
    if s = '$END' then
      endit:=true
    else
      begin
      Writeln(outfile, s);
      Inc(linecount);
      if (linecount <> 255) and (linecount = maxlines) then
        begin
        Writeln(outfile, #12);
        linecount:=1
        end;
      end;
    until endit;
  Close(infile);
  Write(outfile, #12);
  end;

begin
Writeln('PRINTHLP ver 1.57 by Andrew Rossmann. For use with Infoplus 1.57');
Writeln('PRINTHLP prints out the .HLP pages to your printer. Each page');
Writeln('will have a header describing which Infoplus page it belongs to.');
Writeln;
repeat
  Write('Which page to print? 0 - ', maxpage, ', 99 for all.=> ');
  Readln(printpage);
until (printpage in [0..maxpage]) or (printpage = 99);
Write('Which device to you wish to print to? <ENTER> for PRN.=> ');
Readln(filename);
if filename = '' then
  filename:='PRN';
Assign(outfile, filename);
{$I-} ReWrite(outfile); {$I+}
if IOResult <> 0 then
  begin
  Writeln(^G'Unable to open ', FExpand(filename), ' for output!!');
  Exit;
  end;
Write('How many lines per page? <ENTER> for 60, 255 for continuous.=> ');
Readln(s);
maxlines:=0;
Val(s, maxlines, x);
if maxlines = 0 then
  maxlines:=60;
Assign(tablefile, 'INFOPLUS.HLP');
{$I-} Reset(tablefile); {$I+}
if IOResult <> 0 then
  begin
  Writeln(^G'Unable to open INFOPLUS.HLP!');
  Halt
  end;
Read(tablefile, thetable);
Close(tablefile);
if thetable[63] <> helpversion then
  begin
  Writeln(^G'Incorrect version of INFOPLUS.HLP!');
  Writeln('Found version ', (thetable[63] / 100.0):0:2);
  Halt
  end;
if printpage = 99 then
  for x:=0 to maxpage do
    printapage(x)
else
  printapage(printpage);
Close(outfile);
Writeln('Information printing completed.');
end.
