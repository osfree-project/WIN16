/*
 *  time.c – Date & Time dialog for Windows 3.0
 *  Live clock display, ownerdrawn spin buttons, analog clock.
 *  Year is displayed and edited as 2 digits (80–99 = 1980–1999, 00–79 = 2000–2079).
 *  Supports 12-hour time format with automatic AM/PM label when system is set to 12-hour mode.
 *  Includes an analog clock drawn directly on a subclassed static control.
 *  Analog clock drawing math matches EXACTLY the original winclock.c.
 */

#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"

/* Spin button IDs */
#define IDC_DT_DATE_UP       720
#define IDC_DT_DATE_DOWN     721
#define IDC_DT_TIME_UP       722
#define IDC_DT_TIME_DOWN     723

/* Timer ID for live clock */
#define IDT_CLOCK_TIMER      1

/* M_PI exactly as in original winclock.c */
#ifndef M_PI
#define M_PI 3.1415926535
#endif

/* Global state */
static WORD g_dtYear, g_dtMonth, g_dtDay;
static WORD g_dtHour, g_dtMinute, g_dtSecond;
static WORD g_origYear, g_origMonth, g_origDay;
static WORD g_origHour, g_origMinute, g_origSecond;

/* Flags to prevent timer from overwriting manual edits */
static BOOL g_bDateModified = FALSE;
static BOOL g_bTimeModified = FALSE;

/* Last focused control for date and time */
static WORD g_lastDateCtrl = IDC_DT_MONTH;
static WORD g_lastTimeCtrl = IDC_DT_HOUR;

/* ---------- 12-hour format support ---------- */
static BOOL g_bUse12Hour = FALSE;
static HWND g_hwndAmPmLabel = NULL;

/* ---------- Analog clock subclassing ---------- */
static HWND g_hwndClockPlaceholder = NULL;
static WNDPROC g_oldClockProc = NULL;

/* ---- Colors for the clock (as in original, but background now matches window) ---- */
#define FaceColor       (GetSysColor(COLOR_BTNFACE))
#define HandColor       (GetSysColor(COLOR_WINDOWTEXT))
#define TickColor       (GetSysColor(COLOR_WINDOWTEXT))
#define BackgroundColor (GetSysColor(COLOR_WINDOW))

/* ---------- Time delta for manual editing ---------- */
static LONG g_timeDelta = 0;   /* разница в секундах: отображаемое = системное + дельта */

/* ------------------------------------------------------------------ */
/* DOS API for system time (reliable _dos_xxx functions) */
static void GetLocalDateTime(WORD *y, WORD *m, WORD *d,
                             WORD *h, WORD *min, WORD *s)
{
    static struct dosdate_t date;
    static struct dostime_t time;

    _dos_getdate(&date);
    _dos_gettime(&time);

    *y = date.year;
    *m = date.month;
    *d = date.day;
    *h = time.hour;
    *min = time.minute;
    *s = time.second;
}

static void SetLocalDateTime(WORD y, WORD m, WORD d,
                             WORD h, WORD min, WORD s)
{
    static struct dosdate_t date;
    static struct dostime_t time;

    date.year  = y;
    date.month = m;
    date.day   = d;
    time.hour   = h;
    time.minute = min;
    time.second = s;

    _dos_setdate(&date);
    _dos_settime(&time);
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
/* Year helpers */
static WORD YearToDisplay(WORD fullYear)
{
    return fullYear % 100;
}

static WORD YearFromDisplay(WORD displayYear)
{
    if (displayYear >= 80) return displayYear + 1900;
    else return displayYear + 2000;
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
/* Recalculate time delta based on current fields vs system time */
static void RecalcTimeDelta(void)
{
    static WORD sysH, sysM, sysS;
    static WORD dummy;
    LONG sysTotal, userTotal;

    GetLocalDateTime(&dummy, &dummy, &dummy, &sysH, &sysM, &sysS);
    sysTotal = (LONG)sysH * 3600 + sysM * 60 + sysS;
    userTotal = (LONG)g_dtHour * 3600 + g_dtMinute * 60 + g_dtSecond;
    g_timeDelta = userTotal - sysTotal;
}

/* ------------------------------------------------------------------ */
/* AM/PM label update */
static void UpdateAmPmLabel(void)
{
    if (g_bUse12Hour && g_hwndAmPmLabel) {
        SetWindowText(g_hwndAmPmLabel, (g_dtHour >= 12) ? "PM" : "AM");
    }
}

/* Update a single edit field */
static void UpdateField(HWND hDlg, WORD id, WORD value, int digits)
{
    char buf[8];
    if (id == IDC_DT_YEAR) {
        wsprintf(buf, "%02u", YearToDisplay(value));
    } else if (id == IDC_DT_HOUR && g_bUse12Hour) {
        WORD h12 = value % 12;
        if (h12 == 0) h12 = 12;
        wsprintf(buf, "%2u", h12);
    } else {
        if (digits == 2) wsprintf(buf, "%02u", value);
        else wsprintf(buf, "%u", value);
    }
    SetDlgItemText(hDlg, id, buf);
}

/* ------------------------------------------------------------------ */
/* Owner-draw painting for the flat spin buttons */
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
    HBRUSH hBr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hdc, prc, hBr);
    DeleteObject(hBr);
    DrawSpinTriangle(hdc, prc,
        (id == IDC_DT_DATE_UP || id == IDC_DT_TIME_UP) ? TRUE : FALSE);
}

/* ================================================================== */
/*                    Analog clock (EXACT copy from winclock.c)        */
/* ================================================================== */

typedef struct {
    POINT Start;
    POINT End;
    POINT p2, p3, p4;
} HandData;

static HandData HourHand, MinuteHand, SecondHand;

static void DrawTicks(HDC dc, const POINT* centre, int radius)
{
    int t;
    HPEN oldhPen, hPen;
    HBRUSH oldhBrush, hBrush;
    int hourWidth = 0.09 * radius;

    /* Minute divisions */
    if (radius > 64)
    {
        hPen = CreatePen(PS_SOLID, 2, TickColor);
        oldhPen = SelectObject(dc, hPen);
        for (t = 0; t < 60; t++) {
            MoveTo(dc,
                     centre->x + sin(t * M_PI / 30) * 0.95 * radius,
                     centre->y - cos(t * M_PI / 30) * 0.95 * radius);
            LineTo(dc,
                   centre->x + sin(t * M_PI / 30) * 0.94 * radius,
                   centre->y - cos(t * M_PI / 30) * 0.94 * radius);
        }
        SelectObject(dc, oldhPen);
        DeleteObject(hPen);
    }

    hBrush = CreateSolidBrush(RGB(00, 128, 128));
    oldhBrush = SelectObject(dc, hBrush);
    /* Hour divisions */
    for (t = 0; t < 12; t++) {
        hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        oldhPen = SelectObject(dc, hPen);
        Rectangle(dc,
            centre->x + sin(t * M_PI / 6) * 0.95 * radius - hourWidth / 2,
            centre->y - cos(t * M_PI / 6) * 0.95 * radius - hourWidth / 2,
            centre->x + sin(t * M_PI / 6) * 0.95 * radius + hourWidth / 2,
            centre->y - cos(t * M_PI / 6) * 0.95 * radius + hourWidth / 2);
        SelectObject(dc, oldhPen);
        DeleteObject(hPen);
        hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
        oldhPen = SelectObject(dc, hPen);
        MoveTo(dc,
                 centre->x + sin(t * M_PI / 6) * 0.95 * radius - hourWidth / 2,
                 centre->y - cos(t * M_PI / 6) * 0.95 * radius + hourWidth / 2);
        LineTo(dc,
               centre->x + sin(t * M_PI / 6) * 0.95 * radius - hourWidth / 2,
               centre->y - cos(t * M_PI / 6) * 0.95 * radius - hourWidth / 2);
        LineTo(dc,
               centre->x + sin(t * M_PI / 6) * 0.95 * radius + hourWidth / 2,
               centre->y - cos(t * M_PI / 6) * 0.95 * radius - hourWidth / 2);
        SelectObject(dc, oldhPen);
        DeleteObject(hPen);
    }
    SelectObject(dc, oldhBrush);
    DeleteObject(hBrush);
}

static void DrawHand(HDC dc, HandData* hand)
{
    MoveTo(dc, hand->End.x, hand->End.y);
    LineTo(dc, hand->p2.x, hand->p2.y);
    LineTo(dc, hand->p3.x, hand->p3.y);
    LineTo(dc, hand->p4.x, hand->p4.y);
    LineTo(dc, hand->End.x, hand->End.y);
}

static void DrawHands(HDC dc, BOOL bSeconds)
{
    if (bSeconds) {
        SelectObject(dc, CreatePen(PS_SOLID, 1, HandColor));
        MoveTo(dc, SecondHand.Start.x, SecondHand.Start.y);
        LineTo(dc, SecondHand.End.x, SecondHand.End.y);
        DeleteObject(SelectObject(dc, GetStockObject(NULL_PEN)));
    }

    SelectObject(dc, CreatePen(PS_SOLID, 1, HandColor));
    DrawHand(dc, &MinuteHand);
    DrawHand(dc, &HourHand);
    DeleteObject(SelectObject(dc, GetStockObject(NULL_PEN)));
}

static void PositionHand(const POINT* centre, double length, double angle, HandData* hand)
{
    hand->Start = *centre;
    hand->End.x = centre->x + sin(angle) * length;
    hand->End.y = centre->y - cos(angle) * length;
    hand->p2.x = centre->x + sin(angle + M_PI / 2) * length * 0.07;
    hand->p2.y = centre->y - cos(angle + M_PI / 2) * length * 0.07;
    hand->p3.x = centre->x + sin(angle + M_PI) * length * 0.2;
    hand->p3.y = centre->y - cos(angle + M_PI) * length * 0.2;
    hand->p4.x = centre->x + sin(angle - M_PI / 2) * length * 0.07;
    hand->p4.y = centre->y - cos(angle - M_PI / 2) * length * 0.07;
}

void PositionHands(const POINT* centre, int radius, BOOL bSeconds)
{
    double hour, minute, second;

    /* Используем то же время, что показывают поля ввода */
    hour   = g_dtHour % 12;
    minute = g_dtMinute;
    second = g_dtSecond;

    PositionHand(centre, radius * 0.6,  ((hour * 5 + minute / 12) / (12 * 5)) * 2 * M_PI, &HourHand);
    PositionHand(centre, radius * 0.79, minute / 60 * 2 * M_PI, &MinuteHand);
    if (bSeconds)
        PositionHand(centre, radius * 0.79, second / 60 * 2 * M_PI, &SecondHand);
}

void AnalogClock(HDC dc, int x, int y, BOOL bSeconds)
{
    static POINT centre;
    int radius;

    radius = min(x, y) / 2;
    if (radius < 0) return;

    centre.x = x / 2;
    centre.y = y / 2;

    DrawTicks(dc, &centre, radius);
    PositionHands(&centre, radius, bSeconds);
    DrawHands(dc, bSeconds);
}

/* ------------------------------------------------------------ */
/* Subclass procedure for the static analog clock placeholder   */
/* ------------------------------------------------------------ */
LRESULT CALLBACK AnalogClockSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC oldProc = g_oldClockProc;

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        RECT rc;
        HDC dcScreen, dcMem;
        HBITMAP bmMem, bmOld;
        HBRUSH hBr;
        int width, height;

        dcScreen = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rc);
        width  = rc.right - rc.left;
        height = rc.bottom - rc.top;

        /* Двойная буферизация: рисуем в памяти, затем выводим на экран */
        dcMem = CreateCompatibleDC(dcScreen);
        bmMem = CreateCompatibleBitmap(dcScreen, width, height);
        bmOld = SelectObject(dcMem, bmMem);

        /* Фон */
        hBr = CreateSolidBrush(BackgroundColor);
        FillRect(dcMem, &rc, hBr);
        DeleteObject(hBr);

        /* Часы */
        AnalogClock(dcMem, width, height, TRUE);

        /* Копируем на экран */
        BitBlt(dcScreen, 0, 0, width, height, dcMem, 0, 0, SRCCOPY);

        SelectObject(dcMem, bmOld);
        DeleteObject(bmMem);
        DeleteDC(dcMem);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;   /* не стираем фон, чтобы избежать мерцания */
    case WM_DESTROY:
        if (oldProc)
            SetWindowLong(hWnd, GWL_WNDPROC, (LONG)oldProc);
        g_hwndClockPlaceholder = NULL;
        g_oldClockProc = NULL;
        if (oldProc)
            return CallWindowProc(oldProc, hWnd, msg, wParam, lParam);
        return 0;
    }
    if (oldProc)
        return CallWindowProc(oldProc, hWnd, msg, wParam, lParam);
    return DefWindowProc(hWnd, msg, wParam, lParam);
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
        g_timeDelta = 0;

        g_bUse12Hour = (GetProfileInt("intl", "iTime", 0) == 0);

        SendDlgItemMessage(hDlg, IDC_DT_MONTH,  EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_DAY,    EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_YEAR,   EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_HOUR,   EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_MINUTE, EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_SECOND, EM_LIMITTEXT, 2, 0);

        UpdateField(hDlg, IDC_DT_MONTH,  g_dtMonth, 2);
        UpdateField(hDlg, IDC_DT_DAY,    g_dtDay,   2);
        UpdateField(hDlg, IDC_DT_YEAR,   g_dtYear,  4);
        UpdateField(hDlg, IDC_DT_HOUR,   g_dtHour,  2);
        UpdateField(hDlg, IDC_DT_MINUTE, g_dtMinute,2);
        UpdateField(hDlg, IDC_DT_SECOND, g_dtSecond,2);

        SetWindowLong(GetDlgItem(hDlg, IDC_DT_DATE_GROUP), GWL_STYLE,
                      GetWindowLong(GetDlgItem(hDlg, IDC_DT_DATE_GROUP), GWL_STYLE) | WS_CLIPSIBLINGS);
        SetWindowLong(GetDlgItem(hDlg, IDC_DT_TIME_GROUP), GWL_STYLE,
                      GetWindowLong(GetDlgItem(hDlg, IDC_DT_TIME_GROUP), GWL_STYLE) | WS_CLIPSIBLINGS);

        hDlgFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);

        /* Date spin buttons */
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

        /* Time spin buttons */
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

        /* AM/PM label (12-hour mode) */
        if (g_bUse12Hour) {
            int xLabel = rcGroup.right - 32;
            int yLabel = rcGroup.top + 18;
            g_hwndAmPmLabel = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_CENTER,
                                           xLabel, yLabel, 16, 10,
                                           hDlg, (HMENU)IDC_DT_AMPM_LABEL, g_hInst, NULL);
            if (hDlgFont) SendMessage(g_hwndAmPmLabel, WM_SETFONT, (WPARAM)hDlgFont, 0);
            SetWindowPos(g_hwndAmPmLabel, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
            UpdateAmPmLabel();
        }

        /* Subclass the static placeholder for analog clock */
        g_hwndClockPlaceholder = GetDlgItem(hDlg, IDC_ANALOG_CLOCK);
        if (g_hwndClockPlaceholder) {
            g_oldClockProc = (WNDPROC)SetWindowLong(g_hwndClockPlaceholder, GWL_WNDPROC,
                                                    (LONG)AnalogClockSubclassProc);
        }

        SetTimer(hDlg, IDT_CLOCK_TIMER, 100, NULL);   /* 100 мс для плавности стрелки */
        return TRUE;
    }

    case WM_TIMER:
    {
        if (wParam == IDT_CLOCK_TIMER) {
            /* Update analog clock */
            if (g_hwndClockPlaceholder)
                InvalidateRect(g_hwndClockPlaceholder, NULL, FALSE);

            if (!g_bTimeModified) {
                static WORD y, m, d, h, min, s;
                GetLocalDateTime(&y, &m, &d, &h, &min, &s);
                if (h != g_dtHour || min != g_dtMinute || s != g_dtSecond) {
                    g_dtHour = h; g_dtMinute = min; g_dtSecond = s;
                    UpdateField(hDlg, IDC_DT_HOUR,   h,   2);
                    UpdateField(hDlg, IDC_DT_MINUTE, min, 2);
                    UpdateField(hDlg, IDC_DT_SECOND, s,   2);
                    UpdateAmPmLabel();
                }
                if (!g_bDateModified &&
                    (y != g_dtYear || m != g_dtMonth || d != g_dtDay)) {
                    g_dtYear = y; g_dtMonth = m; g_dtDay = d;
                    UpdateField(hDlg, IDC_DT_MONTH, m, 2);
                    UpdateField(hDlg, IDC_DT_DAY,   d, 2);
                    UpdateField(hDlg, IDC_DT_YEAR,  y, 4);
                }
            } else {
                /* Пользователь редактировал время: идём по системному времени + дельта */
                static WORD sysH, sysM, sysS, dummy;
                LONG total;
                GetLocalDateTime(&dummy, &dummy, &dummy, &sysH, &sysM, &sysS);
                total = (LONG)sysH * 3600 + sysM * 60 + sysS + g_timeDelta;
                /* Нормализуем к 0..86399 */
                total = (total % 86400 + 86400) % 86400;
                g_dtHour   = (WORD)(total / 3600);
                g_dtMinute = (WORD)((total % 3600) / 60);
                g_dtSecond = (WORD)(total % 60);
                UpdateAmPmLabel();
                /* Обновляем поля ввода, чтобы они шли синхронно с часами */
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
            if (g_lastTimeCtrl == IDC_DT_HOUR) UpdateAmPmLabel();
            /* Пересчитываем дельту от нового времени */
            RecalcTimeDelta();
            /* Немедленно перерисовать аналоговые часы */
            if (g_hwndClockPlaceholder)
                InvalidateRect(g_hwndClockPlaceholder, NULL, FALSE);
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
                if (h12 < 1 || h12 > 12) ok = FALSE;
                else {
                    BOOL isPM = (g_dtHour >= 12);
                    g_dtHour = (h12 == 12) ? (isPM ? 12 : 0) : (h12 + (isPM ? 12 : 0));
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
