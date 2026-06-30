/*
 * toolbox.h – Toolbar interface (C89, Win16)
 */

#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <windows.h>

#define NUM_TOOLS      16
#define CX_TOOLBAR     78
#define CY_BUTTON      32
#define BTN_MARGIN      4

/* Command identifiers for all 16 tool buttons */
#define IDM_TB_FREESEL    600
#define IDM_TB_RECTSEL    601
#define IDM_TB_RUBBER     602
#define IDM_TB_FILL       603
#define IDM_TB_COLOR      604
#define IDM_TB_ZOOM       605
#define IDM_TB_PEN        606
#define IDM_TB_BRUSH      607
#define IDM_TB_AIRBRUSH   608
#define IDM_TB_TEXT       609
#define IDM_TB_LINE       610
#define IDM_TB_BEZIER     611
#define IDM_TB_RECT       612
#define IDM_TB_SHAPE      613
#define IDM_TB_ELLIPSE    614
#define IDM_TB_RRECT      615

#define IDB_TOOLBARICONS  2000

typedef struct {
    HWND        hWnd;
    HWND        hParent;
    HWND        hBtn[NUM_TOOLS];
    BOOL        bCaptured;
    POINT       ptCapture;
    int         side;
    HINSTANCE   hInst;
    int         currentTool;
} TOOLBOX;

BOOL   Toolbox_Init(TOOLBOX FAR* pTB, HWND hParent, HINSTANCE hInst);
void   Toolbox_Free(TOOLBOX FAR* pTB);
void   Toolbox_Reposition(TOOLBOX FAR* pTB, int cxParent, int cyParent);
void   Toolbox_CheckButton(TOOLBOX FAR* pTB, int idCommand);
void   Toolbox_Show(TOOLBOX FAR* pTB, BOOL bShow);

#endif
