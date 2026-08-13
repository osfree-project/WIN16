/* PAGE17.C Ц CMOS Information (translated from page_17.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

/* Ћокальные константы */
static const char *DayName[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
};
static const char *MonthName[] = {
    "???", "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};
static const char *ScreenName[] = {
    "EGA/VGA", "CGA 40col", "CGA 80col", "Monochrome"
};
static const char *FloppyName[] = {
    "none", "5.25\" 360K", "5.25\" 1.2M", "3.5\" 720K", "3.5\" 1.44M", "3.5\" 2.88M"
};

/* Ћокальные static-функции */
static unsigned char ReadCMOS(unsigned cmosport, unsigned char adr)
{
    unsigned char result;
    _asm {
        cli
        mov dx, cmosport
        mov al, adr
        out dx, al
        mov cx, 10
    delay_loop:
        in al, dx
        loop delay_loop
        inc dx
        in al, dx
        mov result, al
        sti
    }
    return result;
}

static void GetHDValues(unsigned char hdtype, unsigned *cyl, unsigned *heads,
                         unsigned *precomp, unsigned *lz, unsigned *sectors)
{
    unsigned offset = 0xE401 + ((hdtype - 1) * 16);
    *cyl = peekw(0xF000, offset);
    *heads = peekb(0xF000, offset + 2);
    *precomp = peekw(0xF000, offset + 5);
    *lz = peekw(0xF000, offset + 0x0C);
    *sectors = peekb(0xF000, offset + 0x0E);
}

static void ShowHDValues(unsigned cyl, unsigned heads, unsigned precomp,
                         unsigned lz, unsigned sectors)
{
    Caption3("Cyl");
    cprintf("%4u", cyl);
    Caption3("Hds");
    cprintf("%2u", heads);
    Caption3("Sec");
    cprintf("%2u", sectors);
    Caption3("LZ");
    cprintf("%4d", (int)lz);
    Caption3("PreComp");
    cprintf("%4d\r\n", (int)precomp);
}

/* ======================================================================== */
void page17(void)
{
    unsigned cmosport = 0x70;
    int ps2 = 0, phoenix = 0;
    unsigned char floppy, hd0, hd1;
    unsigned char xbyte;
    char s[10];
    unsigned cyl, heads, precomp, lz, sectors;

    Caption2("CMOS");
    {
        union REGS r;
        r.h.ah = 0xC0;
        int86(0x15, &r, &r);
        if (nocarry(&r) || peekb(0xFFFF, 0x000E) < 0xFD)
        {
            /* PS2 detection */
            if (peekw(r.x.es, r.x.bx + 2) == 0xF8)
                ps2 = 1;
            /* Phoenix detection */
            s[0] = peekb(r.x.es, r.x.bx + 0x0D);
            s[1] = peekb(r.x.es, r.x.bx + 0x0E);
            s[2] = peekb(r.x.es, r.x.bx + 0x0F);
            s[3] = '\0';
            if (strcmp(s, "PTL") == 0)
                phoenix = 1;

            cmosport = 0x70;
            cprintf("\r\n");

            /* Power status */
            Caption3("Power status");
            if (ReadCMOS(cmosport, 0x0D) & 0x80)
                cprintf("OK\r\n");
            else
                cprintf("No power!\r\n");

            /* Diagnostics */
            Caption3(" Diagnostics");
            xbyte = ReadCMOS(cmosport, 0x0E);
            if (xbyte == 0)
                cprintf("No problems detected\r\n");
            else {
                if (xbyte & 0x80) cprintf("Clock lost power\r\n");
                if (xbyte & 0x40) cprintf("Incorrect checksum\r\n");
                if (xbyte & 0x20) cprintf("Bad equipment configuration\r\n");
                if (xbyte & 0x10) cprintf("Memory size error\r\n");
                if (xbyte & 8)    cprintf("Disk drive initialization failure\r\n");
                if (xbyte & 4)    cprintf("Invalid time\r\n");
                if (xbyte & 2)    cprintf("Bad adaptor configuration (EISA)\r\n");
                if (xbyte & 1)    cprintf("Timeout reading adaptor ID (EISA)\r\n");
            }
            cprintf("\r\n");

            /* Date */
            Caption3("Date");
            {
                unsigned char date_val = ReadCMOS(cmosport, 7);
                unsigned char month_val = ReadCMOS(cmosport, 8);
                unsigned char year_val = ReadCMOS(cmosport, 9);
                unsigned century;

                if (ps2)
                    century = UnBCD(ReadCMOS(cmosport, 0x37));
                else
                    century = UnBCD(ReadCMOS(cmosport, 0x32));

                if ((ReadCMOS(cmosport, 0x0B) & 4) == 0) {
                    date_val = UnBCD(date_val);
                    year_val = UnBCD(year_val);
                    month_val = UnBCD(month_val);
                }

                switch (Country[0]) {
                    case 0: case 3 ... 255:
                        cprintf("%s %u, %u%s\r\n",
                                MonthName[month_val], date_val,
                                century, AddZero(year_val));
                        break;
                    case 1:
                        cprintf("%u %s, %u%s\r\n",
                                date_val, MonthName[month_val],
                                century, AddZero(year_val));
                        break;
                    case 2:
                        cprintf("%u%s, %s %u\r\n",
                                century, AddZero(year_val),
                                MonthName[month_val], date_val);
                        break;
                }
            }

            /* Time */
            Caption3("Time");
            {
                unsigned char hour = ReadCMOS(cmosport, 4);
                unsigned char min = ReadCMOS(cmosport, 2);
                unsigned char sec = ReadCMOS(cmosport, 0);
                char time_sep = Country[0x0D];
                if ((ReadCMOS(cmosport, 0x0B) & 4) == 0) {
                    hour = UnBCD(hour);
                    min = UnBCD(min);
                    sec = UnBCD(sec);
                }
                if ((ReadCMOS(cmosport, 0x0B) & 2) == 0) {
                    if (hour > 12) hour = (hour - 128) + 11;
                    else hour--;
                }
                if (Country[0x11] & 1) {
                    cprintf("%u%c%02u%c%02u\r\n",
                            hour, time_sep, min, time_sep, sec);
                } else {
                    int pm = 0;
                    if (hour == 0) hour = 12;
                    else if (hour >= 13) { hour -= 12; pm = 1; }
                    else if (hour == 12) pm = 1;
                    cprintf("%u%c%02u%c%02u %s\r\n",
                            hour, time_sep, min, time_sep, sec,
                            pm ? "PM" : "AM");
                }
            }
            cprintf("\r\n");

            /* Video type */
            Caption3("Video type ");
            cprintf("%s\r\n", ScreenName[(ReadCMOS(cmosport, 0x14) >> 4) & 3]);

            /* Coprocessor */
            Caption3("Coprocessor");
            YesOrNo((ReadCMOS(cmosport, 0x14) & 2) == 2);
            cprintf("\r\n");

            /* Floppy drives */
            floppy = ReadCMOS(cmosport, 0x10);
            Caption3("Floppy disk A");
            if ((floppy >> 4) < 6)
                cprintf("%s\r\n", FloppyName[floppy >> 4]);
            else
                cprintf("Unknown value -> %02X\r\n", floppy >> 4);
            Caption3("Floppy disk B");
            if ((floppy & 0x0F) < 6)
                cprintf("%s\r\n", FloppyName[floppy & 0x0F]);
            else
                cprintf("Unknown value -> %02X\r\n", floppy & 0x0F);
            cprintf("\r\n");

            /* Hard disks */
            if (!ps2) {
                hd0 = ReadCMOS(cmosport, 0x12) >> 4;
                hd1 = ReadCMOS(cmosport, 0x12) & 0x0F;
                if (hd0 == 0xF) hd0 = ReadCMOS(cmosport, 0x19);
                if (hd1 == 0xF) hd1 = ReadCMOS(cmosport, 0x1A);
            } else {
                hd0 = ReadCMOS(cmosport, 0x11);
                hd1 = ReadCMOS(cmosport, 0x12);
            }

            Caption3("Hard disk 0");
            if (hd0 == 0) {
                cprintf("None\r\n");
            } else {
                cprintf("Type %u", hd0);
                if (hd0 < 47) {
                    GetHDValues(hd0, &cyl, &heads, &precomp, &lz, &sectors);
                    ShowHDValues(cyl, heads, precomp, lz, sectors);
                } else if (phoenix && hd0 >= 48) {
                    cyl = (ReadCMOS(cmosport, 0x21) << 8) + ReadCMOS(cmosport, 0x20);
                    heads = ReadCMOS(cmosport, 0x22);
                    precomp = (ReadCMOS(cmosport, 0x24) << 8) + ReadCMOS(cmosport, 0x23);
                    lz = (ReadCMOS(cmosport, 0x26) << 8) + ReadCMOS(cmosport, 0x25);
                    sectors = ReadCMOS(cmosport, 0x27);
                    ShowHDValues(cyl, heads, precomp, lz, sectors);
                } else {
                    cprintf("\r\n");
                }
            }

            Caption3("Hard disk 1");
            if (hd1 == 0) {
                cprintf("None\r\n");
            } else {
                cprintf("Type %u", hd1);
                if (hd1 < 47) {
                    GetHDValues(hd1, &cyl, &heads, &precomp, &lz, &sectors);
                    ShowHDValues(cyl, heads, precomp, lz, sectors);
                } else if (phoenix && hd1 >= 48) {
                    cyl = (ReadCMOS(cmosport, 0x36) << 8) + ReadCMOS(cmosport, 0x35);
                    heads = ReadCMOS(cmosport, 0x37);
                    precomp = (ReadCMOS(cmosport, 0x39) << 8) + ReadCMOS(cmosport, 0x38);
                    lz = (ReadCMOS(cmosport, 0x3B) << 8) + ReadCMOS(cmosport, 0x3A);
                    sectors = ReadCMOS(cmosport, 0x3C);
                    ShowHDValues(cyl, heads, precomp, lz, sectors);
                } else {
                    cprintf("\r\n");
                }
            }
            cprintf("\r\n");

            /* Conventional and extended RAM */
            Caption3("Conventional RAM");
            cprintf("%6uK\r\n", (ReadCMOS(cmosport, 0x16) << 8) + ReadCMOS(cmosport, 0x15));
            Caption3("    Extended RAM");
            cprintf("%6uK\r\n", (ReadCMOS(cmosport, 0x18) << 8) + ReadCMOS(cmosport, 0x17));
            cprintf("\r\n");

            /* CMOS checksum */
            Caption3("CMOS checksum");
            {
                unsigned computed = 0;
                unsigned cmos_checksum;
                unsigned count;
                if (!ps2) {
                    for (count = 0x10; count <= 0x2D; count++)
                        computed += ReadCMOS(cmosport, count);
                    cmos_checksum = (ReadCMOS(cmosport, 0x2E) << 8) + ReadCMOS(cmosport, 0x2F);
                } else {
                    for (count = 0x10; count <= 0x31; count++)
                        computed += ReadCMOS(cmosport, count);
                    cmos_checksum = (ReadCMOS(cmosport, 0x33) << 8) + ReadCMOS(cmosport, 0x32);
                }
                if (computed == cmos_checksum)
                    cprintf("OK\r\n");
                else
                    cprintf("Error!  Says %04X should be %04X\r\n",
                            cmos_checksum, computed);
            }
        }
        else
        {
            cprintf("No standard CMOS detected!!\r\n");
        }
    }
}
