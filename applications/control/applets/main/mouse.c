#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"


/* ============================================================
   Mouse applet dialog procedure (Windows 3.0 compatible)
   ============================================================ */

/* Таблица предустановок Mouse Tracking Speed (7 положений ползунка) */
typedef struct {
    int speed;
    int threshold1;
    int threshold2;
} MOUSETRACKPRESET;

static const MOUSETRACKPRESET g_presets[] = {
    {0,  0,  0},   /* позиция 1 */
    {1, 10,  0},   /* позиция 2 */
    {1,  7,  0},   /* позиция 3 */
    {1,  4,  0},   /* позиция 4 */
    {2,  4, 12},   /* позиция 5 */
    {2,  4,  8},   /* позиция 6 */
    {2,  4,  4}    /* позиция 7 */
};

#define NUM_PRESETS  7

static int   g_mouseTrackPos = 4;     /* текущая позиция ползунка скорости (1..7) */
static int   g_dblClickPos = 50;      /* текущая позиция ползунка двойного щелчка (1..100) */
static int   g_dblClickTime = 500;
static BOOL  g_swapButtons = FALSE;
static int   g_origTrackPos;          /* исходная позиция скорости */
static int   g_origDblClick;
static BOOL  g_origSwap;
static HWND  hLStatic;
static HWND  hRStatic;
static HWND  hTestStatic;
static HWND  hSpeedScroll = NULL;
static HWND  hDblClickScroll = NULL;
static BOOL  g_bLeftDown;
static BOOL  g_bRightDown;
static BOOL  g_bTestInverted = FALSE;
static BOOL  g_pressedSwap;
static char  g_szOrigL[16], g_szOrigR[16];

/* Применить тройку параметров для заданной позиции ползунка скорости */
static void NEAR ApplyMouseTrackPos(int pos)
{
    int mouseParams[3];
    if (pos < 1) pos = 1;
    if (pos > NUM_PRESETS) pos = NUM_PRESETS;

    mouseParams[0] = g_presets[pos - 1].threshold1;
    mouseParams[1] = g_presets[pos - 1].threshold2;
    mouseParams[2] = g_presets[pos - 1].speed;

//    SystemParametersInfo(SPI_SETMOUSE, 0, (LPSTR)mouseParams, SPIF_UPDATEINIFILE);
}

/* Определить позицию ползунка (1..7) по тройке параметров.
   Возвращает 0, если точного соответствия нет. */
static int NEAR FindTrackPos(int speed, int thresh1, int thresh2)
{
    int i;
    for (i = 0; i < NUM_PRESETS; i++) {
        if (g_presets[i].speed == speed &&
            g_presets[i].threshold1 == thresh1 &&
            g_presets[i].threshold2 == thresh2)
            return i + 1;
    }
    return 0;
}

/* Invert the client area of a static control (with safety check) */
static void NEAR InvertStatic(HWND hWnd)
{
    HDC hdc;
    RECT rc;
    if (!hWnd || !IsWindow(hWnd))
        return;
    hdc = GetDC(hWnd);
    if (hdc) {
        GetClientRect(hWnd, &rc);
        InvertRect(hdc, &rc);
        ReleaseDC(hWnd, hdc);
    }
}

/* Swap the labels L and R according to current swap state */
static void NEAR UpdateButtonLabels(HWND hDlg, BOOL swapped)
{
    if (swapped) {
        SetDlgItemText(hDlg, IDC_MS_L, g_szOrigR);
        SetDlgItemText(hDlg, IDC_MS_R, g_szOrigL);
    } else {
        SetDlgItemText(hDlg, IDC_MS_L, g_szOrigL);
        SetDlgItemText(hDlg, IDC_MS_R, g_szOrigR);
    }
}

BOOL CALLBACK MouseDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buf[32];
    int pos, curPos, code;
    HWND hScroll;
    int id;
    BOOL swap;
    POINT pt;
    RECT rc;

    switch (msg) {
    case WM_INITDIALOG:
    {
        int speed, thresh1, thresh2;

        /* Читаем текущие параметры мыши из WIN.INI */
        speed   = GetProfileInt("windows", "MouseSpeed", 1);
        thresh1 = GetProfileInt("windows", "MouseThreshold1", 5);
        thresh2 = GetProfileInt("windows", "MouseThreshold2", 10);

        /* Пытаемся найти точное совпадение с одной из предустановок */
        g_mouseTrackPos = FindTrackPos(speed, thresh1, thresh2);
        if (g_mouseTrackPos == 0) {
            g_mouseTrackPos = 4;
        }
        g_origTrackPos = g_mouseTrackPos;

        hSpeedScroll = GetDlgItem(hDlg, IDC_MS_SPEED);
        SetScrollRange(hSpeedScroll, SB_CTL, 1, NUM_PRESETS, TRUE);
        SetScrollPos(hSpeedScroll, SB_CTL, g_mouseTrackPos, TRUE);

        /* Double-click speed: читаем из WIN.INI (100..900 мс) */
        g_dblClickTime = GetProfileInt("windows", "DoubleClickSpeed", 500);
        if (g_dblClickTime < 100) g_dblClickTime = 100;
        if (g_dblClickTime > 900) g_dblClickTime = 900;
        g_origDblClick = g_dblClickTime;

        /* Позиция бегунка: 1 -> 900 мс, 100 -> 100 мс */
        g_dblClickPos = 100 - (g_dblClickTime - 100) * 99 / 800;
        if (g_dblClickPos < 1) g_dblClickPos = 1;
        if (g_dblClickPos > 100) g_dblClickPos = 100;

        hDblClickScroll = GetDlgItem(hDlg, IDC_MS_DOUBLECLICK);
        SetScrollRange(hDblClickScroll, SB_CTL, 1, 100, TRUE);
        SetScrollPos(hDblClickScroll, SB_CTL, g_dblClickPos, TRUE);

        /* Swap buttons */
        /* Store original L/R texts from the dialog resources */
        GetDlgItemText(hDlg, IDC_MS_L, g_szOrigL, sizeof(g_szOrigL));
        GetDlgItemText(hDlg, IDC_MS_R, g_szOrigR, sizeof(g_szOrigR));

        /* Get handles for static frames (clickable squares) */
        hLStatic = GetDlgItem(hDlg, IDC_MS_L_FRAME);
        hRStatic = GetDlgItem(hDlg, IDC_MS_R_FRAME);
        hTestStatic = GetDlgItem(hDlg, IDC_MS_TEST);

	/* Here need to workaround bug 
		PRWIN9103027: System Metrics Not Updated by SwapMouseButton 
		(https://library.thedatadungeon.com/msdn-1992-09/kbase/html/kbas2q3l.content.htm)
		Actually, GetSystemMetrics(SM_SWAPBUTTON) returns value from WIN.INI on windows startup, not current.
		So, we try to read from win.ini first, if no succes, then use value from metric (default value);
	*/
	if (GetProfileString("windows", "SwapMouseButtons", "No", buf, sizeof(buf)) != 0)
            g_swapButtons = (lstrcmpi(buf, "Yes") == 0);
        else
            g_swapButtons = GetSystemMetrics(SM_SWAPBUTTON);

        g_origSwap = g_swapButtons;
        CheckDlgButton(hDlg, IDC_MS_SWAP, g_swapButtons);
        UpdateButtonLabels(hDlg, g_swapButtons);

        g_bLeftDown = FALSE;
        g_bRightDown = FALSE;
        g_bTestInverted = FALSE;
        g_pressedSwap = FALSE;
        return TRUE;
    }

case WM_HSCROLL:
{
    code = wParam;                     /* уведомление */
    pos  = LOWORD(lParam);             /* позиция (только для THUMB) */
    hScroll = (HWND)HIWORD(lParam);    /* дескриптор окна скроллбара */

    if (!hScroll) return TRUE;

    if (hScroll == hSpeedScroll) {
        switch (code) {
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            g_mouseTrackPos = pos;
            break;
        case SB_LINELEFT:
            if (g_mouseTrackPos > 1) g_mouseTrackPos--;
            break;
        case SB_LINERIGHT:
            if (g_mouseTrackPos < NUM_PRESETS) g_mouseTrackPos++;
            break;
        case SB_PAGELEFT:
            if (g_mouseTrackPos > 1) {
                g_mouseTrackPos--;
                if (g_mouseTrackPos < 1) g_mouseTrackPos = 1;
            }
            break;
        case SB_PAGERIGHT:
            if (g_mouseTrackPos < NUM_PRESETS) {
                g_mouseTrackPos++;
                if (g_mouseTrackPos > NUM_PRESETS) g_mouseTrackPos = NUM_PRESETS;
            }
            break;
        default:
            return TRUE;
        }
        SetScrollPos(hSpeedScroll, SB_CTL, g_mouseTrackPos, TRUE);
        ApplyMouseTrackPos(g_mouseTrackPos);
    }
    else if (hScroll == hDblClickScroll) {
        switch (code) {
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            g_dblClickPos = pos;
            break;
        case SB_LINELEFT:
            if (g_dblClickPos > 1) g_dblClickPos--;
            break;
        case SB_LINERIGHT:
            if (g_dblClickPos < 100) g_dblClickPos++;
            break;
        case SB_PAGELEFT:
            if (g_dblClickPos > 1) {
                g_dblClickPos -= 10;
                if (g_dblClickPos < 1) g_dblClickPos = 1;
            }
            break;
        case SB_PAGERIGHT:
            if (g_dblClickPos < 100) {
                g_dblClickPos += 10;
                if (g_dblClickPos > 100) g_dblClickPos = 100;
            }
            break;
        default:
            return TRUE;
        }
        g_dblClickTime = 100 + (100 - g_dblClickPos) * 800 / 99;
        SetDoubleClickTime(g_dblClickTime);
        SetScrollPos(hDblClickScroll, SB_CTL, g_dblClickPos, TRUE);
    }
    return TRUE;
}

    case WM_LBUTTONDOWN:
        if (g_bLeftDown) return TRUE;
        swap = IsDlgButtonChecked(hDlg, IDC_MS_SWAP);
        g_bLeftDown = TRUE;
        g_pressedSwap = swap;
        InvertStatic(swap ? hRStatic : hLStatic);
        return TRUE;

    case WM_LBUTTONUP:
        if (!g_bLeftDown) return TRUE;
        g_bLeftDown = FALSE;
        InvertStatic(g_pressedSwap ? hRStatic : hLStatic);
        return TRUE;

    case WM_RBUTTONDOWN:
        if (g_bRightDown) return TRUE;
        swap = IsDlgButtonChecked(hDlg, IDC_MS_SWAP);
        g_bRightDown = TRUE;
        g_pressedSwap = swap;
        InvertStatic(swap ? hLStatic : hRStatic);
        return TRUE;

    case WM_RBUTTONUP:
        if (!g_bRightDown) return TRUE;
        g_bRightDown = FALSE;
        InvertStatic(g_pressedSwap ? hLStatic : hRStatic);
        return TRUE;

    case WM_LBUTTONDBLCLK:
        if (!hTestStatic) return TRUE;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        GetWindowRect(hTestStatic, &rc);
        ScreenToClient(hDlg, (LPPOINT)&rc.left);
        ScreenToClient(hDlg, (LPPOINT)&rc.right);
        if (PtInRect(&rc, pt)) {
            InvertStatic(hTestStatic);
            g_bTestInverted = !g_bTestInverted;
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hDlg, IDOK);
            return TRUE;

        case IDCANCEL:
            	SetDoubleClickTime(g_origDblClick);
		/* Windows 3.1 bug workaround
			Q110662: BUG: SystemParametersInfo() Does Not Modify WIN.INI
			(https://jeffpar.github.io/kbarchive/kb/110/Q110662/)
			So we write ourself.
		*/
            	SwapMouseButton(g_origSwap);
		WriteProfileString ("Windows", "SwapMouseButtons", g_origSwap?"Yes":"No");
            //@todo trap ApplyMouseSpeed(g_origSpeed);
            EndDialog(hDlg, IDCANCEL);
            return TRUE;

        case IDC_MS_SWAP:
            if (HIWORD(wParam) == BN_CLICKED) {
                swap = IsDlgButtonChecked(hDlg, IDC_MS_SWAP);
                UpdateButtonLabels(hDlg, swap);
		/* Windows 3.1 bug workaround
			Q110662: BUG: SystemParametersInfo() Does Not Modify WIN.INI
			(https://jeffpar.github.io/kbarchive/kb/110/Q110662/)
			So we write ourself.
		*/
                SwapMouseButton(swap);
                WriteProfileString("Windows", "SwapMouseButtons", swap ? "Yes" : "No");
                g_swapButtons = swap;
                /* Reset any pending press state to avoid stuck inverted squares */
                if (g_bLeftDown) {
                    InvertStatic(g_pressedSwap ? hRStatic : hLStatic);
                    g_bLeftDown = FALSE;
                }
                if (g_bRightDown) {
                    InvertStatic(g_pressedSwap ? hLStatic : hRStatic);
                    g_bRightDown = FALSE;
                }
                g_pressedSwap = FALSE;
            }
            return TRUE;
        }
        break;
    }
    return FALSE;
}
