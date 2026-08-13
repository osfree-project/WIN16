/* PAGE20.C Ц QEMM memory manager statistics and memory map */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

/* внешние переменные (объ€влены в msd.h) */
extern int EMSOK;
extern unsigned WindMin;
extern unsigned twidth;

/* прототипы локальных функций */
static void ShowSize(unsigned long val);
static void ShowQEMMinfo(unsigned APISeg, unsigned APIOfs, unsigned char *buffer);

/* ====================================================================== */
static void ShowSize(unsigned long val)
{
    cprintf("%8lu (%7.1fK)", val, val / 1024.0);
}

/* ---------------------------------------------------------------------- */
static void ShowQEMMinfo(unsigned APISeg, unsigned APIOfs, unsigned char *buffer)
{
    static const char QEMMMemType[] = "m?MHXVRAsFrC";
    static const char QEMMMemAccess[] = "-+*!";
    static const char QEMMStealth[] = " S";

    typedef struct {
        unsigned StartingSeg;
        unsigned ParaSize;
    } TStealthRec;

    typedef struct {
        unsigned char  ExtraMemType;
        unsigned long  InitConvMem;
        unsigned long  InitExtMem;
        unsigned long  InitExpMem;
        unsigned long  InitExtraMem;
        unsigned long  UnAvailConv;
        unsigned long  UnAvailExt;
        unsigned long  UnAvailExp;
        unsigned long  UnAvailExtra1;
        unsigned long  CodeSize;
        unsigned long  DataSize;
        unsigned long  TaskSize;
        unsigned long  DMASize;
        unsigned long  MAPSize;
        unsigned long  HiRAMSize;
        unsigned long  MappedROMSize;
        unsigned long  ConvMemSize;
        unsigned long  ExtMemSize;
        unsigned long  EMSXMSMemSize;
        unsigned long  UnAvailExtra2;
        unsigned long  ConvOverhead;
    } TStatRec;

    unsigned long API = ((unsigned long)APISeg << 16) + APIOfs;
    union REGS r;
    unsigned QEMMVersion;
    TStatRec QEMMStat;
    TStealthRec StealthBuf[64];
    unsigned BX_val;
    unsigned long InitMem, CurrentMem;
    int LineNo, BufferPos, StealthCount, StealthStart, StealthSize, StealthSet;

    Caption2("Memory Manager");
    cprintf("QEMM");
    Caption3("Version");
    r.h.ah = 3;
    LongCall(API, &r);
    cprintf("%u.%02u", UnBCD(r.h.bh), UnBCD(r.h.bl));
    QEMMVersion = r.x.bx;
    Caption3("Mode");
    r.h.ah = 0;
    LongCall(API, &r);
    if (r.h.al & 2) {
        cprintf("Auto");
        Caption3("Current Setting");
    }
    if (r.h.al & 1)
        cprintf("OFF\r\n");
    else
        cprintf("ON\r\n");
    Caption3("API Entry");
    SegOfs(APISeg, APIOfs);
    cprintf("\r\n");

    if ((QEMMVersion >> 8) >= 6) {
        Caption3("Stealth");
        r.h.ah = 0x1E; r.h.al = 0;
        LongCall(API, &r);
        switch (r.h.cl) {
            case 0:   cprintf("OFF\r\n"); break;
            case 0x46: cprintf("Frame\r\n"); break;
            case 0x4D: cprintf("Map\r\n"); break;
            default:   cprintf("Unknown value - %02Xh\r\n", r.h.cl);
        }
        if (r.h.cl == 0x46 || r.h.cl == 0x4D) {
            Caption3("Number of ROMs Stealthed");
            r.h.ah = 0x1E; r.h.al = 1;
            LongCall(API, &r);
            if (nocarry(&r))
                cprintf("%u\r\n", r.x.bx);
            else
                cprintf("Error\r\n");
        }
    }

    r.h.ah = 0x17;
    r.x.es = FP_SEG(&QEMMStat);
    r.x.di = FP_OFF(&QEMMStat);
    QEMMStat.ConvOverhead = 0;
    LongCall(API, &r);
    if (!nocarry(&r)) {
        cprintf("Unable to retrieve QEMM Statistics!\r\n");
        cprintf("This information may only be available with 5.00 or newer.\r\n");
    } else {
        cprintf("\r\n");
        Caption1("---Initial Memory Settings---\r\n");
        Caption3("Conventional");
        ShowSize(QEMMStat.InitConvMem);
        Caption3("  Extended");
        ShowSize(QEMMStat.InitExtMem);
        cprintf("\r\n");
        Caption3("    Expanded");
        ShowSize(QEMMStat.InitExpMem);
        if (QEMMStat.ExtraMemType == 1)
            Caption3("    Shadow");
        else
            Caption3(" Top/Other");
        ShowSize(QEMMStat.InitExtraMem);
        cprintf("\r\n");
        Caption3("       Total");
        InitMem = QEMMStat.InitConvMem + QEMMStat.InitExtMem + QEMMStat.InitExtraMem;
        ShowSize(InitMem);
        cprintf("\r\n\r\n");

        Caption1("---Current Memory Settings---\r\n");
        Caption3("Conventional");
        ShowSize(QEMMStat.ConvMemSize);
        Caption3("  Extended");
        ShowSize(QEMMStat.ExtMemSize);
        cprintf("\r\n");
        Caption3("EMS/XMS Pool");
        ShowSize(QEMMStat.EMSXMSMemSize);
        Caption3("  High RAM");
        ShowSize(QEMMStat.HiRAMSize);
        cprintf("\r\n");
        Caption3("  Mapped ROM");
        ShowSize(QEMMStat.MappedROMSize);
        Caption3("DMA Buffer");
        ShowSize(QEMMStat.DMASize);
        cprintf("\r\n");
        Caption3("       TASKS");
        ShowSize(QEMMStat.TaskSize);
        Caption3("      MAPS");
        ShowSize(QEMMStat.MAPSize);
        cprintf("\r\n");
        Caption3("   QEMM code");
        ShowSize(QEMMStat.CodeSize);
        Caption3(" QEMM data");
        ShowSize(QEMMStat.DataSize);
        cprintf("\r\n");
        CurrentMem = QEMMStat.CodeSize + QEMMStat.DataSize + QEMMStat.TaskSize + QEMMStat.DMASize +
                     QEMMStat.MAPSize + QEMMStat.HiRAMSize + QEMMStat.MappedROMSize +
                     QEMMStat.ConvMemSize + QEMMStat.ExtMemSize + QEMMStat.EMSXMSMemSize;
        Caption3("       Total");
        ShowSize(CurrentMem);
        Caption3("Unassigned");
        ShowSize(InitMem - CurrentMem);
        cprintf("\r\n");
        Caption3("                 Conventional Memory Overhead");
        ShowSize(QEMMStat.ConvOverhead);
        cprintf("\r\n\r\n");

        Caption1("---Unavailable Memory Settings---\r\n");
        Caption3("Conventional");
        ShowSize(QEMMStat.UnAvailConv);
        Caption3("  Extended");
        ShowSize(QEMMStat.UnAvailExt);
        cprintf("\r\n");
        Caption3("    Expanded");
        ShowSize(QEMMStat.UnAvailExp);
        Caption3("Shadow/Top");
        ShowSize(QEMMStat.UnAvailExtra1 + QEMMStat.UnAvailExtra2);
        cprintf("\r\n");
    }

    /*  арта пам€ти */
    r.h.ah = 0x11; r.h.al = 0;
    r.x.es = FP_SEG(buffer);
    r.x.di = FP_OFF(buffer);
    LongCall(API, &r);
    if (!nocarry(&r)) {
        cprintf("Unable to get Memory type map.\r\n");
    } else {
        Pause1();
        if (endit) return;
        ClrScr();
        TextColor(LightGray);
        cprintf("                 Memory Type/Memory Access/Stealth Info\r\n");
        for (LineNo = 0; LineNo <= 0x0F; LineNo++) {
            GotoXY(1, 17 - LineNo);
            cprintf("%1Xx00", LineNo);
        }
        GotoXY(1, 18);
        cprintf("      0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F\r\n");
        cprintf("m=mappable RAM, M=Mapped ROM, H=High RAM, X=eXcluded, V=Video, R=ROM\r\n");
        cprintf("A=Adapter ROM, s=split ROM, F=EMS Page Frame, r=RAMmable, C=Conventional\r\n");
        cprintf("-=Not Accessed, +=Read, *=Written, !=Read/Written, S=Stealth ROM");
        TextColor(LightCyan);
        {
            unsigned char x = WindMin & 0xFF;
            unsigned char y = WindMin >> 8;
            Window(x + 6, y + 2, twidth, y + 18);
        }
        if (EMSOK) {
            r.h.ah = 0x41;
            int86(0x67, &r, &r);
            if (r.h.ah == 0) {
                unsigned start = r.x.bx / 256;
                int i;
                for (i = start; i <= start + 15; i++)
                    buffer[i] = 9;
            }
        }
        /* Memory type */
        for (BufferPos = 0; BufferPos < 256; BufferPos++) {
            GotoXY(4 * (BufferPos % 16) + 1, 16 - (BufferPos / 16));
            if (buffer[BufferPos] > 0x0B)
                cprintf("?");
            else
                cprintf("%c", QEMMMemType[buffer[BufferPos]]);
        }
        /* Memory access */
        r.h.ah = 0x16; r.h.al = 0;
        r.x.es = FP_SEG(buffer);
        r.x.di = FP_OFF(buffer);
        LongCall(API, &r);
        if (!nocarry(&r))
            memset(buffer, 0xFF, 256);
        for (BufferPos = 0; BufferPos < 256; BufferPos++) {
            GotoXY(4 * (BufferPos % 16) + 2, 16 - (BufferPos / 16));
            if (buffer[BufferPos] > 3)
                cprintf("?");
            else
                cprintf("%c", QEMMMemAccess[buffer[BufferPos]]);
        }
        /* Stealth */
        memset(buffer, 0, 256);
        r.h.ah = 0x1E; r.h.al = 1;
        LongCall(API, &r);
        BX_val = r.x.bx;
        if (BX_val > 0 && BX_val < 65) {
            r.h.ah = 0x1E; r.h.al = 2;
            r.x.es = FP_SEG(StealthBuf);
            r.x.di = FP_OFF(StealthBuf);
            LongCall(API, &r);
            if (nocarry(&r)) {
                for (StealthCount = 1; StealthCount <= BX_val; StealthCount++) {
                    StealthSize = StealthBuf[StealthCount-1].ParaSize / 256;
                    StealthStart = StealthBuf[StealthCount-1].StartingSeg / 256;
                    for (StealthSet = StealthStart; StealthSet < StealthStart + StealthSize; StealthSet++)
                        if (StealthSet < 256)
                            buffer[StealthSet] = 1;
                }
            }
        }
        for (BufferPos = 0; BufferPos < 256; BufferPos++) {
            GotoXY(4 * (BufferPos % 16) + 3, 16 - (BufferPos / 16));
            if (buffer[BufferPos] > 1)
                cprintf("?");
            else
                cprintf("%c", QEMMStealth[buffer[BufferPos]]);
        }
    }
}

/* ====================================================================== */
void page20(void)
{
    unsigned char buffer[256];
    char fname[] = "QEMM386$";
    union REGS r;
    struct SREGS s;
    unsigned handle;

    r.x.ax = 0x3D00;
    s.ds = FP_SEG(fname);
    r.x.dx = FP_OFF(fname) + 1;   /* пропуск нул€ в начале Pascal-строки */
    int86x(0x21, &r, &r, &s);
    if (!nocarry(&r)) {
        cprintf("Currently, only QEMM 4.23 or newer is supported. It was not found.\r\n");
    } else {
        handle = r.x.ax;
        r.x.ax = 0x4402;
        r.x.bx = handle;
        r.x.cx = 4;
        s.ds = FP_SEG(buffer);
        r.x.dx = FP_OFF(buffer);
        int86x(0x21, &r, &r, &s);
        if (!nocarry(&r)) {
            if (EMSOK) {
                r.h.ah = 0x3F;
                r.x.cx = 0x5145;   /* 'QE' */
                r.x.dx = 0x4D4D;   /* 'MM' */
                int86(0x67, &r, &r);
                if (r.h.ah == 0)
                    ShowQEMMinfo(r.x.es, r.x.di, buffer);
                else {
                    cprintf("QEMM possibly found, but both the IOCTL and special EMS calls failed.\r\n");
                    cprintf("INFOPLUS is unable to find the API entry point.\r\n");
                }
            } else {
                cprintf("QEMM possibly found, but the IOCTL call failed. No EMS memory is being\r\n");
                cprintf("provided, preventing a secondary attempt to find the API entry point.\r\n");
            }
        } else {
            unsigned api_seg = ((unsigned)buffer[3] << 8) | buffer[2];
            unsigned api_ofs = ((unsigned)buffer[1] << 8) | buffer[0];
            ShowQEMMinfo(api_seg, api_ofs, buffer);
        }
        r.h.ah = 0x3E;
        r.x.bx = handle;
        int86(0x21, &r, &r);
    }
}
