/* PAGE18.C Ц TSR Utilities (complete translation of page_18.pas, full version) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include "msd.h"

/* ¬нешние переменные (должны быть объ€влены в msd.h) */
extern int EMSOK;            /* признак наличи€ EMS */
extern int osmajor, osminor; /* верси€ DOS */

/* Ћокальные прототипы секций */
static void Shells(void);
static void DosExtenders(void);
static void MemUtils(void);
static void MultiTaskers(void);
static void NortonUtils(void);
static void VirusUtils(void);
static void SCSI(void);
static void DiskCaches(void);
static void DiskCompress(void);
static void MiscUtils(void);

/* ¬спомогательные локальные функции */
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

static void YesOrNo3(int cond)
{
    cprintf(cond ? "yes" : "no");
}

static void ZeroPad3(unsigned char val)
{
    cprintf("%02u", val);
}

static void nortonstatus(unsigned char b)
{
    if (b == 0) cprintf("disabled");
    else if (b == 1) cprintf("enabled");
    else cprintf("unknown");
}

/* ======================================================================== */
void page18(void)
{
    Shells();
    if (endit) return;
    DosExtenders();
    if (endit) return;
    MemUtils();
    if (endit) return;
    MultiTaskers();
    if (endit) return;
    NortonUtils();
    if (endit) return;
    VirusUtils();
    if (endit) return;
    SCSI();
    if (endit) return;
    DiskCaches();
    if (endit) return;
    DiskCompress();
    if (endit) return;
    MiscUtils();
}

/* ------------------------------------------------------------------------- */
static void Shells(void)
{
    Pause3(-1); if (endit) return;
    Caption1("----Shells and Shell enhancers----\r\n");

    Caption2("JP Software 4DOS");
    {
        union REGS r;
        r.x.ax = 0xD44D; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
        int86(0x2F, &r, &r);
        if (r.x.ax != 0x44DD) {
            cprintf("no\r\n");
        } else {
            cprintf("yes\r\n");
            Caption3("version");
            cprintf("%u.%02u", r.h.bl, r.h.bh);
            Caption3("shell no.");
            cprintf("%u", r.h.dl);
            Caption3("PSP segment");
            cprintf("%04X\r\n", r.x.cx);
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("JP Software KSTACK.COM");
    {
        union REGS r;
        r.x.ax = 0xD44F; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
        int86(0x2F, &r, &r);
        YesOrNo(r.x.ax == 0x44DD);
    }

    Pause3(-2); if (endit) return;
    Caption2("Norton NDOS");
    {
        union REGS r;
        r.x.ax = 0xE44D; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
        int86(0x2F, &r, &r);
        if (r.x.ax != 0x44EE) {
            cprintf("no\r\n");
        } else {
            cprintf("yes\r\n");
            Caption3("version");
            cprintf("%u.%02u", r.h.bl, r.h.bh);
            Caption3("shell no.");
            cprintf("%u", r.h.dl);
            Caption3("PSP segment");
            cprintf("%04X\r\n", r.x.cx);
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("WildUnix");
    {
        union REGS r;
        r.h.ah = 0x4E; r.x.ds = 0; r.x.dx = 0;
        int86(0x21, &r, &r);
        YesOrNo(r.h.ah == 0x99);
    }

    Pause3(-1); if (endit) return;
    Caption2("Anarkey");
    {
        union REGS r;
        r.x.ax = 0xE300;
        int86(0x2F, &r, &r);
        switch (r.h.al) {
            case 0x00: cprintf("no\r\n"); break;
            case 0xFE: cprintf("yes; but suspended\r\n"); break;
            case 0xFF: cprintf("yes; and active\r\n"); break;
            default: cprintf("???\r\n");
        }
    }
}

/* ------------------------------------------------------------------------- */
static void DosExtenders(void)
{
    Pause3(-1); if (endit) return;
    Caption1("----DOS Extenders----\r\n");

    Caption2("DOS/16M");
    {
        union REGS r;
        r.x.ax = 0xBF02; r.x.dx = 0;
        int86(0x15, &r, &r);
        YesOrNo(r.x.dx != 0);
    }

    Pause3(-4); if (endit) return;
    Caption2("Phar Lap DOS Extender");
    {
        int found = 0;
        unsigned char xbyte = 1;
        union REGS r;
        do {
            r.x.ax = 0xED00; r.h.bl = xbyte;
            int86(0x2F, &r, &r);
            if (r.h.al == 0xFF && r.x.si == 0x5048 && r.x.di == 0x4152) {
                if (!found) cprintf("yes");
                found = 1;
                Caption3("type");
                switch (xbyte) {
                    case 1: cprintf("286dosx v1.3+ SDK"); break;
                    case 2: cprintf("286dosx v1.3+ RTK"); break;
                    case 3: cprintf("386dosx v4.0+ SDK"); break;
                    case 4: cprintf("386dosx v4.0+ RTK"); break;
                }
                Caption3("version");
                cprintf("%u.%02u", r.h.ch, r.h.cl);
            }
            xbyte++;
        } while (xbyte <= 4);
        if (!found) cprintf("no\r\n");
    }
}

/* ------------------------------------------------------------------------- */
static void MemUtils(void)
{
    typedef struct {
        unsigned char  version;
        char           signature[6];
        char           verstr[4];
        unsigned       lowseg;
        unsigned       unkw1, unkw2;
        unsigned       flags1;
        unsigned char  unk1[16];
        unsigned       int15port, int67port;
        unsigned       unkw3, unkw4;
        long           unkd1, unkd2;
        unsigned       sysconfig;
        unsigned char  unk2[8];
        unsigned       flags2, flags3, flags4;
        unsigned       unkw5;
        unsigned       extfree;
        long           unkd3;
        unsigned       unkw6;
        long           unkd4;
        unsigned       flags5;
        unsigned       oldint21ofs, oldint21seg;
        unsigned       emsofs, emsseg;
        unsigned char  extra;
    } T386maxbuf;

    Pause3(-3); if (endit) return;
    Caption1("----Memory Managers and Memory utilities----\r\n");

    /* --- QEMM --- */
    Caption2("QEMM");
    {
        unsigned char qe_id = 0xD2;
        int foundit = 0;
        unsigned long api_entry = 0;
        unsigned qe_version = 0;
        union REGS r;
        do {
            r.h.ah = qe_id; r.h.al = 0;
            r.x.bx = 0x5144; r.x.cx = 0x4D45; r.x.dx = 0x4D30;
            int86(0x2F, &r, &r);
            if (r.h.al == 0xFF && r.x.bx == 0x4D45 && r.x.cx == 0x4D44 && r.x.dx == 0x5652)
                foundit = 1;
            else {
                if (qe_id < 0xFF) qe_id++;
                else qe_id = 0xC0;
            }
        } while (!foundit && qe_id != 0xD2);

        if (!foundit) {
            cprintf("no\r\n");
        } else {
            r.h.ah = qe_id; r.h.al = 1;
            r.x.bx = 0x5145; r.x.cx = 0x4D4D; r.x.dx = 0x3432;
            int86(0x2F, &r, &r);
            if (r.x.bx == 0x4F4B) {
                cprintf("yes");
                Caption3("API entry");
                SegOfs(r.x.es, r.x.di);
                api_entry = ((unsigned long)r.x.es << 16) + r.x.di;
                Caption3("version");
                r.h.ah = 3;
                LongCall(api_entry, &r);
                if (!nocarry(&r)) cprintf("error");
                else cprintf("%u.%02u", UnBCD(r.h.bh), UnBCD(r.h.bl));
                qe_version = r.x.bx;
                Caption3("status");
                r.h.ah = 0;
                LongCall(api_entry, &r);
                if (!nocarry(&r)) cprintf("error");
                else if (r.h.al & 1) cprintf("OFF");
                else if (r.h.al & 2) cprintf("Auto");
                else cprintf("ON");
                cprintf("\r\n");

                Caption3("High RAM");
                r.h.ah = 0x12;
                LongCall(api_entry, &r);
                if (!nocarry(&r)) cprintf("error");
                else {
                    YesOrNo2(r.x.bx != 0);
                    if (r.x.bx != 0) {
                        Caption3("first MCB at");
                        cprintf("%04X", r.x.bx);
                    }
                }

                if ((qe_version >> 8) >= 6) {
                    Caption3("Stealth");
                    r.x.ax = 0x1E00;
                    LongCall(api_entry, &r);
                    if (!nocarry(&r)) cprintf("error");
                    else {
                        switch (r.h.cl) {
                            case 0:   cprintf("OFF"); break;
                            case 0x46: cprintf("Frame"); break;
                            case 0x4D: cprintf("Map"); break;
                            default:   cprintf("????");
                        }
                        if (r.h.cl == 0x46 || r.h.cl == 0x4D) {
                            Caption3("Stealthed ROMs");
                            r.x.ax = 0x1E01;
                            LongCall(api_entry, &r);
                            if (!nocarry(&r)) cprintf("error");
                            else cprintf("%u", r.x.bx);
                        }
                    }
                }
                cprintf("\r\n");
            } else {
                cprintf("no\r\n");
            }
        }

        /* --- Quarterdeck Manifest --- */
        Pause3(-1); if (endit) return;
        Caption2("Quarterdeck Manifest (memory resident)");
        if (!foundit) {
            cprintf("no\r\n");
        } else {
            r.h.ah = qe_id; r.h.al = 1;
            r.x.bx = 0x4D41; r.x.cx = 0x4E49; r.x.dx = 0x4645;
            int86(0x2F, &r, &r);
            YesOrNo(r.x.bx == 0x5354);
        }

        /* --- Quarterdeck VIDRAM --- */
        Pause3(-3); if (endit) return;
        Caption2("Quarterdeck VIDRAM");
        if (!foundit) {
            cprintf("no\r\n");
        } else {
            r.h.ah = qe_id; r.h.al = 1;
            r.x.bx = 0x5649; r.x.cx = 0x4452; r.x.dx = 0x414D;
            int86(0x2F, &r, &r);
            if (r.x.bx == 0x4F4B) {
                cprintf("yes");
                Caption3("at code segment");
                cprintf("%04X", r.x.es);
                api_entry = ((unsigned long)r.x.es << 16) + r.x.di;
                r.h.ah = 0;
                LongCall(api_entry, &r);
                Caption3("state");
                switch (r.h.al) {
                    case 0: cprintf("off"); break;
                    case 1: cprintf("no EGA"); break;
                    case 2: cprintf("no graphics"); break;
                    default: cprintf("%u???", r.h.al);
                }
                cprintf("\r\n");
                Caption3("extra RAM");
                switch (r.h.bl) {
                    case 0: cprintf("not used"); break;
                    case 1: cprintf("from EGA"); break;
                    case 2: cprintf("from EMS"); break;
                    default: cprintf("%u???", r.h.bl);
                }
                Caption3("override");
                if (r.h.bh & 1) cprintf("enabled"); else cprintf("disabled");
                Caption3("MDA detected");
                YesOrNo(r.h.bh & 8);
                Caption3("monitor type");
                switch (r.h.cl) {
                    case 0:   cprintf("default"); break;
                    case 1:   cprintf("mono"); break;
                    case 0x80: cprintf("color"); break;
                    default:  cprintf("%u???", r.h.cl);
                }
                Caption3("memory top at");
                cprintf("%04X (%uK)\r\n", r.x.si, r.x.si / 64);
            } else {
                cprintf("no\r\n");
            }
        }
    }

    /* --- 386^Max --- */
    Pause3(-2); if (endit) return;
    Caption2("386^Max");
    {
        char fname[] = "386MAX$$";
        union REGS r;
        struct SREGS s;
        unsigned char handle;
        r.x.ax = 0x3D00;
        s.ds = FP_SEG(fname);
        r.x.dx = FP_OFF(fname) + 1;  /* обход начального нул€ */
        int86x(0x21, &r, &r, &s);
        if (!nocarry(&r)) {
            cprintf("no\r\n");
        } else {
            handle = r.x.ax;
            T386maxbuf buf;
            buf.version = 3;
            r.x.ax = 0x4402;
            r.x.bx = handle;
            r.x.cx = sizeof(buf);
            s.ds = FP_SEG(&buf);
            r.x.dx = FP_OFF(&buf);
            int86x(0x21, &r, &r, &s);
            if (!nocarry(&r)) {
                cprintf("Maybe; IOCTL call failed\r\n");
            } else if (strncmp(buf.signature, "386MAX", 6) != 0) {
                cprintf("No; wrong signature found - \"%s\"\r\n", buf.signature);
            } else {
                cprintf("yes");
                Caption3("version");
                cprintf("%c.%c%c\r\n", buf.verstr[0], buf.verstr[2], buf.verstr[3]);
                Caption3("at segment");
                cprintf("%04X\r\n", buf.lowseg);
                Caption3("EMS active");
                YesOrNo2(buf.flags1 & 0x0080);
                Caption3("Windows 3 support");
                YesOrNo(!(buf.flags4 & 1));
            }
            r.h.ah = 0x3E;
            r.x.bx = handle;
            int86(0x21, &r, &r);
        }
    }

    /* --- MICEMM --- */
    Pause3(-1); if (endit) return;
    Caption2("MICEMM");
    if (!EMSOK) {
        cprintf("no\r\n");
    } else {
        union REGS r;
        r.x.ax = 0x58F0;
        int86(0x67, &r, &r);
        if (r.h.ah != 0) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("Code Segment");
            cprintf("%04X\r\n", r.x.bx);
        }
    }

    /* --- EMM386 --- */
    Pause3(-1); if (endit) return;
    Caption2("EMM386");
    if (!EMSOK) {
        cprintf("no\r\n");
    } else {
        union REGS r;
        r.x.ax = 0xFFA5;
        int86(0x67, &r, &r);
        if (r.x.ax != 0x845A) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("API entry");
            SegOfs(r.x.bx, r.x.cx);
            unsigned long api = ((unsigned long)r.x.bx << 16) + r.x.cx;
            Caption3("Status");
            r.h.ah = 0;
            LongCall(api, &r);
            if (r.h.al & 1) cprintf("ON"); else cprintf("OFF");
            Caption3("Weitek");
            r.h.ah = 2; r.h.al = 0;
            LongCall(api, &r);
            if (r.h.al & 1) {
                cprintf("present ");
                if (r.h.al & 2) cprintf("and enabled\r\n");
                else cprintf("but disabled\r\n");
            } else {
                cprintf("not present\r\n");
            }
        }
    }

    /* --- Virtual DMA Spec. (VDS) --- */
    Pause3(-4); if (endit) return;
    Caption2("Virtual DMA Spec. (VDS)");
    {
        union REGS r;
        struct SREGS s;
        r.x.ax = 0x354B;
        int86x(0x21, &r, &r, &s);
        if (r.x.es == 0 && r.x.bx == 0) {
            cprintf("no\r\n");
        } else {
            r.x.ax = 0x8102;
            r.x.dx = 0;
            r.x.cflag = 1;
            int86(0x4B, &r, &r);
            YesOrNo2(nocarry(&r));
            if (nocarry(&r)) {
                Caption3("version");
                cprintf("%u.%02X", r.h.ah, r.h.al);
                Caption3("product");
                switch (r.x.bx) {
                    case 0x0000: cprintf("QMAPS/HPMM"); break;
                    case 0x0001: cprintf("MSDOS EMM386"); break;
                    case 0x0003: cprintf("Windows 3"); break;
                    case 0x0300: cprintf("OS/2"); break;
                    case 0x0EDC: cprintf("DRDOS EMM386"); break;
                    case 0x4560: cprintf("386^Max"); break;
                    case 0x4D53: cprintf("Memory Cmdr"); break;
                    case 0x5145: cprintf("QEMM"); break;
                    case 0x524D: cprintf("Netroom"); break;
                    default:     cprintf("%04X", r.x.bx);
                }
                Caption3("rev.");
                cprintf("%u.%02X", r.h.ch, r.h.cl);
                Caption3("max. DMA buffer size");
                cprintf("%.1fK", ((long)r.x.si * 65536 + r.x.di) / 1024.0);
                Caption3("transfers OK in");
                if (r.x.dx & 1) cprintf("First Meg only"); else cprintf("any address");
                Caption3("buffer in first meg");
                YesOrNo2(r.x.dx & 2);
                Caption3("auto-remap enabled");
                YesOrNo2(r.x.dx & 4);
                Caption3("contiguous memory");
                YesOrNo(r.x.dx & 8);
                Caption3("BIOS Data bit set");
                YesOrNo(peekb(0x40, 0x7B) & 0x20);
            } else {
                cprintf("\r\n");
            }
        }
    }
}

/* ------------------------------------------------------------------------- */
static void MultiTaskers(void)
{
    static const char *winclass[] = {
        "vector plotter", "raster display", "raster printer",
        "raster camera", "character-stream, PLP", "Metafile, VDM",
        "display-file"
    };

    Pause3(-6); if (endit) return;
    Caption1("----Multi-Taskers and Task Switchers + Utilities---\r\n");

    Caption2("Quarterdeck Desqview");
    if (OSMajor >= 10) {
        cprintf("no\r\n");
    } else {
        union REGS r;
        r.x.ax = 0x2B01; r.x.cx = 0x4445; r.x.dx = 0x5351;
        int86(0x21, &r, &r);
        if (r.h.al == 0xFF) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("version");
            if (r.x.bx == 0x0002) {
                cprintf("2.00\r\n");
            } else {
                cprintf("%u.%02u\r\n", r.h.bh, r.h.bl);
            }
            Caption3("window number");
            r.x.ax = 0xDE07;
            int86(0x15, &r, &r);
            cprintf("%u", r.x.ax);
            Caption3("true video mode");
            r.x.ax = 0xDE1E;
            int86(0x15, &r, &r);
            cprintf("%u", r.h.bl);
            Caption3("width");
            cprintf("%u", r.h.ch);
            Caption3("height");
            cprintf("%u\r\n", r.h.cl);
            Caption3("      common memory -> avail");
            r.x.ax = 0xDE04;
            int86(0x15, &r, &r);
            cprintf("%6u", r.x.bx);
            Caption3("largest");
            cprintf("%6u", r.x.cx);
            Caption3("total");
            cprintf("%6u\r\n", r.x.dx);
            Caption3("conventional memory -> avail");
            r.x.ax = 0xDE05;
            int86(0x15, &r, &r);
            cprintf("%5uK", r.x.bx);
            Caption3("largest");
            cprintf("%5uK", r.x.cx);
            Caption3("total");
            cprintf("%5uK\r\n", r.x.dx);
            Caption3("    expanded memory -> avail");
            r.x.ax = 0xDE06;
            int86(0x15, &r, &r);
            cprintf("%5uK", r.x.bx);
            Caption3("largest");
            cprintf("%5uK", r.x.cx);
            Caption3("total");
            cprintf("%5uK\r\n", r.x.dx);
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("DOS 5 task switcher");
    {
        union REGS r;
        r.x.ax = 0x4B02; r.x.bx = 0; r.x.es = 0; r.x.di = 0;
        int86(0x2F, &r, &r);
        if (nocarry(&r) && r.x.ax == 0 && r.x.bx == 0) {
            cprintf("yes");
            Caption3("switcher entry point");
            SegOfs(r.x.es, r.x.di);
            cprintf("\r\n");
        } else {
            cprintf("no\r\n");
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("DRDOS TaskMAX");
    {
        union REGS r;
        r.x.ax = 0x2700; r.x.bx = 0; r.x.cx = 0;
        int86(0x2F, &r, &r);
        if (r.h.al != 0xFF) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("version");
            r.x.ax = 0x2701;
            int86(0x2F, &r, &r);
            cprintf("%u", r.x.dx);
            Caption3("maximum tasks");
            cprintf("%u", r.x.ax);
            Caption3("active tasks");
            cprintf("%u\r\n", r.x.cx);
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("TAME");
    {
        union REGS r;
        r.x.ax = 0x2B01; r.x.cx = 0x5441; r.x.dx = 0x4D45;
        int86(0x21, &r, &r);
        if (r.h.al != 2) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("data area");
            cprintf("%04X:%04X\r\n", r.x.es, r.x.dx);
        }
    }

    Pause3(-6); if (endit) return;
    Caption2("Microsoft Windows");
    {
        union REGS r;
        r.x.ax = 0x1600;
        int86(0x2F, &r, &r);
        switch (r.h.al) {
            case 0x01: case 0xFF:
                cprintf("yes");
                Caption3("version");
                cprintf("Windows/386 2.x\r\n");
                break;
            case 0x00: case 0x80:
                r.x.ax = 0x4680;
                int86(0x2F, &r, &r);
                if (r.x.ax == 0) {
                    cprintf("yes");
                    Caption3("mode");
                    cprintf("Real or Standard\r\n");
                } else {
                    cprintf("no\r\n");
                }
                break;
            case 0x02 ... 0x7F:
            case 0x81 ... 0xFE:
                cprintf("yes");
                Caption3("version");
                cprintf("%u.%u enhanced mode", r.h.al, r.h.ah);
                Caption3("Virtual Machine ID");
                r.x.ax = 0x1683;
                int86(0x2F, &r, &r);
                cprintf("%u", r.x.bx);
                Caption3("WINOLDAP support");
                r.x.ax = 0x1700;
                int86(0x2F, &r, &r);
                if (r.x.ax == 0x1700) {
                    cprintf("no\r\n");
                } else {
                    cprintf("yes");
                    Caption3("version");
                    cprintf("%u.%u\r\n", r.h.al, r.h.ah);
                }
                Caption3("Driver version");
                cprintf("%u.%u", windev(0) >> 8, windev(0) & 0xFF);
                Caption3("Device type");
                cprintf("%s", winclass[windev(2) & 0xFF]);
                Caption3("Pixel width");
                cprintf("%u", windev(8));
                Caption3("height");
                cprintf("%u", windev(0x0A));
                Caption3("colors");
                cprintf("%u", windev(0x18));
                Caption3("bits/pixel");
                cprintf("%u", windev(0x0C));
                Caption3("bit planes");
                cprintf("%u\r\n", windev(0x0E));
                Caption3("X aspect");
                cprintf("%u", windev(0x28));
                Caption3("Y aspect");
                cprintf("%u\r\n", windev(0x2A));
                Caption3("brushes");
                cprintf("%u", windev(0x10));
                Caption3("pens");
                cprintf("%u", windev(0x12));
                Caption3("markers");
                cprintf("%u", windev(0x14));
                Caption3("fonts");
                cprintf("%u\r\n", windev(0x16));
                break;
        }
    }
}

/* ------------------------------------------------------------------------- */
static void NortonUtils(void)
{
    Pause3(-1); if (endit) return;
    Caption1("----Norton Utilities----\r\n");

    Caption2("Norton NCACHE");
    {
        union REGS r;
        r.x.ax = 0xFE00; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
        r.x.di = 0x4E55; r.x.si = 0x4346;   /* 'NU','CF' */
        int86(0x2F, &r, &r);
        if (r.x.si == 0x6366) {              /* 'cf' */
            cprintf("yes (NCACHE-F or NCACHE v6+)");
            Caption3("status");
            nortonstatus(r.h.ah);
            cprintf("\r\n");
        } else {
            r.x.si = 0x4353;                 /* 'CS' */
            int86(0x2F, &r, &r);
            if (r.x.si == 0x6373) {          /* 'cs' */
                cprintf("yes (NCACHE-S)");
                Caption3("status");
                nortonstatus(r.h.ah);
                cprintf("\r\n");
            } else {
                cprintf("no\r\n");
            }
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("Norton Diskreet");
    {
        union REGS r;
        r.x.ax = 0xFE00; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
        r.x.di = 0x4E55; r.x.si = 0x4443;
        int86(0x2F, &r, &r);
        if (r.x.si == 0x6463) {
            cprintf("yes");
            Caption3("status");
            nortonstatus(r.h.ah);
            Caption3("resident at");
            cprintf("%04X\r\n", r.x.cx);
        } else cprintf("no\r\n");
    }

    Pause3(-1); if (endit) return;
    Caption2("Norton DiskMon");
    {
        union REGS r;
        r.x.ax = 0xFE00; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
        r.x.di = 0x4E55; r.x.si = 0x444D;
        int86(0x2F, &r, &r);
        if (r.x.si == 0x646D) {
            cprintf("yes");
            Caption3("status");
            nortonstatus(r.h.ah);
            Caption3("resident at");
            cprintf("%04X\r\n", r.x.cx);
        } else cprintf("no\r\n");
    }

    Pause3(-1); if (endit) return;
    Caption2("Norton FileSave/EraseProtect");
    {
        union REGS r;
        r.x.ax = 0xFE00; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
        r.x.di = 0x4E55; r.x.si = 0x4653;
        int86(0x2F, &r, &r);
        if (r.x.si == 0x6673) {
            cprintf("yes");
            Caption3("resident at");
            cprintf("%04X\r\n", r.x.cx);
        } else cprintf("no\r\n");
    }
}

/* ------------------------------------------------------------------------- */
static void VirusUtils(void)
{
    Pause3(-2); if (endit) return;
    Caption1("----Virus protectors---\r\n");

    Caption2("F-PROT package -> F-LOCK");
    {
        union REGS r;
        r.x.ax = 0x4653; r.x.bx = 0; r.x.cx = 2;
        int86(0x2F, &r, &r);
        YesOrNo2(r.x.ax == 0xFFFF);
        Caption3("F-XCHK");
        r.x.cx = 3;
        int86(0x2F, &r, &r);
        YesOrNo2(r.x.ax == 0xFFFF);
        Caption3("F-POPUP");
        r.x.cx = 4;
        int86(0x2F, &r, &r);
        YesOrNo2(r.x.ax == 0xFFFF);
        Caption3("F-DLOCK");
        r.x.cx = 5;
        int86(0x2F, &r, &r);
        YesOrNo(r.x.ax == 0xFFFF);
    }

    Pause3(-1); if (endit) return;
    Caption2("TBScanX");
    {
        union REGS r;
        r.x.ax = 0xCA00; r.x.bx = 0x5442;
        int86(0x2F, &r, &r);
        if (r.h.al != 0xFF || r.x.bx != 0x7462) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("version");
            r.x.ax = 0xCA01;
            int86(0x2F, &r, &r);
            if (r.h.ah != 0xCA) cprintf("%u.%02u", r.h.ah >> 4, r.h.ah & 0xF);
            else cprintf("2.2-");
            Caption3("status");
            if (r.h.al == 0) cprintf("disabled"); else cprintf("enabled");
            cprintf("\r\n");
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("Flu_Shot+");
    {
        union REGS r;
        r.x.ax = 0xFF0F;
        int86(0x21, &r, &r);
        YesOrNo(r.x.ax == 0x0101);
    }
}

/* ------------------------------------------------------------------------- */
static void SCSI(void)
{
    Pause3(-2); if (endit) return;
    Caption1("----SCSI drivers----\r\n");

    Caption2("Common Access Method SCSI (CAM-SCSI)");
    {
        union REGS r;
        struct SREGS s;
        r.x.ax = 0x354F;
        int86x(0x21, &r, &r, &s);
        if (r.x.es != 0 || r.x.bx != 0) {
            r.x.ax = 0x8200; r.x.cx = 0x8765; r.x.dx = 0xCBA9;
            int86(0x4F, &r, &r);
            if (r.x.cx == 0x9ABC && r.x.dx == 0x5678) {
                char buf[9];
                int i;
                for (i = 0; i < 8; i++) buf[i] = peekb(r.x.es, r.x.di + i);
                buf[8] = '\0';
                YesOrNo(strcmp(buf, "SCSI_CAM") == 0);
            } else cprintf("no\r\n");
        } else cprintf("no\r\n");
    }

    Pause3(-1); if (endit) return;
    Caption2("CMC International SCSI driver");
    {
        union REGS r;
        struct SREGS s;
        r.x.ax = 0x3578;
        int86x(0x21, &r, &r, &s);
        char buf[5];
        buf[0] = peekb(r.x.es, r.x.bx + 3);
        buf[1] = peekb(r.x.es, r.x.bx + 4);
        buf[2] = peekb(r.x.es, r.x.bx + 5);
        buf[3] = peekb(r.x.es, r.x.bx + 6);
        buf[4] = '\0';
        YesOrNo(strcmp(buf, "SCSI") == 0);
    }
}

/* ------------------------------------------------------------------------- */
static void DiskCaches(void)
{
    typedef struct {
        unsigned char  write_through;
        unsigned char  write_buffered;
        unsigned char  cache_enabled;
        unsigned char  drivertype;
        unsigned       cticks;
        unsigned char  locked;
        unsigned char  reboot_flush;
        unsigned char  full_track_write;
        unsigned char  buffering_type;
        unsigned       origInt13ofs;
        unsigned       origInt13seg;
        unsigned char  minorversion;
        unsigned char  majorversion;
        unsigned       reserved;
        unsigned       secs_read;
        unsigned       secs_in_cache;
        unsigned       secs_in_trk_buf;
        unsigned char  cache_hitrate;
        unsigned char  track_buf_hitrate;
        unsigned       total_tracks;
        unsigned       tracks_used;
        unsigned       locked_tracks;
        unsigned       dirty_tracks;
        unsigned       current_size;
        unsigned       original_size;
        unsigned       minimum_size;
        unsigned       lock_pointer_ofs;
        unsigned       lock_pointer_seg;
    } smartdrvt;

    Pause3(-4); if (endit) return;
    Caption1("----Disk Caches----\r\n");

    Caption2("SMARTDRV");
    {
        char fname[] = "SMARTAAR";
        union REGS r;
        struct SREGS s;
        r.x.ax = 0x3D00;
        s.ds = FP_SEG(fname);
        r.x.dx = FP_OFF(fname) + 1;
        int86x(0x21, &r, &r, &s);
        if (!nocarry(&r)) {
            r.x.ax = 0x4A10; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
            int86(0x2F, &r, &r);
            if (r.x.ax == 0xBABE) {
                cprintf("yes");
                Caption3("ver");
                cprintf("%u.%02u", UnBCD(r.h.bh), UnBCD(r.h.bl));
                Caption3("size now");
                r.x.ax = 0x4A10; r.x.bx = 4;
                int86(0x2F, &r, &r);
                cprintf("%uK", (r.x.cx * r.x.bx) / 1024);
                Caption3("min size");
                cprintf("%uK", (r.x.dx * r.x.cx) / 1024);
                Caption3("element size");
                cprintf("%uK\r\n", r.x.cx / 1024);
                Caption3("cache hits");
                r.x.ax = 0x4A10; r.x.bx = 0;
                int86(0x2F, &r, &r);
                cprintf("%lu", ((unsigned long)r.x.dx << 16) + r.x.bx);
                Caption3("cache misses");
                cprintf("%lu\r\n", ((unsigned long)r.x.di << 16) + r.x.si);
                int drive;
                for (drive = 0; drive <= 0x19; drive++) {
                    Pause3(-1); if (endit) return;
                    r.x.ax = 0x4A10; r.x.bx = 3; r.x.bp = drive; r.x.dx = 0;
                    int86(0x2F, &r, &r);
                    if (r.h.dl != 0xFF) {
                        Caption3("Drive");
                        cprintf("%c", drive + 'A');
                        Caption3("read");
                        YesOrNo3(!(r.h.dl & 0x80));
                        Caption3("write");
                        YesOrNo3(!(r.h.dl & 0x40));
                        Caption3("buffered");
                        r.x.ax = 0x4A10; r.x.bx = 5; r.x.bp = drive;
                        int86(0x2F, &r, &r);
                        YesOrNo3(r.x.ax == 0xBABE);
                        Caption3("DBLSPACE");
                        r.x.ax = 0x4A11; r.x.bx = 0;
                        int86(0x2F, &r, &r);
                        if (r.x.ax == 0 && r.x.bx == 0x444D) {
                            r.x.ax = 0x4A11; r.x.bx = 1; r.h.dl = drive;
                            int86(0x2F, &r, &r);
                            if (r.x.ax == 0) {
                                YesOrNo3(r.h.bl & 0x80);
                                if (r.h.bl & 0x80) {
                                    cprintf(" %c:\\DBLSPACE.%03u", (r.h.bl & 0x7F) + 'A', r.h.bh);
                                }
                            } else cprintf("(error %04X)", r.x.ax);
                        } else cprintf("no\r\n");
                        cprintf("\r\n");
                    }
                }
            } else {
                cprintf("no\r\n");
            }
        } else {
            unsigned char handle = r.x.ax;
            r.x.ax = 0x4400;
            r.x.bx = handle;
            int86(0x21, &r, &r);
            if (!nocarry(&r) || (r.x.dx & 0x4080) != 0x4080) {
                cprintf("Maybe. IOCTL interface not supported.\r\n");
            } else {
                smartdrvt buf;
                r.x.ax = 0x4402; r.x.bx = handle; r.x.cx = sizeof(buf);
                s.ds = FP_SEG(&buf); r.x.dx = FP_OFF(&buf);
                int86x(0x21, &r, &r, &s);
                if (!nocarry(&r)) {
                    cprintf("Maybe. IOCTL read failed.\r\n");
                } else {
                    cprintf("yes");
                    Caption3("ver.");
                    cprintf("%u.%u", buf.majorversion, buf.minorversion);
                    Caption3("Size");
                    cprintf("%uK", buf.current_size * 16);
                    Caption3("Max");
                    cprintf("%uK", buf.original_size * 16);
                    Caption3("Min");
                    cprintf("%uK", buf.minimum_size * 16);
                    Caption3("enabled");
                    YesOrNo(buf.cache_enabled == 1);
                    Caption3("locked tracks");
                    YesOrNo2(buf.locked > 0);
                    Caption3("write-through");
                    YesOrNo2(buf.write_through == 1);
                    Caption3("write-buffered");
                    YesOrNo2(buf.write_buffered == 1);
                    Caption3("hit rate");
                    cprintf("%u%%\r\n", buf.cache_hitrate);
                    Caption3("DMA buffering");
                    switch (buf.buffering_type) {
                        case 0: cprintf("off"); break;
                        case 1: cprintf("on"); break;
                        case 2: cprintf("dynamic"); break;
                        default: cprintf("(unknown)");
                    }
                    Caption3("memory type");
                    switch (buf.drivertype) {
                        case 1: cprintf("XMS"); break;
                        case 2: cprintf("EMS"); break;
                        default: cprintf("unknown:%u", buf.drivertype);
                    }
                    Caption3("flush on reboot");
                    YesOrNo(buf.reboot_flush != 0);
                    Caption3("Tracks total");
                    cprintf("%u", buf.total_tracks);
                    Caption3("used");
                    cprintf("%u", buf.tracks_used);
                    Caption3("locked");
                    cprintf("%u", buf.locked_tracks);
                    Caption3("dirty");
                    cprintf("%u\r\n", buf.dirty_tracks);
                }
            }
            r.h.ah = 0x3E; r.x.bx = handle;
            int86(0x21, &r, &r);
        }
    }

    Pause3(-4); if (endit) return;
    Caption2("HyperDisk");
    {
        union REGS r;
        unsigned char id = 0xDF;
        int foundit = 0;
        int xbool1 = 0;
        do {
            r.h.ah = id; r.h.al = 0; r.x.bx = 0x4448;
            int86(0x2F, &r, &r);
            if (r.h.al == 0xFF && r.x.cx == 0x5948) {
                foundit = 1;
            } else {
                if (id == 0xDF && !xbool1) {
                    id = 0xC0; xbool1 = 1;
                } else id++;
            }
        } while (id != 0 && !foundit);
        if (id == 0) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("at code segment");
            cprintf("%04X", r.x.bx);
            Caption3("local data version");
            cprintf("%u.%02u\r\n", r.h.dh, UnBCD(r.h.dl));
            r.h.ah = id; r.h.al = 1; r.x.bx = 0x4448;
            int86(0x2F, &r, &r);
            if (r.x.ax == 0) {
                Caption3("buffers used");
                cprintf("%u", r.x.bx);
                Caption3("buffers modified but not yet written");
                cprintf("%u\r\n", r.x.cx);
                Caption3("   Floppies - cached");
                YesOrNo3(r.h.dl & 0x40);
                Caption3("verified");
                YesOrNo3(r.h.dl & 4);
                Caption3("staged writes");
                YesOrNo(r.h.dl & 1);
                Caption3("Hard Drives - cached");
                YesOrNo3(r.h.dl & 0x80);
                Caption3("verified");
                YesOrNo3(r.h.dl & 8);
                Caption3("staged writes");
                YesOrNo(r.h.dl & 2);
            }
        }
    }
}

/* ------------------------------------------------------------------------- */
static void DiskCompress(void)
{
    Pause3(-4); if (endit) return;
    Caption1("----Disk Compressors----\r\n");

    Caption2("Stacker");
    {
        struct {
            unsigned signature;
            unsigned unknown;
            unsigned ddofs, ddseg;
        } stackerbuf;
        unsigned xword1 = FP_SEG(&stackerbuf), xword2 = FP_OFF(&stackerbuf);
        _asm {
            mov ax, 0xCDCD
            mov cx, 1
            mov dx, 0
            push ds
            push bp
            mov ds, xword1
            mov bx, xword2
            int 0x25
            pop cx
            pop bp
            pop ds
            mov xword1, ax
        }
        if (stackerbuf.signature == 0xCDCD && peekw(stackerbuf.ddseg, stackerbuf.ddofs) == 0xA55A) {
            cprintf("yes");
            Caption3("version");
            cprintf("%5.2f", peekw(stackerbuf.ddseg, stackerbuf.ddofs + 2) / 100.0);
            Caption3("at address");
            SegOfs(stackerbuf.ddseg, stackerbuf.ddofs);
            cprintf("\r\n");
            Caption3("Stacker drive(s)");
            {
                int foundit = 0;
                unsigned char xbyte;
                unsigned func = (OSMajor == 3 && OSMinor == 31) ? 0x440E : 0x4408;
                for (xbyte = 1; xbyte <= 26; xbyte++) {
                    union REGS r;
                    r.x.bx = xbyte; r.x.ax = func;
                    peekb(stackerbuf.ddseg, stackerbuf.ddofs + 0x3E) = 0xFF;
                    int86(0x21, &r, &r);
                    if (peekb(stackerbuf.ddseg, stackerbuf.ddofs + 0x3E) != 0xFF) {
                        cprintf("%c ", xbyte + '@');
                        foundit = 1;
                    }
                }
                if (!foundit) cprintf("(none)");
                cprintf("\r\n");
            }
            Caption3("Swapped");
            {
                char sw[5];
                int i;
                for (i = 0; i < 4; i++) sw[i] = peekb(stackerbuf.ddseg, stackerbuf.ddofs + 0x52 + i);
                sw[4] = '\0';
                if (strcmp(sw, "SWAP") == 0) {
                    int found = 0;
                    int d;
                    for (d = 0; d < 26; d++) {
                        if (peekb(stackerbuf.ddseg, stackerbuf.ddofs + 0x56 + d) != d) {
                            if (found) cprintf(", ");
                            cprintf("%c was %c", d + 'A', peekb(stackerbuf.ddseg, stackerbuf.ddofs + 0x56 + d) + 'A');
                            found = 1;
                        }
                    }
                    if (!found) cprintf("(none)");
                } else cprintf("(none)");
                cprintf("\r\n");
            }
        } else {
            cprintf("no\r\n");
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("DBLSPACE");
    {
        union REGS r;
        r.x.ax = 0x4A11; r.x.bx = 0;
        int86(0x2F, &r, &r);
        if (r.x.ax == 0 && r.x.bx == 0x444D) {
            cprintf("yes");
            Caption3("First drive");
            cprintf("%c", r.h.cl);
            Caption3("Last drive");
            cprintf("%c\r\n", r.h.cl + r.h.ch - 1);
            int xbool1 = 0;
            unsigned char xbyte;
            for (xbyte = 0; xbyte < 26; xbyte++) {
                Pause3(-4); if (endit) return;
                r.x.ax = 0x4A11; r.x.bx = 1; r.h.dl = xbyte;
                int86(0x2F, &r, &r);
                if (r.x.ax == 0 && (r.h.bl & 0x80)) {
                    xbool1 = 1;
                    Caption3("Drive");
                    cprintf("%c", xbyte + 'A');
                    Caption3("Host");
                    cprintf("%c:\\DBLSPACE.%03u", (r.h.bl & 0x7F) + 'A', r.h.bh);
                    r.x.ax = 0x4A11; r.x.bx = 7; r.h.dl = xbyte;
                    int86(0x2F, &r, &r);
                    if (r.x.ax == 0) {
                        unsigned long DSSect = peekw(r.x.ds, r.x.si) + (peekw(r.x.ds, r.x.si + 2) << 16);
                        unsigned long DSFreeSect = peekw(r.x.ds, r.x.si + 4) + (peekw(r.x.ds, r.x.si + 6) << 16);
                        Caption3("sectors");
                        cprintf("%lu", DSSect);
                        Caption3("free sectors");
                        cprintf("%lu", DSFreeSect);
                    }
                    cprintf("\r\n");
                    /* сохранение позиции и расчЄт estimated ratio */
                    int saveX = WhereX(), saveY = WhereY();
                    TextColor(LIGHTRED + 128);
                    cprintf("  *retrieving information*");
                    r.h.ah = 0x1C;
                    r.h.dl = xbyte + 1;
                    int86(0x21, &r, &r);
                    GotoXY(saveX, saveY);
                    cprintf("                          ");
                    GotoXY(saveX, saveY);
                    if (r.h.al != 0xFF) {
                        long DriveSect = (long)r.x.dx * r.h.al;
                        Caption3("  estimated ratio");
                        if (DSSect) cprintf("%5.2f:1", (double)DriveSect / DSSect);
                        r.h.ah = 0x36;
                        r.h.dl = xbyte + 1;
                        int86(0x21, &r, &r);
                        if (r.x.ax != 0xFFFF) {
                            long tot = (long)r.x.dx * r.x.ax;
                            long free = (long)r.x.bx * r.x.ax;
                            long used_on_host = tot - free;
                            long used_on_ds = DSSect - DSFreeSect;
                            if (used_on_ds) cprintf("actual ratio %5.2f:1", (double)used_on_host / used_on_ds);
                        }
                    }
                    cprintf("\r\n");
                    r.x.ax = 0x4A11; r.x.bx = 8; r.h.dl = xbyte;
                    int86(0x2F, &r, &r);
                    if (r.x.ax == 0) {
                        Caption3("  maximum File Fragment heap entries");
                        cprintf("%u", r.x.bx);
                        Caption3("available");
                        cprintf("%u\r\n", r.x.cx);
                    }
                    r.x.ax = 0x4A11; r.x.bx = 3; r.h.cl = xbyte;
                    int86(0x2F, &r, &r);
                    if (r.h.cl != 0xFF) {
                        Caption3("  Driver strategy");
                        SegOfs(r.x.es, r.x.si);
                        Caption3("Driver interrupt");
                        SegOfs(r.x.es, r.x.di);
                    }
                    cprintf("\r\n");
                }
            }
            if (!xbool1) cprintf("No DBLSPACE drives found!\r\n");
        } else {
            cprintf("no\r\n");
        }
    }
}

/* ------------------------------------------------------------------------- */
static void MiscUtils(void)
{
    static const char *pcAstatus[] = {
        [0xFFFC] = "resident and active",
        [0xFFFD] = "resident and not active",
        [0xFFFE] = "memory resident mode",
        [0xFFFF] = "automatic mode"
    };
    static const unsigned pcAspd[] = {
        50, 75, 110, 134, 150, 300, 600, 1200, 1800,
        2000, 2400, 4800, 7200, 9600, 19200, 38400
    };
    static const char *KeyScan[] = {
        /* 000 */ "??", "ESC",  "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",
        /* 010 */  "9",   "0",   "-",   "=",  "BS", "TAB",   "Q",   "W",   "E",   "R",
        /* 020 */  "T",   "Y",   "U",   "I",   "O",   "P",   "[",   "]", "ENT",  "??",
        /* 030 */  "A",   "S",   "D",   "F",   "G",   "H",   "J",   "K",   "L",   ";",
        /* 040 */ "'",   "`",  "??",   "\\",   "Z",   "X",   "C",   "V",   "B",   "N",
        /* 050 */  "M",   ",",   ".",   "/", "kp*",  "??",  "??",  "SP",  "??",  "F1",
        /* 060 */ "F2",  "F3",  "F4",  "F5",  "F6",  "F7",  "F8",  "F9", "F10",  "??",
        /* 070 */ "??", "HOME", "UP","PGUP", "kp-","LEFT", "kp5","RIGHT","kp+", "END",
        /* 080 */"DOWN","PGDN","INS", "DEL",  "F1",  "F2",  "F3",  "F4",  "F5",  "F6",
        /* 090 */ "F7",  "F8",  "F9", "F10",  "F1",  "F2",  "F3",  "F4",  "F5",  "F6",
        /* 100 */ "F7",  "F8",  "F9", "F10",  "F1",  "F2",  "F3",  "F4",  "F5",  "F6",
        /* 110 */ "F7",  "F8",  "F9", "F10",  "??","LEFT","RIGHT","END","PGDN","HOME",
        /* 120 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 130 */ "??",  "??","PGUP", "F11", "F12", "F11", "F12", "F11", "F12", "F11",
        /* 140 */"F12",  "UP", "kp-", "kp5", "kp+","DOWN", "INS", "DEL",  "??", "kp/",
        /* 150 */ "??","HOME",  "UP","PGUP",  "??","LEFT",  "??","RIGHT", "??", "END",
        /* 160 */"DOWN","PGDN", "INS", "DEL", "kp/",  "??","kpENT", "??",  "??",  "??",
        /* 170 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 180 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 190 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 200 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 210 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 220 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 230 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 240 */ "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??",
        /* 250 */ "??",  "??",  "??",  "??",  "??",  "??"
    };

    typedef struct {
        unsigned fbufsize;
        unsigned char spec;
        unsigned char rev;
        unsigned idstrofs, idstrseg;
        unsigned inbufsize, infree;
        unsigned outbufsize, outfree;
        unsigned char scrwidth, scrlen;
        unsigned char baudrate;
        unsigned char extra[13];
    } fossilbuft;

    typedef struct {
        unsigned fbufsize;
        unsigned ver;
        unsigned rev;
        unsigned hifunc;
    } vfossilbuft;

    Pause3(-2); if (endit) return;
    Caption1("----Miscellaneous----\r\n");

    Caption2("pcAnywhere");
    {
        union REGS r;
        r.h.ah = 0x79;
        int86(0x16, &r, &r);
        if (r.x.ax < 0xFFFC) cprintf("no\r\n");
        else {
            cprintf(pcAstatus[r.x.ax]);
            Caption3("port");
            r.h.ah = 0x7C;
            int86(0x16, &r, &r);
            cprintf("%u", r.h.ah);
            Caption3("baud rate");
            cprintf("%u\r\n", pcAspd[r.h.al]);
        }
    }

    Pause3(-5); if (endit) return;
    Caption2("Disk Spool II");
    {
        union REGS r;
        r.h.ah = 0xA0;
        int86(0x1A, &r, &r);
        if (r.h.ah == 0xB0) {
            cprintf("yes");
            Caption3("at segment");
            cprintf("%04X\r\n", r.x.es);
            Caption3("spooler is");
            switch (r.h.ch) {
                case 0x00: cprintf("disabled\r\n"); break;
                case 0x41:
                    cprintf("enabled");
                    Caption3("spooling file");
                    {
                        int i = 0;
                        char c;
                        while ((c = peekb(r.x.es, r.x.bx + i)) && i < 64) {
                            putchar(c);
                            i++;
                        }
                        if (i == 0) cprintf("(none)");
                        cprintf("\r\n");
                    }
                    break;
                default: cprintf("??\r\n");
            }
            Caption3("despooler is");
            switch (r.h.cl) {
                case 0x00: cprintf("disabled\r\n"); break;
                case 0x41: {
                    cprintf("enabled and ");
                    switch (r.h.dl) {
                        case 0x00: cprintf("actively printing\r\n"); break;
                        case 0x41: cprintf("standing by\r\n"); break;
                        default: cprintf("?????\r\n");
                    }
                    Caption3("despooler file");
                    {
                        int i = 0;
                        char c;
                        while ((c = peekb(r.x.es, r.x.si + i)) && i < 64) {
                            putchar(c);
                            i++;
                        }
                        if (i == 0) cprintf("(none)");
                        cprintf("\r\n");
                    }
                    break;
                }
                default: cprintf("????\r\n");
            }
        } else cprintf("no\r\n");
    }

    Pause3(-1); if (endit) return;
    Caption2("Microsoft/LANtastic Network");
    {
        union REGS r;
        r.h.ah = 0;
        int86(0x2A, &r, &r);
        YesOrNo(r.h.ah != 0);
    }

    Pause3(-1); if (endit) return;
    Caption2("PC/TCP Packet driver");
    {
        unsigned char xbyte = 0x60;
        int foundit = 0;
        union REGS r;
        struct SREGS s;
        do {
            r.h.ah = 0x35; r.h.al = xbyte;
            int86x(0x21, &r, &r, &s);
            char buf[9];
            int i;
            for (i = 0; i < 8; i++) buf[i] = peekb(r.x.es, r.x.bx + 3 + i);
            buf[8] = '\0';
            if (strcmp(buf, "PKT DRVR") == 0) foundit = 1;
            xbyte++;
        } while (!foundit && xbyte != 0x81);
        if (foundit) cprintf("yes, at interrupt $%02X\r\n", xbyte - 1);
        else cprintf("no\r\n");
    }

    Pause3(-1); if (endit) return;
    Caption2("Inset");
    {
        union REGS r;
        r.h.ah = 2; r.x.dx = 0; r.x.cx = 0x07C3; /* 1987 */
        int86(0x17, &r, &r);
        YesOrNo(r.x.cx == 0x07C2); /* 1986 */
    }

    Pause3(-1); if (endit) return;
    Caption2("Microsoft CD-ROM extensions");
    {
        unsigned char xbyte;
        unsigned xword1;
        _asm {
            mov ax, 0xDADA
            push ax
            mov ax, 0x1100
            int 0x2F
            mov xbyte, al
            pop bx
            mov xword1, bx
        }
        if (xbyte != 0xFF || xword1 != 0xADAD) {
            cprintf("no\r\n");
        } else {
            union REGS r;
            cprintf("yes");
            Caption3("version");
            r.x.ax = 0x150C;
            int86(0x2F, &r, &r);
            if (r.x.bx == 0) cprintf("1.xx\r\n");
            else cprintf("%u.%02u\r\n", r.h.bh, r.h.bl);
        }
    }

    Pause3(-2); if (endit) return;
    Caption2("Fossil");
    {
        int xbool1 = 0;
        union REGS r;
        struct SREGS s;
        r.h.ah = 0xBC; r.x.dx = 0x1954;
        int86(0x11, &r, &r);
        if (r.x.ax == 0x1954) xbool1 = 1;
        r.x.ax = 0x1B00; r.x.dx = 0xFF;
        fossilbuft fossilbuf;
        r.x.cx = sizeof(fossilbuf);
        s.es = FP_SEG(&fossilbuf);
        r.x.di = FP_OFF(&fossilbuf);
        int86x(0x14, &r, &r, &s);
        if (r.x.ax != 0x1B00) {
            cprintf("yes");
            Caption3("type");
            if (xbool1) cprintf("BNU");
            else if (r.x.cx == 0x3058 && r.x.dx == 0x2030) cprintf("X00");
            else cprintf("unknown");
            Caption3("specification level");
            cprintf("%u", fossilbuf.spec);
            Caption3("revision level");
            cprintf("%u\r\n", fossilbuf.rev);
            Caption3("ID string");
            while (peekb(fossilbuf.idstrseg, fossilbuf.idstrofs) != 0) {
                putchar(peekb(fossilbuf.idstrseg, fossilbuf.idstrofs));
                fossilbuf.idstrofs++;
            }
            cprintf("\r\n");
        } else cprintf("no\r\n");
    }

    Pause3(-1); if (endit) return;
    Caption2("Video Fossil");
    {
        union REGS r;
        struct SREGS s;
        vfossilbuft vfossilbuf;
        r.x.ax = 0x8100;
        s.es = FP_SEG(&vfossilbuf);
        r.x.di = FP_OFF(&vfossilbuf);
        int86x(0x14, &r, &r, &s);
        if (r.x.ax != 0x1954) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("version");
            cprintf("%u", vfossilbuf.ver);
            Caption3("revision");
            cprintf("%u", vfossilbuf.rev);
            Caption3("highest function");
            cprintf("$%04X\r\n", vfossilbuf.hifunc);
        }
    }

    Pause3(-3); if (endit) return;
    Caption2("Advanced Power Management Spec.");
    {
        union REGS r;
        r.x.ax = 0x5300; r.x.bx = 0;
        int86(0x15, &r, &r);
        if (!nocarry(&r) || r.x.bx != 0x504D) {
            cprintf("no\r\n");
        } else {
            cprintf("yes");
            Caption3("version");
            cprintf("%u.%02u", UnBCD(r.h.ah), UnBCD(r.h.al));
            Caption3("enabled");
            YesOrNo((r.x.cx & 8) == 0);
            Caption3("supports 16-bit protected mode");
            YesOrNo2((r.x.cx & 1) == 1);
            Caption3("32-bit protected mode");
            YesOrNo((r.x.cx & 2) == 2);
            Caption3("AC line status");
            r.x.ax = 0x530A; r.x.bx = 1;
            int86(0x15, &r, &r);
            if (!nocarry(&r)) cprintf("ERROR");
            else {
                switch (r.h.bh) {
                    case 0: cprintf("off-line"); break;
                    case 1: cprintf("on-line"); break;
                    case 0xFF: cprintf("unknown"); break;
                    default: cprintf("???");
                }
                Caption3("battery status");
                switch (r.h.bl) {
                    case 0: cprintf("high"); break;
                    case 1: cprintf("low"); break;
                    case 2: cprintf("critical"); break;
                    case 3: cprintf("charging"); break;
                    case 0xFF: cprintf("unknown"); break;
                    default: cprintf("???");
                }
                Caption3("remaining life");
                if (r.h.cl == 0xFF) cprintf("unknown");
                else cprintf("%u%%", r.h.cl);
                cprintf("\r\n");
            }
        }
    }

    Pause3(-1); if (endit) return;
    Caption2("Norton Guides");
    {
        union REGS r;
        r.x.ax = 0xF398;
        int86(0x16, &r, &r);
        YesOrNo(r.x.ax == 0x6A73);
    }

    Pause3(-1); if (endit) return;
    Caption2("After Dark for DOS");
    {
        union REGS r;
        r.x.ax = 0xC000; r.x.bx = 0; r.x.cx = 0; r.x.dx = 0;
        int86(0x2F, &r, &r);
        if (r.h.al == 0xFF && r.x.bx == 0x4144 && r.x.cx == 0x2D44 && r.x.dx == 0x4F53) {
            cprintf("yes");
            Caption3("at segment");
            r.x.ax = 0xC001; r.x.es = 0; r.x.cflag = 0;
            int86(0x2F, &r, &r);
            if (nocarry(&r)) cprintf("%04X", r.x.es);
            else cprintf("ERROR");
            Caption3("minutes to wait");
            r.x.ax = 0xC004; r.x.cflag = 1;
            int86(0x2F, &r, &r);
            cprintf("%u\r\n", r.x.bx);
            Caption3("blanker");
            r.x.ax = 0xC006; r.x.cflag = 1;
            int86(0x2F, &r, &r);
            if (r.x.bx == 0) cprintf("disabled"); else cprintf("enabled");
            Caption3("password");
            r.x.ax = 0xC00F; r.x.cflag = 1;
            int86(0x2F, &r, &r);
            if (r.x.bx == 0) cprintf("disabled"); else cprintf("enabled");
            Caption3("hot key");
            r.x.ax = 0xC008; r.x.cflag = 1;
            int86(0x2F, &r, &r);
            if (r.x.ax == 0) {
                if (r.h.cl & 3) cprintf("<SHIFT>");
                if (r.h.cl & 4) cprintf("<CTRL>");
                if (r.h.cl & 8) cprintf("<ALT>");
                cprintf("<%s>\r\n", KeyScan[r.h.bh]);
            }
        } else {
            cprintf("no\r\n");
        }
    }
}
