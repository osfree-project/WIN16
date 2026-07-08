/*
 *  intl_num.c – диалог формата чисел (Number Format)
 */
#include "intl.h"

BOOL WINAPI NumberFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        /* Текущие значения глобальных переменных в поля */
        SetDlgItemText(hDlg, IDC_NUMFMT_1000SEP, g_iniThousand);
        SetDlgItemText(hDlg, IDC_NUMFMT_DECSEP,  g_iniDecimal);
        {
            char buf[8];
            wsprintf(buf, "%d", g_iniDigits);
            SetDlgItemText(hDlg, IDC_NUMFMT_DECDIG, buf);
        }
        /* Ведущий ноль: g_iniLZero==0 -> ".7", иначе "0.7" */
        CheckRadioButton(hDlg, IDC_NUMFMT_LZ_DOT7, IDC_NUMFMT_LZ_0DOT7,
                         g_iniLZero ? IDC_NUMFMT_LZ_0DOT7 : IDC_NUMFMT_LZ_DOT7);
        return TRUE;

    case WM_COMMAND:
        /* Ручное переключение радиокнопок (BS_RADIOBUTTON) */
        if (HIWORD(lParam) == BN_CLICKED)
        {
            if (wParam == IDC_NUMFMT_LZ_DOT7 || wParam == IDC_NUMFMT_LZ_0DOT7)
                CheckRadioButton(hDlg, IDC_NUMFMT_LZ_DOT7, IDC_NUMFMT_LZ_0DOT7, wParam);
        }

        if (wParam == IDOK)
        {
            /* Сохраняем разделитель тысяч */
            GetDlgItemText(hDlg, IDC_NUMFMT_1000SEP, g_iniThousand, sizeof(g_iniThousand));
            /* Сохраняем десятичный разделитель */
            GetDlgItemText(hDlg, IDC_NUMFMT_DECSEP, g_iniDecimal, sizeof(g_iniDecimal));
            /* Сохраняем количество знаков после запятой */
            {
                char buf[8];
                GetDlgItemText(hDlg, IDC_NUMFMT_DECDIG, buf, sizeof(buf));
                g_iniDigits = atoi(buf);
                if (g_iniDigits < 0)  g_iniDigits = 0;
                if (g_iniDigits > 9)  g_iniDigits = 9;
            }
            /* Ведущий ноль */
            g_iniLZero = IsDlgButtonChecked(hDlg, IDC_NUMFMT_LZ_0DOT7) ? 1 : 0;

            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        if (wParam == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
