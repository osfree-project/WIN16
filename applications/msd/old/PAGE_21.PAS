unit page_21;

interface

uses crt, ifpglobl, ifpcomon;

procedure page21;

implementation

procedure page21;
  var
    xbyte: byte;

  begin
  TextColor(White);
  GotoXY((twidth div 2) - 15, 1);
  Writeln('Thank You for using INFOPLUS!!');
  Writeln;
  Writeln;
  TextColor(LightCyan);
  Writeln('  This is my final version of Infoplus. It''s not 100% complete, which');
  Writeln('is why it''s an alpha. The help screens have not been updated, and I');
  Writeln('didn''t make all the changes I wanted to.');
  Writeln;
  Writeln('  As of September 17th, 1993, I have no e-mail address. The Infoplus BBS');
  Writeln('has also been shut down.');
{
  Writeln('  If you have any questions, bug reports, or suggestions, I can be');
  Writeln('reached at the following places:');
  Writeln;
  TextColor(LightRed);
  Writeln('Internet       : andyross@infopls.chi.il.us');
  Writeln('UUCP           : uunet!infopls!andyross');
  Writeln('CompuServe     : >INTERNET:andyross@infopls.chi.il.us');
  Writeln;
  Writeln('Infoplus BBS   : (708)537-0247 (14400/9600/2400/1200 v32bis/v42bis/MNP)');
  Writeln('Beacon         : (708)615-0845 (2400/1200)');
}
  Window(1, tlength - 2, twidth, tlength - 2);
  xbyte:=TextAttr;
  TextColor(White);
  TextBackground(Brown);
  ClrScr;
  Write('INFOPLUS ', qversion, ', by Andrew Rossmann, ' + qdate);
  TextAttr:=xbyte;
  end;
end.