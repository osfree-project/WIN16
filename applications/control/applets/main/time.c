/*
 *  time.c Ц Date & Time dialog for Windows 3.0
 *  Live clock display, ownerdrawn spin buttons, analog clock, calendar.
 *  Year is displayed and edited as 2 digits (80Ц99 = 1980Ц1999, 00Ц79 = 2000Ц2079).
 *  Supports 12/24-hour time and date format from WIN.INI [intl] settings.
 *  Includes an analog clock and a monthly calendar with navigation arrows.
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
static char g_szAm[8] = "AM";
static char g_szPm[8] = "PM";
static BOOL g_bTLZero = TRUE;
static char g_szTimeSep[4] = ":";

/* ---------- Date format support ---------- */
static int  g_iDateFormat = 0;
static char g_szDateSep[2][4] = {"/", "/"};
static WORD g_dateFieldMap[3] = {IDC_DT_MONTH, IDC_DT_DAY, IDC_DT_YEAR};

/* ---------- Analog clock subclassing ---------- */
static HWND g_hwndClockPlaceholder = NULL;
static WNDPROC g_oldClockProc = NULL;

/* ---------- Calendar subclassing ---------- */
static HWND g_hwndCalPlaceholder = NULL;
static WNDPROC g_oldCalProc = NULL;
static WORD g_calYear, g_calMonth;

/* ---- Colors ---- */
#define FaceColor       (GetSysColor(COLOR_BTNFACE))
#define HandColor       (GetSysColor(COLOR_WINDOWTEXT))
#define TickColor       (GetSysColor(COLOR_WINDOWTEXT))
#define BackgroundColor (GetSysColor(COLOR_WINDOW))

/* ---------- Time delta for manual editing ---------- */
static LONG g_timeDelta = 0;

/* ------------------------------------------------------------------ */
/* DOS API for system time */
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

static void UpdateAmPmLabel(void)
{
    if (g_bUse12Hour && g_hwndAmPmLabel) {
        SetWindowText(g_hwndAmPmLabel, (g_dtHour >= 12) ? g_szPm : g_szAm);
    }
}

static void UpdateField(HWND hDlg, WORD id, WORD value, int digits)
{
    char buf[8];
    if (id == IDC_DT_YEAR) {
        wsprintf(buf, "%02u", YearToDisplay(value));
    } else if (id == IDC_DT_HOUR && g_bUse12Hour) {
        WORD h12 = value % 12;
        if (h12 == 0) h12 = 12;
        if (g_bTLZero && h12 < 10)
            wsprintf(buf, "0%u", h12);
        else
            wsprintf(buf, "%2u", h12);
    } else {
        if (digits == 2) wsprintf(buf, "%02u", value);
        else wsprintf(buf, "%u", value);
    }
    SetDlgItemText(hDlg, id, buf);
}

static void UpdateDateFields(HWND hDlg)
{
    UpdateField(hDlg, g_dateFieldMap[0], g_dtMonth, 2);
    UpdateField(hDlg, g_dateFieldMap[1], g_dtDay, 2);
    UpdateField(hDlg, g_dateFieldMap[2], g_dtYear, 4);
}

/* ------------------------------------------------------------------ */
/* Owner-draw spin buttons */
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

    if (radius > 64) {
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
LRESULT WINAPI AnalogClockSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

        dcMem = CreateCompatibleDC(dcScreen);
        bmMem = CreateCompatibleBitmap(dcScreen, width, height);
        bmOld = SelectObject(dcMem, bmMem);

        hBr = CreateSolidBrush(BackgroundColor);
        FillRect(dcMem, &rc, hBr);
        DeleteObject(hBr);

        AnalogClock(dcMem, width, height, TRUE);

        BitBlt(dcScreen, 0, 0, width, height, dcMem, 0, 0, SRCCOPY);

        SelectObject(dcMem, bmOld);
        DeleteObject(bmMem);
        DeleteDC(dcMem);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
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
/*                    Calendar drawing and logic                       */
/* ================================================================== */

static void DrawArrow(HDC dc, RECT* rc, BOOL left)
{
    HBRUSH hBr, hOldBr;
    HPEN hPen, hOldPen;
    POINT pts[3];
    int cx = (rc->right - rc->left) / 2;
    int cy = (rc->bottom - rc->top) / 2;
    int mx = rc->left + cx;
    int my = rc->top + cy;

    if (left) {
        pts[0].x = mx - cx/2; pts[0].y = my;
        pts[1].x = mx + cx/2; pts[1].y = my - cy/2;
        pts[2].x = mx + cx/2; pts[2].y = my + cy/2;
    } else {
        pts[0].x = mx + cx/2; pts[0].y = my;
        pts[1].x = mx - cx/2; pts[1].y = my - cy/2;
        pts[2].x = mx - cx/2; pts[2].y = my + cy/2;
    }

    hPen = CreatePen(PS_SOLID, 1, HandColor);
    hBr  = CreateSolidBrush(HandColor);
    hOldPen = SelectObject(dc, hPen);
    hOldBr  = SelectObject(dc, hBr);
    Polygon(dc, pts, 3);
    SelectObject(dc, hOldPen);
    SelectObject(dc, hOldBr);
    DeleteObject(hPen);
    DeleteObject(hBr);
}

static void DrawCalendar(HDC dc, int width, int height)
{
    static const char far * monthNames[] = {
        "January","February","March","April","May","June",
        "July","August","September","October","November","December"
    };
    char buf[40];
    static RECT rc, rArrow, cellRect;
    HBRUSH hBr;
    int cellW, cellH, startX, startY;
    int day, row, col, daysInMonth, startDayOfWeek;
    int today = g_dtDay;
    int thisMonth = g_calMonth, thisYear = g_calYear;
    HFONT hFont, hOldFont;
    int a, y, m;
    int textYOffset;

    /* Background */
    rc.left = 0; rc.top = 0; rc.right = width; rc.bottom = height;
    hBr = CreateSolidBrush(BackgroundColor);
    FillRect(dc, &rc, hBr);
    DeleteObject(hBr);

    /* Use SYSTEM_FONT for everything */
    hFont = (HFONT)GetStockObject(SYSTEM_FONT);
    hOldFont = SelectObject(dc, hFont);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, HandColor);

    /* Header (16 pixels) */
    rc.left = 16; rc.top = 0; rc.right = width - 16; rc.bottom = 16;
    wsprintf(buf, "%s %u", monthNames[thisMonth-1], thisYear);
    DrawText(dc, buf, -1, &rc, DT_CENTER | DT_TOP | DT_SINGLELINE);

    /* Left arrow */
    rArrow.left = 2; rArrow.top = 2; rArrow.right = 14; rArrow.bottom = 14;
    DrawArrow(dc, &rArrow, TRUE);
    /* Right arrow */
    rArrow.left = width - 16; rArrow.top = 2; rArrow.right = width - 2; rArrow.bottom = 14;
    DrawArrow(dc, &rArrow, FALSE);

    /* Day names (16 pixels, no more clipping) */
    rc.left = 0; rc.top = 16; rc.right = width; rc.bottom = 32;
    DrawText(dc, "Su Mo Tu We Th Fr Sa", -1, &rc, DT_CENTER | DT_TOP | DT_SINGLELINE);

    /* Grid Ц 50 pixels remaining (32..82) */
    cellW = (width - 4) / 7;
    cellH = (height - 32) / 6;   /* (82-32)/6 = 8 */
    startY = 32;
    startX = 2;

    textYOffset = 0;   /* 8px font in 8px cell Ц perfectly fits */

    a = (14 - thisMonth) / 12;
    y = thisYear - a;
    m = thisMonth + 12 * a - 2;
    startDayOfWeek = (1 + y + y/4 - y/100 + y/400 + (31*m)/12) % 7;
    daysInMonth = MaxDaysInMonth(thisMonth, thisYear);

    day = 1;
    for (row = 0; row < 6; row++) {
        for (col = 0; col < 7; col++) {
            if (row == 0 && col < startDayOfWeek) continue;
            if (day <= daysInMonth) {
                int x = startX + col * cellW;
                int yy = startY + row * cellH;
                cellRect.left = x; cellRect.top = yy;
                cellRect.right = x + cellW; cellRect.bottom = yy + cellH;

                if (day == today && thisMonth == g_dtMonth && thisYear == g_dtYear) {
                    HBRUSH hSel = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                    FillRect(dc, &cellRect, hSel);
                    DeleteObject(hSel);
                    SetTextColor(dc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                } else {
                    SetTextColor(dc, HandColor);
                }

                wsprintf(buf, "%d", day);
                DrawText(dc, buf, -1, &cellRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
                SetTextColor(dc, HandColor);
                day++;
            }
        }
    }

    SelectObject(dc, hOldFont);
}

static void CalendarOnLButtonDown(HWND hWnd, int x, int y, int width, int height)
{
    int cellW, cellH, startX, startY;
    int col, row;
    int a, y2, m, startDow, daysInMonth, dayIndex, day;

    if (y < 16) {   /* header area */
        if (x < 16) {
            if (g_calMonth == 1) { g_calMonth = 12; g_calYear--; }
            else g_calMonth--;
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);   /* <-- немедленна€ отрисовка */
            return;
        } else if (x > width - 16) {
            if (g_calMonth == 12) { g_calMonth = 1; g_calYear++; }
            else g_calMonth++;
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);   /* <-- немедленна€ отрисовка */
            return;
        }
        return;
    }

    if (y < 32) return;   /* day names area */

    cellW = (width - 4) / 7;
    cellH = (height - 32) / 6;
    startX = 2;
    startY = 32;
    col = (x - startX) / cellW;
    row = (y - startY) / cellH;

    a = (14 - g_calMonth) / 12;
    y2 = g_calYear - a;
    m = g_calMonth + 12 * a - 2;
    startDow = (1 + y2 + y2/4 - y2/100 + y2/400 + (31*m)/12) % 7;
    daysInMonth = MaxDaysInMonth(g_calMonth, g_calYear);
    dayIndex = row * 7 + col;

    if (row < 0 || row >= 6 || col < 0 || col >= 7) return;
    if (row == 0 && col < startDow) return;
    day = dayIndex - startDow + 1;
    if (day >= 1 && day <= daysInMonth) {
        g_dtDay = day;
        g_dtMonth = g_calMonth;
        g_dtYear = g_calYear;
        g_bDateModified = TRUE;
        {
            HWND hDlg = GetParent(hWnd);
            UpdateDateFields(hDlg);
        }
        InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindow(hWnd);   /* <-- немедленна€ отрисовка */
    }
}

/* ------------------------------------------------------------ */
/* Subclass procedure for the calendar static placeholder        */
/* ------------------------------------------------------------ */
LRESULT WINAPI CalendarSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC oldProc = g_oldCalProc;

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        RECT rc;
        HDC dcScreen, dcMem;
        HBITMAP bmMem, bmOld;
        int width, height;

        dcScreen = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rc);
        width  = rc.right - rc.left;
        height = rc.bottom - rc.top;

        /* ƒвойна€ буферизаци€: рисуем в пам€ти, выводим на экран */
        dcMem = CreateCompatibleDC(dcScreen);
        bmMem = CreateCompatibleBitmap(dcScreen, width, height);
        bmOld = SelectObject(dcMem, bmMem);

        DrawCalendar(dcMem, width, height);   /* вс€ отрисовка в оффскрин */

        BitBlt(dcScreen, 0, 0, width, height, dcMem, 0, 0, SRCCOPY);

        SelectObject(dcMem, bmOld);
        DeleteObject(bmMem);
        DeleteDC(dcMem);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;   /* фон уже заполн€етс€ в DrawCalendar */
    case WM_DESTROY:
        if (oldProc)
            SetWindowLong(hWnd, GWL_WNDPROC, (LONG)oldProc);
        g_hwndCalPlaceholder = NULL;
        g_oldCalProc = NULL;
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
        char szSep[8];
        int iDate;

        /* Read international settings */
        g_bUse12Hour = (GetProfileInt("intl", "iTime", 0) == 0);
        g_bTLZero    = (GetProfileInt("intl", "iTLZero", 1) != 0);
        GetProfileString("intl", "sTime", ":", g_szTimeSep, sizeof(g_szTimeSep));
        GetProfileString("intl", "s1159", "AM", g_szAm, sizeof(g_szAm));
        GetProfileString("intl", "s2359", "PM", g_szPm, sizeof(g_szPm));
        GetProfileString("intl", "sDate", "/", szSep, sizeof(szSep));
        g_szDateSep[0][0] = szSep[0]; g_szDateSep[0][1] = 0;
        g_szDateSep[1][0] = szSep[0]; g_szDateSep[1][1] = 0;
        iDate = GetProfileInt("intl", "iDate", 0);
        g_iDateFormat = iDate;

        if (iDate == 1) {
            g_dateFieldMap[0] = IDC_DT_DAY;
            g_dateFieldMap[1] = IDC_DT_MONTH;
            g_dateFieldMap[2] = IDC_DT_YEAR;
        } else if (iDate == 2) {
            g_dateFieldMap[0] = IDC_DT_YEAR;
            g_dateFieldMap[1] = IDC_DT_MONTH;
            g_dateFieldMap[2] = IDC_DT_DAY;
        } else {
            g_dateFieldMap[0] = IDC_DT_MONTH;
            g_dateFieldMap[1] = IDC_DT_DAY;
            g_dateFieldMap[2] = IDC_DT_YEAR;
        }

        SetDlgItemText(hDlg, IDC_DT_SEP_DATE1, g_szDateSep[0]);
        SetDlgItemText(hDlg, IDC_DT_SEP_DATE2, g_szDateSep[1]);
        SetDlgItemText(hDlg, IDC_DT_SEP_TIME1, g_szTimeSep);
        SetDlgItemText(hDlg, IDC_DT_SEP_TIME2, g_szTimeSep);

        GetLocalDateTime(&y, &m, &d, &h, &min, &s);
        g_dtYear = y; g_dtMonth = m; g_dtDay = d;
        g_dtHour = h; g_dtMinute = min; g_dtSecond = s;
        _fmemcpy(&g_origYear, &g_dtYear, sizeof(WORD)*6);

        g_bDateModified = FALSE;
        g_bTimeModified = FALSE;
        g_lastDateCtrl = IDC_DT_MONTH;
        g_lastTimeCtrl = IDC_DT_HOUR;
        g_timeDelta = 0;
        g_calYear = y; g_calMonth = m;

        SendDlgItemMessage(hDlg, IDC_DT_MONTH,  EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_DAY,    EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_YEAR,   EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_HOUR,   EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_MINUTE, EM_LIMITTEXT, 2, 0);
        SendDlgItemMessage(hDlg, IDC_DT_SECOND, EM_LIMITTEXT, 2, 0);

        UpdateDateFields(hDlg);
        UpdateField(hDlg, IDC_DT_HOUR,   g_dtHour,  2);
        UpdateField(hDlg, IDC_DT_MINUTE, g_dtMinute, 2);
        UpdateField(hDlg, IDC_DT_SECOND, g_dtSecond, 2);

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

        /* AM/PM label */
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

        /* Subclass clock */
        g_hwndClockPlaceholder = GetDlgItem(hDlg, IDC_ANALOG_CLOCK);
        if (g_hwndClockPlaceholder) {
            g_oldClockProc = (WNDPROC)SetWindowLong(g_hwndClockPlaceholder, GWL_WNDPROC,
                                                    (LONG)AnalogClockSubclassProc);
        }

        /* Subclass calendar */
        g_hwndCalPlaceholder = GetDlgItem(hDlg, IDC_CALENDAR);
        if (g_hwndCalPlaceholder) {
            g_oldCalProc = (WNDPROC)SetWindowLong(g_hwndCalPlaceholder, GWL_WNDPROC,
                                                  (LONG)CalendarSubclassProc);
        }

        SetTimer(hDlg, IDT_CLOCK_TIMER, 100, NULL);
        return TRUE;
    }

    case WM_TIMER:
    {
        if (wParam == IDT_CLOCK_TIMER) {
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
                    UpdateDateFields(hDlg);
                    if (m != g_calMonth || y != g_calYear) {
                        g_calYear = y; g_calMonth = m;
                        if (g_hwndCalPlaceholder) InvalidateRect(g_hwndCalPlaceholder, NULL, FALSE);
                    }
                }
            } else {
                static WORD sysH, sysM, sysS, dummy;
                LONG total;
                GetLocalDateTime(&dummy, &dummy, &dummy, &sysH, &sysM, &sysS);
                total = (LONG)sysH * 3600 + sysM * 60 + sysS + g_timeDelta;
                total = (total % 86400 + 86400) % 86400;
                g_dtHour   = (WORD)(total / 3600);
                g_dtMinute = (WORD)((total % 3600) / 60);
                g_dtSecond = (WORD)(total % 60);
                UpdateAmPmLabel();
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

    case WM_LBUTTONDOWN:
    {
        if (g_hwndCalPlaceholder) {
            RECT rcCal;
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            GetWindowRect(g_hwndCalPlaceholder, &rcCal);
            ScreenToClient(hDlg, (LPPOINT)&rcCal.left);
            ScreenToClient(hDlg, (LPPOINT)&rcCal.right);
            if (PtInRect(&rcCal, pt)) {
                pt.x -= rcCal.left;
                pt.y -= rcCal.top;
                CalendarOnLButtonDown(g_hwndCalPlaceholder, pt.x, pt.y,
                                      rcCal.right - rcCal.left, rcCal.bottom - rcCal.top);
                return TRUE;
            }
        }
        return FALSE;
    }

    case WM_COMMAND:
    {
        WORD code = HIWORD(lParam);

        if (wParam == IDC_DT_DATE_UP || wParam == IDC_DT_DATE_DOWN) {
            g_bDateModified = TRUE;
            SpinDateField(hDlg, g_lastDateCtrl, (wParam == IDC_DT_DATE_UP));
            UpdateDateFields(hDlg);
            if (g_calMonth != g_dtMonth || g_calYear != g_dtYear) {
                g_calMonth = g_dtMonth;
                g_calYear = g_dtYear;
                if (g_hwndCalPlaceholder) InvalidateRect(g_hwndCalPlaceholder, NULL, FALSE);
            } else if (g_hwndCalPlaceholder)
                InvalidateRect(g_hwndCalPlaceholder, NULL, FALSE);
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
            RecalcTimeDelta();
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
            GetDlgItemText(hDlg, g_dateFieldMap[0], buf, sizeof(buf));
            g_dtMonth = (WORD)atoi(buf);
            GetDlgItemText(hDlg, g_dateFieldMap[1], buf, sizeof(buf));
            g_dtDay   = (WORD)atoi(buf);
            GetDlgItemText(hDlg, g_dateFieldMap[2], buf, sizeof(buf));
            g_dtYear  = YearFromDisplay((WORD)atoi(buf));

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
