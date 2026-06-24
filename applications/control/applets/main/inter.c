/*
 *  intl.c - International dialog for Windows 3.0 Control Panel
 *  Final version, uses only NLS API.
 */
#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "winnls.h"

#define MAX_LIST_ITEMS  64

static char FAR * countries[MAX_LIST_ITEMS];
static int   countryCodes[MAX_LIST_ITEMS];
static char FAR * languages[MAX_LIST_ITEMS];
static char FAR * keyboards[MAX_LIST_ITEMS];

static char FAR szMetric[] = "Metric", FAR szEnglish[] = "English";

static char g_iniCountry[8]="1", g_iniLanguage[32]="English", g_iniKeyboard[32]="US";
static int  g_iniMeasure=0; static char g_iniListSep[4]=",";
int  g_iniDateFormat=0, g_iniTimeFormat=0, g_iniCurrencyFmt=0;
char g_iniCurrencySym[8]="$", g_iniDecimal[4]=".", g_iniThousand[4]=",";
int  g_iniDigits=2, g_iniLZero=0;
static char g_szTimeSep[4]=":", g_szAm[8]="AM", g_szPm[8]="PM";
static int  g_iniTLZero=0;
static char g_szDateSep[4]="/";
static char g_szLongDateFmt[80]="dddd, MMMM dd, yyyy";
static char g_szShortDateFmt[80]="M/d/yy";
int  g_iniNegCurr        = 0;

static int FarStrnicmp(const char FAR *s1, const char FAR *s2, int n) {
    while (n-- > 0) { char c1=*s1++,c2=*s2++;
        if(c1>='A'&&c1<='Z') c1+='a'-'A'; if(c2>='A'&&c2<='Z') c2+='a'-'A';
        if(c1!=c2) return (unsigned char)c1-(unsigned char)c2;
        if(c1=='\0') return 0;
    }
    return 0;
}
static void FAR GetLocalDateTime(WORD FAR *y, WORD FAR *mo, WORD FAR *d, WORD FAR *h, WORD FAR *mi, WORD FAR *s) {
    static struct dosdate_t date; static struct dostime_t time;
    _dos_getdate(&date); _dos_gettime(&time);
    *y=date.year; *mo=date.month; *d=date.day;
    *h=time.hour; *mi=time.minute; *s=time.second;
}
static int DayOfWeek(int y, int m, int d) { static int t[]={0,3,2,5,0,3,5,1,4,6,2,4}; y-=m<3; return (y+y/4-y/100+y/400+t[m-1]+d)%7; }

static void FAR LoadSetupInfData(void)
{
    char szPath[144];
    OFSTRUCT of;
    HFILE hFile;
    char szReadBuf[512];
    char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;

    char szCountryName[64];
    char szParams[128];
    char szLangName[64];
    char szKbdName[64];
    int iCountry = 0, iLang = 0, iKbd = 0;
    int curSection = 0;  /* 0=none, 1=country, 2=language, 3=keyboard.tables */

    GetSystemDirectory(szPath, sizeof(szPath) - 20);
    lstrcat(szPath, "\\SETUP.INF");

    hFile = OpenFile(szPath, &of, OF_READ);
    if (hFile == HFILE_ERROR) {
        countries[0] = NULL;
        languages[0] = NULL;
        keyboards[0] = NULL;
        return;
    }

    _fmemset(countries, 0, sizeof(countries));
    _fmemset(languages, 0, sizeof(languages));
    _fmemset(keyboards, 0, sizeof(keyboards));
    _fmemset(countryCodes, 0, sizeof(countryCodes));

    while (!bEOF) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) {
                szLine[nLinePos] = '\0';
                goto process_line;
            }
            break;
        }

        for (i = 0; i < nRead; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
process_line:
                if (nLinePos > 0) {
                    LPSTR p = szLine;
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p != '\0' && *p != ';') {
                        if (*p == '[') {
                            LPSTR q = _fstrchr(p + 1, ']');
                            if (q) {
                                *q = '\0';
                                if (lstrcmpi(p + 1, "country") == 0)
                                    curSection = 1;
                                else if (lstrcmpi(p + 1, "language") == 0)
                                    curSection = 2;
                                else if (lstrcmpi(p + 1, "keyboard.tables") == 0)
                                    curSection = 3;
                                else
                                    curSection = 0;
                            }
                        }
                        else if (curSection == 1) {
                            /* "Country Name","param1!param2!..." */
                            LPSTR p1 = _fstrchr(p, '\"');
                            if (p1) {
                                LPSTR p2 = _fstrchr(p1 + 1, '\"');
                                if (p2) {
                                    *p2 = '\0';
                                    lstrcpy(szCountryName, p1 + 1);
                                    p1 = _fstrchr(p2 + 1, '\"');
                                    if (p1) {
                                        p2 = _fstrchr(p1 + 1, '\"');
                                        if (p2) {
                                            *p2 = '\0';
                                            lstrcpy(szParams, p1 + 1);
                                            {
                                                LPSTR pExcl = _fstrchr(szParams, '!');
                                                int code = 0;
                                                if (pExcl) *pExcl = '\0';
                                                code = atoi(szParams);
                                                if (iCountry < MAX_LIST_ITEMS) {
                                                    int cb = lstrlen(szCountryName) + 1;
                                                    HLOCAL hStr = LocalAlloc(LMEM_FIXED, cb);
                                                    if (hStr) {
                                                        LPSTR lpStr = (LPSTR)LocalLock(hStr);
                                                        lstrcpy(lpStr, szCountryName);
                                                        countries[iCountry] = lpStr;
                                                        countryCodes[iCountry] = code;
                                                        iCountry++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (curSection == 2) {
                            /* key = file,"LanguageName" */
                            LPSTR p1 = _fstrchr(p, '\"');
                            if (p1) {
                                LPSTR p2 = _fstrchr(p1 + 1, '\"');
                                if (p2) {
                                    *p2 = '\0';
                                    lstrcpy(szLangName, p1 + 1);
                                    if (iLang < MAX_LIST_ITEMS) {
                                        int cb = lstrlen(szLangName) + 1;
                                        HLOCAL hStr = LocalAlloc(LMEM_FIXED, cb);
                                        if (hStr) {
                                            LPSTR lpStr = (LPSTR)LocalLock(hStr);
                                            lstrcpy(lpStr, szLangName);
                                            languages[iLang] = lpStr;
                                            iLang++;
                                        }
                                    }
                                }
                            }
                        }
                        else if (curSection == 3) {
                            /* key = file,"KeyboardName" (может не быть file) */
                            LPSTR p1 = _fstrchr(p, '\"');
                            if (p1) {
                                LPSTR p2 = _fstrchr(p1 + 1, '\"');
                                if (p2) {
                                    *p2 = '\0';
                                    lstrcpy(szKbdName, p1 + 1);
                                    /* проверка на дубликат: две записи "US" – добавляем только одну */
                                    {
                                        int j;
                                        BOOL bDup = FALSE;
                                        for (j = 0; j < iKbd; j++) {
                                            if (lstrcmpi(keyboards[j], szKbdName) == 0) {
                                                bDup = TRUE;
                                                break;
                                            }
                                        }
                                        if (!bDup && iKbd < MAX_LIST_ITEMS) {
                                            int cb = lstrlen(szKbdName) + 1;
                                            HLOCAL hStr = LocalAlloc(LMEM_FIXED, cb);
                                            if (hStr) {
                                                LPSTR lpStr = (LPSTR)LocalLock(hStr);
                                                lstrcpy(lpStr, szKbdName);
                                                keyboards[iKbd] = lpStr;
                                                iKbd++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') {
                    i++;
                }
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1) {
                    szLine[nLinePos++] = c;
                }
            }
        }
    }

    _lclose(hFile);

    countries[iCountry] = NULL;
    languages[iLang] = NULL;
    keyboards[iKbd] = NULL;
}

static void FAR ReadInternationalSettings(void) {
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICOUNTRY, g_iniCountry, sizeof(g_iniCountry));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLANGUAGE, g_iniLanguage, sizeof(g_iniLanguage));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SKEYBOARDTOINSTALL, g_iniKeyboard, sizeof(g_iniKeyboard));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE | LOCALE_RETURN_NUMBER, (LPSTR)&g_iniMeasure, sizeof(int));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, g_iniListSep, sizeof(g_iniListSep));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDATE | LOCALE_RETURN_NUMBER, (LPSTR)&g_iniDateFormat, sizeof(int));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITIME | LOCALE_RETURN_NUMBER, (LPSTR)&g_iniTimeFormat, sizeof(int));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY | LOCALE_RETURN_NUMBER, (LPSTR)&g_iniCurrencyFmt, sizeof(int));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGCURR | LOCALE_RETURN_NUMBER, (LPSTR)&g_iniNegCurr, sizeof(int));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SCURRENCY, g_iniCurrencySym, sizeof(g_iniCurrencySym));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, g_iniDecimal, sizeof(g_iniDecimal));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, g_iniThousand, sizeof(g_iniThousand));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS | LOCALE_RETURN_NUMBER, (LPSTR)&g_iniDigits, sizeof(int));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO | LOCALE_RETURN_NUMBER, (LPSTR)&g_iniLZero, sizeof(int));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, g_szTimeSep, sizeof(g_szTimeSep));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_S1159, g_szAm, sizeof(g_szAm));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_S2359, g_szPm, sizeof(g_szPm));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDATE, g_szDateSep, sizeof(g_szDateSep));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, g_szLongDateFmt, sizeof(g_szLongDateFmt));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, g_szShortDateFmt, sizeof(g_szShortDateFmt));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITLZERO | LOCALE_RETURN_NUMBER, (LPSTR)&g_iniTLZero, sizeof(int));
}

static int FindCountryIndex(int code) { int i; for (i=0; countries[i]; i++) if (countryCodes[i]==code) return i; return 0; }

static void FAR UpdateDateSamples(HWND hDlg) {
    char buf[80];
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, buf, sizeof(buf));
    SetDlgItemText(hDlg, IDC_INTL_DATE_SHORT, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_INTL_DATE_SHORT), NULL, TRUE);
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, NULL, NULL, buf, sizeof(buf));
    SetDlgItemText(hDlg, IDC_INTL_DATE_LONG, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_INTL_DATE_LONG), NULL, TRUE);
}
static void FAR UpdateTimeSample(HWND hDlg) {
    char buf[40];
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, NULL, buf, sizeof(buf));
    SetDlgItemText(hDlg, IDC_INTL_TIME_SAMPLE, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_INTL_TIME_SAMPLE), NULL, TRUE);
}

static void FAR UpdateCurrencySamples(HWND hDlg) {
    char buf[40];
    wsprintf((LPSTR)buf, "%s1.22", (LPSTR)g_iniCurrencySym);
    SetDlgItemText(hDlg, IDC_INTL_CURR_POS, buf); InvalidateRect(GetDlgItem(hDlg, IDC_INTL_CURR_POS), NULL, TRUE);
    wsprintf(buf, "(%s1.22)", (LPSTR)g_iniCurrencySym);
    SetDlgItemText(hDlg, IDC_INTL_CURR_NEG, buf); InvalidateRect(GetDlgItem(hDlg, IDC_INTL_CURR_NEG), NULL, TRUE);
}

static void FAR UpdateNumberSample(HWND hDlg) {
    char buf[40];
    wsprintf(buf, "1%s234%s4444", g_iniThousand, g_iniDecimal);
    SetDlgItemText(hDlg, IDC_INTL_NUM_SAMPLE, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_INTL_NUM_SAMPLE), NULL, TRUE);
}

/* ========== Date Format dialog ========== */
static const char FAR * MonthTokens[] = { "M","MM","MMM","MMMM" };
static const char FAR * DayTokens[]   = { "d","dd" };
static const char FAR * YearTokens[]  = { "yy","yyyy" };
static const char FAR * WeekTokens[]  = { "ddd","dddd" };
static HWND g_hwndAll[7]; static int g_width[7],g_height[7],g_gap[6],g_rowX,g_rowY;

static void FAR SaveOriginalPositions(HWND hDlg) {
    HWND hwndItems[7]; int i,j; RECT rc; POINT pt;
    hwndItems[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndItems[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1);
    hwndItems[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndItems[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2);
    hwndItems[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); hwndItems[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3);
    hwndItems[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4);
    for (i=0;i<7;i++) for (j=i+1;j<7;j++) { RECT r1,r2; GetWindowRect(hwndItems[i],&r1); GetWindowRect(hwndItems[j],&r2); if (r1.left>r2.left) { HWND t=hwndItems[i]; hwndItems[i]=hwndItems[j]; hwndItems[j]=t; } }
    GetWindowRect(hwndItems[0],&rc); pt.x=rc.left; pt.y=rc.top; ScreenToClient(hDlg,&pt); g_rowX=pt.x; g_rowY=pt.y;
    for (i=0;i<7;i++) { g_hwndAll[i]=hwndItems[i]; GetWindowRect(hwndItems[i],&rc); g_width[i]=rc.right-rc.left; g_height[i]=rc.bottom-rc.top; }
    for (i=0;i<6;i++) { RECT rL,rR; GetWindowRect(hwndItems[i],&rL); GetWindowRect(hwndItems[i+1],&rR); g_gap[i]=rR.left-rL.right; }
}

static void FAR ArrangeLongDateOrder(HWND hDlg, int order) {
    HWND hwndRow[7]; int i;
    if (order==0) { hwndRow[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4); hwndRow[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1); hwndRow[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndRow[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2); hwndRow[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndRow[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3); hwndRow[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); }
    else if (order==1) { hwndRow[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4); hwndRow[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1); hwndRow[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndRow[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2); hwndRow[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndRow[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3); hwndRow[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); }
    else { hwndRow[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); hwndRow[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1); hwndRow[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndRow[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2); hwndRow[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndRow[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3); hwndRow[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4); }
    { int x=g_rowX; for (i=0;i<7;i++) { int k; for (k=0;k<7;k++) if (g_hwndAll[k]==hwndRow[i]) break; SetWindowPos(hwndRow[i],NULL,x,g_rowY,g_width[k],g_height[k],SWP_NOZORDER|SWP_NOACTIVATE); if (i<6) x+=g_width[k]+g_gap[i]; } }
}

static void FAR FillComboWithDateValues(HWND hDlg, WORD year, WORD month, WORD day) {
    char buf[32]; HWND hCombo; int dow=DayOfWeek(year,month,day); LCTYPE lcFull, lcAbbr;
    hCombo=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); SendMessage(hCombo,CB_RESETCONTENT,0,0);
    wsprintf(buf,"%u",month); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    wsprintf(buf,"%02u",month); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    lcFull=LOCALE_SMONTHNAME1+(month-1); lcAbbr=LOCALE_SABBREVMONTHNAME1+(month-1);
    GetLocaleInfo(LOCALE_USER_DEFAULT,lcAbbr,buf,sizeof(buf)); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    GetLocaleInfo(LOCALE_USER_DEFAULT,lcFull,buf,sizeof(buf)); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    hCombo=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); SendMessage(hCombo,CB_RESETCONTENT,0,0);
    wsprintf(buf,"%u",day); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    wsprintf(buf,"%02u",day); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    hCombo=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); SendMessage(hCombo,CB_RESETCONTENT,0,0);
    wsprintf(buf,"%02u",year%100); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    wsprintf(buf,"%04u",year); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    hCombo=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4); SendMessage(hCombo,CB_RESETCONTENT,0,0);
    lcFull=LOCALE_SDAYNAME1+dow; lcAbbr=LOCALE_SABBREVDAYNAME1+dow;
    GetLocaleInfo(LOCALE_USER_DEFAULT,lcAbbr,buf,sizeof(buf)); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
    GetLocaleInfo(LOCALE_USER_DEFAULT,lcFull,buf,sizeof(buf)); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)buf);
}
static int FindMonthIndex(const char* fmt) { int i; for (i=0;i<lstrlen(fmt);i++) { if(FarStrnicmp(fmt+i,"MMMM",4)==0) return 3; if(FarStrnicmp(fmt+i,"MMM",3)==0) return 2; if(FarStrnicmp(fmt+i,"MM",2)==0) return 1; if(*(fmt+i)=='M') return 0; } return 3; }
static int FindDayIndex(const char* fmt) { int i; for (i=0;i<lstrlen(fmt);i++) { if(FarStrnicmp(fmt+i,"dd",2)==0) return 1; if(*(fmt+i)=='d') return 0; } return 1; }
static int FindYearIndex(const char* fmt) { int i; for (i=0;i<lstrlen(fmt);i++) { if(FarStrnicmp(fmt+i,"yyyy",4)==0) return 1; if(FarStrnicmp(fmt+i,"yy",2)==0) return 0; if(*(fmt+i)=='y') return 0; } return 1; }
static int FindWeekIndex(const char* fmt) { int i; for (i=0;i<lstrlen(fmt);i++) { if(FarStrnicmp(fmt+i,"dddd",4)==0) return 1; if(FarStrnicmp(fmt+i,"ddd",3)==0) return 0; } return 1; }
static void FAR GetCurrentFormatString(HWND hDlg, char* outFmt) {
    HWND hwndElements[7]; int i,j;
    hwndElements[0]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1); hwndElements[1]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT1);
    hwndElements[2]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2); hwndElements[3]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT2);
    hwndElements[4]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3); hwndElements[5]=GetDlgItem(hDlg,IDC_DATEFMT_L_EDIT3);
    hwndElements[6]=GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4);
    for (i=0;i<7;i++) for (j=i+1;j<7;j++) { RECT r1,r2; GetWindowRect(hwndElements[i],&r1); GetWindowRect(hwndElements[j],&r2); if (r1.left>r2.left) { HWND t=hwndElements[i]; hwndElements[i]=hwndElements[j]; hwndElements[j]=t; } }
    outFmt[0]='\0';
    for (i=0;i<7;i++) { HWND hwnd=hwndElements[i]; char szClass[16]; GetClassName(hwnd,szClass,sizeof(szClass));
        if (lstrcmpi(szClass,"ComboBox")==0) { LRESULT sel=SendMessage(hwnd,CB_GETCURSEL,0,0); if (sel!=CB_ERR && sel>=0) { if (hwnd==GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1)) lstrcat(outFmt,MonthTokens[sel]); else if (hwnd==GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2)) lstrcat(outFmt,DayTokens[sel]); else if (hwnd==GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3)) lstrcat(outFmt,YearTokens[sel]); else if (hwnd==GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4)) lstrcat(outFmt,WeekTokens[sel]); } }
        else if (lstrcmpi(szClass,"Edit")==0) { char sep[20]; GetWindowText(hwnd,sep,sizeof(sep)); lstrcat(outFmt,sep); } }
}
static void FAR ExtractSeparators(HWND hDlg, const char* fmt) {
    const char* p=fmt; char sep[20]; int sepIdx=0, i;
    for (i=0;i<3;i++) SetDlgItemText(hDlg,IDC_DATEFMT_L_EDIT1+i,"");
    while (*p && sepIdx<3) {
        int isToken=0;
        if (FarStrnicmp(p,"MMMM",4)==0)      { isToken=1; p+=4; }
        else if (FarStrnicmp(p,"MMM",3)==0)  { isToken=1; p+=3; }
        else if (FarStrnicmp(p,"MM",2)==0)   { isToken=1; p+=2; }
        else if (*p=='M')                     { isToken=1; p++; }
        else if (FarStrnicmp(p,"dddd",4)==0) { isToken=1; p+=4; }
        else if (FarStrnicmp(p,"ddd",3)==0)  { isToken=1; p+=3; }
        else if (FarStrnicmp(p,"dd",2)==0)   { isToken=1; p+=2; }
        else if (*p=='d')                     { isToken=1; p++; }
        else if (FarStrnicmp(p,"yyyy",4)==0) { isToken=1; p+=4; }
        else if (FarStrnicmp(p,"yy",2)==0)   { isToken=1; p+=2; }
        else if (*p=='y')                     { isToken=1; p++; }
        else {
            int t=0;
            while (*p && t<19 && !isToken) {
                if (FarStrnicmp(p,"MMMM",4)==0||FarStrnicmp(p,"MMM",3)==0||FarStrnicmp(p,"MM",2)==0||*p=='M'||
                    FarStrnicmp(p,"dddd",4)==0||FarStrnicmp(p,"ddd",3)==0||FarStrnicmp(p,"dd",2)==0||*p=='d'||
                    FarStrnicmp(p,"yyyy",4)==0||FarStrnicmp(p,"yy",2)==0||*p=='y') break;
                sep[t++]=*p++;
            }
            sep[t]='\0'; SetDlgItemText(hDlg,IDC_DATEFMT_L_EDIT1+sepIdx,sep); sepIdx++;
        }
    }
}
static void FAR UpdateLongDateSample(HWND hDlg, const char* fmt) {
    char buf[80];
    GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, fmt, buf, sizeof(buf));
    SetDlgItemText(hDlg, IDC_DATEFMT_L_SAMPLE, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_DATEFMT_L_SAMPLE), NULL, TRUE);
}

BOOL WINAPI DateFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int s_dateFmt=0, s_longFmtOrder=0; static char s_szLongFmt[80], s_szShortFmt[80]; static BOOL s_bInit=FALSE;
    switch (msg) {
    case WM_INITDIALOG: { WORD y,m,d,h,mi,s; GetLocalDateTime(&y,&m,&d,&h,&mi,&s); s_bInit=TRUE;
        s_dateFmt=g_iniDateFormat; lstrcpy(s_szLongFmt,g_szLongDateFmt); lstrcpy(s_szShortFmt,g_szShortDateFmt);
        { int val; GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ILDATE|LOCALE_RETURN_NUMBER,(LPSTR)&val,sizeof(val)); s_longFmtOrder=val; }
        CheckRadioButton(hDlg,IDC_DATEFMT_S_MDY,IDC_DATEFMT_S_YMD,IDC_DATEFMT_S_MDY+s_dateFmt);
        SetDlgItemText(hDlg,IDC_DATEFMT_SEPARATOR,g_szDateSep);
        { int val; GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDAYLZERO|LOCALE_RETURN_NUMBER,(LPSTR)&val,sizeof(val)); CheckDlgButton(hDlg,IDC_DATEFMT_S_DAYLZ,val);
          GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IMONLZERO|LOCALE_RETURN_NUMBER,(LPSTR)&val,sizeof(val)); CheckDlgButton(hDlg,IDC_DATEFMT_S_MONTHLZ,val);
          GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ICENTURY|LOCALE_RETURN_NUMBER,(LPSTR)&val,sizeof(val)); CheckDlgButton(hDlg,IDC_DATEFMT_S_CENTURY,val); }
        CheckRadioButton(hDlg,IDC_DATEFMT_L_MDY,IDC_DATEFMT_L_YMD,IDC_DATEFMT_L_MDY+s_longFmtOrder);
        FillComboWithDateValues(hDlg,y,m,d);
        SendMessage(GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO1),CB_SETCURSEL,(WPARAM)FindMonthIndex(s_szLongFmt),0);
        SendMessage(GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO2),CB_SETCURSEL,(WPARAM)FindDayIndex(s_szLongFmt),0);
        SendMessage(GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO3),CB_SETCURSEL,(WPARAM)FindYearIndex(s_szLongFmt),0);
        SendMessage(GetDlgItem(hDlg,IDC_DATEFMT_L_COMBO4),CB_SETCURSEL,(WPARAM)FindWeekIndex(s_szLongFmt),0);
        ExtractSeparators(hDlg,s_szLongFmt); SaveOriginalPositions(hDlg);
        ArrangeLongDateOrder(hDlg,s_longFmtOrder); UpdateLongDateSample(hDlg,s_szLongFmt);
        s_bInit=FALSE; return TRUE; }
    case WM_COMMAND: { WORD id=LOWORD(wParam),code=HIWORD(lParam); if (s_bInit) return FALSE;
        if ((id>=IDC_DATEFMT_L_COMBO1&&id<=IDC_DATEFMT_L_COMBO4)||(id>=IDC_DATEFMT_L_EDIT1&&id<=IDC_DATEFMT_L_EDIT3))
            if (code==CBN_SELCHANGE||code==EN_CHANGE) { GetCurrentFormatString(hDlg,s_szLongFmt); UpdateLongDateSample(hDlg,s_szLongFmt); }
        if (code==BN_CLICKED) { switch (id) {
            case IDC_DATEFMT_S_MDY: case IDC_DATEFMT_S_DMY: case IDC_DATEFMT_S_YMD: s_dateFmt=id-IDC_DATEFMT_S_MDY; CheckRadioButton(hDlg,IDC_DATEFMT_S_MDY,IDC_DATEFMT_S_YMD,id); return TRUE;
            case IDC_DATEFMT_L_MDY: case IDC_DATEFMT_L_DMY: case IDC_DATEFMT_L_YMD: s_longFmtOrder=id-IDC_DATEFMT_L_MDY; CheckRadioButton(hDlg,IDC_DATEFMT_L_MDY,IDC_DATEFMT_L_YMD,id); ArrangeLongDateOrder(hDlg,s_longFmtOrder); GetCurrentFormatString(hDlg,s_szLongFmt); UpdateLongDateSample(hDlg,s_szLongFmt); return TRUE;
        } }
        if (id==IDOK) {
            g_iniDateFormat=s_dateFmt; GetDlgItemText(hDlg,IDC_DATEFMT_SEPARATOR,g_szDateSep,sizeof(g_szDateSep));
            { int dayLZ=IsDlgButtonChecked(hDlg,IDC_DATEFMT_S_DAYLZ), monLZ=IsDlgButtonChecked(hDlg,IDC_DATEFMT_S_MONTHLZ), century=IsDlgButtonChecked(hDlg,IDC_DATEFMT_S_CENTURY);
              char *s=s_szShortFmt; if (s_dateFmt==0) { lstrcpy(s,monLZ?"MM":"M"); lstrcat(s,g_szDateSep); lstrcat(s,dayLZ?"dd":"d"); lstrcat(s,g_szDateSep); lstrcat(s,century?"yyyy":"yy"); }
              else if (s_dateFmt==1) { lstrcpy(s,dayLZ?"dd":"d"); lstrcat(s,g_szDateSep); lstrcat(s,monLZ?"MM":"M"); lstrcat(s,g_szDateSep); lstrcat(s,century?"yyyy":"yy"); }
              else { lstrcpy(s,century?"yyyy":"yy"); lstrcat(s,g_szDateSep); lstrcat(s,monLZ?"MM":"M"); lstrcat(s,g_szDateSep); lstrcat(s,dayLZ?"dd":"d"); } }
            GetCurrentFormatString(hDlg,s_szLongFmt);
            lstrcpy(g_szLongDateFmt,s_szLongFmt); lstrcpy(g_szShortDateFmt,s_szShortFmt);
            EndDialog(hDlg,IDOK); return TRUE; }
        if (id==IDCANCEL) { EndDialog(hDlg,IDCANCEL); return TRUE; } break; } }
    return FALSE; }

BOOL WINAPI TimeFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        CheckRadioButton(hDlg,IDC_TIMEFMT_12H,IDC_TIMEFMT_24H,IDC_TIMEFMT_12H+g_iniTimeFormat);
        SetDlgItemText(hDlg,IDC_TIMEFMT_AM,g_szAm); SetDlgItemText(hDlg,IDC_TIMEFMT_PM,g_szPm);
        SetDlgItemText(hDlg,IDC_TIMEFMT_SEP,g_szTimeSep);
        CheckRadioButton(hDlg,IDC_TIMEFMT_LZ_OFF,IDC_TIMEFMT_LZ_ON,IDC_TIMEFMT_LZ_OFF+(g_iniTLZero?1:0));
        return TRUE;
    case WM_COMMAND:
        if (HIWORD(lParam)==BN_CLICKED) {
            switch (wParam) {
                case IDC_TIMEFMT_12H: case IDC_TIMEFMT_24H: CheckRadioButton(hDlg,IDC_TIMEFMT_12H,IDC_TIMEFMT_24H,wParam); break;
                case IDC_TIMEFMT_LZ_OFF: case IDC_TIMEFMT_LZ_ON: CheckRadioButton(hDlg,IDC_TIMEFMT_LZ_OFF,IDC_TIMEFMT_LZ_ON,wParam); break;
            }
        }
        if (wParam==IDOK) {
            g_iniTimeFormat=IsDlgButtonChecked(hDlg,IDC_TIMEFMT_12H)?0:1;
            GetDlgItemText(hDlg,IDC_TIMEFMT_AM,g_szAm,sizeof(g_szAm));
            GetDlgItemText(hDlg,IDC_TIMEFMT_PM,g_szPm,sizeof(g_szPm));
            GetDlgItemText(hDlg,IDC_TIMEFMT_SEP,g_szTimeSep,sizeof(g_szTimeSep));
            g_iniTLZero=IsDlgButtonChecked(hDlg,IDC_TIMEFMT_LZ_ON); EndDialog(hDlg,IDOK); return TRUE;
        }
        if (wParam==IDCANCEL) { EndDialog(hDlg,IDCANCEL); return TRUE; }
        break;
    }
    return FALSE;
}

BOOL WINAPI NumberFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg,IDC_NUMFMT_DECIMAL,g_iniDecimal); SetDlgItemText(hDlg,IDC_NUMFMT_THOUSAND,g_iniThousand);
        SetDlgItemInt(hDlg,IDC_NUMFMT_DIGITS,g_iniDigits,FALSE); CheckDlgButton(hDlg,IDC_NUMFMT_LEADZERO,g_iniLZero); return TRUE;
    case WM_COMMAND:
        if (wParam==IDOK) {
            GetDlgItemText(hDlg,IDC_NUMFMT_DECIMAL,g_iniDecimal,sizeof(g_iniDecimal));
            GetDlgItemText(hDlg,IDC_NUMFMT_THOUSAND,g_iniThousand,sizeof(g_iniThousand));
            g_iniDigits=GetDlgItemInt(hDlg,IDC_NUMFMT_DIGITS,NULL,FALSE); g_iniLZero=IsDlgButtonChecked(hDlg,IDC_NUMFMT_LEADZERO);
            EndDialog(hDlg,IDOK); return TRUE;
        }
        if (wParam==IDCANCEL) EndDialog(hDlg,IDCANCEL); return TRUE;
    }
    return FALSE;
}

BOOL WINAPI InternationalDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: { HWND hCombo; int i,idx; LRESULT lr; 
	LoadSetupInfData();
	ReadInternationalSettings();
        hCombo=GetDlgItem(hDlg,IDC_INTL_COUNTRY); for (i=0;countries[i];i++) SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)countries[i]); idx=FindCountryIndex(atoi(g_iniCountry)); SendMessage(hCombo,CB_SETCURSEL,idx,0);
        hCombo=GetDlgItem(hDlg,IDC_INTL_LANGUAGE); for (i=0;languages[i];i++) SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)languages[i]); lr=SendMessage(hCombo,CB_SELECTSTRING,-1,(LPARAM)g_iniLanguage); if (lr==CB_ERR) SendMessage(hCombo,CB_SETCURSEL,0,0);
        hCombo=GetDlgItem(hDlg,IDC_INTL_KEYBOARD); for (i=0;keyboards[i];i++) SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)keyboards[i]); lr=SendMessage(hCombo,CB_SELECTSTRING,-1,(LPARAM)g_iniKeyboard); if (lr==CB_ERR) SendMessage(hCombo,CB_SETCURSEL,0,0);
        hCombo=GetDlgItem(hDlg,IDC_INTL_MEASURE); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)szMetric); SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)szEnglish); SendMessage(hCombo,CB_SETCURSEL,g_iniMeasure?1:0,0);
        SetDlgItemText(hDlg,IDC_INTL_LISTSEP,g_iniListSep);
        UpdateDateSamples(hDlg);
	UpdateTimeSample(hDlg);
	UpdateCurrencySamples(hDlg);
#if 0
	UpdateNumberSample(hDlg);
#endif
        return TRUE; }
    case WM_COMMAND: { switch (wParam) {
        case IDOK: { HWND hCombo; LRESULT idx; char buf[10];
            hCombo=GetDlgItem(hDlg,IDC_INTL_COUNTRY); idx=SendMessage(hCombo,CB_GETCURSEL,0,0); if (idx>=0) { wsprintf(buf,"%d",countryCodes[idx]); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ICOUNTRY,buf); }
            hCombo=GetDlgItem(hDlg,IDC_INTL_LANGUAGE); idx=SendMessage(hCombo,CB_GETCURSEL,0,0); if (idx>=0&&idx<7) SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SLANGUAGE,languages[idx]);
            hCombo=GetDlgItem(hDlg,IDC_INTL_KEYBOARD); idx=SendMessage(hCombo,CB_GETCURSEL,0,0); if (idx>=0&&idx<8) SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SKEYBOARDTOINSTALL,keyboards[idx]);
            hCombo=GetDlgItem(hDlg,IDC_INTL_MEASURE); idx=SendMessage(hCombo,CB_GETCURSEL,0,0); wsprintf(buf,"%d",idx==0?0:1); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IMEASURE,buf);
            GetDlgItemText(hDlg,IDC_INTL_LISTSEP,buf,sizeof(buf)); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SLIST,buf);
            wsprintf(buf,"%d",g_iniDateFormat); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDATE,buf);
            wsprintf(buf,"%d",g_iniTimeFormat); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ITIME,buf);
            wsprintf(buf,"%d",g_iniCurrencyFmt); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ICURRENCY,buf);
            SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SCURRENCY,g_iniCurrencySym);
            wsprintf(buf, "%d", g_iniNegCurr); SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGCURR, buf);
            SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,g_iniDecimal);
            SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STHOUSAND,g_iniThousand);
            wsprintf(buf,"%d",g_iniDigits); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ICURRDIGITS,buf);
            wsprintf(buf,"%d",g_iniLZero); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ILZERO,buf);
            SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STIME,g_szTimeSep);
            SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_S1159,g_szAm); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_S2359,g_szPm);
            SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDATE,g_szDateSep);
            SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SLONGDATE,g_szLongDateFmt);
            SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SSHORTDATE,g_szShortDateFmt);
            wsprintf(buf,"%d",g_iniTLZero); SetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_ITLZERO,buf);
            EndDialog(hDlg,IDOK); return TRUE; }
        case IDCANCEL: EndDialog(hDlg,IDCANCEL); return TRUE;
        case IDC_INTL_DATE_CHANGE: DialogBox(g_hInst,MAKEINTRESOURCE(DLG_DATEFMT),hDlg,DateFmtDlgProc); UpdateDateSamples(hDlg); UpdateWindow(hDlg); return TRUE;
        case IDC_INTL_TIME_CHANGE: DialogBox(g_hInst,MAKEINTRESOURCE(DLG_TIMEFMT),hDlg,TimeFmtDlgProc); UpdateTimeSample(hDlg); UpdateWindow(hDlg); return TRUE;
        case IDC_INTL_CURR_CHANGE: DialogBox(g_hInst,MAKEINTRESOURCE(DLG_CURRFMT),hDlg,CurrencyFmtDlgProc); UpdateCurrencySamples(hDlg); UpdateWindow(hDlg); return TRUE;
        case IDC_INTL_NUM_CHANGE: DialogBox(g_hInst,MAKEINTRESOURCE(DLG_NUMFMT),hDlg,NumberFmtDlgProc); UpdateNumberSample(hDlg); UpdateWindow(hDlg); return TRUE;
    } break; } }
    return FALSE; }
