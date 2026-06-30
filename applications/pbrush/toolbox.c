/*
 * toolbox.c Ц Toolbar with 32x32 icons and manual dragging (no system caption drag)
 * Completely avoids ghosting by managing repositioning itself.
 */

#include "toolbox.h"
#include <string.h>

static UINT aCommands[NUM_TOOLS] = {
    IDM_TB_FREESEL, IDM_TB_RECTSEL, IDM_TB_RUBBER, IDM_TB_FILL,
    IDM_TB_COLOR,   IDM_TB_ZOOM,    IDM_TB_PEN,    IDM_TB_BRUSH,
    IDM_TB_AIRBRUSH, IDM_TB_TEXT,   IDM_TB_LINE,   IDM_TB_BEZIER,
    IDM_TB_RECT,    IDM_TB_SHAPE,   IDM_TB_ELLIPSE, IDM_TB_RRECT
};

static int aIconIndex[NUM_TOOLS] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

static HBITMAP g_hToolbarBitmap = NULL;
static HBITMAP g_hMaskedBitmap  = NULL;

static HBITMAP CreateMaskedBitmap(HBITMAP hSrc, COLORREF clrKey, COLORREF clrReplace)
{
    BITMAP bm;
    HDC hdcScreen, hdcSrc, hdcDst;
    HBITMAP hDst, hOldSrc, hOldDst;
    int x, y;
    COLORREF cr;

    GetObject(hSrc, sizeof(BITMAP), &bm);
    hdcScreen = GetDC(NULL);
    hdcSrc = CreateCompatibleDC(hdcScreen);
    hdcDst = CreateCompatibleDC(hdcScreen);
    hDst   = CreateCompatibleBitmap(hdcScreen, bm.bmWidth, bm.bmHeight);
    hOldSrc = SelectObject(hdcSrc, hSrc);
    hOldDst = SelectObject(hdcDst, hDst);

    for (y = 0; y < bm.bmHeight; y++)
    {
        for (x = 0; x < bm.bmWidth; x++)
        {
            cr = GetPixel(hdcSrc, x, y);
            if (cr == clrKey)
                SetPixel(hdcDst, x, y, clrReplace);
            else
                SetPixel(hdcDst, x, y, cr);
        }
    }

    SelectObject(hdcSrc, hOldSrc);
    SelectObject(hdcDst, hOldDst);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    ReleaseDC(NULL, hdcScreen);
    return hDst;
}

static void DrawOwnerButton(HDC hdc, LPRECT prc, int iconIdx, BOOL bPressed)
{
    HBRUSH hbr;
    COLORREF clrTL, clrBR;
    HPEN hPen, oldPen;
    int x, y;

    hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hdc, prc, hbr);
    DeleteObject(hbr);

    clrTL = bPressed ? GetSysColor(COLOR_BTNSHADOW) : GetSysColor(COLOR_BTNHIGHLIGHT);
    clrBR = bPressed ? GetSysColor(COLOR_BTNHIGHLIGHT) : GetSysColor(COLOR_BTNSHADOW);

    hPen = CreatePen(PS_SOLID, 1, clrTL);
    oldPen = SelectObject(hdc, hPen);
    MoveTo(hdc, prc->left, prc->bottom - 1);
    LineTo(hdc, prc->left, prc->top);
    LineTo(hdc, prc->right - 1, prc->top);
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);

    hPen = CreatePen(PS_SOLID, 1, clrBR);
    oldPen = SelectObject(hdc, hPen);
    MoveTo(hdc, prc->right - 1, prc->top);
    LineTo(hdc, prc->right - 1, prc->bottom - 1);
    LineTo(hdc, prc->left - 1, prc->bottom - 1);
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);

    if (bPressed) OffsetRect(prc, 1, 1);

    if (g_hMaskedBitmap && iconIdx >= 0)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hOld = SelectObject(hdcMem, g_hMaskedBitmap);
        x = prc->left + (prc->right  - prc->left - 32) / 2;
        y = prc->top  + (prc->bottom - prc->top  - 32) / 2;
        StretchBlt(hdc, x, y, 32, 32,
                   hdcMem, iconIdx * 16, 0, 16, 16, SRCCOPY);
        SelectObject(hdcMem, hOld);
        DeleteDC(hdcMem);
    }
}

LRESULT CALLBACK ToolboxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TOOLBOX FAR* pTB = (TOOLBOX FAR*)GetWindowLong(hwnd, 0);
    if (!pTB) return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
        int i;
        BOOL bActive;

        if (lpDIS->CtlType != ODT_BUTTON)
            return FALSE;

        for (i = 0; i < NUM_TOOLS; i++)
        {
            if (aCommands[i] == lpDIS->CtlID)
                break;
        }
        if (i == NUM_TOOLS)
            return FALSE;

        bActive = (aCommands[i] == (UINT)pTB->currentTool);
        DrawOwnerButton(lpDIS->hDC, &lpDIS->rcItem, aIconIndex[i],
                        bActive || (lpDIS->itemState & ODS_SELECTED));
        return TRUE;
    }

    case WM_COMMAND:
        pTB->currentTool = LOWORD(wParam);
        InvalidateRect(hwnd, NULL, TRUE);
        SendMessage(pTB->hParent, WM_COMMAND, LOWORD(wParam), 0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        POINT pt;
        int i;
        RECT rc;

        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);

        /* ѕроверим, не нажата ли кнопка */
        for (i = 0; i < NUM_TOOLS; i++)
        {
            if (pTB->hBtn[i])
            {
                GetWindowRect(pTB->hBtn[i], &rc);
                ScreenToClient(hwnd, (POINT*)&rc.left);
                ScreenToClient(hwnd, (POINT*)&rc.right);
                if (PtInRect(&rc, pt))
                {
                    /* јктивируем инструмент и не начинаем перетаскивание */
                    pTB->currentTool = aCommands[i];
                    InvalidateRect(hwnd, NULL, TRUE);
                    SendMessage(pTB->hParent, WM_COMMAND, aCommands[i], 0);
                    return 0;
                }
            }
        }

        /*  лик по пустому месту Ц начинаем перетаскивание */
        pTB->bCaptured = TRUE;
        pTB->ptCapture = pt;
        SetCapture(hwnd);
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        int x, y;

        x = LOWORD(lParam);
        y = HIWORD(lParam);

        if (pTB->bCaptured)
        {
            POINT ptScreen;
            RECT rcParent;
            int centerX, newSide;

            ptScreen.x = x;
            ptScreen.y = y;
            ClientToScreen(hwnd, &ptScreen);
            GetWindowRect(pTB->hParent, &rcParent);
            centerX = (rcParent.left + rcParent.right) / 2;
            newSide = (ptScreen.x < centerX) ? 0 : 1;

            if (newSide != pTB->side)
            {
                pTB->side = newSide;
                PostMessage(pTB->hParent, WM_SIZE, 0, 0);
            }
        }
        return 0;
    }

    case WM_LBUTTONUP:
        if (pTB->bCaptured)
        {
            ReleaseCapture();
            pTB->bCaptured = FALSE;
        }
        return 0;

    case WM_DESTROY:
        if (g_hToolbarBitmap) { DeleteObject(g_hToolbarBitmap); g_hToolbarBitmap = NULL; }
        if (g_hMaskedBitmap)  { DeleteObject(g_hMaskedBitmap);  g_hMaskedBitmap  = NULL; }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL Toolbox_Init(TOOLBOX FAR* pTB, HWND hParent, HINSTANCE hInst)
{
    int i, x, y;
    static BOOL bClassRegistered = FALSE;
    WNDCLASS wc;

    _fmemset(pTB, 0, sizeof(TOOLBOX));
    pTB->hParent = hParent;
    pTB->hInst   = hInst;
    pTB->side    = 0;               /* default left */
    pTB->currentTool = IDM_TB_PEN;

    if (!bClassRegistered)
    {
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = ToolboxWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(LPVOID);
        wc.hInstance     = hInst;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = "PaintToolbox";
        if (!RegisterClass(&wc)) return FALSE;
        bClassRegistered = TRUE;
    }

    pTB->hWnd = CreateWindow("PaintToolbox", "",
                              WS_CHILD | WS_VISIBLE | WS_BORDER,
                              0, 0, CX_TOOLBAR, 200,
                              hParent, NULL, hInst, NULL);
    if (!pTB->hWnd) return FALSE;

    SetWindowLong(pTB->hWnd, 0, (LONG)(LPVOID)pTB);

    if (!g_hToolbarBitmap)
        g_hToolbarBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TOOLBARICONS));
    if (g_hToolbarBitmap && !g_hMaskedBitmap)
        g_hMaskedBitmap = CreateMaskedBitmap(g_hToolbarBitmap, RGB(255,0,255),
                                             GetSysColor(COLOR_BTNFACE));

    x = BTN_MARGIN;
    y = BTN_MARGIN;
    for (i = 0; i < NUM_TOOLS; i++)
    {
        pTB->hBtn[i] = CreateWindow("BUTTON", "",
                                     WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                                     x, y, 32, CY_BUTTON,
                                     pTB->hWnd, (HMENU)aCommands[i], hInst, NULL);

        if (x == BTN_MARGIN)
            x = BTN_MARGIN + 32 + BTN_MARGIN;
        else
        {
            x = BTN_MARGIN;
            y += CY_BUTTON + BTN_MARGIN;
        }
    }

    return TRUE;
}

void Toolbox_Free(TOOLBOX FAR* pTB)
{
    if (g_hToolbarBitmap) { DeleteObject(g_hToolbarBitmap); g_hToolbarBitmap = NULL; }
    if (g_hMaskedBitmap)  { DeleteObject(g_hMaskedBitmap);  g_hMaskedBitmap  = NULL; }
    if (pTB->hWnd) DestroyWindow(pTB->hWnd);
}

void Toolbox_Reposition(TOOLBOX FAR* pTB, int cxParent, int cyParent)
{
    int x = (pTB->side == 0) ? 0 : cxParent - CX_TOOLBAR;
    MoveWindow(pTB->hWnd, x, 0, CX_TOOLBAR, cyParent, TRUE);
}

void Toolbox_CheckButton(TOOLBOX FAR* pTB, int idCommand)
{
    pTB->currentTool = idCommand;
    InvalidateRect(pTB->hWnd, NULL, TRUE);
}

void Toolbox_Show(TOOLBOX FAR* pTB, BOOL bShow)
{
    ShowWindow(pTB->hWnd, bShow ? SW_SHOW : SW_HIDE);
}
