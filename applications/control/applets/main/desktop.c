#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

/* Типы для недокументированных функций */
typedef BOOL (FAR PASCAL *SETDESKPATTERNPROC)(void);
typedef BOOL (FAR PASCAL *SETDESKWALLPAPERPROC)(LPSTR);

/* Ординалы */
#define ORD_SETDESKPATTERN   279
#define ORD_SETDESKWALLPAPER 285

/* Вспомогательная функция для заполнения комбобокса именами ключей из секции WIN.INI */
static void FillComboFromProfile(HWND hCombo, LPCSTR section)
{
    char buf[512];
    char *p;
    int cnt;

    cnt = GetProfileString(section, NULL, "", buf, sizeof(buf));
    if (cnt == 0) return;

    for (p = buf; *p; p += lstrlen(p) + 1)
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)p);
}

/* Вспомогательная функция для записи значения в WIN.INI и оповещения */
static void ApplyDesktopSetting(LPCSTR key, LPCSTR value)
{
    WriteProfileString("Desktop", key, value);
    SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)"Desktop");
}

BOOL CALLBACK DesktopDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buf[260];
    WORD w;
    int pos;
    HWND hCtrl;
    static SETDESKPATTERNPROC lpfnSetDeskPattern = NULL;
    static SETDESKWALLPAPERPROC lpfnSetDeskWallPaper = NULL;

    switch (msg) {
    case WM_INITDIALOG:
    {
        /* Получаем адреса недокументированных функций один раз */
        if (!lpfnSetDeskPattern) {
            HMODULE hUser = GetModuleHandle("USER");
            lpfnSetDeskPattern = (SETDESKPATTERNPROC)GetProcAddress(hUser, (LPSTR)(DWORD)ORD_SETDESKPATTERN);
            lpfnSetDeskWallPaper = (SETDESKWALLPAPERPROC)GetProcAddress(hUser, (LPSTR)(DWORD)ORD_SETDESKWALLPAPER);
        }

        /* --- Pattern --- */
        hCtrl = GetDlgItem(hDlg, IDC_DT_PATTERN_COMBO);
        FillComboFromProfile(hCtrl, "Patterns");
        GetProfileString("Desktop", "Pattern", "(None)", buf, sizeof(buf));
        if (SendMessage(hCtrl, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)buf) == CB_ERR)
            SendMessage(hCtrl, CB_INSERTSTRING, 0, (LPARAM)buf);
        SendMessage(hCtrl, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)buf);

        /* --- Wallpaper --- */
        hCtrl = GetDlgItem(hDlg, IDC_DT_WALLPAPER_COMBO);
        FillComboFromProfile(hCtrl, "Wallpapers");
        GetProfileString("Desktop", "Wallpaper", "", buf, sizeof(buf));
        if (buf[0] && SendMessage(hCtrl, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)buf) == CB_ERR)
            SendMessage(hCtrl, CB_INSERTSTRING, 0, (LPARAM)buf);
        SendMessage(hCtrl, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)buf);

        /* Tile/Center */
        GetProfileString("Desktop", "TileWallpaper", "0", buf, sizeof(buf));
        CheckRadioButton(hDlg, IDC_DT_CENTER, IDC_DT_TILE,
                         atoi(buf) ? IDC_DT_TILE : IDC_DT_CENTER);

        /* --- Icon Spacing --- */
        ControlPanelInfo(CPI_ICONSPACING, 0, (LPSTR)&w);  /* get */
        SetDlgItemInt(hDlg, IDC_DT_ICON_SPACING, w, FALSE);

        /* --- Cursor Blink Rate (ms, диапазон 200..1200, скроллбар 1..100) --- */
        w = GetCaretBlinkTime();
        if (w < 200) w = 200; if (w > 1200) w = 1200;
        pos = (w - 200) / 10;  /* 0..100 */
        if (pos < 1) pos = 1;
        if (pos > 100) pos = 100;
        SetScrollRange(GetDlgItem(hDlg, IDC_DT_CURSOR_SPEED), SB_CTL, 1, 100, FALSE);
        SetScrollPos(GetDlgItem(hDlg, IDC_DT_CURSOR_SPEED), SB_CTL, pos, TRUE);

        /* --- Grid Granularity --- */
        GetProfileString("windows", "GridGranularity", "0", buf, sizeof(buf));
        SetDlgItemInt(hDlg, IDC_DT_GRANULARITY, atoi(buf), FALSE);

        /* --- Border Width --- */
        ControlPanelInfo(CPI_GETBORDER, 0, (LPSTR)&w);  /* get */
        SetDlgItemInt(hDlg, IDC_DT_BORDER_WIDTH, w, FALSE);

        return TRUE;
    }

    case WM_HSCROLL:
    {
        if ((HWND)HIWORD(lParam) == GetDlgItem(hDlg, IDC_DT_CURSOR_SPEED)) {
            pos = LOWORD(wParam);
            w = 200 + pos * 10;
            if (w < 200) w = 200;
            if (w > 1200) w = 1200;
            SetScrollPos(GetDlgItem(hDlg, IDC_DT_CURSOR_SPEED), SB_CTL, pos, TRUE);
            SetCaretBlinkTime(w);
        }
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_DT_EDIT_PATTERN) {
            MessageBox(hDlg, "Pattern Editor not implemented.", "Desktop", MB_OK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDOK) {
            /* --- Pattern --- */
            GetDlgItemText(hDlg, IDC_DT_PATTERN_COMBO, buf, sizeof(buf));
            ApplyDesktopSetting("Pattern", buf);
            if (lpfnSetDeskPattern) {
                lpfnSetDeskPattern();
                InvalidateRect(GetDesktopWindow(), NULL, TRUE);
            }

            /* --- Wallpaper --- */
            GetDlgItemText(hDlg, IDC_DT_WALLPAPER_COMBO, buf, sizeof(buf));
            ApplyDesktopSetting("Wallpaper", buf);
            wsprintf(buf, "%d", IsDlgButtonChecked(hDlg, IDC_DT_TILE) ? 1 : 0);
            ApplyDesktopSetting("TileWallpaper", buf);
            if (lpfnSetDeskWallPaper && buf[0]) {
                lpfnSetDeskWallPaper(buf);
                InvalidateRect(GetDesktopWindow(), NULL, TRUE);
            }

            /* --- Icon Spacing --- */
            w = GetDlgItemInt(hDlg, IDC_DT_ICON_SPACING, NULL, FALSE);
            ControlPanelInfo(CPI_ICONSPACING, w, NULL);  /* set (lpBuffer=NULL) */

            /* --- Cursor Blink Rate --- */
            {
                WORD caret = GetCaretBlinkTime();
                wsprintf(buf, "%u", caret);
                WriteProfileString("windows", "CursorBlinkRate", buf);
            }

            /* --- Grid Granularity --- */
            w = GetDlgItemInt(hDlg, IDC_DT_GRANULARITY, NULL, FALSE);
            wsprintf(buf, "%u", w);
            WriteProfileString("windows", "GridGranularity", buf);
            SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)"windows");

            /* --- Border Width --- */
            w = GetDlgItemInt(hDlg, IDC_DT_BORDER_WIDTH, NULL, FALSE);
            ControlPanelInfo(CPI_SETBORDER, w, NULL);

            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}
