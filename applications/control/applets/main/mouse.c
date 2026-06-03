#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"


/* ============================================================
   Mouse applet dialog procedure (Windows 3.0 compatible)
   ============================================================ */

static int   g_mouseSpeed = 2;
static int   g_dblClickTime = 500;
static BOOL  g_swapButtons = FALSE;
static int   g_origSpeed;
static int   g_origDblClick;
static BOOL  g_origSwap;
static HWND  hLStatic;
static HWND  hRStatic;
static HWND  hTestStatic;
static BOOL  g_bLeftDown;
static BOOL  g_bRightDown;
static BOOL  g_bTestInverted = FALSE;
static BOOL  g_pressedSwap;          /* swap state at press time */
static char  g_szOrigL[16], g_szOrigR[16];   /* original L/R texts from dialog */

/* Apply mouse speed using SystemParametersInfo (Win3.0) */
static void NEAR ApplyMouseSpeed(int speed)
{
    int mouseParams[3];               /* [0]=threshold1, [1]=threshold2, [2]=speed */
    SystemParametersInfo(SPI_GETMOUSE, 0, (LPSTR)mouseParams, 0);
    mouseParams[2] = speed;
    SystemParametersInfo(SPI_SETMOUSE, 0, (LPSTR)mouseParams, SPIF_UPDATEINIFILE);
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
        /* Read mouse speed from WIN.INI */
        GetProfileString("windows", "MouseSpeed", "2", buf, sizeof(buf));
        g_mouseSpeed = atoi(buf);
        if (g_mouseSpeed < 1) g_mouseSpeed = 1;
        if (g_mouseSpeed > 3) g_mouseSpeed = 3;
        g_origSpeed = g_mouseSpeed;

        hScroll = GetDlgItem(hDlg, IDC_MS_SPEED);
        SetScrollRange(hScroll, SB_CTL, 1, 3, FALSE);
        SetScrollPos(hScroll, SB_CTL, g_mouseSpeed, TRUE);

        /* Double-click speed */
        g_dblClickTime = GetDoubleClickTime();
        g_origDblClick = g_dblClickTime;

        pos = 100 - (g_dblClickTime / 10);
        if (pos < 1) pos = 1;
        if (pos > 100) pos = 100;
        hScroll = GetDlgItem(hDlg, IDC_MS_DOUBLECLICK);
        SetScrollRange(hScroll, SB_CTL, 1, 100, FALSE);
        SetScrollPos(hScroll, SB_CTL, pos, TRUE);

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

    case WM_HSCROLL:
        hScroll = (HWND)lParam;
        id = GetDlgCtrlID(hScroll);
        code = LOWORD(wParam);
        pos = HIWORD(wParam);

        if (id == IDC_MS_SPEED) {
            curPos = GetScrollPos(hScroll, SB_CTL);
            switch (code) {
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                g_mouseSpeed = pos;
                break;
            case SB_LINEUP:
                if (g_mouseSpeed > 1) g_mouseSpeed--;
                break;
            case SB_LINEDOWN:
                if (g_mouseSpeed < 3) g_mouseSpeed++;
                break;
            default:
                return TRUE;
            }
            SetScrollPos(hScroll, SB_CTL, g_mouseSpeed, TRUE);
            ApplyMouseSpeed(g_mouseSpeed);
        } else if (id == IDC_MS_DOUBLECLICK) {
            curPos = GetScrollPos(hScroll, SB_CTL);
            switch (code) {
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                curPos = pos;
                g_dblClickTime = (100 - curPos) * 10;
                if (g_dblClickTime < 100) g_dblClickTime = 100;
                if (g_dblClickTime > 1000) g_dblClickTime = 1000;
                SetDoubleClickTime(g_dblClickTime);
                break;
            case SB_LINEUP:
                if (curPos < 100) {
                    curPos++;
                    g_dblClickTime = (100 - curPos) * 10;
                    SetDoubleClickTime(g_dblClickTime);
                }
                break;
            case SB_LINEDOWN:
                if (curPos > 1) {
                    curPos--;
                    g_dblClickTime = (100 - curPos) * 10;
                    SetDoubleClickTime(g_dblClickTime);
                }
                break;
            default:
                return TRUE;
            }
            SetScrollPos(hScroll, SB_CTL, curPos, TRUE);
        }
        return TRUE;

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
		WriteProfileString ("Windows", "SwapMouseButtons", swap?"Yes":"No");
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
