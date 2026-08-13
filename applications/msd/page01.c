/* PAGE01.C Ц Machine & ROM Identification (полный, без заглушек) */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include "msd.h"

static int in_pchar(char c) {
    return (c >= ' ' && c <= '~') || (c >= 0x80);
}

static int BIOSscan(unsigned seg, unsigned start, unsigned finish, unsigned *found) {
    const char *notices[] = { "(C)", "COPR.", "COPYRIGHT" };
    int i;
    unsigned pos;
    int found_any = 0;
    unsigned best = 0;

    for (i = 0; i < 3; i++) {
        const char *pat = notices[i];
        int len = strlen(pat);
        for (pos = start; pos <= finish - len; pos++) {
            int j;
            for (j = 0; j < len; j++) {
                if (toupper(peekb(seg, pos + j)) != pat[j])
                    break;
            }
            if (j == len) {
                if (!found_any || pos < best) {
                    best = pos;
                    found_any = 1;
                }
            }
        }
    }

    if (found_any) {
        while (best > start && in_pchar(peekb(seg, best - 1)))
            best--;
        *found = best;
        return 1;
    }
    return 0;
}

static void showBIOS(unsigned seg, unsigned ofs) {
    char c;
    while (1) {
        c = peekb(seg, ofs);
        if (!in_pchar(c)) break;
        putchar(c);
        ofs++;
        if (ofs == 0xFFFF) break;
    }
    cprintf("\r\n");
}

void page01(void) {
    unsigned xword1, xword2;
    unsigned char xbyte;
    char s[80];
    int xbool;
    unsigned i;
    unsigned ROMInfoSeg = 0, ROMInfoOfs = 0;
    unsigned char ROMDate[9] = "";
    union REGS r;

    Caption2("Machine type");
    if (toupper(peekb(0xF000, 0xE076)) == 'D') {
        s[0] = 0;
        for (xword1 = 0xE077; xword1 <= 0xE079; xword1++) {
            char c = toupper(peekb(0xF000, xword1));
            strncat(s, &c, 1);
        }
        if (strcmp(s, "ELL") == 0) {
            cprintf("Dell ");
            xbyte = peekb(0xF000, 0xE845);
            if ((xbyte >= 2 && xbyte <= 0x11) && (strcmp(dell_ids[xbyte], "?") != 0)) {
                cprintf("%s", dell_ids[xbyte]);
                Caption3("BIOS Revision");
                for (xword1 = 0xE845; xword1 <= 0xE847; xword1++)
                    putchar(peekb(0xF000, xword1));
            } else {
                cprintf("(unknown - ID is %02X)", xbyte);
            }
            cprintf("\r\n");
            Caption2("Standard BIOS call says");
            cprintf("\r\n");
        }
    }

    ROMDate[0] = 0;
    for (xword1 = 0xFFF5; xword1 <= 0xFFFC; xword1++) {
        char c = peekb(0xF000, xword1);
        strncat(ROMDate, &c, 1);
    }

    {
        r.x.ax = 0x6F00;
        r.x.bx = 0;
        r.x.cflag = 0;
        int86(0x16, &r, &r);
        if (nocarry(&r) && r.x.bx == 0x4850) {
            cprintf("HP Vectra series\r\n");
            Caption2("Standard BIOS call says");
        }
    }
    {
        r.x.ax = 0x4DD4;
        r.x.bx = 0;
        int86(0x15, &r, &r);
        if (r.x.bx == 0x4850) {
            cprintf("HP 95LX\r\n");
            Caption2("Standard BIOS call says");
        }
    }

    {
        r.h.ah = 0xC0;
        r.x.es = 0;
        r.x.bx = 0;
        r.x.cflag = 0;
        int86(0x15, &r, &r);
        if (nocarry(&r) && r.h.ah == 0) {
            ROMInfoSeg = r.x.es;
            ROMInfoOfs = r.x.bx;
            xword1 = peekw(ROMInfoSeg, ROMInfoOfs + 2);
            xbyte  = peekb(ROMInfoSeg, ROMInfoOfs + 4);

            switch (xword1) {
                case 0x00FC:
                    if (xbyte == 1) cprintf("PC-AT 2x9, 6MHz\r\n");
                    else cprintf("Industrial AT 7531/2\r\n");
                    break;
                case 0x01FC:
                    if (xbyte == 0x00) {
                        if (!strcmp(ROMDate, "11/15/85")) cprintf("PC-AT 319 or 339, 8MHz\r\n");
                        else if (!strcmp(ROMDate, "01/15&88")) cprintf("Toshiba T5200/100\r\n");
                        else if (!strcmp(ROMDate, "12/26*89")) cprintf("Toshiba T1200/XE\r\n");
                        else if (!strcmp(ROMDate, "07/24&90")) cprintf("Toshiba T5200/200\r\n");
                        else if (!strcmp(ROMDate, "09/17/87")) cprintf("Tandy 3000\r\n");
                        else cprintf("AT clone\r\n");
                    } else if (xbyte == 0x30) cprintf("Tandy 3000NL\r\n");
                    else cprintf("Compaq 286/386 or clone\r\n");
                    break;
                case 0x02FC: cprintf("PC-XT/286\r\n"); break;
                case 0x04FC:
                    if (xbyte == 3) cprintf("PS/2 Model 50Z 10MHz 286\r\n");
                    else cprintf("PS/2 Model 50 10MHz 286\r\n");
                    break;
                case 0x05FC: cprintf("PS/2 Model 60 10MHz 286\r\n"); break;
                case 0x06FC: cprintf("7552 Gearbox\r\n"); break;
                case 0x09FC:
                    if (xbyte == 2) cprintf("PS/2 Model 30-286\r\n");
                    else cprintf("PS/2 Model 25-286\r\n");
                    break;
                case 0x0BFC: cprintf("PS/1 Model 2011 10MHz 286\r\n"); break;
                case 0x42FC: cprintf("Olivetti M280\r\n"); break;
                case 0x45FC: cprintf("Olivetti M380 (XP1, 3, or 5)\r\n"); break;
                case 0x48FC: cprintf("Olivetti M290\r\n"); break;
                case 0x4FFC: cprintf("Olivetti M250\r\n"); break;
                case 0x50FC: cprintf("Olivetti M380 (XP7)\r\n"); break;
                case 0x51FC: cprintf("Olivetti PCS286\r\n"); break;
                case 0x52FC: cprintf("Olivetti M300\r\n"); break;
                case 0x81FC: cprintf("AT clone with Phoenix 386 BIOS\r\n"); break;
                case 0x00FB:
                    if (xbyte == 1) cprintf("PC-XT w/ Enh kbd, 3.5\" support\r\n");
                    else cprintf("PC-XT\r\n");
                    break;
                case 0x01FB: cprintf("PC-XT/2\r\n"); break;
                case 0x4CFB: cprintf("Olivetti M200\r\n"); break;
                case 0x00FA: cprintf("PS/2 Model 30\r\n"); break;
                case 0x01FA: cprintf("PS/2 Model 25/25L\r\n"); break;
                case 0x4EFA: cprintf("Olivetti M111\r\n"); break;
                case 0x00F9: cprintf("PC-Convertible\r\n"); break;
                case 0x00F8: cprintf("PS/2 Model 80 16MHz 386\r\n"); break;
                case 0x01F8: cprintf("PS/2 Model 80 20MHz 386\r\n"); break;
                case 0x04F8: cprintf("PS/2 Model 70 20MHz 386\r\n"); break;
                case 0x09F8: cprintf("PS/2 Model 70 16MHz 386\r\n"); break;
                case 0x0BF8: cprintf("PS/2 Model P70\r\n"); break;
                case 0x0CF8: cprintf("PS/2 Model 55SX 16MHz 386SX\r\n"); break;
                case 0x0DF8: cprintf("PS/2 Model 70 25MHz 386\r\n"); break;
                case 0x11F8: cprintf("PS/2 Model 90 25MHz 386\r\n"); break;
                case 0x13F8: cprintf("PS/2 Model 90 33MHz 386\r\n"); break;
                case 0x14F8: cprintf("PS/2 Model 90-AK9 25MHz 486\r\n"); break;
                case 0x16F8: cprintf("PS/2 Model 90-AKD 33MHz 486\r\n"); break;
                case 0x19F8: cprintf("PS/2 Model 35/35LS/40 20MHz 386SX\r\n"); break;
                case 0x1BF8: cprintf("PS/2 Model 70 25MHz 486\r\n"); break;
                case 0x1CF8: cprintf("PS/2 Model 65-121 16MHz 386SX\r\n"); break;
                case 0x1EF8: cprintf("PS/2 Model 55LS 16MHz 386SX\r\n"); break;
                case 0x23F8: cprintf("PS/2 Model L40 20MHz 386SX\r\n"); break;
                case 0x25F8: cprintf("PS/2 Model M57 20MHz 386SLC\r\n"); break;
                case 0x26F8: cprintf("PS/2 Model 57 20MHz 386SX\r\n"); break;
                case 0x2AF8: cprintf("PS/2 Model 95 50MHz 486\r\n"); break;
                case 0x2BF8: cprintf("PS/2 Model 90 50MHz 486\r\n"); break;
                case 0x2CF8: cprintf("PS/2 Model 95 20MHz 486SX\r\n"); break;
                case 0x2DF8: cprintf("PS/2 Model 90 20MHz 486SX\r\n"); break;
                case 0x2EF8: cprintf("PS/2 Model 95 20MHz 486SX+487SX\r\n"); break;
                case 0x2FF8: cprintf("PS/2 Model 90 20MHz 486SX+487SX\r\n"); break;
                case 0x30F8: cprintf("PS/1 Model 2121 16MHz 386SX\r\n"); break;
                case 0x50F8: cprintf("PS/2 Model P70 16MHz 386\r\n"); break;
                case 0x52F8: cprintf("PS/2 Model P75 33MHz 486\r\n"); break;
                case 0x61F8: cprintf("Olivetti P500\r\n"); break;
                case 0x62F8: cprintf("Olivetti P800\r\n"); break;
                case 0x80F8: cprintf("PS/2 Model 80 25 MHz 386\r\n"); break;
                default:
                    unknown("machine - model/type word", xword1, 4);
            }

            Caption3("BIOS revision level");
            cprintf("%u\r\n", peekb(ROMInfoSeg, ROMInfoOfs + 4));

            xbyte = peekb(ROMInfoSeg, ROMInfoOfs + 5);
            Caption3("DMA channel 3 used by hard disk BIOS");
            YesOrNo(xbyte & 0x80);
            Caption3("Slave 8259 present");
            YesOrNo(xbyte & 0x40);
            Caption3("Real-time clock");
            YesOrNo(xbyte & 0x20);
            Caption3("Keyboard intercept available");
            YesOrNo(xbyte & 0x10);
            Caption3("Wait for external event available");
            YesOrNo(xbyte & 0x08);
            Caption3("Extended BIOS data area segment");
            if (xbyte & 0x04) {
                union REGS r2;
                r2.h.ah = 0xC1;
                int86(0x15, &r2, &r2);
                if (nocarry(&r2))
                    cprintf("%04X\r\n", r2.x.es);
                else
                    dontknow();
            } else {
                cprintf("(none)\r\n");
            }
            Caption3("Bus type");
            if (xbyte & 1) cprintf("Dual ISA/Micro Channel\r\n");
            else if (xbyte & 2) cprintf("Micro Channel\r\n");
            else cprintf("ISA\r\n");

            xbyte = peekb(ROMInfoSeg, ROMInfoOfs + 6);
            Caption3("Keyboard Int 16h/Func 9 support");
            YesOrNo(xbyte & 0x40);
            Caption3("Keyboard controller");
            if (xbyte & 4) cprintf("non-8042\r\n");
            else cprintf("8042\r\n");
            Caption3("POS data supported");
            YesOrNo(xbyte & 0x20);
            Caption3("CPU enable/disable");
            YesOrNo(xbyte & 8);
            Caption3("Data streaming support");
            YesOrNo(xbyte & 2);

            xbyte = peekb(ROMInfoSeg, ROMInfoOfs + 7);
            Caption3("On-board SCSI");
            YesOrNo(xbyte & 8);
            Caption3("Info panel");
            YesOrNo(xbyte & 4);
            Caption3("IML system");
            YesOrNo(xbyte & 2);
            Caption3("IML supports SCSI");
            YesOrNo(xbyte & 1);

            pause1();
            ClrScr();
        } else {
            if (peekb(0xF000, 0xC000) == 0x21) {
                cprintf("Tandy 1000\r\n");
            } else {
                xbyte = peekb(0xFFFF, 0x000E);
                switch (xbyte) {
                    case 0xFF:
                        if (peekb(0xF000, 0xFFFD) == 0x46) cprintf("Olivetti M15\r\n");
                        else {
                            cprintf("PC ");
                            if (!strcmp(ROMDate, "04/24/81")) cprintf("(original)");
                            else if (!strcmp(ROMDate, "10/19/81")) cprintf("(revised BIOS)");
                            else if (!strcmp(ROMDate, "10/27/82")) cprintf("(HD, 640K, EGA supported)");
                            else cprintf("clone");
                            cprintf("\r\n");
                        }
                        break;
                    case 0xFE:
                        if (peekb(0xF000, 0xFFFD) == 0x43) cprintf("Olivetti M240\r\n");
                        else {
                            cprintf("PC-XT");
                            if (!strcmp(ROMDate, "11/08/82")) cprintf(" or Portable");
                            else if (strcmp(ROMDate, "08/16/82")) cprintf(" clone");
                            cprintf("\r\n");
                        }
                        break;
                    case 0xFD: cprintf("PCjr\r\n"); break;
                    case 0xFC: cprintf("PC-AT\r\n"); break;
                    case 0x9A: cprintf("Compaq XT or Compaq Plus\r\n"); break;
                    case 0x30: cprintf("Sperry PC\r\n"); break;
                    case 0x2D: cprintf("Compaq PC or Compaq Deskpro\r\n"); break;
                    default: unknown("machine - model byte", xbyte, 2);
                }
            }
        }
    }

    Caption2("BIOS source");
    if (BIOSscan(0xF000, 0xC000, 0xFFFF, &xword1))
        showBIOS(0xF000, xword1);
    else
        dontknow();

    if (ROMInfoSeg != 0) {
        char tmp[4] = {0};
        for (i = 0; i < 3; i++)
            tmp[i] = peekb(ROMInfoSeg, ROMInfoOfs + 0x0D + i);
        if (!strcmp(tmp, "PTL")) {
            Caption2("BIOS version");
            cprintf("%d.%02d", UnBCD(peekb(ROMInfoSeg, ROMInfoOfs + 0x0B)),
                    UnBCD(peekb(ROMInfoSeg, ROMInfoOfs + 0x0C)));
            Caption2(" last revision");
            for (xword1 = 0xFFDC; xword1 < 0xFFEC; xword1 += 2)
                putchar(peekb(0xF000, xword1));
            cprintf("\r\n");
        }
    }

    Caption2("BIOS date");
    xbool = 0;
    for (i = 0x0005; i < 0x0010; i++) {
        char c = peekb(0xFFFF, i);
        if (in_pchar(c)) {
            putchar(c);
            xbool = 1;
        } else break;
    }
    if (xbool) cprintf("\r\n");
    else dontknow();

    Caption2("BIOS extensions");
    xword1 = 0xC000;
    xbool = 0;
    for (i = 0; i < 95; i++) {
        if (peekw(xword1, 0) == 0xAA55) {
            if (!xbool) {
                cprintf("\r\n");
                Window(3, WhereY() + 1, twidth, tlength - 2);
                Caption1("Segment Size  Copyright notice");
                cprintf("\r\n");
                xbool = 1;
            }
            pause2();
            if (endit) return;
            cprintf("%04X    %3dK  ", xword1, (512 * peekb(xword1, 2)) / 1024);
            if (BIOSscan(xword1, 0x0000, 0x1FFF, &xword2))
                showBIOS(xword1, xword2);
            else
                dontknow();
        }
        xword1 += 0x0080;
    }
    if (!xbool) cprintf("(none)\r\n");
}
