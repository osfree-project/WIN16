/*
 * control.c -- Клон Панели управления для Windows 3.0
 * Поддержка вертикальной и горизонтальной прокрутки.
 */

#include <windows.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "cpl.h"
#include "control.h"

#define MAX_APPLETS         256
#define ICON_SPACING_X      75   /* ширина ячейки */
#define ICON_SPACING_Y      60   /* высота ячейки (было 70) */
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
static int       g_scrollPosY = 0;   /* было g_scrollPos */
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
static void LaunchAppletByName(HWND hwnd, LPCSTR name);

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
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
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
        GetClientRect(hwnd, &rc);
        {
            int nMaxV = g_totalHeight - rc.bottom;
            if (nMaxV < 0) nMaxV = 0;
            SetScrollRange(hwnd, SB_VERT, 0, nMaxV, TRUE);
        }
        {
            int nMaxH = g_totalWidth - rc.right;
            if (nMaxH < 0) nMaxH = 0;
            SetScrollRange(hwnd, SB_HORZ, 0, nMaxH, TRUE);
        }
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
        int mx = LOWORD(lParam) + g_scrollPosX;
        int my = HIWORD(lParam) + g_scrollPosY;
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
        int mx = LOWORD(lParam) + g_scrollPosX;
        int my = HIWORD(lParam) + g_scrollPosY;
        int idx = HitTest(mx, my);
        if (idx >= 0) {
            g_applets[idx].proc(hwnd, CPL_DBLCLK,
                                (LONG)g_applets[idx].index,
                                g_applets[idx].lData);
        }
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        case IDM_SETTINGS_COLOR:        LaunchAppletByName(hwnd, "Color");        return 0;
        case IDM_SETTINGS_DESKTOP:      LaunchAppletByName(hwnd, "Desktop");      return 0;
        case IDM_SETTINGS_DATETIME:     LaunchAppletByName(hwnd, "Date/Time");    return 0;
        case IDM_SETTINGS_FONTS:        LaunchAppletByName(hwnd, "Fonts");        return 0;
        case IDM_SETTINGS_INTERNATIONAL:LaunchAppletByName(hwnd, "International");return 0;
        case IDM_SETTINGS_KEYBOARD:     LaunchAppletByName(hwnd, "Keyboard");     return 0;
        case IDM_SETTINGS_MOUSE:        LaunchAppletByName(hwnd, "Mouse");        return 0;
        case IDM_SETTINGS_NETWORK:      LaunchAppletByName(hwnd, "Network");      return 0;
        case IDM_SETTINGS_PORTS:        LaunchAppletByName(hwnd, "Ports");        return 0;
        case IDM_SETTINGS_PRINTERS:     LaunchAppletByName(hwnd, "Printers");     return 0;
        case IDM_SETTINGS_SOUND:        LaunchAppletByName(hwnd, "Sound");        return 0;
        case IDM_SETTINGS_386ENHANCED:  LaunchAppletByName(hwnd, "386 Enhanced"); return 0;
        case IDM_SETTINGS_DRIVERS:      LaunchAppletByName(hwnd, "Drivers");      return 0;
        default:
            break;
        }
        break;

    case WM_DESTROY:
        UnloadAllApplets();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* Безопасное копирование строки */
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
    int cellWidth = iconW + 32;          /* ширина ячейки */
    int textHeight = 16;                 /* высота подписи (подобрано под Helv 12) */
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

    /* общая высота и ширина для скроллбаров */
    g_totalHeight = ((g_numApplets + MAX_COLS - 1) / MAX_COLS) * ICON_SPACING_Y + MARGIN_Y;
    g_totalWidth  = MAX_COLS * cellWidth + MARGIN_X;

    for (i = 0; i < g_numApplets; i++) {
        /* отсечение по вертикали */
        if (y + ICON_SPACING_Y >= rcClip.top && y <= rcClip.bottom) {
            RECT rcCell, rcText;

            /* 1. Фон всей ячейки */
            rcCell.left   = x;
            rcCell.top    = y;
            rcCell.right  = x + cellWidth;
            rcCell.bottom = y + ICON_SPACING_Y;

            {
                HBRUSH hBr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
                FillRect(hdc, &rcCell, hBr);
                DeleteObject(hBr);
            }

            /* 2. Иконка по центру */
            if (g_applets[i].hIcon) {
                int iconX = x + (cellWidth - iconW) / 2;
                DrawIcon(hdc, iconX, y + 2, g_applets[i].hIcon);
            }

            /* 3. Подпись */
            rcText.left   = x;
            rcText.top    = y + iconH + 4;            /* небольшой отступ от иконки */
            rcText.right  = x + cellWidth;
            rcText.bottom = rcText.top + textHeight;  /* фиксированная высота */

            if (g_applets[i].bSelected) {
                /* подсвечиваем ТОЛЬКО фон подписи */
                HBRUSH hBr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                FillRect(hdc, &rcText, hBr);
                DeleteObject(hBr);
                SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
            } else {
                /* обычный текст на цвете окна */
                SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
            }

            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, g_applets[i].szName, -1, &rcText,
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        /* переход к следующей ячейке */
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
    int curPosY = GetScrollPos(g_hwndMain, SB_VERT);
    int curPosX = GetScrollPos(g_hwndMain, SB_HORZ);
    SetScrollPos(g_hwndMain, SB_VERT, newPosY, TRUE);
    SetScrollPos(g_hwndMain, SB_HORZ, newPosX, TRUE);
    g_scrollPosY = newPosY;
    g_scrollPosX = newPosX;
    ScrollWindow(g_hwndMain, curPosX - newPosX, curPosY - newPosY, NULL, NULL);
    InvalidateRect(g_hwndMain, NULL, FALSE);
    UpdateWindow(g_hwndMain);
}

static void LaunchAppletByName(HWND hwnd, LPCSTR name)
{
    int i;
    for (i = 0; i < g_numApplets; i++) {
        if (lstrcmpi(g_applets[i].szName, name) == 0) {
            g_applets[i].proc(hwnd, CPL_DBLCLK,
                              (LONG)g_applets[i].index,
                              g_applets[i].lData);
            return;
        }
    }
    {
        char buf[150];
        wsprintf(buf, "The '%s' Control Panel applet is not installed.", name);
        MessageBox(hwnd, buf, "Control Panel", MB_ICONEXCLAMATION | MB_OK);
    }
}
