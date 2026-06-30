/*
 * tools.c - All drawing tools for Paint 3.0 (C89, Win16)
 * No Unicode. Selection clipping added.
 */

#include "tools.h"
#include "canvas.h"
#include "palette.h"
#include <stdlib.h>

extern CANVAS   g_Canvas;
extern PALETTE  g_Palette;
extern BOOL     g_bDrawing;
extern POINT    g_ptPrev, g_ptOrigin;

extern BOOL    g_bSelecting;
extern RECT    g_selRect;
extern HBITMAP g_hSelBitmap;
extern BOOL    g_bHasSelection;
extern BOOL    g_bMovingSelection;
extern POINT   g_ptSelStart;

static int min_int(int a, int b) { return a < b ? a : b; }
static int max_int(int a, int b) { return a > b ? a : b; }

/* ---------- Pen ---------- */
void Tool_PenDown(int x, int y, BOOL left) { g_ptPrev.x = x; g_ptPrev.y = y; g_bDrawing = TRUE; }
void Tool_PenMove(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF color = left ? Palette_GetForeColor(&g_Palette) : Palette_GetBackColor(&g_Palette);
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN hOld = SelectObject(hdc, hPen);
    MoveTo(hdc, g_ptPrev.x, g_ptPrev.y);
    LineTo(hdc, x, y);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
    g_ptPrev.x = x; g_ptPrev.y = y;
}
void Tool_PenUp(int x, int y, BOOL left) { g_bDrawing = FALSE; }

/* ---------- Line ---------- */
void Tool_LineDown(int x, int y, BOOL left) { g_ptOrigin = g_ptPrev; g_ptPrev.x = x; g_ptPrev.y = y; g_bDrawing = TRUE; }
void Tool_LineMove(int x, int y, BOOL left) { g_ptPrev.x = x; g_ptPrev.y = y; Canvas_Invalidate(&g_Canvas, FALSE); }
void Tool_LineUp(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF color = left ? Palette_GetForeColor(&g_Palette) : Palette_GetBackColor(&g_Palette);
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN hOld = SelectObject(hdc, hPen);
    MoveTo(hdc, g_ptOrigin.x, g_ptOrigin.y);
    LineTo(hdc, x, y);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
    g_bDrawing = FALSE;
}

/* ---------- Rectangle ---------- */
void Tool_RectDown(int x, int y, BOOL left) { g_ptOrigin.x = x; g_ptOrigin.y = y; g_bDrawing = TRUE; }
void Tool_RectMove(int x, int y, BOOL left) { g_ptPrev.x = x; g_ptPrev.y = y; Canvas_Invalidate(&g_Canvas, FALSE); }
void Tool_RectUp(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF color = left ? Palette_GetForeColor(&g_Palette) : Palette_GetBackColor(&g_Palette);
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN hOld = SelectObject(hdc, hPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, min_int(g_ptOrigin.x, x), min_int(g_ptOrigin.y, y),
              max_int(g_ptOrigin.x, x), max_int(g_ptOrigin.y, y));
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
    g_bDrawing = FALSE;
}

/* ---------- Ellipse ---------- */
void Tool_EllipseDown(int x, int y, BOOL left) { g_ptOrigin.x = x; g_ptOrigin.y = y; g_bDrawing = TRUE; }
void Tool_EllipseMove(int x, int y, BOOL left) { g_ptPrev.x = x; g_ptPrev.y = y; Canvas_Invalidate(&g_Canvas, FALSE); }
void Tool_EllipseUp(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF color = left ? Palette_GetForeColor(&g_Palette) : Palette_GetBackColor(&g_Palette);
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN hOld = SelectObject(hdc, hPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Ellipse(hdc, min_int(g_ptOrigin.x, x), min_int(g_ptOrigin.y, y),
            max_int(g_ptOrigin.x, x), max_int(g_ptOrigin.y, y));
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
    g_bDrawing = FALSE;
}

/* ---------- Fill (scanline) ---------- */
void Tool_FillDown(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF oldColor = GetPixel(hdc, x, y);
    COLORREF newColor = left ? Palette_GetForeColor(&g_Palette) : Palette_GetBackColor(&g_Palette);
    int w = g_Canvas.cxImage, h = g_Canvas.cyImage;
    int *stack, sp;
    int x1, x2, i, dy, xx;

    if (oldColor == newColor) return;
    if (x < 0 || x >= w || y < 0 || y >= h) return;

    stack = (int *)GlobalAlloc(GPTR, (w * h / 2) * sizeof(int) * 2);
    if (!stack) return;

    sp = 0;
    stack[sp++] = x;
    stack[sp++] = y;

    while (sp > 0)
    {
        y = stack[--sp];
        x = stack[--sp];

        if (x < 0 || x >= w || y < 0 || y >= h) continue;
        if (GetPixel(hdc, x, y) != oldColor) continue;

        x1 = x;
        while (x1 > 0 && GetPixel(hdc, x1 - 1, y) == oldColor) x1--;
        x2 = x;
        while (x2 < w - 1 && GetPixel(hdc, x2 + 1, y) == oldColor) x2++;

        for (xx = x1; xx <= x2; xx++)
            SetPixel(hdc, xx, y, newColor);

        if (y > 0)
        {
            BOOL add = FALSE;
            for (xx = x1; xx <= x2; xx++)
            {
                if (GetPixel(hdc, xx, y - 1) == oldColor)
                {
                    if (!add) { stack[sp++] = xx; stack[sp++] = y - 1; add = TRUE; }
                }
                else add = FALSE;
            }
        }
        if (y < h - 1)
        {
            BOOL add = FALSE;
            for (xx = x1; xx <= x2; xx++)
            {
                if (GetPixel(hdc, xx, y + 1) == oldColor)
                {
                    if (!add) { stack[sp++] = xx; stack[sp++] = y + 1; add = TRUE; }
                }
                else add = FALSE;
            }
        }
    }

    GlobalFree((HGLOBAL)stack);
}
void Tool_FillMove(int x, int y, BOOL left) {}
void Tool_FillUp(int x, int y, BOOL left) {}

/* ---------- Eraser ---------- */
void Tool_EraserDown(int x, int y, BOOL left) { g_ptPrev.x = x; g_ptPrev.y = y; g_bDrawing = TRUE; }
void Tool_EraserMove(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    RECT rc;
    HBRUSH hbr;
    rc.left = x - 4; rc.top = y - 4; rc.right = x + 4; rc.bottom = y + 4;
    hbr = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &rc, hbr);
    DeleteObject(hbr);
    g_ptPrev.x = x; g_ptPrev.y = y;
}
void Tool_EraserUp(int x, int y, BOOL left) { g_bDrawing = FALSE; }

/* ---------- Color Picker ---------- */
void Tool_ColorPickDown(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF cr = GetPixel(hdc, x, y);
    if (left) Palette_SetForeColor(&g_Palette, cr);
    else      Palette_SetBackColor(&g_Palette, cr);
}
void Tool_ColorPickMove(int x, int y, BOOL left) {}
void Tool_ColorPickUp(int x, int y, BOOL left) {}

/* ---------- Rectangular Selection ---------- */
void Tool_SelectDown(int x, int y, BOOL left)
{
    POINT pt;
    pt.x = x; pt.y = y;

    if (g_bHasSelection && PtInRect(&g_selRect, pt))
    {
        HDC hdc;
        HBRUSH hbr;

        hdc = Canvas_GetDC(&g_Canvas);
        hbr = CreateSolidBrush(RGB(255,255,255));
        FillRect(hdc, &g_selRect, hbr);
        DeleteObject(hbr);

        g_bMovingSelection = TRUE;
        g_ptSelStart = pt;
        Canvas_Invalidate(&g_Canvas, FALSE);
        return;
    }

    g_bSelecting = TRUE;
    g_bHasSelection = FALSE;
    if (g_hSelBitmap) { DeleteObject(g_hSelBitmap); g_hSelBitmap = NULL; }

    g_selRect.left = g_selRect.right = x;
    g_selRect.top = g_selRect.bottom = y;
}

void Tool_SelectMove(int x, int y, BOOL left)
{
    if (g_bMovingSelection)
    {
        int dx = x - g_ptSelStart.x;
        int dy = y - g_ptSelStart.y;

        OffsetRect(&g_selRect, dx, dy);
        g_ptSelStart.x = x;
        g_ptSelStart.y = y;
        Canvas_Invalidate(&g_Canvas, FALSE);
    }
    else if (g_bSelecting)
    {
        g_selRect.right = x;
        g_selRect.bottom = y;
        Canvas_Invalidate(&g_Canvas, FALSE);
    }
}

void Tool_SelectUp(int x, int y, BOOL left)
{
    if (g_bMovingSelection)
    {
        HDC hdc = Canvas_GetDC(&g_Canvas);
        HDC hdcMem;
        HBITMAP hbmOld;
        int srcX = 0, srcY = 0;
        int dstX = g_selRect.left, dstY = g_selRect.top;
        int w = g_selRect.right - g_selRect.left;
        int h = g_selRect.bottom - g_selRect.top;

        /* Clip to canvas */
        if (dstX < 0) { srcX = -dstX; w += dstX; dstX = 0; }
        if (dstY < 0) { srcY = -dstY; h += dstY; dstY = 0; }
        if (dstX + w > g_Canvas.cxImage) w = g_Canvas.cxImage - dstX;
        if (dstY + h > g_Canvas.cyImage) h = g_Canvas.cyImage - dstY;

        if (w > 0 && h > 0)
        {
            hdcMem = CreateCompatibleDC(hdc);
            hbmOld = SelectObject(hdcMem, g_hSelBitmap);

            BitBlt(hdc, dstX, dstY, w, h,
                   hdcMem, srcX, srcY, SRCCOPY);

            SelectObject(hdcMem, hbmOld);
            DeleteDC(hdcMem);
        }

        DeleteObject(g_hSelBitmap);
        g_hSelBitmap = NULL;

        g_bMovingSelection = FALSE;
        g_bHasSelection = FALSE;
        Canvas_Invalidate(&g_Canvas, FALSE);
        return;
    }

    if (g_bSelecting)
    {
        int t;
        g_bSelecting = FALSE;

        if (g_selRect.left > g_selRect.right) {
            t = g_selRect.left; g_selRect.left = g_selRect.right; g_selRect.right = t;
        }
        if (g_selRect.top > g_selRect.bottom) {
            t = g_selRect.top; g_selRect.top = g_selRect.bottom; g_selRect.bottom = t;
        }

        if (g_selRect.right  > g_Canvas.cxImage) g_selRect.right  = g_Canvas.cxImage;
        if (g_selRect.bottom > g_Canvas.cyImage) g_selRect.bottom = g_Canvas.cyImage;
        if (g_selRect.left < 0) g_selRect.left = 0;
        if (g_selRect.top  < 0) g_selRect.top  = 0;

        if (g_selRect.right > g_selRect.left && g_selRect.bottom > g_selRect.top)
        {
            g_bHasSelection = TRUE;
            if (g_hSelBitmap) DeleteObject(g_hSelBitmap);
            g_hSelBitmap = CreateCompatibleBitmap(Canvas_GetDC(&g_Canvas),
                                                   g_selRect.right - g_selRect.left,
                                                   g_selRect.bottom - g_selRect.top);
            if (g_hSelBitmap)
            {
                HDC hdcMem = CreateCompatibleDC(Canvas_GetDC(&g_Canvas));
                HBITMAP hbmOld = SelectObject(hdcMem, g_hSelBitmap);
                BitBlt(hdcMem, 0, 0,
                       g_selRect.right - g_selRect.left,
                       g_selRect.bottom - g_selRect.top,
                       Canvas_GetDC(&g_Canvas),
                       g_selRect.left, g_selRect.top,
                       SRCCOPY);
                SelectObject(hdcMem, hbmOld);
                DeleteDC(hdcMem);
            }
        }
        else
        {
            g_bHasSelection = FALSE;
        }
        Canvas_Invalidate(&g_Canvas, FALSE);
    }
}

/* ---------- FreeSelect (placeholder) ---------- */
void Tool_FreeSelDown(int x, int y, BOOL left) {}
void Tool_FreeSelMove(int x, int y, BOOL left) {}
void Tool_FreeSelUp(int x, int y, BOOL left) {}

/* ---------- Zoom ---------- */
void Tool_ZoomDown(int x, int y, BOOL left)
{
    if (left)
    {
        if (g_Canvas.nZoom < 8) Canvas_SetZoom(&g_Canvas, g_Canvas.nZoom * 2);
    }
    else
    {
        if (g_Canvas.nZoom > 1) Canvas_SetZoom(&g_Canvas, g_Canvas.nZoom / 2);
    }
}
void Tool_ZoomMove(int x, int y, BOOL left) {}
void Tool_ZoomUp(int x, int y, BOOL left) {}

/* ---------- Brush (square) ---------- */
static void BrushRect(HDC hdc, int x, int y, COLORREF color, int size, HBRUSH hbr)
{
    RECT rc;
    rc.left   = x - size / 2;
    rc.top    = y - size / 2;
    rc.right  = rc.left + size;
    rc.bottom = rc.top  + size;
    if (rc.left   < 0)                rc.left   = 0;
    if (rc.top    < 0)                rc.top    = 0;
    if (rc.right  > g_Canvas.cxImage) rc.right  = g_Canvas.cxImage;
    if (rc.bottom > g_Canvas.cyImage) rc.bottom = g_Canvas.cyImage;
    FillRect(hdc, &rc, hbr);
}

void Tool_BrushDown(int x, int y, BOOL left) { g_ptPrev.x = x; g_ptPrev.y = y; g_bDrawing = TRUE; }
void Tool_BrushMove(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF color = left ? Palette_GetForeColor(&g_Palette) : Palette_GetBackColor(&g_Palette);
    int size = 8;
    int dx = x - g_ptPrev.x, dy = y - g_ptPrev.y;
    int steps, i;
    HBRUSH hbr;

    if (dx == 0 && dy == 0) return;
    steps = max_int(abs(dx), abs(dy));
    hbr = CreateSolidBrush(color);
    for (i = 0; i <= steps; i++)
    {
        int cx = g_ptPrev.x + (dx * i) / steps;
        int cy = g_ptPrev.y + (dy * i) / steps;
        BrushRect(hdc, cx, cy, color, size, hbr);
    }
    DeleteObject(hbr);
    g_ptPrev.x = x; g_ptPrev.y = y;
}
void Tool_BrushUp(int x, int y, BOOL left) { g_bDrawing = FALSE; }

/* ---------- AirBrush ---------- */
void Tool_AirBrushDown(int x, int y, BOOL left) { g_ptPrev.x = x; g_ptPrev.y = y; g_bDrawing = TRUE; }
void Tool_AirBrushMove(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF color = left ? Palette_GetForeColor(&g_Palette) : Palette_GetBackColor(&g_Palette);
    int radius = 5;
    int i;
    int w = g_Canvas.cxImage, h = g_Canvas.cyImage;

    for (i = 0; i < 8; i++)
    {
        int rx = (rand() % (radius * 2 + 1)) - radius;
        int ry = (rand() % (radius * 2 + 1)) - radius;
        int px = x + rx, py = y + ry;
        if (rx * rx + ry * ry <= radius * radius && px >= 0 && px < w && py >= 0 && py < h)
            SetPixel(hdc, px, py, color);
    }
    g_ptPrev.x = x; g_ptPrev.y = y;
}
void Tool_AirBrushUp(int x, int y, BOOL left) { g_bDrawing = FALSE; }

/* ---------- Text (placeholder) ---------- */
void Tool_TextDown(int x, int y, BOOL left) {}
void Tool_TextMove(int x, int y, BOOL left) {}
void Tool_TextUp(int x, int y, BOOL left) {}

/* ---------- Bezier (placeholder) ---------- */
void Tool_BezierDown(int x, int y, BOOL left) {}
void Tool_BezierMove(int x, int y, BOOL left) {}
void Tool_BezierUp(int x, int y, BOOL left) {}

/* ---------- Shape (Polygon placeholder) ---------- */
void Tool_ShapeDown(int x, int y, BOOL left) {}
void Tool_ShapeMove(int x, int y, BOOL left) {}
void Tool_ShapeUp(int x, int y, BOOL left) {}

/* ---------- Rounded Rectangle ---------- */
void Tool_RRectDown(int x, int y, BOOL left) { g_ptOrigin.x = x; g_ptOrigin.y = y; g_bDrawing = TRUE; }
void Tool_RRectMove(int x, int y, BOOL left) { g_ptPrev.x = x; g_ptPrev.y = y; Canvas_Invalidate(&g_Canvas, FALSE); }
void Tool_RRectUp(int x, int y, BOOL left)
{
    HDC hdc = Canvas_GetDC(&g_Canvas);
    COLORREF color = left ? Palette_GetForeColor(&g_Palette) : Palette_GetBackColor(&g_Palette);
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN hOld = SelectObject(hdc, hPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, min_int(g_ptOrigin.x, x), min_int(g_ptOrigin.y, y),
              max_int(g_ptOrigin.x, x), max_int(g_ptOrigin.y, y), 16, 16);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
    g_bDrawing = FALSE;
}
