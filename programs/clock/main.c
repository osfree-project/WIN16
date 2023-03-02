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

void CLOCK_SaveConfiguration(void)
{
	char buffer[100];
    //RECT rect;
    WritePrivateProfileString("Clock", "Maximized",
                              Globals.bMaximized ? "1" : "0",
                              Globals.lpszIniFile);

    wsprintf(buffer, "%d,%d,%d,%d,%d,%d", Globals.bAnalog, Globals.bMinimized, !Globals.bSeconds, Globals.bWithoutTitle, Globals.bAlwaysOnTop, !Globals.bDate);
    WritePrivateProfileString("Clock", "Options", buffer, Globals.lpszIniFile);

    WritePrivateProfileString("Clock", "sFont",
                              Globals.logfont.lfFaceName,
                              Globals.lpszIniFile);

//    GetWindowRect(Globals.hMainWnd, &rect);

    wsprintf(buffer, "%d,%d,%d,%d", Globals.x, Globals.y, Globals.x+Globals.MaxX, Globals.y+Globals.MaxY);
    WritePrivateProfileString("Clock", "Position", buffer, Globals.lpszIniFile);
}

void CLOCK_ReadConfiguration(void)
{
    int  right, bottom;
	char buffer[100];
	DWORD dwVersion;
	BYTE bMajorVersion;
	BYTE bMinorVersion;

    /* Read Options from `win.ini' */
	GetProfileString("intl", "s1159", "AM", Globals.s1159, sizeof(Globals.s1159));
	GetProfileString("intl", "s2359", "PM", Globals.s2359, sizeof(Globals.s2359));
	GetProfileString("intl", "sTime", ":", Globals.sTime, sizeof(Globals.sTime));
    Globals.iTime=GetProfileInt("intl", "iTime", 0);
    Globals.iTLZero=GetProfileInt("intl", "iTLZero", 0);
	GetProfileString("intl", "sDate", "/", Globals.sDate, sizeof(Globals.sDate));
	GetProfileString("intl", "sShortDate", "MM/dd/yy", Globals.sShortDate, sizeof(Globals.sShortDate));

    /* Read Options from `clock.ini' */
    Globals.bMaximized = GetPrivateProfileInt("Clock", "Maximized", FALSE, Globals.lpszIniFile);

    // Get the Windows version.
    dwVersion = GetVersion();
 
    bMajorVersion = (LOBYTE(LOWORD(dwVersion)));
    bMinorVersion = (HIBYTE(LOWORD(dwVersion)));
	
    Globals.bWin30Style = GetPrivateProfileInt("Clock", "Win30Style", (bMajorVersion==3)&&(bMinorVersion==0), Globals.lpszIniFile);
    GetPrivateProfileString("Clock", "sFont", "", Globals.logfont.lfFaceName, sizeof(Globals.logfont.lfFaceName), Globals.lpszIniFile);
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

    GetPrivateProfileString("Clock", "Position", "", buffer, sizeof(buffer), Globals.lpszIniFile);
    if (4 == sscanf(buffer, "%d,%d,%d,%d", &(Globals.x), &(Globals.y), &right, &bottom))
    {
      Globals.MaxX = right - Globals.x;
      Globals.MaxY = bottom - Globals.y;
    }
    else
    {
      Globals.x = Globals.y = CW_USEDEFAULT;
      Globals.MaxX = Globals.MaxY = INITIAL_WINDOW_SIZE;
//      Globals.MaxY +=GetSystemMetrics(SM_CYCAPTION);
    }

}

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
    char szCaption[MAX_STRING_LEN]="";
    char szTmp[MAX_STRING_LEN]="";

    LoadString(Globals.hInstance, IDS_CLOCK, szCaption, MAX_STRING_LEN);

    /* Set frame caption */
    if (Globals.bDate && Globals.bAnalog) {
/*	chars = GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, NULL, NULL,
                               szCaption, ARRAY_SIZE(szCaption));*/
		strcat(szCaption, " - ");
		FormatDate(szTmp, !IsIconic(Globals.hMainWnd));
		strcat(szCaption, szTmp);
	}

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
    newfont = SizeFont(dc, Globals.MaxX*0.85, Globals.MaxY*0.6, Globals.bSeconds, &Globals.logfont);
    if (newfont) {
		DeleteObject(Globals.hFont);
		Globals.hFont = newfont;
    }

	newfont = SizeFont(dc, Globals.MaxX*0.85*0.8, Globals.MaxY*0.6*0.8, Globals.bSeconds, &Globals.logfont);
    if (newfont) {
		DeleteObject(Globals.hDateFont);
		Globals.hDateFont = newfont;
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
			RECT rect;
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
			CLOCK_UpdateWindowCaption();
			CLOCK_ResetTimer();
			InvalidateRect(Globals.hMainWnd, NULL, FALSE);
            break;
        }
            /* switch to digital */
        case IDM_DIGITAL: {
            Globals.bAnalog = FALSE;
            CLOCK_UpdateMenuCheckmarks();
			CLOCK_UpdateWindowCaption();
			CLOCK_ResetTimer();
			CLOCK_ResetFont();
			InvalidateRect(Globals.hMainWnd, NULL, FALSE);
            break;
        }
            /* change font */
        case IDM_FONT: {
            CLOCK_ChooseFont();
			InvalidateRect(Globals.hMainWnd, NULL, FALSE);
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
			InvalidateRect(Globals.hMainWnd, NULL, FALSE);
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
    HBRUSH hBrush, oldhBrush;
    HPEN oldhPen, hBlackPen, hRedPen;
	RECT rc;

    dc = BeginPaint(hWnd, &ps);

    GetClientRect(hWnd, &rc); 
	if (IsIconic(hWnd)) 
    { 
        SetWindowExt(dc, 100, 100); 
        SetViewportExt(dc, rc.right, rc.bottom); 
		
		hBrush=CreateSolidBrush(RGB(0xC0,0xC0,0xC0));
		oldhBrush=SelectObject(dc, hBrush);
		
		hBlackPen=CreatePen(PS_SOLID, 1, RGB(0x0,0,0x0));
		hRedPen=CreatePen(PS_SOLID, 1, RGB(0xff,0,0x11));
		
		oldhPen=SelectObject(dc, hBlackPen);
		Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
		SelectObject(dc, hRedPen);
		Rectangle(dc, rc.left+1, rc.top+1, rc.right-1, rc.bottom-1);
		SelectObject(dc, hBlackPen);
		Rectangle(dc, rc.left+2, rc.top+2, rc.right-2, rc.bottom-2);
		SetPixel(dc, rc.left, rc.top, RGB(0xC0,0xC0,0xC0));
		SetPixel(dc, rc.right-1, rc.top, RGB(0xC0,0xC0,0xC0));
		SetPixel(dc, rc.left, rc.bottom-1, RGB(0xC0,0xC0,0xC0));
		SetPixel(dc, rc.right-1, rc.bottom-1, RGB(0xC0,0xC0,0xC0));
		
		SelectObject(dc, oldhPen);
		DeleteObject(hBlackPen);
		DeleteObject(hRedPen);
		
		SelectObject(dc, oldhBrush);
		DeleteObject(hBrush);
		if(Globals.bAnalog)
		{
			IconAnalogClock(dc, rc.right-rc.left, rc.bottom-rc.top);
		}
		else {
			IconDigitalClock(dc, rc.right-rc.left, rc.bottom-rc.top);
		}
    } 
    else 
	{	
    /* Use an offscreen dc to avoid flicker */
    dcMem = CreateCompatibleDC(dc);
    bmMem = CreateCompatibleBitmap(dc, rc.right - rc.left,
				    rc.bottom - rc.top);

    bmOld = SelectObject(dcMem, bmMem);

    SetViewportOrgEx(dcMem, -rc.left, -rc.top, NULL);

    hBrush=CreateSolidBrush(BackgroundColor);
    /* Erase the background */
    FillRect(dcMem, &rc,  hBrush);
    DeleteObject(hBrush);

    if(Globals.bAnalog)
		AnalogClock(dcMem, rc.right-rc.left, rc.bottom-rc.top, Globals.bSeconds);
    else
		DigitalClock(dcMem, rc.right-rc.left, rc.bottom-rc.top, Globals.bSeconds);

    /* Blit the changes to the screen */
    BitBlt(dc, 
	   rc.left, rc.top,
	   rc.right - rc.left, rc.bottom - rc.top,
           dcMem,
	   rc.left, rc.top,
           SRCCOPY);

    SelectObject(dcMem, bmOld);
    DeleteObject(bmMem);
    DeleteDC(dcMem);
	}
	
    EndPaint(hWnd, &ps);
}

/***********************************************************************
 *
 *           CLOCK_WndProc
 */

static LRESULT WINAPI CLOCK_WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
	/* L button drag moves the window if no title mode */
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

        case WM_MOVE: {
			RECT rect;
			if (!IsIconic(hWnd))
			{
				GetWindowRect(Globals.hMainWnd, &rect);
				Globals.x = rect.left;
				Globals.y = rect.top;
				Globals.MaxX = rect.right-rect.left;
				Globals.MaxY = rect.bottom-rect.top;
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		
        case WM_SIZE: {
			RECT rect;
			
			if (!IsIconic(hWnd))
			{
				GetWindowRect(Globals.hMainWnd, &rect);
				Globals.x = rect.left;
				Globals.y = rect.top;
				Globals.MaxX = rect.right-rect.left;
				Globals.MaxY = rect.bottom-rect.top;
			}
			
			if (wParam==SIZE_MINIMIZED)
			{
				Globals.bMaximized=FALSE;
				Globals.bMinimized=TRUE;
			} else
			if (wParam==SIZE_MAXIMIZED)
			{
				Globals.bMaximized=TRUE;
				Globals.bMinimized=FALSE;
			} else
			if (wParam==SIZE_RESTORED)
			{
				Globals.bMaximized=FALSE;
				Globals.bMinimized=FALSE;
			}
			
			CLOCK_ResetFont();
			CLOCK_UpdateWindowCaption();
            return 0;//DefWindowProc(hWnd, msg, wParam, lParam);
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
				CLOCK_SaveConfiguration();
				PostQuitMessage (0);
            break;
        }

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

BOOL CLOCK_RegisterMainWinClass(void)
{
	WNDCLASS class;
	
	class.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	class.lpfnWndProc   = CLOCK_WndProc;
	class.cbClsExtra    = 0;
	class.cbWndExtra    = 0;
	class.hInstance     = Globals.hInstance;
	class.hIcon         = 0;//LoadIcon(0, (LPCSTR)IDI_APPLICATION);
	class.hCursor       = LoadCursor(0, (LPCSTR)IDC_ARROW);
	class.hbrBackground = 0;
	class.lpszMenuName  = 0;
	class.lpszClassName = "CLClass";

    return RegisterClass(&class);
}


/***********************************************************************
 *
 *           WinMain
 */

int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE prev, LPSTR cmdline, int show)
{
    MSG      msg;
    LONG style = WS_OVERLAPPEDWINDOW;

    /* Setup Globals */
    memset(&Globals, 0, sizeof (Globals));
    Globals.hInstance       = hInstance;
    Globals.lpszIniFile     = "clock.ini";

	/* Read application configuration */
	CLOCK_ReadConfiguration();
	
	if (!prev)
	{
		if (!CLOCK_RegisterMainWinClass()) return(FALSE);
	}

	if (Globals.bMaximized) show=SW_SHOWMAXIMIZED;
	if (Globals.bMinimized) show=SW_SHOWMINIMIZED;

    Globals.hMainWnd = CreateWindow("CLClass", "Clock", style/*WS_OVERLAPPEDWINDOW*/,
                                     Globals.x, Globals.y,
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

	if (Globals.bWin30Style)
	{
		Globals.hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(MAIN3_MENU));
	} else {
		Globals.hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(MAIN_MENU));
	}
	
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
