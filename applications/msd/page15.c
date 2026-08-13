/* PAGE15.C Ц Partition table data (from page_15.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

#define SECSIZ  512          /* размер сектора */

extern int np;               /* флаг запрета чтени€ таблицы разделов (1 = запрещено) */

/* --------------------------------------------------------------------------
   ¬нутренн€€ реализаци€ ReadPartitionTable Ц возвращает 1, если чтение
   разрешено (нет ключа NP)
   -------------------------------------------------------------------------- */
static int ReadPartitionTable(void)
{
    return (np == 0);
}

/* ========================================================================== */
void page15(void)
{
    unsigned int drive;
    int j;
    int k;
    unsigned char Part[SECSIZ];   /* буфер дл€ первого сектора (MBR) */
    int EndOfDrives;
    int ValidDrive;
    int LastDrive;
    int AnotherDrive;
    unsigned char xbyte;
    unsigned long xlong;
    unsigned int xword;
    char s[80];

    Caption2("Partition table data");

    if (ReadPartitionTable())
    {
        drive = 0x80;               /* начинаем с первого жЄсткого диска */
        EndOfDrives = 0;
        ValidDrive = 0;

        /* »щем первый доступный жЄсткий диск */
        do
        {
            union REGS r;
            struct SREGS sr;
            r.x.ax = 0x0201;         /* читаем один сектор */
            r.x.cx = 0x0001;         /* цилиндр 0, сектор 1 */
            r.x.dx = drive;          /* головка 0, диск */
            sr.es = FP_SEG(Part);
            r.x.bx = FP_OFF(Part);
            int86x(0x13, &r, &r, &sr);
            if (nocarry(&r))
            {
                EndOfDrives = 1;
                ValidDrive = 1;
            }
            else
            {
                if (drive < 0x99)
                    drive++;
                else
                    EndOfDrives = 1;
            }
        } while (!EndOfDrives);

        if (ValidDrive)
        {
            /* «аголовки колонок */
            cprintf("\r\n");
            Caption3("Unit");              cprintf("\r\n");
            Caption3("Partition");         cprintf("\r\n");
            Caption3("Bootable");          cprintf("\r\n");
            Caption3("Starting head");     cprintf("\r\n");
            Caption3("Starting sector");   cprintf("\r\n");
            Caption3("Starting cylinder"); cprintf("\r\n");
            Caption3("System ID");         cprintf("\r\n");
            Caption3("Ending head");       cprintf("\r\n");
            Caption3("Ending sector");     cprintf("\r\n");
            Caption3("Ending cylinder");   cprintf("\r\n");
            Caption3("First partition sector"); cprintf("\r\n");
            Caption3("Sectors in partition");   cprintf("\r\n");

            LastDrive = 0;
            do
            {
                /* ќкно дл€ номера диска */
                Window(9, 4, twidth, tlength - 2);
                cprintf("%u\r\n", drive);

                /* ќкно дл€ разделов Ц очистка */
                Window(27, 5, twidth, tlength - 2);
                ClrScr();

                for (j = 0; j < 4; j++)
                {
                    Window(27 + 12 * j, 5, 38 + 12 * j, tlength - 2);

                    /* Ќомер раздела */
                    cprintf("%d\r\n", j + 1);

                    xword = 0x01BE + j * 16;   /* смещение записи раздела */

                    /* Bootable */
                    xbyte = Part[xword];
                    switch (xbyte)
                    {
                        case 0x00: cprintf("no\r\n"); break;
                        case 0x80: cprintf("yes\r\n"); break;
                        default:   cprintf("(%02X)\r\n", xbyte);
                    }

                    /* System ID */
                    xbyte = Part[xword + 4];
                    if (xbyte > 0x00)
                    {
                        /* Starting head */
                        cprintf("%u\r\n", Part[xword + 1]);
                        /* Starting sector */
                        cprintf("%u\r\n", Part[xword + 2] & 0x3F);
                        /* Starting cylinder */
                        cprintf("%u\r\n", Part[xword + 3] +
                                ((Part[xword + 2] >> 6) << 8));

                        /* System ID text */
                        s[0] = '\0';
                        switch (xbyte)
                        {
                            case 0x00: strcpy(s, "None"); break;
                            case 0x01: strcpy(s, "DOS-12"); break;
                            case 0x02: strcpy(s, "XENIX root"); break;
                            case 0x03: strcpy(s, "XENIX /usr"); break;
                            case 0x04: strcpy(s, "DOS-16"); break;
                            case 0x05: strcpy(s, "Ext DOS-16"); break;
                            case 0x06: strcpy(s, "Big DOS-16"); break;
                            case 0x07: strcpy(s, "OS/2 HPFS"); break;
                            case 0x08: strcpy(s, "AIX data"); break;
                            case 0x09: strcpy(s, "AIX boot"); break;
                            case 0x0A: strcpy(s, "OS/2 BtMngr"); break;
                            case 0x10: strcpy(s, "OPUS"); break;
                            case 0x24: strcpy(s, "NEC DOS 3.x"); break;
                            case 0x40: strcpy(s, "VENIX 286"); break;
                            case 0x44: strcpy(s, "386BSD"); break;
                            case 0x50: strcpy(s, "DskMngrR/O"); break;
                            case 0x51: strcpy(s, "Dsk Managr"); break;
                            case 0x52: strcpy(s, "CP/M"); break;
                            case 0x56: strcpy(s, "GB Vfeatre"); break;
                            case 0x61: strcpy(s, "Speedstor"); break;
                            case 0x63: strcpy(s, "SysV/386"); break;
                            case 0x64: strcpy(s, "NOVELL"); break;
                            case 0x75: strcpy(s, "PC/IX"); break;
                            case 0x80: strcpy(s, "Minix v1.3-"); break;
                            case 0x81: strcpy(s, "Minix v1.4+"); break;
                            case 0x82: strcpy(s, "Minix Swap"); break;
                            case 0x83: strcpy(s, "Linux extd"); break;
                            case 0x84: strcpy(s, "OS/2 hidden"); break;
                            case 0x93: strcpy(s, "Amoeba file"); break;
                            case 0x94: strcpy(s, "Amoeba BBT"); break;
                            case 0xB7: strcpy(s, "BSDI file"); break;
                            case 0xB8: strcpy(s, "BSDI swap"); break;
                            case 0xC1: strcpy(s, "DRDOSscr12b"); break;
                            case 0xC4: strcpy(s, "DRDOSscr16b"); break;
                            case 0xC6: strcpy(s, "DRDOSscrHug"); break;
                            case 0xC7: strcpy(s, "Cyrnix boot"); break;
                            case 0xDB: strcpy(s, "CP/M"); break;
                            case 0xE1: strcpy(s, "SpdStr-12"); break;
                            case 0xE3: strcpy(s, "SpdStr R/O"); break;
                            case 0xE4: strcpy(s, "SpdStr-16"); break;
                            case 0xF2: strcpy(s, "DOS secndry"); break;
                            case 0xFE: strcpy(s, "LANstep"); break;
                            case 0xFF: strcpy(s, "Xenix BBT"); break;
                            default: break;
                        }
                        if (s[0] == '\0')
                            cprintf("(%02X)\r\n", xbyte);
                        else
                            cprintf("%s\r\n", s);

                        /* Ending head */
                        cprintf("%u\r\n", Part[xword + 5]);
                        xbyte = Part[xword + 6];
                        /* Ending sector */
                        cprintf("%u\r\n", xbyte & 0x3F);
                        /* Ending cylinder */
                        cprintf("%u\r\n", Part[xword + 7] + ((xbyte >> 6) << 8));

                        /* First partition sector (LBA) */
                        xlong = 0;
                        for (k = 11; k >= 8; k--)
                            xlong = (xlong << 8) + Part[xword + k];
                        cprintf("%lu\r\n", xlong);

                        /* Sectors in partition (LBA) */
                        xlong = 0;
                        for (k = 15; k >= 12; k--)
                            xlong = (xlong << 8) + Part[xword + k];
                        cprintf("%lu\r\n", xlong);
                    }
                    else
                    {
                        /* ѕустой раздел Ц прочерки */
                        cprintf("-\r\n");  /* head */
                        cprintf("-\r\n");  /* sector */
                        cprintf("-\r\n");  /* cylinder */
                        cprintf("-\r\n");  /* system ID */
                        cprintf("-\r\n");  /* end head */
                        cprintf("-\r\n");  /* end sector */
                        cprintf("-\r\n");  /* end cylinder */
                        cprintf("-\r\n");  /* first sector */
                        cprintf("-\r\n");  /* sectors */
                    }
                }

                /* ѕоиск следующего диска */
                AnotherDrive = 0;
                do
                {
                    if (drive < 0x99)
                    {
                        drive++;
                        union REGS r;
                        struct SREGS sr;
                        r.x.ax = 0x0201;
                        r.x.cx = 0x0001;
                        r.x.dx = drive;
                        sr.es = FP_SEG(Part);
                        r.x.bx = FP_OFF(Part);
                        int86x(0x13, &r, &r, &sr);
                        if (nocarry(&r))
                        {
                            AnotherDrive = 1;
                            pause1();
                            if (endit) return;
                        }
                    }
                    else
                    {
                        LastDrive = 1;
                        AnotherDrive = 1;
                    }
                } while (!AnotherDrive);
            } while (!LastDrive);
        }
        else
        {
            cprintf("(no fixed disks)\r\n");
        }
    }
    else
    {
        cprintf("\r\n");
        cprintf("Reading of Partition Table blocked by NP command-line switch!!\r\n");
    }
}
