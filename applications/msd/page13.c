/* PAGE13.C – Disk information (from page_13.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

/* Константа тиков в секунду (18.2) */
#define TICK1  18.2

/* Тип структуры IOCTL (iotbltype) */
typedef struct {
    unsigned char  spclfunc;    /* специальные функции */
    unsigned char  devtype;     /* тип устройства */
    unsigned int   devattr;     /* атрибуты устройства */
    unsigned int   cylcount;    /* количество цилиндров */
    unsigned char  medtype;     /* тип носителя */
    unsigned int   bpsec;       /* байт на сектор */
    unsigned char  secpclus;    /* секторов на кластер */
    unsigned int   resvsec;     /* зарезервированных секторов */
    unsigned char  fats;        /* количество FAT */
    unsigned int   rootentries; /* записей корневого каталога */
    unsigned int   numsecs;     /* количество секторов (16 бит) */
    unsigned char  meddescr;    /* дескриптор носителя */
    unsigned int   secpfat;     /* секторов на FAT */
    unsigned int   secptrk;     /* секторов на дорожку */
    unsigned int   numheads;    /* головок */
    unsigned long  numhidden;   /* скрытых секторов */
    unsigned long  largesec;    /* количество секторов (32 бита) */
    unsigned char  reserved[6]; /* зарезервировано ($19..$1E) */
} iotbltype;

/* Внешние функции, которые должны быть в COMMON.C */
extern void DrvName(unsigned char drive);
extern void media(unsigned char media_byte, unsigned char al);
extern long disksize(unsigned char drive);
extern long diskfree(unsigned char drive);

/* ========================================================================== */
void page13(void)
{
    unsigned char i;
    unsigned char xbyte;
    char xchar;
    unsigned char xFCB[0x2C];
    long xlong;
    char xstring[80];
    unsigned int xword1, xword2, xword3, xword4, xword5;
    iotbltype iotable;
    unsigned char saveX, saveY;

    /* LASTDRIVE */
    Caption2("LASTDRIVE");
    if (OSMajor >= 20)   /* OS/2 или подобное */
        DrvName(lastdrv);
    else
        DrvName(lastdrv - 1);
    cprintf("\r\n");

    /* Логические диски */
    Caption2("Logical drives");
    {
        union REGS r;
        xbyte = 0;
        for (xchar = 'A'; xchar <= 'Z'; xchar++)
        {
            r.h.ah = 0x0E;
            r.h.dl = xchar - 'A';
            int86(0x21, &r, &r);
            r.h.ah = 0x19;
            int86(0x21, &r, &r);
            if (r.h.al == r.h.dl)
            {
                DrvName(r.h.al);
                xbyte++;
                if (xbyte == 20)
                {
                    cprintf("\r\n                ");
                    xbyte = 0;
                }
            }
        }
        cprintf("\r\n");
        r.h.ah = 0x0E;
        r.h.dl = currdrv;
        int86(0x21, &r, &r);
    }

    /* Дисководы */
    Caption2("Diskette drives");
    if (equip & 1)
        cprintf("%u\r\n", 1 + ((equip & 0xC0) >> 6));
    else
        cprintf("0\r\n");

    xword1 = (unsigned)(FP_SEG(intvec[0x1E]));
    xword2 = (unsigned)(FP_OFF(intvec[0x1E]));
    Caption3("Sectors/track");
    cprintf("%u\r\n", peekb(xword1, xword2 + 4));
    Caption3("Bytes/sector");
    cprintf("%u\r\n", peekb(xword1, xword2 + 3) << 8);
    Caption3("On time (ms)");
    cprintf("%u\r\n", 125 * peekb(xword1, xword2 + 10));
    Caption3("Off time (s)");
    cprintf("%.1f\r\n", ((long)peekb(xword1, xword2 + 2) << 16) / TICK1);
    Caption3("Head settle time (ms)");
    cprintf("%u\r\n", peekb(xword1, xword2 + 9));

    if ((1 + ((equip & 0xC0) >> 6)) == 1)
    {
        Caption1("  Single drive is now ");
        xbyte = peekb(0x40, 0x104);  /* BIOSdseg */
        if (xbyte <= 'Z' - 'A')
        {
            DrvName(xbyte);
            cprintf("\r\n");
        }
        else if (xbyte == 0xFF)
            cprintf("N/A\r\n");
        else
            unknown("status", xbyte, 2);
    }

    /* Текущий каталог */
    Caption2("Current drive and path");
    {
        union REGS r;
        r.h.ah = 0x47;
        r.h.dl = 0;   /* текущий диск */
        r.x.si = FP_OFF(xstring);
        r.x.ds = FP_SEG(xstring);
        int86x(0x21, &r, &r);
        xstring[0] = currdrv + 'A';
        xstring[1] = ':';
        xstring[2] = '\0'; /* будет переписан, на самом деле надо обработать */
        /* Стандартная обработка: функция 47h возвращает путь без диска,
           начинающийся с '\0'. Скопируем в конец. */
        {
            char tmp[64];
            tmp[0] = currdrv + 'A';
            tmp[1] = ':';
            strcpy(tmp + 2, xstring);
            strcpy(xstring, tmp);
        }
        cprintf("%s\r\n", xstring);
    }

    /* Информация о диске */
    {
        union REGS r;
        struct SREGS s;

        r.h.ah = 0x52;
        int86x(0x21, &r, &r, &s);
        if (OSMajor == 3 && OSMinor == 0)
        {
            xword1 = peekw(s.es, r.x.bx + 0x19);
            xword2 = peekw(s.es, r.x.bx + 0x17);
        }
        else
        {
            xword1 = peekw(s.es, r.x.bx + 0x18);
            xword2 = peekw(s.es, r.x.bx + 0x16);
        }

        if (OSMajor >= 4 && OSMajor < 10)
            xword5 = 0x58;
        else
            xword5 = 0x51;

        if (!(xword1 == 0xFFFF && xword2 == 0xFFFF))
        {
            xword3 = xword2 + xword5 * currdrv;
            Caption3("Drive type is");
            switch ((peekw(xword1, xword3 + 0x43) >> 14) & 0x03)
            {
                case 0: cprintf("invalid\r\n"); break;
                case 1: cprintf("physical\r\n"); break;
                case 2: cprintf("network\r\n"); break;
                case 3: cprintf("Installable File System\r\n"); break;
                default: cprintf("???\r\n");
            }

            if (OSMajor >= 4 || (OSMajor == 3 && OSMinor >= 20))
            {
                r.h.ah = 0x44;
                r.h.al = 0x0D;
                r.h.bl = 0;
                r.h.ch = 8;
                r.h.cl = 0x60;
                s.ds = FP_SEG(&iotable);
                r.x.dx = FP_OFF(&iotable);
                int86x(0x21, &r, &r, &s);
                if ((r.x.cflag & 1) == 0)
                {
                    Caption3("removable");
                    cprintf("%s", (iotable.devattr & 1) ? "no" : "yes");
                    Caption3("door lock");
                    YesOrNo(iotable.devattr & 2);
                }
                else
                {
                    r.x.ax = 0x4408;
                    r.h.bl = 0;
                    int86(0x21, &r, &r);
                    if ((r.x.cflag & 1) == 0)
                    {
                        Caption3("removable");
                        YesOrNo(r.h.al == 0);
                    }
                }
            }

            Caption3("JOIN'd ");
            if (peekw(xword1, xword3 + 0x43) & 0x2000)
            {
                cprintf("yes");
                Caption3("actually");
                xword4 = xword3;
                while (peekb(xword1, xword4) != 0)
                {
                    putchar(peekb(xword1, xword4));
                    xword4++;
                }
                cprintf("\r\n");
            }
            else
                cprintf("no\r\n");

            Caption3("SUBST'd");
            if (peekw(xword1, xword3 + 0x43) & 0x1000)
            {
                cprintf("yes");
                Caption3("actually");
                xword4 = xword3;
                while (peekb(xword1, xword4) != 0)
                {
                    putchar(peekb(xword1, xword4));
                    xword4++;
                }
                cprintf("\r\n");
            }
            else
                cprintf("no\r\n");
        }
    }

    /* Метка тома */
    Caption3("Volume label");
    memset(xFCB, 0, 0x2C);
    xFCB[0x00] = 0xFF;      /* расширенный FCB */
    xFCB[0x06] = 0x08;      /* атрибут метки тома */
    for (i = 0x08; i <= 0x12; i++)
        xFCB[i] = '?';

    {
        union REGS r;
        struct SREGS s;

        r.h.ah = 0x11;
        s.ds = FP_SEG(xFCB);
        r.x.dx = FP_OFF(xFCB);
        int86x(0x21, &r, &r, &s);
        switch (r.h.al)
        {
            case 0x00:
                r.h.ah = 0x2F;
                int86x(0x21, &r, &r, &s);
                i = 0x08;
                xchar = peekb(s.es, r.x.bx + i);
                while (i <= 0x12 && xchar > 0)
                {
                    putchar(showchar(xchar));
                    i++;
                    xchar = peekb(s.es, r.x.bx + i);
                }
                cprintf("\r\n");
                break;
            case 0xFF:
                cprintf("(none)\r\n");
                break;
            default:
                unknown("status", r.h.al, 2);
        }
    }

    /* Информация о носителе */
    {
        union REGS r;
        saveX = WhereX();
        saveY = WhereY();
        TextColor(LIGHTRED + 128);  /* LightRed + Blink */
        cprintf("  *retrieving information*");
        r.h.ah = 0x1B;
        int86(0x21, &r, &r);
        GotoXY(saveX, saveY);
        cprintf("                          ");
        GotoXY(saveX, saveY);
        media(peekb(r.x.ds, r.x.bx), r.h.al);
        Caption3("Clusters");
        cprintf("%u\r\n", r.x.dx);
        Caption3("Sectors/cluster");
        cprintf("%u\r\n", r.h.al);
        Caption3("Bytes/sector");
        cprintf("%u\r\n", r.x.cx);
    }

    /* Общий размер */
    Caption3("Total space (bytes)");
    xlong = disksize(0);
    if (xlong != -1)
        cprintf("%9ld (%.0fK)\r\n", xlong, xlong / 1024.0);
    else
        cprintf("(invalid drive)\r\n");

    /* Свободное место */
    Caption3("Free space (bytes) ");
    xlong = diskfree(0);
    if (xlong != -1)
        cprintf("%9ld (%.0fK)\r\n", xlong, xlong / 1024.0);
    else
        cprintf("(invalid drive)\r\n");
}
