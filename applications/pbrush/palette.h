/*
 * palette.h – Palette interface (C89, Win16)
 */

#ifndef PALETTE_H
#define PALETTE_H

#include <windows.h>

#define PALETTE_COUNT 16

typedef struct {
    HWND      hWnd;
    COLORREF  crPal[PALETTE_COUNT];
    COLORREF  crFore;
    COLORREF  crBack;
    HINSTANCE hInst;
} PALETTE;

BOOL   Palette_Init(PALETTE FAR* pPal, HWND hParent, HINSTANCE hInst);
void   Palette_Free(PALETTE FAR* pPal);
COLORREF Palette_GetForeColor(PALETTE FAR* pPal);
COLORREF Palette_GetBackColor(PALETTE FAR* pPal);
void   Palette_SetForeColor(PALETTE FAR* pPal, COLORREF cr);
void   Palette_SetBackColor(PALETTE FAR* pPal, COLORREF cr);

#endif
