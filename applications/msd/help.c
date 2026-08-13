/* HELP.C – Help screen system (translated from ifphelp.pas) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include "msd.h"

/* --------------------------------------------------------------------------
   Типы данных (из ifphelp.pas)
   -------------------------------------------------------------------------- */
typedef struct helptextrec {
    struct helptextrec *before, *after;
    unsigned int lineno;
    char helptext[80];            /* макс. 79 символов + 0 */
} helptextrec;

/* Глобальные переменные модуля (static) */
static unsigned char scrbuf[9600];          /* буфер экрана */
static unsigned char oldattr, oldx, oldy;
static unsigned int vidsize, oldwindmin, oldwindmax;
static int filefound = 0;
static helptextrec *helphead = NULL;
static unsigned long thetable[64];

/* Far-указатели на видеопамять */
#define MONOSCRN   ((unsigned char __far *)0xB0000000UL)
#define COLORSCRN  ((unsigned char __far *)0xB8000000UL)

/* --------------------------------------------------------------------------
   Вспомогательные процедуры (аналоги из ifphelp.pas)
   -------------------------------------------------------------------------- */

/* Позиционирование в файле через DOS (аналог textseek) */
static void textseek(FILE *f, unsigned long position)
{
    union REGS r;
    int handle = fileno(f);
    r.h.ah = 0x42;
    r.h.al = 0;                 /* от начала файла */
    r.x.bx = handle;
    r.x.cx = position >> 16;
    r.x.dx = position & 0xFFFF;
    int86(0x21, &r, &r);
}

/* Чтение строки из текстового файла (замена Readln) */
static void my_Readln(FILE *f, char *s, int maxlen)
{
    int c, i = 0;
    while (i < maxlen - 1) {
        c = fgetc(f);
        if (c == EOF || c == '\n') break;
        s[i++] = c;
    }
    s[i] = '\0';
}

/* Ожидание нажатия любой клавиши */
static void anykey(void)
{
    center("Press <any key> to continue");
    while (!kbhit());
    getch();                     /* съедаем код клавиши */
    if (kbhit()) getch();        /* расширенный код */
}

/* Очистка связного списка строк помощи */
static void clearheap(void)
{
    helptextrec *now = helphead, *next;
    while (now) {
        next = now->after;
        free(now);
        now = next;
    }
    helphead = NULL;
}

/* --------------------------------------------------------------------------
   Сохранение экрана в буфер (setup)
   -------------------------------------------------------------------------- */
static void setup(void)
{
    unsigned int x, y, position = 0;
    union REGS r;
    unsigned char l_vidmode, l_vidlen, l_vidpg;
    unsigned l_vidwid;

    oldattr = TextAttr;
    oldwindmin = WindMin;
    oldwindmax = WindMax;
    oldx = wherex();
    oldy = wherey();
    filefound = 0;

    modeinfo(&l_vidmode, &l_vidlen, &l_vidpg, &l_vidwid);
    vidmode = l_vidmode;
    tlength = l_vidlen;
    vidpg = l_vidpg;
    vidsize = (l_vidwid * l_vidlen) * 2;

    if (_directvideo) {
        if (vidmode == 7)
            _fmemcpy(scrbuf, MONOSCRN, vidsize);
        else
            _fmemcpy(scrbuf, COLORSCRN, vidsize);
    } else {
        for (y = 0; y < l_vidlen; y++) {
            for (x = 0; x < l_vidwid; x++) {
                r.h.ah = 2; r.h.bh = l_vidpg; r.h.dh = y; r.h.dl = x;
                int86(0x10, &r, &r);
                r.h.ah = 8; r.h.bh = l_vidpg;
                int86(0x10, &r, &r);
                scrbuf[position] = r.h.al;
                scrbuf[position + 1] = r.h.ah;
                position += 2;
            }
        }
    }
}

/* --------------------------------------------------------------------------
   Восстановление экрана из буфера (cleanup)
   -------------------------------------------------------------------------- */
static void cleanup(void)
{
    unsigned int x, y, position = 0;
    union REGS r;
    unsigned l_vidwid = vidsize / (2 * tlength);

    if (_directvideo) {
        if (vidmode == 7)
            _fmemcpy(MONOSCRN, scrbuf, vidsize);
        else
            _fmemcpy(COLORSCRN, scrbuf, vidsize);
    } else {
        for (y = 0; y < tlength; y++) {
            for (x = 0; x < l_vidwid; x++) {
                r.h.ah = 2; r.h.bh = vidpg; r.h.dh = y; r.h.dl = x;
                int86(0x10, &r, &r);
                r.h.ah = 9; r.h.al = scrbuf[position]; r.h.bh = vidpg;
                r.h.bl = scrbuf[position + 1]; r.x.cx = 1;
                int86(0x10, &r, &r);
                position += 2;
            }
        }
    }
    TextAttr = oldattr;
    WindMin = oldwindmin;
    WindMax = oldwindmax;
    GotoXY(oldx, oldy);
}

/* --------------------------------------------------------------------------
   Чтение файла помощи для конкретной страницы (readfile)
   -------------------------------------------------------------------------- */
static void readfile(int pg, unsigned long helpver)
{
    char filename[128] = "";
    char *env;
    FILE *tablefile = NULL;
    char s[256];
    int found = 0;
    int half;
    unsigned vidwid_temp;
    unsigned char dummy_mode, dummy_len, dummy_pg;
    FILE *infile;
    helptextrec *previous, *now;
    int endread, linecount;

    modeinfo(&dummy_mode, &dummy_len, &dummy_pg, &vidwid_temp);

    /* Проверка переменной окружения INFOPLUS */
    env = getenv("INFOPLUS");
    if (env && env[0]) {
        strcpy(filename, env);
        if (!strchr(filename, '.')) {
            char last = filename[strlen(filename)-1];
            if (last != ':' && last != '\\' && last != '/')
                strcat(filename, "\\");
            strcat(filename, "INFOPLUS.HLP");
        }
        tablefile = fopen(filename, "rb");
        if (tablefile) found = 1;
        else {
            textcolor(WHITE); textbackground(RED);
            strcpy(s, "INFOPLUS environment variable does not point");
            half = (vidwid_temp - strlen(s)) / 2;
            Window(half - 2, (tlength/2)-3, half + strlen(s) + 2, (tlength/2)+3);
            box(); ClrScr();
            center(s);
            cprintf("\r\n");
            center("to a valid help file directory.");
            cprintf("\r\n");
            cprintf("INFOPLUS=%s\r\n", env);
            cprintf("\r\n");
            anykey(); cleanup(); return;
        }
    }

    if (!found) {
        if (access("INFOPLUS.HLP", 0) == 0) {
            tablefile = fopen("INFOPLUS.HLP", "rb");
            if (tablefile) found = 1;
        }
        if (!found) {
            textcolor(WHITE); textbackground(RED);
            strcpy(s, "Unable to find INFOPLUS.HLP!");
            half = (vidwid_temp - strlen(s)) / 2;
            Window(half - 2, (tlength/2)-2, half + strlen(s) + 2, (tlength/2)+2);
            box(); ClrScr();
            center(s);
            cprintf("\r\n\n");
            anykey(); cleanup(); return;
        }
    }

    /* Чтение таблицы смещений */
    if (fread(thetable, sizeof(long), 64, tablefile) != 64) { fclose(tablefile); return; }
    fclose(tablefile);

    /* Проверка версии */
    if (thetable[63] != helpver) {
        textcolor(WHITE); textbackground(RED);
        strcpy(s, "Incorrect version of INFOPLUS.HLP!");
        half = (vidwid_temp - strlen(s)) / 2;
        Window(half - 2, (tlength/2)-2, half + strlen(s) + 2, (tlength/2)+2);
        box(); ClrScr();
        center(s);
        cprintf("\r\n");
        cprintf("Found version: %.2f\r\n", thetable[63] / 100.0);
        anykey(); cleanup(); return;
    }

    /* Открываем текстовый файл и загружаем строки */
    infile = fopen(filename, "rt");
    if (!infile) return;
    textseek(infile, thetable[pg]);

    clearheap();
    previous = NULL; now = NULL; endread = 0; linecount = 0;

    while (!endread) {
        my_Readln(infile, s, 80);
        if (strcmp(s, "$END") == 0) {
            endread = 1;
        } else {
            now = malloc(sizeof(helptextrec));
            if (!now) {
                textcolor(WHITE); textbackground(RED);
                strcpy(s, "Insufficient memory to read the");
                half = (vidwid_temp - strlen(s)) / 2;
                Window(half - 2, (tlength/2)-3, half + strlen(s) + 2, (tlength/2)+3);
                box(); ClrScr();
                center(s);
                cprintf("\r\n");
                center("full help page");
                cprintf("\r\n\n");
                anykey();
                endread = 1;
            } else {
                if (!helphead) helphead = now;
                else previous->after = now;
                now->before = previous;
                strcpy(now->helptext, s);
                linecount++;
                now->lineno = linecount;
                now->after = NULL;
                previous = now;
            }
        }
    }
    fclose(infile);
    filefound = 1;
}

/* --------------------------------------------------------------------------
   Показ справки с навигацией (showhelp)
   -------------------------------------------------------------------------- */
static void showhelp(void)
{
    unsigned int height, helplength, topline, btmline;
    int endhelp = 0;
    char c2[2];
    helptextrec *now, *p;

    textcolor(WHITE); textbackground(BLUE);
    Window(x2, tlength, twidth, tlength);
    ClrScr();
    cprintf(" \030 \031 PgUp PgDn Home End ESC");
    Window(1, 3, twidth, tlength - 2);
    ClrScr();
    height = (WindMax >> 8) - (WindMin >> 8) + 1;

    now = helphead; helplength = 0;
    while (now) { helplength = now->lineno; now = now->after; }

    topline = 1;
    btmline = (height >= helplength) ? helplength : height;

    p = helphead;
    while (p && p->lineno != topline) p = p->after;
    GotoXY(1, 1);
    while (p && p->lineno <= btmline) {
        if (WhereY() == height) cprintf("%s", p->helptext);
        else cprintf("%s\r\n", p->helptext);
        p = p->after;
    }

    do {
        getkey2(c2);
        if (c2[0] == 27) endhelp = 1;
        else if (c2[0] == 0) {
            switch (c2[1]) {
                case 0x50: if (btmline < helplength) { topline++; btmline++; } break;
                case 0x48: if (topline > 1) { topline--; btmline--; } break;
                case 0x51: if (btmline < helplength) {
                              topline += height; btmline += height;
                              if (btmline > helplength) {
                                  btmline = helplength;
                                  topline = btmline - height + 1;
                              }
                           } break;
                case 0x49: if (topline > 1) {
                              if (topline <= height) {
                                  topline = 1;
                                  btmline = (height >= helplength) ? helplength : height;
                              } else { topline -= height; btmline -= height; }
                           } break;
                case 0x47: topline = 1;
                           btmline = (height >= helplength) ? helplength : height; break;
                case 0x4F: btmline = helplength;
                           if (btmline >= height) topline = btmline - height + 1;
                           else topline = 1; break;
            }
            if (c2[1]) {
                p = helphead;
                while (p && p->lineno != topline) p = p->after;
                GotoXY(1, 1);
                while (p && p->lineno <= btmline) {
                    if (WhereY() == height) cprintf("%s", p->helptext);
                    else cprintf("%s\r\n", p->helptext);
                    p = p->after;
                }
            }
        }
    } while (!endhelp);
}

/* ======================================================================
   HelpScreen – главная процедура (интерфейс из ifphelp)
   ====================================================================== */
void HelpScreen(int pg, unsigned long helpver)
{
    setup();
    readfile(pg, helpver);
    if (!filefound) { cleanup(); return; }
    showhelp();
    clearheap();
    cleanup();
}
