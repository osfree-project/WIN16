/*
 * nlsapi_locale.c – работа с SETUP.INF, таблица LCID, GetKeyboardLayoutList
 *                   (с отладкой в GetLocaleInfoFromInf)
 */
#include "nlsapi_internal.h"

/* Таблица сопоставления */
const KNOWN_LCID knownLCIDs[] = {
    { 1,   "usa", 0x0409 },
    { 44,  "eng", 0x0809 },
    { 61,  "eng", 0x0C09 },
    { 64,  "eng", 0x1409 },
    { 2,   "eng", 0x1009 },
    { 2,   "frn", 0x0C0C },
    { 33,  "frn", 0x040C },
    { 49,  "ger", 0x0407 },
    { 39,  "itn", 0x0410 },
    { 34,  "spa", 0x0C0A },
    { 52,  "spa", 0x080A },
    { 46,  "swe", 0x041D },
    { 358, "fin", 0x040B },
    { 45,  "dan", 0x0406 },
    { 47,  "nor", 0x0414 },
    { 31,  "dut", 0x0413 },
    { 32,  "dut", 0x0813 },
    { 32,  "frn", 0x080C },
    { 351, "por", 0x0816 },
    { 55,  "por", 0x0416 },
    { 354, "ice", 0x040F },
    { 43,  "ger", 0x0C07 },
    { 41,  "frn", 0x100C },
    { 41,  "ger", 0x0807 },
    { 41,  "itn", 0x0810 },
    { 82,  "eng", 0xE052 }, // @@todo fix
    { 886, "eng", 0xE376 }, // @@todo fix
};

const int KNOWN_LCID_COUNT = sizeof(knownLCIDs) / sizeof(knownLCIDs[0]);

LCID LookupLCID(int countryCode, const char FAR *szLang)
{
    int i;
    for (i = 0; i < KNOWN_LCID_COUNT; i++) {
        if (knownLCIDs[i].iCountry == countryCode &&
            lstrcmpi(knownLCIDs[i].szLang, szLang) == 0)
            return knownLCIDs[i].lcid;
    }
    return (LCID)(0xE000 + (WORD)countryCode);
}

#if 0
BOOL GetLocaleInfoFromInf(int iCountryCode, const char FAR *szLang,
                          LCTYPE LCType, LPSTR lpLCData, int cchData,
                          BOOL returnNumber)
{
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
    static char szFirstParam[16];
    static char szFileLang[16];
    int fieldIdx;
    char FAR *pField;
    int cur;
    static int code;
    int len, totalLen, pos, langLen;
    static char szCountryName[64];
    int countryNameLen;

    wsprintf(szTargetCountryCode, "%d", iCountryCode);
    lstrcpy(szTargetLang, szLang);

    hFile = OpenSetupInf();
    if (hFile == HFILE_ERROR) {
        return FALSE;
    }

    while (!bEOF && !bFound) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) { szLine[nLinePos] = '\0'; goto parse_line; }
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
                        q = (char FAR *)_fstrchr(p + 1, ']');
                        if (q) { *q = '\0'; bInCountry = (lstrcmpi(p + 1, "country") == 0); }
                    } else if (bInCountry && *p != '\0' && *p != ';') {

                        ParseCountryLine(p, (LPSTR)szCountryName, sizeof(szCountryName), (int FAR *)&code, (LPSTR)szFileLang, sizeof(szFileLang), (LPSTR)szParams, sizeof(szParams));

                        if (code==iCountryCode) {
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

                                    if (fieldIdx >= 0) {
                                        pField = szParams; cur = 0;
                                        while (cur < fieldIdx && *pField) {
                                            pField = (char FAR *)_fstrchr(pField, '!');
                                            if (pField) { pField++; cur++; } else break;
                                        }
                                        if (pField) {
                                            char FAR *pEnd = (char FAR *)_fstrchr(pField, '!');
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
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') i++;
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1) szLine[nLinePos++] = c;
            }
        }
    }
    _lclose(hFile);

    return bFound;
}
#endif

#if 0
BOOL GetLocaleInfoFromInf(int iCountryCode, const char FAR *szLang,
                          LCTYPE LCType, LPSTR lpLCData, int cchData,
                          BOOL returnNumber)
{
    LPINF_SECTION sec;
    int i, count;
    LPCSTR line;
    char szTargetCountryCode[16];
    static char szCountryName[64];
    static char szParams[256];
    static char szFileLang[16];
    static int code;
    int fieldIdx;
    char FAR *pField;
    int cur;

    if (!g_hInf) return FALSE;

    sec = InfFindSection(g_hInf, "country");
    if (!sec) return FALSE;

    wsprintf(szTargetCountryCode, "%d", iCountryCode);
    count = InfGetLineCount(sec);

    for (i = 0; i < count; i++)
    {
        line = InfGetLine(sec, i);
        if (!line) continue;

        while (*line == ' ' || *line == '\t') line++;
        if (*line == ';' || *line == '\0') continue;

        ParseCountryLine(line, (LPSTR)szCountryName, sizeof(szCountryName),
                         (int FAR *)&code, (LPSTR)szFileLang, sizeof(szFileLang),
                         (LPSTR)szParams, sizeof(szParams));

        if (code == iCountryCode && lstrcmpi(szFileLang, szLang) == 0)
        {
            if (LCType == LOCALE_SCOUNTRY)
            {
                StringCopyN(lpLCData, szCountryName, cchData);
                return TRUE;
            }

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

            if (fieldIdx >= 0)
            {
                pField = szParams;
                cur = 0;
                while (cur < fieldIdx && *pField)
                {
                    pField = (char FAR *)_fstrchr(pField, '!');
                    if (pField) { pField++; cur++; }
                    else break;
                }
                if (pField)
                {
                    char FAR *pEnd = (char FAR *)_fstrchr(pField, '!');
                    if (pEnd) *pEnd = '\0';
                    if (returnNumber)
                    {
                        if (cchData >= (int)sizeof(int))
                        {
                            *(int FAR*)lpLCData = AtoiFar(pField);
                            return TRUE;
                        }
                    }
                    else
                    {
                        StringCopyN(lpLCData, pField, cchData);
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}

#endif

BOOL GetLocaleInfoFromInf(int iCountryCode, const char FAR *szLang,
                          LCTYPE LCType, LPSTR lpLCData, int cchData,
                          BOOL returnNumber)
{
    LPINF_SECTION sec;
    int i, count;
    LPCSTR line;
    static COUNTRY_ENTRY entry;
//    static char dbg[512];

    if (!g_hInf) return FALSE;

    sec = InfFindSection(g_hInf, "country");
    if (!sec) return FALSE;

    count = InfGetLineCount(sec);
    for (i = 0; i < count; i++)
    {
        line = InfGetLine(sec, i);
        if (!line) continue;

//        wsprintf(dbg, "Line %d: parsing...", i);
//        MessageBox(0, dbg, "TRACE", MB_OK);

        if (InfParseCountryLine(line, &entry))
        {
//            wsprintf(dbg, "Parsed: ICOUNTRY=%d, lang='%s'",
//                     entry.ICOUNTRY,
//                     entry.lang ? entry.lang : "NULL");
//            MessageBox(0, dbg, "TRACE", MB_OK);

            if (entry.ICOUNTRY == iCountryCode &&
                lstrcmpi(entry.lang, szLang) == 0)
            {
//                MessageBox(0, "Match found!", "TRACE", MB_OK);

                if (LCType == LOCALE_SCOUNTRY) {
                    if (entry.name)
                        StringCopyN(lpLCData, entry.name, cchData);
                    InfFreeCountryEntry(&entry);
                    return TRUE;
                }

                if (returnNumber && cchData >= (int)sizeof(int)) {
                    int value = 0;
                    switch (LCType) {
                    case LOCALE_ICOUNTRY:    value = entry.ICOUNTRY;    break;
                    case LOCALE_ICURRDIGITS: value = entry.ICURRDIGITS; break;
                    case LOCALE_ICURRENCY:   value = entry.ICURRENCY;   break;
                    case LOCALE_IDATE:       value = entry.IDATE;       break;
                    case LOCALE_IMEASURE:    value = entry.IMEASURE;    break;
                    case LOCALE_INEGCURR:    value = entry.INEGCURR;    break;
                    case LOCALE_ITIME:       value = entry.ITIME;       break;
                    case LOCALE_ITLZERO:     value = entry.ITLZERO;     break;
                    case LOCALE_ILZERO:      value = entry.ILZERO;      break;
                    case LOCALE_IDIGITS:     value = entry.IDIGITS;     break;
                    default: InfFreeCountryEntry(&entry); return FALSE;
                    }
                    *(int FAR*)lpLCData = value;
                    InfFreeCountryEntry(&entry);
                    return TRUE;
                }
                else if (!returnNumber) {
                    LPSTR str = NULL;
                    switch (LCType) {
                    case LOCALE_S1159:      str = entry.S1159;      break;
                    case LOCALE_S2359:      str = entry.S2359;      break;
                    case LOCALE_SCURRENCY:  str = entry.SCURRENCY;  break;
                    case LOCALE_STHOUSAND:  str = entry.STHOUSAND;  break;
                    case LOCALE_SDECIMAL:   str = entry.SDECIMAL;   break;
                    case LOCALE_SDATE:      str = entry.SDATE;      break;
                    case LOCALE_STIME:      str = entry.STIME;      break;
                    case LOCALE_SLIST:      str = entry.SLIST;      break;
                    case LOCALE_SSHORTDATE: str = entry.SSHORTDATE; break;
                    case LOCALE_SLONGDATE:  str = entry.SLONGDATE;  break;
                    default: InfFreeCountryEntry(&entry); return FALSE;
                    }
                    if (str) StringCopyN(lpLCData, str, cchData);
                    InfFreeCountryEntry(&entry);
                    return TRUE;
                }
            }
        }
        InfFreeCountryEntry(&entry);
    }

    return FALSE;
}
