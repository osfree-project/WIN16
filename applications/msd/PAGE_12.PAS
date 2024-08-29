unit page_12;

interface

uses crt, ifpglobl, ifpcomon;

procedure page12;

implementation

procedure page12;

const
  headermin = 0;
  headermax = 17;

var
  header : array[headermin..headermax] of byte;
  i : headermin..headermax;
  xword1 : word;
  xword2 : word;

  begin
  caption1('Device      Units    Header       Attributes'
    + '             Strategy     Interrupt');
  xword1:=devseg;
  xword2:=devofs + $0022;
  window(1, 4, twidth, tlength - 2);
  while xword2 < $FFFF do
    begin
    pause2;
    if endit then
      Exit;
    for i:=0 to 17 do
      header[i]:=mem[xword1 : xword2 + i];
    if header[5] and $80 = $00 then
      write('            ', header[10] : 5)
    else
      begin
      for i:=10 to 17 do
        write(showchar(chr(header[i])));
      write('         ')
      end;
    write('    ');
    segofs(xword1, xword2);
    write('    ', bin16(cbw(header[4], header[5])), '    ');
    segofs(xword1, cbw(header[6], header[7]));
    write('    ');
    segofs(xword1, cbw(header[8], header[9]));
    writeln;
    xword1:=cbw(header[2], header[3]);
    xword2:=cbw(header[0], header[1])
    end
  end;
end.