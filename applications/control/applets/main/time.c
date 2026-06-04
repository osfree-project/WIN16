#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

/* ============================================================
 *  Date/Time
 * ============================================================ */
static WORD g_dtYear, g_dtMonth, g_dtDay, g_dtHour, g_dtMinute, g_dtSecond;
static WORD g_origYear, g_origMonth, g_origDay, g_origHour, g_origMinute, g_origSecond;

static void GetLocalDateTime(WORD *y, WORD *m, WORD *d, WORD *h, WORD *min, WORD *s)
{
    static union REGS regs;
    regs.h.ah = 0x2A; intdos(&regs, &regs);
    *y = regs.x.cx; *m = regs.h.dh; *d = regs.h.dl;
    regs.h.ah = 0x2C; intdos(&regs, &regs);
    *h = regs.h.ch; *min = regs.h.cl; *s = regs.h.dh;
}

static void SetLocalDateTime(WORD y, WORD m, WORD d, WORD h, WORD min, WORD s)
{
    static union REGS regs;
    regs.h.ah = 0x2B; regs.x.cx = y; regs.h.dh = m; regs.h.dl = d;
    intdos(&regs, &regs);
    regs.h.ah = 0x2D; regs.h.ch = h; regs.h.cl = min; regs.h.dh = s;
    intdos(&regs, &regs);
}

static void UpdateDateDisplay(HWND hDlg) {
    char buf[64];
    wsprintf(buf, "%s %u, %u", "Date", g_dtMonth, g_dtYear);
    SetDlgItemText(hDlg, IDC_DT_DATE_TEXT, buf);
}
static void UpdateTimeDisplay(HWND hDlg) {
    char buf[32];
    wsprintf(buf, "%02u:%02u:%02u", g_dtHour, g_dtMinute, g_dtSecond);
    SetDlgItemText(hDlg, IDC_DT_TIME_TEXT, buf);
}
static void DrawCalendar(HDC hdc, LPRECT rc) {
    char buf[8];
    wsprintf(buf, "%02u", g_dtDay);
    DrawText(hdc, buf, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

BOOL CALLBACK DateTimeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        GetLocalDateTime(&g_dtYear, &g_dtMonth, &g_dtDay, &g_dtHour, &g_dtMinute, &g_dtSecond);
        _fmemcpy(&g_origYear, &g_dtYear, sizeof(WORD)*6);
        UpdateDateDisplay(hDlg);
        UpdateTimeDisplay(hDlg);
        return TRUE;
    case WM_DRAWITEM:
        if (wParam == IDC_DT_CALENDAR) {
            LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
            DrawCalendar(lpDIS->hDC, &lpDIS->rcItem);
            return TRUE;
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            SetLocalDateTime(g_dtYear, g_dtMonth, g_dtDay, g_dtHour, g_dtMinute, g_dtSecond);
            EndDialog(hDlg, IDOK);
        }
        if (LOWORD(wParam) == IDCANCEL) {
            SetLocalDateTime(g_origYear, g_origMonth, g_origDay, g_origHour, g_origMinute, g_origSecond);
            EndDialog(hDlg, IDCANCEL);
        }
        return TRUE;
    }
    return FALSE;
}
