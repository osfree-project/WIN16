/*
 *  Notepad (notepad.h)
 *
 *  Copyright 1997,98 Marcel Baur <mbaur@g26.ethz.ch>
 *  Copyright 2002 Sylvain Petreolle <spetreolle@yahoo.fr>
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

#define SIZEOF(a) sizeof(a)/sizeof((a)[0])

#include "notepad_res.h"
#include <dos.h>
#define MAX_PATH NAME_MAX
#define MAX_STRING_LEN      255

typedef struct
{
  HANDLE  hInstance;
  HWND    hMainWnd;
  HWND    hFindReplaceDlg;
  HWND    hEdit;
  HFONT   hFont; /* Font used by the edit control */
  LOGFONT lfFont;
  BOOL    bWrapLongLines;
  char   szFindText[MAX_PATH];
  char   szFileName[MAX_PATH];
  char   szFileTitle[MAX_PATH];
  char   szFilter[2 * MAX_STRING_LEN + 100];
  int     iMarginTop;
  int     iMarginBottom;
  int     iMarginLeft;
  int     iMarginRight;
  char   szHeader[MAX_PATH];
  char   szFooter[MAX_PATH];

  //FINDREPLACE find;
  //FINDREPLACE lastFind;
  HGLOBAL hDevMode; /* printer mode */
  HGLOBAL hDevNames; /* printer names */
} NOTEPAD_GLOBALS;

extern NOTEPAD_GLOBALS Globals;

VOID SetFileName(LPCSTR szFileName);
//void NOTEPAD_DoFind(FINDREPLACE *fr);
DWORD get_dpi(void);

DWORD WINAPI GetCurrentDirectory(DWORD cchCurDir, char * lpszCurDir);
