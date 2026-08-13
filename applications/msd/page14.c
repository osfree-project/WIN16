/* PAGE14.C – BIOS disk parameters (from page_14.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

/* Константа количества тиков в секунду (18.2 Гц) */
#ifndef TICK1
#define TICK1  18.2
#endif

/* ========================================================================== */
void page14(void)
{
    unsigned int drive;
    int NoDisks = 1;          /* true – диски не найдены */
    unsigned char xbyte1 = 0; /* будет инициализирована при первом диске */
    unsigned char xbyte2;

    Caption2("BIOS disk parameters");

    for (drive = 0x00; drive <= 0xFF; drive++)
    {
        union REGS r;
        struct SREGS s;
        r.h.ah = 0x08;
        r.h.dl = drive;
        int86x(0x13, &r, &r, &s);

        if (nocarry(&r) && (r.h.bl > 0x00 || drive >= 0x80) && r.h.dl > 0)
        {
            if (NoDisks)
            {
                NoDisks = 0;
                cprintf("\r\n");
                Caption3("Unit");
                cprintf("\r\n");
                Caption3("Type");
                cprintf("\r\n");
                Caption3("Drives");
                cprintf("\r\n");
                Caption3("Heads");
                cprintf("\r\n");
                Caption3("Cylinders");
                cprintf("\r\n");
                Caption3("Sectors/track");
                cprintf("\r\n");
                Caption3("Specify bytes");
                cprintf("\r\n");
                Caption3("Off time (s)");
                cprintf("\r\n");
                Caption3("Bytes/sector");
                cprintf("\r\n");
                Caption3("Sectors/track");
                cprintf("\r\n");
                Caption3("Gap length");
                cprintf("\r\n");
                Caption3("Data length");
                cprintf("\r\n");
                Caption3("Gap length for format");
                cprintf("\r\n");
                Caption3("Fill byte for format");
                cprintf("\r\n");
                Caption3("Head settle time (ms)");
                cprintf("\r\n");
                Caption3("On time (ms)");
                cprintf("\r\n");
                xbyte1 = 27;   /* начальная позиция колонки */
            }

            /* Если место в строке кончилось, переходим вправо и очищаем */
            if (xbyte1 + 10 > twidth)
            {
                pause1();
                if (endit) return;
                xbyte1 = 27;
                Window(xbyte1, 4, twidth, tlength - 2);
                ClrScr();
            }

            Window(xbyte1, 4, xbyte1 + 11, tlength - 2);

            /* Вывод номера диска */
            cprintf("%u\r\n", drive);

            /* Тип диска */
            if (drive < 0x80)
            {
                switch (r.h.bl)
                {
                    case 0x01: cprintf("360KB 5.25\"\r\n"); break;
                    case 0x02: cprintf("1.2MB 5.25\"\r\n"); break;
                    case 0x03: cprintf("720KB 3.5\"\r\n"); break;
                    case 0x04: cprintf("1.44MB 3.5\"\r\n"); break;
                    case 0x05: cprintf("2.88MB 3.5\"\r\n"); break;
                    default:   cprintf("(%02X)\r\n", r.h.bl);
                }
            }
            else
                cprintf("fixed disk\r\n");

            /* Количество устройств (DL) */
            cprintf("%u\r\n", r.h.dl);
            /* Головки */
            cprintf("%u\r\n", r.h.dh + 1);
            /* Цилиндры */
            cprintf("%u\r\n", ((r.h.cl >> 6) << 8 | r.h.ch) + 1);   /* cbw(ch, cl shr 6) + 1 */
            /* Секторов на дорожку */
            cprintf("%u\r\n", r.h.cl & 0x3F);

            if (drive < 0x80)
            {
                /* Параметры дискеты из таблицы ES:DI */
                cprintf("$%02X $%02X\r\n", peekb(s.es, r.x.di), peekb(s.es, r.x.di + 1));
                cprintf("%.1f\r\n", ((long)peekb(s.es, r.x.di + 2) << 16) / TICK1);

                xbyte2 = peekb(s.es, r.x.di + 3);
                switch (xbyte2)
                {
                    case 0x00: cprintf("128\r\n"); break;
                    case 0x01: cprintf("256\r\n"); break;
                    case 0x02: cprintf("512\r\n"); break;
                    case 0x03: cprintf("1024\r\n"); break;
                    default:   cprintf("(%04X)\r\n", xbyte2);
                }
                cprintf("%u\r\n", peekb(s.es, r.x.di + 4));   /* секторов/дор. */
                cprintf("%u\r\n", peekb(s.es, r.x.di + 5));   /* gap length */
                cprintf("%u\r\n", peekb(s.es, r.x.di + 6));   /* data length */
                cprintf("$%02X\r\n", peekb(s.es, r.x.di + 7)); /* fill byte для форматирования */
                cprintf("%u\r\n", peekb(s.es, r.x.di + 8));   /* gap length для форматирования */
                cprintf("%u\r\n", peekb(s.es, r.x.di + 9));   /* head settle time (ms) */
                cprintf("%u\r\n", 125 * peekb(s.es, r.x.di + 0x0A)); /* on time (ms) */
            }

            xbyte1 += 13;
        }
    }

    if (NoDisks)
        cprintf("(no disks)\r\n");
}
