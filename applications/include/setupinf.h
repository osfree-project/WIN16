#ifndef SETUPINF_H
#define SETUPINF_H

#include "common_types.h"

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

/* ---- высокоуровневый разбор country ---- */
/* inf_country.h Ц окончательна€ верси€ */
#ifndef INF_COUNTRY_H
#define INF_COUNTRY_H

#include "common_types.h"

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

#endif

#endif
