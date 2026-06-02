/*
 * main.c -- MAIN.CPL для Windows 3.0
 * Содержит апплеты: Color, Fonts, Ports, Mouse, Desktop, Keyboard,
 * Printers, International, Date/Time, Network.
 */

#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

HINSTANCE g_hInst = NULL;

/* ----- Прототипы диалоговых процедур ----- */
BOOL CALLBACK ColorDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK FontsDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PortsDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DesktopDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK KeyboardDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PrintersDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK InternationalDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DateTimeDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK NetworkDlgProc(HWND, UINT, WPARAM, LPARAM);

/* ----- Точка входа панели управления ----- */
LONG WINAPI CPlApplet(HWND hwndCPL, UINT msg, LONG lParam1, LONG lParam2)
{
    switch (msg) {
    case CPL_INIT:
        return TRUE;
    case CPL_GETCOUNT:
        return NUM_APPLETS;
    case CPL_INQUIRE: {
        LPCPLINFO lpInfo = (LPCPLINFO)lParam2;
        int idx = (int)lParam1;
        lpInfo->lData = (LONG)idx;
        lpInfo->idIcon = 0;      /* используем IDI_APPLICATION */
        switch (idx) {
        case COLOR_IDX:
            lpInfo->idName = IDS_COLOR_NAME;
            lpInfo->idInfo = IDS_COLOR_INFO;
            break;
        case FONTS_IDX:
            lpInfo->idName = IDS_FONTS_NAME;
            lpInfo->idInfo = IDS_FONTS_INFO;
            break;
        case PORTS_IDX:
            lpInfo->idName = IDS_PORTS_NAME;
            lpInfo->idInfo = IDS_PORTS_INFO;
            break;
        case MOUSE_IDX:
            lpInfo->idName = IDS_MOUSE_NAME;
            lpInfo->idInfo = IDS_MOUSE_INFO;
            break;
        case DESKTOP_IDX:
            lpInfo->idName = IDS_DESKTOP_NAME;
            lpInfo->idInfo = IDS_DESKTOP_INFO;
            break;
        case KEYBOARD_IDX:
            lpInfo->idName = IDS_KEYBOARD_NAME;
            lpInfo->idInfo = IDS_KEYBOARD_INFO;
            break;
        case PRINTERS_IDX:
            lpInfo->idName = IDS_PRINTERS_NAME;
            lpInfo->idInfo = IDS_PRINTERS_INFO;
            break;
        case INTERNATIONAL_IDX:
            lpInfo->idName = IDS_INTERNATIONAL_NAME;
            lpInfo->idInfo = IDS_INTERNATIONAL_INFO;
            break;
        case DATETIME_IDX:
            lpInfo->idName = IDS_DATETIME_NAME;
            lpInfo->idInfo = IDS_DATETIME_INFO;
            break;
        case NETWORK_IDX:
            lpInfo->idName = IDS_NETWORK_NAME;
            lpInfo->idInfo = IDS_NETWORK_INFO;
            break;
        default:
            return FALSE;
        }
        return TRUE;
    }
    case CPL_DBLCLK: {
        int idx = (int)lParam1;
        switch (idx) {
        case COLOR_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_COLOR), hwndCPL, ColorDlgProc);
            break;
        case FONTS_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_FONTS), hwndCPL, FontsDlgProc);
            break;
        case PORTS_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_PORTS), hwndCPL, PortsDlgProc);
            break;
        case MOUSE_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_MOUSE), hwndCPL, MouseDlgProc);
            break;
        case DESKTOP_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_DESKTOP), hwndCPL, DesktopDlgProc);
            break;
        case KEYBOARD_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_KEYBOARD), hwndCPL, KeyboardDlgProc);
            break;
        case PRINTERS_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_PRINTERS), hwndCPL, PrintersDlgProc);
            break;
        case INTERNATIONAL_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_INTERNATIONAL), hwndCPL, InternationalDlgProc);
            break;
        case DATETIME_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_DATETIME), hwndCPL, DateTimeDlgProc);
            break;
        case NETWORK_IDX:
            DialogBox(g_hInst, MAKEINTRESOURCE(DLG_NETWORK), hwndCPL, NetworkDlgProc);
            break;
        }
        return TRUE;
    }
    case CPL_STOP:
    case CPL_EXIT:
        return TRUE;
    }
    return FALSE;
}

/* ----- Точка входа DLL ----- */
int WINAPI LibMain(HINSTANCE hInst, WORD wDataSeg, WORD wHeapSize, LPSTR lpszCmdLine)
{
    g_hInst = hInst;
    if (wHeapSize > 0)
        UnlockData(0);
    return 1;
}

/* ============================================================
 *  Color
 * ============================================================ */
static const char *szColorElements[] = {
    "Active Title Bar", "Inactive Title Bar", "Title Bar Text",
    "Menu Bar", "Menu Text", "Window Background", "Window Text",
    "Scrollbars", "Button Face", "Button Shadow", "Border", NULL
};

BOOL CALLBACK ColorDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        HWND hList = GetDlgItem(hDlg, IDC_COL_ELEMENTS);
        int i;
        for (i = 0; szColorElements[i]; i++)
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szColorElements[i]);
        SendMessage(hList, LB_SETCURSEL, 0, 0);
        SendDlgItemMessage(hDlg, IDC_COL_SCHEMES, CB_ADDSTRING, 0, (LPARAM)"Windows Default");
        SendDlgItemMessage(hDlg, IDC_COL_SCHEMES, CB_SETCURSEL, 0, 0);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/* ============================================================
 *  Fonts
 * ============================================================ */
static int CALLBACK EnumFontsProc(LPLOGFONT lplf, LPTEXTMETRIC lptm, int fType, LPARAM lParam)
{
    HWND hList = (HWND)lParam;
    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(lplf->lfFaceName));
    return 1;
}

BOOL CALLBACK FontsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        HWND hList = GetDlgItem(hDlg, IDC_FNT_LIST);
        HDC hdc = GetDC(hDlg);
        EnumFonts(hdc, (LPSTR)NULL, (FONTENUMPROC)EnumFontsProc, (LPSTR)hList);
        ReleaseDC(hDlg, hdc);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}

/* ============================================================
 *  Ports
 * ============================================================ */
BOOL CALLBACK PortsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        HWND hList = GetDlgItem(hDlg, IDC_PRT_LIST);
        char buf[1024];
        char *p;
        GetProfileString("ports", NULL, "", buf, sizeof(buf));
        p = buf;
        while (*p) {
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)p);
            p += lstrlen(p) + 1;
        }
        if (!SendMessage(hList, LB_GETCOUNT, 0, 0)) {
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"COM1:");
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"COM2:");
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"LPT1:");
        }
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}


/* ============================================================
 *  Desktop
 * ============================================================ */
BOOL CALLBACK DesktopDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        char buf[260];
        GetProfileString("Desktop", "Pattern", "(None)", buf, sizeof(buf));
        SetDlgItemText(hDlg, IDC_DT_PATTERN, buf);
        GetProfileString("Desktop", "Wallpaper", "", buf, sizeof(buf));
        SetDlgItemText(hDlg, IDC_DT_WALLPAPER, buf);
        GetProfileString("Desktop", "TileWallpaper", "0", buf, sizeof(buf));
        CheckDlgButton(hDlg, IDC_DT_TILE, atoi(buf));
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_DT_BROWSE_WP) {
            MessageBox(hDlg, "Enter path to .BMP file in Wallpaper field.", "Browse", MB_OK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDOK) {
            char pattern[260], wp[260], tile[4];
            GetDlgItemText(hDlg, IDC_DT_PATTERN, pattern, sizeof(pattern));
            WriteProfileString("Desktop", "Pattern", pattern);
            GetDlgItemText(hDlg, IDC_DT_WALLPAPER, wp, sizeof(wp));
            WriteProfileString("Desktop", "Wallpaper", wp);
            wsprintf(tile, "%d", IsDlgButtonChecked(hDlg, IDC_DT_TILE) ? 1 : 0);
            WriteProfileString("Desktop", "TileWallpaper", tile);
            SystemParametersInfo(SPI_SETDESKPATTERN, 0, pattern, SPIF_UPDATEINIFILE);
            if (lstrlen(wp) > 0)
                SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, wp, SPIF_UPDATEINIFILE);
            else
                SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, "", SPIF_UPDATEINIFILE);
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}


/* ============================================================
 *  Keyboard
 * ============================================================ */
BOOL CALLBACK KeyboardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        int delay, speed;
        SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &delay, 0);
        SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &speed, 0);
        SetScrollRange(GetDlgItem(hDlg, IDC_KB_DELAY), SB_CTL, 0, 3, FALSE);
        SetScrollPos(GetDlgItem(hDlg, IDC_KB_DELAY), SB_CTL, delay, TRUE);
        SetScrollRange(GetDlgItem(hDlg, IDC_KB_RATE), SB_CTL, 0, 31, FALSE);
        SetScrollPos(GetDlgItem(hDlg, IDC_KB_RATE), SB_CTL, speed, TRUE);
        return TRUE;
    }
    case WM_HSCROLL: {
        int id = GetWindowWord((HWND)lParam, GWW_ID);
        int pos = HIWORD(wParam);
        if (id == IDC_KB_DELAY)
            SystemParametersInfo(SPI_SETKEYBOARDDELAY, pos, NULL, 0);
        else if (id == IDC_KB_RATE)
            SystemParametersInfo(SPI_SETKEYBOARDSPEED, pos, NULL, 0);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}

/* ============================================================
 *  Printers
 * ============================================================ */
BOOL CALLBACK PrintersDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        HWND hList = GetDlgItem(hDlg, IDC_PRN_LIST);
        char buf[1024];
        char *p;
        GetProfileString("devices", NULL, "", buf, sizeof(buf));
        p = buf;
        while (*p) {
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)p);
            p += lstrlen(p) + 1;
        }
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}

/* ============================================================
 *  International
 * ============================================================ */
static const char *countries[] = {
    "United States", "Canada", "United Kingdom", "Australia",
    "France", "Germany", "Italy", "Japan", "Spain", "Sweden",
    NULL
};
static const int countryCodes[] = {1,2,44,61,33,49,39,81,34,46};

BOOL CALLBACK InternationalDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        HWND hCountry = GetDlgItem(hDlg, IDC_INTL_COUNTRY);
        char buf[32];
        int i, code, sel;
        for (i = 0; countries[i]; i++)
            SendMessage(hCountry, CB_ADDSTRING, 0, (LPARAM)countries[i]);
        GetProfileString("intl", "iCountry", "1", buf, sizeof(buf));
        code = atoi(buf);
        sel = 0;
        for (i = 0; countryCodes[i]; i++)
            if (countryCodes[i] == code) { sel = i; break; }
        SendMessage(hCountry, CB_SETCURSEL, sel, 0);
        CheckDlgButton(hDlg, IDC_INTL_MEASURE,
            GetProfileInt("intl", "iMeasure", 0) == 0);
        GetProfileString("intl", "sList", ",", buf, sizeof(buf));
        SetDlgItemText(hDlg, IDC_INTL_LISTSEP, buf);
        {
            int iDate = GetProfileInt("intl", "iDate", 0);
            CheckRadioButton(hDlg, IDC_INTL_DATEFORMAT, IDC_INTL_DATEFORMAT+2, IDC_INTL_DATEFORMAT+iDate);
        }
        {
            int iCurr = GetProfileInt("intl", "iCurrency", 0);
            CheckRadioButton(hDlg, IDC_INTL_CURRENCY, IDC_INTL_CURRENCY+3, IDC_INTL_CURRENCY+iCurr);
        }
        SetDlgItemInt(hDlg, IDC_INTL_DIGITS, GetProfileInt("intl", "iDigits", 2), FALSE);
        CheckDlgButton(hDlg, IDC_INTL_LEADZERO, GetProfileInt("intl", "iLzero", 0));
        CheckRadioButton(hDlg, IDC_INTL_TIMEFORMAT, IDC_INTL_TIMEFORMAT+1,
            IDC_INTL_TIMEFORMAT + (GetProfileInt("intl", "iTime", 0) ? 1 : 0));
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            HWND hCountry = GetDlgItem(hDlg, IDC_INTL_COUNTRY);
            int idx = (int)SendMessage(hCountry, CB_GETCURSEL, 0, 0);
            int i;
            if (idx >= 0) {
                char buf[10];
                wsprintf(buf, "%d", countryCodes[idx]);
                WriteProfileString("intl", "iCountry", buf);
            }
            WriteProfileString("intl", "iMeasure",
                IsDlgButtonChecked(hDlg, IDC_INTL_MEASURE) ? "0" : "1");
            {
                char buf[32];
                GetDlgItemText(hDlg, IDC_INTL_LISTSEP, buf, sizeof(buf));
                WriteProfileString("intl", "sList", buf);
            }
            for (i=0;i<3;i++)
                if (IsDlgButtonChecked(hDlg, IDC_INTL_DATEFORMAT+i)) {
                    char buf[4];
                    wsprintf(buf, "%d", i);
                    WriteProfileString("intl", "iDate", buf);
                    break;
                }
            for (i=0;i<4;i++)
                if (IsDlgButtonChecked(hDlg, IDC_INTL_CURRENCY+i)) {
                    char buf[4];
                    wsprintf(buf, "%d", i);
                    WriteProfileString("intl", "iCurrency", buf);
                    break;
                }
            {
                int dig = GetDlgItemInt(hDlg, IDC_INTL_DIGITS, NULL, FALSE);
                char buf[4];
                wsprintf(buf, "%d", dig);
                WriteProfileString("intl", "iDigits", buf);
            }
            WriteProfileString("intl", "iLzero",
                IsDlgButtonChecked(hDlg, IDC_INTL_LEADZERO) ? "1" : "0");
            WriteProfileString("intl", "iTime",
                IsDlgButtonChecked(hDlg, IDC_INTL_TIMEFORMAT) ? "0" : "1");
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

/* ============================================================
 *  Date/Time
 * ============================================================ */
static WORD g_dtYear, g_dtMonth, g_dtDay, g_dtHour, g_dtMinute, g_dtSecond;
static WORD g_origYear, g_origMonth, g_origDay, g_origHour, g_origMinute, g_origSecond;

static void GetLocalDateTime(WORD *y, WORD *m, WORD *d, WORD *h, WORD *min, WORD *s)
{
    static union REGS regs;
    regs.h.ah = 0x2A; intdos(&regs, &regs);
    *y = regs.x.cx; *m = regs.h.dh; *d = regs.h.dl;
    regs.h.ah = 0x2C; intdos(&regs, &regs);
    *h = regs.h.ch; *min = regs.h.cl; *s = regs.h.dh;
}

static void SetLocalDateTime(WORD y, WORD m, WORD d, WORD h, WORD min, WORD s)
{
    static union REGS regs;
    regs.h.ah = 0x2B; regs.x.cx = y; regs.h.dh = m; regs.h.dl = d;
    intdos(&regs, &regs);
    regs.h.ah = 0x2D; regs.h.ch = h; regs.h.cl = min; regs.h.dh = s;
    intdos(&regs, &regs);
}

static void UpdateDateDisplay(HWND hDlg) {
    char buf[64];
    wsprintf(buf, "%s %u, %u", "Date", g_dtMonth, g_dtYear);
    SetDlgItemText(hDlg, IDC_DT_DATE_TEXT, buf);
}
static void UpdateTimeDisplay(HWND hDlg) {
    char buf[32];
    wsprintf(buf, "%02u:%02u:%02u", g_dtHour, g_dtMinute, g_dtSecond);
    SetDlgItemText(hDlg, IDC_DT_TIME_TEXT, buf);
}
static void DrawCalendar(HDC hdc, LPRECT rc) {
    char buf[8];
    wsprintf(buf, "%02u", g_dtDay);
    DrawText(hdc, buf, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

BOOL CALLBACK DateTimeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        GetLocalDateTime(&g_dtYear, &g_dtMonth, &g_dtDay, &g_dtHour, &g_dtMinute, &g_dtSecond);
        _fmemcpy(&g_origYear, &g_dtYear, sizeof(WORD)*6);
        UpdateDateDisplay(hDlg);
        UpdateTimeDisplay(hDlg);
        return TRUE;
    case WM_DRAWITEM:
        if (wParam == IDC_DT_CALENDAR) {
            LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
            DrawCalendar(lpDIS->hDC, &lpDIS->rcItem);
            return TRUE;
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            SetLocalDateTime(g_dtYear, g_dtMonth, g_dtDay, g_dtHour, g_dtMinute, g_dtSecond);
            EndDialog(hDlg, IDOK);
        }
        if (LOWORD(wParam) == IDCANCEL) {
            SetLocalDateTime(g_origYear, g_origMonth, g_origDay, g_origHour, g_origMinute, g_origSecond);
            EndDialog(hDlg, IDCANCEL);
        }
        return TRUE;
    }
    return FALSE;
}

/* ============================================================
 *  Network
 * ============================================================ */
BOOL CALLBACK NetworkDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
        EndDialog(hDlg, LOWORD(wParam));
    return FALSE;
}
