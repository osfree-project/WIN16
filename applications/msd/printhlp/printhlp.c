/* PRINTHLP.C – Print help pages from INFOPLUS.HLP (translated from printhlp.pas) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#define MINPAGE   0
#define MAXPAGE   21          /* pgmax из ifpglobl */
#define HELPVERSION 157       /* helpversion из ifpglobl */
#define VERNUM    "1.58a"

const char *dashes =
    "----------------------------------------"
    "---------------------------------------";

static const char *pgnames[] = {
    "Table of Contents",
    "Machine & ROM Identification",
    "CPU Identification",
    "RAM Identification",
    "Memory Block Listing",
    "Video Identification",
    "Video Information",
    "Keyboard & Mouse Information",
    "Parallel/Serial/Sound Ports",
    "DOS Information",
    "Multiplex Programs",
    "Environment Variables",
    "Device Drivers",
    "DOS Drive Information",
    "BIOS Drive Information",
    "Partition Table Listing",
    "Boot & DOS drive parameters",
    "CMOS information",
    "TSR's and Drivers",
    "Alternate Multiplex",
    "Memory Managers",
    "Thanks"
};

/* Таблица смещений страниц в файле помощи (64 элемента) */
static unsigned long thetable[64];

/* Позиционирование текстового файла (как в HELP.C) */
static void textseek(FILE *f, unsigned long position)
{
    union REGS r;
    int handle = fileno(f);
    r.h.ah = 0x42;
    r.h.al = 0;
    r.x.bx = handle;
    r.x.cx = position >> 16;
    r.x.dx = position & 0xFFFF;
    int86(0x21, &r, &r);
}

/* Печать одной страницы */
static void printapage(FILE *outfile, int page, unsigned maxlines)
{
    char s[256];
    int linecount;
    int endit = 0;
    FILE *infile = fopen("INFOPLUS.HLP", "rt");

    if (!infile) {
        fprintf(stderr, "Cannot reopen INFOPLUS.HLP!\n");
        return;
    }
    textseek(infile, thetable[page]);

    fprintf(outfile, "%s\r\n", dashes);
    fprintf(outfile, "Infoplus %s   Page %02d - %s\r\n", VERNUM, page, pgnames[page]);
    fprintf(outfile, "%s\r\n\r\n", dashes);

    linecount = 4;   /* уже вывели 4 строки */
    while (!endit) {
        if (fgets(s, sizeof(s), infile) == NULL) break;
        /* убираем символ новой строки */
        s[strcspn(s, "\r\n")] = '\0';
        if (strcmp(s, "$END") == 0) {
            endit = 1;
        } else {
            fprintf(outfile, "%s\r\n", s);
            linecount++;
            if (linecount != 255 && linecount == maxlines) {
                fputc('\f', outfile);
                linecount = 1;
            }
        }
    }
    fclose(infile);
    fputc('\f', outfile);
}

int main(void)
{
    char filename[128];
    char s[80];
    int printpage;
    unsigned maxlines;
    int x;
    FILE *outfile;

    printf("PRINTHLP ver 1.57 by Andrew Rossmann. For use with Infoplus 1.57\n");
    printf("PRINTHLP prints out the .HLP pages to your printer. Each page\n");
    printf("will have a header describing which Infoplus page it belongs to.\n\n");

    /* Выбор страницы */
    do {
        printf("Which page to print? %d - %d, 99 for all.=> ", MINPAGE, MAXPAGE);
        fgets(s, sizeof(s), stdin);
        printpage = atoi(s);
    } while ((printpage < MINPAGE || printpage > MAXPAGE) && printpage != 99);

    /* Устройство вывода */
    printf("Which device to you wish to print to? <ENTER> for PRN.=> ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\r\n")] = '\0';
    if (filename[0] == '\0')
        strcpy(filename, "PRN");

    outfile = fopen(filename, "wt");
    if (!outfile) {
        fprintf(stderr, "\aUnable to open %s for output!!\n", filename);
        return 1;
    }

    /* Количество строк на странице */
    printf("How many lines per page? <ENTER> for 60, 255 for continuous.=> ");
    fgets(s, sizeof(s), stdin);
    maxlines = atoi(s);
    if (maxlines == 0) maxlines = 60;

    /* Чтение таблицы смещений */
    {
        FILE *tbl = fopen("INFOPLUS.HLP", "rb");
        if (!tbl) {
            fprintf(stderr, "\aUnable to open INFOPLUS.HLP!\n");
            fclose(outfile);
            return 1;
        }
        fread(thetable, sizeof(long), 64, tbl);
        fclose(tbl);
    }

    /* Проверка версии */
    if (thetable[63] != HELPVERSION) {
        fprintf(stderr, "\aIncorrect version of INFOPLUS.HLP!\n");
        fprintf(stderr, "Found version %.2f\n", thetable[63] / 100.0);
        fclose(outfile);
        return 1;
    }

    /* Печать */
    if (printpage == 99) {
        for (x = MINPAGE; x <= MAXPAGE; x++)
            printapage(outfile, x, maxlines);
    } else {
        printapage(outfile, printpage, maxlines);
    }

    fclose(outfile);
    printf("Information printing completed.\n");
    return 0;
}
