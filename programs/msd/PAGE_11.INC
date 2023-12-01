procedure page_11;
  begin
  caption2('Program name');
  Writeln(ParamStr(0));
  caption2('Environment');
  window(3, 5, twidth, tlength - 2);
  for i:=1 to envcount do begin
    pause2;
    if endit then
      Exit;
    writeln(envstr(i))
  end
  end;
