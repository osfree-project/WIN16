/*
 * canvas.h – Canvas interface (C89, Win16)
 */

#ifndef CANVAS_H
#define CANVAS_H

#include <windows.h>

#define GRIP_SIZE        6
#define HIT_NONE         0
#define HIT_UPPER_LEFT   1
#define HIT_UPPER_CENTER 2
#define HIT_UPPER_RIGHT  3
#define HIT_MIDDLE_LEFT  4
#define HIT_MIDDLE_RIGHT 5
#define HIT_LOWER_LEFT   6
#define HIT_LOWER_CENTER 7
#define HIT_LOWER_RIGHT  8
#define HIT_INNER        9

typedef void (*PFNTOOL_MOUSEDOWN)(int x, int y, BOOL leftButton);
typedef void (*PFNTOOL_MOUSEMOVE)(int x, int y, BOOL leftButton);
typedef void (*PFNTOOL_MOUSEUP)  (int x, int y, BOOL leftButton);

typedef struct {
    HWND      hWnd;
    HBITMAP   hBitmap;
    HDC       hdcMem;
    int       cxImage, cyImage;
    int       nZoom;
    BOOL      bGrid;
    BOOL      bDrawing;
    BOOL      bSizing;
    int       hitSizeBox;
    POINT     ptOrig;
    RECT      rcResizing;
    POINT     ptPrev;
    int       nHorzPos, nVertPos;
    int       nHorzMin, nHorzMax, nHorzPage;
    int       nVertMin, nVertMax, nVertPage;

    PFNTOOL_MOUSEDOWN pfnMouseDown;
    PFNTOOL_MOUSEMOVE pfnMouseMove;
    PFNTOOL_MOUSEUP   pfnMouseUp;
} CANVAS;

BOOL Canvas_Init(CANVAS* pCanvas, HWND hParent, HINSTANCE hInst);
void Canvas_Free(CANVAS* pCanvas);
void Canvas_ResizeImage(CANVAS* pCanvas, int cx, int cy);
void Canvas_Invalidate(CANVAS* pCanvas, BOOL bErase);
void Canvas_SetZoom(CANVAS* pCanvas, int nZoom);
void Canvas_SetGrid(CANVAS* pCanvas, BOOL bShow);
HDC  Canvas_GetDC(CANVAS* pCanvas);
void Canvas_ReleaseDC(CANVAS* pCanvas);
void Canvas_ImageToCanvas(CANVAS* pCanvas, POINT* pt);
void Canvas_CanvasToImage(CANVAS* pCanvas, POINT* pt);
void Canvas_GetImageRectInCanvas(CANVAS* pCanvas, RECT* rc);

BOOL RegisterCanvasClass(HINSTANCE hInst);

extern CANVAS* g_pActiveCanvas;

#endif
