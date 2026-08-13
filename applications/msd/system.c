/* SYSTEM.C – Системные обёртки, работа с памятью и видео (без ассемблера) */
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

/* --------------------------------------------------------------------------
   Прямой доступ к портам (нужен для некоторых функций)
   -------------------------------------------------------------------------- */
static unsigned char inportb(unsigned short port)
{
    unsigned char val;
    _asm {
        mov dx, port
        in al, dx
        mov val, al
    }
    return val;
}

static void outportb(unsigned short port, unsigned char val)
{
    _asm {
        mov dx, port
        mov al, val
        out dx, al
    }
}

/* ======================================================================
   Видеофункции (прямая запись в B800:0000)
   ====================================================================== */
void video_putch(int row, int col, char ch, unsigned char attr)
{
    char __far *vram = (char __far *)SCREEN_BASE;
    vram[(row * MAX_COLS + col) * 2] = ch;
    vram[(row * MAX_COLS + col) * 2 + 1] = attr;
}

void video_puts(int row, int col, const char *str, unsigned char attr)
{
    while (*str) video_putch(row, col++, *str++, attr);
}

void video_clear(unsigned char attr)
{
    char __far *vram = (char __far *)SCREEN_BASE;
    int i;
    for (i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        vram[i*2] = ' ';
        vram[i*2+1] = attr;
    }
}

/* ======================================================================
   Чтение/запись памяти
   ====================================================================== */
unsigned char peekb(unsigned seg, unsigned ofs)
{
    return *(unsigned char far *)(((unsigned long)seg << 16) + ofs);
}

unsigned peekw(unsigned seg, unsigned ofs)
{
    return *(unsigned far *)(((unsigned long)seg << 16) + ofs);
}

/* ======================================================================
   Системная информация (память, версия DOS, видео, мышь)
   ====================================================================== */
unsigned get_conventional_memory(void)
{
    union REGS r;
    int86(0x12, &r, &r);
    return r.x.ax;
}

unsigned get_extended_memory(void)
{
    union REGS r;
    r.h.ah = 0x88;
    int86(0x15, &r, &r);
    if (r.h.ah == 0) return r.x.ax;
    return 0;
}

unsigned get_video_memory_size(void)
{
    union REGS r;
    r.h.ah = 0x12;
    r.h.bl = 0x30;
    int86(0x10, &r, &r);
    switch (r.h.bh) {
        case 0: return 256;
        case 1: return 512;
        case 2: return 1024;
        case 3: return 2048;
        default: return 256;
    }
}

void get_dos_version(int *major, int *minor, char *oem_name)
{
    union REGS r;
    r.h.ah = 0x30;
    int86(0x21, &r, &r);
    *major = r.h.al;
    *minor = r.h.ah;
    switch (r.h.bh) {
        case 0xFF: strcpy(oem_name, "Microsoft"); break;
        case 0xFD: strcpy(oem_name, "IBM"); break;
        case 0x99: strcpy(oem_name, "Compaq"); break;
        default:   sprintf(oem_name, "OEM %02X", r.h.bh);
    }
}

void get_vga_info(char *buf, int bufsize)
{
    union REGS r;
    r.h.ah = 0x12;
    r.h.bl = 0x10;
    int86(0x10, &r, &r);
    if (r.h.bl == 0x10) {
        strncpy(buf, "Video: VGA compatible", bufsize);
    } else {
        sprintf(buf, "Video: VGA (subsystem %02X)", r.h.bh);
    }
}

int bios_disk_read(int drive, int head, int track, int sector,
                   int count, void __far *buffer)
{
    union REGS r;
    struct SREGS s;
    r.h.ah = 0x02;
    r.h.al = (unsigned char)count;
    r.h.dl = (unsigned char)drive;
    r.h.dh = (unsigned char)head;
    r.h.ch = (unsigned char)track;
    r.h.cl = (unsigned char)sector;
    s.es = FP_SEG(buffer);
    r.x.bx = FP_OFF(buffer);
    int86x(0x13, &r, &r, &s);
    return (r.h.ah == 0) ? 0 : r.h.ah;
}

int mouse_driver_installed(void)
{
    union REGS r;
    r.x.ax = 0;
    int86(0x33, &r, &r);
    return (r.x.ax == 0xFFFF) ? 1 : 0;
}

void get_mouse_info(int *num_buttons, int *irq)
{
    union REGS r;
    r.x.ax = 0;
    int86(0x33, &r, &r);
    *num_buttons = (r.x.bx != 0) ? r.x.bx : 2;
    *irq = -1;
}

unsigned get_equipment_list(void)
{
    union REGS r;
    int86(0x11, &r, &r);
    return r.x.ax;
}

void get_disk_info(int drive_letter, unsigned long *total_clusters,
                   unsigned long *free_clusters, unsigned *bytes_per_cluster)
{
    int drive_num = (drive_letter >= 'A' && drive_letter <= 'Z')
                    ? drive_letter - 'A' + 1 : 0;
    union REGS r;
    r.h.ah = 0x36;
    r.h.dl = (unsigned char)drive_num;
    int86(0x21, &r, &r);
    if (r.x.ax == 0xFFFF) {
        *total_clusters = 0; *free_clusters = 0; *bytes_per_cluster = 0;
    } else {
        *bytes_per_cluster = r.x.cx;
        *total_clusters = r.x.dx;
        *free_clusters = r.x.bx;
    }
}

