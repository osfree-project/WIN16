/*
**  INFOPLUS.C
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <i86.h>
#include <graph.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include <conio.h>

#define VIDPAGE    (*((unsigned char far *)0x0462))
#define textmode _setvideomode
#define window(a,b,c,d) _settextwindow(b,a,d,c)
#define clrscr() _clearscreen(_GWINDOW)

void gotoxy(int col, int row)
{
      union REGS regs;

      setbuf(stdout, NULL);
      regs.h.dh = (unsigned)row;
      regs.h.dl = (unsigned)col;
      regs.h.bh = VIDPAGE;
      regs.h.ah = 2;
      int86(0x10, &regs, &regs);
}

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHTGRAY 7
#define DARKGRAY 8
#define LIGHTBLUE 9
#define LIGHTGREEN 10
#define LIGHTCYAN 11
#define LIGHTRED 12
#define LIGHTMAGENTA 13
#define YELLOW 14
#define WHITE 15
#define BLINK 16

void textbackground(int color)
{
	long hex;
	short tcolor;
	if (color > 15){_settextcolor(_gettextcolor() + BLINK);color-=16;}
	if (color == 0) hex = _BLACK;
	if (color == 1) hex = _BLUE;
	if (color == 2) hex = _GREEN;
	if (color == 3) hex = _CYAN;
	if (color == 4) hex = _RED;
	if (color == 5) hex = _MAGENTA;
	if (color == 6) hex = _BROWN;
	if (color == 7) hex = _WHITE;
	if (color == 8) hex = _GRAY;
	if (color == 9) hex = _LIGHTBLUE;
	if (color == 10)hex = _LIGHTGREEN;
	if (color == 11)hex = _LIGHTCYAN;
	if (color == 12)hex = _LIGHTRED;
	if (color == 13)hex = _LIGHTMAGENTA;
	if (color == 14)hex = _YELLOW;
	if (color == 15)hex = _BRIGHTWHITE;
	_setbkcolor(hex);
}

void textcolor(int color)
{
	_settextcolor(color);
}

void textattr(int attr)
{
	int bg = (attr >> 4) & 7, fg = attr & 15, blink = (attr >> 7) & 1;
	textbackground(bg);
	textcolor(fg + blink);
}

int wherex(void)
{
	struct rccoord xy;
	xy = _gettextposition();
	return xy.col-1;
}

int wherey(void)
{
    union REGS regs;

    setbuf(stdout, NULL);
    regs.h.bh = VIDPAGE;
    regs.h.ah = 3;
    int86(0x10, &regs, &regs);

	return regs.h.dh;
}

#define FAR far
#define SCROLL_UP 0
#define SCROLL_DN 1
#if !defined(COLORMODE)
#define COLORMODE  ((*(char FAR *)0x0449) != 7)
#define EXT_KBD    (*(char FAR *)0x0496 & 16)
#define VIDPAGE    (*((unsigned char far *)0x0462))
#define ROWSIZE    (*(int FAR *)0x044A)
#define SCANLINES  ((int)*(char FAR*)0x0461)
#define SCRBUFF    ((unsigned FAR *)((COLORMODE)?0xB8000000:0xB0000000))
#define SCREENSEG  ((unsigned)((COLORMODE)?0xB800:0xB000))
#define SCREENSIZE ((*(int FAR *)0x044C) >> 1)
#define SCREENCOLS (*(int FAR *)0x044A)
#define SCREENROWS ((*(char FAR *)0x0484)?1+(*(char FAR *)0x0484):25)
#endif

void scroll(int direction, int num_lines, int vattrib, int ulrow, int ulcomumn, int lrrow, int lrcolumn)
{
      union REGS regs;
 
      /*
            BH = attribute to be used on blank line
            CH = row of upper left corner of scroll window
            CL = column of upper left corner of scroll window
            DH = row of lower right corner of scroll window
            DL = column of lower right corner of scroll window
      */
 
      regs.h.al = (unsigned char)num_lines;
      regs.h.bh = (unsigned char)vattrib;
      regs.h.ch = (unsigned char)ulrow;
      regs.h.cl = (unsigned char)ulcomumn;
      regs.h.dh = (unsigned char)lrrow;
      regs.h.dl = (unsigned char)lrcolumn;
 
      if (direction == SCROLL_UP)
            regs.h.ah = 0x06;
      else  regs.h.ah = 0x07;
 
      int86(0x10, &regs, &regs);
}

void GetCurPos(int *col, int *row)
{
      union REGS regs;

      regs.h.ah = 0x03;
      regs.h.bh = VIDPAGE;
      int86(0x10, &regs, &regs);
      *row = regs.h.dh;
      *col = regs.h.dl;
}

int GetCurAtr(void)
{
      int row, col;
      unsigned short chat;

      GetCurPos(&col, &row);
      chat = *((unsigned FAR *)MK_FP(SCREENSEG,
            (row * SCREENCOLS + col) << 1));
      return (chat >> 8);
}

void clreol(void)
{
      int row, col;

      GetCurPos(&col, &row);
      scroll(0, 0, GetCurAtr(), row, col, row, SCREENCOLS);
}

#include "ifprun.ic"

void main(int argc, char* argv[])
{
  runit(argc, argv);
}

