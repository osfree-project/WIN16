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
BOOL CALLBACK PrintersDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK InternationalDlgProc(HWND, UINT, WPARAM, LPARAM);
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
	    lpInfo->idIcon = IDI_COLORS;
            break;
        case FONTS_IDX:
            lpInfo->idName = IDS_FONTS_NAME;
            lpInfo->idInfo = IDS_FONTS_INFO;
	    lpInfo->idIcon = IDI_FONTS;
            break;
        case PORTS_IDX:
            lpInfo->idName = IDS_PORTS_NAME;
            lpInfo->idInfo = IDS_PORTS_INFO;
            break;
        case MOUSE_IDX:
            lpInfo->idName = IDS_MOUSE_NAME;
            lpInfo->idInfo = IDS_MOUSE_INFO;
	    lpInfo->idIcon = IDI_MOUSE;
            break;
        case DESKTOP_IDX:
            lpInfo->idName = IDS_DESKTOP_NAME;
            lpInfo->idInfo = IDS_DESKTOP_INFO;
	    lpInfo->idIcon = IDI_DESKTOP;
            break;
        case KEYBOARD_IDX:
            lpInfo->idName = IDS_KEYBOARD_NAME;
            lpInfo->idInfo = IDS_KEYBOARD_INFO;
	    lpInfo->idIcon = IDI_KEYBOARD;
            break;
        case PRINTERS_IDX:
            lpInfo->idName = IDS_PRINTERS_NAME;
            lpInfo->idInfo = IDS_PRINTERS_INFO;
	    lpInfo->idIcon = IDI_PRINTERS;
            break;
        case INTERNATIONAL_IDX:
            lpInfo->idName = IDS_INTERNATIONAL_NAME;
            lpInfo->idInfo = IDS_INTERNATIONAL_INFO;
	    lpInfo->idIcon = IDI_INTL;
            break;
        case DATETIME_IDX:
            lpInfo->idName = IDS_DATETIME_NAME;
            lpInfo->idInfo = IDS_DATETIME_INFO;
	    lpInfo->idIcon = IDI_TIME;
            break;
        case NETWORK_IDX:
            lpInfo->idName = IDS_NETWORK_NAME;
            lpInfo->idInfo = IDS_NETWORK_INFO;
	    lpInfo->idIcon = IDI_NETWORK;
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
 *  Network
 * ============================================================ */
BOOL CALLBACK NetworkDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
        EndDialog(hDlg, LOWORD(wParam));
    return FALSE;
}
