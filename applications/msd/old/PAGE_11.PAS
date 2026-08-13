unit page_11;

interface

uses crt, dos, ifpglobl, ifpcomon;

procedure page11;

implementation

procedure page11;
  var
    temp, temp1, envseg, envlen, envused: word;
    foundit, endfound: boolean;

  begin
  caption2('Program name');
  Writeln(ParamStr(0));
{PC Magazine, Vol 8, no. 20 (11/28/89), pg 309, from Barry Simon}
  temp:=MemW[PrefixSeg:$16];
  foundit:=false;
  while not foundit do
    begin
    temp1:=MemW[temp:$16];
    if (temp1 = 0) or (temp1 = temp) then
      foundit:=true
    else
      temp:=temp1
    end;
  envseg:=MemW[temp:$2C];
  if (envseg = 0) or ((osminor > 19) and (osminor < 30)) then
    envseg:=temp + MemW[temp-1:3] + 1;
  envlen:=MemW[envseg - 1:3] * 16;
  envused:=0;
  endfound:=false;
  while not endfound do
    if MemW[envseg:envused] = 0 then
      endfound:=true
    else
      Inc(envused);
  Inc(envused, 2);
  caption2('Environment');
  Writeln;
  caption3('Segment');
  Write(hex(envseg, 4));
  caption3('Size');
  Write(envlen:4);
  caption3('Used');
  Write(envused:4);
  caption3('Free');
  Writeln((envlen - envused):4);
  caption2('Variables');
  window(3, 7, twidth, tlength - 2);
  for i:=1 to envcount do begin
    pause2;
    if endit then
      Exit;
    writeln(envstr(i))
  end
  end;
end.