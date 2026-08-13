/* PAGE00.C Ц Welcome screen and page list (translated from page_00.pas) */

#include <stdio.h>
#include <string.h>
#include "msd.h"

void page00(void)
{
    int x, y;
    int center_x;
    char buf[128];

    /* Ћоготип "MSD" (5 строк, как в оригинальном "INFO") */
    textcolor(LIGHTGREEN);
    Window((twidth / 2) - 16, 3, (twidth / 2) + 17, 8);
    cprintf("#   #  #####  #### \r\n");
    cprintf("## ##  #      #   #\r\n");
    cprintf("# # #  #####  #   #\r\n");
    cprintf("#   #      #  #   #\r\n");
    cprintf("#   #  #####  #### \r\n");

    Window(1, (tlength / 2) - 4, twidth, (tlength / 2) - 3);
    textcolor(LIGHTCYAN);
    for (x = 1; x <= twidth; x++)
        cprintf("\xDC");                     /* нижн€€ половина блока */

    Window(1, (tlength / 2) - 3, (twidth / 2) - 1, tlength - 2);
    textcolor(WHITE);
    for (x = 0; x <= 9; x++)
        cprintf("Page %d  - %s\r\n", x, pgnames[x]);
    cprintf("Page 10 - %s", pgnames[10]);

    Window((twidth / 2) + 1, (tlength / 2) - 3, twidth, tlength - 2);
    for (x = 11; x <= 20; x++)
        cprintf("Page %d - %s\r\n", x, pgnames[x]);
    cprintf("Page 21 - %s", pgnames[21]);

    Window(1, (tlength / 2) + 8, twidth, (tlength / 2) + 9);
    textcolor(LIGHTCYAN);
    for (x = 1; x <= twidth; x++)
        cprintf("\xDC");

    Window(1, 1, twidth, tlength);
    center_x = twidth / 2;
    for (y = (tlength / 2) - 3; y <= (tlength / 2) + 8; y++) {
        GotoXY(center_x, y);
        cprintf("\xDB");                     /* полный блок */
    }

    Window(1, tlength - 5, twidth, tlength - 1);
    textcolor(YELLOW);
    cprintf("\r\n");
    sprintf(buf, "INFOPLUS - %s by Andrew Rossmann, %s", qversion, qdate);
    center(buf);
    center("Based on SYSID 4.44, by Steve Grant");
    center("Released to the Public Domain");
}
