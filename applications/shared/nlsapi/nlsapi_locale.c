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
    { 82,  "eng", 0xE052 },
    { 886, "eng", 0xE376 },
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

BOOL GetLocaleInfoFromInf(int iCountryCode, const char FAR *szLang,
                          LCTYPE LCType, LPSTR lpLCData, int cchData,
                          BOOL returnNumber)
{
    static char szPath[144];
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
    static char szFirstParam[16];
    static char szFileLang[16];
    int fieldIdx;
    char FAR *pField;
    int cur;
    int len, totalLen, pos, langLen;
    static char szCountryName[64];
    int countryNameLen;

    wsprintf(szTargetCountryCode, "%d", iCountryCode);
    lstrcpy(szTargetLang, szLang);

    /* ----- отладка: вход в функцию ----- */
//    {
//        char szMsg[128];
//        wsprintf(szMsg, "GetLocaleInfoFromInf: country=%d, lang=%s, LCTYPE=0x%08lX",
//                 iCountryCode, (LPSTR)szLang, LCType);
//        MessageBox(0, szMsg, "GetLocaleInfoFromInf", MB_OK);
//    }

    GetSystemDirectory(szPath, sizeof(szPath) - 20);
    lstrcat(szPath, "\\SETUP.INF");
    hFile = OpenFile(szPath, &of, OF_READ);
    if (hFile == HFILE_ERROR) {
//        MessageBox(0, "GetLocaleInfoFromInf: FAILED to open SETUP.INF", "Trace", MB_OK);
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
                        /* ----- отладка: сырая строка ----- */
//                        {
//                            char szMsg[300];
//                            wsprintf(szMsg, "Raw line: %s", (LPSTR)p);
//                            MessageBox(0, szMsg, "GetLocaleInfoFromInf", MB_OK);
//                        }

                        p1 = (char FAR *)_fstrchr(p, '\"');
                        if (p1) {
                            p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                            if (p2) {
                                /* Имя страны (посимвольно) */
                                countryNameLen = 0;
                                {
                                    char FAR *tmp = p1 + 1;
                                    while (tmp < p2 && countryNameLen < 63)
                                        szCountryName[countryNameLen++] = *tmp++;
                                }
                                szCountryName[countryNameLen] = '\0';

                                /* ----- отладка: имя страны ----- */
//                                {
//                                    char szMsg[128];
//                                    wsprintf(szMsg, "Country name: '%s'", (LPSTR)szCountryName);
//                                    MessageBox(0, szMsg, "GetLocaleInfoFromInf", MB_OK);
//                                }

                                p1 = (char FAR *)_fstrchr(p2 + 1, '\"');
                                if (p1) {
                                    p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                                    if (p2) {
                                        /* Параметры (посимвольно) */
                                        len = 0;
                                        {
                                            char FAR *tmp = p1 + 1;
                                            while (tmp < p2 && len < 255) {
                                                szParams[len++] = *tmp++;
                                            }
                                        }
                                        szParams[len] = '\0';

                                        /* ----- отладка: параметры ----- */
//                                        {
//                                            char szMsg[300];
//                                            wsprintf(szMsg, "Params: '%s'", (LPSTR)szParams);
//                                            MessageBox(0, szMsg, "GetLocaleInfoFromInf", MB_OK);
//                                        }

                                        /* Код страны */
                                        pos = 0;
                                        while (pos < len && szParams[pos] != '!' && pos < 15)
                                            szFirstParam[pos] = szParams[pos], pos++;
                                        szFirstParam[pos] = '\0';

                                        /* ----- отладка: код страны и сравнение ----- */
//                                        {
//                                            char szMsg[128];
//                                            wsprintf(szMsg, "First param='%s', target='%s'",
//                                                     (LPSTR)szFirstParam, (LPSTR)szTargetCountryCode);
//                                            MessageBox(0, szMsg, "GetLocaleInfoFromInf", MB_OK);
//                                        }

                                        if (lstrcmp(szFirstParam, szTargetCountryCode) == 0) {
                                            /* Код языка */
                                            totalLen = lstrlen(szParams);
                                            pos = totalLen - 1;
                                            while (pos >= 0 && szParams[pos] != '!') pos--;
                                            if (pos >= 0 && pos < totalLen - 1) {
                                                langLen = 0; pos++;
                                                while (pos < totalLen && langLen < 15)
                                                    szFileLang[langLen++] = szParams[pos++];
                                                szFileLang[langLen] = '\0';
                                            } else lstrcpy(szFileLang, "eng");

                                            /* ----- отладка: код языка и сравнение ----- */
//                                            {
//                                                char szMsg[128];
//                                                wsprintf(szMsg, "File lang='%s', target lang='%s'",
//                                                         (LPSTR)szFileLang, (LPSTR)szTargetLang);
//                                                MessageBox(0, szMsg, "GetLocaleInfoFromInf", MB_OK);
//                                            }

                                            if (lstrcmpi(szFileLang, szTargetLang) == 0) {
//                                                MessageBox(0, "MATCH FOUND", "GetLocaleInfoFromInf", MB_OK);

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

    if (!bFound) {
//        MessageBox(0, "GetLocaleInfoFromInf: NOT FOUND", "Trace", MB_OK);
    } else {
//        MessageBox(0, "GetLocaleInfoFromInf: SUCCESS", "Trace", MB_OK);
    }
    return bFound;
}

int WINAPI __export GetKeyboardLayoutList(int nBuff, LPSTR lpList)
{
    static char szPath[144];
    OFSTRUCT of;
    HFILE hFile;
    static char szReadBuf[512];
    static char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;
    BOOL bInKbd = FALSE;
    char FAR *p, FAR *q, FAR *p1, FAR *p2;
    static char szName[64];
    int len;
    static char szPrevNames[64][64];
    int nPrev = 0, nCount = 0;
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
            if (nLinePos > 0) { szLine[nLinePos] = '\0'; goto parse_kbd; }
            break;
        }
        for (i = 0; i < nRead; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
parse_kbd:
                if (nLinePos > 0) {
                    p = szLine; while (*p == ' ' || *p == '\t') p++;
                    if (*p == '[') {
                        q = (char FAR *)_fstrchr(p + 1, ']');
                        if (q) { *q = '\0'; bInKbd = (lstrcmpi(p + 1, "keyboard.tables") == 0); }
                    } else if (bInKbd && *p != '\0' && *p != ';') {
                        p1 = (char FAR *)_fstrchr(p, '\"');
                        if (p1) {
                            p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                            if (p2) {
                                len = 0;
                                { char FAR *tmp = p1 + 1; while (tmp < p2 && len < 63) szName[len++] = *tmp++; }
                                szName[len] = '\0';
                                bDup = FALSE;
                                for (j = 0; j < nPrev; j++) {
                                    if (lstrcmpi(szPrevNames[j], szName) == 0) { bDup = TRUE; break; }
                                }
                                if (!bDup) {
                                    if (nPrev < 64) { lstrcpy(szPrevNames[nPrev], szName); nPrev++; }
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

    nCount = nPrev;
    if (!lpList || nBuff <= 0) return nCount;
    for (j = 0; j < nCount; j++) {
        int cb = lstrlen(szPrevNames[j]) + 1;
        if (cb <= nRemain) { lstrcpy(lpDest, szPrevNames[j]); lpDest += cb; nRemain -= cb; }
        else break;
    }
    if (nRemain >= 1) *lpDest = '\0';
    return nCount;
}
