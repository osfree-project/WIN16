unit ifpglobl;

interface
  uses dos;

const
  qversion = 'Version 1.58à';
  qdate = 'September 17, 1993';
  vernum = '1.58à';
  helpversion = 157;
  BIOSdseg = $0040;
  pgmax = 21;
  pchar = [' '..'~'];
  secsiz = 2048;
  tick1 = 1193180;
  pgnames: array [0..pgmax] of string[32] = (
             'Table of Contents',
             'Machine & ROM Identification',
             'CPU Identification',
             'RAM Identification',
             'Memory Block Listing',
             'Video Identification',
             'Video Information',
             'Keyboard & Mouse Information',
             'Parallel/Serial/Sound Ports',
             'DOS Information',
         {10}'Multiplex Programs',
             'Environment Variables',
             'Device Drivers',
             'DOS Drive Information',
             'BIOS Drive Information',
             'Partition Table Listing',
             'Boot & DOS drive parameters',
             'CMOS information',
             'TSR''s and Drivers',
             'Alternate Multiplex',
         {20}'Memory Managers',
             'Thanks');
type
  char2 = string[2];
  directions = (none, up, down, updown);

var
  attrsave : byte;
  country : array[0..33] of byte;
  currdrv : byte;
  devofs : word;
  devseg : word;
  dirsep : set of char;
  DOScofs : word;
  DOScseg : word;
  DOSmem : longint;
  equip : word;
  graphdriver : integer;
  i : word;
  intvec : array[$00..$FF] of pointer;
  lastdrv : byte;
  osmajor : byte;
  osminor : byte;
  pg : 0..pgmax;
  regs : registers;
  switchar : char;
  tlength : byte;
  twidth : byte;
  vidpg : byte;
  x1 : byte;
  x2 : byte;
  xbool1 : boolean;
  xbool2 : boolean;
  xchar1 : char;
  xchar2 : char;
  xword : word;
  gotcountry: boolean;
  c2: char2;
  quiet, endit: boolean;
  ccode: word;
  vidmode: word;
  decimal: char;
  mono: boolean;
  resetvideo: boolean;
  novgacheck: boolean;
  ReadPartitionTable: boolean;
  FifoOn: boolean;

implementation

end.
