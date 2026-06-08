/*
 *  desktop.c – без отладки, с рабочим восстановлением узора.
 */

#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

typedef BOOL (FAR PASCAL *SETDESKPATTERNPROC)(void);
typedef BOOL (FAR PASCAL *SETDESKWALLPAPERPROC)(LPSTR);

#define ORD_SETDESKPATTERN   279
#define ORD_SETDESKWALLPAPER 285

static HWND hCaretWnd = NULL;
static BOOL bCaretCreated = FALSE;

/* ------------------------------------------------------------
 * Безопасное копирование строки с ограничением длины
 * ------------------------------------------------------------ */
static void MyLstrcpyn(LPSTR dest, LPCSTR src, int maxLen)
{
    int i;
    if (maxLen <= 0) return;
    for (i = 0; i < maxLen - 1 && src[i] != '\0'; i++)
        dest[i] = src[i];
    dest[i] = '\0';
}

/* ------------------------------------------------------------
 * Получить значение паттерна по его имени из CONTROL.INI
 * ------------------------------------------------------------ */
static void GetPatternValue(LPCSTR lpszName, LPSTR lpszValue, int cbValueMax)
{
    char szIniPath[128];
    GetWindowsDirectory(szIniPath, sizeof(szIniPath));
    lstrcat(szIniPath, "\\CONTROL.INI");
    GetPrivateProfileString("Patterns", lpszName, "", lpszValue, cbValueMax, szIniPath);
}

/* ------------------------------------------------------------
 * Заполнить комбобокс именами паттернов из CONTROL.INI
 * ------------------------------------------------------------ */
static void FillPatternCombo(HWND hCombo, LPCSTR currentPatternValue)
{
    char szIniPath[128];
    char buf[2048];
    char FAR *p;
    int cnt;
    LRESULT idx;
    char currentName[64];
    char searchValue[260];

    currentName[0] = '\0';

    /* Копируем искомое значение (из WIN.INI) */
    MyLstrcpyn(searchValue, currentPatternValue, sizeof(searchValue));

    GetWindowsDirectory(szIniPath, sizeof(szIniPath));
    lstrcat(szIniPath, "\\CONTROL.INI");

    cnt = GetPrivateProfileString("Patterns", NULL, "", buf, sizeof(buf), szIniPath);
    if (cnt == 0 || buf[0] == '\0') {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"(None)");
        SendMessage(hCombo, CB_SETCURSEL, 0, 0);
        return;
    }

    /* Добавляем имена в комбобокс */
    p = (char FAR *)buf;
    while (*p) {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR)p);
        p += lstrlen((LPSTR)p) + 1;
    }

    /* Ищем имя текущего узора по его значению */
    if (searchValue[0]) {
        char FAR *pKey = (char FAR *)buf;
        while (*pKey) {
            char valueBuf[260];
            GetPatternValue((LPSTR)pKey, valueBuf, sizeof(valueBuf));
            if (lstrcmp(valueBuf, searchValue) == 0) {
                MyLstrcpyn(currentName, (LPSTR)pKey, sizeof(currentName));
                break;
            }
            pKey += lstrlen((LPSTR)pKey) + 1;
        }
    }

    /* Устанавливаем выбор */
    if (currentName[0]) {
        idx = SendMessage(hCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPSTR)currentName);
        if (idx == CB_ERR) {
            /* Если не найден – добавляем и выбираем */
            SendMessage(hCombo, CB_INSERTSTRING, 0, (LPARAM)(LPSTR)currentName);
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);
        }
    } else {
        SendMessage(hCombo, CB_SETCURSEL, 0, 0);
    }
}

/* ------------------------------------------------------------
 * Заполнить комбобокс именами .BMP из каталога Windows
 * ------------------------------------------------------------ */
static void FillWallpaperCombo(HWND hCombo, LPCSTR currentWallpaper)
{
    char szMask[260];
    static struct find_t ff;
    int done;
    int count = 0;
    LRESULT idx;

    GetWindowsDirectory(szMask, sizeof(szMask));
    lstrcat(szMask, "\\*.BMP");

    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"(None)");

    done = _dos_findfirst(szMask, _A_NORMAL, &ff);
    while (done == 0) {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPSTR)ff.name);
        count++;
        done = _dos_findnext(&ff);
    }
    _dos_findclose(&ff);

    if (currentWallpaper[0]) {
        idx = SendMessage(hCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)currentWallpaper);
        if (idx != CB_ERR)
            SendMessage(hCombo, CB_SETCURSEL, idx, 0);
        else {
            SendMessage(hCombo, CB_INSERTSTRING, 0, (LPARAM)currentWallpaper);
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);
        }
    } else {
        SendMessage(hCombo, CB_SETCURSEL, 0, 0);
    }
}

/* ------------------------------------------------------------
 * Пересоздать каретку с заданным временем мигания
 * ------------------------------------------------------------ */
static void RecreateCaret(WORD blinkTime)
{
    if (bCaretCreated) {
        DestroyCaret();
        bCaretCreated = FALSE;
    }
    if (hCaretWnd) {
        RECT rc;
        GetClientRect(hCaretWnd, &rc);
        CreateCaret(hCaretWnd, NULL, 2, rc.bottom - rc.top);
        SetCaretPos(0, 0);
        ShowCaret(hCaretWnd);
        bCaretCreated = TRUE;
        SetCaretBlinkTime(blinkTime);
    }
}

/* =================================================================== */
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
        if (!lpfnSetDeskPattern) {
            HMODULE hUser = GetModuleHandle("USER");
            lpfnSetDeskPattern = (SETDESKPATTERNPROC)GetProcAddress(hUser,
                (LPSTR)(DWORD)ORD_SETDESKPATTERN);
            lpfnSetDeskWallPaper = (SETDESKWALLPAPERPROC)GetProcAddress(hUser,
                (LPSTR)(DWORD)ORD_SETDESKWALLPAPER);
        }

        hCtrl = GetDlgItem(hDlg, IDC_DT_PATTERN_COMBO);
        GetProfileString("Desktop", "Pattern", "", buf, sizeof(buf));
        FillPatternCombo(hCtrl, buf);

        hCtrl = GetDlgItem(hDlg, IDC_DT_WALLPAPER_COMBO);
        GetProfileString("Desktop", "Wallpaper", "", buf, sizeof(buf));
        FillWallpaperCombo(hCtrl, buf);

        GetProfileString("Desktop", "TileWallpaper", "0", buf, sizeof(buf));
        CheckRadioButton(hDlg, IDC_DT_CENTER, IDC_DT_TILE,
                         atoi(buf) ? IDC_DT_TILE : IDC_DT_CENTER);

        ControlPanelInfo(CPI_ICONSPACING, 0, (LPSTR)&w);
        SetDlgItemInt(hDlg, IDC_DT_ICON_SPACING, w, FALSE);

        w = GetCaretBlinkTime();
        if (w < 200) w = 200;
        if (w > 1200) w = 1200;
        pos = 100 - (w - 200) / 10;
        if (pos < 1) pos = 1;
        if (pos > 100) pos = 100;

        SetScrollRange(GetDlgItem(hDlg, IDC_DT_CURSOR_SPEED), SB_CTL, 1, 100, FALSE);
        SetScrollPos(GetDlgItem(hDlg, IDC_DT_CURSOR_SPEED), SB_CTL, pos, TRUE);

        hCaretWnd = GetDlgItem(hDlg, IDC_DT_CARET);
        RecreateCaret(w);

        GetProfileString("windows", "GridGranularity", "0", buf, sizeof(buf));
        SetDlgItemInt(hDlg, IDC_DT_GRANULARITY, atoi(buf), FALSE);

        ControlPanelInfo(CPI_GETBORDER, 0, (LPSTR)&w);
        SetDlgItemInt(hDlg, IDC_DT_BORDER_WIDTH, w, FALSE);

        return TRUE;
    }

    case WM_HSCROLL:
    {
        HWND hScroll = (HWND)HIWORD(lParam);
        if (hScroll == GetDlgItem(hDlg, IDC_DT_CURSOR_SPEED)) {
            int code = wParam;
            int pos = LOWORD(lParam);

            switch (code) {
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                break;
            case SB_LINELEFT:
                pos = GetScrollPos(hScroll, SB_CTL) - 1;
                if (pos < 1) pos = 1;
                break;
            case SB_LINERIGHT:
                pos = GetScrollPos(hScroll, SB_CTL) + 1;
                if (pos > 100) pos = 100;
                break;
            case SB_PAGELEFT:
                pos = GetScrollPos(hScroll, SB_CTL) - 10;
                if (pos < 1) pos = 1;
                break;
            case SB_PAGERIGHT:
                pos = GetScrollPos(hScroll, SB_CTL) + 10;
                if (pos > 100) pos = 100;
                break;
            default:
                return TRUE;
            }

            w = 200 + (100 - pos) * 10;
            if (w < 200) w = 200;
            if (w > 1200) w = 1200;

            SetScrollPos(hScroll, SB_CTL, pos, TRUE);
            RecreateCaret(w);
        }
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_DT_EDIT_PATTERN) {
            MessageBox(hDlg, "Pattern Editor not implemented.", "Desktop", MB_OK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDOK) {
            char szPatternName[260];
            char szPatternValue[260];

            if (bCaretCreated) {
                DestroyCaret();
                bCaretCreated = FALSE;
            }

            GetDlgItemText(hDlg, IDC_DT_PATTERN_COMBO, szPatternName, sizeof(szPatternName));
            GetPatternValue(szPatternName, szPatternValue, sizeof(szPatternValue));
            if (szPatternValue[0]) {
                WriteProfileString("Desktop", "Pattern", szPatternValue);
            } else {
                WriteProfileString("Desktop", "Pattern", szPatternName);
            }

            GetDlgItemText(hDlg, IDC_DT_WALLPAPER_COMBO, buf, sizeof(buf));
            WriteProfileString("Desktop", "Wallpaper", buf);
            wsprintf(buf, "%d", IsDlgButtonChecked(hDlg, IDC_DT_TILE) ? 1 : 0);
            WriteProfileString("Desktop", "TileWallpaper", buf);

            w = GetDlgItemInt(hDlg, IDC_DT_ICON_SPACING, NULL, FALSE);
            ControlPanelInfo(CPI_ICONSPACING, w, NULL);

            {
                WORD caret = GetCaretBlinkTime();
                wsprintf(buf, "%u", caret);
                WriteProfileString("windows", "CursorBlinkRate", buf);
            }

            w = GetDlgItemInt(hDlg, IDC_DT_GRANULARITY, NULL, FALSE);
            wsprintf(buf, "%u", w);
            WriteProfileString("windows", "GridGranularity", buf);

            w = GetDlgItemInt(hDlg, IDC_DT_BORDER_WIDTH, NULL, FALSE);
            ControlPanelInfo(CPI_SETBORDER, w, NULL);

            if (lpfnSetDeskPattern)
                lpfnSetDeskPattern();

            if (lpfnSetDeskWallPaper) {
                GetDlgItemText(hDlg, IDC_DT_WALLPAPER_COMBO, buf, sizeof(buf));
                if (buf[0])
                    lpfnSetDeskWallPaper(buf);
            }

            SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)"Desktop");
            SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)"windows");
            InvalidateRect(GetDesktopWindow(), NULL, TRUE);

            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            if (bCaretCreated) {
                DestroyCaret();
                bCaretCreated = FALSE;
            }
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
