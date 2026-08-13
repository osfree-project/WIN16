/* run.c – Главный цикл (C89, регистр цветов исправлен) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "msd.h"

extern void page00(void); extern void page01(void); extern void page02(void);
extern void page03(void); extern void page04(void); extern void page05(void);
extern void page06(void); extern void page07(void); extern void page08(void);
extern void page09(void); extern void page10(void); extern void page11(void);
extern void page12(void); extern void page13(void); extern void page14(void);
extern void page15(void); extern void page16(void); extern void page17(void);
extern void page18(void); extern void page19(void); extern void page20(void);
extern void page21(void);

void runit(int argc, char *argv[])
{
    union REGS r;
    unsigned xword;
    int done;
    int page_changed;
    unsigned num;

    /* Определение версии DOS */
    r.x.ax = 0x3306;
    int86(0x21, &r, &r);
    if (r.h.al == 0x06 && r.h.bh < 100 && r.h.bl >= 5) {
        xword = r.x.bx;
    } else {
        xword = (_osmajor << 8) | _osminor;
    }
    osmajor = xword & 0xFF;
    osminor = (xword >> 8) & 0xFF;

    if (osmajor >= 3) {
        init(argc, argv);
        done = 0;
        do {
            pagenameclr();
            GotoXY(x1, tlength);
            textcolor(LIGHTGRAY);                     /* БЫЛО LightGray */
            cprintf("%2u - %s", Pg, pgnames[Pg]);
            Window(1, 3, twidth, tlength - 2);
            ClrScr();

            switch (Pg) {
                case 0:  page00(); break;
/*                case 1:  page01(); break;
                case 2:  page02(); break;
                case 3:  page03(); break;
                case 4:  page04(); break;
                case 5:  page05(); break;
                case 6:  page06(); break;
                case 7:  page07(); break;
                case 8:  page08(); break;
                case 9:  page09(); break;
                case 10: page10(); break;
                case 11: page11(); break;
                case 12: page12(); break;
                case 13: page13(); break;
                case 14: page14(); break;
                case 15: page15(); break;
                case 16: page16(); break;
                case 17: page17(); break;
                case 18: page18(); break;
                case 19: page19(); break;
                case 20: page20(); break;
                case 21: page21(); break;*/
            }

            page_changed = 0;
            do {
                Window(1, 1, twidth, tlength);
                GotoXY(x2 - 1, tlength);
                quiet = 0;

                if (PrinterRec.Mode != 'A') {
                    if (!endit) {
                        while (!kbhit());
                        xchar1 = getch();
                        if (xchar1 == 0) xchar2 = getch();
                        else xchar2 = 0;
                    } else {
                        endit = 0;
                        xchar1 = c2[0];
                        xchar2 = c2[1];
                    }
                } else {
                    ScreenPrint(Pg, pgnames[Pg], vernum);
                    xchar1 = 0;
                    if (Pg == pgmax) {
                        PrinterRec.Mode = 'S';
                        xchar2 = 0x47;
                    } else {
                        xchar2 = 0x51;
                    }
                }

                if (xchar1 == 27 && xchar2 == 0) {
                    page_changed = 1;
                    done = 1;
                } else if (xchar1 == 13 && xchar2 == 0) {
                    pagenameclr();
                    GotoXY(x1, tlength);
                    textcolor(WHITE);               /* БЫЛО White */
                    cprintf("Go to page no.=> ");
                    num = getnum();
                    if (num <= pgmax) {
                        Pg = num;
                        page_changed = 1;
                    }
                    if (num == 999) page_changed = 1;
                    pagenameclr();
                    GotoXY(x1, tlength);
                    textcolor(LIGHTGRAY);           /* БЫЛО LightGray */
                    cprintf("%2u - %s", Pg, pgnames[Pg]);
                } else if (xchar1 == 0) {
                    switch (xchar2) {
                        case 0x47: page_changed = 1; Pg = 0; break;
                        case 0x48: if (Pg > 0) { page_changed = 1; Pg--; } break;
                        case 0x4F: page_changed = 1; Pg = pgmax; break;
                        case 0x50: if (Pg < pgmax) { page_changed = 1; Pg++; } break;
                        case 0x49: if (Pg > 0) { page_changed = 1; Pg--; } break;
                        case 0x51: if (Pg < pgmax) { page_changed = 1; Pg++; } break;
                        case 0x19: ScreenPrint(Pg, pgnames[Pg], vernum);
                                   page_changed = 0; quiet = 1; break;
                        case 0x3B: HelpScreen(Pg, HelpVersion);
                                   page_changed = 0; quiet = 1; break;
                    }
                }

                if (!page_changed && !quiet) {
                    sound(220);
                    delay(100);
                    nosound();
                }
            } while (!page_changed);
        } while (!done);

        TextAttr = attrsave;
        if (resetvideo) textmode(vidmode);
        ClrScr();
    } else {
        cprintf("\r\n");
        decimal = '.';
        cprintf("MSD  requires DOS version 3.0 or later\r\n");
        cprintf("Your DOS version is ");
        showvers();
    }
}
