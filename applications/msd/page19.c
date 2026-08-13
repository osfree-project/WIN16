/* PAGE19.C – Alternate Multiplex Interrupt (INT 2Dh) information */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

void page19(void)
{
    union REGS r;
    int multiNumber;
    int foundOne = 0;
    unsigned int address, lastAddress;
    unsigned int intNumber;
    int foundInt;

    for (multiNumber = 0; multiNumber <= 0xFF; multiNumber++)
    {
        r.h.ah = (unsigned char)multiNumber;
        r.h.al = 0;
        int86(0x2D, &r, &r);
        if (r.h.al == 0xFF)
        {
            foundOne = 1;
            Pause3(-4);
            if (endit) return;

            Caption2("Function");
            cprintf("%02Xh", multiNumber);

            Caption3("Product");
            for (address = r.x.di + 8; address <= r.x.di + 0x0F; address++)
                putchar(showchar(peekb(r.x.dx, address)));

            Caption3("Manufacturer");
            for (address = r.x.di; address <= r.x.di + 7; address++)
                putchar(showchar(peekb(r.x.dx, address)));

            Caption3("Version");
            cprintf("%u.%02u\r\n", (r.x.cx >> 8), (r.x.cx & 0xFF));

            Caption3("Entry point");
            r.h.ah = (unsigned char)multiNumber;
            r.h.al = 1;
            int86(0x2D, &r, &r);
            if (r.h.al == 0)
                cprintf("Use INT 2Dh\r\n");
            else
            {
                SegOfs(r.x.dx, r.x.bx);
                cprintf("\r\n");
            }

            Caption3("Hooked interrupts (hex)");
            r.h.ah = (unsigned char)multiNumber;
            r.h.al = 4;
            int86(0x2D, &r, &r);
            if (r.h.al == 0)
                cprintf("(function call not implemented)\r\n");
            else if (r.h.al == 4)
            {
                /* Список оканчивается байтом 2Dh */
                address = r.x.bx;
                foundInt = 0;
                while (peekb(r.x.dx, address) != 0x2D)
                {
                    foundInt = 1;
                    cprintf("%02X ", peekb(r.x.dx, address));
                    address += 3;   /* каждый элемент занимает 3 байта? В оригинале inc(Address, 3) */
                }
                if (!foundInt)
                    cprintf("(none)");
                cprintf("\r\n");
            }
            else
            {
                /* Перебор всех возможных векторов прерываний */
                foundInt = 0;
                for (intNumber = 0; intNumber < 256; intNumber++)
                {
                    r.h.ah = (unsigned char)multiNumber;
                    r.h.al = 4;
                    r.h.bl = (unsigned char)intNumber;   /* предполагаем, что нужно передать номер в BL */
                    int86(0x2D, &r, &r);
                    if (r.h.al == 2 || r.h.al == 3)
                    {
                        foundInt = 1;
                        cprintf("%02X ", intNumber);
                    }
                }
                if (!foundInt)
                    cprintf("(none)");
                cprintf("\r\n");
            }

            Caption3("Description");
            r.h.ah = (unsigned char)multiNumber;
            r.h.al = 0;
            int86(0x2D, &r, &r);
            if (peekb(r.x.dx, r.x.di + 0x10) == 0)
                cprintf("(none)\r\n");
            else
            {
                address = r.x.di + 0x10;
                lastAddress = address + 64;
                while (peekb(r.x.dx, address) != 0 && address <= lastAddress)
                {
                    putchar(showchar(peekb(r.x.dx, address)));
                    address++;
                }
                cprintf("\r\n");
            }
        }
    }

    if (!foundOne)
        Caption1("No Alternate Multiplex Programs found!\r\n");
}
