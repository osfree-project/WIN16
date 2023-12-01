/*
**  INFOPLUS.PAS
**
**  Version 1.57 by Yuri Prokushev 01/01/2024
*/

#include <stdlib.h>
#include <stdio.h>
#include <i86.h>
#include <graph.h>

#define VIDPAGE    (*((unsigned char far *)0x0462))

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

void textcolor(int color)
{
	_settextcolor(color);
}

#include "ifprun.ic"

void main(void)
{
  runit();
}

