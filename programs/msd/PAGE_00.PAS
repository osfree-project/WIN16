unit page_00;

interface

uses Crt, ifpglobl, ifpcomon;

procedure page00;

implementation

procedure page00;
  var
    x, y: word;

  procedure center(s: string);
    begin
    GotoXY((twidth div 2) - (Length(s) div 2), WhereY);
    Writeln(s)
    end;

  begin
  TextColor(LightGreen);
  Window((twidth div 2) - 16, 3, (twidth div 2) + 17, 8);
  Writeln('ллллл  л   л  ллллл  ллллл       ');
  Writeln('  л    лл  л  л      л   л    л  ');
  Writeln('  л    л л л  ллл    л   л  ллллл');
  Writeln('  л    л  лл  л      л   л    л  ');
  Writeln('ллллл  л   л  л      ллллл       ');
  Window(1, (tlength div 2) - 4, twidth, (tlength div 2) - 3);
  TextColor(LightCyan);
  for x:=1 to twidth do
    Write(#220);
  Window(1, (tlength div 2) - 3, (twidth div 2) - 1, tlength - 2);
  TextColor(White);
  for x:= 0 to 9 do
    Writeln('Page ', x, '  - ', pgnames[x]);
  Write('Page 10 - ', pgnames[10]);
  Window((twidth div 2) + 1, (tlength div 2) - 3, twidth, tlength - 2);
  for x:=11 to 20 do
    Writeln('Page ', x, ' - ', pgnames[x]);
  Write('Page 21 - ', pgnames[21]);
  Window(1, (tlength div 2) + 8, twidth, (tlength div 2) + 9);
  TextColor(LightCyan);
  for x:=1 to twidth do
    Write(#220);
  Window(1, 1, twidth, tlength);
  x:=twidth div 2;
  for y:=(tlength div 2) - 3 to (tlength div 2) + 8 do
    begin
    GotoXY(x, y);
    Write(#219);
    end;
  Window(1, tlength - 5, twidth, tlength - 1);
  TextColor(Yellow);
  Writeln;
  Center('INFOPLUS - ' + qversion + ' by Andrew Rossmann, ' + qdate);
  Center('Based on SYSID 4.44, by Steve Grant');
  Center('Released to the Public Domain');
  end;
end.