/* INIT.C – инициализация (исправлено) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <graph.h>
#include "msd.h"

static void rjustify(const char *a);
static void border(unsigned char ch);
static void checkparams(int argc, char *argv[]);
static char *deletejunk(char *s);

void init(int argc, char *argv[])
{
    int xint;
    union REGS r;
    struct SREGS s;

    /* Установка начальных значений */
    mono = 0;
    vidmode = lastmode;
    attrsave = GetTextAttr();          /* <-- исправлено */
    TextAttr = attrsave;
    CheckBreak = 0;
    resetvideo = 0;
    novgacheck = 0;
    ReadPartitionTable = 1;
    FifoOn = 0;

    /* Если режим 0 или 1, переключаем на более цветной */
    if ((lastmode & 0xFF) == 0 || (lastmode & 0xFF) == 1) {
        textmode(lastmode + 2);
        resetvideo = 1;
    }

    /* Определяем ширину экрана */
    r.h.ah = 0x0F; int86(0x10, &r, &r);
    twidth = r.h.ah; vidpg = r.h.bh;

    /* Графический драйвер – упрощённо VGA */
    graphdriver = VGA;
    if (graphdriver == EGA || graphdriver == MCGA || graphdriver == VGA) {
        r.x.ax = 0x1130; r.h.bh = 0; int86(0x10, &r, &r);
        tlength = r.h.dl + 1;
    } else {
        tlength = 25;
    }

    /* Получение оборудования, памяти, текущего диска, адресов DOS */
    int86(0x11, &r, &r); equip = r.x.ax;
    int86(0x12, &r, &r); DOSmem = (unsigned long)r.x.ax << 10;
    r.h.ah = 0x19; int86(0x21, &r, &r); currdrv = r.h.al;
    r.h.ah = 0x34; int86x(0x21, &r, &r, &s); DOScseg = s.es; DOScofs = r.x.bx;

    for (i = 0; i < 256; i++) intvec[i] = _dos_getvect(i);

    r.x.ax = 0x3700; int86(0x21, &r, &r); switchar = (unsigned char)r.h.dl;
    dirsep[0] = '\\'; dirsep[1] = '\0';
    if (switchar != '/') strcat(dirsep, "/");

    r.h.ah = 0x52; int86x(0x21, &r, &r, &s); devseg = s.es; devofs = r.x.bx;
    lastdrv = peekb(devseg, devofs + 0x21);

    if ((lastmode & 0xFF) == 2 || (lastmode & 0xFF) == 7) mono = 1;

    r.x.ax = 0x2B01; r.x.cx = 0x4445; r.x.dx = 0x5351;
    int86(0x21, &r, &r);
    if (r.h.al != 0xFF) DirectVideo = 0;

    if (argc > 1) checkparams(argc, argv);

    textcolor(LIGHTGREEN); textbackground(BLUE); clrscr();
    cprintf("MSD+");
    textcolor(LIGHTGRAY); cprintf(" - Information on all computer functions");
    rjustify(qversion);
    cprintf("\r\n");
    border('\xDF');
    gotoxy(1, tlength - 1);
    border('\xDC');
    cprintf("Page ");
    x1 = wherex();
    textcolor(LIGHTGREEN);
    rjustify("F1 Enter PgUp PgDn Home End Esc Alt-P");

    Pg = 0; endit = 0;

    if (osmajor >= 3) {
        r.x.ax = 0x3800; s.ds = FP_SEG(Country); r.x.dx = FP_OFF(Country);
        int86x(0x21, &r, &r, &s);
        ccode = r.x.bx; decimal = Country[9];
    }
}

static void rjustify(const char *a) {
    int x = 1 + (WindMax & 0xFF) - strlen(a);
    gotoxy(x, wherey()); x2 = wherex(); cprintf("%s", a);
}

static void border(unsigned char ch) {
    int i;
    textcolor(LIGHTCYAN);
    for (i = 1; i <= twidth; i++) putchar(ch);
    textcolor(LIGHTGRAY);
}

static char *deletejunk(char *s) {
    static char buf[256];
    char *p = buf;
    while (*s == ' ' || *s == '-' || *s == '/') s++;
    while (*s) *p++ = *s++;
    *p = '\0';
    p = buf + strlen(buf) - 1;
    while (p >= buf && *p == ' ') *p-- = '\0';
    strupr(buf);
    return buf;
}

static void checkparams(int argc, char *argv[]) {
    int parm;
    for (parm = 1; parm < argc; parm++) {
        char *s = deletejunk(argv[parm]);
        if (!strcmp(s,"B")) DirectVideo = 0;
        else if (!strcmp(s,"D")) DirectVideo = 1;
        else if (!strcmp(s,"M")) mono = 1;
        else if (!strcmp(s,"C")) mono = 0;
        else if (!strcmp(s,"F")) FifoOn = 1;
        else if (!strcmp(s,"NP")) ReadPartitionTable = 0;
        else if (!strcmp(s,"NV")) novgacheck = 1;
        else if (!strncmp(s,"AP",2)) {
            PrinterRec.Mode = 'A';
            if (s[2] == ':') {
                PrinterRec.Destination = 'F';
                strcpy(PrinterRec.Filename, deletejunk(argv[parm]+3));
                PrinterRec.HiStrip = 1;
                PrinterRec.HeaderStr[0] = '\0';
                PrinterRec.ScreensPerPage = (tlength == 25) ? 2 : 1;
                PrinterRec.ScreenCount = 0;
            }
        }
        else if (!strcmp(s,"?") || !strcmp(s,"H")) {
            freopen("CON","w",stdout); freopen("CON","r",stdin);
            printf("MSD %s  Compiled: %s\n\n", qversion, qdate);
            printf("Syntax:\n  MSD [B][D][M][C][F][NP][NV][AP[:filename]][H][?]\n");
            printf("  B  = Write to screen using BIOS (default under Desqview)\n");
            printf("  D  = Write directly to screen memory (default)\n");
            printf("  M  = Use monochrome colors\n");
            printf("  C  = Use normal colors\n");
            printf("  F  = Leave 16550 FIFO's enabled\n");
            printf("  NV = Do not perform VGA chipset detection\n");
            printf("  NP = Do not read Partition Table\n");
            printf("  AP = AutoPrint all screens, ask for setup\n");
            printf("  AP:filename = AutoPrint to a file or device\n");
            printf("  H or ? = This help screen\n");
            exit(0);
        }
    }
}
