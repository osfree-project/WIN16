/*
 * currency.c - Диалог формата валюты (новый ресурс)
 *              Все near-буферы переданы через FAR-указатели.
 */
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "winnls.h"
#include "intl_currency.h"

/* Внешние глобальные переменные (определены в inter.c) */
extern char g_iniCurrencySym[8];
extern int  g_iniCurrencyFmt;
extern int  g_iniNegCurr;
extern int  g_iniDigits;

/* Строки для комбобоксов размещения */
static const char FAR *placementItems[] = { "$1", "1$", "$ 1", "1 $" };
static const int placementValues[] = { 0, 1, 2, 3 };

/* Строки для комбобокса отрицательного формата (iNegCurr 0-7) */
static const char FAR *negativeItems[] = {
    "($1)", "-$1", "$-1", "$1-", "(1$)", "-1$", "1-$", "1$-"
};

BOOL WINAPI CurrencyFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
    {
        char buf[16];
        char FAR *farbuf = (char FAR *)buf;
        char FAR *farsym = (char FAR *)g_iniCurrencySym;
        HWND hPlacement, hNegative;
        int i, sel;

        hPlacement = GetDlgItem(hDlg, IDC_CURRFMT_PLACEMENT);
        hNegative  = GetDlgItem(hDlg, IDC_CURRFMT_NEGATIVE);

        /* Заполняем комбобокс размещения */
        for (i = 0; i < 4; i++) {
            SendMessage(hPlacement, CB_ADDSTRING, 0, (LPARAM)(LPSTR)placementItems[i]);
        }
        for (i = 0; i < 4; i++) {
            if (placementValues[i] == g_iniCurrencyFmt) break;
        }
        if (i < 4) sel = i; else sel = 0;
        SendMessage(hPlacement, CB_SETCURSEL, sel, 0);

        /* Заполняем комбобокс отрицательного формата */
        for (i = 0; i < 8; i++) {
            SendMessage(hNegative, CB_ADDSTRING, 0, (LPARAM)(LPSTR)negativeItems[i]);
        }
        SendMessage(hNegative, CB_SETCURSEL, (WPARAM)g_iniNegCurr, 0);

        /* Устанавливаем символ валюты и количество знаков */
        SetDlgItemText(hDlg, IDC_CURRFMT_SYMBOL_EDIT, farsym);
        wsprintf(farbuf, (LPCSTR)"%d", g_iniDigits);
        SetDlgItemText(hDlg, IDC_CURRFMT_DIGITS_EDIT, farbuf);

        return TRUE;
    }

    case WM_COMMAND:
        if (wParam == IDOK) {
            HWND hPlacement = GetDlgItem(hDlg, IDC_CURRFMT_PLACEMENT);
            HWND hNegative  = GetDlgItem(hDlg, IDC_CURRFMT_NEGATIVE);
            char buf[16];
            char FAR *farbuf = (char FAR *)buf;
            char FAR *farsym = (char FAR *)g_iniCurrencySym;
            LRESULT sel;

            /* Сохраняем формат размещения */
            sel = SendMessage(hPlacement, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel <= 3) {
                g_iniCurrencyFmt = placementValues[sel];
            }

            /* Сохраняем отрицательный формат */
            sel = SendMessage(hNegative, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel <= 7) {
                g_iniNegCurr = (int)sel;
            }

            /* Сохраняем символ валюты */
            GetDlgItemText(hDlg, IDC_CURRFMT_SYMBOL_EDIT, farsym, sizeof(g_iniCurrencySym)); /* теперь sizeof работает */

            /* Сохраняем количество десятичных знаков */
            GetDlgItemText(hDlg, IDC_CURRFMT_DIGITS_EDIT, farbuf, sizeof(buf));
            g_iniDigits = atoi(buf);
            if (g_iniDigits < 0) g_iniDigits = 0;

            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        if (wParam == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
