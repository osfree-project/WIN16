/*
 *  time.c – Date & Time dialog for Windows 3.0
 *  Live clock display, ownerdrawn spin buttons.
 *  Year is displayed and edited as 2 digits (80–99 = 1980–1999, 00–79 = 2000–2079).
 *  Supports 12-hour time format with automatic AM/PM label when system is set to 12-hour mode.
 */

#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

/* Spin button IDs */
#define IDC_DT_DATE_UP       720
#define IDC_DT_DATE_DOWN     721
#define IDC_DT_TIME_UP       722
#define IDC_DT_TIME_DOWN     723

/* Timer ID for live clock */
#define IDT_CLOCK_TIMER      1

/* Global state */
static WORD g_dtYear, g_dtMonth, g_dtDay;
static WORD g_dtHour, g_dtMinute, g_dtSecond;
static WORD g_origYear, g_origMonth, g_origDay;
static WORD g_origHour, g_origMinute, g_origSecond;

/* Flags to prevent timer from overwriting manual edits */
static BOOL g_bDateModified = FALSE;
static BOOL g_bTimeModified = FALSE;

/* Last focused control for date and time (survives focus loss) */
static WORD g_lastDateCtrl = IDC_DT_MONTH;
static WORD g_lastTimeCtrl = IDC_DT_HOUR;

/* ---------- 12-hour format support ---------- */
static BOOL g_bUse12Hour = FALSE;          /* TRUE if system uses 12-hour time */
static HWND g_hwndAmPmLabel = NULL;        /* handle of the AM/PM static text */

/* ------------------------------------------------------------------ */
/* DOS API for system time */
static void GetLocalDateTime(WORD *y, WORD *m, WORD *d,
                             WORD *h, WORD *min, WORD *s)
{
    static union REGS regs;
    regs.h.ah = 0x2A; intdos(&regs, &regs);
    *y = regs.x.cx; *m = regs.h.dh; *d = regs.h.dl;
    regs.h.ah = 0x2C; intdos(&regs, &regs);
    *h = regs.h.ch; *min = regs.h.cl; *s = regs.h.dh;
}

static void SetLocalDateTime(WORD y, WORD m, WORD d,
                             WORD h, WORD min, WORD s)
{
    static union REGS regs;
    regs.h.ah = 0x2B; regs.x.cx = y; regs.h.dh = m; regs.h.dl = d;
    intdos(&regs, &regs);
    regs.h.ah = 0x2D; regs.h.ch = h; regs.h.cl = min; regs.h.dh = s;
    intdos(&regs, &regs);
}

/* ------------------------------------------------------------------ */
/* Date validation */
static BOOL IsLeapYear(WORD year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static WORD MaxDaysInMonth(WORD month, WORD year)
{
    static const WORD days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && IsLeapYear(year)) return 29;
    return days[month-1];
}

/* ------------------------------------------------------------------ */
/* Year helpers – convert between 2-digit display and full year */
static WORD YearToDisplay(WORD fullYear)
{
    return fullYear % 100;               /* 1980 -> 80, 2026 -> 26 */
}

static WORD YearFromDisplay(WORD displayYear)
{
    if (displayYear >= 80)               /* 80-99 -> 1980-1999 */
        return displayYear + 1900;
    else                                 /* 0-79 -> 2000-2079 */
        return displayYear + 2000;
}

/* ------------------------------------------------------------------ */
/* Spin helpers */
static void SpinDateField(HWND hDlg, WORD ctrl, BOOL up)
{
    WORD *pVal;
    WORD min, max;

    if (ctrl == IDC_DT_MONTH)       { pVal = &g_dtMonth; min = 1; max = 12; }
    else if (ctrl == IDC_DT_DAY)    { pVal = &g_dtDay;   min = 1; max = MaxDaysInMonth(g_dtMonth, g_dtYear); }
    else if (ctrl == IDC_DT_YEAR)   { pVal = &g_dtYear;  min = 1980; max = 2099; }
    else return;

    if (up) { if (*pVal < max) (*pVal)++; else *pVal = min; }
    else    { if (*pVal > min) (*pVal)--; else *pVal = max; }

    if (pVal == &g_dtMonth || pVal == &g_dtYear) {
        if (g_dtDay > MaxDaysInMonth(g_dtMonth, g_dtYear))
            g_dtDay = MaxDaysInMonth(g_dtMonth, g_dtYear);
    }
}

static void SpinTimeField(HWND hDlg, WORD ctrl, BOOL up)
{
    WORD *pVal;
    WORD min, max;

    if (ctrl == IDC_DT_HOUR)        { pVal = &g_dtHour;   min = 0; max = 23; }
    else if (ctrl == IDC_DT_MINUTE) { pVal = &g_dtMinute; min = 0; max = 59; }
    else if (ctrl == IDC_DT_SECOND) { pVal = &g_dtSecond; min = 0; max = 59; }
    else return;

    if (up) { if (*pVal < max) (*pVal)++; else *pVal = min; }
    else    { if (*pVal > min) (*pVal)--; else *pVal = max; }
}

/* ------------------------------------------------------------------ */
/* AM/PM label update */
static void UpdateAmPmLabel(void)
{
    if (g_bUse12Hour && g_hwndAmPmLabel) {
        SetWindowText(g_hwndAmPmLabel, (g_dtHour >= 12) ? "PM" : "AM");
    }
}

/* Update a single edit field – special treatment for year (2 digits)
   and for hour in 12-hour mode. */
static void UpdateField(HWND hDlg, WORD id, WORD value, int digits)
{
    char buf[8];
    if (id == IDC_DT_YEAR) {
        wsprintf(buf, "%02u", YearToDisplay(value));
    } else if (id == IDC_DT_HOUR && g_bUse12Hour) {
        /* 24h -> 12h display */
        WORD h12 = value % 12;
        if (h12 == 0) h12 = 12;
        wsprintf(buf, "%2u", h12);   /* right-aligned in a 2-char field */
    } else {
        if (digits == 2) wsprintf(buf, "%02u", value);
        else wsprintf(buf, "%u", value);
    }
    SetDlgItemText(hDlg, id, buf);
}

/* ------------------------------------------------------------------ */
/* Owner-draw painting for the flat spin buttons (up/down triangles).  */
static void DrawSpinTriangle(HDC hdc, LPRECT prc, BOOL up)
{
    POINT pts[3];
    int cx = (prc->right - prc->left) / 2;
    int cy = (prc->bottom - prc->top) / 2;
    int midx = prc->left + cx;
    HBRUSH hBr, hOldBr;
    HPEN hPen, hOldPen;

    if (up) {
        pts[0].x = midx;       pts[0].y = prc->top    + cy/2;
        pts[1].x = midx - cx/2; pts[1].y = prc->bottom - cy/2;
        pts[2].x = midx + cx/2; pts[2].y = prc->bottom - cy/2;
    } else {
        pts[0].x = midx;       pts[0].y = prc->bottom - cy/2;
        pts[1].x = midx - cx/2; pts[1].y = prc->top    + cy/2;
        pts[2].x = midx + cx/2; pts[2].y = prc->top    + cy/2;
    }

    hPen   = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
    hBr    = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
    hOldPen   = SelectObject(hdc, hPen);
    hOldBr    = SelectObject(hdc, hBr);

    Polygon(hdc, pts, 3);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBr);
    DeleteObject(hPen);
    DeleteObject(hBr);
}

static void PaintSpinButton(HWND hwndBtn, HDC hdc, LPRECT prc)
{
    WORD id = GetWindowWord(hwndBtn, GWW_ID);
    HBRUSH hBr;

    hBr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hdc, prc, hBr);
    DeleteObject(hBr);

    DrawSpinTriangle(hdc, prc,
        (id == IDC_DT_DATE_UP || id == IDC_DT_TIME_UP) ? TRUE : FALSE);
}

/* ================================================================== */
BOOL WINAPI DateTimeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buf[8];
    BOOL ok;

    switch (msg) {
    case WM_INITDIALOG:
    {
        static WORD y, m, d, h, min, s;
        HFONT hDlgFont;
        HWND hChild;
        RECT rcGroup;
        int xRight, yTop, yBottom;

        GetLocalDateTime(&y, &m, &d, &h, &min, &s);
        g_dtYear = y; g_dtMonth = m; g_dtDay = d;
        g_dtHour = h; g_dtMinute = min; g_dtSecond = s;
        _fmemcpy(&g_origYear, &g_dtYear, sizeof(WORD)*6);

        g_bDateModified = FALSE;
        g_bTimeModified = FALSE;
        g_lastDateCtrl = IDC_DT_MONTH;
        g_lastTimeCtrl = IDC_DT_HOUR;

        /* Detect 12-hour vs 24-hour time format from WIN.INI */
        g_bUse12Hour = (GetProfileInt("intl", "iTime", 0) == 0);

        /* Year field – only 2 digits */
        SendDlgItemMessage(hDlg, IDC_DT_MONTH,  EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_DAY,    EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_YEAR,   EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_HOUR,   EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_MINUTE, EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_SECOND, EM_LIMITTEXT, 2, 0);

        UpdateField(hDlg, IDC_DT_MONTH,  g_dtMonth, 2);
        UpdateField(hDlg, IDC_DT_DAY,    g_dtDay,   2);
        UpdateField(hDlg, IDC_DT_YEAR,   g_dtYear,  4);   /* digits ignored, prints 2 digits */
        UpdateField(hDlg, IDC_DT_HOUR,   g_dtHour,  2);
        UpdateField(hDlg, IDC_DT_MINUTE, g_dtMinute,2);
        UpdateField(hDlg, IDC_DT_SECOND, g_dtSecond,2);

        SetWindowLong(GetDlgItem(hDlg, IDC_DT_DATE_GROUP), GWL_STYLE,
                      GetWindowLong(GetDlgItem(hDlg, IDC_DT_DATE_GROUP), GWL_STYLE) | WS_CLIPSIBLINGS);
        SetWindowLong(GetDlgItem(hDlg, IDC_DT_TIME_GROUP), GWL_STYLE,
                      GetWindowLong(GetDlgItem(hDlg, IDC_DT_TIME_GROUP), GWL_STYLE) | WS_CLIPSIBLINGS);

        hDlgFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);

        /* ----- Date spin buttons (inside the Date group, forced on top) ----- */
        GetWindowRect(GetDlgItem(hDlg, IDC_DT_DATE_GROUP), &rcGroup);
        ScreenToClient(hDlg, (LPPOINT)&rcGroup.left);
        ScreenToClient(hDlg, (LPPOINT)&rcGroup.right);
        xRight  = rcGroup.right - 16;
        yTop    = rcGroup.top + 12;
        yBottom = rcGroup.top + 24;

        hChild = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_CLIPSIBLINGS,
                              xRight, yTop, 14, 10, hDlg, (HMENU)IDC_DT_DATE_UP, g_hInst, NULL);
        if (hDlgFont) SendMessage(hChild, WM_SETFONT, (WPARAM)hDlgFont, 0);
        SetWindowPos(hChild, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

        hChild = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_CLIPSIBLINGS,
                              xRight, yBottom, 14, 10, hDlg, (HMENU)IDC_DT_DATE_DOWN, g_hInst, NULL);
        if (hDlgFont) SendMessage(hChild, WM_SETFONT, (WPARAM)hDlgFont, 0);
        SetWindowPos(hChild, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

        /* ----- Time spin buttons (inside the Time group, forced on top) ----- */
        GetWindowRect(GetDlgItem(hDlg, IDC_DT_TIME_GROUP), &rcGroup);
        ScreenToClient(hDlg, (LPPOINT)&rcGroup.left);
        ScreenToClient(hDlg, (LPPOINT)&rcGroup.right);
        xRight  = rcGroup.right - 16;
        yTop    = rcGroup.top + 12;
        yBottom = rcGroup.top + 24;

        hChild = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_CLIPSIBLINGS,
                              xRight, yTop, 14, 10, hDlg, (HMENU)IDC_DT_TIME_UP, g_hInst, NULL);
        if (hDlgFont) SendMessage(hChild, WM_SETFONT, (WPARAM)hDlgFont, 0);
        SetWindowPos(hChild, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

        hChild = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_CLIPSIBLINGS,
                              xRight, yBottom, 14, 10, hDlg, (HMENU)IDC_DT_TIME_DOWN, g_hInst, NULL);
        if (hDlgFont) SendMessage(hChild, WM_SETFONT, (WPARAM)hDlgFont, 0);
        SetWindowPos(hChild, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

        /* ----- AM/PM static label (only for 12-hour mode) ----- */
        if (g_bUse12Hour) {
            int xLabel = rcGroup.right - 32;   /* between seconds edit and spin buttons */
            int yLabel = rcGroup.top + 18;     /* vertically aligned with edit fields */
            g_hwndAmPmLabel = CreateWindow(
                "STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                xLabel, yLabel, 16, 10,
                hDlg, (HMENU)IDC_DT_AMPM_LABEL, g_hInst, NULL);
            if (hDlgFont) SendMessage(g_hwndAmPmLabel, WM_SETFONT, (WPARAM)hDlgFont, 0);
            SetWindowPos(g_hwndAmPmLabel, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
            UpdateAmPmLabel();
        }

        SetTimer(hDlg, IDT_CLOCK_TIMER, 1000, NULL);
        return TRUE;
    }

    case WM_TIMER:
    {
        if (wParam == IDT_CLOCK_TIMER) {
            if (!g_bTimeModified) {
                static WORD y, m, d, h, min, s;
                GetLocalDateTime(&y, &m, &d, &h, &min, &s);
                if (h != g_dtHour || min != g_dtMinute || s != g_dtSecond) {
                    g_dtHour   = h;
                    g_dtMinute = min;
                    g_dtSecond = s;
                    UpdateField(hDlg, IDC_DT_HOUR,   h,   2);
                    UpdateField(hDlg, IDC_DT_MINUTE, min, 2);
                    UpdateField(hDlg, IDC_DT_SECOND, s,   2);
                    UpdateAmPmLabel();
                }
                if (!g_bDateModified &&
                    (y != g_dtYear || m != g_dtMonth || d != g_dtDay)) {
                    g_dtYear  = y;
                    g_dtMonth = m;
                    g_dtDay   = d;
                    UpdateField(hDlg, IDC_DT_MONTH, m, 2);
                    UpdateField(hDlg, IDC_DT_DAY,   d, 2);
                    UpdateField(hDlg, IDC_DT_YEAR,  y, 4);
                }
            } else {
                /* Tick from manually edited time */
                g_dtSecond++;
                if (g_dtSecond > 59) {
                    g_dtSecond = 0;
                    g_dtMinute++;
                    if (g_dtMinute > 59) {
                        g_dtMinute = 0;
                        g_dtHour++;
                        if (g_dtHour > 23) {
                            g_dtHour = 0;
                            if (!g_bDateModified) {
                                g_dtDay++;
                                if (g_dtDay > MaxDaysInMonth(g_dtMonth, g_dtYear)) {
                                    g_dtDay = 1;
                                    g_dtMonth++;
                                    if (g_dtMonth > 12) {
                                        g_dtMonth = 1;
                                        g_dtYear++;
                                    }
                                }
                                UpdateField(hDlg, IDC_DT_MONTH, g_dtMonth, 2);
                                UpdateField(hDlg, IDC_DT_DAY,   g_dtDay,   2);
                                UpdateField(hDlg, IDC_DT_YEAR,  g_dtYear,  4);
                            }
                        }
                        UpdateAmPmLabel();   /* hour might have crossed AM/PM boundary */
                    }
                }
                UpdateField(hDlg, IDC_DT_HOUR,   g_dtHour,   2);
                UpdateField(hDlg, IDC_DT_MINUTE, g_dtMinute, 2);
                UpdateField(hDlg, IDC_DT_SECOND, g_dtSecond, 2);
            }
        }
        return TRUE;
    }

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
        if (lpDIS->CtlType == ODT_BUTTON) {
            PaintSpinButton(lpDIS->hwndItem, lpDIS->hDC, &lpDIS->rcItem);
            return TRUE;
        }
        break;
    }

    case WM_COMMAND:
    {
        WORD code = HIWORD(lParam);

        if (wParam == IDC_DT_DATE_UP || wParam == IDC_DT_DATE_DOWN) {
            g_bDateModified = TRUE;
            SpinDateField(hDlg, g_lastDateCtrl, (wParam == IDC_DT_DATE_UP));
            UpdateField(hDlg, g_lastDateCtrl,
                        g_lastDateCtrl == IDC_DT_MONTH ? g_dtMonth :
                        g_lastDateCtrl == IDC_DT_DAY   ? g_dtDay   : g_dtYear,
                        g_lastDateCtrl == IDC_DT_YEAR ? 4 : 2);
            return TRUE;
        }

        if (wParam == IDC_DT_TIME_UP || wParam == IDC_DT_TIME_DOWN) {
            g_bTimeModified = TRUE;
            SpinTimeField(hDlg, g_lastTimeCtrl, (wParam == IDC_DT_TIME_UP));
            UpdateField(hDlg, g_lastTimeCtrl,
                        g_lastTimeCtrl == IDC_DT_HOUR   ? g_dtHour   :
                        g_lastTimeCtrl == IDC_DT_MINUTE ? g_dtMinute : g_dtSecond,
                        2);
            if (g_lastTimeCtrl == IDC_DT_HOUR) {
                UpdateAmPmLabel();
            }
            return TRUE;
        }

        if (code == EN_SETFOCUS) {
            if (wParam >= IDC_DT_MONTH && wParam <= IDC_DT_YEAR)
                g_lastDateCtrl = wParam;
            else if (wParam >= IDC_DT_HOUR && wParam <= IDC_DT_SECOND)
                g_lastTimeCtrl = wParam;
            return TRUE;
        }

        if (code == EN_CHANGE) {
            if (wParam >= IDC_DT_MONTH && wParam <= IDC_DT_YEAR)
                g_bDateModified = TRUE;
            else if (wParam >= IDC_DT_HOUR && wParam <= IDC_DT_SECOND)
                g_bTimeModified = TRUE;
            return TRUE;
        }

        if (wParam == IDCANCEL) {
            KillTimer(hDlg, IDT_CLOCK_TIMER);
            SetLocalDateTime(g_origYear, g_origMonth, g_origDay,
                             g_origHour, g_origMinute, g_origSecond);
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }

        if (wParam == IDOK) {
            GetDlgItemText(hDlg, IDC_DT_MONTH, buf, sizeof(buf));
            g_dtMonth = (WORD)atoi(buf);
            GetDlgItemText(hDlg, IDC_DT_DAY, buf, sizeof(buf));
            g_dtDay = (WORD)atoi(buf);
            GetDlgItemText(hDlg, IDC_DT_YEAR, buf, sizeof(buf));
            g_dtYear = YearFromDisplay((WORD)atoi(buf));

            GetDlgItemText(hDlg, IDC_DT_HOUR, buf, sizeof(buf));
            if (g_bUse12Hour) {
                WORD h12 = (WORD)atoi(buf);
                if (h12 < 1 || h12 > 12) {
                    ok = FALSE;
                } else {
                    /* Determine AM/PM from the current 24-hour hour */
                    BOOL isPM = (g_dtHour >= 12);
                    if (h12 == 12)
                        g_dtHour = isPM ? 12 : 0;
                    else
                        g_dtHour = h12 + (isPM ? 12 : 0);
                }
            } else {
                g_dtHour = (WORD)atoi(buf);
            }

            GetDlgItemText(hDlg, IDC_DT_MINUTE, buf, sizeof(buf));
            g_dtMinute = (WORD)atoi(buf);
            GetDlgItemText(hDlg, IDC_DT_SECOND, buf, sizeof(buf));
            g_dtSecond = (WORD)atoi(buf);

            ok = TRUE;
            if (g_dtMonth < 1 || g_dtMonth > 12) ok = FALSE;
            if (g_dtDay < 1 || g_dtDay > MaxDaysInMonth(g_dtMonth, g_dtYear)) ok = FALSE;
            if (g_dtYear < 1980 || g_dtYear > 2099) ok = FALSE;
            if (g_dtHour > 23) ok = FALSE;
            if (g_dtMinute > 59) ok = FALSE;
            if (g_dtSecond > 59) ok = FALSE;

            if (!ok) {
                MessageBox(hDlg, "Invalid date or time. Please re-enter.",
                           "Date & Time", MB_ICONEXCLAMATION | MB_OK);
                return TRUE;
            }

            KillTimer(hDlg, IDT_CLOCK_TIMER);
            SetLocalDateTime(g_dtYear, g_dtMonth, g_dtDay,
                             g_dtHour, g_dtMinute, g_dtSecond);
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}
