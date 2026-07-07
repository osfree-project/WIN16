/*
 *  intl_time.c Ц диалог формата времени
 */
#include "intl.h"

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
