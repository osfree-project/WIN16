#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

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
