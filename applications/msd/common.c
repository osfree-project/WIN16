/* COMMON.C – Полный набор общих процедур (C89-совместимая версия) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <graph.h>
#include <math.h>
#include "msd.h"

/* ======================================================================
   Низкоуровневые функции (аналоги из INFOPLUS.C, адаптированы)
   ====================================================================== */
#define VIDPAGE   (*((unsigned char far *)0x0462))

int wherex(void)
{
    union REGS regs;
    regs.h.bh = VIDPAGE;
    regs.h.ah = 3;
    int86(0x10, &regs, &regs);
    return regs.h.dl;
}

int wherey(void)
{
    union REGS regs;
    regs.h.bh = VIDPAGE;
    regs.h.ah = 3;
    int86(0x10, &regs, &regs);
    return regs.h.dh;
}

void GotoXY(int col, int row)
{
    union REGS regs;
    regs.h.dh = (unsigned)row;
    regs.h.dl = (unsigned)col;
    regs.h.bh = VIDPAGE;
    regs.h.ah = 2;
    int86(0x10, &regs, &regs);
}

void Window(int left, int top, int right, int bottom)
{
    _settextwindow(top, left, bottom, right);
}

void ClrScr(void)
{
    _clearscreen(_GWINDOW);
}

int WhereY(void)
{
    return wherey();
}

/* ======================================================================
   Цветовые обёртки (учёт монохромного режима)
   ====================================================================== */
void TextColor(int color)
{
    if (mono) {
        switch (color & 0x0F) {
            case 0:  color = 0; break;
            case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                color = 7; break;
            default: color = 15; break;
        }
        if (color > 15) color += BLINK;
    }
    textattr(color);
    TextAttr = color;
}

void TextBackground(int color)
{
    if (mono && color < 7) color = 0;
    _setbkcolor(color);
}

/* ======================================================================
   Вспомогательные функции
   ====================================================================== */
void getkey2(char *ch2)
{
    int c = getch();
    if (c == 0) {
        ch2[0] = 0;
        ch2[1] = getch();
    } else {
        ch2[0] = (char)c;
        ch2[1] = 0;
    }
}

unsigned getnum(void)
{
    char number_string[3] = "";
    int position = 0;
    int finish = 0;
    unsigned temp = 99;
    int row = wherey(), col = wherex();
    int c;

    cprintf("   ");
    GotoXY(col, row);
    TextColor(LIGHTGRAY);
    do {
        c = getch();
        if (c >= '0' && c <= '9' && position < 2) {
            number_string[position++] = (char)c;
            number_string[position] = '\0';
            putchar(c);
        } else if (c == '\b' && position > 0) {
            position--;
            number_string[position] = '\0';
            cprintf("\b \b");
        } else if (c == 27) {
            if (position == 0) finish = 1;
            else {
                number_string[0] = '\0';
                GotoXY(col, row);
                ClrScr();
                position = 0;
            }
        } else if (c == '\r') finish = 1;
    } while (!finish);

    if (number_string[0]) {
        char *end;
        unsigned long val = strtoul(number_string, &end, 10);
        if (end != number_string) temp = (unsigned)val;
    } else temp = 999;
    return temp;
}

/* ======================================================================
   Функции вывода
   ====================================================================== */
void Caption1(const char *a)
{
    TextColor(LIGHTGRAY);
    cprintf("%s", a);
    TextColor(LIGHTCYAN);
}

void Caption2(const char *a)
{
    char buf[256];
    int len;
    strcpy(buf, a);
    len = strlen(buf);
    while (len > 0 && buf[len-1] == ' ') len--;
    buf[len] = '\0';
    strcat(buf, ": ");
    Caption1(buf);
}

void Caption3(const char *a)
{
    char buf[256] = "  ";
    strcat(buf, a);
    Caption2(buf);
}

int nocarry(union REGS *r)
{
    return (r->x.cflag & 1) == 0;
}

char *hex(unsigned a, int b)
{
    static char buf[9];
    int i;
    for (i = b-1; i >= 0; i--) {
        buf[i] = "0123456789ABCDEF"[a & 0xF];
        a >>= 4;
    }
    buf[b] = '\0';
    return buf;
}

void unknown(const char *msg, unsigned code, int base)
{
    cprintf("(unknown %s %s)\r\n", msg, hex(code, base));
}

void YesOrNo(int cond)
{
    cprintf(cond ? "yes\r\n" : "no\r\n");
}

void YesOrNo2(int cond)
{
    cprintf(cond ? "yes" : "no");
}

void YesOrNo3(int cond)
{
    YesOrNo2(cond);
    if (!cond) cprintf(" ");
}

void dontknow(void)
{
    cprintf("(unknown)\r\n");
}

void dontknow2(void)
{
    cprintf("(unknown)");
}

void SegOfs(unsigned seg, unsigned ofs)
{
    cprintf("%04X:%04X", seg, ofs);
}

char showchar(char c)
{
    return (c >= ' ' && c <= '~') ? c : '.';
}

unsigned long power2(unsigned y)
{
    return 1UL << y;
}

/* ======================================================================
   Паузы и навигация
   ====================================================================== */
void pause1(void)
{
    unsigned char saveattr = TextAttr;
    int sx, sy;
    char ch2[2];
    endit = 0;
    TextColor(CYAN);
    sx = wherex();
    sy = wherey();
    cprintf("(\x19 for more)");   /* 0x19 = стрелка вниз */
    if (PrinterRec.Mode == 'A') {
        c2[0] = 0; c2[1] = 0x80;
        TextAttr = saveattr;
        GotoXY(sx, sy);
        cprintf("            ");
        return;
    }
    do {
        getkey2(ch2);
        if (ch2[0] == 0 && ch2[1] == 25) { } /* Ctrl+Y */
        if (ch2[0] == 0 && ch2[1] == 0x3B) { } /* F1 */
    } while (ch2[0] == 0 && ch2[1] == 0);
    if (!(ch2[0] == 0 && ch2[1] == 0x80)) {
        endit = 1;
        c2[0] = ch2[0]; c2[1] = ch2[1];
    }
    TextAttr = saveattr;
    GotoXY(sx, sy);
    cprintf("            ");
}

void pause2(void)
{
    if (wherey() + (WindMin >> 8) > (WindMax >> 8)) {
        unsigned char attr = TextAttr;
        TextColor(CYAN);
        pause1();
        if (!endit) {
            ClrScr();
            cprintf("(continued)\r\n");
        }
        TextAttr = attr;
    }
}

void pause3(int extra)
{
    endit = 0;
    if (wherey() + (WindMin >> 8) + abs(extra) > (WindMax >> 8)) {
        unsigned char attr = TextAttr;
        TextColor(CYAN);
        pause1();
        if (!endit) {
            ClrScr();
            if (extra < 0) cprintf("(continued)\r\n");
        }
        TextAttr = attr;
    }
}

void pause4(int direc, char *ch2)
{
    unsigned char attr = TextAttr;
    int sx, sy;
    endit = 0;
    TextColor(CYAN);
    sx = wherex();
    sy = wherey();
    switch (direc) {
        case DIR_NONE:   cprintf("(any key)"); break;
        case DIR_UP:     cprintf("(\x18 for more)"); break;   /* 0x18 = стрелка вверх */
        case DIR_DOWN:   cprintf("(\x19 for more)"); break;
        case DIR_UPDOWN: cprintf("(\x18 or \x19 for more)"); break;
    }
    do {
        if (PrinterRec.Mode == 'A') {
            if (direc == DIR_UP)
                ch2[0] = 0, ch2[1] = 0x81;
            else {
                ch2[0] = 0; ch2[1] = 0x80;
            }
        } else {
            getkey2(ch2);
            if (ch2[0] == 0 && ch2[1] == 25) { ch2[0] = ch2[1] = 0; }
            if (ch2[0] == 0 && ch2[1] == 0x3B) { ch2[0] = ch2[1] = 0; }
        }
    } while (ch2[0] == 0 && ch2[1] == 0);
    if (ch2[0] != 0 || (ch2[0] == 0 && ch2[1] != 0x80 && ch2[1] != 0x72)) {
        endit = 1;
    }
    TextAttr = attr;
    GotoXY(sx, sy);
    cprintf("                 ");
}

void pause5(int direc, char *ch2)
{
    ch2[0] = ch2[1] = 0;
    if (wherey() + (WindMin >> 8) > (WindMax >> 8)) {
        unsigned char attr = TextAttr;
        TextColor(CYAN);
        pause4(direc, ch2);
        if (!endit) ClrScr();
        TextAttr = attr;
    }
}

/* ======================================================================
   Форматирование и строки
   ====================================================================== */
char *bin4(unsigned char a)
{
    static char buf[5];
    int i;
    for (i = 3; i >= 0; i--)
        buf[3-i] = (a & (1 << i)) ? '1' : '0';
    buf[4] = '\0';
    return buf;
}

void offoron(const char *a, int b)
{
    Caption3(a);
    cprintf(b ? "on" : "off");
}

void zeropad(unsigned a)
{
    cprintf("%02u", a);
}

void zeropad3(unsigned a)
{
    cprintf("%03u", a);
}

void showvers(void)
{
    if (osmajor > 0)
        cprintf("%u.%02u", osmajor, osminor);
    else
        cprintf("1.x");
}

unsigned cbw(unsigned char a, unsigned char b)
{
    return ((unsigned)b << 8) | a;
}

char *bin16(unsigned a)
{
    static char buf[20];
    sprintf(buf, "%s_%s_%s_%s",
            bin4(a >> 12), bin4((a >> 8) & 0xF),
            bin4((a >> 4) & 0xF), bin4(a & 0xF));
    return buf;
}

void drvname(unsigned char a)
{
    cprintf("%c: ", 'A' + a);
}

void media(unsigned char a, unsigned char b)
{
    Caption3("Media");
    switch (a) {
        case 0xFF: cprintf("floppy 2 side, 8 sctr, 40 trk\r\n"); break;
        case 0xFE: cprintf("floppy 1 side, 8 sctr, 40 trk\r\n"); break;
        case 0xFD: cprintf("floppy 2 side, 9 sctr, 40 trk\r\n"); break;
        case 0xFC: cprintf("floppy 1 side, 9 sctr, 40 trk\r\n"); break;
        case 0xF9: cprintf("floppy 2 side, %s sctr, 80 trk\r\n",
                           (b == 1) ? "15" : "9"); break;
        case 0xF8: cprintf("fixed disk\r\n"); break;
        case 0xF0: cprintf("floppy 2 side, 18 sctr, 80 trk\r\n"); break;
        default:   unknown("media", a, 2);
    }
}

void pagenameclr(void)
{
    unsigned char attr = TextAttr;
    Window(1, tlength, twidth - 1, tlength);
    TextColor((attr & 0x70) >> 4);
    ClrScr();
    TextAttr = attr;
    Window(1, 1, twidth, tlength);
}

void box(void)
{
    static const char frame[] = "\xDA\xC4\xBF\xB3\xB3\xC0\xC4\xD9";
    unsigned w = (WindMax & 0xFF) - (WindMin & 0xFF) + 1;
    unsigned h = (WindMax >> 8) - (WindMin >> 8) + 1;
    unsigned x, y;
    WindMax += 0x0101;
    GotoXY(1, 1);
    putchar(frame[0]);
    for (x = 2; x < w; x++) putchar(frame[1]);
    GotoXY(w, 1); putchar(frame[2]);
    for (y = 2; y < h; y++) {
        GotoXY(1, y); putchar(frame[3]);
        GotoXY(w, y); putchar(frame[4]);
    }
    GotoXY(1, h); putchar(frame[5]);
    for (x = 2; x < w; x++) putchar(frame[6]);
    GotoXY(w, h); putchar(frame[7]);
    WindMax -= 0x0202;
    WindMin += 0x0101;
}

void center(const char *s)
{
    int halfwidth = ((WindMax & 0xFF) - (WindMin & 0xFF)) / 2;
    int halfstr = strlen(s) / 2;
    int i;
    for (i = 0; i < halfwidth - halfstr; i++) putchar(' ');
    cprintf("%s", s);
}

int EMSOK(void)
{
    unsigned long vec;
    unsigned seg;
    char name[9];
    int i;
    union REGS r;

    vec = ((unsigned long)FP_SEG(intvec[0x67]) << 16) + FP_OFF(intvec[0x67]);
    if (vec == 0) return 0;
    seg = (unsigned)(vec >> 16);
    for (i = 0; i < 8; i++) name[i] = peekb(seg, 0x0A + i);
    name[8] = '\0';
    if (strcmp(name, "EMMXXXX0") != 0) return 0;
    r.h.ah = 0x40;
    int86(0x67, &r, &r);
    return (r.h.ah == 0);
}

/* ======================================================================
   Работа с регистрами и прерываниями
   ====================================================================== */
void _settextattr(int attr)
{
    _settextcolor(attr & 0x0F);          /* младший полубайт – цвет текста */
    _setbkcolor((attr >> 4) & 0x07);     /* старший полубайт – цвет фона */
}

void Intr(int intno, union REGS *regs)
{
    AltIntr(intno, regs);
}

void MsDos(union REGS *regs)
{
    AltMsDos(regs);
}

unsigned char UnBCD(unsigned char b)
{
    return (b & 0x0F) + ((b >> 4) * 10);
}

char *addzero(unsigned char b)
{
    static char s[3];
    sprintf(s, "%02u", b);
    return s;
}

void modeinfo(unsigned char *vidmode, unsigned char *vidlen,
              unsigned char *vidpg, unsigned *vidwid)
{
    union REGS r;
    r.h.ah = 0x0F;
    int86(0x10, &r, &r);
    *vidmode = r.h.al;
    *vidwid = r.h.ah;
    *vidpg = r.h.bh;
    r.x.ax = 0x1200;
    r.h.bl = 0x10;
    int86(0x10, &r, &r);
    if (r.h.bl == 0x10)
        *vidlen = 25;
    else
        *vidlen = peekb(0x40, 0x84) + 1;
}

/* Обертки для стандартных функций, отсутствующих в библиотеке */
void textmode(int mode) {
    _setvideomode(mode);
}

void textcolor(int color) {
    _settextcolor(color);
}

void textbackground(int color) {
    _setbkcolor(color);
}

void clrscr(void) {
    _clearscreen(_GCLEARSCREEN);
}

void gotoxy(int x, int y) {
    GotoXY(x, y);
}

void textattr(int attr) {
    _settextattr(attr);
}

int GetTextAttr(void) {
    union REGS r;
    r.h.ah = 0x08;
    r.h.bh = vidpg;       /* текущая страница */
    int86(0x10, &r, &r);
    return (int)r.h.ah;   /* атрибут символа под курсором */
}
