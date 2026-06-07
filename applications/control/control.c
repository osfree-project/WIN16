/*
 * control.c -- Клон Панели управления для Windows 3.0
 *
 * Динамическое меню Settings, иконка приложения, обработчики Help.
 */

#include <windows.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "cpl.h"
#include "control.h"

#define MAX_APPLETS         256
#define ICON_SPACING_X      75
#define ICON_SPACING_Y      60   /* высота ячейки */
#define MARGIN_X            15
#define MARGIN_Y            15
#define MAX_COLS            5

typedef struct {
    HMODULE         hLib;
    CPLAPPLET_PROC  proc;
    int             index;
    LONG            lData;
    HICON           hIcon;
    char            szName[32];
    char            szInfo[64];
    BOOL            bSelected;
} APPLET;

static APPLET    g_applets[MAX_APPLETS];
static int       g_numApplets = 0;
static int       g_selectedApplet = -1;
static HWND      g_hwndMain = NULL;
static HINSTANCE g_hInst = NULL;
static int       g_scrollPosY = 0;
static int       g_scrollPosX = 0;
static int       g_totalHeight = 0;
static int       g_totalWidth = 0;

/* Прототипы */
static BOOL LoadCplFile(LPCSTR path);
static void UnloadAllApplets(void);
static void ScanCplFiles(void);
static int  HitTest(int mx, int my);
static void DrawApplets(HDC hdc);
static void InvalidateApplet(int idx);
static void ScrollTo(int newPosY, int newPosX);
static void LaunchAppletByIndex(HWND hwnd, int appIndex);
static void BuildSettingsMenu(HWND hwnd);
static void OnHelpAbout(HWND hwnd);

/* Оконная процедура */
LRESULT WINAPI MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    HWND     hwnd;
    MSG      msg;

    g_hInst = hInstance;

    if (!hPrevInstance) {
        wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc   = MainWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(hInstance, "MYICON");
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = "CPLCloneClass";

        if (!RegisterClass(&wc))
            return FALSE;
    }

    hwnd = CreateWindow(
        "CPLCloneClass",
        "Control Panel",
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT,
        450, 380,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
        return FALSE;

    g_hwndMain = hwnd;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

LRESULT WINAPI MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        {
            HMENU hMenu = LoadMenu(g_hInst, "CTLPANELMENU");
            if (hMenu)
                SetMenu(hwnd, hMenu);
        }
        g_numApplets = 0;
        g_selectedApplet = -1;
        g_scrollPosY = 0;
        g_scrollPosX = 0;
        ScanCplFiles();

        /* Построение меню Settings на основе загруженных апплетов */
        BuildSettingsMenu(hwnd);

        if (g_numApplets == 0) {
            MessageBox(hwnd, "No Control Panel applets could be loaded.\n"
                             "Please make sure valid *.CPL files are installed.",
                       "Control Panel", MB_ICONEXCLAMATION | MB_OK);
        }

        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            SetScrollRange(hwnd, SB_VERT, 0, g_totalHeight, FALSE);
            SetScrollPos(hwnd, SB_VERT, 0, TRUE);
            SetScrollRange(hwnd, SB_HORZ, 0, g_totalWidth, FALSE);
            SetScrollPos(hwnd, SB_HORZ, 0, TRUE);
        }
        return 0;

case WM_SIZE: {
    RECT rc;
    int nMaxV, nMaxH;
    int curPosV, curPosH;

    GetClientRect(hwnd, &rc);

    nMaxV = g_totalHeight - rc.bottom;
    if (nMaxV < 0) nMaxV = 0;
    SetScrollRange(hwnd, SB_VERT, 0, nMaxV, TRUE);

    nMaxH = g_totalWidth - rc.right;
    if (nMaxH < 0) nMaxH = 0;
    SetScrollRange(hwnd, SB_HORZ, 0, nMaxH, TRUE);

    curPosV = GetScrollPos(hwnd, SB_VERT);
    if (curPosV > nMaxV) curPosV = nMaxV;
    curPosH = GetScrollPos(hwnd, SB_HORZ);
    if (curPosH > nMaxH) curPosH = nMaxH;

    if (curPosV != g_scrollPosY || curPosH != g_scrollPosX)
        ScrollTo(curPosV, curPosH);
    else
        InvalidateRect(hwnd, NULL, TRUE);

    return 0;
}

    case WM_VSCROLL: {
        int nMin, nMax, nPage, newPos;
        RECT rc;
        GetClientRect(hwnd, &rc);
        nPage = rc.bottom;
        GetScrollRange(hwnd, SB_VERT, &nMin, &nMax);
        newPos = GetScrollPos(hwnd, SB_VERT);
        switch (LOWORD(wParam)) {
        case SB_LINEUP:    newPos -= 20; break;
        case SB_LINEDOWN:  newPos += 20; break;
        case SB_PAGEUP:    newPos -= nPage; break;
        case SB_PAGEDOWN:  newPos += nPage; break;
        case SB_THUMBTRACK: newPos = HIWORD(wParam); break;
        default: return 0;
        }
        if (newPos < nMin) newPos = nMin;
        if (newPos > nMax) newPos = nMax;
        if (newPos != GetScrollPos(hwnd, SB_VERT)) {
            ScrollTo(newPos, g_scrollPosX);
        }
        return 0;
    }

    case WM_HSCROLL: {
        int nMin, nMax, nPage, newPos;
        RECT rc;
        GetClientRect(hwnd, &rc);
        nPage = rc.right;
        GetScrollRange(hwnd, SB_HORZ, &nMin, &nMax);
        newPos = GetScrollPos(hwnd, SB_HORZ);
        switch (LOWORD(wParam)) {
        case SB_LINEUP:    newPos -= 20; break;
        case SB_LINEDOWN:  newPos += 20; break;
        case SB_PAGEUP:    newPos -= nPage; break;
        case SB_PAGEDOWN:  newPos += nPage; break;
        case SB_THUMBTRACK: newPos = HIWORD(wParam); break;
        default: return 0;
        }
        if (newPos < nMin) newPos = nMin;
        if (newPos > nMax) newPos = nMax;
        if (newPos != GetScrollPos(hwnd, SB_HORZ)) {
            ScrollTo(g_scrollPosY, newPos);
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawApplets(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);
        int idx = HitTest(mx, my);
        if (g_selectedApplet >= 0 && g_selectedApplet != idx) {
            g_applets[g_selectedApplet].bSelected = FALSE;
            InvalidateApplet(g_selectedApplet);
        }
        if (idx >= 0) {
            g_selectedApplet = idx;
            g_applets[idx].bSelected = TRUE;
            InvalidateApplet(idx);
        } else {
            g_selectedApplet = -1;
        }
        return 0;
    }

    case WM_LBUTTONDBLCLK: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);
        int idx = HitTest(mx, my);
        if (idx >= 0) {
            g_applets[idx].proc(hwnd, CPL_DBLCLK,
                                (LONG)g_applets[idx].index,
                                g_applets[idx].lData);
        }
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDM_SETTINGS_EXIT) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
        if (LOWORD(wParam) >= IDM_SETTINGS_APPS_BASE &&
            LOWORD(wParam) < IDM_SETTINGS_APPS_BASE + g_numApplets) {
            int idx = LOWORD(wParam) - IDM_SETTINGS_APPS_BASE;
            LaunchAppletByIndex(hwnd, idx);
            return 0;
        }

        /* Обработка пунктов Help */
        switch (LOWORD(wParam)) {
        case IDM_HELP_INDEX:
        case IDM_HELP_KEYBOARD:
        case IDM_HELP_COMMANDS:
        case IDM_HELP_PROCEDURES:
        case IDM_HELP_USING_HELP:
            WinHelp(hwnd, "CONTROL.HLP", HELP_INDEX, 0L);
            return 0;
        case IDM_HELP_ABOUT:
            OnHelpAbout(hwnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        UnloadAllApplets();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void MyLstrcpyn(LPSTR dest, LPCSTR src, int iMaxLength)
{
    int i;
    if (iMaxLength <= 0) {
        if (iMaxLength == 0) return;
        dest[0] = '\0';
        return;
    }
    for (i = 0; i < iMaxLength - 1 && src[i] != '\0'; i++)
        dest[i] = src[i];
    dest[i] = '\0';
}

static BOOL LoadCplFile(LPCSTR path)
{
    HMODULE hLib;
    CPLAPPLET_PROC proc;
    int i, count;

    hLib = LoadLibrary(path);
    if ((UINT)hLib < 32) {
        char buf[300];
        wsprintf(buf, "Failed to load Control Panel applet:\n%s\n\nError code: %d", path, hLib);
        MessageBox(g_hwndMain, buf, "Control Panel", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    proc = (CPLAPPLET_PROC)GetProcAddress(hLib, "CPlApplet");
    if (!proc) {
        char buf[300];
        wsprintf(buf, "The file is not a valid Control Panel applet:\n%s\n\nMissing CPlApplet entry point.", path);
        MessageBox(g_hwndMain, buf, "Control Panel", MB_ICONEXCLAMATION | MB_OK);
        FreeLibrary(hLib);
        return FALSE;
    }

    if (!proc(g_hwndMain, CPL_INIT, 0L, 0L)) {
        MessageBox(g_hwndMain, "Control Panel applet failed to initialize.\nThe applet will be skipped.",
                   "Control Panel", MB_ICONEXCLAMATION | MB_OK);
        FreeLibrary(hLib);
        return FALSE;
    }

    count = proc(g_hwndMain, CPL_GETCOUNT, 0L, 0L);
    if (count <= 0) {
        FreeLibrary(hLib);
        return FALSE;
    }

    for (i = 0; i < count && g_numApplets < MAX_APPLETS; i++) {
        NEWCPLINFO newInfo;
        CPLINFO    info;
        HICON      hIcon = NULL;
        char       name[32] = "", desc[64] = "";
        LONG       lData = 0;
        BOOL       bGotInfo = FALSE;

        _fmemset(&newInfo, 0, sizeof(newInfo));
        newInfo.dwSize = sizeof(NEWCPLINFO);
        if (proc(g_hwndMain, CPL_NEWINQUIRE, (LONG)i, (LONG)(LPNEWCPLINFO)&newInfo)) {
            hIcon = newInfo.hIcon;
            MyLstrcpyn(name, newInfo.szName, 32);
            MyLstrcpyn(desc, newInfo.szInfo, 64);
            lData = newInfo.lData;
            bGotInfo = TRUE;
        } else {
            _fmemset(&info, 0, sizeof(info));
            if (proc(g_hwndMain, CPL_INQUIRE, (LONG)i, (LONG)(LPCPLINFO)&info)) {
                if (info.idIcon)
                    hIcon = LoadIcon(hLib, MAKEINTRESOURCE(info.idIcon));
                if (info.idName) {
                    if (!LoadString(hLib, info.idName, name, 31)) {
                        char buf[150];
                        wsprintf(buf, "Warning: Failed to load name string for applet #%d", i+1);
                        MessageBox(g_hwndMain, buf, "Control Panel", MB_ICONEXCLAMATION | MB_OK);
                    }
                }
                if (info.idInfo) {
                    LoadString(hLib, info.idInfo, desc, 63);
                }
                lData = info.lData;
                bGotInfo = TRUE;
            }
        }

        if (!bGotInfo) {
            char buf[150];
            wsprintf(buf, "Failed to get information for Control Panel applet #%d.\nIt will be skipped.", i+1);
            MessageBox(g_hwndMain, buf, "Control Panel", MB_ICONEXCLAMATION | MB_OK);
            continue;
        }

        if (!hIcon) hIcon = LoadIcon(NULL, IDI_APPLICATION);

        g_applets[g_numApplets].hLib  = hLib;
        g_applets[g_numApplets].proc  = proc;
        g_applets[g_numApplets].index = i;
        g_applets[g_numApplets].lData = lData;
        g_applets[g_numApplets].hIcon = hIcon;
        g_applets[g_numApplets].bSelected = FALSE;
        MyLstrcpyn(g_applets[g_numApplets].szName, name[0] ? name : "Unknown", 32);
        MyLstrcpyn(g_applets[g_numApplets].szInfo, desc[0] ? desc : "", 64);
        g_numApplets++;
    }
    return TRUE;
}

static void ScanCplFiles(void)
{
    char sysDir[256];
    char pattern[260];
    struct find_t ff;
    int done;

    GetSystemDirectory(sysDir, sizeof(sysDir));

    if (sysDir[0] == '\0') {
        MessageBox(g_hwndMain,
                   "Cannot determine Windows SYSTEM directory.\n"
                   "Please check your Windows installation.",
                   "Control Panel", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    wsprintf(pattern, "%s\\*.CPL", (LPSTR)sysDir);

    done = _dos_findfirst(pattern, _A_NORMAL, &ff);
    while (done == 0) {
        char fullPath[260];
        wsprintf(fullPath, "%s\\%s", (LPSTR)sysDir, (LPSTR)ff.name);
        LoadCplFile(fullPath);
        done = _dos_findnext(&ff);
    }
    _dos_findclose(&ff);

    /* Секция [MMCPL] */
    {
        char key[32], buf[260];
        int i;
        for (i = 0; ; i++) {
            wsprintf(key, "CPL%d", i);
            if (GetPrivateProfileString("MMCPL", key, "", buf, sizeof(buf), "CONTROL.INI") > 0) {
                LoadCplFile(buf);
            } else {
                break;
            }
        }
    }
}

static void UnloadAllApplets(void)
{
    HMODULE doneList[MAX_APPLETS];
    int doneCount = 0;
    int i, j;
    BOOL already;

    for (i = 0; i < g_numApplets; i++) {
        g_applets[i].proc(g_hwndMain, CPL_STOP,
                          (LONG)g_applets[i].index, g_applets[i].lData);
    }
    for (i = 0; i < g_numApplets; i++) {
        HMODULE hLib = g_applets[i].hLib;
        already = FALSE;
        for (j = 0; j < doneCount; j++) {
            if (doneList[j] == hLib) { already = TRUE; break; }
        }
        if (!already) {
            g_applets[i].proc(g_hwndMain, CPL_EXIT, 0L, 0L);
            FreeLibrary(hLib);
            doneList[doneCount++] = hLib;
        }
    }
    g_numApplets = 0;
}

static int HitTest(int mx, int my)
{
    int i, x, y, col;
    int iconW = GetSystemMetrics(SM_CXICON);
    int cellWidth = iconW + 32;

    x = MARGIN_X - g_scrollPosX;
    y = MARGIN_Y - g_scrollPosY;
    col = 0;

    for (i = 0; i < g_numApplets; i++) {
        RECT rc;
        rc.left   = x;
        rc.top    = y;
        rc.right  = x + cellWidth;
        rc.bottom = y + ICON_SPACING_Y;
        if (mx >= rc.left && mx <= rc.right &&
            my >= rc.top  && my <= rc.bottom)
            return i;

        col++;
        if (col >= MAX_COLS) { col = 0; x = MARGIN_X - g_scrollPosX; y += ICON_SPACING_Y; }
        else { x += cellWidth; }
    }
    return -1;
}

static void InvalidateApplet(int idx)
{
    int i, x, y, col;
    int iconW = GetSystemMetrics(SM_CXICON);
    int cellWidth = iconW + 32;
    RECT rc;

    x = MARGIN_X - g_scrollPosX;
    y = MARGIN_Y - g_scrollPosY;
    col = 0;
    for (i = 0; i < g_numApplets; i++) {
        if (i == idx) {
            rc.left = x;
            rc.top = y;
            rc.right = x + cellWidth;
            rc.bottom = y + ICON_SPACING_Y;
            InvalidateRect(g_hwndMain, &rc, FALSE);
            return;
        }
        col++;
        if (col >= MAX_COLS) { col = 0; x = MARGIN_X - g_scrollPosX; y += ICON_SPACING_Y; }
        else { x += cellWidth; }
    }
}

static void DrawApplets(HDC hdc)
{
    int i, x, y, col;
    int iconW = GetSystemMetrics(SM_CXICON);
    int iconH = GetSystemMetrics(SM_CYICON);
    int cellWidth = iconW + 32;
    int textHeight = 16;
    HFONT hFont, hOldFont;
    RECT rcClip;

    hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                       ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Helv");
    hOldFont = SelectObject(hdc, hFont);

    GetClientRect(g_hwndMain, &rcClip);

    x = MARGIN_X - g_scrollPosX;
    y = MARGIN_Y - g_scrollPosY;
    col = 0;
    g_totalHeight = ((g_numApplets + MAX_COLS - 1) / MAX_COLS) * ICON_SPACING_Y + MARGIN_Y;
    g_totalWidth  = MAX_COLS * cellWidth + MARGIN_X;

    for (i = 0; i < g_numApplets; i++) {
        if (y + ICON_SPACING_Y >= rcClip.top && y <= rcClip.bottom) {
            RECT rcCell, rcText;

            /* Фон ячейки */
            rcCell.left   = x;
            rcCell.top    = y;
            rcCell.right  = x + cellWidth;
            rcCell.bottom = y + ICON_SPACING_Y;
            {
                HBRUSH hBr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
                FillRect(hdc, &rcCell, hBr);
                DeleteObject(hBr);
            }

            /* Иконка */
            if (g_applets[i].hIcon) {
                int iconX = x + (cellWidth - iconW) / 2;
                DrawIcon(hdc, iconX, y + 2, g_applets[i].hIcon);
            }

            /* Подпись */
            rcText.left   = x;
            rcText.top    = y + iconH + 4;
            rcText.right  = x + cellWidth;
            rcText.bottom = rcText.top + textHeight;

            if (g_applets[i].bSelected) {
                HBRUSH hBr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                FillRect(hdc, &rcText, hBr);
                DeleteObject(hBr);
                SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
            } else {
                SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
            }

            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, g_applets[i].szName, -1, &rcText,
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        col++;
        if (col >= MAX_COLS) {
            col = 0;
            x = MARGIN_X - g_scrollPosX;
            y += ICON_SPACING_Y;
        } else {
            x += cellWidth;
        }
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

static void ScrollTo(int newPosY, int newPosX)
{
    SetScrollPos(g_hwndMain, SB_VERT, newPosY, TRUE);
    SetScrollPos(g_hwndMain, SB_HORZ, newPosX, TRUE);
    g_scrollPosY = newPosY;
    g_scrollPosX = newPosX;
    InvalidateRect(g_hwndMain, NULL, TRUE);
    UpdateWindow(g_hwndMain);
}

static void LaunchAppletByIndex(HWND hwnd, int appIndex)
{
    if (appIndex >= 0 && appIndex < g_numApplets) {
        g_applets[appIndex].proc(hwnd, CPL_DBLCLK,
                                 (LONG)g_applets[appIndex].index,
                                 g_applets[appIndex].lData);
    }
}

static void BuildSettingsMenu(HWND hwnd)
{
    HMENU hMenu, hSettings;
    hMenu = GetMenu(hwnd);
    if (!hMenu) return;

    /* Первое подменю – Settings */
    hSettings = GetSubMenu(hMenu, 0);
    if (!hSettings) return;

    /* Удаляем старые фиктивные пункты (кроме разделителя и Exit) */
    {
        int count = GetMenuItemCount(hSettings);
        int i;
        for (i = count - 1; i >= 0; i--) {
            UINT id = GetMenuItemID(hSettings, i);
            if (id != 0 && id != IDM_SETTINGS_EXIT) {
                DeleteMenu(hSettings, i, MF_BYPOSITION);
            }
        }
    }

    /* Добавляем апплеты перед разделителем */
    {
        int i;
        for (i = 0; i < g_numApplets; i++) {
            int pos = GetMenuItemCount(hSettings) - 1;
            if (pos < 0) pos = 0;
            InsertMenu(hSettings, pos, MF_BYPOSITION | MF_STRING,
                       IDM_SETTINGS_APPS_BASE + i,
                       g_applets[i].szName);
        }
    }
}

static void OnHelpAbout(HWND hwnd)
{
    HINSTANCE hShell;
    UINT oldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    hShell = LoadLibrary("SHELL.DLL");
    SetErrorMode(oldErrorMode);

    if ((UINT)hShell >= 32) {
        typedef BOOL (WINAPI *SHELLABOUTPROC)(HWND, LPCSTR, LPCSTR, HICON);
        SHELLABOUTPROC lpShellAbout =
            (SHELLABOUTPROC)GetProcAddress(hShell, "ShellAbout");
        if (lpShellAbout) {
            lpShellAbout(hwnd, "Control Panel",
                         "osFree Janus Control Panel",
                         LoadIcon(g_hInst, "MYICON"));
        }
        FreeLibrary(hShell);
        return;
    }

    /* Если SHELL.DLL отсутствует – показываем собственное окно */
    MessageBox(hwnd,
               "Control Panel for Windows 3.0\n\n"
               "Part of osFree Win16 Personality\n"
               "Copyright (C) 2026 osFree Team",
               "About Control Panel", MB_ICONINFORMATION | MB_OK);
}
