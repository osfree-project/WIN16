/*
 * textedit.c – Simplified text editor for Paint 3.0 (C89, Win16)
 * No Unicode. Uses system font to avoid CreateFontIndirect issues.
 */

#include "textedit.h"
#include <string.h>
#include "canvas.h"
#include "paint.h"      /* for IDM_TEXT_CANCEL */

#ifndef GWL_WNDPROC
#define GWL_WNDPROC (-4)
#endif

#ifndef CTLCOLOR_EDIT
#define CTLCOLOR_EDIT 1
#endif

#define MAX_TEXT 2048

static TEXTEDIT* g_pTE = NULL;
static WNDPROC   g_oldEditProc = NULL;
static WNDPROC   g_ourEditProc = NULL;

static void FixEditPos(TEXTEDIT* pTE);
static void DrawGrips(HDC hdc, const RECT* rc);

BOOL TextEdit_Init(TEXTEDIT* pTE, HWND hParent, HINSTANCE hInst)
{
    memset(pTE, 0, sizeof(TEXTEDIT));
    pTE->hParent     = hParent;
    pTE->nZoom       = 1;
    pTE->bTransparent = FALSE;
    pTE->crText      = RGB(0,0,0);
    pTE->crBack      = RGB(255,255,255);

    pTE->hEdit = CreateWindow("EDIT", "",
                               WS_CHILD | ES_LEFT | ES_MULTILINE | ES_WANTRETURN |
                               ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                               0, 0, 100, 50, hParent, NULL, hInst, NULL);
    if (!pTE->hEdit) return FALSE;

    g_ourEditProc = (WNDPROC)MakeProcInstance((FARPROC)TextEdit_SubclassProc, hInst);
    if (!g_ourEditProc) return FALSE;

    g_oldEditProc = (WNDPROC)GetWindowLong(pTE->hEdit, GWL_WNDPROC);
    SetWindowLong(pTE->hEdit, GWL_WNDPROC, (LONG)g_ourEditProc);

    g_pTE = pTE;

    /* Use system font to avoid potential issues with CreateFontIndirect */
    pTE->hFont = GetStockObject(SYSTEM_FONT);
    pTE->hFontZoomed = GetStockObject(SYSTEM_FONT);
    SendMessage(pTE->hEdit, WM_SETFONT, (WPARAM)pTE->hFontZoomed, FALSE);

    return TRUE;
}

void TextEdit_Free(TEXTEDIT* pTE)
{
    if (pTE->hEdit)
    {
        SetWindowLong(pTE->hEdit, GWL_WNDPROC, (LONG)g_oldEditProc);
        if (g_ourEditProc) FreeProcInstance((FARPROC)g_ourEditProc);
        g_ourEditProc = NULL;
        DestroyWindow(pTE->hEdit);
    }
    g_pTE = NULL;
    g_oldEditProc = NULL;
}

void TextEdit_Show(TEXTEDIT* pTE, int x, int y, int w, int h)
{
    RECT rc;
    if (!g_pActiveCanvas) return;
    rc.left = x; rc.top = y; rc.right = x + w; rc.bottom = y + h;
    Canvas_ImageToCanvas(g_pActiveCanvas, (POINT*)&rc.left);
    Canvas_ImageToCanvas(g_pActiveCanvas, (POINT*)&rc.right);
    SetWindowPos(pTE->hEdit, NULL, rc.left, rc.top,
                 rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
    ShowWindow(pTE->hEdit, SW_SHOW);
    SetFocus(pTE->hEdit);
    FixEditPos(pTE);
}

void TextEdit_Hide(TEXTEDIT* pTE, HDC hdcImage)
{
    char szText[MAX_TEXT];
    RECT rcWnd;
    HDC hdcEdit;
    HFONT hOldFont;

    if (!IsWindowVisible(pTE->hEdit)) return;

    GetWindowText(pTE->hEdit, szText, sizeof(szText)-1);
    ShowWindow(pTE->hEdit, SW_HIDE);

    GetWindowRect(pTE->hEdit, &rcWnd);
    MapWindowPoints(NULL, pTE->hParent, (POINT*)&rcWnd, 2);
    if (g_pActiveCanvas)
        Canvas_CanvasToImage(g_pActiveCanvas, (POINT*)&rcWnd.left);

    hdcEdit = GetDC(pTE->hEdit);
    if (hdcEdit)
    {
        hOldFont = SelectObject(hdcEdit, pTE->hFont);
        if (!pTE->bTransparent)
        {
            HBRUSH hbr = CreateSolidBrush(pTE->crBack);
            FillRect(hdcImage, &rcWnd, hbr);
            DeleteObject(hbr);
        }
        SetBkMode(hdcImage, pTE->bTransparent ? TRANSPARENT : OPAQUE);
        SetBkColor(hdcImage, pTE->crBack);
        SetTextColor(hdcImage, pTE->crText);
        DrawText(hdcImage, szText, -1, &rcWnd, DT_LEFT | DT_TOP);
        SelectObject(hdcEdit, hOldFont);
        ReleaseDC(pTE->hEdit, hdcEdit);
    }
}

void TextEdit_UpdateColors(TEXTEDIT* pTE, COLORREF crText, COLORREF crBack)
{
    pTE->crText = crText;
    pTE->crBack = crBack;
    InvalidateRect(pTE->hEdit, NULL, TRUE);
}

void TextEdit_UpdateFont(TEXTEDIT* pTE, const char* szFace, int ptSize,
                         BOOL bBold, BOOL bItalic, BOOL bUnderline)
{
    /* Stub for future implementation */
    (void)szFace; (void)ptSize; (void)bBold; (void)bItalic; (void)bUnderline;
}

void TextEdit_SetZoom(TEXTEDIT* pTE, int nZoom)
{
    pTE->nZoom = nZoom;
}

static void FixEditPos(TEXTEDIT* pTE)
{
    char szText[MAX_TEXT];
    HDC hdc;
    RECT rc;

    GetWindowText(pTE->hEdit, szText, sizeof(szText)-1);
    if (szText[0] == '\0')
        lstrcpy(szText, " ");

    hdc = GetDC(pTE->hEdit);
    if (!hdc) return;

    SelectObject(hdc, pTE->hFontZoomed);
    SetRect(&rc, 0, 0, 100, 100);
    DrawText(hdc, szText, -1, &rc, DT_LEFT | DT_TOP | DT_CALCRECT);
    rc.right += 4;
    rc.bottom += 4;
    ReleaseDC(pTE->hEdit, hdc);

    SetWindowPos(pTE->hEdit, NULL, 0, 0, rc.right, rc.bottom,
                 SWP_NOZORDER | SWP_NOMOVE);
    SendMessage(pTE->hEdit, WM_HSCROLL, SB_LEFT, 0);
    SendMessage(pTE->hEdit, WM_VSCROLL, SB_TOP, 0);
    InvalidateRect(pTE->hEdit, NULL, TRUE);
}

static void DrawGrips(HDC hdc, const RECT* rc)
{
    int w, h;
    RECT grip;
    HBRUSH hbr;

    w = rc->right - rc->left;
    h = rc->bottom - rc->top;
    hbr = CreateSolidBrush(RGB(0,0,0));

    SetRect(&grip, rc->left, rc->top, rc->left+CX_GRIP, rc->top+CX_GRIP);
    FillRect(hdc, &grip, hbr);
    SetRect(&grip, rc->right-CX_GRIP, rc->top, rc->right, rc->top+CX_GRIP);
    FillRect(hdc, &grip, hbr);
    SetRect(&grip, rc->left, rc->bottom-CX_GRIP, rc->left+CX_GRIP, rc->bottom);
    FillRect(hdc, &grip, hbr);
    SetRect(&grip, rc->right-CX_GRIP, rc->bottom-CX_GRIP, rc->right, rc->bottom);
    FillRect(hdc, &grip, hbr);
    SetRect(&grip, rc->left + (w-CX_GRIP)/2, rc->top, rc->left + (w+CX_GRIP)/2, rc->top+CX_GRIP);
    FillRect(hdc, &grip, hbr);
    SetRect(&grip, rc->left + (w-CX_GRIP)/2, rc->bottom-CX_GRIP, rc->left + (w+CX_GRIP)/2, rc->bottom);
    FillRect(hdc, &grip, hbr);
    SetRect(&grip, rc->left, rc->top + (h-CX_GRIP)/2, rc->left+CX_GRIP, rc->top + (h+CX_GRIP)/2);
    FillRect(hdc, &grip, hbr);
    SetRect(&grip, rc->right-CX_GRIP, rc->top + (h-CX_GRIP)/2, rc->right, rc->top + (h+CX_GRIP)/2);
    FillRect(hdc, &grip, hbr);

    DeleteObject(hbr);
}

LRESULT CALLBACK TextEdit_SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TEXTEDIT* pTE = g_pTE;
    WNDPROC oldProc = g_oldEditProc;

    if (!pTE || !oldProc)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_CHAR:
        if (wParam == VK_TAB) return 0;
        CallWindowProc((WNDPROC)oldProc, hwnd, msg, wParam, lParam);
        FixEditPos(pTE);
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            SendMessage(pTE->hParent, WM_COMMAND, IDM_TEXT_CANCEL, 0);
            return 0;
        }
        CallWindowProc((WNDPROC)oldProc, hwnd, msg, wParam, lParam);
        FixEditPos(pTE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc;
        RECT rc;

        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        if (!pTE->bTransparent)
        {
            HBRUSH hbr = CreateSolidBrush(pTE->crBack);
            FillRect(hdc, &rc, hbr);
            DeleteObject(hbr);
        }
        CallWindowProc((WNDPROC)oldProc, hwnd, msg, wParam, lParam);
        DrawGrips(hdc, &rc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CTLCOLOR:
        if (HIWORD(wParam) == CTLCOLOR_EDIT)
        {
            SetTextColor((HDC)lParam, pTE->crText);
            if (pTE->bTransparent)
            {
                SetBkMode((HDC)lParam, TRANSPARENT);
                return (LRESULT)GetStockObject(NULL_BRUSH);
            }
            else
            {
                SetBkColor((HDC)lParam, pTE->crBack);
                return (LRESULT)CreateSolidBrush(pTE->crBack);
            }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);

    case WM_NCHITTEST:
    {
        POINT pt;
        RECT rcWnd;

        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        GetWindowRect(hwnd, &rcWnd);
        ScreenToClient(hwnd, &pt);
        if (PtInRect(&rcWnd, pt))
            return HTCAPTION;
        return HTCLIENT;
    }

    case WM_LBUTTONDOWN:
    {
        POINT pt;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        SendMessage(pTE->hParent, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
        return 0;
    }

    case WM_DESTROY:
        SetWindowLong(hwnd, GWL_WNDPROC, (LONG)oldProc);
        if (g_ourEditProc)
        {
            FreeProcInstance((FARPROC)g_ourEditProc);
            g_ourEditProc = NULL;
        }
        g_pTE = NULL;
        g_oldEditProc = NULL;
        break;
    }
    return CallWindowProc((WNDPROC)oldProc, hwnd, msg, wParam, lParam);
}
