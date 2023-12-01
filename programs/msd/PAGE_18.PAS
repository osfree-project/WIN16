unit page_18;

interface

uses crt, dos, ifpextrn, ifpglobl, ifpcomon;

procedure page18;

implementation

procedure page18;

var
  i : 1..63;
  s: string;
  xlong: longint;
  SaveX, Savey: byte;



  procedure Shells;
    var
      xword1: word;
      regs: registers;

    begin
    Caption1('----Shells and Shell enhancers----');
    Writeln;
    Caption2('JP Software 4DOS');
    with regs do
      begin
      AX:=$D44D;
      BX:=0;
      CX:=0;
      DX:=0;
      Intr($2F, regs);
      if AX <> $44DD then
        Writeln('no')
      else
        begin
        Writeln('yes');
        Caption3('version');
        xword1:=BH;
        Write(BL, decimal);
        ZeroPad(xword1);
        Caption3('shell no.');
        Write(DL);
        Caption3('PSP segment');
        Writeln(Hex(CX, 4))
        end;
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('JP Software KSTACK.COM');
    with regs do
      begin
      AX:=$D44F;
      BX:=0;
      CX:=0;
      DX:=0;
      Intr($2F, regs);
      yesorno(AX = $44DD);
      end;
    Pause3(-2);
    if endit then
      Exit;
    Caption2('Norton NDOS');
    with regs do
      begin
      AX:=$E44D;
      BX:=0;
      CX:=0;
      DX:=0;
      Intr($2F, regs);
      if AX <> $44EE then
        Writeln('no')
      else
        begin
        Writeln('yes');
        Caption3('version');
        xword1:=BH;
        Write(BL, decimal);
        ZeroPad(xword1);
        Caption3('shell no.');
        Write(DL);
        Caption3('PSP segment');
        Writeln(Hex(CX, 4))
        end;
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('WildUnix');
    with regs do
      begin
      AH:=$4E;
      DS:=0;
      DX:=0;
      MsDos(regs);
      yesorno(AH = $99);
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Anarkey');
    with regs do
      begin
      AX:=$E300;
      Intr($2F, regs);
      case AL of
        $00: Writeln('no');
        $FE: Writeln('yes; but suspended');
        $FF: Writeln('yes; and active');
      else
        Writeln('???')
      end
      end;
    end; {Shells}

  procedure DosExtenders;
    var
      xbool1: boolean;
      xbyte: byte;

    begin
    Pause3(-1);
    if endit then
      Exit;
    Caption1('----DOS Extenders----');
    Writeln;
    Caption2('DOS/16M');
    with regs do
      begin
      AX:=$BF02;
      DX:=0;
      Intr($15, regs);
      yesorno(DX <> 0);
      end;
    Pause3(-4);
    if endit then
      Exit;
    Caption2('Phar Lap DOS Extender');
    xbool1:=false;
    xbyte:=1;
    with regs do
      repeat
        AX:=$ED00;
        BL:=xbyte;
        Intr($2F, regs);
        if (AL = $FF) and (SI = $5048 {PH}) and (DI = $4152 {AR}) then
          begin
          xbool1:=true;
          Write('yes');
          Caption3('type');
          case xbyte of
            1: Write('286dosx v1.3+ SDK');
            2: Write('286dosx v1.3+ RTK');
            3: Write('386dosx v4.0+ SDK');
            4: Write('386dosx v4.0+ RTK')
          end;
          Caption3('version');
          Write(CH, decimal);
          ZeroPad(CL);
          end;
        Inc(xbyte);
      until xbyte > 4;
    if not xbool1 then
      Writeln('no');
    end; {DosExtenders}

  procedure MemUtils;
    type
      T386maxbuf = record
                     version: byte;
                     signature: array[1..6] of char;
                     verstr: array[1..4] of char;
                     lowseg: word;
                     unkw1: word;
                     unkw2: word;
                     flags1: word;
                     unk1: array [1..16] of byte;
                     int15port: word;
                     int67port: word;
                     unkw3: word;
                     unkw4: word;
                     unkd1: longint;
                     unkd2: longint;
                     sysconfig: word;
                     unk2: array [1..8] of byte;
                     flags2: word;
                     flags3: word;
                     flags4: word;
                     unkw5: word;
                     extfree: word;
                     unkd3: longint;
                     unkw6: word;
                     unkd4: longint;
                     flags5: word;
                     oldint21ofs: word;
                     oldint21seg: word;
                     emsofs: word;
                     emsseg: word;
                     extra: byte;
                   end;

    var
      V386maxbuf: T386maxbuf;
      QEMMid: byte;
      xword: word;
      xbyte: byte;
      xlong: longint;
      foundit: boolean;

    begin
    Pause3(-3);
    if endit then
      Exit;
    Caption1('----Memory Managers and Memory utilities----');
    Writeln;
    Caption2('QEMM');
    with regs do
      begin
      QEMMid:=$D2;
      foundit:=false;
      repeat
        AH:=QEMMId;
        AL:=0;
        BX:=$5144; {'QD'}
        CX:=$4D45; {'ME'}
        DX:=$4D30; {'M0'}
        Intr($2F, regs);
        if (AL = $FF) and (BX = $4D45) and (CX = $4D44) and (DX = $5652) then
          foundit:=true
        else
          begin
          if QEMMid < $FF then
            Inc(QEMMid)
          else
            QEMMid:=$C0;
          end;
      until foundit or (QEMMid = $D2);
      if not foundit then
        Writeln('no')
      else
        begin
        AH:=QEMMid;
        AL:=1;
        BX:=$5145; {'QE'}
        CX:=$4D4D; {'MM'}
        DX:=$3432; {'42'}
        Intr($2F, regs);
        if BX = $4F4B {'OK'} then
          begin
          Write('yes');
          Caption3('API entry');
          SegOfs(ES, DI);
          xlong:=longint(ES) shl 16 + DI;
          Caption3('version');
          AH:=3;
          longcall(xlong, regs);
          if not nocarry(regs) then
            Write('error')
          else
            Write(unBCD(BH), decimal, AddZero(unBCD(BL)));
          xword:=BX;
          Caption3('status');
          AH:=0;
          longcall(xlong, regs);
          if not nocarry(regs) then
            Write('error')
          else
            if AL and 1 = 1 then
              Write('OFF')
            else
              if AL and 2 = 2 then
                 Write('Auto')
              else
                Write('ON');
          Writeln;
          Caption3('High RAM');
          AH:=$12;
          longcall(xlong, regs);
          if not nocarry(regs) then
            Write('error')
          else
            begin
            yesorno2(BX <> 0);
            if BX <> 0 then
              begin
              Caption3('first MCB at');
              Write(Hex(BX, 4));
              end;
            end;
          if Hi(xword) >= 6 then
            begin
            Caption3('Stealth');
            AX:=$1E00;
            longcall(xlong, regs);
            if not nocarry(regs) then
              Write('error')
            else
              begin
              case CL of
                  0: Write('OFF');
                $46: Write('Frame');
                $4D: Write('Map')
              else
                Write('????');
              end;
              if (CL = $46) or (CL = $4D) then
                begin
                Caption3('Stealthed ROMs');
                AX:=$1E01;
                longcall(xlong, regs);
                if not nocarry(regs) then
                  Write('error')
                else
                  Write(BX);
                end;
              end;
            end;
          Writeln;
          end
        else
          Writeln('no')
        end;
      Pause3(-1);
      if endit then
        Exit;
      Caption2('Quarterdeck Manifest (memory resident)');
      if not foundit then
        Writeln('no')
      else
        begin
        AH:=QEMMid;
        AL:=1;
        BX:=$4D41; {'MA'}
        CX:=$4E49; {'NI'}
        DX:=$4645; {'FE'}
        Intr($2F, regs);
        yesorno(BX = $5354 {'ST'});
        end;
      Pause3(-3);
      if endit then
        Exit;
      Caption2('Quarterdeck VIDRAM');
      if not foundit then
        Writeln('no')
      else
        begin
        AH:=QEMMid;
        AL:=1;
        BX:=$5649; {'VI'}
        CX:=$4452; {'DR'}
        DX:=$414D; {'AM'}
        Intr($2F, regs);
        if BX = $4F4B {'OK'} then
          begin
          Write('yes');
          Caption3('at code segment');
          Write(Hex(ES, 4));
          xlong:=longint(ES) shl 16 + DI;
          AH:=0;
          LongCall(xlong, regs);
          Caption3('state');
          case AL of
            0: Write('off');
            1: Write('no EGA');
            2: Write('no graphics')
          else
            Write(AL, '???');
          end;
          Writeln;
          Caption3('extra RAM');
          case BL of
            0: Write('not used');
            1: Write('from EGA');
            2: Write('from EMS')
          else
            Write(BL, '???')
          end;
          Caption3('override');
          if BH and 1 = 1 then
            Write('enabled')
          else
            Write('disabled');
          Caption3('MDA detected');
          YesOrNo(BH and 8 = 8);
          Caption3('monitor type');
          case CL of
              0: Write('default');
              1: Write('mono');
            $80: Write('color')
          else
            Write(CL, '???');
          end;
          Caption3('memory top at');
          Writeln(Hex(SI, 4), ' (', SI div 64, 'K)');
          end
        else
          Writeln('no');
        end
      end;
    Pause3(-2);
    if endit then
      Exit;
    Caption2('386^Max');
    with regs do
      begin
      s:='386MAX$$'#0;
      AX:=$3D00;
      DS:=Seg(s);
      DX:=Ofs(s) + 1;
      MsDos(regs);
      if not nocarry(regs) then
        Writeln('no')
      else
        begin
        xbyte:=AX;
        AX:=$4402;
        BX:=xbyte;
        CX:=$5A;
        DS:=Seg(V386Maxbuf);
        DX:=Ofs(V386Maxbuf);
        V386Maxbuf.version:=3;
        MsDos(regs);
        if not nocarry(regs) then
          Writeln('Maybe; IOCTL call failed')
        else
          with V386maxbuf do
            if signature <> '386MAX' then
              Writeln('No; wrong signature found - "', signature, '"')
            else
              begin
              Write('yes');
              Caption3('version');
              Write(verstr[1], decimal, verstr[3], verstr[4]);
              Caption3('at segment');
              Writeln(Hex(lowseg, 4));
              Caption3('EMS active');
              yesorno2(flags1 and $0080 = $0080);
              Caption3('Windows 3 support');
              yesorno(flags4 and 1 <> 1);
              end;
        AH:=$3E;
        BX:=xbyte;
        MsDos(regs);
        end
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('MICEMM');
    if not EMSOK then
      Writeln('no')
    else
      with Regs do
        begin
        AX:=$58F0;
        Intr($67, Regs);
        if AH <> 0 then
          Writeln('no')
        else
          begin
          Write('yes');
          Caption3('Code Segment');
          Writeln(Hex(BX, 4));
          end;
        end;
    Pause3(-1);
    if EndIt then
      Exit;
    Caption2('EMM386');
    if not EMSOK then
      Writeln('no')
    else
      with Regs do
        begin
        AX:=$FFA5;
        Intr($67, Regs);
        if AX <> $845A then
          Writeln('no')
        else
          begin
          Write('yes');
          Caption3('API entry');
          SegOfs(BX, CX);
          Caption3('Status');
          xlong:=longint(BX) shl 16 + CX;
          AH:=0;
          LongCall(xlong, Regs);
          if AL and 1 = 1 then
            Write('ON')
          else
            Write('OFF');
          Caption3('Weitek');
          AH:=2;
          AL:=0;
          LongCall(xlong, Regs);
          if AL and 1 = 1 then
            begin
            Write('present ');
            if AL and 2 = 2 then
              Writeln('and enabled')
            else
              Writeln('but disabled')
            end
          else
            Writeln('not present');
          end;
        end;
    Pause3(-4);
    if endit then
      Exit;
    Caption2('Virtual DMA Spec. (VDS)');
    with regs do
      begin
      AX:=$354B;
      MsDos(regs);
      if (ES = 0) and (BX = 0) then
        Writeln('no')
      else
        begin
        AX:=$8102;
        DX:=0;
        Flags:=FCarry;
        Intr($4B, regs);
        yesorno2(nocarry(regs));
        if nocarry(regs) then
          begin
          Caption3('version');
          Write(AH, decimal, Hex(AL, 2));
          Caption3('product');
          case BX of
            $0000: Write('QMAPS/HPMM');
            $0001: Write('MSDOS EMM386');
            $0003: Write('Windows 3');
            $0300: Write('OS/2');
            $0EDC: Write('DRDOS EMM386');
            $4560: Write('386^Max');
            $4D53: Write('Memory Cmdr');
            $5145: Write('QEMM');
            $524D: Write('Netroom')
          else
            Write(Hex(BX, 4));
          end;
          Caption3('rev.');
          Writeln(CH, decimal, Hex(CL, 2));
          Caption3('max. DMA buffer size');
          Write((longint(SI) * 65536 + DI)/1024:0:1, 'K');
          Caption3('transfers OK in');
          if DX and 1 = 1 then
            Writeln('First Meg only')
          else
            Writeln('any address');
          Caption3('buffer in first meg');
          yesorno2(DX and 2 = 2);
          Caption3('auto-remap enabled');
          yesorno2(DX and 4 = 4);
          Caption3('contiguous memory');
          yesorno(DX and 8 = 8);
          Caption3('BIOS Data bit set');
          yesorno(Mem[$40:$7B] and $20 = $20);
          end
        else
          Writeln;
        end;
      end;
    end; {MemUtils}

  procedure MultiTaskers;

    function windev(device: byte): word;
      var
        regs: registers;
        saveit: word;

      begin
      with regs do
        begin
        AX:=$1682;
        Intr($2F, regs);
        AX:=$170A;
        DX:=device;
        Intr($2F, regs);
        windev:=AX
        end
      end; {windev}

    const
      winclass: array[0..6] of string[21] = ('vector plotter', 'raster display',
                  'raster printer', 'raster camera', 'character-stream, PLP',
                  'Metafile, VDM', 'display-file');

    var
      xword1: word;

    begin
    Pause3(-6);
    if endit then
      Exit;
    Caption1('----Multi-Taskers and Task Switchers + Utilities---');
    Writeln;
    Caption2('Quarterdeck Desqview');
    if OSMajor >= 10 then
      Writeln('no')
    else
      begin
      with regs do
        begin
        AX:=$2B01;
        CX:=$4445; {DE}
        DX:=$5351; {SQ}
        MsDos(regs);
        if AL = $FF then
          Writeln('no')
        else
          begin
          Write('yes');
          Caption3('version');
          if BX = $0002 then
            Writeln('2', decimal, '00')
          else
            begin
            Write(BH, decimal);
            ZeroPad(BL);
            Writeln;
            end;
          Caption3('window number');
          AX:=$DE07;
          Intr($15, regs);
          Write(AX);
          Caption3('true video mode');
          AX:=$DE1E;
          Intr($15, regs);
          Write(BL);
          Caption3('width');
          Write(CH);
          Caption3('height');
          Writeln(CL);
          Caption3('      common memory -> avail');
          AX:=$DE04;
          Intr($15, regs);
          Write(BX:6);
          Caption3('largest');
          Write(CX:6);
          Caption3('total');
          Writeln(DX:6);
          Caption3('conventional memory -> avail');
          AX:=$DE05;
          Intr($15, regs);
          Write(BX:5, 'K');
          Caption3('largest');
          Write(CX:5, 'K');
          Caption3('total');
          Writeln(DX:5, 'K');
          Caption3('    expanded memory -> avail');
          AX:=$DE06;
          Intr($15, regs);
          Write(BX:5, 'K');
          Caption3('largest');
          Write(CX:5, 'K');
          Caption3('total');
          Writeln(DX:5, 'K');
          end;
        end;
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('DOS 5 task switcher');
    with regs do
      begin
      AX:=$4B02;
      BX:=0;
      ES:=0;
      DI:=0;
      Intr($2F, regs);
      if nocarry(regs) and (AX = 0) and (BX = 0) then
        begin
        Write('yes');
        Caption3('switcher entry point');
        SegOfs(ES, DI);
        Writeln;
        end
      else
        Writeln('no');
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('DRDOS TaskMAX');
    with regs do
      begin
      AX:=$2700;
      BX:=0;
      CX:=0;
      Intr($2F, regs);
      if AL <> $FF then
        Writeln('no')
      else
        begin
        Write('yes');
        Caption3('version');
        AX:=$2701;
        Intr($2F, regs);
        Write(DX);
        Caption3('maximum tasks');
        Write(AX);
        Caption3('active tasks');
        Writeln(CX)
        end;
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('TAME');
    with regs do
      begin
      AX:=$2B01;
      CX:=$5441;
      DX:=$4D45;
      MsDos(regs);
      if AL <> 2 then
        Writeln('no')
      else
        begin
        Write('yes');
        Caption3('data area');
        Writeln(Hex(ES, 4), ':', Hex(DX, 4))
        end
      end;
    Pause3(-6);
    if endit then
      Exit;
    Caption2('Microsoft Windows');
    with regs do
      begin
      AX:=$1600;
      Intr($2F, regs);
      case AL of
      $01,$FF: begin
               Writeln('yes');
               Caption3('version');
               Writeln('Windows/386 2.x')
               end;
      $00,$80: begin
               AX:=$4680;
               Intr($2F, regs);
               if AX = 0 then
                 begin
                 Writeln('yes');
                 Caption3('mode');
                 Writeln('Real or Standard')
                 end
               else
                 Writeln('no');
               end;
      $02..$7F,$81..$FE: begin
                         Writeln('yes');
                         Caption3('version');
                         Write(AL, decimal, AH, ' enhanced mode');
                         Caption3('Virtual Machine ID');
                         AX:=$1683;
                         Intr($2F, regs);
                         Writeln(BX);
                         Caption3('WINOLDAP support');
                         AX:=$1700;
                         Intr($2F, regs);
                         if AX = $1700 then
                           Writeln('no')
                         else
                           begin
                           Write('yes');
                           Caption3('version');
                           Writeln(AL, decimal, AH);
                           end;
                         Caption3('Driver version');
                         xword1:=windev(0);
                         Write(Hi(xword1), decimal, Lo(xword1));
                         Caption3('Device type');
                         Writeln(winclass[Lo(windev(2))]);
                         Caption3('Pixel width');
                         Write(windev(8));
                         Caption3('height');
                         Write(windev($A));
                         Caption3('colors');
                         Write(windev($18));
                         Caption3('bits/pixel');
                         Write(windev($C));
                         Caption3('bit planes');
                         Writeln(windev($E));
                         Caption3('X aspect');
                         Write(windev($28));
                         Caption3('Y aspect');
                         Writeln(windev($2A));
                         Caption3('brushes');
                         Write(windev($10));
                         Caption3('pens');
                         Write(windev($12));
                         Caption3('markers');
                         Write(windev($14));
                         Caption3('fonts');
                         Writeln(windev($16));
                         end;
      end; {case}
      end;
    end; {MultiTaskers}

  procedure NortonUtils;
    procedure nortonstatus(b: byte);
      begin
      case b of
        0: Write('disabled');
        1: Write('enabled')
      else
        Write('unknown')
      end
      end; {nortonstatus}

    var
      regs: registers;

    begin
    Pause3(-1);
    if endit then
      Exit;
    Caption1('----Norton Utilities----');
    Writeln;
    Caption2('Norton NCACHE');
    with regs do
      begin
      AX:=$FE00;
      BX:=0;
      CX:=0;
      DX:=0;
      DI:=$4E55; {NU}
      SI:=$4346; {CF}
      Intr($2F, regs);
      if SI = $6366 {cf} then
        begin
        Write('yes (NCACHE-F or NCACHE v6+)');
        Caption3('status');
        nortonstatus(AH);
        Writeln;
        end
      else
        begin
        AX:=$FE00;
        BX:=0;
        CX:=0;
        DX:=0;
        DI:=$4E55; {NU}
        SI:=$4353; {CS}
        Intr($2F, regs);
        if SI = $6373 {cs} then
          begin
          Write('yes (NCACHE-S)');
          Caption3('status');
          nortonstatus(AH);
          Writeln
          end
        else
          Writeln('no');
        end
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Norton Diskreet');
    with regs do
      begin
      AX:=$FE00;
      BX:=0;
      CX:=0;
      DX:=0;
      DI:=$4E55; {NU}
      SI:=$4443; {DC}
      Intr($2F, regs);
      if SI = $6463 {dc} then
        begin
        Write('yes');
        Caption3('status');
        nortonstatus(AH);
        Caption3('resident at');
        Writeln(Hex(CX, 4))
        end
      else
        Writeln('no');
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Norton DiskMon');
    with regs do
      begin
      AX:=$FE00;
      BX:=0;
      CX:=0;
      DX:=0;
      DI:=$4E55; {NU}
      SI:=$444D; {DM}
      Intr($2F, regs);
      if SI = $646D {dm} then
        begin
        Write('yes');
        Caption3('status');
        nortonstatus(AH);
        Caption3('resident at');
        Writeln(Hex(CX, 4));
        end
      else
        Writeln('no');
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Norton FileSave/EraseProtect');
    with regs do
      begin
      AX:=$FE00;
      BX:=0;
      CX:=0;
      DX:=0;
      DI:=$4E55; {NU}
      SI:=$4653; {FS}
      Intr($2F, regs);
      if SI = $6673 {fs} then
        begin
        Write('yes');
        Caption3('resident at');
        Writeln(Hex(CX, 4));
        end
      else
        Writeln('no');
      end;
   end; {NortonUtils}

   procedure VirusUtils;
     var
       regs: registers;

     begin
     Pause3(-2);
     if endit then
       Exit;
     Caption1('----Virus protectors---');
     Writeln;
     Caption2('F-PROT package -> F-LOCK');
     with regs do
       begin
       AX:=$4653;
       BX:=0;
       CX:=2;
       Intr($2F, regs);
       yesorno2(AX = $FFFF);
       Caption3('F-XCHK');
       AX:=$4653;
       BX:=0;
       CX:=3;
       Intr($2F, regs);
       yesorno2(AX = $FFFF);
       Caption3('F-POPUP');
       AX:=$4653;
       BX:=0;
       CX:=4;
       Intr($2F, regs);
       yesorno2(AX = $FFFF);
       Caption3('F-DLOCK');
       AX:=$4653;
       BX:=0;
       CX:=5;
       Intr($2F, regs);
       yesorno(AX = $FFFF)
       end;
     Pause3(-1);
     if endit then
       Exit;
     Caption2('TBScanX');
     with regs do
       begin
       AX:=$CA00;
       BX:=$5442; {TB}
       Intr($2F, regs);
       if (AL <> $FF) or (BX <> $7462 {tb}) then
         Writeln('no')
       else
         begin
         Write('yes');
         Caption3('version');
         AX:=$CA01;
         Intr($2F, regs);
         if AH <> $CA then
           Write(AH shr 4, decimal, AddZero(AH and $F))
         else
           Write('2.2-');
         Caption3('status');
         if AL = 0 then
           Writeln('disabled')
         else
           Writeln('enabled');
         end
       end;
     Pause3(-1);
     if endit then
       Exit;
     Caption2('Flu_Shot+');
     with regs do
       begin
       AX:=$FF0F;
       MsDos(regs);
       yesorno(AX = $0101);
       end;
     end; {VirusUtils}

  procedure SCSI;
    var
      xword1: word;
      regs: registers;

    begin
    Pause3(-2);
    if endit then
      Exit;
    Caption1('----SCSI drivers----');
    Writeln;
    Caption2('Common Access Method SCSI (CAM-SCSI)');
    with regs do
      begin
      AX:=$354F;
      MsDos(regs);
      if (ES <> 0) and (BX <> 0) then
        begin
        AX:=$8200;
        CX:=$8765;
        DX:=$CBA9;
        Intr($4F, regs);
        if (CX = $9ABC) and (DX=$5678) then
          begin
          s:='';
          for xword1:=DI to DI + 7 do
            s:=s + Chr(Mem[ES:xword1]);
          yesorno(s = 'SCSI_CAM');
          end
        else
          Writeln('no');
        end;
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('CMC International SCSI driver');
    with regs do
      begin
      AX:=$3578;
      MsDos(regs);
      s:='';
      for xword1:=BX + 3 to BX + 6 do
        s:=s + Chr(Mem[ES:xword1]);
      yesorno(s = 'SCSI');
      end;
    end; {SCSI}

  procedure DiskCaches;
    type
      smartdrvt = record
               write_through: byte;
               write_buffered: byte;
               cache_enabled: byte;
               drivertype: byte;
               cticks: word;
               locked: byte;
               reboot_flush: byte;
               full_track_write: byte;
               buffering_type: byte;
               origInt13ofs: word;
               origInt13seg: word;
               minorversion: byte;
               majorversion: byte;
               reserved: word;
               secs_read: word;
               secs_in_cache: word;
               secs_in_trk_buf: word;
               cache_hitrate: byte;
               track_buf_hitrate: byte;
               total_tracks: word;
               tracks_used: word;
               locked_tracks: word;
               dirty_tracks: word;
               current_size: word;
               original_size: word;
               minimum_size: word;
               lock_pointer_ofs: word;
               lock_pointer_seg: word;
             end;

    var
      xbyte: byte;
      smartdrvbuf: smartdrvt;
      xbool1: boolean;
      regs: registers;
      foundit: boolean;

    begin
    Pause3(-4);
    if endit then
      Exit;
    Caption1('----Disk Caches----');
    Writeln;
    Caption2('SMARTDRV');
    with regs do
      begin
      s:='SMARTAAR'#0;
      AX:=$3D00;
      DS:=Seg(s);
      DX:=Ofs(s) + 1;
      MsDos(regs);
      if not nocarry(regs) then
        begin
        AX:=$4A10;
        BX:=0;
        CX:=0;
        DX:=0;
        Intr($2F, regs);
        if AX = $BABE then
          begin
          Write('yes');
          Caption3('ver');
          Write(unBCD(Hi(BP)), decimal, AddZero(unBCD(Lo(BP))));
          Caption3('size now');
          AX:=$4A10;
          BX:=4;
          Intr($2F, regs);
          Write((longint(CX) * BX) div 1024, 'K');
          Caption3('min size');
          Write((longint(DX) * CX) div 1024, 'K');
          Caption3('element size');
          Writeln(CX div 1024, 'K');
          Caption3('cache hits');
          AX:=$4A10;
          BX:=0;
          Intr($2F, regs);
          Write(longint(DX) shl 16 + BX);
          Caption3('cache misses');
          Writeln(longint(DI) shl 16 + SI);
          for xbyte:=0 to $19 do
            begin
            Pause3(-1);
            if endit then
              Exit;
            AX:=$4A10;
            BX:=3;
            BP:=xbyte;
            DX:=0;
            Intr($2F, regs);
            if (DL <> $FF) then
              begin
              Caption3('Drive');
              Write(Chr(xbyte + Ord('A')));
              Caption3('read');
              yesorno3(DL and $80 <> $80);
              Caption3('write');
              yesorno3(DL and $40 <> $40);
              Caption3('buffered');
              AX:=$4A10;
              BX:=5;
              BP:=xbyte;
              Intr($2F, regs);
              yesorno3(AX = $BABE);
              Caption3('DBLSPACE');
              AX:=$4A11;
              BX:=0;
              Intr($2F, regs);
              if (AX = 0) and (BX = $444D) then
                begin
                AX:=$4A11;
                BX:=1;
                DL:=xbyte;
                Intr($2F, regs);
                if AX = 0 then
                  begin
                  yesorno3(BL and $80 = $80);
                  if BL and $80 = $80 then
                    begin
                    Write(' ', Chr((BL and $7F) + Ord('A')), ':\DBLSPACE.');
                    ZeroPad3(BH);
                    end;
                  end
                else
                  Write('(error ', Hex(AX, 4), ')');
                Writeln;
                end
              else
                Writeln('no');
              end;
            end;
          end
        else
          Writeln('no')
        end
      else
        begin
        xbyte:=AX;
        AX:=$4400;
        BX:=xbyte;
        MsDos(regs);
        if (not nocarry(regs)) or (DX and $4080 <> $4080) then
          Writeln('Maybe. IOCTL interface not supported.')
        else
          begin
          AX:=$4402;
          BX:=xbyte;
          CX:=SizeOf(smartdrvbuf);
          DS:=Seg(smartdrvbuf);
          DX:=Ofs(smartdrvbuf);
          MsDos(regs);
          if not nocarry(regs) then
            Writeln('Maybe. IOCTL read failed.')
          else
            begin
            Write('yes');
            with smartdrvbuf do
              begin
              Caption3('ver.');
              Write(majorversion, decimal, minorversion);
              Caption3('Size');
              Write(current_size * 16, 'K');
              Caption3('Max');
              Write(original_size * 16, 'K');
              Caption3('Min');
              Write(minimum_size * 16, 'K');
              Caption3('enabled');
              yesorno(cache_enabled = 1);
              Caption3('locked tracks');
              yesorno2(locked > 0);
              Caption3('write-through');
              yesorno2(write_through = 1);
              Caption3('write-buffered');
              yesorno2(write_buffered = 1);
              Caption3('hit rate');
              Writeln(cache_hitrate, '%');
              Caption3('DMA buffering');
              case buffering_type of
                0: Write('off');
                1: Write('on');
                2: Write('dynamic')
              else
                Write('(unknown)');
              end; {case}
              Caption3('memory type');
              case drivertype of
                1: Write('XMS');
                2: Write('EMS')
              else
                Write('unknown:', drivertype);
              end;
              Caption3('flush on reboot');
              yesorno(reboot_flush <> 0);
              Caption3('Tracks total');
              Write(total_tracks);
              Caption3('used');
              Write(tracks_used);
              Caption3('locked');
              Write(locked_tracks);
              Caption3('dirty');
              Writeln(dirty_tracks);
              end
            end
          end;
        AH:=$3E;
        BX:=xbyte;
        MsDos(regs);
        end
      end;
    Pause3(-4);
    if endit then
      Exit;
    Caption2('HyperDisk');
    with regs do
      begin
      xbyte:=$DF;
      foundit:=false;
      xbool1:=false;
      repeat
        AH:=xbyte;
        AL:=0;
        BX:=$4448; {DH}
        Intr($2F, regs);
        if (AL = $FF) and (CX = $5948 {YH}) then
          foundit:=true
        else
          if (xbyte = $DF) and (not xbool1) then
            begin
            xbyte:=$C0;
            xbool1:=true;
            end
          else
            Inc(xbyte);
      until (xbyte = 0) or (foundit);
      if xbyte = 0 then
        Writeln('no')
      else
        begin
        Write('yes');
        Caption3('at code segment');
        Write(Hex(BX, 4));
        Caption3('local data version');
        Write(DH, decimal, AddZero(unBCD(DL)));
        Writeln;
        AH:=xbyte;
        AL:=1;
        BX:=$4448; {DH}
        Intr($2F, regs);
        if AX = 0 then
          begin
          Caption3('buffers used');
          Write(BX);
          Caption3('buffers modified but not yet written');
          Writeln(CX);
          Caption3('   Floppies - cached');
          YesOrNo3(DL and $40 = $40);
          Caption3('verified');
          YesOrNo3(DL and 4 = 4);
          Caption3('staged writes');
          YesOrNo(DL and 1 = 1);
          Caption3('Hard Drives - cached');
          YesOrNo3(DL and $80 = $80);
          Caption3('verified');
          YesOrNo3(DL and 8 = 8);
          Caption3('staged writes');
          YesOrNo(DL and 2 = 2);
          end;
        end;
      end;
    end; {DiskCaches}

  procedure DiskCompress;
    type
      stackerbuft = record
                  signature: word;
                  unknown: word;
                  ddofs: word;
                  ddseg: word
                end;

    var
      xbyte, xbyte2: byte;
      stackerbuf: stackerbuft;
      xword, xword1, xword2: word;
      DSSect, DSFreeSect, DriveSect, FreeSect: longint;
      regs: registers;
      foundit: boolean;

    begin
    Pause3(-4);
    if endit then
      Exit;
    Caption1('----Disk Compressors----');
    Writeln;
    xword1:=Seg(stackerbuf);
    xword2:=Ofs(stackerbuf);
    Caption2('Stacker');
      asm
      mov  ax,$CDCD       {signature entry}
      mov  cx,1
      mov  dx,0
      push ds             {need to preserve these}
      push bp
      mov  ds,xword1      {pointer to address buffer}
      mov  bx,xword2
      int  $25            {DOS absolute read sectors}
      pop  cx             {remove old flags}
      pop  bp             {restore important regs}
      pop  ds
      mov  xword1,ax      {save return code}
      end;
    with stackerbuf do
      begin
      if (signature = $CDCD) and (MemW[ddseg:ddofs] = $A55A) then
        begin
        Write('yes');
        Caption3('version');
        Write((MemW[ddseg:ddofs + 2] / 100.0):5:2);
        Caption3('at address');
        SegOfs(ddseg, ddofs);
        Writeln;
        Caption3('Stacker drive(s)');
        foundit:=false;
        if (OSMajor = 3) and (OSMinor = 31) then
          xword:=$440E
        else
          xword:=$4408;
        for xbyte:=1 to 26 do
          with Regs do
            begin
            BX:=xbyte;
            AX:=xword;
            Mem[ddseg:ddofs + $3E]:=$FF;
            MsDos(Regs);
            xbyte2:=Mem[ddseg:ddofs + $3E];
            if Mem[ddseg:ddofs + $3E] <> $FF then
              begin
              Write(Chr(xbyte + $40), ' ');
              foundit:=true
              end
            end;
        if not foundit then
          Write('(none)');
        Writeln;
        Caption3('Swapped');
        foundit:=false;
        s:='';
        for xword:=ddofs + $52 to ddofs + $55 do
          s:=s + Chr(Mem[ddseg:xword]);
        if s = 'SWAP' then
          for xbyte:=0 to 25 do
            if Mem[ddseg:ddofs + $56 + xbyte] <> xbyte then
              begin
              if foundit then
                Write(', ');
              Write(Chr(xbyte + $41), ' was ',
                    Chr(Mem[ddseg:ddofs + $56 + xbyte] + $41));
              foundit:=true;
              end;
        if not foundit then
          Write('(none)');
        Writeln;
        end
      else
        Writeln('no');
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('DBLSPACE');
    with regs do
      begin
      AX:=$4A11;
      BX:=0;
      Intr($2F, regs);
      if (AX = 0) and (BX = $444D) then
        begin
        Write('yes');
        Caption3('First drive');
        Write(Chr(CL));
        Caption3('Last drive');
        Writeln(Chr(CL + CH - 1));
        xbool1:=false;
        for xbyte:=0 to $19 do
          begin
          Pause3(-4);
          if endit then
            Exit;
          AX:=$4A11;
          BX:=1;
          DL:=xbyte;
          Intr($2F, regs);
          if (AX = 0) and (BL and $80 = $80) then
            begin
            xbool1:=true;
            Caption3('Drive');
            Write(Chr(xbyte + Ord('A')));
            Caption3('Host');
            Write(Chr((BL and $7F) + Ord('A')), ':\DBLSPACE.');
            ZeroPad3(BH);
            AX:=$4A11;
            BX:=7;
            DL:=xbyte;
            Intr($2F, regs);
            if AX = 0 then
              begin
              Caption3('sectors');
              DSSect:=longint(MemW[DS:SI] + MemW[DS:SI + 2] * 65536);
              Write(DSSect);
              Caption3('free sectors');
              DSFreeSect:=longint(MemW[DS:SI + 4] + MemW[DS:SI + 6] * 65536);
              Write(DSFreeSect);
              end;
            Writeln;
            SaveX:=WhereX;
            SaveY:=WhereY;
            TextColor(LightRed+Blink);
            Write('  *retrieving information*');
            AH:=$1C;
            DL:=xbyte + 1;
            MsDos(regs);
            GotoXY(SaveX, SaveY);
            Write('                          ');
            GotoXY(SaveX, SaveY);
            if AL <> $FF then
              begin
              DriveSect:=longint(DX) * longint(AL);
              Caption3('  estimated ratio');
              Write((DriveSect / DSSect):5:2, ':1');
              AH:=$36;
              DL:=xbyte + 1;
              MsDos(regs);
              DriveSect:=longint(DX) * longint(AX);
              FreeSect:=longint(BX) * longint(AX);
              Caption3('actual ratio');
              Write(((DriveSect - FreeSect) / (DSSect - DSFreeSect)):5:2, ':1');
              end;
            Writeln;
            AX:=$4A11;
            BX:=8;
            DL:=xbyte;
            Intr($2F, regs);
            if AX = 0 then
              begin
              Caption3('  maximum File Fragment heap entries');
              Write(BX);
              Caption3('available');
              Write(CX);
              end;
            Writeln;
            AX:=$4A11;
            BX:=3;
            CL:=xbyte;
            Intr($2F, regs);
            if CL <> $FF then
              begin
              Caption3('  Driver strategy');
              SegOfs(ES, SI);
              Caption3('Driver interrupt');
              SegOfs(ES, DI);
              end;
            Writeln;
            end;
          end;
        if not xbool1 then
          Writeln('No DBLSPACE drives found!');
        end
      else
        Writeln('no');
      end;
    end; {DiskCompress}

  procedure MiscUtils;
    const
      pcAstatus: array[$FFFC..$FFFF] of string[23] = ('resident and active',
                   'resident and not active', 'memory resident mode',
                   'automatic mode');
      pcAspd: array[0..$F] of word = (50, 75, 110, 134, 150, 300, 600, 1200, 1800,
                2000, 2400, 4800, 7200, 9600, 19200, 38400);
      KeyScan: array[0..255] of string[5] =
       {00}( '??', 'ESC',   '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',
       {10}   '9',   '0',   '-',   '=',  'BS', 'TAB',   'Q',   'W',   'E',   'R',
       {20}   'T',   'Y',   'U',   'I',   'O',   'P',   '[',   ']', 'ENT',  '??',
       {30}   'A',   'S',   'D',   'F',   'G',   'H',   'J',   'K',   'L',   ';',
       {40}  '''',   '`',  '??',   '\',   'Z',   'X',   'C',   'V',   'B',   'N',
       {50}   'M',   ',',   '.',   '/', 'kp*',  '??',  '??',  'SP',  '??',  'F1',
       {60}  'F2',  'F3',  'F4',  'F5',  'F6',  'F7',  'F8',  'F9', 'F10',  '??',
       {70}  '??', 'HOME', 'UP','PGUP', 'kp-','LEFT', 'kp5','RIGHT','kp+', 'END',
       {80}'DOWN', 'PGDN','INS', 'DEL',  'F1',  'F2',  'F3',  'F4',  'F5',  'F6',
       {90}  'F7',  'F8',  'F9', 'F10',  'F1',  'F2',  'F3',  'F4',  'F5',  'F6',
      {100}  'F7',  'F8',  'F9', 'F10',  'F1',  'F2',  'F3',  'F4',  'F5',  'F6',
      {110}  'F7',  'F8',  'F9', 'F10',  '??','LEFT','RIGHT','END','PGDN','HOME',
      {120}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {130}  '??',  '??','PGUP', 'F11', 'F12', 'F11', 'F12', 'F11', 'F12', 'F11',
      {140} 'F12',  'UP', 'kp-', 'kp5', 'kp+','DOWN', 'INS', 'DEL',  '??', 'kp/',
      {150}  '??','HOME',  'UP','PGUP',  '??','LEFT',  '??','RIGHT', '??', 'END',
      {160}'DOWN','PGDN', 'INS', 'DEL', 'kp/',  '??','kpENT', '??',  '??',  '??',
      {170}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {180}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {190}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {200}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {210}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {220}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {230}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {240}  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',  '??',
      {250}  '??',  '??',  '??',  '??',  '??',  '??');

    type
      fossilbuft = record
                     fbufsize: word;
                     spec: byte;
                     rev: byte;
                     idstrofs: word;
                     idstrseg: word;
                     inbufsize: word;
                     infree: word;
                     outbufsize: word;
                     outfree: word;
                     scrwidth: byte;
                     scrlen: byte;
                     baudrate: byte;
                     extra: array[1..13] of byte;
                   end;
      vfossilbuft = record
                      fbufsize: word;
                      ver: word;
                      rev: word;
                      hifunc: word
                    end;
    var
      xbyte: byte;
      xword1: word;
      xbool1: boolean;
      xchar: char;
      fossilbuf: fossilbuft;
      vfossilbuf: vfossilbuft;
      regs: registers;
      foundit: boolean;

    begin
    Pause3(-2);
    if endit then
      Exit;
    Caption1('----Miscellaneous----');
    Writeln;
    Caption2('pcAnywhere');
    with regs do
      begin
      AH:=$79;
      Intr($16, regs);
      if AX < $FFFC then
        Writeln('no')
      else
        begin
        Write(pcAstatus[AX]);
        Caption3('port');
        AH:=$7C;
        Intr($16, regs);
        Write(AH);
        Caption3('baud rate');
        Writeln(pcAspd[AL]);
        end;
      end;
    Pause3(-5);
    if endit then
      Exit;
    Caption2('Disk Spool II');
    with regs do
      begin
      AH:=$A0;
      Intr($1A, regs);
      if AH = $B0 then
        begin
        Write('yes');
        Caption3('at segment');
        Writeln(Hex(ES, 4));
        Caption3('spooler is');
        case CH of
          $00: Writeln('disabled');
          $41: begin
               Writeln('enabled');
               Caption3('spooling file');
               xbyte:=0;
               repeat
                 xchar:=Chr(Mem[ES:BX]);
                 if xchar <> #0 then
                   Write(xchar);
                 Inc(xbyte);
                 Inc(BX);
               until (xchar = #0) or (xbyte >= 64);
               if xbyte = 0 then
                 Write('(none)');
               Writeln;
               end;
        else
          Writeln('??')
        end;
        Caption3('despooler is');
        case CL of
          $00: Writeln('disabled');
          $41: begin
               Write('enabled and ');
               case DL of
                 $00: Writeln('actively printing');
                 $41: Writeln('standing by');
               else
                 Writeln('?????');
               end;
               Caption3('despooler file');
               xbyte:=0;
               repeat
                 xchar:=Chr(Mem[ES:SI]);
                 if xchar <> #0 then
                   Write(xchar);
                 Inc(xbyte);
                 Inc(SI);
               until (xchar = #0) or (xbyte >= 64);
               if xbyte = 0 then
                 Write('(none)');
               Writeln;
               end;
        else
          Writeln('????');
        end;
        end
      else
        Writeln('no');
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Microsoft/LANtastic Network');
    regs.AH:=0;
    Intr($2A, regs);
    yesorno(regs.AH <> 0);
    Pause3(-1);
    if endit then
      Exit;
    Caption2('PC/TCP Packet driver');
    with regs do
      begin
      xbyte:=$60;
      foundit:=false;
      repeat
        AH:=$35;
        AL:=xbyte;
        MsDos(regs);
        s:='';
        for xword1:=BX + 3 to BX + $A do
          s:=s + Chr(Mem[ES:xword1]);
        if s = 'PKT DRVR' then
          foundit:=true;
        Inc(xbyte);
      until foundit or (xbyte = $81);
      if foundit then
        Writeln('yes, at interrupt $', Hex(xbyte - 1, 2))
      else
        Writeln('no')
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Inset');
    with regs do
      begin
      AH:=2;
      DX:=0;
      CX:=$07C3; {1987}
      Intr($17, regs);
      yesorno(CX = $07C2 {1986})
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Microsoft CD-ROM extensions');
    asm
    mov ax,$DADA
    push ax
    mov ax,$1100
    int $2F
    mov xbyte,al
    pop bx
    mov xword1,bx
    end;
    if (xbyte <> $FF) or (xword1 <> $ADAD) then
      Writeln('no')
    else
      with regs do
        begin
        Write('yes');
        Caption3('version');
        AX:=$150C;
        Intr($2F, regs);
        if BX = 0 then
          Writeln('1.xx')
        else
          Writeln(BH, decimal, addzero(BL));
        end;
    Pause3(-2);
    if endit then
      Exit;
    Caption2('Fossil');
    xbool1:=false;
    with regs do
      begin
      AH:=$BC;
      DX:=$1954;
      Intr($11, regs);
      if AX = $1954 then
        xbool1:=true;
      AX:=$1B00;
      DX:=$FF;
      CX:=SizeOf(fossilbuf);
      ES:=Seg(fossilbuf);
      DI:=Ofs(fossilbuf);
      Intr($14, regs);
      if AX <> $1B00 then
        begin
        Write('yes');
        Caption3('type');
        if xbool1 then
          Write('BNU')
        else
          if (CX = $3058 {0X}) and (DX = $2030 { 0}) then
            Write('X00')
          else
            Write('unknown');
        with fossilbuf do
          begin
          Caption3('specification level');
          Write(spec);
          Caption3('revision level');
          Writeln(rev);
          Caption3('ID string');
          while Mem[idstrseg:idstrofs] <> 0 do
            begin
            Write(Chr(Mem[idstrseg:idstrofs]));
            Inc(idstrofs)
            end;
          Writeln;
          end
        end
      else
        Writeln('no');
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Video Fossil');
    with regs do
      begin
      AX:=$8100;
      ES:=Seg(vfossilbuf);
      DI:=Ofs(vfossilbuf);
      Intr($14, regs);
      if AX <> $1954 then
        Writeln('no')
      else
        with vfossilbuf do
          begin
          Write('yes');
          Caption3('version');
          Write(ver);
          Caption3('revision');
          Write(rev);
          Caption3('highest function');
          Write('$', Hex(hifunc, 4));
          end
      end;
    Pause3(-3);
    if endit then
      Exit;
    Caption2('Advanced Power Management Spec.');
    with regs do
      begin
      AX:=$5300;
      BX:=0;
      Intr($15, regs);
      if (not NoCarry(regs)) or (BX <> $504D) then
        Writeln('no')
      else
        begin
        Write('yes');
        Caption3('version');
        Write(unBCD(AH), decimal, AddZero(unBCD(AL)));
        Caption3('enabled');
        YesOrNo((CX and 8) = 0);
        Caption3('supports 16-bit protected mode');
        YesOrNo2((CX and 1) = 1);
        Caption3('32-bit protected mode');
        YesOrNo((CX and 2) = 2);
        Caption3('AC line status');
        AX:=$530A;
        BX:=1;
        Intr($15, regs);
        if not NoCarry(regs) then
          Writeln('ERROR')
        else
          begin
          case BH of
              0: Write('off-line');
              1: Write('on-line');
            $FF: Write('unknown')
          else
            Write('???');
          end;
          Caption3('battery status');
          case BL of
              0: Write('high');
              1: Write('low');
              2: Write('critical');
              3: Write('charging');
            $FF: Write('unknown')
          else
            Write('???');
          end;
          Caption3('remaining life');
          if CL = $FF then
            Writeln('unknown')
          else
            Writeln(CL, '%');
          end;
        end;
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('Norton Guides');
    with regs do
      begin
      AX:=$F398;
      Intr($16, regs);
      YesOrNo(AX = $6A73);
      end;
    Pause3(-1);
    if endit then
      Exit;
    Caption2('After Dark for DOS');
    with regs do
      begin
      AX:=$C000;
      BX:=0;
      CX:=0;
      DX:=0;
      Intr($2F, regs);
      if (AL = $FF) and (BX=$4144 {AD}) and (CX=$2D44 {-D}) and
         (DX=$4F53 {OS}) then
        begin
        Write('yes');
        Caption3('at segment');
        AX:=$C001;
        ES:=0;
        Flags:=Flags and (not FCarry);
        Intr($2F, regs);
        if NoCarry(regs) then
          Write(Hex(ES, 4))
        else
          Write('ERROR');
        Caption3('minutes to wait');
        AX:=$C004;
        Flags:=Flags or FCarry;
        Intr($2F, regs);
        Writeln(BX);
        Caption3('blanker');
        AX:=$C006;
        Flags:=Flags or FCarry;
        Intr($2F, regs);
        if BX = 0 then
          Write('disabled')
        else
          Write('enabled');
        Caption3('password');
        AX:=$C00F;
        Flags:=Flags or FCarry;
        Intr($2F, regs);
        if BX = 0 then
          Write('disabled')
        else
          Write('enabled');
        Caption3('hot key');
        AX:=$C008;
        Flags:=Flags or FCarry;
        Intr($2F, regs);
        if AX = 0 then
          begin
          if CL and 3 <> 0 then
            Write('<SHIFT>');
          if CL and 4 = 4 then
            Write('<CTRL>');
          if CL and 8 = 8 then
            Write('<ALT>');
          Writeln('<', KeyScan[BH], '>')
          end;
        end
      else
        Writeln('no');
      end
    end; {MiscUtils}

  begin {procedure page_18}
  Shells;
  if endit then
    Exit;
  DosExtenders;
  if endit then
    Exit;
  MemUtils;
  if endit then
    Exit;
  MultiTaskers;
  if endit then
    Exit;
  NortonUtils;
  if endit then
    Exit;
  VirusUtils;
  if endit then
    Exit;
  SCSI;
  if endit then
    Exit;
  DiskCaches;
  if endit then
    Exit;
  DiskCompress;
  if endit then
    Exit;
  MiscUtils;
  end;
end.
