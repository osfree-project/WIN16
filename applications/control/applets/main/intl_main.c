/*
 *  intl_main.c – основной диалог International, образцы, колбэки стран/языков
 *  ВРЕМЕННАЯ ТРАССИРОВКА ДЛЯ ПОИСКА ТРАПА
 */
#include "intl.h"

/* ---------- строки для системы мер ---------- */
static char FAR szMetric[] = "Metric", FAR szEnglish[] = "English";

/* ---------- определения глобальных переменных ---------- */
char g_iniCountry[8]="1";
char g_iniLanguage[32]="English";
char g_iniKeyboard[32]="US";
int  g_iniMeasure=0;
char g_iniListSep[4]=",";
int  g_iniDateFormat=0;
int  g_iniTimeFormat=0;
int  g_iniCurrencyFmt=0;
char g_iniCurrencySym[8]="$";
char g_iniDecimal[4]=".";
char g_iniThousand[4]=",";
int  g_iniDigits=2;
int  g_iniLZero=0;
char g_szTimeSep[4]=":";
char g_szAm[8]="AM";
char g_szPm[8]="PM";
int  g_iniTLZero=0;
char g_szDateSep[4]="/";
char g_szLongDateFmt[80]="dddd, MMMM dd, yyyy";
char g_szShortDateFmt[80]="M/d/yy";
int  g_iniNegCurr = 0;

/* ---------- вспомогательные функции ---------- */
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

/* Преобразование шестнадцатеричной строки (FAR) в LCID */
static LCID HexStrToLCID(const char FAR *str)
{
    LCID result = 0;
    while (*str) {
        char c = *str++;
        if (c >= '0' && c <= '9') result = (result << 4) | (c - '0');
        else if (c >= 'A' && c <= 'F') result = (result << 4) | (c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') result = (result << 4) | (c - 'a' + 10);
    }
    return result;
}

/* Локальная статическая версия StringCopyN */
static void StringCopyN(LPSTR dest, LPCSTR src, int n)
{
    int i;
    if (n <= 0) return;
    for (i = 0; i < n - 1 && src[i] != '\0'; i++)
        dest[i] = src[i];
    dest[i] = '\0';
}

static void FAR ReadInternationalSettings(void) {
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICOUNTRY, g_iniCountry, sizeof(g_iniCountry));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLANGUAGE, g_iniLanguage, sizeof(g_iniLanguage));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SKEYBOARDSTOINSTALL, (LPSTR)g_iniKeyboard, sizeof(g_iniKeyboard));
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

/* ---------- глобальные переменные для языковых колбэков ---------- */
static char szLangCodes[64][8];
static int  nLangCount;
static HWND g_hwndLangCombo;

static BOOL CALLBACK CollectLangCodesProc(LPSTR lpszLangCode, LONG lParam)
{
//    MessageBox(NULL, "CollectLangCodesProc: called", "Trace", MB_OK);
    if (nLangCount < 64) {
        StringCopyN(szLangCodes[nLangCount], lpszLangCode, 8);
        nLangCount++;
    }
    return TRUE;
}

static BOOL CALLBACK AddLangNamesProc(LPSTR lpszLangName, LONG lParam)
{
    int *pIdx = (int *)lParam;
    LRESULT idx;
//    MessageBox(NULL, "AddLangNamesProc: called", "Trace", MB_OK);
    idx = SendMessage(g_hwndLangCombo, CB_ADDSTRING, 0, (LPARAM)lpszLangName);
    if (idx != CB_ERR && idx != CB_ERRSPACE) {
        HLOCAL hCode = LocalAlloc(LMEM_FIXED, lstrlen(szLangCodes[*pIdx]) + 1);
        if (hCode) {
            lstrcpy((LPSTR)LocalLock(hCode), szLangCodes[*pIdx]);
            SendMessage(g_hwndLangCombo, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)hCode);
        }
        (*pIdx)++;
    }
    return TRUE;
}

/* ---------- контекст и колбэк для стран ---------- */
typedef struct {
    HWND hCombo;
} COUNTRY_ENUM_CTX;

static COUNTRY_ENUM_CTX g_CountryCtx;

static BOOL CALLBACK CountryEnumProc(LPSTR lpszLCID) {
    LCID lcid;
    char szCountry[64];
    LRESULT idx;
//    char szMsg[128];

    lcid = HexStrToLCID(lpszLCID);
//    wsprintf(szMsg, "LCID=%08lX", lcid);
//    MessageBox(NULL, szMsg, "CountryEnumProc: LCID", MB_OK);

    if (GetLocaleInfo(lcid, LOCALE_SCOUNTRY, szCountry, sizeof(szCountry))) {
//        wsprintf(szMsg, "Country='%s'", szCountry);
//        MessageBox(NULL, szMsg, "CountryEnumProc: Got name", MB_OK);
        idx = SendMessage(g_CountryCtx.hCombo, CB_ADDSTRING, 0, (LPARAM)szCountry);
        if (idx != CB_ERR && idx != CB_ERRSPACE)
            SendMessage(g_CountryCtx.hCombo, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)lcid);
    } else {
//        MessageBox(NULL, "GetLocaleInfo FAILED", "CountryEnumProc", MB_OK);
    }
    return TRUE;
}

/* ---------- функции инициализации ---------- */
static void InitCountries(HWND hDlg)
{
    HWND hCombo = GetDlgItem(hDlg, IDC_INTL_COUNTRY);
    char szCurCountry[64];
    LRESULT lr;

//    MessageBox(hDlg, "TRACE: InitCountries start", "Debug", MB_OK);
    g_CountryCtx.hCombo = hCombo;
    EnumSystemLocalesA(CountryEnumProc, 0);
//    MessageBox(hDlg, "TRACE: EnumSystemLocalesA done", "Debug", MB_OK);

    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SCOUNTRY, szCurCountry, sizeof(szCurCountry));
    lr = SendMessage(hCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)szCurCountry);
    if (lr == CB_ERR) SendMessage(hCombo, CB_SETCURSEL, 0, 0);
//    MessageBox(hDlg, "TRACE: InitCountries end", "Debug", MB_OK);
}

static void InitLanguages(HWND hDlg)
{
    HWND hCombo = GetDlgItem(hDlg, IDC_INTL_LANGUAGE);
    int langIdx;
    char szCurLangCode[8];
    int count, i;
    BOOL bFound;

//    MessageBox(hDlg, "TRACE: InitLanguages start", "Debug", MB_OK);
    g_hwndLangCombo = hCombo;
    nLangCount = 0;
    EnumUILanguagesA(CollectLangCodesProc, MUI_LANGUAGE_ID, 0);
    langIdx = 0;
    EnumUILanguagesA(AddLangNamesProc, MUI_LANGUAGE_NAME, (LONG)&langIdx);

    /* выбираем текущий язык */
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, szCurLangCode, sizeof(szCurLangCode));
    count = (int)SendMessage(hCombo, CB_GETCOUNT, 0, 0);
    bFound = FALSE;
    for (i = 0; i < count; i++) {
        HLOCAL hCode = (HLOCAL)SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)i, 0);
        if (hCode) {
            LPSTR lpCode = (LPSTR)LocalLock(hCode);
            if (lstrcmpi(lpCode, szCurLangCode) == 0) {
                SendMessage(hCombo, CB_SETCURSEL, (WPARAM)i, 0);
                bFound = TRUE;
                break;
            }
        }
    }
    if (!bFound) SendMessage(hCombo, CB_SETCURSEL, 0, 0);
//    MessageBox(hDlg, "TRACE: InitLanguages end", "Debug", MB_OK);
}

static void InitKeyboards(HWND hDlg)
{
    HWND hCombo = GetDlgItem(hDlg, IDC_INTL_KEYBOARD);
    int nKbd, k, nBufSize, nActual, cb;
    LPSTR lpList;
    HLOCAL hMem, hName;
    LRESULT lr;

//    MessageBox(hDlg, "TRACE: InitKeyboards start", "Debug", MB_OK);
    nKbd = GetKeyboardLayoutList(0, NULL);
    if (nKbd > 0) {
//        MessageBox(hDlg, "TRACE: Keyboard count > 0", "Debug", MB_OK);
        nBufSize = nKbd * 64 + 1;
        hMem = LocalAlloc(LMEM_FIXED, nBufSize);
        if (hMem) {
//            MessageBox(hDlg, "TRACE: LocalAlloc OK", "Debug", MB_OK);
            lpList = (LPSTR)LocalLock(hMem);
            nActual = GetKeyboardLayoutList(nBufSize, lpList);
            for (k = 0; k < nActual; k++) {
                LPSTR pName = lpList;
                while (*lpList) lpList++;
                cb = lstrlen(pName) + 1;
                hName = LocalAlloc(LMEM_FIXED, cb);
                if (hName) {
                    lstrcpy((LPSTR)LocalLock(hName), pName);
                    lr = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)pName);
                    if (lr != CB_ERR && lr != CB_ERRSPACE)
                        SendMessage(hCombo, CB_SETITEMDATA, (WPARAM)lr, (LPARAM)hName);
                }
                lpList++;
            }
            LocalUnlock(hMem);
            LocalFree(hMem);
//            MessageBox(hDlg, "TRACE: loop finished", "Debug", MB_OK);
        } else {
//            MessageBox(hDlg, "TRACE: LocalAlloc FAILED", "Debug", MB_OK);
        }
    }
    lr = SendMessage(hCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)g_iniKeyboard);
    if (lr == CB_ERR)
        SendMessage(hCombo, CB_SETCURSEL, 0, 0);
//    MessageBox(hDlg, "TRACE: InitKeyboards end", "Debug", MB_OK);
}

/* ---------- образцы (исправлены: буферы статические, far-указатели) ---------- */
void FAR UpdateDateSamples(HWND hDlg) {
    static char buf[80];
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, buf, sizeof(buf));
    SetDlgItemText(hDlg, IDC_INTL_DATE_SHORT, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_INTL_DATE_SHORT), NULL, TRUE);
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, NULL, NULL, buf, sizeof(buf));
    SetDlgItemText(hDlg, IDC_INTL_DATE_LONG, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_INTL_DATE_LONG), NULL, TRUE);
}
void FAR UpdateTimeSample(HWND hDlg) {
    static char buf[40];
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, NULL, buf, sizeof(buf));
    SetDlgItemText(hDlg, IDC_INTL_TIME_SAMPLE, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_INTL_TIME_SAMPLE), NULL, TRUE);
}

void FAR UpdateCurrencySamples(HWND hDlg) {
    static char buf[40];
    wsprintf(buf, "%s1.22", (LPSTR)g_iniCurrencySym);
    SetDlgItemText(hDlg, IDC_INTL_CURR_POS, buf); InvalidateRect(GetDlgItem(hDlg, IDC_INTL_CURR_POS), NULL, TRUE);
    wsprintf(buf, "(%s1.22)", (LPSTR)g_iniCurrencySym);
    SetDlgItemText(hDlg, IDC_INTL_CURR_NEG, buf); InvalidateRect(GetDlgItem(hDlg, IDC_INTL_CURR_NEG), NULL, TRUE);
}

void FAR UpdateNumberSample(HWND hDlg) {
    static char buf[40];
    wsprintf(buf, "1%s234%s4444", (LPSTR)g_iniThousand, (LPSTR)g_iniDecimal);
    SetDlgItemText(hDlg, IDC_INTL_NUM_SAMPLE, buf);
    InvalidateRect(GetDlgItem(hDlg, IDC_INTL_NUM_SAMPLE), NULL, TRUE);
}

/* ========== Основная диалоговая процедура International ========== */
BOOL WINAPI InternationalDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
//        MessageBox(hDlg, "TRACE: WM_INITDIALOG begin", "Debug", MB_OK);

        ReadInternationalSettings();

        InitCountries(hDlg);
        InitLanguages(hDlg);
        InitKeyboards(hDlg);

        /* Система мер */
        {
            HWND hCombo = GetDlgItem(hDlg, IDC_INTL_MEASURE);
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szMetric);
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szEnglish);
            SendMessage(hCombo, CB_SETCURSEL, (WPARAM)(g_iniMeasure ? 1 : 0), 0);
        }

        SetDlgItemText(hDlg, IDC_INTL_LISTSEP, g_iniListSep);

        UpdateDateSamples(hDlg);
        UpdateTimeSample(hDlg);
        UpdateCurrencySamples(hDlg);
        UpdateNumberSample(hDlg);

//        MessageBox(hDlg, "TRACE: WM_INITDIALOG end", "Debug", MB_OK);
        return TRUE;
    }

    case WM_COMMAND: {
        switch (wParam) {
            case IDOK: {
                HWND hCombo; LRESULT idx; char buf[10];
                /* Сохранение страны */
                hCombo = GetDlgItem(hDlg, IDC_INTL_COUNTRY);
                idx = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                if (idx != CB_ERR) {
                    LCID lcid = (LCID)SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)idx, 0);
                    int iCountry;
                    if (GetLocaleInfo(lcid, LOCALE_ICOUNTRY | LOCALE_RETURN_NUMBER, (LPSTR)&iCountry, sizeof(iCountry))) {
                        wsprintf(buf, "%d", iCountry);
                        SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICOUNTRY, buf);
                    }
                }
                /* Сохранение языка */
                hCombo = GetDlgItem(hDlg, IDC_INTL_LANGUAGE);
                idx = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                if (idx != CB_ERR) {
                    HLOCAL hCode = (HLOCAL)SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)idx, 0);
                    if (hCode) {
                        LPSTR lpCode = (LPSTR)LocalLock(hCode);
                        SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLANGUAGE, lpCode);
                    }
                }
                /* Сохранение клавиатуры */
                hCombo = GetDlgItem(hDlg, IDC_INTL_KEYBOARD);
                idx = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                if (idx != CB_ERR) {
                    HLOCAL hName = (HLOCAL)SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)idx, 0);
                    if (hName) {
                        LPSTR lpName = (LPSTR)LocalLock(hName);
                        SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SKEYBOARDSTOINSTALL, lpName);
                    }
                }
                /* Система мер */
                hCombo = GetDlgItem(hDlg, IDC_INTL_MEASURE);
                idx = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                wsprintf(buf, "%d", (idx == 0) ? 0 : 1);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, buf);

                GetDlgItemText(hDlg, IDC_INTL_LISTSEP, buf, sizeof(buf));
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, buf);

                wsprintf(buf, "%d", g_iniDateFormat);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDATE, buf);
                wsprintf(buf, "%d", g_iniTimeFormat);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITIME, buf);
                wsprintf(buf, "%d", g_iniCurrencyFmt);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, buf);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SCURRENCY, g_iniCurrencySym);
                wsprintf(buf, "%d", g_iniNegCurr);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGCURR, buf);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, g_iniDecimal);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, g_iniThousand);
                wsprintf(buf, "%d", g_iniDigits);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, buf);
                wsprintf(buf, "%d", g_iniLZero);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, buf);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, g_szTimeSep);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_S1159, g_szAm);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_S2359, g_szPm);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDATE, g_szDateSep);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, g_szLongDateFmt);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, g_szShortDateFmt);
                wsprintf(buf, "%d", g_iniTLZero);
                SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITLZERO, buf);

                EndDialog(hDlg, IDOK);
                return TRUE;
            }
            case IDCANCEL:
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            case IDC_INTL_DATE_CHANGE:
                DialogBox(g_hInst, MAKEINTRESOURCE(DLG_DATEFMT), hDlg, DateFmtDlgProc);
                UpdateDateSamples(hDlg);
                UpdateWindow(hDlg);
                return TRUE;
            case IDC_INTL_TIME_CHANGE:
                DialogBox(g_hInst, MAKEINTRESOURCE(DLG_TIMEFMT), hDlg, TimeFmtDlgProc);
                UpdateTimeSample(hDlg);
                UpdateWindow(hDlg);
                return TRUE;
            case IDC_INTL_CURR_CHANGE:
                DialogBox(g_hInst, MAKEINTRESOURCE(DLG_CURRFMT), hDlg, CurrencyFmtDlgProc);
                UpdateCurrencySamples(hDlg);
                UpdateWindow(hDlg);
                return TRUE;
            case IDC_INTL_NUM_CHANGE:
                DialogBox(g_hInst, MAKEINTRESOURCE(DLG_NUMFMT), hDlg, NumberFmtDlgProc);
                UpdateNumberSample(hDlg);
                UpdateWindow(hDlg);
                return TRUE;
        }
        break;
    }
    }
    return FALSE;
}
