procedure page_00;
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
  Writeln('ллллл  лн  л  ллллл  ллллл       ');
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
  Writeln('Page 0  - Table of Contents');
  Writeln('Page 1  - Machine & ROM Identification');
  Writeln('Page 2  - CPU and FPU Identification');
  Writeln('Page 3  - RAM type Identification');
  Writeln('Page 4  - List of Memory Blocks');
  Writeln('Page 5  - Video Identification');
  Writeln('Page 6  - Video Information');
  Writeln('Page 7  - Keyboard & Mouse Information');
  Writeln('Page 8  - Parallel/Serial Port Info');
  Write('Page 9  - DOS Information');
  Window((twidth div 2) + 1, (tlength div 2) - 3, twidth, tlength - 2);
  Writeln('Page 10 - Multiplex Programs');
  Writeln('Page 11 - Environment Variables');
  Writeln('Page 12 - Device Drivers');
  Writeln('Page 13 - DOS Drive Information');
  Writeln('Page 14 - BIOS Drive Information');
  Writeln('Page 15 - Partition Table Listing');
  Writeln('Page 16 - Boot & DOS Drive Parameters');
  Writeln('Page 17 - CMOS Information');
  Write('Page 18 - Thanks');
  Window(1, (tlength div 2) + 7, twidth, (tlength div 2) + 8);
  TextColor(LightCyan);
  for x:=1 to twidth do
    Write(#220);
  Window(1, 1, twidth, tlength);
  x:=twidth div 2;
  for y:=(tlength div 2) - 3 to (tlength div 2) + 7 do
    begin
    GotoXY(x, y);
    Write(#219);
    end;
  Window(1, tlength - 6, twidth, tlength - 1);
  TextColor(Yellow);
  Writeln;
  Center('INFOPLUS - ' + qversion);
  Center('by Andrew Rossmann, ' + qdate);
  Center('Based on SYSID 4.44, by Steve Grant');
  Center('Released to the Public Domain');
  end;