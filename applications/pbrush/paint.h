/*
 * paint.h – Common definitions for Paint 3.0 (C89, Win16)
 */

#ifndef PAINT_H
#define PAINT_H

#include <windows.h>

/* Menu identifiers */
#define IDM_FILE_NEW       100
#define IDM_FILE_OPEN      101
#define IDM_FILE_SAVE      102
#define IDM_FILE_SAVEAS    103
#define IDM_FILE_EXIT      104

#define IDM_EDIT_COPY      200
#define IDM_EDIT_PASTE     201
#define IDM_EDIT_CLEAR     202
#define IDM_EDIT_SELECTALL 203
#define IDM_EDIT_COLOR     204

#define IDM_TOOL_PEN       300
#define IDM_TOOL_LINE      301
#define IDM_TOOL_RECT      302
#define IDM_TOOL_ELLIPSE   303
#define IDM_TOOL_FILL      304
#define IDM_TOOL_ERASER    305
#define IDM_TOOL_COLORPICK 306
#define IDM_TOOL_SELECT    307

#define IDM_IMAGE_INVERT   400
#define IDM_IMAGE_FLIPH    401
#define IDM_IMAGE_FLIPV    402
#define IDM_IMAGE_ROT90    403
#define IDM_IMAGE_ROT180   404
#define IDM_IMAGE_ROT270   405

#define IDM_VIEW_ZOOM1     500
#define IDM_VIEW_ZOOM2     501
#define IDM_VIEW_ZOOM4     502
#define IDM_VIEW_ZOOM8     503
#define IDM_VIEW_GRID      504
#define IDM_VIEW_MINIATURE 505
#define IDM_VIEW_PALETTE   506
#define IDM_VIEW_TOOLBOX   507

#define IDM_TEXT_CANCEL    601   /* for text edit tool */

#define IDD_OPENFILE       1000
#define IDD_SAVEFILE       1001
#define IDD_COLOR          2000
#define IDC_FILENAME       101
#define IDC_DIRLIST        102
#define IDC_FILELIST       103
#define IDC_DRIVELIST      104

/* Tool codes */
#define TOOL_PEN       0
#define TOOL_LINE      1
#define TOOL_RECT      2
#define TOOL_ELLIPSE   3
#define TOOL_FILL      4
#define TOOL_ERASER    5
#define TOOL_COLORPICK 6
#define TOOL_SELECT    7
#define TOOL_TEXT      8
#define TOOL_BRUSH     9
#define TOOL_AIRBRUSH  10
#define TOOL_ZOOM      11
#define TOOL_FREESEL   12
#define TOOL_RRECT     13

/* Custom cursor identifiers */
#define IDC_PEN_CURSOR      2001
#define IDC_FILL_CURSOR     2002
#define IDC_COLOR_CURSOR    2003
#define IDC_ZOOM_CURSOR     2004
#define IDC_AIRBRUSH_CURSOR 2007

/* Global variables */
extern HINSTANCE g_hInst;
extern HWND      g_hMainWnd;
extern int       g_nTool;
extern BOOL      g_bDrawing;
extern POINT     g_ptPrev, g_ptOrigin;

/* Helper functions */
int  min_int(int a, int b);
int  max_int(int a, int b);
void UpdateTitle(void);
void SelectTool(int n);

#endif
