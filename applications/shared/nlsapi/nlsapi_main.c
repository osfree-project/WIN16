/*
 * nlsapi_main.c Ц основные API NLS: GetLocaleInfo, SetLocaleInfo,
 *                 GetDateFormat, GetTimeFormat, GetThreadLocale и пр.
 */
#include "nlsapi_internal.h"
#include <dos.h>

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

/* ---------- вспомогательные функции (формулы и сравнение) ---------- */
int MyStrnicmp(const char FAR *s1, const char FAR *s2, int n) {
    while (n-- > 0) { char c1=*s1++,c2=*s2++;
        if(c1>='A'&&c1<='Z') c1+='a'-'A'; if(c2>='A'&&c2<='Z') c2+='a'-'A';
        if(c1!=c2) return (unsigned char)c1-(unsigned char)c2;
        if(c1=='\0') return 0;
    }
    return 0;
}
static int DayOfWeek(int y, int m, int d) {
    static int t[]={0,3,2,5,0,3,5,1,4,6,2,4}; y-=m<3;
    return (y+y/4-y/100+y/400+t[m-1]+d)%7;
}
static void GetCurrentDateTime(SYSTEMTIME FAR *pst) {
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

/* ================================================================== */
int WINAPI DECLSPEC GetLocaleInfoA(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData) {
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
int WINAPI DECLSPEC GetDateFormatA(LCID Locale, DWORD dwFlags, const SYSTEMTIME FAR *lpDate,
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
int WINAPI DECLSPEC GetTimeFormatA(LCID Locale, DWORD dwFlags, const SYSTEMTIME FAR *lpTime,
    LPCSTR lpFormat, LPSTR lpTimeStr, int cchTime) {
    static SYSTEMTIME FAR stCur; const SYSTEMTIME FAR *pSt;
    char fmtBuf[80], outBuf[80], FAR *f, FAR *out, tmp[16], szTimeSep[4], szAm[8], szPm[8];
    int hour24, remain, iTime, bTLZero; WORD h;
    if (Locale == 0) Locale = LOCALE_USER_DEFAULT;
    if (Locale != LOCALE_USER_DEFAULT) return 0;
    if (!lpTimeStr || cchTime < 1) return 0;
    out = outBuf; *out = '\0';

    GetLocaleInfo(Locale, LOCALE_STIME, szTimeSep, sizeof(szTimeSep));
    GetLocaleInfo(Locale, LOCALE_S1159, szAm, sizeof(szAm));
    GetLocaleInfo(Locale, LOCALE_S2359, szPm, sizeof(szPm));
    { int val;
      GetLocaleInfo(Locale, LOCALE_ITIME | LOCALE_RETURN_NUMBER, (LPSTR)&val, sizeof(val)); iTime = val;
      GetLocaleInfo(Locale, LOCALE_ITLZERO | LOCALE_RETURN_NUMBER, (LPSTR)&val, sizeof(val)); bTLZero = val;
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
BOOL WINAPI DECLSPEC SetLocaleInfoA(LCID Locale, LCTYPE LCType, LPCSTR lpLCData) {
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
LCID WINAPI DECLSPEC GetThreadLocale(void) { return LOCALE_USER_DEFAULT; }
BOOL WINAPI DECLSPEC SetThreadLocale(LCID Locale) { return FALSE; }
LCID WINAPI DECLSPEC GetSystemDefaultLCID(void) { return LOCALE_SYSTEM_DEFAULT; }
LCID WINAPI DECLSPEC GetUserDefaultLCID(void) { return LOCALE_USER_DEFAULT; }
