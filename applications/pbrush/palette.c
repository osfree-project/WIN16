#include "palette.h"
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void drawColorBox(HDC hdc, LPRECT prc, COLORREF cr, BOOL bSunken)
{
    HBRUSH hbr;
    HPEN hPenTL, hPenBR, hOldPen;
    COLORREF clrTL, clrBR;

    hbr = CreateSolidBrush(cr);
    FillRect(hdc, prc, hbr);
    DeleteObject(hbr);

    if (bSunken)
    {
        clrTL = GetSysColor(COLOR_BTNSHADOW);
        clrBR = GetSysColor(COLOR_BTNHIGHLIGHT);
    }
    else
    {
        clrTL = GetSysColor(COLOR_BTNHIGHLIGHT);
        clrBR = GetSysColor(COLOR_BTNSHADOW);
    }

    hPenTL = CreatePen(PS_SOLID, 1, clrTL);
    hOldPen = SelectObject(hdc, hPenTL);
    MoveTo(hdc, prc->left, prc->bottom - 1);
    LineTo(hdc, prc->left, prc->top);
    LineTo(hdc, prc->right - 1, prc->top);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenTL);

    hPenBR = CreatePen(PS_SOLID, 1, clrBR);
    hOldPen = SelectObject(hdc, hPenBR);
    MoveTo(hdc, prc->right - 1, prc->top);
    LineTo(hdc, prc->right - 1, prc->bottom - 1);
    LineTo(hdc, prc->left - 1, prc->bottom - 1);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenBR);
}

LRESULT CALLBACK PaletteWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PALETTE FAR* pPal = (PALETTE FAR*)GetWindowLong(hwnd, 0);
    if (!pPal) return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_ERASEBKGND:
        return TRUE; /* avoid flickering */

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc, hdcMem;
        HBITMAP hbmMem, hbmOld;
        RECT rc, rcClient;
        int cxIndicator = 40;
        int cyIndicator = 40;
        int leftIndicator = 4;
        int topIndicator;
        int i, w, h;
        HBRUSH hbrBkgnd;

        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rcClient);

        /* memory DC to reduce flicker */
        hdcMem = CreateCompatibleDC(hdc);
        hbmMem = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
        hbmOld = SelectObject(hdcMem, hbmMem);

        /* background */
        hbrBkgnd = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        FillRect(hdcMem, &rcClient, hbrBkgnd);
        DeleteObject(hbrBkgnd);

        /* big indicator box */
        {
            RECT rcBox;
            rcBox.left   = leftIndicator;
            rcBox.top    = (rcClient.bottom - cyIndicator) / 2;
            rcBox.right  = rcBox.left + cxIndicator;
            rcBox.bottom = rcBox.top  + cyIndicator;

            /* Fill with background color (no checkerboard for simplicity) */
            FillRect(hdcMem, &rcBox, CreateSolidBrush(GetSysColor(COLOR_BTNFACE)));

            /* sunken border */
            {
                HPEN hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
                HPEN hOld = SelectObject(hdcMem, hPen);
                HPEN hPen2 = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
                MoveTo(hdcMem, rcBox.left, rcBox.bottom - 1);
                LineTo(hdcMem, rcBox.left, rcBox.top);
                LineTo(hdcMem, rcBox.right - 1, rcBox.top);
                SelectObject(hdcMem, hPen2);
                MoveTo(hdcMem, rcBox.right - 1, rcBox.top);
                LineTo(hdcMem, rcBox.right - 1, rcBox.bottom - 1);
                LineTo(hdcMem, rcBox.left - 1, rcBox.bottom - 1);
                SelectObject(hdcMem, hOld);
                DeleteObject(hPen);
                DeleteObject(hPen2);
            }

            /* background square (at 5/8) */
            {
                RECT rcBack;
                int cxSmall = 15;
                int cySmall = 15;
                rcBack.left   = rcBox.left + (cxIndicator * 5 / 8) - (cxSmall / 2);
                rcBack.top    = rcBox.top  + (cyIndicator * 5 / 8) - (cySmall / 2);
                rcBack.right  = rcBack.left + cxSmall;
                rcBack.bottom = rcBack.top  + cySmall;
                drawColorBox(hdcMem, &rcBack, pPal->crBack, TRUE);
            }

            /* foreground square (at 3/8, overlapping) */
            {
                RECT rcFore;
                int cxSmall = 15;
                int cySmall = 15;
                rcFore.left   = rcBox.left + (cxIndicator * 3 / 8) - (cxSmall / 2);
                rcFore.top    = rcBox.top  + (cyIndicator * 3 / 8) - (cySmall / 2);
                rcFore.right  = rcFore.left + cxSmall;
                rcFore.bottom = rcFore.top  + cySmall;
                drawColorBox(hdcMem, &rcFore, pPal->crFore, TRUE);
            }
        }

        /* 8x2 color grid */
        {
            int gridLeft = leftIndicator + cxIndicator + 8;
            int gridRight = rcClient.right - 2;
            int gridTop = 2;
            int gridBottom = rcClient.bottom - 2;
            w = (gridRight - gridLeft) / 8;
            h = (gridBottom - gridTop) / 2;

            for (i = 0; i < 16; i++)
            {
                int col = i % 8;
                int row = i / 8;
                RECT rcCell;
                rcCell.left   = gridLeft + col * w;
                rcCell.top    = gridTop  + row * h;
                rcCell.right  = rcCell.left + w;
                rcCell.bottom = rcCell.top  + h;
                drawColorBox(hdcMem, &rcCell, pPal->crPal[i], FALSE);
            }
        }

        /* transfer to screen */
        BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        RECT rc;
        POINT pt;
        int cxIndicator = 40;
        int cyIndicator = 40;
        int leftIndicator = 4;
        int topIndicator;
        RECT rcFore, rcBack, rcBox;

        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        GetClientRect(hwnd, &rc);

        topIndicator = (rc.bottom - cyIndicator) / 2;

        rcBox.left   = leftIndicator;
        rcBox.top    = topIndicator;
        rcBox.right  = rcBox.left + cxIndicator;
        rcBox.bottom = rcBox.top  + cyIndicator;

        rcFore.left   = rcBox.left + (cxIndicator * 3 / 8) - 7;
        rcFore.top    = rcBox.top  + (cyIndicator * 3 / 8) - 7;
        rcFore.right  = rcFore.left + 15;
        rcFore.bottom = rcFore.top  + 15;

        rcBack.left   = rcBox.left + (cxIndicator * 5 / 8) - 7;
        rcBack.top    = rcBox.top  + (cyIndicator * 5 / 8) - 7;
        rcBack.right  = rcBack.left + 15;
        rcBack.bottom = rcBack.top  + 15;

        /* click on indicator swaps colors */
        if (PtInRect(&rcFore, pt) || PtInRect(&rcBack, pt))
        {
            COLORREF tmp = pPal->crFore;
            pPal->crFore = pPal->crBack;
            pPal->crBack = tmp;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        /* click inside the 8x2 grid */
        {
            int gridLeft = leftIndicator + cxIndicator + 8;
            int gridRight = rc.right - 2;
            int gridTop = 2;
            int gridBottom = rc.bottom - 2;

            if (pt.x >= gridLeft && pt.x < gridRight && pt.y >= gridTop && pt.y < gridBottom)
            {
                int w = (gridRight - gridLeft) / 8;
                int h = (gridBottom - gridTop) / 2;
                int col = (pt.x - gridLeft) / w;
                int row = (pt.y - gridTop) / h;
                if (col >= 0 && col < 8 && row >= 0 && row < 2)
                {
                    int i = row * 8 + col;
                    if (i < 16)
                    {
                        if (msg == WM_LBUTTONDOWN)
                            pPal->crFore = pPal->crPal[i];
                        else
                            pPal->crBack = pPal->crPal[i];
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
        }
        return 0;
    }

    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL Palette_Init(PALETTE FAR* pPal, HWND hParent, HINSTANCE hInst)
{
    static const COLORREF defPal[16] = {
        RGB(0,0,0),       RGB(128,0,0),     RGB(0,128,0),     RGB(128,128,0),
        RGB(0,0,128),     RGB(128,0,128),   RGB(0,128,128),   RGB(192,192,192),
        RGB(128,128,128), RGB(255,0,0),     RGB(0,255,0),     RGB(255,255,0),
        RGB(0,0,255),     RGB(255,0,255),   RGB(0,255,255),   RGB(255,255,255)
    };
    static BOOL bClassRegistered = FALSE;
    WNDCLASS wc;

    _fmemcpy(pPal->crPal, defPal, sizeof(defPal));
    pPal->crFore = RGB(0,0,0);
    pPal->crBack = RGB(255,255,255);
    pPal->hInst = hInst;

    if (!bClassRegistered)
    {
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = PaletteWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(LONG);
        wc.hInstance     = hInst;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = "PaletteClass";
        if (!RegisterClass(&wc)) return FALSE;
        bClassRegistered = TRUE;
    }

    pPal->hWnd = CreateWindow("PaletteClass", "",
                               WS_CHILD | WS_VISIBLE | WS_BORDER,
                               0, 0, 220, 52,
                               hParent, NULL, hInst, NULL);
    if (!pPal->hWnd) return FALSE;

    SetWindowLong(pPal->hWnd, 0, (LONG)(LPVOID)pPal);
    return TRUE;
}

void Palette_Free(PALETTE FAR* pPal) { if (pPal->hWnd) DestroyWindow(pPal->hWnd); }

COLORREF Palette_GetForeColor(PALETTE FAR* pPal) { return pPal->crFore; }
COLORREF Palette_GetBackColor(PALETTE FAR* pPal) { return pPal->crBack; }

void Palette_SetForeColor(PALETTE FAR* pPal, COLORREF cr)
{
    pPal->crFore = cr;
    InvalidateRect(pPal->hWnd, NULL, TRUE);
}

void Palette_SetBackColor(PALETTE FAR* pPal, COLORREF cr)
{
    pPal->crBack = cr;
    InvalidateRect(pPal->hWnd, NULL, TRUE);
}
