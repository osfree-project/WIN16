/*    
	About.c	2.21
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include "windows.h"
//#include "windowsx.h"
#include "commdlg.h"
//#include "Log.h"
//#include "Dialog.h"
#include "About.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define TWIN_PLATFORM	"DOS"
#define GET_WM_COMMAND_ID(wp, lp)                   (wp)

typedef struct {
	LPSTR	lpszCaption;
	LPSTR	lpszText;
	HICON	hIcon;
} SHELLABOUTDATA;



static BOOL FAR PASCAL ShellAboutHandler(HWND, unsigned, WPARAM, LPARAM);

void WINAPI
ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText,
		HICON hIcon)
{
	FARPROC	lpProc;
	SHELLABOUTDATA sad;
	HINSTANCE  hInst;
	int     rc;
	

	if(hWnd)
//		hInst = GetWindowInstance(hWnd);
		hInst = ((HMODULE)GetWindowWord(hWnd,GWW_HINSTANCE));
	else    hInst = 0;

	lpProc= MakeProcInstance((FARPROC)ShellAboutHandler,hInst);

	sad.lpszText    = (LPSTR)lpszAboutText;
	sad.lpszCaption = (LPSTR) lpszCaption;
	sad.hIcon = hIcon;

	rc = DialogBoxParam(
	        0, 
		"ShellAbout",
		hWnd,
		lpProc,
		(LPARAM)&sad		
		);
	FreeProcInstance(lpProc); 
}

static BOOL FAR PASCAL
ShellAboutHandler(HWND hDlg, unsigned msg, WPARAM wParam, LPARAM lParam)
{
	SHELLABOUTDATA *sad;
	HWND	        hWnd;
	HDC	        hDC;
	char		abouttext[256];
	char		dirname[256];
	DWORD		version;
	int		bpp;

	switch(msg) {
	case WM_INITDIALOG:
		sad = (SHELLABOUTDATA *) lParam;
		
		/************************************************/
		/*	Caption first				*/
		/************************************************/

		sprintf(abouttext,"About %s",sad->lpszCaption);
	    	SetWindowText(hDlg,abouttext);

		/************************************************/
		/*	Default first two lines			*/
		/************************************************/
		SendDlgItemMessage (hDlg, SAB_ABOUT, WM_SETTEXT, 0,
		                    ( LONG )sad->lpszCaption);
		SendDlgItemMessage (hDlg, SAB_TEXT, WM_SETTEXT, 0,
		                    ( LONG )sad->lpszText);

		/************************************************/
		/*	Twin specific 5 lines			*/
		/************************************************/
		GetModuleFileName(0,abouttext,256);

		/* add any shell about specific string */
		SendDlgItemMessage(hDlg, SAB_USER, WM_SETTEXT, 0,
			(LONG) abouttext);

		GetWindowsDirectory(dirname,256);
		sprintf(abouttext,"Windows: %s",dirname);
		SendDlgItemMessage(hDlg, SAB_WINDOW, WM_SETTEXT, 0,
			(LONG) abouttext);

		GetSystemDirectory(dirname,256);
		sprintf(abouttext,"System: %s",dirname);
		SendDlgItemMessage(hDlg, SAB_SYSTEM, WM_SETTEXT, 0,
			(LONG) abouttext);

		/************************************************/
		/*	Host specific 2 lines			*/
		/************************************************/
#ifdef LATER
	Add platform specific string to platform.h
	Use to fill out dialog box...
#endif
		/* host system information */
		sprintf ( abouttext, "Host: %s",TWIN_PLATFORM);
		SendDlgItemMessage(hDlg, SAB_HOST, WM_SETTEXT, 0,
			(LONG)abouttext);

		/* workstation information */
		hDC = GetDC(hDlg);
		bpp = GetDeviceCaps(hDC, BITSPIXEL);
		ReleaseDC(hDlg, hDC);


		/* add the current mode... */
		version = GetVersion();
		sprintf(abouttext,"Version: %s %d.%d build %d) ",
			"Win32",
			LOBYTE(LOWORD(version)), HIBYTE(LOWORD(version)),
			HIWORD(version)
			);
		SendDlgItemMessage(hDlg, SAB_VERSION, WM_SETTEXT, 0,
			(LONG)abouttext);
	
		/************************************************/
		/*	Icon passed in?				*/
		/************************************************/
		if (sad->hIcon) 
	    		SendDlgItemMessage(hDlg, SAB_ICON, STM_SETICON,
				sad->hIcon, 0L);
		 else  {
			hWnd = GetDlgItem(hDlg,SAB_ICON);
			ShowWindow(hWnd,SW_HIDE);
		}

		return TRUE;

	case WM_COMMAND:
		switch ( GET_WM_COMMAND_ID(wParam, lParam) )
		{
			case IDOK:
				EndDialog(hDlg, IDOK);
				break;
			default:
				return ( FALSE );
		}
		return ( TRUE );
	}
	return FALSE;
}
