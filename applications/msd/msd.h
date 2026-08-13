#ifndef MSD_H
#define MSD_H

#include <dos.h>
#include <conio.h>
#include <graph.h>

/* --------------------------------------------------------------------------
   Основные константы
   -------------------------------------------------------------------------- */
#define MAX_ROWS         25
#define MAX_COLS         80
#define SCREEN_BASE      0xB8000000UL

/* Идентификаторы страниц (22 экрана) */
#define PAGE_COMPUTER     0
#define PAGE_MEMORY       1
#define PAGE_VIDEO        2
#define PAGE_OS           3
#define PAGE_MOUSE        4
#define PAGE_ADAPTERS     5
#define PAGE_DISKS        6
#define PAGE_LPT          7
#define PAGE_COM          8
#define PAGE_IRQ          9
#define PAGE_TSR         10
#define PAGE_DRIVERS     11
#define PAGE_CMOS        12
#define PAGE_MEMMAP      13
#define PAGE_ENVIRON     14
#define PAGE_NETWORK     15
#define PAGE_WINDOWS     16
#define PAGE_VIDEODET    17
#define PAGE_INTVECT     18
#define PAGE_MEMDET      19
#define PAGE_DISKDET     20
#define PAGE_SUMMARY     21
#define MAX_PAGES        22

/* Цвета (Turbo Pascal style) */
#define BLACK            0
#define BLUE             1
#define GREEN            2
#define CYAN             3
#define RED              4
#define MAGENTA          5
#define BROWN            6
#define LIGHTGRAY        7
#define DARKGRAY         8
#define LIGHTBLUE        9
#define LIGHTGREEN       10
#define LIGHTCYAN        11
#define LIGHTRED         12
#define LIGHTMAGENTA     13
#define YELLOW           14
#define WHITE            15
#define BLINK            128

/* Графические драйверы (упрощённо) */
#define CGA              1
#define MCGA             2
#define EGA              3
#define EGAmono          6
#define VGA              4
#define hercmono         7
#define IBM8514          8
#define ATT400           9
#define PC3270           10

/* Направления для pause4 / pause5 */
#define DIR_NONE          0
#define DIR_UP            1
#define DIR_DOWN          2
#define DIR_UPDOWN        3

/* --------------------------------------------------------------------------
   Типы данных
   -------------------------------------------------------------------------- */
typedef struct {
    int          page_id;
    void (__far *display_func)(void);
    char         title[20];
} PAGE_ENTRY;

/* Тип для CPUID */
typedef struct {
    unsigned char cpu_type;
    unsigned int  MSW;
    unsigned char GDT[6];
    unsigned char IDT[6];
    unsigned char intflag;          /* boolean */
    unsigned char ndp_type;
    unsigned int  ndp_cw;
    unsigned char weitek;
    char          test_type;        /* 'C', 'N', 'W' */
} cpu_info_t;

/* Запись для автопечати */
typedef struct {
    char          Mode;             /* 'S' – одиночный, 'A' – авто */
    char          Destination;      /* 'P' – принтер, 'F' – файл, '?' – не задан */
    char          Filename[128];
    int           HiStrip;          /* boolean */
    char          HeaderStr[256];
    unsigned char ScreensPerPage;
    unsigned char ScreenCount;
} TPrinterRec;

/* --------------------------------------------------------------------------
   Глобальные переменные (extern – определены в GLOBALS.C)
   -------------------------------------------------------------------------- */
extern char qversion[];             /* "Version 1.58a" */
extern char qdate[];                /* "September 17, 1993" */
extern char vernum[];               /* "1.58a" */
extern unsigned HelpVersion;        /* 157 */
extern unsigned BIOSdseg;           /* 0x0040 */

extern int pgmax;                   /* 21 */
extern char *pgnames[22];           /* имена страниц */

extern unsigned Pg;                 /* текущая страница */

/* Видео и экран */
extern unsigned char attrsave;
extern unsigned char tlength;
extern unsigned char twidth;
extern unsigned char x1, x2;
extern unsigned char vidpg;
extern unsigned vidmode;
extern unsigned char lastmode;

extern unsigned Country[0x22];      /* массив информации о стране */
extern unsigned ccode;              /* код страны */
extern unsigned char decimal;       /* десятичный разделитель */
extern int gotcountry;

/* Диски и оборудование */
extern unsigned char currdrv;
extern unsigned char lastdrv;
extern unsigned equip;
extern unsigned long DOSmem;
extern unsigned PrefixSeg;

/* Адреса данных DOS */
extern unsigned devofs;
extern unsigned devseg;
extern unsigned DOScofs;
extern unsigned DOScseg;

/* Векторы прерываний */
extern void far *intvec[256];

/* Версия DOS */
extern unsigned char osmajor;
extern unsigned char osminor;

/* Символ переключателя */
extern unsigned char switchar;

/* Флаги состояния */
extern int endit;
extern int quiet;
extern int mono;
extern int resetvideo;
extern int novgacheck;
extern int ReadPartitionTable;
extern int FifoOn;

/* Графический драйвер */
extern int graphdriver;

/* Символы клавиш (глобальные) */
extern char c2[2];
extern char xchar1, xchar2;

/* Набор символов для путей */
extern char dirsep[];

/* Дополнительные переменные */
extern unsigned i;
extern unsigned xword;
extern int xbool1, xbool2;

/* Регистры (глобальная копия) */
extern union REGS regs;

/* Принтер / авто-печать */
extern TPrinterRec PrinterRec;

/* Флаг DirectVideo (из conio.h, но может потребоваться extern) */
extern int _directvideo;            /* обычно 1 */
extern int DirectVideo;             /* для ясности (0=BIOS, 1=прямой) */
extern int CheckSnow;               /* 1 = проверка снега на CGA */
extern int CheckBreak;              /* 1 = Ctrl-Break проверяется */

/* WindMin / WindMax (определены в <conio.h>, но если нужно extern) */
extern unsigned WindMin;
extern unsigned WindMax;
extern unsigned char TextAttr;

/* --------------------------------------------------------------------------
   Прототипы обёрток (реализованы в COMMON.C)
   -------------------------------------------------------------------------- */
int  wherex(void);
int  wherey(void);
void GotoXY(int x, int y);
void Window(int left, int top, int right, int bottom);
void ClrScr(void);
int  WhereY(void);

void TextColor(int color);
void TextBackground(int color);

/* --------------------------------------------------------------------------
   Прототипы общих функций (COMMON.C)
   -------------------------------------------------------------------------- */
void  Caption1(const char *a);
void  Caption2(const char *a);
void  Caption3(const char *a);
int   nocarry(union REGS *r);
char *hex(unsigned a, int b);
void  unknown(const char *msg, unsigned code, int base);
void  YesOrNo(int cond);
void  YesOrNo2(int cond);
void  YesOrNo3(int cond);
void  dontknow(void);
void  dontknow2(void);
void  SegOfs(unsigned seg, unsigned ofs);
char  showchar(char c);
unsigned long power2(unsigned y);
void  pause1(void);
void  pause2(void);
void  pause3(int extra);
void  pause4(int direc, char *ch2);
void  pause5(int direc, char *ch2);
char *bin4(unsigned char a);
void  offoron(const char *a, int b);
void  zeropad(unsigned a);
void  zeropad3(unsigned a);
void  showvers(void);
unsigned cbw(unsigned char a, unsigned char b);
char *bin16(unsigned a);
void  drvname(unsigned char a);
void  media(unsigned char a, unsigned char b);
void  pagenameclr(void);
void  Intr(int intno, union REGS *regs);
void  MsDos(union REGS *regs);
unsigned char UnBCD(unsigned char b);
char *addzero(unsigned char b);
void  modeinfo(unsigned char *vidmode, unsigned char *vidlen,
               unsigned char *vidpg, unsigned *vidwid);
void  box(void);
void  center(const char *s);
int   EMSOK(void);
void  getkey2(char *ch2);
unsigned getnum(void);

/* --------------------------------------------------------------------------
   Прототипы низкоуровневых функций (SYSTEM.C)
   -------------------------------------------------------------------------- */
void     __cdecl CPUID(cpu_info_t *info);
unsigned __cdecl diskread(unsigned char drive, unsigned long starting_sector,
                          unsigned int number_of_sectors, void __far *buffer);
void     __cdecl longcall(unsigned long addr, union REGS *regs);
unsigned char __cdecl ATIinfo(unsigned char data_in, unsigned int reg);
void     __cdecl AltIntr(unsigned char intno, union REGS *regs);
void     __cdecl AltMsDos(union REGS *regs);
unsigned char __cdecl bugtst(void);
unsigned char __cdecl CTICK(void);
unsigned char __cdecl TsengCK(void);
unsigned char __cdecl ZyMOSCK(void);
unsigned char __cdecl CirrusCK(void);

unsigned get_conventional_memory(void);
unsigned get_extended_memory(void);
unsigned get_video_memory_size(void);
void     get_dos_version(int *major, int *minor, char *oem_name);
void     get_vga_info(char *buf, int bufsize);
int      bios_disk_read(int drive, int head, int track, int sector,
                        int count, void __far *buffer);
int      mouse_driver_installed(void);
void     get_mouse_info(int *num_buttons, int *irq);
unsigned get_equipment_list(void);
void     get_disk_info(int drive_letter, unsigned long *total_clusters,
                       unsigned long *free_clusters, unsigned *bytes_per_cluster);
void     display_lpt_info(int start_row);
void     display_com_info(int start_row);
void     display_irq_info(int start_row);
void     display_tsr_info(int start_row);
void     display_device_drivers_info(int start_row);
void     display_cmos_info(int start_row);
void     display_memory_map(int start_row);
void     display_environment(int start_row);
void     display_network_info(int start_row);
void     display_windows_info(int start_row);
void     display_video_details(int start_row);
void     display_interrupt_vectors(int start_row);
void     display_memory_details(int start_row);
void     display_disk_details(int start_row);
void     display_summary(int start_row);

/* Видео (SYSTEM.C) */
void video_putch(int row, int col, char ch, unsigned char attr);
void video_puts(int row, int col, const char *str, unsigned char attr);
void video_clear(unsigned char attr);

/* Чтение/запись памяти (SYSTEM.C) */
unsigned char peekb(unsigned seg, unsigned ofs);
unsigned      peekw(unsigned seg, unsigned ofs);

/* Инициализация и главный цикл */
void init(int argc, char *argv[]);
void runit(int argc, char *argv[]);

/* Помощь и печать экрана */
void HelpScreen(int pg, unsigned long helpver);
void ScreenPrint(int Pg, const char *PgName, const char *VerNum);

/* Прототипы страниц */
void page00(void); void page01(void); void page02(void); void page03(void);
void page04(void); void page05(void); void page06(void); void page07(void);
void page08(void); void page09(void); void page10(void); void page11(void);
void page12(void); void page13(void); void page14(void); void page15(void);
void page16(void); void page17(void); void page18(void); void page19(void);
void page20(void); void page21(void);

void textattr(int attr);
int  GetTextAttr(void);
void textmode(int mode);
void textcolor(int color);
void textbackground(int color);
void clrscr(void);
void gotoxy(int x, int y);

#endif /* MSD_H */
