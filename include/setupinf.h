#ifndef SETUPINF_H
#define SETUPINF_H

#if defined(_WINDOWS) || defined(__WINDOWS__)
  #include <windows.h>
#else
  #define FAR            far
  typedef unsigned long  DWORD;
  typedef unsigned int   UINT;
  typedef int            BOOL;
  typedef char FAR       *LPSTR;
  typedef const char FAR *LPCSTR;
  #define CALLBACK        FAR __pascal
#endif

/* ---- базовый доступ к INF ---- */
typedef struct INF_FILE_TAG   FAR *HINF;
typedef struct INF_SECTION_TAG FAR *LPINF_SECTION;
typedef LPINF_SECTION HINF_SECTION;

HINF InfOpen(LPCSTR filename);
void InfClose(HINF hInf);

LPINF_SECTION InfFindSection(HINF hInf, LPCSTR sectionName);
int InfGetLineCount(LPINF_SECTION hSection);
LPCSTR InfGetLine(LPINF_SECTION hSection, int index);

void InfFreeSection(HINF hInf, LPCSTR sectionName);
void InfClearAllCache(HINF hInf);

/* ---- высокоуровневые функции разбора ---- */

typedef struct {
    LPSTR   name;                /* название страны (первое поле в кавычках) */

    /* 10 целочисленных параметров (индексы 0..9) */
    int     ICOUNTRY;            /*  0 Ц международный телефонный код */
    int     ICURRDIGITS;         /*  1 Ц знаков после зап€той в валюте */
    int     ICURRENCY;           /*  2 Ц позици€ символа валюты */
    int     IDATE;               /*  3 Ц формат даты */
    int     IMEASURE;            /*  4 Ц система мер (0=метрическа€) */
    int     INEGCURR;            /*  5 Ц формат отрицательной валюты */
    int     ITIME;               /*  6 Ц 12/24-часовой формат времени */
    int     ITLZERO;             /*  7 Ц ведущий ноль в часах */
    int     ILZERO;              /*  8 Ц ведущий ноль в дн€х/мес€цах */
    int     IDIGITS;             /*  9 Ц количество цифр дробной части */

    /* 10 строковых параметров (индексы 10..19) */
    LPSTR   S1159;               /* 10 Ц строка AM */
    LPSTR   S2359;               /* 11 Ц строка PM */
    LPSTR   SCURRENCY;           /* 12 Ц символ валюты */
    LPSTR   STHOUSAND;           /* 13 Ц разделитель тыс€ч */
    LPSTR   SDECIMAL;            /* 14 Ц дес€тичный разделитель */
    LPSTR   SDATE;               /* 15 Ц разделитель даты */
    LPSTR   STIME;               /* 16 Ц разделитель времени */
    LPSTR   SLIST;               /* 17 Ц разделитель списков */
    LPSTR   SSHORTDATE;          /* 18 Ц краткий формат даты */
    LPSTR   SLONGDATE;           /* 19 Ц полный формат даты */

    LPSTR   lang;                /* 20 Ц трЄхбуквенный код €зыка */
} COUNTRY_ENTRY;

BOOL InfParseCountryLine(LPCSTR line, COUNTRY_ENTRY FAR *entry);
void InfFreeCountryEntry(COUNTRY_ENTRY FAR *entry);

typedef struct {
    LPSTR code;          /* код €зыка (например, "usa") */
    int   disk;          /* номер диска (0, если не указан) */
    LPSTR file;          /* им€ файла без префикса диска, или "" */
    LPSTR description;   /* описание (из кавычек) или NULL */
} LANGUAGE_ENTRY;

BOOL InfParseLanguageLine(LPCSTR line, LANGUAGE_ENTRY FAR *entry);
void InfFreeLanguageEntry(LANGUAGE_ENTRY FAR *entry);

typedef struct {
    LPSTR code;          /* код раскладки (например, "beldll") */
    int   disk;          /* номер диска (0, если нет префикса) */
    LPSTR file;          /* им€ файла DLL или "" */
    LPSTR description;   /* описание (из кавычек) или NULL */
} KEYBOARD_ENTRY;

BOOL InfParseKeyboardLine(LPCSTR line, KEYBOARD_ENTRY FAR *entry);
void InfFreeKeyboardEntry(KEYBOARD_ENTRY FAR *entry);

#endif
