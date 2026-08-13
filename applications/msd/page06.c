/* PAGE06.C - Full video adapter & chipset identification (from page_06.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <math.h>   /* для pow() */
#include "msd.h"

/* --------------------------------------------------------------------------
   Локальные константы и типы (копия из оригинального модуля)
   -------------------------------------------------------------------------- */
#define EGA      3
#define MCGA     4
#define VGA      5
#define EGAmono  6
#define hercmono 7
#define IBM8514  8
#define ATT400   9
#define PC3270   10

/* Строковые массивы */
static const char *trividmons[] = {
    "MDA", "CGA", "EGA", "Digital multisync", "VGA", "8514",
    "SuperVGA", "Analog multisync"
};
static const char *parachips[] = {
    "?", "PVGA1A", "WD90C00", "WD90C10", "WD90C11"
};
static const char *atividmons[] = {
    "EGA", "analog monochrome", "TTL monochrome", "analog color",
    "RGB color", "Multisync or compatible", "(unknown)",
    "PS/2 8514 or compatible", "Seiko 1430", "MultiSync 2A",
    "Tatung OmniScan", "NEC 3D or compatible", "TVM 3M",
    "NEC MultiSync XL/+/4D/5D", "TVM 2A", "TVM 3A"
};

/* Типы для VESA (должны совпадать с оригинальными размерами) */
typedef struct {
    char     signature[4];
    unsigned version;
    unsigned oemname_off;
    unsigned oemname_seg;
    unsigned char capabilities[4];
    unsigned modes_off;
    unsigned modes_seg;
    unsigned char reserved[238];
} VESA_info;

typedef struct {
    unsigned modeattr;
    unsigned char winaattr;
    unsigned char winbattr;
    unsigned wingran;
    unsigned winsize;
    unsigned winaseg;
    unsigned winbseg;
    unsigned pos_off;
    unsigned pos_seg;
    unsigned scansize;
    unsigned pixwidth;
    unsigned pixheight;
    unsigned char charwidth;
    unsigned char charheight;
    unsigned char memplanes;
    unsigned char pixelbits;
    unsigned char banks;
    unsigned char memmodel;
    unsigned char banksize;
    unsigned char imagepages;
    unsigned char reserved0a;
    unsigned char RedMaskSize;
    unsigned char RedFieldPos;
    unsigned char GrnMaskSize;
    unsigned char GrnFieldPos;
    unsigned char BluMaskSize;
    unsigned char BluFieldPos;
    unsigned char RsrvdMaskSize;
    unsigned char RsrvdMaskPos;
    unsigned char DirectColorInfo;
    unsigned char Reserved0b;
    unsigned char reserved[216];
} VESA_mode_info;

/* --------------------------------------------------------------------------
   Локальные static-функции, реализованные прямо здесь
   -------------------------------------------------------------------------- */

/* Задержка на пустом цикле (аналог for temp:=1 to x do) */
static void delay_io(int loops) {
    int i;
    for (i = 0; i < loops; i++) inportb(0x80);
}

/* cli / sti (встроенный ассемблер) */
static void cli(void) { _asm cli }
static void sti(void) { _asm sti }

/* readROM – копирует length символов из ROM */
static char *readROM(unsigned seg, unsigned ofs, int length) {
    static char buf[256];
    int i;
    for (i = 0; i < length; i++)
        buf[i] = peekb(seg, ofs + i);
    buf[length] = '\0';
    return buf;
}

/* Вывод BCD (showbcd) */
static void showbcd(unsigned x) {
    putchar(((x >> 12) & 0xF) + '0');
    putchar(((x >> 8) & 0xF) + '0');
    putchar('.');
    putchar(((x >> 4) & 0xF) + '0');
    putchar((x & 0xF) + '0');
}

/* checking – временное сообщение */
static void checking(const char *s) {
    int x = wherex(), y = wherey();
    clreol();
    cprintf("Checking for %s", s);
    gotoxy(x, y);
}

/* d8or16bit */
static void d8or16bit(int flag) {
    cprintf(flag ? "8-bit" : "16-bit");
}

/* ATIinfo – чтение регистров ATI */
static unsigned char ATIinfo_internal(unsigned char reg, unsigned base) {
    outportb(base, reg);
    return inportb(base + 1);
}
#define ATIinfo(reg, base) ATIinfo_internal(reg, base)

/* isXGA – возвращает порт POS или 0 */
static unsigned isXGA(void) {
    unsigned posport, cardID;
    unsigned char tmp1, tmp2, tmp3, tmp4;
    int slot = 0, foundit = 0;
    union REGS r;

    r.x.ax = 0xC400;
    r.x.dx = 0xFFFF;
    int86(0x15, &r, &r);
    if (!nocarry(&r) || r.x.dx == 0xFFFF) return 0;
    posport = r.x.dx;

    do {
        cli();
        if (slot == 0) {
            outportb(0x94, 0xDF);
        } else {
            r.x.ax = 0xC401;
            r.x.bx = slot;
            int86(0x15, &r, &r);
        }
        cardID = inportb(posport) | (inportb(posport + 1) << 8);
        tmp1   = inportb(posport + 2);
        tmp2   = inportb(posport + 3);
        tmp3   = inportb(posport + 4);
        tmp4   = inportb(posport + 5);
        if (slot == 0) {
            outportb(0x94, 0xFF);
        } else {
            r.x.ax = 0xC402;
            r.x.bx = slot;
            int86(0x15, &r, &r);
        }
        cli();
        if (cardID >= 0x8FD8 && cardID <= 0x8FDB) {
            unsigned tmpw = (tmp1 & 0x0E) << 3;
            posport = 0x2100 + tmpw;
            outportb(posport + 0x0A, 0x52);
            unsigned char tmp = inportb(posport + 0x0B) & 0x0F;
            if (tmp != 0 && tmp != 0x0F)
                foundit = 1;
            else
                slot++;
        } else {
            slot++;
        }
    } while (!foundit && slot <= 9);

    return foundit ? posport : 0;
}

/* isport2 – проверка порта */
static int isport2(unsigned port) {
    unsigned savebx, saveax, tmp;
    unsigned char al, ah, bh = 0xFF;   /* BH = маска? в оригинале передаётся через regs */
    /* В оригинале isport2 использует регистры; здесь реализована логика для DX=port, AX=0xFF11 */
    union REGS r;
    r.x.dx = port;
    r.x.ax = 0xFF11;
    r.h.bh = 0xFF;  /* предполагается маска */
    /* Сохраняем BX */
    savebx = r.x.bx;
    r.x.bx = r.x.ax;
    outportb(r.x.dx, r.h.al);
    r.h.ah = r.h.al;
    r.h.al = inportb(r.x.dx + 1);
    tmp = r.h.ah; r.h.ah = r.h.al; r.h.al = tmp;
    saveax = r.x.ax;
    r.x.ax = r.x.bx;
    outportb(r.x.dx, r.h.al);
    r.h.ah = r.h.al;
    r.h.al = inportb(r.x.dx + 1);
    r.h.al &= r.h.bh;
    int foundit = (r.h.al == r.h.bh);
    if (foundit) {
        r.h.al = r.h.ah;
        r.h.ah = 0;
        outportb(r.x.dx, r.x.ax);
        outportb(r.x.dx, r.h.al);
        r.h.ah = r.h.al;
        r.h.al = inportb(r.x.dx + 1);
        r.h.al &= r.h.bh;
        foundit = (r.h.al == 0);
    }
    r.x.ax = saveax;
    outportb(r.x.dx, r.x.ax);
    r.x.bx = savebx;
    return foundit;
}

/* captfont */
static void captfont(void) {
    Caption1("Font           Address\r\n");
    cprintf("INT 1FH        ");
    SegOfs(FP_SEG(intvec[0x1F]), FP_OFF(intvec[0x1F]));
    cprintf("\r\n");
}

/* showfont */
static void showfont(int a) {
    static const char *fontnames[] = {
        "INT 1FH     ", "INT 43H     ", "ROM 8x14    ", "ROM 8x8 (lo)",
        "ROM 8x8 (hi)", "ROM 9x14    ", "ROM 8x16    ", "ROM 9x16    "
    };
    if (a < 0 || a > 7) return;
    cprintf("%s   ", fontnames[a]);
    union REGS r;
    r.x.ax = 0x1130;
    r.h.bh = a;
    int86(0x10, &r, &r);
    SegOfs(r.x.es, r.x.bp);
    cprintf("\r\n");
}

/* int101210 – информация EGA/VGA */
static void int101210(void) {
    static const char *memnames[] = { "64K", "128K", "192K", "256K" };
    union REGS r;
    r.h.ah = 0x12;
    r.h.bl = 0x10;
    int86(0x10, &r, &r);
    Caption2("Display type");
    switch (r.h.bh) {
        case 0x00: cprintf("color\r\n"); break;
        case 0x01: cprintf("monochrome\r\n"); break;
        default: unknown("display", r.h.bh, 2);
    }
    Caption2("Memory");
    if (vidmem > 0) {
        cprintf("%uK\r\n", vidmem);
    } else if (r.h.bl < 4) {
        cprintf("%s as determined from standard BIOS call\r\n", memnames[r.h.bl]);
    } else {
        unknown("size", r.h.bl, 2);
    }
    Caption2("Feature bits");
    {
        int i;
        for (i = 3; i >= 0; i--) cprintf("%d", (r.h.ch >> i) & 1);
        cprintf("\r\n");
    }
    Caption2("DIP switches (EGA)");
    {
        int i;
        for (i = 3; i >= 0; i--) cprintf("%d", (r.h.cl >> i) & 1);
        cprintf("\r\n");
    }
}

/* --------------------------------------------------------------------------
   Основная процедура page06
   -------------------------------------------------------------------------- */
void page06(void) {
    /* Локальные переменные */
    unsigned char i, xbyte, xbyte2, xbyte3, paralock1, paralock2;
    unsigned xword1, xword2, xword3, xword4;
    unsigned vidmem = 0;                 /* размер видео-памяти в КБ */
    unsigned char VGAbuf[0x11];          /* буфер для палитры */
    char s[80];
    int foundit, foundone;
    unsigned gusport_save;               /* фиктивная переменная для сохранения порта */

    /* ---- Начало вывода ---- */
    Caption2("Display adapter");

    /* ---------- VESA detection ---------- */
    checking("VESA");
    {
        VESA_info VESAinfo;
        union REGS r;
        r.x.ax = 0x4F00;
        r.x.es = FP_SEG(&VESAinfo);
        r.x.di = FP_OFF(&VESAinfo);
        int86(0x10, &r, &r);
        if (r.h.al == 0x4F && r.h.ah == 0 && strncmp(VESAinfo.signature, "VESA", 4) == 0) {
            clreol();
            cprintf("VESA version %u.%u\r\n", VESAinfo.version >> 8, VESAinfo.version & 0xFF);
            Caption2("OEM ID");
            {
                char far *oem = (char far *)(((unsigned long)VESAinfo.oemname_seg << 16) + VESAinfo.oemname_off);
                s[0] = '\0';
                while (*oem) {
                    putchar(*oem);
                    strncat(s, oem, 1);
                    oem++;
                }
            }
            Caption3("Manufacturer");
            if (strcmp(s, "761295520") == 0) cprintf("ATI\r\n");
            else cprintf("%s\r\n", s);

            Caption1("Video modes supported:\r\n");
            {
                unsigned far *modes = (unsigned far *)(((unsigned long)VESAinfo.modes_seg << 16) + VESAinfo.modes_off);
                while (*modes != 0xFFFF) {
                    xword1 = *modes;
                    pause3(3);
                    if (endit) return;
                    Caption2("Number");
                    cprintf("%04X", xword1);
                    {
                        VESA_mode_info VESAmode;
                        r.x.ax = 0x4F01;
                        r.x.cx = xword1;
                        r.x.es = FP_SEG(&VESAmode);
                        r.x.di = FP_OFF(&VESAmode);
                        int86(0x10, &r, &r);
                        if (r.x.ax == 0x004F && (VESAmode.modeattr & 1)) {
                            Caption3("Mode");
                            if (VESAmode.modeattr & 8) cprintf("Color ");
                            else cprintf("Monochrome ");
                            if (VESAmode.modeattr & 0x10) cprintf("graphics");
                            else cprintf("text");
                            Caption3("BIOS output support");
                            YesOrNo2(VESAmode.modeattr & 4);
                            if (VESAmode.modeattr & 2) {
                                Caption3("Screen size");
                                cprintf("%ux%u", VESAmode.pixwidth, VESAmode.pixheight);
                                Caption3("Character size");
                                cprintf("%ux%u", VESAmode.charwidth, VESAmode.charheight);
                                Caption3("Colors");
                                cprintf("%.0f", pow(2.0, VESAmode.pixelbits));
                                Caption3("Memory model");
                                switch (VESAmode.memmodel) {
                                    case 0: cprintf("Text"); break;
                                    case 1: cprintf("CGA"); break;
                                    case 2: cprintf("Hercules"); break;
                                    case 3: cprintf("4-plane"); break;
                                    case 4: cprintf("Packed-pixel"); break;
                                    case 5: cprintf("Nonchain 4"); break;
                                    case 6: cprintf("direct color"); break;
                                    case 7: cprintf("YUV"); break;
                                    default: cprintf("(unknown)");
                                }
                                Caption3("Memory planes");
                                cprintf("%u", VESAmode.memplanes);
                                Caption3("Memory banks");
                                cprintf("%u", VESAmode.banks);
                                if (VESAmode.banks > 1) {
                                    Caption3("Bank size");
                                    cprintf("%uK", VESAmode.banksize);
                                }
                            }
                        } else {
                            Caption1("  No available information on this mode.\r\n");
                        }
                    }
                    modes++;
                }
            }
            TextColor(LIGHTGREEN);
            cprintf("The next screen will show standard information, so ");
            pause1();
            if (endit) return;
            clrscr();
            Caption2("Display adapter");
        }
    }

    /* ---------- Стандартное определение адаптера ---------- */
    clreol();
    switch (graphdriver) {
        case CGA:
            cprintf("CGA\r\n");
            captfont();
            break;

        case MCGA:
            cprintf("MCGA\r\n");
            captfont();
            showfont(1);
            showfont(3);
            showfont(4);
            showfont(6);
            break;

        case EGA:
        case EGAmono:
            cprintf("EGA\r\n");
            captfont();
            showfont(1);
            showfont(2);
            showfont(3);
            showfont(4);
            showfont(5);
            int101210();
            xbyte = peekb(0x40, 0x87);
            Caption2("Mode change preserves screen buffer");
            YesOrNo2(xbyte & 0x80);
            Caption2("EGA active");
            YesOrNo2((xbyte & 0x08) == 0);
            Caption2("Wait for display enable");
            YesOrNo2(xbyte & 0x04);
            Caption2("CGA cursor emulation");
            YesOrNo2((xbyte & 0x01) == 0);
            Caption2("Save area                    ");
            {
                unsigned seg = peekw(0x40, 0xAA);
                unsigned off = peekw(0x40, 0xA8);
                SegOfs(seg, off);
            }
            cprintf("\r\n");
            {
                unsigned seg = peekw(0x40, 0xAA);
                unsigned off = peekw(0x40, 0xA8);
                Caption2("Video parameter table        ");
                SegOfs(peekw(seg, off + 2), peekw(seg, off));
                cprintf("\r\n");
                Caption2("Dynamic save area            ");
                unsigned dyn_seg = peekw(seg, off + 6);
                unsigned dyn_off = peekw(seg, off + 4);
                if (dyn_seg || dyn_off) {
                    SegOfs(dyn_seg, dyn_off);
                    cprintf("\r\n");
                } else cprintf("(none)\r\n");
                Caption2("Auxiliary character generator");
                unsigned aux_seg = peekw(seg, off + 10);
                unsigned aux_off = peekw(seg, off + 8);
                if (aux_seg || aux_off) {
                    SegOfs(aux_seg, aux_off);
                    cprintf("\r\n");
                } else cprintf("(none)\r\n");
                Caption2("Graphics mode auxiliary table");
                unsigned gr_seg = peekw(seg, off + 14);
                unsigned gr_off = peekw(seg, off + 12);
                if (gr_seg || gr_off) {
                    SegOfs(gr_seg, gr_off);
                } else cprintf("(none)");
                cprintf("\r\n");
            }
            break;

        case hercmono:
            cprintf("Hercules or MDA\r\n");
            captfont();
            break;

        case IBM8514:
            cprintf("IBM 8514\r\n");
            captfont();
            break;

        case ATT400:
            cprintf("AT&T 400\r\n");
            captfont();
            break;

        case VGA:
            /* VGA / XGA */
            if (novgacheck) {
                cprintf("VGA\r\n");
                Caption3("Chipset");
                cprintf("Detection blocked by NV command-line switch!\r\n");
            } else {
                xword1 = isXGA();
                if (xword1 > 0) {
                    if (inportb(xword1) & 1)
                        cprintf("XGA\r\n");
                    else
                        cprintf("VGA, XGA on other monitor\r\n");
                } else
                    cprintf("VGA\r\n");

                vidmem = 0;
                Caption3("Chipset");

                /* --- Проверка Video 7 --- */
                checking("Video 7");
                {
                    unsigned port_base = (inportb(0x3CC) & 1) ? 0x3D0 : 0x3B0;
                    outportb(port_base + 4, 0x0C);
                    i = inportb(port_base + 5);
                    outportb(port_base + 5, 0x55);
                    xbyte = inportb(port_base + 5);
                    outportb(port_base + 4, 0x1F);
                    xbyte2 = inportb(port_base + 5);
                    outportb(port_base + 4, 0x0C);
                    outportb(port_base + 5, i);
                    if (xbyte2 == (0x55 ^ 0xEA)) {
                        clreol();
                        cprintf("Video 7 - ");
                        outportb(0x3C4, 0x8E);
                        xbyte = inportb(0x3C5);
                        switch (xbyte) {
                            case 0x80 ... 0xFF: cprintf("Vega VGA"); break;
                            case 0x70 ... 0x7F: {
                                union REGS rv;
                                rv.x.ax = 0x6F07;
                                int86(0x10, &rv, &rv);
                                if (rv.h.ah & 0x80) cprintf("VRAM"); else cprintf("FastWrite");
                                break;
                            }
                            case 0x50 ... 0x59: cprintf("VGA Version 5"); break;
                            case 0x40 ... 0x49: cprintf("1024i"); break;
                            default: cprintf("unknown value $%02X", xbyte);
                        }
                        Caption3("  Chip revision");
                        outportb(0x3C4, 0x8E);
                        cprintf("%u", inportb(0x3C5));
                        outportb(0x3C4, 0xFF);
                        xbyte = inportb(0x3C5);
                        {
                            union REGS rv;
                            rv.x.ax = 0x6F07;
                            int86(0x10, &rv, &rv);
                            if (rv.h.al == 0x6F) {
                                vidmem = 256 * (rv.h.ah & 0x7F);
                                Caption3("Memory type");
                                if (rv.h.ah & 0x80) cprintf("VRAM"); else cprintf("DRAM");
                            } else vidmem = 256;
                        }
                        Caption3("  Memory");
                        d8or16bit((xbyte & 1) == 0);
                        Caption3("I/O");
                        d8or16bit((xbyte & 2) == 0);
                        Caption3("BIOS");
                        d8or16bit((xbyte & 8) == 0);
                        Caption3("bus");
                        Caption3("Fast-Write");
                        YesOrNo((xbyte & 4) == 4);
                    }
                }

                /* --- Ahead --- */
                if (vidmem == 0) {   /* условие "если ещё не определён чип" */
                    checking("AHEAD");
                    s[0] = '\0';
                    strcpy(s, readROM(0xC000, 0x25, 5));
                    if (strcmp(s, "AHEAD") == 0) {
                        clreol();
                        cprintf("Ahead\r\n");
                        /* (здесь можно добавить детали, в оригинале больше ничего) */
                    }
                }

                /* --- Genoa --- */
                if (vidmem == 0) {
                    checking("Genoa");
                    unsigned off = peekw(0xC000, 0x37);
                    s[0] = readROM(0xC000, off, 4)[0];
                    s[1] = readROM(0xC000, off, 4)[1];
                    s[2] = readROM(0xC000, off, 4)[2];
                    s[3] = readROM(0xC000, off, 4)[3];
                    s[4] = '\0';
                    if ((s[0] == 0x77) && (s[2] == 0x99) && (s[3] == 0x66)) {
                        clreol();
                        cprintf("Genoa ");
                        switch (s[1]) {
                            case 0x33: cprintf("5100/5200 (Tseng ET3000 based)"); break;
                            case 0x55: cprintf("5300/5400 (Tseng ET3000 based)"); break;
                            case 0x22: cprintf("6100"); break;
                            case 0x00: cprintf("6200/6300"); break;
                            case 0x11: cprintf("6400/6600"); break;
                            default: cprintf("(unknown type - $%02X)", s[1]);
                        }
                        cprintf("\r\n");
                        if (s[1] == 0x33 || s[1] == 0x55) {
                            /* Ничего дополнительного */
                        } else {
                            outportb(0x3C4, 5);
                            xbyte = inportb(0x3C5);
                            Caption3("  Bus");
                            if (xbyte & 1) cprintf("PC"); else cprintf("MCA");
                            Caption3("Video width");
                            d8or16bit((xbyte & 2) == 2);
                            Caption3("BIOS width");
                            d8or16bit((xbyte & 4) == 4);
                            Caption3("I/O ports at");
                            if (xbyte & 0x10) cprintf("$3xx"); else cprintf("$2xx");
                            Caption3("  BIOS size");
                            switch ((xbyte & 0x60) >> 5) {
                                case 0: case 3: cprintf("24K"); break;
                                case 1: cprintf("30K"); break;
                                case 2: cprintf("32K"); break;
                            }
                            outportb(0x3C4, 7);
                            xbyte = inportb(0x3C5);
                            Caption3("Monitor type");
                            if (xbyte & 0x20) cprintf("TTL digital"); else cprintf("analog");
                            Caption3("Chipset on");
                            if (xbyte & 8) cprintf("motherboard"); else cprintf("adapter card");
                            outportb(0x3C4, 0x10);
                            xbyte = inportb(0x3C5);
                            Caption3("  Fast scroll");
                            YesOrNo2((xbyte & 1) == 1);
                            Caption3("Fast address");
                            YesOrNo2((xbyte & 2) == 2);
                            Caption3("Fast write");
                            YesOrNo((xbyte & 0x40) == 0x40);
                            outportb(0x3C4, 8);
                            xbyte = inportb(0x3C5);
                            Caption3("  70Hz vertical retrace");
                            YesOrNo2((xbyte & 0x10) == 0x10);
                            unsigned crtport = peekw(0x40, 0x63);
                            outportb(crtport, 0x2F);
                            xbyte = inportb(crtport + 1);
                            Caption3("Interlaced");
                            YesOrNo((xbyte & 1) == 1);
                        }
                    }
                }

                /* --- Cirrus --- */
                if (vidmem == 0) {
                    checking("Cirrus");
                    if (CirrusCK != 0) {
                        clreol();
                        cprintf("Cirrus");
                        Caption3("chipset type");
                        switch (CirrusCK) {
                            case 0xEC: cprintf("510/520"); break;
                            case 0xCA: cprintf("610/620"); break;
                            case 0xEA: cprintf("Video Seven"); break;
                            default: cprintf("unknown - $%02X", CirrusCK);
                        }
                        Caption3("Cirrus BIOS");
                        s[0] = readROM(0xC000, 6, 2)[0];
                        s[1] = readROM(0xC000, 6, 2)[1];
                        s[2] = '\0';
                        YesOrNo(strcmp(s, "CL") == 0);
                    }
                }

                /* --- CTI --- */
                if (vidmem == 0) {
                    checking("CTI");
                    outportb(0x46E8, 0x1E);
                    xbyte = inportb(0x104);
                    outportb(0x46E8, 0x0E);
                    if (xbyte == 0xA5) {
                        union REGS r;
                        r.h.ah = 0x5F;
                        r.h.al = 0;
                        int86(0x10, &r, &r);
                        clreol();
                        cprintf("CTI 82C45");
                        outportb(0x46E8, 0x1E);
                        outportb(0x103, 0x80);
                        outportb(0x46E8, 0x0E);
                        outportb(0x3D6, 0);
                        xbyte = inportb(0x3D7);
                        xbyte3 = (xbyte & 0xF0) >> 4;
                        if (xbyte3 == 1) {
                            outportb(0x3D6, 0x3A);
                            i = inportb(0x3D7);
                            outportb(0x3D7, 0xAA);
                            xbyte2 = inportb(0x3D7);
                            outportb(0x3D7, i);
                            if (xbyte2 == 0xAA) xbyte3 = 2; else xbyte3 = 1;
                        }
                        outportb(0x46E8, 0x1E);
                        outportb(0x103, 0);
                        outportb(0x46E8, 0x0E);
                        outportb(0x3D6, 0);
                        cprintf("%d\r\n", xbyte3);
                        switch (r.h.bh) {
                            case 0: vidmem = 256; break;
                            case 1: vidmem = 512; break;
                            case 2: vidmem = 1024; break;
                            default: vidmem = 0;
                        }
                        Caption3("  Chip revision");
                        cprintf("%d\r\n", xbyte & 0x0F);
                        Caption3("micro-channel");
                        YesOrNo2(r.h.cl & 2);
                        Caption3("DAC size");
                        if (r.h.cl & 1) cprintf("8-bit"); else cprintf("6-bit");
                    }
                }

                /* --- Trident --- */
                if (vidmem == 0) {
                    checking("Trident");
                    outportb(0x3C4, 0x0B);
                    xbyte = inportb(0x3C5);
                    if (xbyte >= 2 && xbyte <= 6) {
                        clreol();
                        cprintf("Trident ");
                        switch (xbyte) {
                            case 1: cprintf("8800BR\r\n"); break;
                            case 2: cprintf("8800CS\r\n"); break;
                            default:
                                cprintf("8900\r\n");
                                outportb(0x3C4, 0x1F);
                                xbyte = inportb(0x3C5) & 3;
                                switch (xbyte) {
                                    case 0: vidmem = 256; break;
                                    case 1: vidmem = 512; break;
                                    case 2: case 3: vidmem = 1024; break;
                                }
                        }
                        Caption3("  BIOS");
                        outportb(0x3C4, 0x0F);
                        xbyte = inportb(0x3C5);
                        d8or16bit((xbyte & 0x80) == 0);
                        Caption3("interlaced");
                        outportb(0x3C4, 0x1E);
                        xbyte = inportb(0x3C5);
                        YesOrNo((xbyte & 0x20) == 0x20);
                        {
                            union REGS r;
                            r.x.ax = 0x7000;
                            r.x.bx = 0;
                            int86(0x10, &r, &r);
                            if (r.h.al == 0x70) {
                                Caption3("Everex Card");
                                switch ((r.x.dx >> 4) & 0xFFF) {
                                    case 0x678: cprintf("Viewpoint"); break;
                                    case 0x236: cprintf("Ultragraphics II"); break;
                                    case 0x620: cprintf("Vision VGA"); break;
                                    case 0x673: cprintf("EVGA"); break;
                                    default: dontknow();
                                }
                                vidmem = ((r.h.ch >> 6) * 256) + 256;
                                Caption3("Monitor");
                                if (r.h.cl < 8) cprintf("%s\r\n", trividmons[r.h.cl]);
                                else cprintf("(unknown) - %u\r\n", r.h.cl);
                            }
                        }
                    }
                }

                /* --- Tseng --- */
                if (vidmem == 0) {
                    checking("Tseng");
                    if (tsengCK == 1) {
                        clreol();
                        cprintf("Tseng ET");
                        unsigned port_base = (inportb(0x3CC) & 1) ? 0x3D0 : 0x3B0;
                        outportb(port_base + 4, 0x33);
                        xbyte = inportb(port_base + 5);
                        outportb(port_base + 5, xbyte ^ 0x0F);
                        xbyte2 = inportb(port_base + 5);
                        outportb(port_base + 5, xbyte);
                        if (xbyte2 == (xbyte ^ 0x0F)) {
                            cprintf("4000");
                            outportb(0x3BF, 3);
                            outportb(0x3D8, 0xA0);
                            {
                                union REGS r;
                                r.x.ax = 0x10F1;
                                r.h.bl = 0;
                                int86(0x10, &r, &r);
                                if (r.h.al == 0x10) {
                                    switch (r.h.bl) {
                                        case 0: cprintf(" w/ normal DAC"); break;
                                        case 1: cprintf(" w/ SC1148x HiColor DAC"); break;
                                        case 2: cprintf(" w/ new HiColor DAC"); break;
                                        default: cprintf(" w/ unknown HiColor DAC");
                                    }
                                }
                            }
                            outportb(port_base + 4, 0x37);
                            xbyte = inportb(port_base + 5);
                            if ((xbyte & 8) == 0) vidmem = 256;
                            else {
                                switch (xbyte & 3) {
                                    case 0: case 1: vidmem = 256; break;
                                    case 2: vidmem = 512; break;
                                    case 3: vidmem = 1024; break;
                                }
                            }
                        } else {
                            cprintf("3000");
                        }
                        Caption3("ROM");
                        d8or16bit((xbyte & 0x10) == 0);
                        Caption3("Video");
                        outportb(port_base + 4, 0x36);
                        xbyte = inportb(port_base + 5);
                        d8or16bit((xbyte & 0x40) == 0);
                        Caption3("I/O");
                        d8or16bit((xbyte & 0x80) == 0);
                        cprintf("\r\n");
                        outportb(port_base + 4, 0x37);
                        xbyte = inportb(port_base + 5);
                        Caption3("Memory type");
                        if (xbyte & 0x80) cprintf("VRAM"); else cprintf("DRAM");
                        outportb(0x3C4, 7);
                        xbyte = inportb(0x3C5);
                        Caption3("Compatibility");
                        if (xbyte & 0x80) cprintf("VGA"); else cprintf("EGA");
                        Caption3("ROM address");
                        if ((xbyte & 0x20) == 0) {
                            if ((xbyte & 8) == 0) cprintf("C000-C3FF");
                            else cprintf("disabled");
                        } else {
                            if ((xbyte & 8) == 0) cprintf("C000-C5FF and C680 - C7FF");
                            else cprintf("C000-C7FF");
                        }
                        cprintf("\r\n");
                    }
                }

                /* --- ZyMOS --- */
                if (vidmem == 0) {
                    checking("ZyMOS");
                    if (zymosCK == 2) {
                        clreol();
                        cprintf("ZyMOS\r\n");
                    }
                }

                /* --- Oak --- */
                if (vidmem == 0) {
                    checking("Oak");
                    if (isport2(0x3DE)) {
                        clreol();
                        cprintf("Oak\r\n");
                        outportb(0x3DE, 0x0D);
                        delay_io(1);
                        xbyte = inportb(0x3DF);
                        if (xbyte & 0x80) vidmem = 512;
                        else vidmem = 256;
                    }
                }

                /* --- ATI --- */
                if (vidmem == 0) {
                    checking("ATI");
                    strcpy(s, readROM(0xC000, 0x31, 9));
                    if (strcmp(s, "761295520") == 0) {
                        clreol();
                        cprintf("ATI ");
                        char c = peekb(0xC000, 0x43);
                        switch (c) {
                            case '1': cprintf("18800"); break;
                            case '2': cprintf("18800-1"); break;
                            case '3': cprintf("28800-2"); break;
                            case '4': cprintf("28800-4"); break;
                            case '5': cprintf("28800-5"); break;
                            case 'a': cprintf("68875"); break;
                            default: cprintf("\"%c\"???", c);
                        }
                        Caption3("Board");
                        s[0] = peekb(0xC000, 0x40);
                        s[1] = peekb(0xC000, 0x41);
                        s[2] = '\0';
                        if (strcmp(s, "31") == 0) {
                            if (c == 'a') cprintf("Ultra Pro"); else cprintf("VGAWonder");
                        } else if (strcmp(s, "32") == 0) {
                            cprintf("EGAWonder 800+");
                        }
                        Caption3("Revision");
                        cprintf("%u.%02u\r\n", peekb(0xC000, 0x4D), peekb(0xC000, 0x4C));
                        xbyte = peekb(0xC000, 0x42);
                        Caption3("mouse port");
                        YesOrNo(xbyte & 2);
                        Caption3("programmable video clock");
                        YesOrNo2(xbyte & 0x10);
                        unsigned ati_base = peekw(0xC000, 0x10);
                        xbyte = ATIinfo(0xBB, ati_base);
                        Caption3("monitor");
                        cprintf("%s\r\n", atividmons[xbyte & 0x0F]);
                        if (c > '0') {
                            xbyte = peekb(0xC000, 0x44);
                            Caption3("70Hz non-interlace");
                            YesOrNo2(xbyte & 1);
                            Caption3("Korean chars");
                            YesOrNo2(xbyte & 2);
                            Caption3("Memory clock");
                            if (xbyte & 4) cprintf("45MHz"); else cprintf("40MHz");
                            Caption3("Zero wait state");
                            YesOrNo2(xbyte & 8);
                            Caption3("Paged ROMs");
                            YesOrNo2(xbyte & 0x10);
                            Caption3("8514/A");
                            YesOrNo2(!(xbyte & 0x40));
                            Caption3("HiColor DAC");
                            YesOrNo(xbyte & 0x80);
                        }
                        Caption3("Video modes");
                        {
                            int cnt = 0, any = 0;
                            union REGS r;
                            for (i = 0; i < 255; i++) {
                                r.h.ah = 0x12;
                                r.h.al = i;
                                r.x.bx = 0x5506;
                                r.x.bp = 0xFFFF;
                                int86(0x10, &r, &r);
                                if (r.x.bp != 0xFFFF) {
                                    cprintf("%02X ", i);
                                    cnt++;
                                    any = 1;
                                    if (cnt == 21) {
                                        cprintf("\r\n               ");
                                        cnt = 0;
                                    }
                                }
                            }
                            if (!any || (any && cnt != 0)) cprintf("\r\n");
                        }
                        vidmem = 256;
                        if (c == '1' || c == '2') {
                            if (ATIinfo(0xBB, ati_base) & 0x20) vidmem = 512;
                        } else {
                            xbyte = ATIinfo(0xB0, ati_base);
                            if (xbyte & 0x10) vidmem = 512;
                            if (c != '3') {
                                if (xbyte & 8) vidmem = 1024;
                            }
                        }
                    }
                }

                /* --- Paradise / Western Digital --- */
                if (vidmem == 0) {
                    checking("Paradise");
                    strcpy(s, readROM(0xC000, 0x7D, 4));
                    if (strcmp(s, "VGA=") == 0) {
                        clreol();
                        cprintf("Western Digital/Paradise ");
                        outportb(0x3CE, 0x0F);
                        paralock1 = inportb(0x3CF);
                        outportb(0x3CF, 5);
                        outportb(0x3C4, 7);
                        unsigned port_base = (inportb(0x3CC) & 1) ? 0x3D0 : 0x3B0;
                        outportb(port_base + 4, 0x29);
                        paralock2 = inportb(port_base + 5);
                        outportb(port_base + 5, 0x85);
                        outportb(0x3C4, 0x4806);   /* outportb word ? Упростим: два байта */
                        outportb(port_base + 4, 0x2B);
                        xbyte = inportb(port_base + 5);
                        outportb(port_base + 5, 0xAA);
                        xbyte2 = inportb(port_base + 5);
                        outportb(port_base + 5, xbyte);
                        if (xbyte2 != 0xAA) {
                            xbyte3 = 1;
                        } else {
                            outportb(0x3C4, 0x12);
                            xbyte = inportb(0x3C5);
                            outportb(0x3C5, xbyte & 0xBF);
                            xbyte2 = inportb(0x3C5) & 0x40;
                            if (xbyte2 != 0) {
                                outportb(0x3C5, xbyte);
                                xbyte3 = 2;
                            } else {
                                outportb(0x3C5, xbyte | 0x40);
                                xbyte2 = inportb(0x3C5) & 0x40;
                                if (xbyte2 == 0) {
                                    outportb(0x3C5, xbyte);
                                    xbyte3 = 2;
                                } else {
                                    outportb(0x3C5, xbyte);
                                    xbyte3 = 4;
                                    outportb(0x3C4, 0x10);
                                    xbyte = inportb(0x3C5);
                                    outportb(0x3C5, xbyte & 0xFB);
                                    xbyte2 = inportb(0x3C5) & 4;
                                    if (xbyte2 != 0) xbyte3 = 3;
                                    outportb(0x3C5, xbyte | 4);
                                    xbyte2 = inportb(0x3C5) & 4;
                                    if (xbyte2 == 0) xbyte3 = 3;
                                    outportb(0x3C5, xbyte);
                                }
                            }
                        }
                        cprintf("%s\r\n", parachips[xbyte3]);
                        outportb(0x3CE, 0x0B);
                        delay_io(2);
                        xbyte = inportb(0x3CF);
                        vidmem = 64 * (xbyte >> 4);
                        Caption3("Video");
                        d8or16bit((xbyte & 4) == 0);
                        Caption3("ROM");
                        d8or16bit((xbyte & 2) == 0);
                        Caption3("Frequencies are");
                        outportb(0x3CE, 0x0F);
                        xbyte = inportb(0x3CF);
                        if (xbyte & 0x80) cprintf("multi-sync\r\n"); else cprintf("fixed-sync\r\n");
                        outportb(port_base + 4, 0x29);
                        outportb(port_base + 5, paralock2);
                        outportb(0x3CE, 0x0F);
                        outportb(0x3CF, paralock1);
                    }
                }

                /* Если ни один чип не определён */
                if (vidmem == 0) {
                    clreol();
                    dontknow();
                }
            }

            /* Общая информация для VGA */
            captfont();
            showfont(1);
            showfont(2);
            showfont(3);
            showfont(4);
            showfont(5);
            showfont(6);
            showfont(7);
            int101210();

            /* Палитра */
            {
                unsigned char saveattr = TextAttr;
                int savex = wherex(), savey = wherey();
                TextColor(LIGHTRED + 128);
                cprintf("**Retrieving palette information**");
                {
                    union REGS r;
                    r.x.ax = 0x1009;
                    r.x.es = FP_SEG(VGAbuf);
                    r.x.dx = FP_OFF(VGAbuf);
                    int86(0x10, &r, &r);
                }
                gotoxy(savex, savey);
                cprintf("                                  ");
                gotoxy(savex, savey);
                TextAttr = saveattr;
            }
            Caption2("Palette registers");
            for (i = 0; i <= 0x0F; i++)
                cprintf("%02X ", VGAbuf[i]);
            cprintf("\r\n");
            Caption2("Border color");
            cprintf("%02X", VGAbuf[0x10]);
            {
                union REGS r;
                r.x.ax = 0x101A;
                int86(0x10, &r, &r);
                Caption3("Color page");
                cprintf("$%02X", r.h.bh);
                Caption3("Paging mode");
                switch (r.h.bl) {
                    case 0x00: cprintf("4 pages of 64 registers\r\n"); break;
                    case 0x01: cprintf("16 pages of 16 registers\r\n"); break;
                    default: unknown("mode", r.h.bl, 2);
                }
            }
            break;

        case PC3270:
            cprintf("3270 PC\r\n");
            captfont();
            break;

        default:
            unknown("adapter", graphdriver, 4);
            break;
    }
}
