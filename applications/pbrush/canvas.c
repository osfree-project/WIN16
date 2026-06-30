/*
 * canvas.c – Canvas with selection drawing and cursors from .cur files (C89, Win16)
 * No Unicode.  Cursors are loaded directly from CURSOR resources – no runtime conversion.
 */

#include "canvas.h"
#include "paint.h"
#include <string.h>

CANVAS* g_pActiveCanvas = NULL;

/* Preloaded custom cursors – created once, destroyed on exit */
static HCURSOR g_hCurPen      = NULL;
static HCURSOR g_hCurAirbrush = NULL;
static HCURSOR g_hCurFill     = NULL;
static HCURSOR g_hCurColor    = NULL;

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* ---------------------------------------------------------------------------
   Preload all custom cursors (called once during Canvas_Init)
   .cur files are loaded as CURSOR resources.
   --------------------------------------------------------------------------- */
static void PreloadCursors(HINSTANCE hInst)
{
    g_hCurPen      = LoadCursor(hInst, MAKEINTRESOURCE(IDC_PEN_CURSOR));
    g_hCurAirbrush = LoadCursor(hInst, MAKEINTRESOURCE(IDC_AIRBRUSH_CURSOR));
    g_hCurFill     = LoadCursor(hInst, MAKEINTRESOURCE(IDC_FILL_CURSOR));
    g_hCurColor    = LoadCursor(hInst, MAKEINTRESOURCE(IDC_COLOR_CURSOR));
}

/* ---------------------------------------------------------------------------
   Free all custom cursors (called once during Canvas_Free)
   --------------------------------------------------------------------------- */
static void FreeCursors(void)
{
    if (g_hCurPen)      { DestroyCursor(g_hCurPen);      g_hCurPen      = NULL; }
    if (g_hCurAirbrush) { DestroyCursor(g_hCurAirbrush); g_hCurAirbrush = NULL; }
    if (g_hCurFill)     { DestroyCursor(g_hCurFill);     g_hCurFill     = NULL; }
    if (g_hCurColor)    { DestroyCursor(g_hCurColor);    g_hCurColor    = NULL; }
}

/* ---------------------------------------------------------------------------
   Standard canvas functions (unchanged)
   --------------------------------------------------------------------------- */
void Canvas_ImageToCanvas(CANVAS* pCanvas, POINT* pt)
{
    pt->x = pt->x * pCanvas->nZoom + GRIP_SIZE - pCanvas->nHorzPos;
    pt->y = pt->y * pCanvas->nZoom + GRIP_SIZE - pCanvas->nVertPos;
}

void Canvas_CanvasToImage(CANVAS* pCanvas, POINT* pt)
{
    pt->x = (pt->x - GRIP_SIZE + pCanvas->nHorzPos) / pCanvas->nZoom;
    pt->y = (pt->y - GRIP_SIZE + pCanvas->nVertPos) / pCanvas->nZoom;
}

void Canvas_GetImageRectInCanvas(CANVAS* pCanvas, RECT* rc)
{
    rc->left   = GRIP_SIZE - pCanvas->nHorzPos;
    rc->top    = GRIP_SIZE - pCanvas->nVertPos;
    rc->right  = rc->left + pCanvas->cxImage * pCanvas->nZoom;
    rc->bottom = rc->top  + pCanvas->cyImage * pCanvas->nZoom;
}

static int HitTest(const CANVAS* pCanvas, int x, int y)
{
    RECT rc;
    POINT pt;
    Canvas_GetImageRectInCanvas((CANVAS*)pCanvas, &rc);
    pt.x = x; pt.y = y;
    if (PtInRect(&rc, pt))
        return HIT_INNER;

    if (x >= rc.left - GRIP_SIZE && x < rc.left && y >= rc.top - GRIP_SIZE && y < rc.top)
        return HIT_UPPER_LEFT;
    if (x >= rc.right && x < rc.right + GRIP_SIZE && y >= rc.top - GRIP_SIZE && y < rc.top)
        return HIT_UPPER_RIGHT;
    if (x >= rc.left - GRIP_SIZE && x < rc.left && y >= rc.bottom && y < rc.bottom + GRIP_SIZE)
        return HIT_LOWER_LEFT;
    if (x >= rc.right && x < rc.right + GRIP_SIZE && y >= rc.bottom && y < rc.bottom + GRIP_SIZE)
        return HIT_LOWER_RIGHT;
    if (x >= rc.left && x < rc.right && y >= rc.top - GRIP_SIZE && y < rc.top)
        return HIT_UPPER_CENTER;
    if (x >= rc.left && x < rc.right && y >= rc.bottom && y < rc.bottom + GRIP_SIZE)
        return HIT_LOWER_CENTER;
    if (x >= rc.left - GRIP_SIZE && x < rc.left && y >= rc.top && y < rc.bottom)
        return HIT_MIDDLE_LEFT;
    if (x >= rc.right && x < rc.right + GRIP_SIZE && y >= rc.top && y < rc.bottom)
        return HIT_MIDDLE_RIGHT;
    return HIT_NONE;
}

static void DrawGrip(HDC hdc, int x, int y)
{
    RECT rc;
    HBRUSH hbr;
    HPEN hPen, hOldPen;

    rc.left = x; rc.top = y; rc.right = x + GRIP_SIZE; rc.bottom = y + GRIP_SIZE;
    hbr = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &rc, hbr);
    DeleteObject(hbr);

    hPen = GetStockObject(BLACK_PEN);
    hOldPen = SelectObject(hdc, hPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hdc, hOldPen);
}

static void DrawGrips(HDC hdc, const CANVAS* pCanvas)
{
    RECT rc;
    Canvas_GetImageRectInCanvas((CANVAS*)pCanvas, &rc);
    DrawGrip(hdc, rc.left - GRIP_SIZE, rc.top - GRIP_SIZE);
    DrawGrip(hdc, rc.right, rc.top - GRIP_SIZE);
    DrawGrip(hdc, rc.left - GRIP_SIZE, rc.bottom);
    DrawGrip(hdc, rc.right, rc.bottom);
    DrawGrip(hdc, rc.left + (rc.right - rc.left) / 2 - GRIP_SIZE / 2, rc.top - GRIP_SIZE);
    DrawGrip(hdc, rc.left + (rc.right - rc.left) / 2 - GRIP_SIZE / 2, rc.bottom);
    DrawGrip(hdc, rc.left - GRIP_SIZE, rc.top + (rc.bottom - rc.top) / 2 - GRIP_SIZE / 2);
    DrawGrip(hdc, rc.right, rc.top + (rc.bottom - rc.top) / 2 - GRIP_SIZE / 2);
}

static void DrawXorRect(HDC hdc, const RECT* rc)
{
    HPEN hPen;
    int oldROP;
    HBRUSH hOldBrush;
    HPEN hOldPen;

    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    oldROP = SetROP2(hdc, R2_NOTXORPEN);
    hOldPen = SelectObject(hdc, hPen);
    hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, rc->left, rc->top, rc->right, rc->bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    SetROP2(hdc, oldROP);
    DeleteObject(hPen);
}

static void UpdateScrollRange(CANVAS* pCanvas)
{
    RECT rcClient;
    GetClientRect(pCanvas->hWnd, &rcClient);

    pCanvas->nHorzMin  = 0;
    pCanvas->nHorzMax  = pCanvas->cxImage * pCanvas->nZoom + 2 * GRIP_SIZE - 1;
    pCanvas->nHorzPage = rcClient.right;
    SetScrollRange(pCanvas->hWnd, SB_HORZ, pCanvas->nHorzMin, pCanvas->nHorzMax, TRUE);
    SetScrollPos(pCanvas->hWnd, SB_HORZ, pCanvas->nHorzPos, TRUE);

    pCanvas->nVertMin  = 0;
    pCanvas->nVertMax  = pCanvas->cyImage * pCanvas->nZoom + 2 * GRIP_SIZE - 1;
    pCanvas->nVertPage = rcClient.bottom;
    SetScrollRange(pCanvas->hWnd, SB_VERT, pCanvas->nVertMin, pCanvas->nVertMax, TRUE);
    SetScrollPos(pCanvas->hWnd, SB_VERT, pCanvas->nVertPos, TRUE);

    InvalidateRect(pCanvas->hWnd, NULL, TRUE);
}

BOOL Canvas_Init(CANVAS* pCanvas, HWND hParent, HINSTANCE hInst)
{
    HDC hdc;
    HBRUSH hbr;

    memset(pCanvas, 0, sizeof(CANVAS));
    pCanvas->cxImage = 400;
    pCanvas->cyImage = 300;
    pCanvas->nZoom   = 1;

    pCanvas->hWnd = CreateWindow(
        "CanvasClass", "",
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
        0, 0, 0, 0, hParent, NULL, hInst, NULL);

    if (!pCanvas->hWnd)
        return FALSE;

    SetWindowLong(pCanvas->hWnd, 0, (LONG)pCanvas);
    g_pActiveCanvas = pCanvas;

    hdc = GetDC(pCanvas->hWnd);
    pCanvas->hdcMem  = CreateCompatibleDC(hdc);
    pCanvas->hBitmap = CreateCompatibleBitmap(hdc, pCanvas->cxImage, pCanvas->cyImage);
    SelectObject(pCanvas->hdcMem, pCanvas->hBitmap);
    ReleaseDC(pCanvas->hWnd, hdc);

    hbr = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(pCanvas->hdcMem, hbr);
    PatBlt(pCanvas->hdcMem, 0, 0, pCanvas->cxImage, pCanvas->cyImage, PATCOPY);
    DeleteObject(hbr);

    pCanvas->nHorzPos = 0;
    pCanvas->nVertPos = 0;
    UpdateScrollRange(pCanvas);

    /* Preload all custom cursors ONCE */
    PreloadCursors(hInst);

    return TRUE;
}

void Canvas_Free(CANVAS* pCanvas)
{
    /* Free preloaded cursors */
    FreeCursors();

    if (pCanvas->hBitmap) DeleteObject(pCanvas->hBitmap);
    if (pCanvas->hdcMem)  DeleteDC(pCanvas->hdcMem);
    if (pCanvas->hWnd)    DestroyWindow(pCanvas->hWnd);
}

void Canvas_ResizeImage(CANVAS* pCanvas, int cx, int cy)
{
    HBITMAP hNew;
    HDC hdcNew;
    HBITMAP hOld;
    int copyW, copyH;

    hNew   = CreateCompatibleBitmap(pCanvas->hdcMem, cx, cy);
    hdcNew = CreateCompatibleDC(pCanvas->hdcMem);
    hOld   = SelectObject(hdcNew, hNew);

    PatBlt(hdcNew, 0, 0, cx, cy, WHITENESS);
    copyW = MIN(pCanvas->cxImage, cx);
    copyH = MIN(pCanvas->cyImage, cy);
    BitBlt(hdcNew, 0, 0, copyW, copyH,
           pCanvas->hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcNew, hOld);
    DeleteDC(hdcNew);
    DeleteObject(pCanvas->hBitmap);

    pCanvas->hBitmap = hNew;
    SelectObject(pCanvas->hdcMem, hNew);
    pCanvas->cxImage = cx;
    pCanvas->cyImage = cy;

    pCanvas->nHorzPos = 0;
    pCanvas->nVertPos = 0;
    UpdateScrollRange(pCanvas);
}

void Canvas_Invalidate(CANVAS* pCanvas, BOOL bErase)
{
    InvalidateRect(pCanvas->hWnd, NULL, bErase);
}

void Canvas_SetZoom(CANVAS* pCanvas, int nZoom)
{
    pCanvas->nZoom = nZoom;
    pCanvas->nHorzPos = 0;
    pCanvas->nVertPos = 0;
    UpdateScrollRange(pCanvas);
}

void Canvas_SetGrid(CANVAS* pCanvas, BOOL bShow)
{
    pCanvas->bGrid = bShow;
    InvalidateRect(pCanvas->hWnd, NULL, FALSE);
}

HDC Canvas_GetDC(CANVAS* pCanvas)
{
    return pCanvas->hdcMem;
}

void Canvas_ReleaseDC(CANVAS* pCanvas)
{
}

/* External selection globals */
extern BOOL    g_bHasSelection;
extern RECT    g_selRect;
extern HBITMAP g_hSelBitmap;
extern BOOL    g_bMovingSelection;
extern int     g_nTool;

LRESULT CALLBACK CanvasWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CANVAS* pCanvas = (CANVAS*)GetWindowLong(hwnd, 0);
    if (!pCanvas)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_ERASEBKGND:
        return TRUE;

    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT)
        {
            HCURSOR hCur = NULL;

            if (g_bMovingSelection)
            {
                hCur = LoadCursor(NULL, IDC_ARROW);
            }
            else
            {
                switch (g_nTool)
                {
                case TOOL_PEN:
                    hCur = g_hCurPen ? g_hCurPen : LoadCursor(NULL, IDC_CROSS);
                    break;
                case TOOL_AIRBRUSH:
                    hCur = g_hCurAirbrush ? g_hCurAirbrush : LoadCursor(NULL, IDC_CROSS);
                    break;
                case TOOL_FILL:
                    hCur = g_hCurFill ? g_hCurFill : LoadCursor(NULL, IDC_CROSS);
                    break;
                case TOOL_COLORPICK:
                case TOOL_ZOOM:
                    hCur = g_hCurColor ? g_hCurColor : LoadCursor(NULL, IDC_ARROW);
                    break;
                case TOOL_LINE:
                case TOOL_RECT:
                case TOOL_ELLIPSE:
                case TOOL_BRUSH:
                case TOOL_ERASER:
                case TOOL_RRECT:
                case TOOL_FREESEL:
                case TOOL_SELECT:
                    hCur = LoadCursor(NULL, IDC_CROSS);
                    break;
                case TOOL_TEXT:
                    hCur = LoadCursor(NULL, IDC_IBEAM);
                    break;
                default:
                    hCur = LoadCursor(NULL, IDC_ARROW);
                    break;
                }
            }

            if (hCur)
            {
                SetCursor(hCur);
                return TRUE;
            }
        }
        break;
    }

    case WM_SIZE:
        UpdateScrollRange(pCanvas);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc;
        RECT rcClient;
        HDC hdcMem;
        HBITMAP hbmMem, hbmOld;
        RECT rcImg;
        HPEN hGridPen, hOldPen;
        int i;
        RECT rcSel, rcClip;
        HDC hdcSel;
        HBITMAP hbmOldSel;

        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rcClient);

        hdcMem = CreateCompatibleDC(hdc);
        hbmMem = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
        hbmOld = SelectObject(hdcMem, hbmMem);

        FillRect(hdcMem, &rcClient, GetStockObject(LTGRAY_BRUSH));

        Canvas_GetImageRectInCanvas(pCanvas, &rcImg);
        StretchBlt(hdcMem, rcImg.left, rcImg.top,
                   rcImg.right - rcImg.left, rcImg.bottom - rcImg.top,
                   pCanvas->hdcMem, 0, 0,
                   pCanvas->cxImage, pCanvas->cyImage, SRCCOPY);

        /* Draw selection rectangle (clipped to image area) */
        if (g_bHasSelection && !g_bMovingSelection && !IsRectEmpty(&g_selRect))
        {
            rcSel = g_selRect;
            Canvas_ImageToCanvas(pCanvas, (POINT*)&rcSel.left);
            Canvas_ImageToCanvas(pCanvas, (POINT*)&rcSel.right);
            IntersectRect(&rcClip, &rcSel, &rcImg);
            if (!IsRectEmpty(&rcClip))
                DrawXorRect(hdcMem, &rcClip);
        }

        /* Draw moving selection (clipped to image area) */
        if (g_bMovingSelection && g_hSelBitmap)
        {
            rcSel = g_selRect;
            Canvas_ImageToCanvas(pCanvas, (POINT*)&rcSel.left);
            Canvas_ImageToCanvas(pCanvas, (POINT*)&rcSel.right);

            IntersectRect(&rcClip, &rcSel, &rcImg);
            if (!IsRectEmpty(&rcClip))
            {
                int srcX, srcY, srcW, srcH;
                srcX = (rcClip.left - rcSel.left) * (g_selRect.right - g_selRect.left) / (rcSel.right - rcSel.left);
                srcY = (rcClip.top - rcSel.top) * (g_selRect.bottom - g_selRect.top) / (rcSel.bottom - rcSel.top);
                srcW = (rcClip.right - rcClip.left) * (g_selRect.right - g_selRect.left) / (rcSel.right - rcSel.left);
                srcH = (rcClip.bottom - rcClip.top) * (g_selRect.bottom - g_selRect.top) / (rcSel.bottom - rcSel.top);

                hdcSel = CreateCompatibleDC(hdc);
                hbmOldSel = SelectObject(hdcSel, g_hSelBitmap);
                StretchBlt(hdcMem, rcClip.left, rcClip.top,
                           rcClip.right - rcClip.left, rcClip.bottom - rcClip.top,
                           hdcSel, srcX, srcY, srcW, srcH, SRCCOPY);
                SelectObject(hdcSel, hbmOldSel);
                DeleteDC(hdcSel);
            }
        }

        if (!pCanvas->bDrawing)
            DrawGrips(hdcMem, pCanvas);

        if (pCanvas->bGrid && pCanvas->nZoom >= 4)
        {
            hGridPen = CreatePen(PS_DOT, 1, RGB(160, 160, 160));
            hOldPen  = SelectObject(hdcMem, hGridPen);
            for (i = 0; i <= pCanvas->cxImage; i++)
            {
                MoveTo(hdcMem, rcImg.left + i * pCanvas->nZoom, rcImg.top);
                LineTo(hdcMem, rcImg.left + i * pCanvas->nZoom, rcImg.bottom);
            }
            for (i = 0; i <= pCanvas->cyImage; i++)
            {
                MoveTo(hdcMem, rcImg.left, rcImg.top + i * pCanvas->nZoom);
                LineTo(hdcMem, rcImg.right, rcImg.top + i * pCanvas->nZoom);
            }
            SelectObject(hdcMem, hOldPen);
            DeleteObject(hGridPen);
        }

        if (pCanvas->bSizing && !IsRectEmpty(&pCanvas->rcResizing))
            DrawXorRect(hdcMem, &pCanvas->rcResizing);

        BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
               ps.rcPaint.right - ps.rcPaint.left,
               ps.rcPaint.bottom - ps.rcPaint.top,
               hdcMem, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_HSCROLL:
    case WM_VSCROLL:
    {
        int nBar = (msg == WM_HSCROLL) ? SB_HORZ : SB_VERT;
        int nPos;
        int nMin, nMax, nPage;

        if (nBar == SB_HORZ)
        {
            nPos  = pCanvas->nHorzPos;
            nMin  = pCanvas->nHorzMin;
            nMax  = pCanvas->nHorzMax;
            nPage = pCanvas->nHorzPage;
        }
        else
        {
            nPos  = pCanvas->nVertPos;
            nMin  = pCanvas->nVertMin;
            nMax  = pCanvas->nVertMax;
            nPage = pCanvas->nVertPage;
        }

        switch (LOWORD(wParam))
        {
        case SB_THUMBTRACK: nPos = HIWORD(wParam); break;
        case SB_LINELEFT:   nPos -= 15; break;
        case SB_LINERIGHT:  nPos += 15; break;
        case SB_PAGELEFT:   nPos -= nPage; break;
        case SB_PAGERIGHT:  nPos += nPage; break;
        }
        if (nPos < nMin) nPos = nMin;
        if (nPos > nMax - nPage + 1) nPos = nMax - nPage + 1;
        if (nPos < 0) nPos = 0;

        if (nBar == SB_HORZ)
            pCanvas->nHorzPos = nPos;
        else
            pCanvas->nVertPos = nPos;

        SetScrollPos(hwnd, nBar, nPos, TRUE);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        POINT pt;
        int hit;
        POINT ptImg;
        BOOL leftButton = (msg == WM_LBUTTONDOWN);

        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        hit = HitTest(pCanvas, pt.x, pt.y);
        if (hit == HIT_INNER)
        {
            pCanvas->bDrawing = TRUE;
            SetCapture(hwnd);
            ptImg = pt;
            Canvas_CanvasToImage(pCanvas, &ptImg);
            pCanvas->ptPrev = ptImg;
            if (pCanvas->pfnMouseDown)
                pCanvas->pfnMouseDown(ptImg.x, ptImg.y, leftButton);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (hit != HIT_NONE)
        {
            pCanvas->bSizing = TRUE;
            pCanvas->hitSizeBox = hit;
            pCanvas->ptOrig = pt;
            SetCapture(hwnd);
        }
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        POINT pt;
        BOOL leftButton;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);

        if (wParam & MK_RBUTTON)
            leftButton = FALSE;
        else
            leftButton = TRUE;

        if (pCanvas->bDrawing)
        {
            POINT ptImg = pt;
            Canvas_CanvasToImage(pCanvas, &ptImg);
            if (pCanvas->pfnMouseMove)
                pCanvas->pfnMouseMove(ptImg.x, ptImg.y, leftButton);
            pCanvas->ptPrev = ptImg;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (pCanvas->bSizing)
        {
            int dx, dy, newW, newH;
            RECT rc;
            dx = pt.x - pCanvas->ptOrig.x;
            dy = pt.y - pCanvas->ptOrig.y;
            newW = pCanvas->cxImage;
            newH = pCanvas->cyImage;
            switch (pCanvas->hitSizeBox)
            {
            case HIT_UPPER_LEFT:   newW -= dx; newH -= dy; break;
            case HIT_UPPER_CENTER: newH -= dy; break;
            case HIT_UPPER_RIGHT:  newW += dx; newH -= dy; break;
            case HIT_MIDDLE_LEFT:  newW -= dx; break;
            case HIT_MIDDLE_RIGHT: newW += dx; break;
            case HIT_LOWER_LEFT:   newW -= dx; newH += dy; break;
            case HIT_LOWER_CENTER: newH += dy; break;
            case HIT_LOWER_RIGHT:  newW += dx; newH += dy; break;
            }
            newW = MAX(1, MIN(newW, 2048));
            newH = MAX(1, MIN(newH, 2048));
            rc.left = GRIP_SIZE - pCanvas->nHorzPos;
            rc.top  = GRIP_SIZE - pCanvas->nVertPos;
            if (pCanvas->hitSizeBox == HIT_UPPER_LEFT ||
                pCanvas->hitSizeBox == HIT_MIDDLE_LEFT ||
                pCanvas->hitSizeBox == HIT_LOWER_LEFT)
                rc.left += (pCanvas->cxImage - newW) * pCanvas->nZoom;
            if (pCanvas->hitSizeBox == HIT_UPPER_LEFT ||
                pCanvas->hitSizeBox == HIT_UPPER_CENTER ||
                pCanvas->hitSizeBox == HIT_UPPER_RIGHT)
                rc.top += (pCanvas->cyImage - newH) * pCanvas->nZoom;
            rc.right  = rc.left + newW * pCanvas->nZoom;
            rc.bottom = rc.top  + newH * pCanvas->nZoom;
            pCanvas->rcResizing = rc;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    {
        BOOL leftButton = (msg == WM_LBUTTONUP);

        if (pCanvas->bDrawing)
        {
            POINT pt, ptImg;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            ptImg = pt;
            Canvas_CanvasToImage(pCanvas, &ptImg);
            if (pCanvas->pfnMouseUp)
                pCanvas->pfnMouseUp(ptImg.x, ptImg.y, leftButton);
            pCanvas->bDrawing = FALSE;
            ReleaseCapture();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (pCanvas->bSizing)
        {
            int dx, dy, newW, newH;
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            dx = pt.x - pCanvas->ptOrig.x;
            dy = pt.y - pCanvas->ptOrig.y;
            newW = pCanvas->cxImage;
            newH = pCanvas->cyImage;
            switch (pCanvas->hitSizeBox)
            {
            case HIT_UPPER_LEFT:   newW -= dx; newH -= dy; break;
            case HIT_UPPER_CENTER: newH -= dy; break;
            case HIT_UPPER_RIGHT:  newW += dx; newH -= dy; break;
            case HIT_MIDDLE_LEFT:  newW -= dx; break;
            case HIT_MIDDLE_RIGHT: newW += dx; break;
            case HIT_LOWER_LEFT:   newW -= dx; newH += dy; break;
            case HIT_LOWER_CENTER: newH += dy; break;
            case HIT_LOWER_RIGHT:  newW += dx; newH += dy; break;
            }
            newW = MAX(1, MIN(newW, 2048));
            newH = MAX(1, MIN(newH, 2048));
            Canvas_ResizeImage(pCanvas, newW, newH);
            pCanvas->bSizing = FALSE;
            ReleaseCapture();
        }
        return 0;
    }

    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL RegisterCanvasClass(HINSTANCE hInst)
{
    WNDCLASS wc;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = CanvasWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(LONG);
    wc.hInstance     = hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "CanvasClass";
    return RegisterClass(&wc);
}
