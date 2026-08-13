/*
 * nlsapi_enum.c Ц функции перечислени€ NLS API
 * ”силенна€ трассировка кода страны, C89.
 */
#include "nlsapi_internal.h"

#if 0
BOOL WINAPI DECLSPEC EnumSystemLocalesA(LOCALE_ENUMPROCA lpLocaleEnumProc, DWORD dwFlags)
{
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
    static char szParams[256];
    static char szCountryName[64];
    static char szLang[8];
    static int code;
    LCID lcid;
    char szLCID[16];
    int len;
    int pos;
    int totalLen;
    int langLen;
    int nameLen;

    if (!lpLocaleEnumProc) return FALSE;

    hFile = OpenSetupInf();
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
                        q = (char FAR *)_fstrchr(p + 1, ']');
                        if (q) {
                            *q = '\0';
                            bInCountry = (lstrcmpi(p + 1, "country") == 0);
                        }
                    } else if (bInCountry && *p != '\0' && *p != ';') {
                        ParseCountryLine(p, (LPSTR)szCountryName, sizeof(szCountryName), (int FAR *)&code, (LPSTR)szLang, sizeof(szLang), (LPSTR)szParams, sizeof(szParams));
                        lcid = LookupLCID(code, (LPCSTR)szLang);
                        wsprintf(szLCID, "%08lX", lcid);
                        if (!lpLocaleEnumProc(szLCID)) {
                          bContinue = FALSE;
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
#endif
#if 0
BOOL WINAPI DECLSPEC EnumSystemLocalesA(LOCALE_ENUMPROCA lpLocaleEnumProc, DWORD dwFlags)
{
    LPINF_SECTION sec;
    int i, count;
    LPCSTR line;
    static char szCountryName[64];
    static char szParams[256];
    static char szLang[16];
    static int code;
    LCID lcid;
    char szLCID[16];

    MessageBox(0, "EnumSystemLocalesA called", "TRACE", MB_OK);

    if (!lpLocaleEnumProc || !g_hInf) return FALSE;

    sec = InfFindSection(g_hInf, "country");
    if (!sec) return FALSE;

    count = InfGetLineCount(sec);
    for (i = 0; i < count; i++)
    {
        line = InfGetLine(sec, i);
        if (!line) continue;
        while (*line == ' ' || *line == '\t') line++;
        if (*line == ';' || *line == '\0') continue;

        ParseCountryLine(line, (LPSTR)szCountryName, sizeof(szCountryName),
                         (int FAR *)&code, (LPSTR)szLang, sizeof(szLang),
                         (LPSTR)szParams, sizeof(szParams));
        lcid = LookupLCID(code, (LPCSTR)szLang);
        wsprintf(szLCID, "%08lX", lcid);
        if (!lpLocaleEnumProc(szLCID))
            break;
    }
    return TRUE;
}
#endif

BOOL WINAPI DECLSPEC EnumSystemLocalesA(LOCALE_ENUMPROCA lpLocaleEnumProc, DWORD dwFlags)
{
    LPINF_SECTION sec;
    int i, count;
    LPCSTR line;
    static COUNTRY_ENTRY entry;
    LCID lcid;
    char szLCID[16];

    if (!lpLocaleEnumProc || !g_hInf) return FALSE;

    sec = InfFindSection(g_hInf, "country");
    if (!sec) return FALSE;

    count = InfGetLineCount(sec);
    for (i = 0; i < count; i++)
    {
        line = InfGetLine(sec, i);
        if (!line) continue;

        if (InfParseCountryLine(line, &entry))
        {
            lcid = LookupLCID(entry.ICOUNTRY, (LPCSTR)entry.lang);
            wsprintf(szLCID, "%08lX", lcid);
            if (!lpLocaleEnumProc(szLCID))
            {
                InfFreeCountryEntry(&entry);
                break;
            }
            InfFreeCountryEntry(&entry);
        }
    }
    return TRUE;
}

#if 0
BOOL WINAPI DECLSPEC EnumUILanguagesA(UILANGUAGE_ENUMPROCA lpUILanguageEnumProc, DWORD dwFlags, LONG lParam)
{
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
    static char szLangName[64];
    int len;
    static char szKey[32];

    if (!lpUILanguageEnumProc) return FALSE;

    hFile = OpenSetupInf();
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
                        q = (char FAR *)_fstrchr(p + 1, ']');
                        if (q) {
                            *q = '\0';
                            bInLang = (lstrcmpi(p + 1, "language") == 0);
                        }
                    } else if (bInLang && *p != '\0' && *p != ';') {
                        if (dwFlags == MUI_LANGUAGE_ID) {
                            char FAR *pEq = (char FAR *)_fstrchr(p, '=');
                            if (pEq) {
                                int keyLen = (int)(pEq - p);
                                while (keyLen > 0 && (p[keyLen-1] == ' ' || p[keyLen-1] == '\t')) keyLen--;
                                if (keyLen > 0) {
                                    if (keyLen > 31) keyLen = 31;
                                    {
                                        int k;
                                        for (k = 0; k < keyLen; k++) szKey[k] = p[k];
                                        szKey[k] = '\0';
                                    }
                                    if (!lpUILanguageEnumProc(szKey, lParam))
                                        bContinue = FALSE;
                                }
                            }
                        } else { /* MUI_LANGUAGE_NAME */
                            p1 = (char FAR *)_fstrchr(p, '\"');
                            if (p1) {
                                p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                                if (p2) {
                                    len = 0;
                                    {
                                        char FAR *tmp = p1 + 1;
                                        while (tmp < p2 && len < 63) {
                                            szLangName[len++] = *tmp++;
                                        }
                                    }
                                    szLangName[len] = '\0';
                                    if (!lpUILanguageEnumProc(szLangName, lParam))
                                        bContinue = FALSE;
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

#endif

#if 0
BOOL WINAPI DECLSPEC EnumUILanguagesA(UILANGUAGE_ENUMPROCA lpUILanguageEnumProc,
                                      DWORD dwFlags, LONG lParam)
{
    LPINF_SECTION sec;
    int i, count;
    LPCSTR line;
    static char szLangName[64];
    static char szKey[32];

    if (!lpUILanguageEnumProc || !g_hInf) return FALSE;

    sec = InfFindSection(g_hInf, "language");
    if (!sec) return FALSE;

    count = InfGetLineCount(sec);
    for (i = 0; i < count; i++)
    {
        line = InfGetLine(sec, i);
        if (!line) continue;
        while (*line == ' ' || *line == '\t') line++;
        if (*line == ';' || *line == '\0') continue;

        if (dwFlags == MUI_LANGUAGE_ID)
        {
            char FAR *pEq = (char FAR *)_fstrchr(line, '=');
            if (pEq)
            {
                int keyLen = (int)(pEq - line);
                while (keyLen > 0 && (line[keyLen-1] == ' ' || line[keyLen-1] == '\t'))
                    keyLen--;
                if (keyLen > 0)
                {
                    if (keyLen > 31) keyLen = 31;
                    {
                        int k;
                        for (k = 0; k < keyLen; k++) szKey[k] = line[k];
                        szKey[k] = '\0';
                    }
                    if (!lpUILanguageEnumProc(szKey, lParam))
                        return TRUE;
                }
            }
        }
        else /* MUI_LANGUAGE_NAME */
        {
            char FAR *p1 = (char FAR *)_fstrchr(line, '\"');
            if (p1)
            {
                char FAR *p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                if (p2)
                {
                    int len = 0;
                    char FAR *tmp = p1 + 1;
                    while (tmp < p2 && len < 63)
                        szLangName[len++] = *tmp++;
                    szLangName[len] = '\0';
                    if (!lpUILanguageEnumProc(szLangName, lParam))
                        return TRUE;
                }
            }
        }
    }
    return TRUE;
}
#endif

BOOL WINAPI DECLSPEC EnumUILanguagesA(UILANGUAGE_ENUMPROCA lpUILanguageEnumProc,
                                      DWORD dwFlags, LONG lParam)
{
    LPINF_SECTION sec;
    int i, count;
    LPCSTR line;
    static LANGUAGE_ENTRY entry;

    if (!lpUILanguageEnumProc || !g_hInf) return FALSE;

    sec = InfFindSection(g_hInf, "language");
    if (!sec) return FALSE;

    count = InfGetLineCount(sec);
    for (i = 0; i < count; i++)
    {
        line = InfGetLine(sec, i);
        if (!line) continue;

        if (InfParseLanguageLine(line, &entry))
        {
            if (dwFlags == MUI_LANGUAGE_ID)
            {
                if (!lpUILanguageEnumProc(entry.code, lParam))
                {
                    InfFreeLanguageEntry(&entry);
                    break;
                }
            }
            else /* MUI_LANGUAGE_NAME */
            {
                if (entry.description)
                {
                    if (!lpUILanguageEnumProc(entry.description, lParam))
                    {
                        InfFreeLanguageEntry(&entry);
                        break;
                    }
                }
            }
            InfFreeLanguageEntry(&entry);
        }
    }
    return TRUE;
}

BOOL WINAPI DECLSPEC EnumDateFormatsA(DATEFMT_ENUMPROCA lpDateFmtEnumProc, LCID Locale, DWORD dwFlags)
{
    char szFormat[80];
    if (!lpDateFmtEnumProc) return FALSE;

    if (GetLocaleInfo(Locale, LOCALE_SSHORTDATE, szFormat, sizeof(szFormat)))
        if (!lpDateFmtEnumProc(szFormat)) return TRUE;

    if (GetLocaleInfo(Locale, LOCALE_SLONGDATE, szFormat, sizeof(szFormat)))
        lpDateFmtEnumProc(szFormat);

    return TRUE;
}

BOOL WINAPI DECLSPEC EnumTimeFormatsA(TIMEFMT_ENUMPROCA lpTimeFmtEnumProc, LCID Locale, DWORD dwFlags)
{
    char szFormat[80];
    int iTime;
    char szTimeSep[4];
    int bTLZero;
    int val;

    if (!lpTimeFmtEnumProc) return FALSE;

    GetLocaleInfo(Locale, LOCALE_STIME, szTimeSep, sizeof(szTimeSep));
    GetLocaleInfo(Locale, LOCALE_ITIME | LOCALE_RETURN_NUMBER, (LPSTR)&val, sizeof(val));
    iTime = val;
    GetLocaleInfo(Locale, LOCALE_ITLZERO | LOCALE_RETURN_NUMBER, (LPSTR)&val, sizeof(val));
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

BOOL WINAPI DECLSPEC EnumCalendarInfoA(CALINFO_ENUMPROCA lpCalInfoEnumProc, LCID Locale, CALID Calendar, CALTYPE CalType)
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
            if (GetLocaleInfo(Locale, LOCALE_SDAYNAME1 + i, szBuf, sizeof(szBuf)))
                lpCalInfoEnumProc(szBuf);
            break;
        case CAL_SABBREVDAYNAME1: case CAL_SABBREVDAYNAME2: case CAL_SABBREVDAYNAME3:
        case CAL_SABBREVDAYNAME4: case CAL_SABBREVDAYNAME5: case CAL_SABBREVDAYNAME6:
        case CAL_SABBREVDAYNAME7:
            i = CalType - CAL_SABBREVDAYNAME1;
            if (GetLocaleInfo(Locale, LOCALE_SABBREVDAYNAME1 + i, szBuf, sizeof(szBuf)))
                lpCalInfoEnumProc(szBuf);
            break;
        case CAL_SSHORTDATE:
            GetLocaleInfo(Locale, LOCALE_SSHORTDATE, szBuf, sizeof(szBuf));
            lpCalInfoEnumProc(szBuf);
            break;
        case CAL_SLONGDATE:
            GetLocaleInfo(Locale, LOCALE_SLONGDATE, szBuf, sizeof(szBuf));
            lpCalInfoEnumProc(szBuf);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}
