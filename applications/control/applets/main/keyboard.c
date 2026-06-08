/*
 * keyboard.c -- Keyboard applet dialog procedure (Windows 3.0)
 */

#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

/* ControlPanelInfo constants */
#define CPI_GETKEYBOARDSPEED 10
#define CPI_SETKEYBOARDSPEED 11

/* Prototype (недокументированная функция из USER) */
VOID FAR PASCAL ControlPanelInfo(int nInfoType, WORD wData, LPSTR lpBuffer);

static int   g_keyboardSpeed = 31;   /* текущая скорость (1..32) */
static int   g_origSpeed;
static HWND  hSpeedScroll = NULL;

BOOL CALLBACK KeyboardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
    {
        WORD wBuf;

        /* Получаем текущую скорость клавиатуры */
        ControlPanelInfo(CPI_GETKEYBOARDSPEED, 0, (LPSTR)&wBuf);
        g_keyboardSpeed = wBuf;
        if (g_keyboardSpeed < 1)  g_keyboardSpeed = 1;
        if (g_keyboardSpeed > 32) g_keyboardSpeed = 32;
        g_origSpeed = g_keyboardSpeed;

        hSpeedScroll = GetDlgItem(hDlg, IDC_KB_SPEED);
        SetScrollRange(hSpeedScroll, SB_CTL, 1, 32, FALSE);
        SetScrollPos(hSpeedScroll, SB_CTL, g_keyboardSpeed, TRUE);
        return TRUE;
    }

    case WM_HSCROLL:
    {
        int code = wParam;
        int pos = LOWORD(lParam);

        if ((HWND)HIWORD(lParam) != hSpeedScroll)
            return FALSE;

        switch (code) {
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            g_keyboardSpeed = pos;
            break;
        case SB_LINELEFT:
            if (g_keyboardSpeed > 1) g_keyboardSpeed--;
            break;
        case SB_LINERIGHT:
            if (g_keyboardSpeed < 32) g_keyboardSpeed++;
            break;
        case SB_PAGELEFT:
            g_keyboardSpeed -= 5;
            if (g_keyboardSpeed < 1) g_keyboardSpeed = 1;
            break;
        case SB_PAGERIGHT:
            g_keyboardSpeed += 5;
            if (g_keyboardSpeed > 32) g_keyboardSpeed = 32;
            break;
        default:
            return FALSE;
        }
        SetScrollPos(hSpeedScroll, SB_CTL, g_keyboardSpeed, TRUE);
        ControlPanelInfo(CPI_SETKEYBOARDSPEED, g_keyboardSpeed, NULL);
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        {
            char buf[16];
            wsprintf(buf, "%d", g_keyboardSpeed);
            WriteProfileString("windows", "KeyboardSpeed", buf);
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            /* Восстанавливаем исходную скорость и закрываем */
            ControlPanelInfo(CPI_SETKEYBOARDSPEED, g_origSpeed, NULL);
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
