procedure page_02;

var
  cpu_info: cpu_info_t;
  save_cpu_type: byte;

procedure showNDP(a: string; b: word);
  begin
  writeln(a);
  caption2('  Infinity');
  case b and $1000 of
    $0000 : writeln('projective');
    $1000 : writeln('affine')
  end;
  caption2('  Rounding');
  case b and $0C00 of
    $0000 : writeln('to nearest or even');
    $0400 : writeln('down');
    $0800 : writeln('up');
    $0C00 : writeln('chop')
  end;
  caption2('  Precision');
  case b and $0300 of
    $0000 : writeln('24 bits');
    $0100 : writeln('(reserved)');
    $0200 : writeln('53 bits');
    $0300 : writeln('64 bits')
  end
  end; {showNDP}

  begin (* procedure page_02 *)
  caption2('CPU');
  cpu_info.test_type:='C';
  CPUID(cpu_info);
  with cpu_info do
    begin
    case cpu_type of
      $00 : writeln('8088');
      $01 : writeln('8086');
      $02 : writeln('V20');
      $03 : writeln('V30');
      $04 : writeln('80188');
      $05 : writeln('80186');
      $06 : writeln('80286');
      $07 : writeln('80386');
      $08 : Writeln('80486')
      else
        unknown('CPU', cpu_type, 2)
    end;
    save_cpu_type:=cpu_type;
    case cpu_type of
      $06..$08 : begin
                 caption3('Machine Status Word');
                 writeln(hex(MSW, 4));
                 caption3('  Bit 0 - Protected mode     ');
                 yesorno(MSW and 1 = 1);
                 caption3('  Bit 1 - Monitor Coprocessor');
                 yesorno(MSW and 2 = 2);
                 caption3('  Bit 2 - Emulate Coprocessor');
                 yesorno(MSW and 4 = 4);
                 caption3('Global Descriptor Table   ');
                 for i:=1 to 6 do
                   write(hex(GDT[i], 2), ' ');
                 writeln;
                 caption3('Interrupt Descriptor Table');
                 for i:=1 to 6 do
                   write(hex(IDT[i], 2), ' ');
                 writeln
                 end
    end;
    caption3('Interrupts acknowledged immediately after segment register change');
    yesorno(intflag);
    caption2('Coprocessor');
    if save_cpu_type = $08 then
      showNDP('(built-in)', ndp_cw) {'486 has coprocessor built in}
    else
      begin
      test_type:='N';
      CPUID(cpu_info);
      case ndp_type of
        $00 : writeln('none');
        $01 : showNDP('8087', ndp_cw);
        $02 : showNDP('80287', ndp_cw);
        $03 : showNDP('80387', ndp_cw)
      else
        unknown('coprocessor', ndp_type, 4)
      end {case}
      end
  end;
  caption2('BIOS coprocessor flag set');
  yesorno(equip and $0002 = $0002);
  caption2('BIOS Weitek coprocessor flag set');
  with cpu_info do
    begin
    if (save_cpu_type = 7) or (save_cpu_type = 8) then
      begin
      test_type:='W';
      CPUID(cpu_info);
      yesorno(weitek and 1 = 1);
      if weitek and 1 = 1 then
        begin
        caption3('Weitek addressable in real mode');
        yesorno(weitek and $80 = $80)
        end
      end
    else
      Writeln('Not Applicable');
    end;
  end;