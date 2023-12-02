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
	return xy.col;
}

int wherey(void)
{
	struct rccoord xy;
	xy = _gettextposition();
	return xy.row;
}

#include "ifprun.ic"

void main(int argc, char* argv[])
{
  runit(argc, argv);
}

