/*
 * textedit.h – Текстовый редактор поверх холста (Windows 3.0, C89)
 */

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <windows.h>

#define CX_GRIP 4

typedef struct tagTEXTEDIT {
    HWND      hEdit;
    HWND      hParent;
    HFONT     hFont;
    HFONT     hFontZoomed;
    RECT      rc;
    int       nZoom;
    BOOL      bTransparent;
    COLORREF  crText, crBack;
} TEXTEDIT;

BOOL TextEdit_Init(TEXTEDIT* pTE, HWND hParent, HINSTANCE hInst);
void TextEdit_Free(TEXTEDIT* pTE);
void TextEdit_Show(TEXTEDIT* pTE, int x, int y, int w, int h);
void TextEdit_Hide(TEXTEDIT* pTE, HDC hdcImage);
void TextEdit_UpdateColors(TEXTEDIT* pTE, COLORREF crText, COLORREF crBack);
void TextEdit_UpdateFont(TEXTEDIT* pTE, const char* szFace, int ptSize,
                         BOOL bBold, BOOL bItalic, BOOL bUnderline);
void TextEdit_SetZoom(TEXTEDIT* pTE, int nZoom);

LRESULT CALLBACK TextEdit_SubclassProc(HWND, UINT, WPARAM, LPARAM);

#endif
