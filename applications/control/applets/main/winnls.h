/*
 * winnls.h - Win16-совместимые объявления для NLS API
 *            Значения приведены к стандарту (см. olenls.h / winnls.h).
 */
#ifndef _WINNLS_H_
#define _WINNLS_H_
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long DWORD;
typedef DWORD          LCID;
typedef DWORD          LCTYPE;

typedef struct {
    WORD wYear;
    WORD wMonth;          /* 1..12 */
    WORD wDayOfWeek;      /* 0=воскресенье */
    WORD wDay;
    WORD wHour;           /* 0..23 */
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;
typedef SYSTEMTIME FAR *LPSYSTEMTIME;

/* Default locales */
#define LOCALE_SYSTEM_DEFAULT   0x0400L
#define LOCALE_USER_DEFAULT     0x0400L

/* ------ Строковые типы ------ */
#define LOCALE_SLANGUAGE             0x00000002L   /* localized name of locale */
#define LOCALE_SCOUNTRY              0x00000006L   /* localized name of country/region */
#define LOCALE_SLIST                 0x0000000CL
#define LOCALE_SDECIMAL              0x0000000EL
#define LOCALE_STHOUSAND             0x0000000FL
#define LOCALE_SDATE                 0x0000001DL
#define LOCALE_STIME                 0x0000001EL
#define LOCALE_SSHORTDATE            0x0000001FL
#define LOCALE_SLONGDATE             0x00000020L
#define LOCALE_SCURRENCY             0x00000014L
#define LOCALE_S1159                 0x00000028L
#define LOCALE_S2359                 0x00000029L
#define LOCALE_SAM                   LOCALE_S1159
#define LOCALE_SPM                   LOCALE_S2359

/* ------ Числовые типы ------ */
#define LOCALE_ILANGUAGE             0x00000001L
#define LOCALE_ICOUNTRY              0x00000005L
#define LOCALE_IDIGITS               0x00000011L
#define LOCALE_ILZERO                0x00000012L
#define LOCALE_ICURRDIGITS           0x00000019L
#define LOCALE_ICURRENCY             0x0000001BL
#define LOCALE_INEGCURR              0x0000001CL
#define LOCALE_IMEASURE              0x0000000DL
#define LOCALE_ITIME                 0x00000023L
#define LOCALE_ITLZERO               0x00000025L
#define LOCALE_IDATE                 0x00000021L
#define LOCALE_ILDATE                0x00000022L
#define LOCALE_IDAYLZERO             0x00000026L
#define LOCALE_IMONLZERO             0x00000027L
#define LOCALE_ICENTURY              0x00000024L

/* ------ Дни недели (полные и сокращённые) ------ */
#define LOCALE_SDAYNAME1             0x0000002AL
#define LOCALE_SDAYNAME2             0x0000002BL
#define LOCALE_SDAYNAME3             0x0000002CL
#define LOCALE_SDAYNAME4             0x0000002DL
#define LOCALE_SDAYNAME5             0x0000002EL
#define LOCALE_SDAYNAME6             0x0000002FL
#define LOCALE_SDAYNAME7             0x00000030L

#define LOCALE_SABBREVDAYNAME1       0x00000031L
#define LOCALE_SABBREVDAYNAME2       0x00000032L
#define LOCALE_SABBREVDAYNAME3       0x00000033L
#define LOCALE_SABBREVDAYNAME4       0x00000034L
#define LOCALE_SABBREVDAYNAME5       0x00000035L
#define LOCALE_SABBREVDAYNAME6       0x00000036L
#define LOCALE_SABBREVDAYNAME7       0x00000037L

/* ------ Самые короткие сокращения дней недели ------ */
#define LOCALE_SSHORTESTDAYNAME1     0x00000060L
#define LOCALE_SSHORTESTDAYNAME2     0x00000061L
#define LOCALE_SSHORTESTDAYNAME3     0x00000062L
#define LOCALE_SSHORTESTDAYNAME4     0x00000063L
#define LOCALE_SSHORTESTDAYNAME5     0x00000064L
#define LOCALE_SSHORTESTDAYNAME6     0x00000065L
#define LOCALE_SSHORTESTDAYNAME7     0x00000066L

/* ------ Месяцы (полные и сокращённые) ------ */
#define LOCALE_SMONTHNAME1           0x00000038L
#define LOCALE_SMONTHNAME2           0x00000039L
#define LOCALE_SMONTHNAME3           0x0000003AL
#define LOCALE_SMONTHNAME4           0x0000003BL
#define LOCALE_SMONTHNAME5           0x0000003CL
#define LOCALE_SMONTHNAME6           0x0000003DL
#define LOCALE_SMONTHNAME7           0x0000003EL
#define LOCALE_SMONTHNAME8           0x0000003FL
#define LOCALE_SMONTHNAME9           0x00000040L
#define LOCALE_SMONTHNAME10          0x00000041L
#define LOCALE_SMONTHNAME11          0x00000042L
#define LOCALE_SMONTHNAME12          0x00000043L

#define LOCALE_SABBREVMONTHNAME1     0x00000044L
#define LOCALE_SABBREVMONTHNAME2     0x00000045L
#define LOCALE_SABBREVMONTHNAME3     0x00000046L
#define LOCALE_SABBREVMONTHNAME4     0x00000047L
#define LOCALE_SABBREVMONTHNAME5     0x00000048L
#define LOCALE_SABBREVMONTHNAME6     0x00000049L
#define LOCALE_SABBREVMONTHNAME7     0x0000004AL
#define LOCALE_SABBREVMONTHNAME8     0x0000004BL
#define LOCALE_SABBREVMONTHNAME9     0x0000004CL
#define LOCALE_SABBREVMONTHNAME10    0x0000004DL
#define LOCALE_SABBREVMONTHNAME11    0x0000004EL
#define LOCALE_SABBREVMONTHNAME12    0x0000004FL

/* ------ Дополнительные строковые ------ */
#define LOCALE_SNAME                 0x0000005CL   /* locale name, e.g. "en-us" */
#define LOCALE_SENGCOUNTRY           0x00001002L
#define LOCALE_SENGLANGUAGE          0x00001001L
#define LOCALE_SNATIVECOUNTRY        0x00000008L
#define LOCALE_SNATIVELANGNAME       0x00000004L
#define LOCALE_IDEFAULTANSICODEPAGE  0x00001004L
#define LOCALE_IDEFAULTCODEPAGE      0x0000000BL
#define LOCALE_SGROUPING             0x00000010L
#define LOCALE_SPERCENT              0x00000076L

/* ------ Нестандартная константа (используется в intl.c) ------ */
#define LOCALE_SKEYBOARDSTOINSTALL   0x0000005EL

/* ------ Флаги ------ */
#define LOCALE_NOUSEROVERRIDE        0x80000000L
#define LOCALE_RETURN_NUMBER         0x20000000L
#define DATE_SHORTDATE               0x00000001L
#define DATE_LONGDATE                0x00000002L
#define DATE_USE_ALT_CALENDAR        0x00000004L
#define TIME_NOMINUTESORSECONDS      0x00000001L
#define TIME_NOSECONDS               0x00000002L
#define TIME_NOTIMEMARKER            0x00000004L
#define TIME_FORCE24HOURFORMAT       0x00000008L

/* ------ Прототипы ------ */
int WINAPI GetDateFormatA(LCID, DWORD, const SYSTEMTIME FAR *, LPCSTR, LPSTR, int);
int WINAPI GetTimeFormatA(LCID, DWORD, const SYSTEMTIME FAR *, LPCSTR, LPSTR, int);
int WINAPI GetLocaleInfoA(LCID, LCTYPE, LPSTR, int);
BOOL WINAPI SetLocaleInfoA(LCID, LCTYPE, LPCSTR);
LCID WINAPI GetThreadLocale(void);
BOOL WINAPI SetThreadLocale(LCID);
LCID WINAPI GetSystemDefaultLCID(void);
LCID WINAPI GetUserDefaultLCID(void);

#define GetDateFormat  GetDateFormatA
#define GetTimeFormat  GetTimeFormatA
#define GetLocaleInfo  GetLocaleInfoA
#define SetLocaleInfo  SetLocaleInfoA

#ifdef __cplusplus
}
#endif
#endif /* _WINNLS_H_ */
