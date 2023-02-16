/*
 * Clock (main.h)
 *
 * Copyright 1998 Marcel Baur <mbaur@g26.ethz.ch>
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

#include "colors.h"
#include "clock_res.h"

#define MAX_STRING_LEN      255
#define DEFAULTICON OIC_WINLOGO

typedef struct
{
  LOGFONT logfont;
  HFONT   hFont;	// Font for time in digital clock mode
  HFONT   hDateFont;	// Font for date in digital clock mode
  HANDLE  hInstance;
  HWND    hMainWnd;
  HMENU   hMainMenu;
  HMENU   hSysMenu;

  LPSTR   lpszIniFile;	// INI filename

  // Clock configuration
  BOOL    bAnalog;			// Show analog clock
  BOOL    bAlwaysOnTop;		// Always on tom
  BOOL    bWithoutTitle;	// Without title
  BOOL    bSeconds;			// Show seconds
  BOOL    bDate;			// Show date
  BOOL    bMaximized;		// Maximize window
  BOOL    bMinimized;		// Minimize window
  BOOL    bWin30Style;		// Use Windows 3.0 style (red border, white background, black ticks)
  
  // Window position
  int     x;
  int     y;
  int     MaxX;
  int     MaxY;
  
  // Time settings
  char    s1159[10];
  char    s2359[10];
  char    sTime[5];
  int     iTime;
  int     iTLZero;
  
  // Date settings
  char    sShortDate[20];
  char    sDate[3];

} CLOCK_GLOBALS;

extern CLOCK_GLOBALS Globals;

int WINAPI ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText,
                HICON hIcon);


