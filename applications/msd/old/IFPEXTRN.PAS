unit ifpextrn;

interface

uses Crt, Dos;

type
  cpu_info_t = record
    cpu_type : byte;
    MSW : word;
    GDT : array[1..6] of byte;
    IDT : array[1..6] of byte;
    intflag : boolean;
    ndp_type : byte;
    ndp_cw : word;
    weitek: byte;
    test_type: char
  end;

procedure CPUID(var a: cpu_info_t);

function diskread(drive: byte; starting_sector: longint;
  number_of_sectors: word; var buffer): word;

procedure longcall(addr: longint; var regs: registers);

function ATIinfo(data_in: byte; register: word): byte;

procedure AltIntr(intno: byte; var regs: registers);

procedure AltMsDos(var regs: registers);

function CTICK: byte;

function TsengCK: byte;

function ZyMOSCK: byte;

function CirrusCK: byte;

function bugtst: byte;

implementation
{$L INFOPLUS}

{$F+}

procedure CPUID(var a: cpu_info_t); external;

function diskread(drive: byte; starting_sector: longint;
  number_of_sectors: word; var buffer): word; external;

procedure longcall(addr: longint; var regs: registers); external;

function ATIinfo(data_in: byte; register: word): byte; external;

procedure AltIntr(intno: byte; var regs: registers); external;

procedure AltMsDos(var regs: registers); external;

function CTICK: byte; external;

function TsengCK: byte; external;

function ZyMOSCK: byte; external;

function CirrusCK: byte; external;

function bugtst: byte; external;

end.