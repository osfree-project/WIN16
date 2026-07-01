/*
 * nlsapi.c Ц полна€ реализаци€ NLS API дл€ Windows 3.0
 *            все указатели согласованы, C89
 */
#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "winnls.h"

/* ---------- встроенные строки (дни, мес€цы) ---------- */
static const char FAR *MonthFull[] = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"
};
static const char FAR *MonthAbbr[] = {
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};
static const char FAR *DayFull[] = {
    "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};
static const char FAR *DayAbbr[] = {
    "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static const char FAR *DayShort[] = {
    "Su","Mo","Tu","We","Th","Fr","Sa"
};

/* ---------- вспомогательные функции ---------- */
static int FAR MyStrnicmp(const char FAR *s1, const char FAR *s2, int n) {
    while (n-- > 0) { char c1=*s1++,c2=*s2++;
        if(c1>='A'&&c1<='Z') c1+='a'-'A'; if(c2>='A'&&c2<='Z') c2+='a'-'A';
        if(c1!=c2) return (unsigned char)c1-(unsigned char)c2;
        if(c1=='\0') return 0;
    }
    return 0;
}
static int FAR DayOfWeek(int y, int m, int d) {
    static int t[]={0,3,2,5,0,3,5,1,4,6,2,4}; y-=m<3;
    return (y+y/4-y/100+y/400+t[m-1]+d)%7;
}
static void FAR GetCurrentDateTime(SYSTEMTIME FAR *pst) {
    static struct dosdate_t date; static struct dostime_t time;
    _dos_getdate(&date); _dos_gettime(&time);
    pst->wYear=date.year; pst->wMonth=date.month; pst->wDay=date.day;
    pst->wDayOfWeek=DayOfWeek(date.year,date.month,date.day);
    pst->wHour=time.hour; pst->wMinute=time.minute; pst->wSecond=time.second;
    pst->wMilliseconds=0;
}
static const char FAR * MyStrStrFar(const char FAR *s, const char FAR *sub) {
    int sublen = lstrlen(sub);
    if (sublen == 0) return NULL;
    while (*s) {
        int i;
        for (i = 0; i < sublen; i++) {
            if (s[i] != sub[i]) break;
        }
        if (i == sublen) return s;
        s++;
    }
    return NULL;
}
static int GetDayLeadingZero(const char FAR *fmt) {
    const char FAR *p = fmt;
    while (*p) { if (*p == 'd') { if (p[1] == 'd') return 1; return 0; } p++; }
    return 1;
}
static int GetMonthLeadingZero(const char FAR *fmt) {
    const char FAR *p = fmt;
    while (*p) { if (*p == 'M') { if (p[1] == 'M') return 1; return 0; } p++; }
    return 1;
}
static int GetCentury(const char FAR *fmt) {
    if (MyStrStrFar(fmt, (const char FAR *)"yyyy")) return 1;
    return 0;
}
static int GetLongDateOrder(const char FAR *fmt) {
    const char FAR *p = fmt;
    while (*p) {
        if (*p == 'M') return 0;
        if (*p == 'd' && p[1]=='d' && p[2]=='d' && p[3]=='d') return 1;
        if (*p == 'd') return 1;
        if (*p == 'y') return 2;
        p++;
    }
    return 0;
}

/* Ѕезопасное преобразование FAR-строки в int */
static int AtoiFar(const char FAR *s)
{
    int result = 0;
    int sign = 1;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return sign * result;
}

/* Ѕезопасное копирование строки с ограничением длины (far-совместима€) */
static void StringCopyN(LPSTR dest, LPCSTR src, int n)
{
    int i;
    if (n <= 0) return;
    for (i = 0; i < n - 1 && src[i] != '\0'; i++)
        dest[i] = src[i];
    dest[i] = '\0';
}

/* ------------------------------------------------------------------ */
/* “аблица сопоставлени€ "код страны + код €зыка -> LCID".            */
/* ƒл€ неизвестных комбинаций генерируетс€ фиктивный LCID.           */
/* ------------------------------------------------------------------ */
typedef struct {
    int   iCountry;      /* телефонный код страны */
    char  szLang[4];     /* код €зыка (usa, eng, frn и т.д.) */
    LCID  lcid;
} KNOWN_LCID;

static const KNOWN_LCID knownLCIDs[] = {
    { 1,   "usa", 0x0409 },  /* United States */
    { 44,  "eng", 0x0809 },  /* United Kingdom */
    { 61,  "eng", 0x0C09 },  /* Australia */
    { 64,  "eng", 0x1409 },  /* New Zealand */
    { 2,   "eng", 0x1009 },  /* Canada (English) */
    { 2,   "frn", 0x0C0C },  /* Canada (French) */
    { 33,  "frn", 0x040C },  /* France */
    { 49,  "ger", 0x0407 },  /* Germany */
    { 39,  "itn", 0x0410 },  /* Italy */
    { 34,  "spa", 0x0C0A },  /* Spain */
    { 52,  "spa", 0x080A },  /* Mexico */
    { 46,  "swe", 0x041D },  /* Sweden */
    { 358, "fin", 0x040B },  /* Finland */
    { 45,  "dan", 0x0406 },  /* Denmark */
    { 47,  "nor", 0x0414 },  /* Norway */
    { 31,  "dut", 0x0413 },  /* Netherlands */
    { 32,  "dut", 0x0813 },  /* Belgium (Dutch) */
    { 32,  "frn", 0x080C },  /* Belgium (French) */
    { 351, "por", 0x0816 },  /* Portugal */
    { 55,  "por", 0x0416 },  /* Brazil */
    { 354, "ice", 0x040F },  /* Iceland */
    { 43,  "ger", 0x0C07 },  /* Austria */
    { 41,  "frn", 0x100C },  /* Switzerland (French) */
    { 41,  "ger", 0x0807 },  /* Switzerland (German) */
    { 41,  "itn", 0x0810 },  /* Switzerland (Italian) */
    { 82,  "eng", 0xE052 },  /* South Korea (English) */
    { 886, "eng", 0xE376 },  /* Taiwan (English) */
};
#define KNOWN_LCID_COUNT (sizeof(knownLCIDs)/sizeof(knownLCIDs[0]))

/* ¬озвращает LCID дл€ пары страна/€зык. ≈сли не найдено Ц генерирует 0xE000+код_страны */
static LCID LookupLCID(int countryCode, const char FAR *szLang)
{
    int i;
    for (i = 0; i < KNOWN_LCID_COUNT; i++) {
        if (knownLCIDs[i].iCountry == countryCode &&
            lstrcmpi(knownLCIDs[i].szLang, szLang) == 0)
            return knownLCIDs[i].lcid;
    }
    return (LCID)(0xE000 + (WORD)countryCode);
}

/* ------------------------------------------------------------------ */
/* GetLocaleInfoFromInf                                                */
/* »звлекает из SETUP.INF параметр дл€ указанной страны/€зыка.        */
/* ¬озвращает TRUE, если параметр найден.                             */
/* ------------------------------------------------------------------ */
static BOOL GetLocaleInfoFromInf(int iCountryCode, const char FAR *szLang,
                                 LCTYPE LCType, LPSTR lpLCData, int cchData,
                                 BOOL returnNumber)
{
    OFSTRUCT of;
    HFILE hFile;
    static char szReadBuf[512];
    static char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;
    BOOL bInCountry = FALSE;
    BOOL bFound = FALSE;
    char szTargetCountryCode[16];
    char szTargetLang[8];
    char FAR *p;
    char FAR *q;
    char FAR *p1;
    char FAR *p2;
    static char szParams[256];
    char szFirstParam[16];
    char szFileLang[16];
    int fieldIdx;
    char FAR *pField;
    int cur;
    int len;
    int totalLen;
    int pos;
    int langLen;
    static char szCountryName[64];
    int countryNameLen;

    wsprintf(szTargetCountryCode, "%d", iCountryCode);
    lstrcpy(szTargetLang, szLang);

    GetSystemDirectory(szLine, sizeof(szLine) - 20);
    lstrcat(szLine, "\\SETUP.INF");
    hFile = OpenFile(szLine, &of, OF_READ);
    if (hFile == HFILE_ERROR) return FALSE;

    while (!bEOF && !bFound) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) {
                szLine[nLinePos] = '\0';
                goto parse_line;
            }
            break;
        }

        for (i = 0; i < nRead && !bFound; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
parse_line:
                if (nLinePos > 0) {
                    p = szLine;
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p == '[') {
                        q = _fstrchr(p + 1, ']');
                        if (q) {
                            *q = '\0';
                            bInCountry = (lstrcmpi(p + 1, "country") == 0);
                        }
                    } else if (bInCountry && *p != '\0' && *p != ';') {
                        p1 = _fstrchr(p, '\"');
                        if (p1) {
                            p2 = _fstrchr(p1 + 1, '\"');
                            if (p2) {
                                /* —охран€ем им€ страны */
                                countryNameLen = 0;
                                {
                                    char FAR *tmp = p1 + 1;
                                    while (tmp < p2 && countryNameLen < 63) {
                                        szCountryName[countryNameLen++] = *tmp++;
                                    }
                                }
                                szCountryName[countryNameLen] = '\0';

                                /* »щем параметры (втора€ пара кавычек) */
                                p1 = _fstrchr(p2 + 1, '\"');
                                if (p1) {
                                    p2 = _fstrchr(p1 + 1, '\"');
                                    if (p2) {
                                        /* ƒлина параметров */
                                        len = 0;
                                        {
                                            char FAR *tmp = p1 + 1;
                                            while (tmp < p2 && len < 255) {
                                                len++;
                                                tmp++;
                                            }
                                        }
                                        _fmemcpy(szParams, p1 + 1, len);
                                        szParams[len] = '\0';

                                        /* »звлекаем код страны (первое поле) */
                                        pos = 0;
                                        while (pos < len && szParams[pos] != '!' && pos < 15) {
                                            szFirstParam[pos] = szParams[pos];
                                            pos++;
                                        }
                                        szFirstParam[pos] = '\0';

                                        if (lstrcmp(szFirstParam, szTargetCountryCode) == 0) {
                                            /* »звлекаем код €зыка (последнее поле после последнего '!') */
                                            totalLen = lstrlen(szParams);
                                            pos = totalLen - 1;
                                            while (pos >= 0 && szParams[pos] != '!') {
                                                pos--;
                                            }
                                            if (pos >= 0 && pos < totalLen - 1) {
                                                langLen = 0;
                                                pos++;
                                                while (pos < totalLen && langLen < 15) {
                                                    szFileLang[langLen++] = szParams[pos++];
                                                }
                                                szFileLang[langLen] = '\0';
                                            } else {
                                                lstrcpy(szFileLang, "eng");
                                            }

                                            if (lstrcmpi(szFileLang, szTargetLang) == 0) {
                                                if (LCType == LOCALE_SCOUNTRY) {
                                                    StringCopyN(lpLCData, szCountryName, cchData);
                                                    bFound = TRUE;
                                                } else {
                                                    fieldIdx = -1;
                                                    if (LCType == LOCALE_ICOUNTRY) fieldIdx = 0;
                                                    else if (LCType == LOCALE_ICURRDIGITS) fieldIdx = 1;
                                                    else if (LCType == LOCALE_ICURRENCY) fieldIdx = 2;
                                                    else if (LCType == LOCALE_IDATE) fieldIdx = 3;
                                                    else if (LCType == LOCALE_IMEASURE) fieldIdx = 4;
                                                    else if (LCType == LOCALE_INEGCURR) fieldIdx = 5;
                                                    else if (LCType == LOCALE_ITIME) fieldIdx = 6;
                                                    else if (LCType == LOCALE_ITLZERO) fieldIdx = 7;
                                                    else if (LCType == LOCALE_ILZERO) fieldIdx = 8;
                                                    else if (LCType == LOCALE_IDIGITS) fieldIdx = 9;
                                                    else if (LCType == LOCALE_S1159) fieldIdx = 10;
                                                    else if (LCType == LOCALE_S2359) fieldIdx = 11;
                                                    else if (LCType == LOCALE_SCURRENCY) fieldIdx = 12;
                                                    else if (LCType == LOCALE_STHOUSAND) fieldIdx = 13;
                                                    else if (LCType == LOCALE_SDECIMAL) fieldIdx = 14;
                                                    else if (LCType == LOCALE_SDATE) fieldIdx = 15;
                                                    else if (LCType == LOCALE_STIME) fieldIdx = 16;
                                                    else if (LCType == LOCALE_SLIST) fieldIdx = 17;
                                                    else if (LCType == LOCALE_SSHORTDATE) fieldIdx = 18;
                                                    else if (LCType == LOCALE_SLONGDATE) fieldIdx = 19;

                                                    if (fieldIdx >= 0 && fieldIdx <= 19) {
                                                        pField = szParams;
                                                        cur = 0;
                                                        while (cur < fieldIdx && pField && *pField) {
                                                            char FAR *pNext = _fstrchr(pField, '!');
                                                            if (pNext) {
                                                                pField = pNext + 1;
                                                                cur++;
                                                            } else {
                                                                pField = NULL;
                                                                break;
                                                            }
                                                        }
                                                        if (pField) {
                                                            char FAR *pEnd = _fstrchr(pField, '!');
                                                            if (pEnd) *pEnd = '\0';
                                                            if (returnNumber) {
                                                                if (cchData >= (int)sizeof(int)) {
                                                                    *(int FAR*)lpLCData = AtoiFar(pField);
                                                                    bFound = TRUE;
                                                                }
                                                            } else {
                                                                StringCopyN(lpLCData, pField, cchData);
                                                                bFound = TRUE;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') i++;
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1)
                    szLine[nLinePos++] = c;
            }
        }
    }

    _lclose(hFile);
    return bFound;
}

/* ================================================================== */
int WINAPI GetLocaleInfoA(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData) {
    int number = 0;
    int returnNumber = (LCType & LOCALE_RETURN_NUMBER) ? 1 : 0;
    LCType &= ~LOCALE_RETURN_NUMBER;
    if (Locale == 0) Locale = LOCALE_USER_DEFAULT;
    if (!lpLCData || cchData <= 0) { if(returnNumber) return 0; return 0; }

    if (Locale != LOCALE_USER_DEFAULT) {
        int i;
        int countryCode = 0;
        char szLang[8] = {0};
        BOOL bHandled = FALSE;

        for (i = 0; i < (int)KNOWN_LCID_COUNT; i++) {
            if (knownLCIDs[i].lcid == Locale) {
                countryCode = knownLCIDs[i].iCountry;
                lstrcpy(szLang, knownLCIDs[i].szLang);
                bHandled = TRUE;
                break;
            }
        }
        if (!bHandled && (Locale & 0xFFFF0000) == 0xE000) {
            countryCode = (int)(Locale & 0xFFFF);
            lstrcpy(szLang, "eng");
            bHandled = TRUE;
        }

        if (bHandled) {
            if (GetLocaleInfoFromInf(countryCode, szLang, LCType, lpLCData, cchData, returnNumber))
                return returnNumber ? sizeof(int) : lstrlen(lpLCData);
        }
        return 0;
    }

    /* текуща€ локаль Ц WIN.INI */
    switch (LCType) {
    case LOCALE_SLONGDATE:
        if (returnNumber) return 0;
        GetProfileString("intl", "sLongDate", "dddd, MMMM d, yyyy", lpLCData, cchData);
        break;
    case LOCALE_SSHORTDATE:
        if (returnNumber) return 0;
        GetProfileString("intl", "sShortDate", "M/d/yy", lpLCData, cchData);
        break;
    case LOCALE_SDATE:
        if (returnNumber) return 0;
        GetProfileString("intl","sDate","/",lpLCData,cchData);
        break;
    case LOCALE_STIME:
        if (returnNumber) return 0;
        GetProfileString("intl","sTime",":",lpLCData,cchData);
        break;
    case LOCALE_S1159:
        if (returnNumber) return 0;
        GetProfileString("intl","s1159","AM",lpLCData,cchData);
        break;
    case LOCALE_S2359:
        if (returnNumber) return 0;
        GetProfileString("intl","s2359","PM",lpLCData,cchData);
        break;
    case LOCALE_SCURRENCY:
        if (returnNumber) return 0;
        GetProfileString("intl","sCurrency","$",lpLCData,cchData);
        break;
    case LOCALE_INEGCURR:
        number = GetProfileInt("intl","iNegCurr",0);
        break;
    case LOCALE_SDECIMAL:
        if (returnNumber) return 0;
        GetProfileString("intl","sDecimal",".",lpLCData,cchData);
        break;
    case LOCALE_STHOUSAND:
        if (returnNumber) return 0;
        GetProfileString("intl","sThousand",",",lpLCData,cchData);
        break;
    case LOCALE_SLIST:
        if (returnNumber) return 0;
        GetProfileString("intl","sList",",",lpLCData,cchData);
        break;
    case LOCALE_SCOUNTRY:
        if (returnNumber) return 0;
        GetProfileString("intl","sCountry","United States",lpLCData,cchData);
        break;
    case LOCALE_SLANGUAGE:
        if (returnNumber) return 0;
        GetProfileString("intl","sLanguage","usa",lpLCData,cchData);
        break;
    case LOCALE_SKEYBOARDSTOINSTALL:
        if (returnNumber) return 0;
        GetProfileString("intl","sKeyboard","US",lpLCData,cchData);
        break;
    case LOCALE_ILZERO:
        number = GetProfileInt("intl","iLzero",0);
        break;
    case LOCALE_ICURRDIGITS:
        number = GetProfileInt("intl","iCurrDigits",2);
        break;
    case LOCALE_ICURRENCY:
        number = GetProfileInt("intl","iCurrency",0);
        break;
    case LOCALE_IMEASURE:
        number = GetProfileInt("intl","iMeasure",1);
        break;
    case LOCALE_ITIME:
        number = GetProfileInt("intl","iTime",0);
        break;
    case LOCALE_ITLZERO:
        number = GetProfileInt("intl","iTLZero",0);
        break;
    case LOCALE_IDATE:
        number = GetProfileInt("intl","iDate",0);
        break;
    case LOCALE_ICOUNTRY:
        number = GetProfileInt("intl","iCountry",1);
        break;
    case LOCALE_IDEFAULTANSICODEPAGE:
        number = 1252;
        break;
    case LOCALE_IDEFAULTCODEPAGE:
        number = 437;
        break;
    case LOCALE_SGROUPING:
        if (returnNumber) return 0;
        lstrcpy(lpLCData, "3;0");
        break;
    case LOCALE_SPERCENT:
        if (returnNumber) return 0;
        lstrcpy(lpLCData, "%");
        break;
    case LOCALE_IDAYLZERO:
        {
            char buf[80];
            GetProfileString("intl","sShortDate","M/d/yy",buf,sizeof(buf));
            number = GetDayLeadingZero(buf);
        }
        break;
    case LOCALE_IMONLZERO:
        {
            char buf[80];
            GetProfileString("intl","sShortDate","M/d/yy",buf,sizeof(buf));
            number = GetMonthLeadingZero(buf);
        }
        break;
    case LOCALE_ICENTURY:
        {
            char buf[80];
            GetProfileString("intl","sShortDate","M/d/yy",buf,sizeof(buf));
            number = GetCentury(buf);
        }
        break;
    case LOCALE_ILDATE:
        {
            char buf[80];
            GetProfileString("intl","sLongDate","dddd, MMMM d, yyyy",buf,sizeof(buf));
            number = GetLongDateOrder(buf);
        }
        break;

    case LOCALE_SDAYNAME1:  if (returnNumber) return 0; lstrcpy(lpLCData, DayFull[1]); break;
    case LOCALE_SDAYNAME2:  if (returnNumber) return 0; lstrcpy(lpLCData, DayFull[2]); break;
    case LOCALE_SDAYNAME3:  if (returnNumber) return 0; lstrcpy(lpLCData, DayFull[3]); break;
    case LOCALE_SDAYNAME4:  if (returnNumber) return 0; lstrcpy(lpLCData, DayFull[4]); break;
    case LOCALE_SDAYNAME5:  if (returnNumber) return 0; lstrcpy(lpLCData, DayFull[5]); break;
    case LOCALE_SDAYNAME6:  if (returnNumber) return 0; lstrcpy(lpLCData, DayFull[6]); break;
    case LOCALE_SDAYNAME7:  if (returnNumber) return 0; lstrcpy(lpLCData, DayFull[0]); break;

    case LOCALE_SABBREVDAYNAME1: if (returnNumber) return 0; lstrcpy(lpLCData, DayAbbr[1]); break;
    case LOCALE_SABBREVDAYNAME2: if (returnNumber) return 0; lstrcpy(lpLCData, DayAbbr[2]); break;
    case LOCALE_SABBREVDAYNAME3: if (returnNumber) return 0; lstrcpy(lpLCData, DayAbbr[3]); break;
    case LOCALE_SABBREVDAYNAME4: if (returnNumber) return 0; lstrcpy(lpLCData, DayAbbr[4]); break;
    case LOCALE_SABBREVDAYNAME5: if (returnNumber) return 0; lstrcpy(lpLCData, DayAbbr[5]); break;
    case LOCALE_SABBREVDAYNAME6: if (returnNumber) return 0; lstrcpy(lpLCData, DayAbbr[6]); break;
    case LOCALE_SABBREVDAYNAME7: if (returnNumber) return 0; lstrcpy(lpLCData, DayAbbr[0]); break;

    case LOCALE_SSHORTESTDAYNAME1: if (returnNumber) return 0; lstrcpy(lpLCData, DayShort[1]); break;
    case LOCALE_SSHORTESTDAYNAME2: if (returnNumber) return 0; lstrcpy(lpLCData, DayShort[2]); break;
    case LOCALE_SSHORTESTDAYNAME3: if (returnNumber) return 0; lstrcpy(lpLCData, DayShort[3]); break;
    case LOCALE_SSHORTESTDAYNAME4: if (returnNumber) return 0; lstrcpy(lpLCData, DayShort[4]); break;
    case LOCALE_SSHORTESTDAYNAME5: if (returnNumber) return 0; lstrcpy(lpLCData, DayShort[5]); break;
    case LOCALE_SSHORTESTDAYNAME6: if (returnNumber) return 0; lstrcpy(lpLCData, DayShort[6]); break;
    case LOCALE_SSHORTESTDAYNAME7: if (returnNumber) return 0; lstrcpy(lpLCData, DayShort[0]); break;

    case LOCALE_SMONTHNAME1:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[0]); break;
    case LOCALE_SMONTHNAME2:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[1]); break;
    case LOCALE_SMONTHNAME3:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[2]); break;
    case LOCALE_SMONTHNAME4:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[3]); break;
    case LOCALE_SMONTHNAME5:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[4]); break;
    case LOCALE_SMONTHNAME6:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[5]); break;
    case LOCALE_SMONTHNAME7:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[6]); break;
    case LOCALE_SMONTHNAME8:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[7]); break;
    case LOCALE_SMONTHNAME9:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[8]); break;
    case LOCALE_SMONTHNAME10: if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[9]); break;
    case LOCALE_SMONTHNAME11: if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[10]); break;
    case LOCALE_SMONTHNAME12: if (returnNumber) return 0; lstrcpy(lpLCData, MonthFull[11]); break;

    case LOCALE_SABBREVMONTHNAME1:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[0]); break;
    case LOCALE_SABBREVMONTHNAME2:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[1]); break;
    case LOCALE_SABBREVMONTHNAME3:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[2]); break;
    case LOCALE_SABBREVMONTHNAME4:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[3]); break;
    case LOCALE_SABBREVMONTHNAME5:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[4]); break;
    case LOCALE_SABBREVMONTHNAME6:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[5]); break;
    case LOCALE_SABBREVMONTHNAME7:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[6]); break;
    case LOCALE_SABBREVMONTHNAME8:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[7]); break;
    case LOCALE_SABBREVMONTHNAME9:  if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[8]); break;
    case LOCALE_SABBREVMONTHNAME10: if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[9]); break;
    case LOCALE_SABBREVMONTHNAME11: if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[10]); break;
    case LOCALE_SABBREVMONTHNAME12: if (returnNumber) return 0; lstrcpy(lpLCData, MonthAbbr[11]); break;

    default:
        return 0;
    }

    if (returnNumber) {
        if (cchData >= (int)sizeof(int)) { *(int FAR*)lpLCData = number; return sizeof(int); }
        return 0;
    } else {
        if (LCType >= LOCALE_ILZERO && LCType <= LOCALE_IDEFAULTCODEPAGE)
            wsprintf(lpLCData, "%d", number);
        return lstrlen(lpLCData);
    }
}

/* ================================================================== */
int WINAPI GetDateFormatA(LCID Locale, DWORD dwFlags, const SYSTEMTIME FAR *lpDate,
    LPCSTR lpFormat, LPSTR lpDateStr, int cchDate) {
    static SYSTEMTIME stCur; const SYSTEMTIME FAR *pSt;
    char fmtBuf[80]; char outBuf[80]; char FAR *f, FAR *out; char tmp[16];
    int dow, remain;
    if (Locale == 0) Locale = LOCALE_USER_DEFAULT;
    if (Locale != LOCALE_USER_DEFAULT) return 0;
    if (!lpDateStr || cchDate < 1) return 0;
    out = outBuf; *out = '\0';

    if (lpFormat) lstrcpy(fmtBuf, lpFormat);
    else if (dwFlags & DATE_SHORTDATE) GetLocaleInfoA(Locale, LOCALE_SSHORTDATE, fmtBuf, sizeof(fmtBuf));
    else GetLocaleInfoA(Locale, LOCALE_SLONGDATE, fmtBuf, sizeof(fmtBuf));

    if (!lpDate) { GetCurrentDateTime(&stCur); pSt = &stCur; } else pSt = lpDate;
    dow = DayOfWeek(pSt->wYear, pSt->wMonth, pSt->wDay);
    f = fmtBuf; remain = sizeof(outBuf)-1;

    while (*f && remain > 0) {
        if (*f == '\'') { f++; while (*f && *f != '\'' && remain > 0) { *out++ = *f++; remain--; } if (*f == '\'') f++; }
        else {
            char FAR *p = f; int tokLen = 0;
            if (*p=='M'||*p=='d'||*p=='y') {
                if (*p=='M') { if (p[1]=='M'&&p[2]=='M'&&p[3]=='M') tokLen=4; else if (p[1]=='M'&&p[2]=='M') tokLen=3; else if (p[1]=='M') tokLen=2; else tokLen=1; }
                else if (*p=='d') { if (p[1]=='d'&&p[2]=='d'&&p[3]=='d') tokLen=4; else if (p[1]=='d'&&p[2]=='d') tokLen=3; else if (p[1]=='d') tokLen=2; else tokLen=1; }
                else { if (p[1]=='y'&&p[2]=='y'&&p[3]=='y') tokLen=4; else if (p[1]=='y') tokLen=2; else tokLen=1; }
            }
            if (tokLen > 0) {
                const char FAR *src = NULL;
                if (tokLen==4 && MyStrnicmp(p,"MMMM",4)==0) src = MonthFull[pSt->wMonth-1];
                else if (tokLen==3 && MyStrnicmp(p,"MMM",3)==0) src = MonthAbbr[pSt->wMonth-1];
                else if (tokLen==2 && MyStrnicmp(p,"MM",2)==0) { wsprintf(tmp,"%02u",pSt->wMonth); src = tmp; }
                else if (*p=='M') { wsprintf(tmp,"%u",pSt->wMonth); src = tmp; }
                else if (tokLen==4 && MyStrnicmp(p,"dddd",4)==0) src = DayFull[dow];
                else if (tokLen==3 && MyStrnicmp(p,"ddd",3)==0) src = DayAbbr[dow];
                else if (tokLen==2 && MyStrnicmp(p,"dd",2)==0) { wsprintf(tmp,"%02u",pSt->wDay); src = tmp; }
                else if (*p=='d') { wsprintf(tmp,"%u",pSt->wDay); src = tmp; }
                else if (tokLen==4 && MyStrnicmp(p,"yyyy",4)==0) { wsprintf(tmp,"%04u",pSt->wYear); src = tmp; }
                else if (tokLen==2 && MyStrnicmp(p,"yy",2)==0) { wsprintf(tmp,"%02u",pSt->wYear%100); src = tmp; }
                else if (*p=='y') { wsprintf(tmp,"%02u",pSt->wYear%100); src = tmp; }
                if (src) { while (*src && remain > 0) { *out++ = *src++; remain--; } }
                f += tokLen;
            } else { *out++ = *f++; remain--; }
        }
    }
    *out = '\0'; lstrcpy(lpDateStr, outBuf); return lstrlen(lpDateStr);
}

/* ================================================================== */
int WINAPI GetTimeFormatA(LCID Locale, DWORD dwFlags, const SYSTEMTIME FAR *lpTime,
    LPCSTR lpFormat, LPSTR lpTimeStr, int cchTime) {
    static SYSTEMTIME FAR stCur; const SYSTEMTIME FAR *pSt;
    char fmtBuf[80], outBuf[80], FAR *f, FAR *out, tmp[16], szTimeSep[4], szAm[8], szPm[8];
    int hour24, remain, iTime, bTLZero; WORD h;
    if (Locale == 0) Locale = LOCALE_USER_DEFAULT;
    if (Locale != LOCALE_USER_DEFAULT) return 0;
    if (!lpTimeStr || cchTime < 1) return 0;
    out = outBuf; *out = '\0';

    GetLocaleInfoA(Locale, LOCALE_STIME, szTimeSep, sizeof(szTimeSep));
    GetLocaleInfoA(Locale, LOCALE_S1159, szAm, sizeof(szAm));
    GetLocaleInfoA(Locale, LOCALE_S2359, szPm, sizeof(szPm));
    { int val;
      GetLocaleInfoA(Locale, LOCALE_ITIME | LOCALE_RETURN_NUMBER, (LPSTR)&val, sizeof(val)); iTime = val;
      GetLocaleInfoA(Locale, LOCALE_ITLZERO | LOCALE_RETURN_NUMBER, (LPSTR)&val, sizeof(val)); bTLZero = val;
    }

    if (!lpTime) { GetCurrentDateTime(&stCur); pSt = &stCur; } else pSt = lpTime;
    h = pSt->wHour;
    hour24 = (dwFlags & TIME_FORCE24HOURFORMAT) ? 1 : (iTime == 1);

    if (!hour24) {
        int h12 = h % 12; if (h12 == 0) h12 = 12;
        if (bTLZero) wsprintf(tmp, "%02u", h12); else wsprintf(tmp, "%u", h12);
    } else {
        if (bTLZero) wsprintf(tmp, "%02u", h); else wsprintf(tmp, "%u", h);
    }

    if (lpFormat) lstrcpy(fmtBuf, lpFormat);
    else {
        if (dwFlags & TIME_NOMINUTESORSECONDS) lstrcpy(fmtBuf, tmp);
        else {
            lstrcpy(fmtBuf, tmp); lstrcat(fmtBuf, szTimeSep); lstrcat(fmtBuf, "mm");
            if (!(dwFlags & TIME_NOSECONDS)) { lstrcat(fmtBuf, szTimeSep); lstrcat(fmtBuf, "ss"); }
            if (!(dwFlags & TIME_NOTIMEMARKER) && !hour24) { lstrcat(fmtBuf, " tt"); }
        }
    }

    f = fmtBuf; remain = sizeof(outBuf)-1;
    while (*f && remain > 0) {
        if (*f == '\'') { f++; while (*f && *f != '\'' && remain > 0) { *out++ = *f++; remain--; } if (*f == '\'') f++; }
        else {
            char FAR *p = f; int tokLen = 0;
            if (*p == 'h' || *p == 'H' || *p == 'm' || *p == 's' || *p == 't') {
                if (*p == 'h' || *p == 'H') { if (p[1]==*p) tokLen=2; else tokLen=1; }
                else if (*p == 'm' || *p == 's') { if (p[1]==*p) tokLen=2; else tokLen=1; }
                else if (*p == 't') { if (p[1]=='t') tokLen=2; else tokLen=1; }
            }
            if (tokLen > 0) {
                const char FAR *src = NULL;
                if (tokLen==2 && MyStrnicmp(p,"hh",2)==0) { int h12=pSt->wHour%12; if(h12==0)h12=12; wsprintf(tmp,"%02u",h12); src=tmp; }
                else if (tokLen==1 && *p=='h') { int h12=pSt->wHour%12; if(h12==0)h12=12; wsprintf(tmp,"%u",h12); src=tmp; }
                else if (tokLen==2 && MyStrnicmp(p,"HH",2)==0) { wsprintf(tmp,"%02u",pSt->wHour); src=tmp; }
                else if (tokLen==1 && *p=='H') { wsprintf(tmp,"%u",pSt->wHour); src=tmp; }
                else if (tokLen==2 && MyStrnicmp(p,"mm",2)==0) { wsprintf(tmp,"%02u",pSt->wMinute); src=tmp; }
                else if (tokLen==1 && *p=='m') { wsprintf(tmp,"%u",pSt->wMinute); src=tmp; }
                else if (tokLen==2 && MyStrnicmp(p,"ss",2)==0) { wsprintf(tmp,"%02u",pSt->wSecond); src=tmp; }
                else if (tokLen==1 && *p=='s') { wsprintf(tmp,"%u",pSt->wSecond); src=tmp; }
                else if (*p == 't') {
                    const char FAR *marker = (pSt->wHour < 12) ? szAm : szPm;
                    if (tokLen==2) src = marker; else { *out++ = *marker; f += tokLen; remain--; continue; }
                }
                if (src) { while (*src && remain > 0) { *out++ = *src++; remain--; } }
                f += tokLen;
            } else { *out++ = *f++; remain--; }
        }
    }
    *out = '\0'; lstrcpy(lpTimeStr, outBuf); return lstrlen(lpTimeStr);
}

/* ================================================================== */
BOOL WINAPI SetLocaleInfoA(LCID Locale, LCTYPE LCType, LPCSTR lpLCData) {
    if (Locale == 0) Locale = LOCALE_USER_DEFAULT;
    if (Locale != LOCALE_USER_DEFAULT) return FALSE;
    switch (LCType) {
    case LOCALE_SLONGDATE:       return WriteProfileString("intl","sLongDate",lpLCData);
    case LOCALE_SSHORTDATE:      return WriteProfileString("intl","sShortDate",lpLCData);
    case LOCALE_SDATE:           return WriteProfileString("intl","sDate",lpLCData);
    case LOCALE_STIME:           return WriteProfileString("intl","sTime",lpLCData);
    case LOCALE_S1159: return WriteProfileString("intl","s1159",lpLCData);
    case LOCALE_S2359: return WriteProfileString("intl","s2359",lpLCData);
    case LOCALE_SCURRENCY:       return WriteProfileString("intl","sCurrency",lpLCData);
    case LOCALE_INEGCURR:        return WriteProfileString("intl","iNegCurr",lpLCData);
    case LOCALE_SDECIMAL:        return WriteProfileString("intl","sDecimal",lpLCData);
    case LOCALE_STHOUSAND:       return WriteProfileString("intl","sThousand",lpLCData);
    case LOCALE_SLIST:           return WriteProfileString("intl","sList",lpLCData);
    case LOCALE_SCOUNTRY:        return WriteProfileString("intl","sCountry",lpLCData);
    case LOCALE_SLANGUAGE:       return WriteProfileString("intl","sLanguage",lpLCData);
    case LOCALE_SKEYBOARDSTOINSTALL: return WriteProfileString("intl","sKeyboard",lpLCData);
    case LOCALE_ILZERO:          return WriteProfileString("intl","iLzero",lpLCData);
    case LOCALE_ICURRDIGITS:     return WriteProfileString("intl","iCurrDigits",lpLCData);
    case LOCALE_ICURRENCY:       return WriteProfileString("intl","iCurrency",lpLCData);
    case LOCALE_IMEASURE:        return WriteProfileString("intl","iMeasure",lpLCData);
    case LOCALE_ITIME:           return WriteProfileString("intl","iTime",lpLCData);
    case LOCALE_ITLZERO:         return WriteProfileString("intl","iTLZero",lpLCData);
    case LOCALE_IDATE:           return WriteProfileString("intl","iDate",lpLCData);
    case LOCALE_ICOUNTRY:        return WriteProfileString("intl","iCountry",lpLCData);
    default: return FALSE;
    }
}

/* ================================================================== */
LCID WINAPI GetThreadLocale(void) { return LOCALE_USER_DEFAULT; }
BOOL WINAPI SetThreadLocale(LCID Locale) { return FALSE; }
LCID WINAPI GetSystemDefaultLCID(void) { return LOCALE_SYSTEM_DEFAULT; }
LCID WINAPI GetUserDefaultLCID(void) { return LOCALE_USER_DEFAULT; }

/* ------------------------------------------------------------------ */
/* EnumSystemLocalesA Ц перебирает все страны из [country]            */
/* ------------------------------------------------------------------ */
BOOL WINAPI EnumSystemLocalesA(LOCALE_ENUMPROCA lpLocaleEnumProc, DWORD dwFlags)
{
    char szPath[144];
    OFSTRUCT of;
    HFILE hFile;
    static char szReadBuf[512];
    static char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;
    BOOL bInCountry = FALSE;
    BOOL bContinue = TRUE;
    char FAR *p;
    char FAR *q;
    char FAR *p1;
    char FAR *p2;
    static char szParams[256];
    char szCode[16];
    char szLang[8];
    int code;
    LCID lcid;
    char szLCID[16];
    int len;
    int pos;
    int totalLen;
    int langLen;

    if (!lpLocaleEnumProc) return FALSE;

    GetSystemDirectory(szPath, sizeof(szPath) - 20);
    lstrcat(szPath, "\\SETUP.INF");

    hFile = OpenFile(szPath, &of, OF_READ);
    if (hFile == HFILE_ERROR) return FALSE;

    while (!bEOF && bContinue) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) {
                szLine[nLinePos] = '\0';
                goto parse_enum;
            }
            break;
        }
        for (i = 0; i < nRead && bContinue; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
parse_enum:
                if (nLinePos > 0) {
                    p = szLine;
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p == '[') {
                        q = _fstrchr(p + 1, ']');
                        if (q) {
                            *q = '\0';
                            bInCountry = (lstrcmpi(p + 1, "country") == 0);
                        }
                    } else if (bInCountry && *p != '\0' && *p != ';') {
                        p1 = _fstrchr(p, '\"');
                        if (p1) {
                            p2 = _fstrchr(p1 + 1, '\"');
                            if (p2) {
                                p1 = _fstrchr(p2 + 1, '\"');
                                if (p1) {
                                    p2 = _fstrchr(p1 + 1, '\"');
                                    if (p2) {
                                        len = 0;
                                        {
                                            char FAR *tmp = p1 + 1;
                                            while (tmp < p2 && len < 255) {
                                                len++;
                                                tmp++;
                                            }
                                        }
                                        _fmemcpy(szParams, p1 + 1, len);
                                        szParams[len] = '\0';

                                        /* извлекаем код страны (первое поле) */
                                        pos = 0;
                                        while (pos < len && szParams[pos] != '!' && pos < 15)
                                        {
                                            szCode[pos] = szParams[pos];
                                            pos++;
                                        }
                                        szCode[pos] = '\0';
                                        code = AtoiFar(szCode);

                                        /* извлекаем код €зыка (последнее поле после последнего '!') */
                                        totalLen = lstrlen(szParams);
                                        pos = totalLen - 1;
                                        while (pos >= 0 && szParams[pos] != '!') {
                                            pos--;
                                        }
                                        if (pos >= 0 && pos < totalLen - 1) {
                                            langLen = 0;
                                            pos++;
                                            while (pos < totalLen && langLen < 7) {
                                                szLang[langLen++] = szParams[pos++];
                                            }
                                            szLang[langLen] = '\0';
                                        } else {
                                            lstrcpy(szLang, "eng");
                                        }

                                        lcid = LookupLCID(code, szLang);
                                        wsprintf(szLCID, "%08lX", lcid);
                                        if (!lpLocaleEnumProc(szLCID))
                                            bContinue = FALSE;
                                    }
                                }
                            }
                        }
                    }
                }
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') i++;
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1)
                    szLine[nLinePos++] = c;
            }
        }
    }

    _lclose(hFile);
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* EnumUILanguagesA Ц перебирает все €зыки из [language]              */
/* ------------------------------------------------------------------ */
BOOL WINAPI EnumUILanguagesA(UILANGUAGE_ENUMPROCA lpUILanguageEnumProc, DWORD dwFlags, LONG lParam)
{
    char szPath[144];
    OFSTRUCT of;
    HFILE hFile;
    static char szReadBuf[512];
    static char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;
    BOOL bInLang = FALSE;
    BOOL bContinue = TRUE;
    char FAR *p;
    char FAR *q;
    char FAR *p1;
    char FAR *p2;
    char szLangName[64];
    int len;

    if (!lpUILanguageEnumProc) return FALSE;

    GetSystemDirectory(szPath, sizeof(szPath) - 20);
    lstrcat(szPath, "\\SETUP.INF");

    hFile = OpenFile(szPath, &of, OF_READ);
    if (hFile == HFILE_ERROR) return FALSE;

    while (!bEOF && bContinue) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) {
                szLine[nLinePos] = '\0';
                goto lang_parse;
            }
            break;
        }
        for (i = 0; i < nRead && bContinue; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
lang_parse:
                if (nLinePos > 0) {
                    p = szLine;
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p == '[') {
                        q = _fstrchr(p + 1, ']');
                        if (q) {
                            *q = '\0';
                            bInLang = (lstrcmpi(p + 1, "language") == 0);
                        }
                    } else if (bInLang && *p != '\0' && *p != ';') {
                        p1 = _fstrchr(p, '\"');
                        if (p1) {
                            p2 = _fstrchr(p1 + 1, '\"');
                            if (p2) {
                                len = 0;
                                {
                                    char FAR *tmp = p1 + 1;
                                    while (tmp < p2 && len < 63) {
                                        len++;
                                        tmp++;
                                    }
                                }
                                _fmemcpy(szLangName, p1 + 1, len);
                                szLangName[len] = '\0';
                                if (!lpUILanguageEnumProc(szLangName, lParam))
                                    bContinue = FALSE;
                            }
                        }
                    }
                }
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') i++;
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1)
                    szLine[nLinePos++] = c;
            }
        }
    }

    _lclose(hFile);
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* EnumDateFormatsA Ц отдаЄт короткий и длинный форматы даты         */
/* ------------------------------------------------------------------ */
BOOL WINAPI EnumDateFormatsA(DATEFMT_ENUMPROCA lpDateFmtEnumProc, LCID Locale, DWORD dwFlags)
{
    char szFormat[80];
    if (!lpDateFmtEnumProc) return FALSE;

    if (GetLocaleInfoA(Locale, LOCALE_SSHORTDATE, szFormat, sizeof(szFormat)))
        if (!lpDateFmtEnumProc(szFormat)) return TRUE;

    if (GetLocaleInfoA(Locale, LOCALE_SLONGDATE, szFormat, sizeof(szFormat)))
        lpDateFmtEnumProc(szFormat);

    return TRUE;
}

/* ------------------------------------------------------------------ */
/* EnumTimeFormatsA Ц возвращает формат времени дл€ текущей локали   */
/* ------------------------------------------------------------------ */
BOOL WINAPI EnumTimeFormatsA(TIMEFMT_ENUMPROCA lpTimeFmtEnumProc, LCID Locale, DWORD dwFlags)
{
    char szFormat[80];
    int iTime;
    char szTimeSep[4];
    int bTLZero;
    int val;

    if (!lpTimeFmtEnumProc) return FALSE;

    GetLocaleInfoA(Locale, LOCALE_STIME, szTimeSep, sizeof(szTimeSep));
    GetLocaleInfoA(Locale, LOCALE_ITIME | LOCALE_RETURN_NUMBER, (LPSTR)&val, sizeof(val));
    iTime = val;
    GetLocaleInfoA(Locale, LOCALE_ITLZERO | LOCALE_RETURN_NUMBER, (LPSTR)&val, sizeof(val));
    bTLZero = val;

    if (iTime == 0) {
        wsprintf(szFormat, "%s%smm%sss tt",
                 bTLZero ? "hh" : "h",
                 szTimeSep, szTimeSep);
    } else {
        wsprintf(szFormat, "%s%smm%sss",
                 bTLZero ? "HH" : "H",
                 szTimeSep, szTimeSep);
    }
    lpTimeFmtEnumProc(szFormat);
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* EnumCalendarInfoA Ц перебирает календарную информацию              */
/* ------------------------------------------------------------------ */
BOOL WINAPI EnumCalendarInfoA(CALINFO_ENUMPROCA lpCalInfoEnumProc, LCID Locale, CALID Calendar, CALTYPE CalType)
{
    char szBuf[80];
    int i;

    if (!lpCalInfoEnumProc) return FALSE;
    if (Calendar != CAL_GREGORIAN) return FALSE;

    switch (CalType) {
        case CAL_ICALINTVALUE:
            wsprintf(szBuf, "%d", CAL_GREGORIAN);
            lpCalInfoEnumProc(szBuf);
            break;
        case CAL_SCALNAME:
            lstrcpy(szBuf, "Gregorian");
            lpCalInfoEnumProc(szBuf);
            break;
        case CAL_SDAYNAME1: case CAL_SDAYNAME2: case CAL_SDAYNAME3:
        case CAL_SDAYNAME4: case CAL_SDAYNAME5: case CAL_SDAYNAME6:
        case CAL_SDAYNAME7:
            i = CalType - CAL_SDAYNAME1;
            if (GetLocaleInfoA(Locale, LOCALE_SDAYNAME1 + i, szBuf, sizeof(szBuf)))
                lpCalInfoEnumProc(szBuf);
            break;
        case CAL_SABBREVDAYNAME1: case CAL_SABBREVDAYNAME2: case CAL_SABBREVDAYNAME3:
        case CAL_SABBREVDAYNAME4: case CAL_SABBREVDAYNAME5: case CAL_SABBREVDAYNAME6:
        case CAL_SABBREVDAYNAME7:
            i = CalType - CAL_SABBREVDAYNAME1;
            if (GetLocaleInfoA(Locale, LOCALE_SABBREVDAYNAME1 + i, szBuf, sizeof(szBuf)))
                lpCalInfoEnumProc(szBuf);
            break;
        case CAL_SSHORTDATE:
            GetLocaleInfoA(Locale, LOCALE_SSHORTDATE, szBuf, sizeof(szBuf));
            lpCalInfoEnumProc(szBuf);
            break;
        case CAL_SLONGDATE:
            GetLocaleInfoA(Locale, LOCALE_SLONGDATE, szBuf, sizeof(szBuf));
            lpCalInfoEnumProc(szBuf);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* GetKeyboardLayoutList Ц аналог Win32 API                           */
/* ------------------------------------------------------------------ */
int WINAPI GetKeyboardLayoutList(int nBuff, LPSTR lpList)
{
    char szPath[144];
    OFSTRUCT of;
    HFILE hFile;
    static char szReadBuf[512];
    static char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;
    BOOL bInKbd = FALSE;
    char FAR *p;
    char FAR *q;
    char FAR *p1;
    char FAR *p2;
    char szName[64];
    int len;
    static char szPrevNames[64][64];
    int nPrev = 0;
    int nCount = 0;
    LPSTR lpDest = lpList;
    int nRemain = (lpList && nBuff > 0) ? nBuff : 0;
    int j;
    BOOL bDup;

    GetSystemDirectory(szPath, sizeof(szPath) - 20);
    lstrcat(szPath, "\\SETUP.INF");

    hFile = OpenFile(szPath, &of, OF_READ);
    if (hFile == HFILE_ERROR) return 0;

    while (!bEOF) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) {
                szLine[nLinePos] = '\0';
                goto parse_kbd_list;
            }
            break;
        }
        for (i = 0; i < nRead; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
parse_kbd_list:
                if (nLinePos > 0) {
                    p = szLine;
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p == '[') {
                        q = _fstrchr(p + 1, ']');
                        if (q) {
                            *q = '\0';
                            bInKbd = (lstrcmpi(p + 1, "keyboard.tables") == 0);
                        }
                    } else if (bInKbd && *p != '\0' && *p != ';') {
                        p1 = _fstrchr(p, '\"');
                        if (p1) {
                            p2 = _fstrchr(p1 + 1, '\"');
                            if (p2) {
                                len = 0;
                                {
                                    char FAR *tmp = p1 + 1;
                                    while (tmp < p2 && len < 63) {
                                        len++;
                                        tmp++;
                                    }
                                }
                                _fmemcpy(szName, p1 + 1, len);
                                szName[len] = '\0';
                                bDup = FALSE;
                                for (j = 0; j < nPrev; j++) {
                                    if (lstrcmpi(szPrevNames[j], szName) == 0) {
                                        bDup = TRUE;
                                        break;
                                    }
                                }
                                if (!bDup) {
                                    if (nPrev < 64) {
                                        lstrcpy(szPrevNames[nPrev], szName);
                                        nPrev++;
                                    }
                                }
                            }
                        }
                    }
                }
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') i++;
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1)
                    szLine[nLinePos++] = c;
            }
        }
    }
    _lclose(hFile);

    nCount = nPrev;
    if (!lpList || nBuff <= 0)
        return nCount;

    for (j = 0; j < nCount; j++) {
        int cb = lstrlen(szPrevNames[j]) + 1;
        if (cb <= nRemain) {
            lstrcpy(lpDest, szPrevNames[j]);
            lpDest += cb;
            nRemain -= cb;
        } else {
            break;
        }
    }
    if (nRemain >= 1)
        *lpDest = '\0';

    return nCount;
}

/* ------------------------------------------------------------------ */
/* GetLanguageCodeFromName Ц поиск кода €зыка по полному имени        */
/* ------------------------------------------------------------------ */
BOOL WINAPI GetLanguageCodeFromName(LPCSTR lpszName, LPSTR lpszCode, int cchCode)
{
    char szPath[144];
    OFSTRUCT of;
    HFILE hFile;
    static char szReadBuf[512];
    static char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;
    BOOL bInLang = FALSE;
    BOOL bFound = FALSE;
    char FAR *p;
    char FAR *q;
    char FAR *pEq;
    char FAR *p1;
    char FAR *p2;
    int keyLen;
    char szKey[32];
    int nameLen;

    GetSystemDirectory(szPath, sizeof(szPath) - 20);
    lstrcat(szPath, "\\SETUP.INF");

    hFile = OpenFile(szPath, &of, OF_READ);
    if (hFile == HFILE_ERROR) return FALSE;

    while (!bEOF && !bFound) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) {
                szLine[nLinePos] = '\0';
                goto parse_lang_code;
            }
            break;
        }
        for (i = 0; i < nRead && !bFound; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
parse_lang_code:
                if (nLinePos > 0) {
                    p = szLine;
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p == '[') {
                        q = _fstrchr(p + 1, ']');
                        if (q) {
                            *q = '\0';
                            bInLang = (lstrcmpi(p + 1, "language") == 0);
                        }
                    } else if (bInLang && *p != '\0' && *p != ';') {
                        pEq = _fstrchr(p, '=');
                        if (pEq) {
                            keyLen = 0;
                            {
                                char FAR *tmp = p;
                                while (tmp < pEq && keyLen < 31) {
                                    keyLen++;
                                    tmp++;
                                }
                                while (keyLen > 0 && (p[keyLen-1] == ' ' || p[keyLen-1] == '\t')) keyLen--;
                            }
                            if (keyLen > 0) {
                                if (keyLen > 31) keyLen = 31;
                                _fmemcpy(szKey, p, keyLen);
                                szKey[keyLen] = '\0';

                                p1 = _fstrchr(pEq, '\"');
                                if (p1) {
                                    p2 = _fstrchr(p1 + 1, '\"');
                                    if (p2) {
                                        nameLen = 0;
                                        {
                                            char FAR *tmp = p1 + 1;
                                            while (tmp < p2 && nameLen < 63) {
                                                nameLen++;
                                                tmp++;
                                            }
                                        }
                                        if (lstrlen(lpszName) == nameLen &&
                                            MyStrnicmp(p1 + 1, lpszName, nameLen) == 0) {
                                            StringCopyN(lpszCode, szKey, cchCode);
                                            bFound = TRUE;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') i++;
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1)
                    szLine[nLinePos++] = c;
            }
        }
    }
    _lclose(hFile);
    return bFound;
}
