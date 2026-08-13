/* GLOBALS.C – Global variables (complete) */

#include "msd.h"

 #include "stddef.h"

/* Версия и дата */
char qversion[] = "Version 1.58a";
char qdate[]    = "September 17, 1993";
char vernum[]   = "1.58a";
unsigned HelpVersion = 157;

/* Адрес данных BIOS */
unsigned BIOSdseg = 0x40;

/* Количество страниц */
int pgmax = 21;

/* Имена страниц */
char *pgnames[22] = {
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

/* Текущая страница */
unsigned Pg = 0;

/* Видео и экран */
unsigned char attrsave = 0x07;
unsigned char tlength  = 25;
unsigned char twidth   = 80;
unsigned char x1 = 1, x2 = 80;
unsigned char vidpg = 0;
unsigned vidmode = 0x03;
unsigned char lastmode = 0x03;

/* Страна и формат */
unsigned Country[0x22] = {0};
unsigned ccode = 0;
unsigned char decimal = '.';
int gotcountry = 0;

/* Диски и оборудование */
unsigned char currdrv = 0;
unsigned char lastdrv = 0;
unsigned equip = 0;
unsigned long DOSmem = 0;
unsigned PrefixSeg = 0;

/* Адреса данных DOS */
unsigned devofs  = 0;
unsigned devseg  = 0;
unsigned DOScofs = 0;
unsigned DOScseg = 0;

/* Векторы прерываний */
void far *intvec[256] = { NULL };

/* Версия DOS */
unsigned char osmajor = 0;
unsigned char osminor = 0;

/* Символ переключателя */
unsigned char switchar = '/';

/* Флаги состояния */
int endit    = 0;
int quiet    = 0;
int mono     = 0;
int resetvideo = 0;
int novgacheck = 0;
int ReadPartitionTable = 1;
int FifoOn = 1;

/* Графический драйвер */
int graphdriver = 0;

/* Символы клавиш (глобальные) */
char c2[2] = {0, 0};
char xchar1, xchar2;

/* Набор символов для путей */
char dirsep[4] = "\\/:";

/* Дополнительные переменные (для совместимости) */
unsigned i;
unsigned xword;
int xbool1, xbool2;

/* Регистры (глобальная копия) */
union REGS regs;

/* Принтер / авто-печать */
TPrinterRec PrinterRec = {
    'S', '?', "", 1, "", 2, 0
};

/* Атрибуты экрана и окна */
unsigned char TextAttr = 0x07;
unsigned WindMin = 0x0101;
unsigned WindMax = 0x1950;      /* 80x25 */
int _directvideo = 1;
int DirectVideo = 1;
int CheckSnow = 1;
int CheckBreak = 1;
