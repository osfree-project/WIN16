/* PAGE21.C – Thank You / Final screen (translated from page_21.pas) */

#include <stdio.h>
#include <string.h>
#include "msd.h"

void page21(void)
{
    unsigned char xbyte;

    TextColor(White);
    GotoXY((twidth / 2) - 15, 1);
    cprintf("Thank You for using INFOPLUS!!\r\n");
    cprintf("\r\n");
    cprintf("\r\n");

    TextColor(LightCyan);
    cprintf("  This is my final version of Infoplus. It's not 100% complete, which\r\n");
    cprintf("is why it's an alpha. The help screens have not been updated, and I\r\n");
    cprintf("didn't make all the changes I wanted to.\r\n");
    cprintf("\r\n");
    cprintf("  As of September 17th, 1993, I have no e-mail address. The Infoplus BBS\r\n");
    cprintf("has also been shut down.\r\n");

    /* Закомментированный блок из оригинала (контактная информация) сохранён как комментарий */
#if 0
    cprintf("  If you have any questions, bug reports, or suggestions, I can be\r\n");
    cprintf("reached at the following places:\r\n");
    cprintf("\r\n");
    TextColor(LightRed);
    cprintf("Internet       : andyross@infopls.chi.il.us\r\n");
    cprintf("UUCP           : uunet!infopls!andyross\r\n");
    cprintf("CompuServe     : >INTERNET:andyross@infopls.chi.il.us\r\n");
    cprintf("\r\n");
    cprintf("Infoplus BBS   : (708)537-0247 (14400/9600/2400/1200 v32bis/v42bis/MNP)\r\n");
    cprintf("Beacon         : (708)615-0845 (2400/1200)\r\n");
#endif

    Window(1, tlength - 2, twidth, tlength - 2);
    xbyte = TextAttr;
    TextColor(White);
    TextBackground(Brown);
    ClrScr();
    cprintf("INFOPLUS %s, by Andrew Rossmann, %s", qversion, qdate);
    TextAttr = xbyte;
}
