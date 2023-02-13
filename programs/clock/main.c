/*
 * Clock
 *
 * Copyright 1998 Marcel Baur <mbaur@g26.ethz.ch>
 *
 * Clock is partially based on
 * - Program Manager by Ulrich Schmied
 * - rolex.c by Jim Peterson
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>
#include <string.h>

#include "windows.h"
//#include "commctrl.h"
#include "commdlg.h"
#include "shellapi.h"

#include "main.h"
#include "winclock.h"

#define INITIAL_WINDOW_SIZE 208
#define TIMER_ID 1

CLOCK_GLOBALS Globals;

static VOID CLOCK_UpdateMenuCheckmarks(VOID)
{
    HMENU hPropertiesMenu;
    hPropertiesMenu = GetSubMenu(Globals.hMainMenu, 0);
    if (!hPropertiesMenu)
	return;

    if(Globals.bAnalog) {

        /* analog clock */
        CheckMenuItem(hPropertiesMenu, IDM_ANALOG, MF_CHECKED);
        CheckMenuItem(hPropertiesMenu, IDM_DIGITAL, MF_UNCHECKED);

        EnableMenuItem(hPropertiesMenu, IDM_FONT, MF_GRAYED);
    }
    else
    {
        /* digital clock */
        CheckMenuItem(hPropertiesMenu, IDM_ANALOG, MF_UNCHECKED);
        CheckMenuItem(hPropertiesMenu, IDM_DIGITAL, MF_CHECKED);

        EnableMenuItem(hPropertiesMenu, IDM_FONT, 0);
    }

    CheckMenuItem(hPropertiesMenu, IDM_NOTITLE, (Globals.bWithoutTitle ? MF_CHECKED : MF_UNCHECKED));

    //CheckMenuItem(hPropertiesMenu, IDM_ONTOP, (Globals.bAlwaysOnTop ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hPropertiesMenu, IDM_SECONDS, (Globals.bSeconds ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hPropertiesMenu, IDM_DATE, (Globals.bDate ? MF_CHECKED : MF_UNCHECKED));

    CheckMenuItem(Globals.hSysMenu, IDM_ONTOP, (Globals.bAlwaysOnTop ? MF_CHECKED : MF_UNCHECKED));
}

static VOID CLOCK_UpdateWindowCaption(VOID)
{
    char szCaption[MAX_STRING_LEN];
    int chars = 0;

    /* Set frame caption */
/*    if (Globals.bDate) {
	chars = GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, NULL, NULL,
                               szCaption, ARRAY_SIZE(szCaption));
        if (chars) {
	    --chars;
	    szCaption[chars++] = ' ';
	    szCaption[chars++] = '-';
	    szCaption[chars++] = ' ';
	    szCaption[chars] = '\0';
	}
    }
	*/
    LoadString(Globals.hInstance, IDS_CLOCK, szCaption + chars, MAX_STRING_LEN - chars);
    SetWindowText(Globals.hMainWnd, szCaption);
}

/***********************************************************************
 *
 *           CLOCK_ResetTimer
 */
static BOOL CLOCK_ResetTimer(void)
{
    UINT period; /* milliseconds */

    KillTimer(Globals.hMainWnd, TIMER_ID);

//    if (Globals.bSeconds)
//	if (Globals.bAnalog)
//	    period = 500; // was 500 for smooth tick, but we don't support smooth tick in win 3.x ;)
//	else
//	    period = 500;
//    else
	period = 1000;

    if (!SetTimer (Globals.hMainWnd, TIMER_ID, period, NULL)) {
        char szApp[MAX_STRING_LEN];
        LoadString(Globals.hInstance, IDS_CLOCK, szApp, MAX_STRING_LEN);
        MessageBox(0, "No available timers", szApp, MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }
    return TRUE;
}

/***********************************************************************
 *
 *           CLOCK_ResetFont
 */
static VOID CLOCK_ResetFont(VOID)
{
    HFONT newfont;
    HDC dc = GetDC(Globals.hMainWnd);
    newfont = SizeFont(dc, Globals.MaxX, Globals.MaxY, Globals.bSeconds, &Globals.logfont);
    if (newfont) {
	DeleteObject(Globals.hFont);
	Globals.hFont = newfont;
    }
	
    ReleaseDC(Globals.hMainWnd, dc);
}


/***********************************************************************
 *
 *           CLOCK_ChooseFont
 */
static VOID CLOCK_ChooseFont(VOID)
{
    LOGFONT lf;
    CHOOSEFONT cf;

    memset(&cf, 0, sizeof(cf));
    lf = Globals.logfont;
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = Globals.hMainWnd;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;// | CF_NOVERTFONTS;
    if (ChooseFont(&cf)) {
	Globals.logfont = lf;
	CLOCK_ResetFont();
    }
}

/***********************************************************************
 *
 *           CLOCK_ToggleTitle
 */
static VOID CLOCK_ToggleTitle(VOID)
{
    /* Also shows/hides the menu */
    LONG style = GetWindowLong(Globals.hMainWnd, GWL_STYLE);
    if ((Globals.bWithoutTitle = !Globals.bWithoutTitle)) {
	style = (style & ~WS_OVERLAPPEDWINDOW) | WS_POPUP|WS_THICKFRAME;
	SetMenu(Globals.hMainWnd, 0);
    }
    else {
	style = (style & ~(WS_POPUP|WS_THICKFRAME)) | WS_OVERLAPPEDWINDOW;
        SetMenu(Globals.hMainWnd, Globals.hMainMenu);
        //SetWindowRgn(Globals.hMainWnd, 0, TRUE);
    }
    SetWindowLong(Globals.hMainWnd, GWL_STYLE, style);
    SetWindowPos(Globals.hMainWnd, 0,0,0,0,0, 
		 SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
    
    CLOCK_UpdateMenuCheckmarks();
    CLOCK_UpdateWindowCaption();
}

/***********************************************************************
 *
 *           CLOCK_ToggleOnTop
 */
static VOID CLOCK_ToggleOnTop(VOID)
{
    if ((Globals.bAlwaysOnTop = !Globals.bAlwaysOnTop)) {
	SetWindowPos(Globals.hMainWnd, HWND_TOPMOST, 0,0,0,0,
		     SWP_NOMOVE|SWP_NOSIZE);
    }
    else {
	SetWindowPos(Globals.hMainWnd, HWND_NOTOPMOST, 0,0,0,0,
		     SWP_NOMOVE|SWP_NOSIZE);
    }
    CLOCK_UpdateMenuCheckmarks();
}
/***********************************************************************
 *
 *           CLOCK_MenuCommand
 *
 *  All handling of main menu events
 */

static int CLOCK_MenuCommand (WPARAM wParam)
{
    char szApp[MAX_STRING_LEN];
    char szAppRelease[MAX_STRING_LEN];

    switch (wParam) {
        /* switch to analog */
        case IDM_ANALOG: {
            Globals.bAnalog = TRUE;
            CLOCK_UpdateMenuCheckmarks();
	    CLOCK_ResetTimer();
	    InvalidateRect(Globals.hMainWnd, NULL, FALSE);
            break;
        }
            /* switch to digital */
        case IDM_DIGITAL: {
            Globals.bAnalog = FALSE;
            CLOCK_UpdateMenuCheckmarks();
	    CLOCK_ResetTimer();
	    CLOCK_ResetFont();
	    InvalidateRect(Globals.hMainWnd, NULL, FALSE);
            break;
        }
            /* change font */
        case IDM_FONT: {
            CLOCK_ChooseFont();
            break;
        }
            /* hide title bar */
        case IDM_NOTITLE: {
	    CLOCK_ToggleTitle();
            break;
        }
            /* always on top */
        case IDM_ONTOP: {
	    CLOCK_ToggleOnTop();
            break;
        }
            /* show or hide seconds */
        case IDM_SECONDS: {
            Globals.bSeconds = !Globals.bSeconds;
            CLOCK_UpdateMenuCheckmarks();
	    CLOCK_ResetTimer();
	    if (!Globals.bAnalog)
		CLOCK_ResetFont();
	    InvalidateRect(Globals.hMainWnd, NULL, FALSE);
            break;
        }
            /* show or hide date */
        case IDM_DATE: {
            Globals.bDate = !Globals.bDate;
            CLOCK_UpdateMenuCheckmarks();
            CLOCK_UpdateWindowCaption();
            break;
        }
            /* show "about" box */
        case IDM_ABOUT: {
            LoadString(Globals.hInstance, IDS_CLOCK, szApp, sizeof(szApp));
            lstrcpy(szAppRelease, szApp);
            ShellAbout(Globals.hMainWnd, szApp, szAppRelease, 0);
            break;
        }
    }
    return 0;
}

/***********************************************************************
 *
 *           CLOCK_Paint
 */
static VOID CLOCK_Paint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC dcMem, dc;
    HBITMAP bmMem, bmOld;
    HBRUSH hBrush;
	
    dc = BeginPaint(hWnd, &ps);


    /* Use an offscreen dc to avoid flicker */
    dcMem = CreateCompatibleDC(dc);
    bmMem = CreateCompatibleBitmap(dc, ps.rcPaint.right - ps.rcPaint.left,
				    ps.rcPaint.bottom - ps.rcPaint.top);

    bmOld = SelectObject(dcMem, bmMem);

    SetViewportOrgEx(dcMem, -ps.rcPaint.left, -ps.rcPaint.top, NULL);

    hBrush=CreateSolidBrush(BackgroundColor);
    /* Erase the background */
    FillRect(dcMem, &ps.rcPaint,  hBrush);
    DeleteObject(hBrush);

    if(Globals.bAnalog)
	AnalogClock(dcMem, Globals.MaxX, Globals.MaxY, Globals.bSeconds, Globals.bWithoutTitle);
    else
	DigitalClock(dcMem, Globals.MaxX, Globals.MaxY, Globals.bSeconds, Globals.hFont);

    /* Blit the changes to the screen */
    BitBlt(dc, 
	   ps.rcPaint.left, ps.rcPaint.top,
	   ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
           dcMem,
	   ps.rcPaint.left, ps.rcPaint.top,
           SRCCOPY);

    SelectObject(dcMem, bmOld);
    DeleteObject(bmMem);
    DeleteDC(dcMem);
  
    EndPaint(hWnd, &ps);
}

/***********************************************************************
 *
 *           CLOCK_WndProc
 */

static LRESULT WINAPI CLOCK_WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
	/* L button drag moves the window */
        case WM_NCHITTEST: {
	    LRESULT ret = DefWindowProc(hWnd, msg, wParam, lParam);
            if (Globals.bWithoutTitle) 
	      if (ret == HTCLIENT)
		ret = HTCAPTION;
            return ret;
	}

        case WM_NCLBUTTONDBLCLK:
        case WM_LBUTTONDBLCLK: {
	    CLOCK_ToggleTitle();
            break;
        }

        case WM_PAINT: {
	    CLOCK_Paint(hWnd);
            break;

        }

        case WM_SIZE: {
            Globals.MaxX = LOWORD(lParam);
            Globals.MaxY = HIWORD(lParam);
			if (wParam==SIZE_MINIMIZED)
			{
				Globals.bMaximized=FALSE;
				Globals.bMinimized=TRUE;
			}
			if (wParam==SIZE_MAXIMIZED)
			{
				Globals.bMaximized=TRUE;
				Globals.bMinimized=FALSE;
			}
#if 0
// Windows 3.x doesn't support window regions
            if (Globals.bAnalog && Globals.bWithoutTitle)
            {
                RECT rect;
                int diameter = min( Globals.MaxX, Globals.MaxY );
                HRGN hrgn = CreateEllipticRgn( (Globals.MaxX - diameter) / 2,
                                               (Globals.MaxY - diameter) / 2,
                                               (Globals.MaxX + diameter) / 2,
                                               (Globals.MaxY + diameter) / 2 );
                GetWindowRect( hWnd, &rect );
                MapWindowPoints( 0, hWnd, (LPPOINT)&rect, 2 );
                OffsetRgn( hrgn, -rect.left, -rect.top );
                //SetWindowRgn( Globals.hMainWnd, hrgn, TRUE );
            }
#endif
			CLOCK_ResetFont();
            break;
        }

		case WM_SYSCOMMAND: {
			if (wParam==IDM_ONTOP)
			{
				CLOCK_ToggleOnTop();
				return 0;
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}

        case WM_COMMAND: {
            CLOCK_MenuCommand(wParam);
            break;
        }

        case WM_TIMER: {
            /* Could just invalidate what has changed,
             * but it doesn't really seem worth the effort
             */
	    InvalidateRect(Globals.hMainWnd, NULL, FALSE);
	    break;
        }

        case WM_DESTROY: {
            PostQuitMessage (0);
            break;
        }

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}


/***********************************************************************
 *
 *           WinMain
 */

int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE prev, LPSTR cmdline, int show)
{
    MSG      msg;
    WNDCLASS class;
	char buffer[100];
    LONG style = WS_OVERLAPPEDWINDOW;
    int  left , top, right, bottom;

    /* Setup Globals */
    memset(&Globals, 0, sizeof (Globals));
    Globals.hInstance       = hInstance;
    Globals.lpszIniFile     = "clock.ini";

    /* Read Options from `clock.ini' */
    Globals.bMaximized = GetPrivateProfileInt("Clock", "Maximized", 0, Globals.lpszIniFile);

    /* Get the geometry of the main window */
    GetPrivateProfileString("Clock", "Options", "", buffer, sizeof(buffer), Globals.lpszIniFile);
    if (6 == sscanf(buffer, "%d,%d,%d,%d,%d,%d", &(Globals.bAnalog), &(Globals.bMinimized), &(Globals.bSeconds), &(Globals.bWithoutTitle), &(Globals.bAlwaysOnTop), &(Globals.bDate)))
    {
      Globals.bSeconds=!Globals.bSeconds;
      Globals.bDate=!Globals.bDate;
    }
    else
    {
      Globals.bAnalog         = TRUE;
      Globals.bSeconds        = TRUE;
      Globals.bDate           = TRUE;
    }

	if (Globals.bMaximized) show=SW_SHOWMAXIMIZED;
	if (Globals.bMinimized) show=SW_SHOWMINIMIZED;

    if (!prev){
        class.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        class.lpfnWndProc   = CLOCK_WndProc;
        class.cbClsExtra    = 0;
        class.cbWndExtra    = 0;
        class.hInstance     = hInstance;
        class.hIcon         = LoadIcon(0, (LPCSTR)IDI_APPLICATION);
        class.hCursor       = LoadCursor(0, (LPCSTR)IDC_ARROW);
        class.hbrBackground = 0;
        class.lpszMenuName  = 0;
        class.lpszClassName = "CLClass";
    }

    if (!RegisterClass(&class)) return FALSE;


    GetPrivateProfileString("Clock", "Position", "", buffer, sizeof(buffer), Globals.lpszIniFile);
    if (4 == sscanf(buffer, "%d,%d,%d,%d", &left, &top, &right, &bottom))
    {
      Globals.MaxX = right - left;
      Globals.MaxY = bottom - top;
    }
    else
    {
      left = top = CW_USEDEFAULT;
      Globals.MaxX = Globals.MaxY = INITIAL_WINDOW_SIZE;
      Globals.MaxY +=GetSystemMetrics(SM_CYCAPTION);//+GetSystemMetrics(SM_CYMENU);
    }

    GetPrivateProfileString("Clock", "sFont", "", Globals.logfont.lfFaceName, sizeof(Globals.logfont.lfFaceName), Globals.lpszIniFile);
	
    Globals.hMainWnd = CreateWindow("CLClass", "Clock", style/*WS_OVERLAPPEDWINDOW*/,
                                     left, top,
                                     Globals.MaxX, Globals.MaxY, 0,
                                     0, hInstance, 0);

 if (Globals.hMainWnd)
  {
    Globals.hSysMenu = GetSystemMenu(Globals.hMainWnd, FALSE);
    InsertMenu(Globals.hSysMenu, 10, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenu(Globals.hSysMenu, 11, MF_BYPOSITION |(Globals.bAlwaysOnTop ? MF_CHECKED : MF_UNCHECKED), (UINT) IDM_ONTOP, "Always on &Top");
  }

    if (!CLOCK_ResetTimer())
        return FALSE;

    Globals.hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(MAIN_MENU));
    SetMenu(Globals.hMainWnd, Globals.hMainMenu);
    CLOCK_UpdateMenuCheckmarks();
    CLOCK_UpdateWindowCaption();
    
    ShowWindow (Globals.hMainWnd, show);
    if (Globals.bWithoutTitle) {CLOCK_ToggleTitle(); CLOCK_ToggleTitle();}
    if (Globals.bAlwaysOnTop) {CLOCK_ToggleOnTop(); CLOCK_ToggleOnTop();}
    UpdateWindow (Globals.hMainWnd);
    
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KillTimer(Globals.hMainWnd, TIMER_ID);
    DeleteObject(Globals.hFont);

    return 0;
}
