unit page_20;

interface

uses Crt, Dos, IFPGlobl, IFPComon, IFPExtrn;

procedure page20;

implementation

procedure page20;

var
  Regs: registers;
  Handle: word;
  Buffer: array[0..255] of byte;
  S: string;

  procedure ShowQEMMinfo(APISeg, APIOfs: word);
    const
      QEMMMemType: array[0..$B] of char = 'm?MHXVRAsFrC';
      QEMMMemAccess: array[0..3] of char = '-+*!';
      QEMMStealth: array[0..1] of char = ' S';

    type
      TStealthRec = record
                      StartingSeg: word;
                      ParaSize: word;
                    end;

      TStatRec = record
                   ExtraMemType: byte;
                   InitConvMem: longint;
                   InitExtMem: longint;
                   InitExpMem: longint;
                   InitExtraMem: longint;
                   UnAvailConv: longint;
                   UnAvailExt: longint;
                   UnAvailExp: longint;
                   UnAvailExtra1: longint;
                   CodeSize: longint;
                   DataSize: longint;
                   TaskSize: longint;
                   DMASize: longint;
                   MAPSize: longint;
                   HiRAMSize: longint;
                   MappedROMSize: longint;
                   ConvMemSize: longint;
                   ExtMemSize: longint;
                   EMSXMSMemSize: longint;
                   UnAvailExtra2: longint;
                   ConvOverhead: longint;
                 end;

    var
      API: longint;
      Regs: registers;
      QEMMVersion: word;
      QEMMStat: TStatRec;
      LineNo: word;
      BufferPos: word;
      StealthBuf: array[1..64] of TStealthRec;
      StealthCount: word;
      StealthStart, StealthSize, StealthSet: word;
      InitMem, CurrentMem: longint;

    procedure ShowSize(long: longint);
      begin
      Write(long:8, ' (', (long / 1024):7:1, 'K)');
      end;

    begin
    API:=longint(APISeg) shl 16 + APIOfs;
    with Regs do
      begin
      Caption2('Memory Manager');
      Write('QEMM');
      Caption3('Version');
      AH:=3;
      LongCall(API, Regs);
      Write(unBCD(BH), decimal, AddZero(unBCD(BL)));
      QEMMVersion:=BX;
      Caption3('Mode');
      AH:=0;
      LongCall(API, Regs);
      if AL and 2 = 2 then
        begin
        Write('Auto');
        Caption3('Current Setting');
        end;
      if AL and 1 = 1 then
        Writeln('OFF')
      else
        Writeln('ON');
      Caption3('API Entry');
      SegOfs(APISeg, APIOfs);
      if Hi(QEMMVersion) >= 6 then
        begin
        Caption3('Stealth');
        AH:=$1E;
        AL:=0;
        LongCall(API, Regs);
        case CL of
          0:   Writeln('OFF');
          $46: Write('Frame');
          $4D: Write('Map')
        else
          Writeln('Unknown value - ', Hex(CL, 2), 'h');
        end;
        if (CL = $46) or (CL = $4D) then
          begin
          Caption3('Number of ROMs Stealthed');
          AH:=$1E;
          AL:=1;
          LongCall(API, Regs);
          if NoCarry(Regs) then
            Writeln(BX)
          else
            Writeln('Error');
          end;
        end;
      AH:=$17;
      ES:=Seg(QEMMStat);
      DI:=Ofs(QEMMStat);
      QEMMStat.ConvOverhead:=0;
      LongCall(API, Regs);
      if not NoCarry(Regs) then
        begin
        Writeln('Unable to retrieve QEMM Statistics!');
        Writeln('This information may only be available with 5.00 or newer.');
        end
      else
        with QEMMStat do
          begin
          Writeln;
          Caption1('---Initial Memory Settings---');
          Writeln;
          Caption3('Conventional');
          ShowSize(InitConvMem);
          Caption3('  Extended');
          ShowSize(InitExtMem);
          Writeln;
          Caption3('    Expanded');
          ShowSize(InitExpMem);
          if ExtraMemType = 1 then
            Caption3('    Shadow')
          else
            Caption3(' Top/Other');
          ShowSize(InitExtraMem);
          Writeln;
          Caption3('       Total');
          InitMem:=InitConvMem + InitExtMem + InitExtraMem;
          ShowSize(InitMem);
          Writeln;
          Writeln;
          Caption1('---Current Memory Settings---');
          Writeln;
          Caption3('Conventional');
          ShowSize(ConvMemSize);
          Caption3('  Extended');
          ShowSize(ExtMemSize);
          Writeln;
          Caption3('EMS/XMS Pool');
          ShowSize(EMSXMSMemSize);
          Caption3('  High RAM');
          ShowSize(HiRAMSize);
          Writeln;
          Caption3('  Mapped ROM');
          Showsize(MappedROMSize);
          Caption3('DMA Buffer');
          ShowSize(DMASize);
          Writeln;
          Caption3('       TASKS');
          ShowSize(TaskSize);
          Caption3('      MAPS');
          ShowSize(MAPSize);
          Writeln;
          Caption3('   QEMM code');
          ShowSize(CodeSize);
          Caption3(' QEMM data');
          ShowSize(DataSize);
          Writeln;
          CurrentMem:=CodeSize + DataSize + TaskSize + DMASize + MAPSize +
                      HiRAMSize + MappedROMSize + ConvMemSize + ExtMemSize +
                      EMSXMSMemSize;
          Caption3('       Total');
          Showsize(CurrentMem);
          Caption3('Unassigned');
          ShowSize(InitMem - CurrentMem);
          Writeln;
          Caption3('                 Conventional Memory Overhead');
          ShowSize(ConvOverhead);
          Writeln;
          Writeln;
          Caption1('---Unavailable Memory Settings---');
          Writeln;
          Caption3('Conventional');
          ShowSize(UnAvailConv);
          Caption3('  Extended');
          ShowSize(UnAvailExt);
          Writeln;
          Caption3('    Expanded');
          ShowSize(UnAvailExp);
          Caption3('Shadow/Top');
          ShowSize(UnAvailExtra1 + UnAvailExtra2);
          Writeln;
          end;
      AH:=$11;
      AL:=0;
      ES:=Seg(Buffer);
      DI:=Ofs(Buffer);
      LongCall(API, regs);
      if not NoCarry(Regs) then
        Writeln('Unable to get Memory type map.')
      else
        begin
        Pause1;
        if EndIt then
          Exit;
        ClrScr;
        TextColor(LightGray);
        Write('                 Memory Type/Memory Access/Stealth Info');
        for LineNo:=0 to $0F do
          begin
          GotoXY(1, 17 - LineNo);
          Write(Hex(LineNo, 1), 'x00');
          end;
        GotoXY(1, 18);
        Writeln('      0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F');
        Writeln('m=mappable RAM, M=Mapped ROM, H=High RAM, X=eXcluded, V=Video, R=ROM');
        Writeln('A=Adapter ROM, s=split ROM, F=EMS Page Frame, r=RAMmable, C=Conventional');
        Write('-=Not Accessed, +=Read, *=Written, !=Read/Written, S=Stealth ROM');
        TextColor(LightCyan);
        Window(Lo(WindMin) + 6, Hi(WindMin) + 2, twidth, Hi(WindMin) + 18);
        if EMSOK then
          begin
          AH:=$41;
          Intr($67, regs);
          if AH = 0 then
            for BufferPos:=BX div 256 to (BX div 256) + 15 do
              Buffer[BufferPos]:=9;
          end;
        for BufferPos:=0 to 255 do
          begin
          GotoXY(4 * (BufferPos mod 16) + 1, 16 - (BufferPos div 16));
          if Buffer[BufferPos] > $B then
            Write('?')
          else
            Write(QEMMMemType[Buffer[BufferPos]]);
          end;
        AH:=$16;
        AL:=0;
        ES:=Seg(Buffer);
        DI:=Ofs(Buffer);
        LongCall(API, Regs);
        if not NoCarry(Regs) then
          FillChar(Buffer, SizeOf(Buffer), #255);
        for BufferPos:=0 to 255 do
          begin
          GotoXY(4 * (BufferPos mod 16) + 2, 16 - (BufferPos div 16));
          if Buffer[BufferPos] > 3 then
            Write('?')
          else
            Write(QEMMMemAccess[Buffer[BufferPos]]);
          end;
        FillChar(Buffer, SizeOf(Buffer), 0);
        AH:=$1E;
        AL:=1;
        LongCall(API, Regs);
        if (BX > 0) and (BX < 65) then
          begin
          AH:=$1E;
          AL:=2;
          ES:=Seg(StealthBuf);
          DI:=Ofs(StealthBuf);
          LongCall(API, Regs);
          if NoCarry(Regs) then
            for StealthCount:=1 to BX do
              begin
              StealthSize:=StealthBuf[StealthCount].ParaSize div 256;
              StealthStart:=StealthBuf[StealthCount].StartingSeg div 256;
              for StealthSet:=StealthStart to StealthStart + StealthSize do
                Buffer[StealthSet]:=1;
              end;
          end;
        for BufferPos:=0 to 255 do
          begin
          GotoXY(4 * (BufferPos mod 16) + 3, 16 - (BufferPos div 16));
          if Buffer[BufferPos] > 1 then
            Write('?')
          else
            Write(QEMMStealth[Buffer[BufferPos]]);
          end;
        end;
      end;
    end;

begin
with Regs do
  begin
  S:='QEMM386$'#0;
  AX:=$3D00;
  DS:=Seg(S);
  DX:=Ofs(S) + 1;
  MsDos(Regs);
  if not NoCarry(Regs) then
    Writeln('Currently, only QEMM 4.23 or newer is supported. It was not found.')
  else
    begin
    Handle:=AX;
    AX:=$4402;
    BX:=Handle;
    CX:=4;
    DS:=Seg(Buffer);
    DX:=Ofs(Buffer);
    MsDos(Regs);
    if not NoCarry(Regs) then
      if EMSOK then
        begin
        AH:=$3F;
        CX:=$5145; {QE}
        DX:=$4D4D; {MM}
        Intr($67, regs);
        if AH = 0 then
          ShowQEMMinfo(ES, DI)
        else
          begin
          Writeln('QEMM possibly found, but both the IOCTL and special EMS calls failed.');
          Writeln('INFOPLUS is unable to find the API entry point.');
          end;
        end
      else
        begin
        Writeln('QEMM possibly found, but the IOCTL call failed. No EMS memory is being');
        Writeln('provided, preventing a secondary attempt to find the API entry point.');
        end
    else
      ShowQEMMinfo(word(Buffer[3]) shl 8 + Buffer[2],
                  word(Buffer[1]) shl 8 + Buffer[0]);
    AH:=$3E;
    BX:=Handle;
    MsDos(Regs);
    end;
  end;
end;
end.