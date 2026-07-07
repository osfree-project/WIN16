/*
 *  intl_num.c Ц диалог формата чисел
 */
#include "intl.h"

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
