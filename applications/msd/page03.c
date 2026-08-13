/* PAGE03.C - Memory information (translated from PAGE_03.PAS) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

#define EMMint  0x67

static const char *EMMerrs[] = {
    /* 80h..A4h */
    "internal error in EMM software",
    "malfunction in expanded memory hardware",
    "memory manager busy",
    "invalid handle",
    "undefined function",
    "no more handles available",
    "error in save or restore of mapping context",
    "not enough physical pages available",
    "not enough free pages available",
    "no pages requested",
    "logical page outside range assigned to handle",
    "invalid physical page number",
    "page map hardware state save area full",
    "mapping context already in save area",
    "mapping context not in save area",
    "undefined subfunction parameter",
    "attribute type not defined",
    "feature not supported",
    "src & dest overlap;move done, but source overwritten",
    "length for src or dst longer than allocated",
    "conventional and EMS memory overlap",
    "offset outside logical page",
    "region length >1M",
    "src & dest overlap;not moved",
    "src & dest types undefined",
    "unused error code",
    "Alt map or DMA supported, but specified set isn't",
    "Alt map or DMA supported, but all allocated",
    "Alt map or DMA not supported, specified set <> 0",
    "Alt map or DMA supported, specified set <> 0",
    "Dedicated DMA channels not supported",
    "Dedicated DMA channels supported, but not specified one",
    "No handle found for specified name",
    "handle with same name already exists",
    "???",
    "invalid pointer passed, or contents of source corrupted",
    "access to function denied"
};

void page_memory(void) {
    unsigned int EMMarray[0x400];  /* 1024 words */
    long xlong;
    unsigned xword1, xword2;
    unsigned numhandles;
    char xstring[80];
    unsigned char EMMver;
    int i, j;
    char EMMname[9];
    int isdpmi = 0;
    unsigned char ch2;
    REGS regs;
    SREGS sregs;

    /* ---- Conventional memory ---- */
    Caption2("Total conventional memory (bytes)  ");
    cprintf("%6u (%uK)\r\n", DOSmem, DOSmem / 1024);

    Caption2("Free conventional memory (bytes)   ");
    xlong = DOSmem - ((long)PrefixSeg << 4);
    cprintf("%6ld (%ldK)\r\n", xlong, xlong / 1024);

    /* ---- Extended memory via BIOS ---- */
    Caption2("Extended memory (from BIOS call) ");
    regs.h.ah = 0x88;
    regs.x.cflag = 0;
    int86(0x15, &regs, &regs);
    if (nocarry(&regs))
        cprintf("%8ld (%uK)\r\n", (long)regs.x.ax << 10, regs.x.ax);
    else
        cprintf("     N/A\r\n");

    /* ---- XMS driver ---- */
    Caption2("XMS driver present ");
    regs.x.ax = 0x4300;
    int86(0x2F, &regs, &regs);
    if (regs.h.al != 0x80) {
        cprintf("no\r\n");
    } else {
        cprintf("yes\r\n");
        regs.x.ax = 0x4310;
        int86(0x2F, &regs, &regs);
        xlong = ((long)regs.x.es << 16) + regs.x.bx;

        /* XMS version */
        Caption3("XMS version");
        regs.x.ax = 0;
        LongCall(xlong, &regs);
        if (regs.x.ax != 0) {
            showbcd(regs.x.ax);
            Caption3("XMM version");
            showbcd(regs.x.bx);
        } else {
            cprintf("ERROR");
        }

        /* A20 state */
        Caption3("A20 is");
        regs.x.ax = 0x0700;
        LongCall(xlong, &regs);
        if ((regs.x.ax != 0) || ((regs.x.ax == 0) && (regs.h.bl == 0))) {
            switch (regs.x.ax) {
                case 0: cprintf("disabled"); break;
                case 1: cprintf("enabled"); break;
                default: cprintf("unknown");
            }
        } else cprintf("ERROR");

        /* Total free XMS memory */
        Caption3("Total free XMS memory");
        regs.x.ax = 0x0800;
        LongCall(xlong, &regs);
        if ((regs.x.ax != 0) || ((regs.x.ax == 0) && (regs.h.bl == 0 || regs.h.bl == 0xA0))) {
            cprintf("%uK\r\n", regs.x.dx);
            Caption3("Largest available block");
            cprintf("%uK\r\n", regs.x.ax);
        } else cprintf("ERROR\r\n");

        /* Upper memory blocks */
        Caption3("Upper memory Blocks");
        regs.x.ax = 0x1000;
        regs.x.dx = 1;
        LongCall(xlong, &regs);
        if (regs.x.ax == 0 && regs.h.bl != 0xB1) {
            cprintf("no\r\n");
        } else if (regs.x.ax == 0 && regs.h.bl == 0xB1) {
            cprintf("supported, but none available\r\n");
        } else {
            cprintf("yes");
            Caption3("Largest available size");
            regs.x.ax = 0x1100;
            regs.x.dx = regs.x.bx;   /* предыдущий BX? В оригинале DX:=BX; */
            LongCall(xlong, &regs);
            regs.x.ax = 0x1000;
            regs.x.dx = 0xFFFF;
            LongCall(xlong, &regs);
            cprintf("%lu (%lu.%uK)\r\n",
                    (unsigned long)regs.x.dx * 16,
                    (unsigned long)((regs.x.dx * 16UL) / 1024),
                    (unsigned)((regs.x.dx * 16UL) % 1024 * 10 / 1024)); /* 1 decimal */
        }

        /* HMA */
        regs.x.ax = 0;
        LongCall(xlong, &regs);
        Caption3("HMA");
        YesOrNo2(regs.x.dx == 1);
        regs.x.ax = 0x0100;
        regs.x.dx = 0xFFFF;
        LongCall(xlong, &regs);
        if (regs.x.ax == 0) cprintf(" (in use)");
        else cprintf(" (free)");

        /* DOS in HMA (for MS-DOS 5+) */
        if (OSMajor >= 5 && OSMajor < 10) {
            Caption3("Used by DOS");
            regs.x.ax = 0x4A01;
            regs.x.bx = 0;
            regs.x.es = 0;
            regs.x.di = 0;
            int86(0x2F, &regs, &regs);
            YesOrNo2(regs.x.bx != 0);
            if (regs.x.bx != 0) {
                Caption3("bytes free");
                cprintf("%u", regs.x.bx);
                Caption3("at");
                SegOfs(regs.x.es, regs.x.di);
            }
        }
        cprintf("\r\n");
    }

    /* ---- DPMI driver ---- */
    isdpmi = 0;
    Caption2("DPMI driver present");
    regs.x.ax = 0x1687;
    int86(0x2F, &regs, &regs);
    if (regs.x.ax != 0) {
        cprintf("no\r\n");
    } else {
        cprintf("yes\r\n");
        isdpmi = 1;
        Caption3("version");
        cprintf("%u.%u\r\n", regs.h.dh, regs.h.dl);
        Caption3("CPU");
        switch (regs.h.cl) {
            case 2: cprintf("286"); break;
            case 3: cprintf("386"); break;
            case 4: cprintf("486"); break;
            case 5: cprintf("P5"); break;
            default: cprintf("???");
        }
        Caption3("switch mode entry");
        SegOfs(regs.x.es, regs.x.di);
        cprintf("\r\n");
    }

    pause3(-12);
    if (endit) return;

    /* ---- Expanded memory (EMS) ---- */
    Caption2("Expanded memory");
    if (FP_SEG(intvec[EMMint]) != 0) {
        cprintf("\r\n");
        Caption3("Interrupt vector");
        xlong = (long)intvec[EMMint];
        xword1 = xlong >> 16;
        xword2 = xlong & 0xFFFF;
        SegOfs(xword1, xword2);
        cprintf("\r\n");

        Caption3("Driver");
        xstring[0] = '\0';
        for (i = 0x000A; i <= 0x0011; i++) {
            char c = peekb(xword1, i);
            sprintf(xstring + strlen(xstring), "%c", showchar(c));
        }
        cprintf("%s", xstring);

        if (strcmp(xstring, "EMMXXXX0") == 0) {
            Caption3("status");
            regs.h.ah = 0x40;
            int86(EMMint, &regs, &regs);
            if (regs.h.ah == 0) cprintf("available");
            else cprintf("%s", EMMerrs[regs.h.ah - 0x80]);

            Caption3("version");
            regs.h.ah = 0x46;
            int86(EMMint, &regs, &regs);
            if (regs.h.ah == 0)
                cprintf("%u.%u\r\n", regs.h.al >> 4, regs.h.al & 0x0F);
            else
                cprintf("%s", EMMerrs[regs.h.ah - 0x80]);
            EMMver = regs.h.al >> 4;

            Caption3("Page frame segment");
            regs.h.ah = 0x41;
            int86(EMMint, &regs, &regs);
            if (regs.h.ah == 0)
                cprintf("%04X\r\n", regs.x.bx);
            else
                cprintf("%s", EMMerrs[regs.h.ah - 0x80]);

            Caption3("Total EMS memory");
            regs.h.ah = 0x42;
            int86(EMMint, &regs, &regs);
            if (regs.h.ah == 0) {
                cprintf("%luK", (long)regs.x.dx * 16);
                Caption3("available");
                if (regs.h.ah == 0)
                    cprintf("%luK\r\n", (long)regs.x.bx * 16);
                else
                    cprintf("%s", EMMerrs[regs.h.ah - 0x80]);
            } else
                cprintf("%s", EMMerrs[regs.h.ah - 0x80]);

            /* VCPI detection (EMM 4+) */
            if (EMMver >= 4) {
                Caption3("VCPI capable");
                regs.x.ax = 0x1600;
                int86(0x2F, &regs, &regs);
                if (regs.h.al == 0 || regs.h.al == 1 || regs.h.al == 0x80 || regs.h.al == 0xFF) {
                    /* allocate 1 page for test */
                    regs.h.ah = 0x43;
                    regs.x.bx = 1;
                    int86(EMMint, &regs, &regs);
                    if (regs.h.ah != 0) {
                        cprintf("error: need 16K available EMS to detect\r\n");
                    } else {
                        xword1 = regs.x.dx;  /* handle */
                        regs.x.ax = 0xDE00;
                        int86(EMMint, &regs, &regs);
                        if (regs.h.ah != 0) {
                            cprintf("no\r\n");
                        } else {
                            cprintf("yes");
                            Caption3("VCPI version");
                            cprintf("%u.%u\r\n", regs.h.bh, regs.h.bl);
                        }
                        /* release handle */
                        regs.h.ah = 0x45;
                        regs.x.dx = xword1;
                        int86(EMMint, &regs, &regs);
                    }
                } else {
                    cprintf("no\r\n");
                }
            }

            /* Enumerate EMS handles */
            Caption1("  Handle   Size  Name\r\n");
            regs.h.ah = 0x4D;
            regs.x.es = FP_SEG(EMMarray);
            regs.x.di = FP_OFF(EMMarray);
            int86(EMMint, &regs, &regs);
            if (regs.h.ah == 0) {
                if (regs.x.bx > 0) {
                    Window(3, wherey() + 1, twidth, tlength - 2);
                    numhandles = regs.x.bx;
                    for (i = 1; i <= numhandles; i++) {
                        pause2();
                        if (endit) break;
                        long size = (long)EMMarray[2 * i - 1] * 16;
                        if (size > 0) {
                            cprintf("%04X   %5luK  ", EMMarray[2 * i - 2], size);
                            if (EMMver >= 4) {
                                regs.x.ax = 0x5300;
                                regs.x.dx = EMMarray[2 * i - 2];
                                regs.x.es = FP_SEG(EMMname);
                                regs.x.di = FP_OFF(EMMname);
                                int86(EMMint, &regs, &regs);
                                if (regs.h.ah == 0) {
                                    for (j = 0; j < 8; j++) {
                                        if (EMMname[j] != 0)
                                            cprintf("%c", EMMname[j]);
                                    }
                                }
                            }
                            cprintf("\r\n");
                        }
                    }
                } else {
                    cprintf("  (no active handles)\r\n");
                }
            } else {
                cprintf("%s", EMMerrs[regs.h.ah - 0x80]);
            }
        } else {
            cprintf("\r\n");
            dontknow();
        }
    } else {
        cprintf("(none)\r\n");
    }
}
