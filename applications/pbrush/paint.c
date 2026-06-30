/*
 * paint.c - Main module for Paint 3.0 (C89, Win16)
 * No Unicode. All tools, selection, clipboard, custom cursors, dialogs.
 */

#include "paint.h"
#include "canvas.h"
#include "toolbox.h"
#include "textedit.h"
#include "palette.h"
#include "file.h"
#include "clipboard.h"
#include "tools.h"
#include <string.h>
#include <stdio.h>

/* Global variables */
HINSTANCE g_hInst;
HWND      g_hMainWnd;
int       g_nTool = TOOL_PEN;
BOOL      g_bDrawing = FALSE;
POINT     g_ptPrev, g_ptOrigin;

/* Objects – global so tools.c can access them */
CANVAS   g_Canvas;
TOOLBOX  g_Toolbox;
TEXTEDIT g_TextEdit;
PALETTE  g_Palette;

/* Selection (globals used by tools.c and canvas.c) */
BOOL    g_bSelecting = FALSE;
RECT    g_selRect;
HBITMAP g_hSelBitmap = NULL;
BOOL    g_bHasSelection = FALSE;
BOOL    g_bMovingSelection = FALSE;
POINT   g_ptSelStart;

/* Prototypes */
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK OpenDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SaveDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ColorDlgProc(HWND, UINT, WPARAM, LPARAM);
void UpdateTitle(void);
void SelectTool(int n);

void UpdateTitle(void)
{
    char buf[144];
    wsprintf(buf, "Paint 3.0 [%dx%d]", g_Canvas.cxImage, g_Canvas.cyImage);
    SetWindowText(g_hMainWnd, buf);
}

void SelectTool(int n)
{
    g_nTool = n;
    Toolbox_CheckButton(&g_Toolbox, IDM_TB_PEN + (n - TOOL_PEN));
}

/* ---------- Dialog callbacks ---------- */
BOOL CALLBACK OpenDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SendDlgItemMessage(hdlg, IDC_DIRLIST, LB_DIR, DDL_READWRITE, (LPARAM)"*.bmp");
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDC_FILELIST)
        {
            char szFile[256];
            int w, h;
            DlgDirSelect(hdlg, szFile, IDC_FILELIST);
            EndDialog(hdlg, IDOK);
            if (LoadBMP(szFile, Canvas_GetDC(&g_Canvas), &w, &h))
            {
                Canvas_ResizeImage(&g_Canvas, w, h);
                UpdateTitle();
            }
        }
        else if (LOWORD(wParam) == IDCANCEL)
            EndDialog(hdlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

BOOL CALLBACK SaveDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hdlg, IDC_FILENAME, "untitled.bmp");
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            char szFile[144];
            GetDlgItemText(hdlg, IDC_FILENAME, szFile, sizeof(szFile));
            if (szFile[0])
            {
                EndDialog(hdlg, IDOK);
                SaveBMP(szFile, Canvas_GetDC(&g_Canvas),
                        g_Canvas.cxImage, g_Canvas.cyImage, g_Palette.crPal, 16);
            }
        }
        else if (LOWORD(wParam) == IDCANCEL)
            EndDialog(hdlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

BOOL CALLBACK ColorDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG: return TRUE;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc;
        RECT rc;
        int w, h;
        int i;

        hdc = BeginPaint(hdlg, &ps);
        GetClientRect(hdlg, &rc);
        w = rc.right / 4;
        h = rc.bottom / 4;
        for (i = 0; i < 16; i++)
        {
            int x = (i % 4) * w;
            int y = (i / 4) * h;
            HBRUSH hbr = CreateSolidBrush(g_Palette.crPal[i]);
            SelectObject(hdc, hbr);
            Rectangle(hdc, x, y, x + w, y + h);
            DeleteObject(hbr);
            {
                HPEN hPen = GetStockObject(BLACK_PEN);
                HPEN hOld = SelectObject(hdc, hPen);
                SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Rectangle(hdc, x, y, x + w, y + h);
                SelectObject(hdc, hOld);
            }
        }
        EndPaint(hdlg, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        RECT rc;
        int w, h;
        int i;

        GetClientRect(hdlg, &rc);
        w = rc.right / 4;
        h = rc.bottom / 4;
        i = (HIWORD(lParam) / h) * 4 + (LOWORD(lParam) / w);
        if (i >= 0 && i < 16)
        {
            Palette_SetForeColor(&g_Palette, g_Palette.crPal[i]);
            EndDialog(hdlg, IDOK);
        }
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
            EndDialog(hdlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

/* ---------- Main window procedure ---------- */
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        if (!Canvas_Init(&g_Canvas, hwnd, g_hInst))
            return -1;

        g_Toolbox.side = 0;
        if (!Toolbox_Init(&g_Toolbox, hwnd, g_hInst))
            return -1;

        if (!Palette_Init(&g_Palette, hwnd, g_hInst))
            return -1;

        g_Canvas.pfnMouseDown = Tool_PenDown;
        g_Canvas.pfnMouseMove = Tool_PenMove;
        g_Canvas.pfnMouseUp   = Tool_PenUp;

        Toolbox_Reposition(&g_Toolbox, rc.right, rc.bottom - 52);
        MoveWindow(g_Canvas.hWnd, (g_Toolbox.side == 0) ? CX_TOOLBAR : 0, 0,
                   rc.right - CX_TOOLBAR, rc.bottom - 52, TRUE);
        MoveWindow(g_Palette.hWnd, 0, rc.bottom - 52, rc.right, 52, TRUE);

        /* Update cursor for the default tool */
        SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));

        return 0;
    }

    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        Toolbox_Reposition(&g_Toolbox, rc.right, rc.bottom - 52);
        MoveWindow(g_Canvas.hWnd, (g_Toolbox.side == 0) ? CX_TOOLBAR : 0, 0,
                   rc.right - CX_TOOLBAR, rc.bottom - 52, TRUE);
        MoveWindow(g_Palette.hWnd, 0, rc.bottom - 52, rc.right, 52, TRUE);

        InvalidateRect(g_Canvas.hWnd, NULL, TRUE);
        UpdateWindow(g_Canvas.hWnd);

        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, GetStockObject(LTGRAY_BRUSH));
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            if (g_bHasSelection || g_bSelecting || g_bMovingSelection)
            {
                g_bHasSelection = FALSE;
                g_bSelecting = FALSE;
                g_bMovingSelection = FALSE;
                if (g_hSelBitmap) { DeleteObject(g_hSelBitmap); g_hSelBitmap = NULL; }
                Canvas_Invalidate(&g_Canvas, FALSE);
            }
            return 0;

        case VK_DELETE:
            if (g_bHasSelection && !g_bMovingSelection)
            {
                HBRUSH hbr = CreateSolidBrush(RGB(255,255,255));
                HDC hdc = Canvas_GetDC(&g_Canvas);
                RECT rc = g_selRect;
                FillRect(hdc, &rc, hbr);
                DeleteObject(hbr);
                g_bHasSelection = FALSE;
                if (g_hSelBitmap) { DeleteObject(g_hSelBitmap); g_hSelBitmap = NULL; }
                Canvas_Invalidate(&g_Canvas, FALSE);
            }
            return 0;
        }
        break;
    }

    case WM_LBUTTONUP:
        if (GetCapture())
            ReleaseCapture();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        /* ---- Tools (each changes g_nTool and updates cursor) ---- */
        case IDM_TB_PEN:
            g_Canvas.pfnMouseDown=Tool_PenDown; g_Canvas.pfnMouseMove=Tool_PenMove; g_Canvas.pfnMouseUp=Tool_PenUp;
            g_nTool = TOOL_PEN;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_LINE:
            g_Canvas.pfnMouseDown=Tool_LineDown; g_Canvas.pfnMouseMove=Tool_LineMove; g_Canvas.pfnMouseUp=Tool_LineUp;
            g_nTool = TOOL_LINE;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_RECT:
            g_Canvas.pfnMouseDown=Tool_RectDown; g_Canvas.pfnMouseMove=Tool_RectMove; g_Canvas.pfnMouseUp=Tool_RectUp;
            g_nTool = TOOL_RECT;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_ELLIPSE:
            g_Canvas.pfnMouseDown=Tool_EllipseDown; g_Canvas.pfnMouseMove=Tool_EllipseMove; g_Canvas.pfnMouseUp=Tool_EllipseUp;
            g_nTool = TOOL_ELLIPSE;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_FILL:
            g_Canvas.pfnMouseDown=Tool_FillDown; g_Canvas.pfnMouseMove=Tool_FillMove; g_Canvas.pfnMouseUp=Tool_FillUp;
            g_nTool = TOOL_FILL;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_RUBBER:
            g_Canvas.pfnMouseDown=Tool_EraserDown; g_Canvas.pfnMouseMove=Tool_EraserMove; g_Canvas.pfnMouseUp=Tool_EraserUp;
            g_nTool = TOOL_ERASER;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_COLOR:
            g_Canvas.pfnMouseDown=Tool_ColorPickDown; g_Canvas.pfnMouseMove=Tool_ColorPickMove; g_Canvas.pfnMouseUp=Tool_ColorPickUp;
            g_nTool = TOOL_COLORPICK;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_RECTSEL:
            g_Canvas.pfnMouseDown=Tool_SelectDown; g_Canvas.pfnMouseMove=Tool_SelectMove; g_Canvas.pfnMouseUp=Tool_SelectUp;
            g_nTool = TOOL_SELECT;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_FREESEL:
            g_Canvas.pfnMouseDown=Tool_FreeSelDown; g_Canvas.pfnMouseMove=Tool_FreeSelMove; g_Canvas.pfnMouseUp=Tool_FreeSelUp;
            g_nTool = TOOL_FREESEL;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_ZOOM:
            g_Canvas.pfnMouseDown=Tool_ZoomDown; g_Canvas.pfnMouseMove=Tool_ZoomMove; g_Canvas.pfnMouseUp=Tool_ZoomUp;
            g_nTool = TOOL_ZOOM;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_BRUSH:
            g_Canvas.pfnMouseDown=Tool_BrushDown; g_Canvas.pfnMouseMove=Tool_BrushMove; g_Canvas.pfnMouseUp=Tool_BrushUp;
            g_nTool = TOOL_BRUSH;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_AIRBRUSH:
            g_Canvas.pfnMouseDown=Tool_AirBrushDown; g_Canvas.pfnMouseMove=Tool_AirBrushMove; g_Canvas.pfnMouseUp=Tool_AirBrushUp;
            g_nTool = TOOL_AIRBRUSH;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_TEXT:
            g_Canvas.pfnMouseDown=Tool_TextDown; g_Canvas.pfnMouseMove=Tool_TextMove; g_Canvas.pfnMouseUp=Tool_TextUp;
            g_nTool = TOOL_TEXT;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_BEZIER:
            g_Canvas.pfnMouseDown=Tool_BezierDown; g_Canvas.pfnMouseMove=Tool_BezierMove; g_Canvas.pfnMouseUp=Tool_BezierUp;
            g_nTool = TOOL_PEN;   /* reuse pen cursor temporarily */
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_SHAPE:
            g_Canvas.pfnMouseDown=Tool_ShapeDown; g_Canvas.pfnMouseMove=Tool_ShapeMove; g_Canvas.pfnMouseUp=Tool_ShapeUp;
            g_nTool = TOOL_PEN;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;
        case IDM_TB_RRECT:
            g_Canvas.pfnMouseDown=Tool_RRectDown; g_Canvas.pfnMouseMove=Tool_RRectMove; g_Canvas.pfnMouseUp=Tool_RRectUp;
            g_nTool = TOOL_RRECT;
            SendMessage(g_Canvas.hWnd, WM_SETCURSOR, 0, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            break;

        /* ---- Edit commands ---- */
        case IDM_EDIT_SELECTALL:
            g_selRect.left = 0;
            g_selRect.top = 0;
            g_selRect.right = g_Canvas.cxImage;
            g_selRect.bottom = g_Canvas.cyImage;
            g_bHasSelection = TRUE;
            if (g_hSelBitmap) { DeleteObject(g_hSelBitmap); g_hSelBitmap = NULL; }
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
                       Canvas_GetDC(&g_Canvas), 0, 0, SRCCOPY);
                SelectObject(hdcMem, hbmOld);
                DeleteDC(hdcMem);
            }
            Canvas_Invalidate(&g_Canvas, FALSE);
            break;

        case IDM_EDIT_COPY:
            if (g_bHasSelection && g_hSelBitmap && !g_bMovingSelection)
                Clipboard_Copy(g_hSelBitmap);
            break;

        case IDM_EDIT_PASTE:
        {
            HBITMAP hbm = Clipboard_Paste();
            if (hbm)
            {
                BITMAP bm;
                GetObject(hbm, sizeof(BITMAP), &bm);

                if (g_bHasSelection || g_bMovingSelection)
                {
                    g_bHasSelection = FALSE;
                    g_bMovingSelection = FALSE;
                    if (g_hSelBitmap) DeleteObject(g_hSelBitmap);
                }

                g_selRect.left   = (g_Canvas.cxImage - bm.bmWidth) / 2;
                g_selRect.top    = (g_Canvas.cyImage - bm.bmHeight) / 2;
                g_selRect.right  = g_selRect.left + bm.bmWidth;
                g_selRect.bottom = g_selRect.top  + bm.bmHeight;

                g_hSelBitmap = hbm;
                g_bHasSelection = TRUE;
                Canvas_Invalidate(&g_Canvas, FALSE);
            }
            break;
        }

        case IDM_EDIT_CLEAR:
            if (g_bHasSelection && !g_bMovingSelection)
            {
                HBRUSH hbr = CreateSolidBrush(RGB(255,255,255));
                HDC hdc = Canvas_GetDC(&g_Canvas);
                RECT rc = g_selRect;
                FillRect(hdc, &rc, hbr);
                DeleteObject(hbr);
                g_bHasSelection = FALSE;
                if (g_hSelBitmap) { DeleteObject(g_hSelBitmap); g_hSelBitmap = NULL; }
                Canvas_Invalidate(&g_Canvas, FALSE);
            }
            break;

        /* ---- File commands ---- */
        case IDM_FILE_NEW:
            Canvas_ResizeImage(&g_Canvas, 400, 300);
            UpdateTitle();
            break;
        case IDM_FILE_OPEN:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_OPENFILE), hwnd, OpenDlgProc);
            break;
        case IDM_FILE_SAVE:
            SaveBMP("untitled.bmp", Canvas_GetDC(&g_Canvas),
                    g_Canvas.cxImage, g_Canvas.cyImage, g_Palette.crPal, 16);
            break;
        case IDM_FILE_SAVEAS:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SAVEFILE), hwnd, SaveDlgProc);
            break;
        case IDM_FILE_EXIT:
            DestroyWindow(hwnd);
            break;
        }
        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        if (g_hSelBitmap) DeleteObject(g_hSelBitmap);
        Canvas_Free(&g_Canvas);
        Toolbox_Free(&g_Toolbox);
        Palette_Free(&g_Palette);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    MSG msg;

    g_hInst = hInstance;

    if (!hPrevInstance)
    {
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MainWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
        wc.lpszMenuName = "MAINMENU";
        wc.lpszClassName = "PaintClass";
        if (!RegisterClass(&wc)) return FALSE;
    }

    RegisterCanvasClass(hInstance);

    g_hMainWnd = CreateWindow("PaintClass", "Paint 3.0",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                               CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
                               NULL, NULL, hInstance, NULL);
    if (!g_hMainWnd) return FALSE;

    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
