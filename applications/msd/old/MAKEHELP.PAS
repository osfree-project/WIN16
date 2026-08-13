program makehelp;

type
  tabletype = array[0..63] of longint;

const
  version: longint = 157;
  firstfile = 0;
  lastfile = 21;

var
  tablefile: file of tabletype;
  bytefile: file of byte;
  infile, outfile: text;
  inname, outname, s: string;
  thetable: tabletype;
  filecount: word;
  file_size: longint;

begin
Writeln('Making INFOPLUS.HLP for version ', version);
Writeln('Creating pages ', firstfile, ' through ', lastfile);
FillChar(thetable, Sizeof(thetable), #0);
Assign(bytefile, 'INFOPLUS.HLP');
{$I-} Erase(bytefile); {$I+}
for filecount:=firstfile to lastfile do
  begin
  Assign(bytefile, 'INFOPLUS.HLP');
  {$I-} Reset(bytefile); {$I+}
  if IOResult <> 0 then
    ReWrite(bytefile);
  file_size:=FileSize(bytefile);
  Close(bytefile);
  Assign(tablefile, 'INFOPLUS.HLP');
  {$I-} Reset(tablefile); {$I+}
  Writeln(filecount, '-', file_size);
  Seek(tablefile, 0);
  if file_size > 0 then
    Read(tablefile, thetable)
  else
    file_size:=256;
  thetable[63]:=version;
  thetable[filecount]:=file_size;
  Seek(tablefile, 0);
  Write(tablefile, thetable);
  Close(tablefile);
  Str(filecount, s);
  if filecount < 10 then
    s:='0' + s;
  Assign(infile, 'PAGE_' + s + '.INF');
  {$I-}; Reset(infile); {$I+}
  if IOResult <> 0 then
    begin
    Writeln('Unable to open PAGE_', s, '.INF!!'^G);
    Halt
    end;
  Assign(outfile, 'INFOPLUS.HLP');
  Append(outfile);
  while not Eof(infile) do
    begin
    Readln(infile, s);
    Writeln(outfile, s);
    end;
  Writeln(outfile, '$END');
  Close(outfile);
  Close(infile);
  end;
end.

