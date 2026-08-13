unit page_19;

interface

uses Crt, Dos, IFPGlobl, IFPComon;

procedure page19;

implementation

procedure page19;

var
  Regs: Registers;
  MultiNumber: byte;
  FoundOne, FoundInt: boolean;
  Address, LastAddress: word;
  IntNumber: word;

begin
FoundOne:=false;
for MultiNumber:=0 to $FF do
  with regs do
    begin
    AH:=MultiNumber;
    AL:=0;
    Intr($2D, regs);
    if AL = $FF then
      begin
      FoundOne:=true;
      Pause3(-4);
      Caption2('Function');
      Write(Hex(MultiNumber, 2), 'h');
      Caption3('Product');
      for Address:=DI + 8 to DI + $0F do
        Write(Chr(Mem[DX:Address]));
      Caption3('Manufacturer');
      for Address:=DI to DI + 7 do
        Write(Chr(Mem[DX:Address]));
      Caption3('Version');
      Writeln(Hi(CX), decimal, addzero(Lo(CX)));
      Caption3('Entry point');
      AH:=MultiNumber;
      AL:=1;
      Intr($2D, regs);
      if AL = 0 then
        Write('Use INT 2Dh')
      else
        SegOfs(DX, BX);
      Writeln;
      Caption3('Hooked interrupts (hex)');
      IntNumber:=0;
      FoundInt:=false;
      AH:=MultiNumber;
      AL:=4;
      Intr($2D, regs);
      if AL = 0 then
        Writeln('(function call not implemented)')
      else
        if AL = 4 then
          begin
          Address:=BX;
          while Mem[DX:Address] <> $2D do
            begin
            FoundInt:=true;
            Write(Hex(Mem[DX:Address], 2), ' ');
            Inc(Address, 3);
            end;
          if not FoundInt then
            Write('(none)');
          Writeln;
          end
        else
          begin
          while IntNumber < 256 do
            begin
            AH:=MultiNumber;
            AL:=4;
            Intr($2D, regs);
            if (AL = 2) or (AL = 3) then
              begin
              FoundInt:=true;
              Write(Hex(IntNumber, 2), ' ');
              end;
            Inc(IntNumber);
            end;
          if not FoundInt then
            Write('(none)');
          Writeln;
          end;
      Caption3('Description');
      AH:=MultiNumber;
      AL:=0;
      Intr($2D, regs);
      if Mem[DX:DI + $10] = 0 then
        Writeln('(none)')
      else
        begin
        Address:=DI + $10;
        LastAddress:=Address + 64;
        while (Mem[DX:Address] <> 0) and (Address <= LastAddress) do
          begin
          Write(Chr(Mem[DX:Address]));
          Inc(Address);
          end;
        Writeln;
        end;
      end
    end;
if not FoundOne then
  Caption1('No Alternate Multiplex Programs found!');
end;
end.