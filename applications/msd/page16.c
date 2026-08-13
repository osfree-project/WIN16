/* PAGE16.C Ц Boot record and DOS Disk Parameter Block (from page_16.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include "msd.h"

#define SECSIZ 512          /* размер сектора */

/* ѕрототипы внешних функций (должны быть в COMMON.C / SYSTEM.C) */
extern unsigned diskread(unsigned drive, unsigned cylinder, unsigned sector, unsigned char far *buffer);
extern void media(unsigned char media_byte, unsigned char sectors_per_cluster);
extern void pause4(int direction, char *ch2);
extern void DrvName(unsigned char drive);

/* Ќаправлени€ дл€ pause4 (определены в проекте) */
#define UP      1
#define DOWN    2
#define UPDOWN  3

/* ========================================================================== */
void page16(void)
{
    unsigned char bootrec[SECSIZ];
    unsigned int i, j;
    unsigned long l;
    int xbool;
    unsigned char xbyte;
    char xchar;
    unsigned int xword1, xword2, xword3, xword4, xword5;
    unsigned int bpbsize;
    char ch2[2];
    int direction;

    Window(1, 3, twidth / 2, tlength - 2);

    Caption1("Boot record of ");
    DrvName(currdrv);
    cprintf("\r\n");

    xword1 = diskread(currdrv, 0, 1, bootrec);
    if (xword1 == 0x0000)
    {
        /* ћедиа-дескриптор и байт секторов на кластер */
        media(bootrec[0x15], bootrec[0x0D]);

        Caption3("Sectors/cluster");
        cprintf("%u\r\n", bootrec[0x0D]);

        Caption3("Bytes/sector");
        cprintf("%u\r\n", ((unsigned)bootrec[0x0C] << 8) | bootrec[0x0B]);

        Caption3("Reserved sectors");
        cprintf("%u\r\n", ((unsigned)bootrec[0x0F] << 8) | bootrec[0x0E]);

        Caption3("FAT's");
        cprintf("%u\r\n", bootrec[0x10]);

        Caption3("Sectors/FAT");
        cprintf("%u\r\n", ((unsigned)bootrec[0x17] << 8) | bootrec[0x16]);

        Caption3("Root directory entries");
        cprintf("%u\r\n", ((unsigned)bootrec[0x12] << 8) | bootrec[0x11]);

        cprintf("\r\n");

        Caption3("Heads");
        cprintf("%u\r\n", ((unsigned)bootrec[0x1B] << 8) | bootrec[0x1A]);

        Caption3("Total sectors");
        xword1 = ((unsigned)bootrec[0x14] << 8) | bootrec[0x13];
        if (xword1 == 0)
        {
            l = bootrec[0x20] | (bootrec[0x21] << 8) |
                ((unsigned long)bootrec[0x22] << 16) | ((unsigned long)bootrec[0x23] << 24);
            cprintf("%lu\r\n", l);
        }
        else
            cprintf("%u\r\n", xword1);

        Caption3("Sectors/track");
        cprintf("%u\r\n", ((unsigned)bootrec[0x19] << 8) | bootrec[0x18]);

        Caption3("Hidden sectors");
        if (xword1 == 0)
        {
            l = bootrec[0x1C] | (bootrec[0x1D] << 8) |
                ((unsigned long)bootrec[0x1E] << 16) | ((unsigned long)bootrec[0x1F] << 24);
            cprintf("%lu\r\n", l);
        }
        else
            cprintf("%u\r\n", ((unsigned)bootrec[0x1D] << 8) | bootrec[0x1C]);

        Caption3("OEM name and version");
        for (i = 0x03; i <= 0x0A; i++)
            putchar(showchar(bootrec[i]));
        cprintf("\r\n");

        Caption3("Extended boot record");
        if ((osmajor >= 4) && (bootrec[0x26] == 0x29))
        {
            cprintf("yes\r\n");
            Caption3("Physical drive number");
            cprintf("%u\r\n", bootrec[0x24]);

            Caption3("Volume label");
            for (j = 0x2B; j <= 0x35; j++)
                putchar(showchar(bootrec[j]));
            cprintf("\r\n");

            Caption3("Serial Number");
            cprintf("%04X-%04X\r\n",
                    ((unsigned)bootrec[0x2A] << 8) | bootrec[0x29],
                    ((unsigned)bootrec[0x28] << 8) | bootrec[0x27]);

            Caption3("FAT type");
            for (j = 0x36; j <= 0x3D; j++)
                putchar(showchar(bootrec[j]));
            cprintf("\r\n");
        }
        else
            cprintf("no\r\n");
    }
    else
    {
        cprintf("  Can't read boot record\r\n");
        cprintf("  ");

        xbyte = xword1 >> 8;
        switch (xbyte)
        {
            case 0x80: cprintf("Attachment failed to respond\r\n"); break;
            case 0x40: cprintf("Seek operation failed\r\n"); break;
            case 0x20: cprintf("Controller failed\r\n"); break;
            case 0x10: cprintf("Data error (bad CRC)\r\n"); break;
            case 0x08: cprintf("DMA failure\r\n"); break;
            case 0x04: cprintf("Sector not found\r\n"); break;
            case 0x03: cprintf("Write-protect fault\r\n"); break;
            case 0x02: cprintf("Bad address mark\r\n"); break;
            case 0x01: cprintf("Bad command\r\n"); break;
            case 0x00: cprintf("\r\n"); break;
            default:   unknown("error", xbyte, 2); break;
        }

        cprintf("  ");
        xbyte = xword1 & 0xFF;
        switch (xbyte)
        {
            case 0x00: cprintf("Write-protect error\r\n"); break;
            case 0x01: cprintf("Unknown unit\r\n"); break;
            case 0x02: cprintf("Drive not ready\r\n"); break;
            case 0x03: cprintf("Unknown command\r\n"); break;
            case 0x04: cprintf("Data error (bad CRC)\r\n"); break;
            case 0x05: cprintf("Bad request structure length\r\n"); break;
            case 0x06: cprintf("Seek error\r\n"); break;
            case 0x07: cprintf("Unknown media type\r\n"); break;
            case 0x08: cprintf("Sector not found\r\n"); break;
            case 0x09: cprintf("Printer out of paper\r\n"); break;
            case 0x0A: cprintf("Write fault\r\n"); break;
            case 0x0B: cprintf("Read fault\r\n"); break;
            case 0x0C: cprintf("General failure\r\n"); break;
            default:   unknown("error", xbyte, 2); break;
        }

        if (osmajor >= 10)
        {
            unsigned char saveattr = TextAttr;
            cprintf("\r\n");
            TextColor(LIGHTRED);
            cprintf("**NOTICE**\r\n");
            TextAttr = saveattr;
            cprintf("Information for SUBST'd drives\r\n");
            cprintf("is not available under OS/2.\r\n");
            cprintf("If you recieved an Unknown unit\r\n");
            cprintf("error, change to a real drive\r\n");
            cprintf("and try INFOPLUS again.\r\n");
        }
    }

    Window(1 + twidth / 2, 3, twidth, tlength - 2);

    i = 1;
    xbool = 0;   /* никогда не станет истинным, это бесконечный цикл с ручным выходом */

    xword1 = peekw(devseg, devofs + 0x0018);
    xword2 = peekw(devseg, devofs + 0x0016);

    if (!(xword1 == 0xFFFF && xword2 == 0xFFFF))
    {
        if (osmajor >= 4 && osmajor < 10)
        {
            xbyte = 1;
            bpbsize = 0x58;
        }
        else
        {
            xbyte = 0;
            bpbsize = 0x51;
        }

        do
        {
            Caption1("DOS disk parameter block for ");
            xword2 = peekw(devseg, devofs + 0x0016) + ((i - 1) * bpbsize);
            DrvName(i - 1);
            cprintf("\r\n");

            /* ”казатель на media descriptor (CDS?) */
            xword3 = peekw(xword1, xword2 + 0x0047);
            xword4 = peekw(xword1, xword2 + 0x0045);

            /* Ѕайт media и секторов на кластер */
            media(peekb(xword3, xword4 + 0x0016 + xbyte),
                  peekb(xword3, xword4 + 0x0004) + 1);

            Caption3("Sectors/cluster");
            cprintf("%u\r\n", peekb(xword3, xword4 + 0x0004) + 1);

            Caption3("Bytes/sector");
            cprintf("%u\r\n", peekw(xword3, xword4 + 0x0002));

            Caption3("Reserved sectors");
            cprintf("%u\r\n", peekw(xword3, xword4 + 0x0006));

            Caption3("FAT's");
            cprintf("%u\r\n", peekb(xword3, xword4 + 0x0008));

            Caption3("Sectors/FAT");
            if (osmajor >= 4 && osmajor < 10)
                cprintf("%u\r\n", peekw(xword3, xword4 + 0x000F));
            else
                cprintf("%u\r\n", peekw(xword3, xword4 + 0x000F) & 0xFF); /* т.е. младший байт? оригинал: Mem[xword3:xword4 + $000F] Ц один байт? Ќо в оригинале Writeln(Mem[xword3:xword4 + $000F]); ”точним: дл€ старых DOS это байт, дл€ новых Ц слово. ¬ коде используем peekw, но выводим как слово, если нужно; или учтЄм что во второй ветке выводитс€ целое слово. —делаем по оригиналу: в старой ветке Ц байт, в новой Ц слово. */

            Caption3("Root directory entries");
            cprintf("%u\r\n", peekw(xword3, xword4 + 0x0009));

            cprintf("\r\n");

            Caption3("DPB valid");
            YesOrNo(peekb(xword3, xword4 + 0x0017 + xbyte) < 0xFF);

            Caption3("Current directory");
            j = xword2;
            xchar = peekb(xword1, j);
            while (xchar > 0)
            {
                putchar(xchar);
                j++;
                xchar = peekb(xword1, j);
            }
            cprintf("\r\n");

            Caption3("Device header");
            SegOfs(peekw(xword3, xword4 + 0x0014 + xbyte),
                   peekw(xword3, xword4 + 0x0012 + xbyte));
            cprintf("\r\n");

            Caption3("Unit within driver");
            cprintf("%u\r\n", peekb(xword3, xword4 + 0x0001));

            Caption3("Clusters");
            cprintf("%u\r\n", peekw(xword3, xword4 + 0x000D) - 1);

            Caption3("Cluster to sector shift");
            cprintf("%u\r\n", peekb(xword3, xword4 + 0x0005));

            Caption3("Root directory sector");
            cprintf("%u\r\n", peekw(xword3, xword4 + 0x0010 + xbyte));

            Caption3("First data sector");
            cprintf("%u\r\n", peekw(xword3, xword4 + 0x000B));

            Caption3("Next DPB");
            xword5 = peekw(xword3, xword4 + 0x0018 + xbyte);
            SegOfs(peekw(xword3, xword4 + 0x001A + xbyte), xword5);
            cprintf("\r\n");

            if (i == 1)
                direction = DOWN;
            else if ((i == lastdrv) || (xword5 == 0xFFFF))
                direction = UP;
            else
                direction = UPDOWN;

            cprintf("  ");
            pause4(direction, ch2);
            if (endit) return;

            if (ch2[0] == 0 && ch2[1] == 72 && i > 1)   /* стрелка вверх */
                i--;
            else if (ch2[0] == 0 && ch2[1] == 80 && i < lastdrv && xword5 < 0xFFFF) /* стрелка вниз */
                i++;
            ClrScr();
        } while (!xbool);
    }
}
