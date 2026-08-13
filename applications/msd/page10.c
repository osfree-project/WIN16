/* PAGE10.C – Multiplex interrupts, APPEND, PRINT, SETVER (from page_10.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

/* --------------------------------------------------------------------------
   Локальные static-функции
   -------------------------------------------------------------------------- */

static void muxint(const char *name, unsigned char func)
{
    Caption3(name);
    union REGS r;
    r.h.ah = func;
    r.h.al = 0;
    r.x.bx = 0;
    r.x.cx = 0;
    r.x.dx = 0;
    int86(0x2F, &r, &r);
    switch (r.h.al)
    {
        case 0x00: cprintf("no; OK to load\r\n"); break;
        case 0x01: cprintf("no; not OK to load\r\n"); break;
        case 0xFF:
        case 2:    cprintf("yes\r\n"); break;
        default:   unknown("status", r.h.al, 2);
    }
}

static unsigned windev(unsigned char device)
{
    union REGS r;
    r.x.ax = 0x1682;
    int86(0x2F, &r, &r);
    r.x.ax = 0x170A;
    r.x.dx = device;
    int86(0x2F, &r, &r);
    return r.x.ax;
}

/* ========================================================================== */
void page10(void)
{
    unsigned i;
    int xbool1, xbool2, xbool3;
    char xchar;
    unsigned xword1, xword2;
    unsigned char xbyte;
    unsigned count;
    char s[80];

    Caption1("");
    cprintf("Multiplex interrupt ($2F)\r\n");

    Window(1, 4, twidth / 2, tlength - 2);

    muxint("DOS            ", 0x12);
    muxint("DRIVER.SYS     ", 0x08);
    muxint("DISPLAY.SYS    ", 0xB0);
    muxint("ANSI.SYS       ", 0x1A);
    muxint("EGA.SYS        ", 0xBC);
    muxint("PRINT          ", 0x01);
    muxint("ASSIGN         ", 0x06);
    muxint("SHARE          ", 0x10);
    muxint("NLSFUNC        ", 0x14);
    muxint("GRAFTABL (4.0-)", 0xB0);

    Caption3("GRAFTABL (5.0+)");
    {
        union REGS r;
        r.x.ax = 0x2300;
        r.x.bx = 0;
        r.x.cx = 0;
        r.x.dx = 0;
        int86(0x2F, &r, &r);
        if (r.h.ah == 0xFF)
            cprintf("yes\r\n");
        else
            cprintf("no; OK to load\r\n");
    }

    muxint("NETBIOS append ", 0x87);
    muxint("NETBIOS network", 0x88);
    muxint("SHELLB         ", 0x19);
    muxint("XMA2EMS        ", 0x1B);
    muxint("APPEND         ", 0xB7);
    muxint("GRAPHICS.COM   ", 0x15);
    muxint("Crit.err.handlr", 0x05);

    pause3(-2);
    if (endit) return;

    Caption3("CDROM          ");
    {
        union REGS r;
        r.x.ax = 0x1500;
        r.x.bx = 0;
        int86(0x2F, &r, &r);
        if (r.x.bx == 0)
        {
            cprintf("no; OK to load\r\n");
        }
        else
        {
            cprintf("yes\r\n");
            Caption3("  on drives");
            putchar(r.h.cx + 'A');
            Caption3("through");
            cprintf("%c\r\n", r.h.cx + r.h.bx + 'A' - 1);
        }
    }

    Caption3("Network        ");
    {
        union REGS r;
        r.x.ax = 0xB800;
        int86(0x2F, &r, &r);
        if (r.h.al == 0)
        {
            cprintf("no; OK to load\r\n");
        }
        else
        {
            cprintf("yes");
            Caption3("is a");
            if (r.x.bx & 0x40)      cprintf("server");
            else if (r.x.bx & 4)    cprintf("messenger");
            else if (r.x.bx & 0x80) cprintf("receiver");
            else if (r.x.bx & 8)    cprintf("redirector");
            cprintf("\r\n");
        }
    }

    muxint("DOSKEY         ", 0x48);

    Caption3("DOS Extender   ");
    {
        union REGS r;
        r.x.ax = 0xF100;
        r.x.bx = 0;
        r.x.cx = 0;
        r.x.dx = 0;
        int86(0x2F, &r, &r);
        if (r.h.al == 0xFF && r.x.si == 0x444F /* 'DO' */ && r.x.di == 0x5358 /* 'SX' */)
            cprintf("yes\r\n");
        else
            cprintf("no; OK to load\r\n");
    }

    /* Правая половина экрана */
    Window(1 + twidth / 2, 3, twidth, tlength - 3);

    if (OSMajor >= 4)
    {
        union REGS r;
        r.x.ax = 0xB700;
        int86(0x2F, &r, &r);
        if (r.h.al == 0xFF)
        {
            Caption2("APPEND ");
            r.x.ax = 0xB706;
            int86(0x2F, &r, &r);
            if (r.x.bx & 1)        cprintf("enabled ");
            if (r.x.bx & 0x2000)   cprintf("/PATH ");
            if (r.x.bx & 0x4000)   cprintf("/E ");
            if (r.x.bx & 0x8000)   cprintf("/X");
            cprintf("\r\n");

            Caption2("APPEND path");
            r.x.ax = 0xB704;
            int86(0x2F, &r, &r);
            while (peekb(r.x.es, r.x.di) != 0)
            {
                putchar(peekb(r.x.es, r.x.di));
                r.x.di++;
            }
            cprintf("\r\n");
        }
    }

    {
        union REGS r;
        r.x.ax = 0x0100;
        int86(0x2F, &r, &r);
        if (r.h.al == 0xFF)
        {
            Caption2("PRINT queue");
            r.x.ax = 0x0104;
            int86(0x2F, &r, &r);
            xbool1 = 1;
            xbool2 = 0;
            do
            {
                xchar = peekb(r.x.ds, r.x.si);
                if (xchar > 0)
                {
                    if (xbool1)
                    {
                        xbool1 = 0;
                        cprintf("\r\n");
                        Window(2 + twidth / 2, wherey() + 1, twidth, tlength - 3);
                    }
                    pause2();
                    if (endit) return;
                    putchar(xchar);
                    i = 1;
                    xbool3 = 0;
                    do
                    {
                        xchar = peekb(r.x.ds, r.x.si + i);
                        if (xchar > 0)
                        {
                            putchar(xchar);
                            i++;
                        }
                        else
                        {
                            cprintf("\r\n");
                            xbool3 = 1;
                        }
                    } while (!xbool3);
                    r.x.si += 64;
                }
                else
                    xbool2 = 1;
            } while (!xbool2);
            if (xbool1)
                cprintf("(empty)\r\n");
            r.x.ax = 0x0105;
            int86(0x2F, &r, &r);
        }
    }

    if (OSMajor == 5)
    {
        xword1 = peekw(devseg, devofs + 0x39);
        xword2 = peekw(devseg, devofs + 0x37);
        if (xword1 != 0 && xword2 != 0)
        {
            Caption2("SETVER list at ");
            SegOfs(xword1, xword2);
            cprintf("\r\n");
            while (peekb(xword1, xword2) != 0)
            {
                xbyte = peekb(xword1, xword2);
                xword2++;
                s[0] = '\0';
                for (count = 0; count < xbyte; count++)
                    s[count] = peekb(xword1, xword2 + count);
                s[xbyte] = '\0';
                xword2 += xbyte;
                cprintf("%-14s %u.%02u\r\n", s,
                        peekb(xword1, xword2),
                        peekb(xword1, xword2 + 1));
                pause2();
                if (endit) return;
                xword2 += 2;
            }
        }
    }
}
