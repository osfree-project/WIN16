/* SCRPRINT.C – Screen printing routines (исправлено) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <graph.h>
#include "msd.h"

static const char LoChars[32] = " abcdefghijklmnopqrstuvwxyz<+>^v";
static const char HiChars[128] = {
    'c','u','e','a','a','a','a','c','e','e','e','i','i','i','A','A',
    'E','a','A','o','o','o','u','u','y','O','U','c','L','Y','P','f',
    'a','i','o','u','n','N','a','o','?','+','+','2','4','i','<','>',
    '.','o','O','|','+','+','+','+','+','+','|','+','+','+','+','+',
    '+','+','+','+','-','+','+','+','+','+','+','+','+','-','+','+',
    '+','+','+','+','+','+','+','+','+','+','_','|','|','~',' ',' ',
    'a','B','r','#','E','o','u','t','0','0','^','o','8','0','E','U',
    '=','+','>','<','f','j','-','~','o','O','j','n','2','O',' ',' '
};
static const char Dashes[] = "----------------------------------------"
                             "---------------------------------------";

static char *Today(void) {
    static char buf[12];
    struct dosdate_t d;
    _dos_getdate(&d);
    sprintf(buf, "%02u/%02u/%04u", d.month, d.day, d.year);
    return buf;
}

static char *Now(void) {
    static char buf[12];
    struct dostime_t t;
    _dos_gettime(&t);
    sprintf(buf, "%02u:%02u:%02u", t.hour, t.minute, t.second);
    return buf;
}

void ScreenPrint(int Pg, const char *PgName, const char *VerNum)
{
    unsigned char scrbuf[9600];
    unsigned char vidmode, vidpg, tlength;
    unsigned vidsize, vidwidth, position, x, y, bytesperline, bytesperscreen;
    unsigned charcount, first, last;
    unsigned oldwindmin, oldwindmax;
    union REGS r;
    FILE *outfile = NULL;
    char c;
    int striphi;
    char extrastr[256] = "";
    int firstrun = (PrinterRec.Mode == 'A' && PrinterRec.Destination == '?');
    int singlescreen = (PrinterRec.Mode != 'A');
    unsigned char oldattr;
    int oldx, oldy;

    /* Получаем параметры текущего видеорежима напрямую */
    r.h.ah = 0x0F; int86(0x10, &r, &r);
    vidmode = r.h.al; vidwidth = r.h.ah; vidpg = r.h.bh;

    r.x.ax = 0x1130; r.h.bh = 0; int86(0x10, &r, &r);
    tlength = r.h.dl + 1;

    vidsize = vidwidth * tlength * 2;
    position = 0;

    /* Сохраняем текущие атрибуты и позицию */
    oldattr = GetTextAttr();
    oldwindmin = WindMin; oldwindmax = WindMax;
    oldx = wherex(); oldy = wherey();

    if (_directvideo) {
        if (vidmode == 7)
            _fmemcpy(scrbuf, (unsigned char far *)0xB0000000UL, vidsize);
        else
            _fmemcpy(scrbuf, (unsigned char far *)0xB8000000UL, vidsize);
    } else {
        for (y = 0; y < tlength; y++)
            for (x = 0; x < vidwidth; x++) {
                r.h.ah = 2; r.h.bh = vidpg; r.h.dh = y; r.h.dl = x; int86(0x10, &r, &r);
                r.h.ah = 8; r.h.bh = vidpg; int86(0x10, &r, &r);
                scrbuf[position] = r.h.al; scrbuf[position+1] = r.h.ah;
                position += 2;
            }
    }

    if (firstrun || singlescreen) c = 'P'; else c = PrinterRec.Destination;
    if (c == 'P') { outfile = fopen("PRN", "w"); if (!outfile) return; }
    else { outfile = fopen(PrinterRec.Filename, "w"); if (!outfile) return; }

    if (singlescreen || firstrun) striphi = 1; else striphi = PrinterRec.HiStrip;

    bytesperline = vidwidth * 2; bytesperscreen = bytesperline * tlength;
    first = bytesperline * 2; last = bytesperscreen - bytesperline * 2 - 1;

    fprintf(outfile, "%s\r\n", Dashes);
    fprintf(outfile, "Infoplus %s   Page %u - %s\r\n", VerNum, Pg, PgName);
    fprintf(outfile, "Generated: %s at %s\r\n", Today(), Now());
    fprintf(outfile, "%s\r\n", Dashes);

    charcount = 0;
    for (x = first; x < last; x += 2) {
        c = scrbuf[x];
        if ((unsigned char)c < 32) c = LoChars[(unsigned char)c];
        if (striphi && (unsigned char)c > 127) c = HiChars[(unsigned char)c - 128];
        fputc(c, outfile); charcount++;
        if (charcount == 80) { fputs("\r\n", outfile); charcount = 0; }
    }
    fputs("\r\n", outfile);
    fputc('\f', outfile);
    fclose(outfile);

    /* Восстановление экрана */
    if (_directvideo) {
        if (vidmode == 7) _fmemcpy((unsigned char far *)0xB0000000UL, scrbuf, vidsize);
        else _fmemcpy((unsigned char far *)0xB8000000UL, scrbuf, vidsize);
    } else {
        position = 0;
        for (y = 0; y < tlength; y++)
            for (x = 0; x < vidwidth; x++) {
                r.h.ah = 2; r.h.bh = vidpg; r.h.dh = y; r.h.dl = x; int86(0x10, &r, &r);
                r.h.ah = 9; r.h.al = scrbuf[position]; r.h.bh = vidpg;
                r.h.bl = scrbuf[position+1]; r.x.cx = 1;
                int86(0x10, &r, &r);
                position += 2;
            }
    }
    textattr(oldattr);
    WindMin = oldwindmin; WindMax = oldwindmax;
    GotoXY(oldx, oldy);
}
