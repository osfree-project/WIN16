/*
    
	DragDrop.c	1.4
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

#include "shellapi.h"

#define __SHELLAPI_H__
//#include "Willows.h"

//#include "Log.h"
#include "DragDrop.h"

/******************************************************************************
 * DragAcceptFiles   [SHELL.9]
 */
void WINAPI DragAcceptFiles(HWND hWnd, BOOL fAccept)
{
    DWORD dwExStyle;

	MessageBox(0, "DragAcceptFiles", "DragAcceptFiles", MB_OK);
//    APISTR((LF_APICALL,"DragAcceptFiles(HWND=%x,BOOL=%d)\n",
//	hWnd,fAccept));

    if (!IsWindow(hWnd)) {
//        APISTR((LF_APIRET,"DragAcceptFiles: returns void\n"));
	return;
    }

    dwExStyle = GetWindowLong(hWnd,GWL_EXSTYLE);

    if (fAccept)
	dwExStyle |= WS_EX_ACCEPTFILES;
    else
	dwExStyle &= ~WS_EX_ACCEPTFILES;

    SetWindowLong(hWnd,GWL_EXSTYLE,dwExStyle);
//    APISTR((LF_APIRET,"DragAcceptFiles: returns void\n"));
}



/*************************************************************************
 * DragQueryFile		[SHELL.11]
 */
UINT WINAPI DragQueryFile(
	HDROP hDrop,
	UINT wFile,
	LPSTR lpszFile,
	UINT wLength)
{
 	LPSTR lpDrop;
	UINT i = 0;
	LPDROPFILESTRUCT lpDropFileStruct = (LPDROPFILESTRUCT) GlobalLock(hDrop);

	MessageBox(0, "DragQueryFile", "DragQueryFile", MB_OK);
//	TRACE("(%04x, %x, %p, %u)\n", hDrop,wFile,lpszFile,wLength);

	if(!lpDropFileStruct) goto end;

	lpDrop = (LPSTR) lpDropFileStruct + lpDropFileStruct->wSize;

	while (i++ < wFile)
	{
	  while (*lpDrop++); /* skip filename */
	  if (!*lpDrop)
	  {
	    i = (wFile == 0xFFFF) ? i : 0;
	    goto end;
	  }
	}

	i = lstrlen(lpDrop);
	if (!lpszFile ) goto end;   /* needed buffer size */
	lstrcpyn (lpszFile, lpDrop, wLength);
end:
	GlobalUnlock(hDrop);
	return i;
}



/*************************************************************************
 * DragQueryPoint		[SHELL.13]
 */
BOOL WINAPI DragQueryPoint(HDROP hDrop, LPPOINT lppt)
{
    LPDROPFILESTRUCT lpDragInfo;
    BOOL bRet;

	MessageBox(0, "DragQueryPoint", "DragQueryPoint", MB_OK);
//    APISTR((LF_APICALL,"DragQueryPoint(HDROP=%x,POINT=%x)\n",
//	hDrop,lppt));

    if (NULL == (lpDragInfo = (LPDROPFILESTRUCT)GlobalLock(hDrop))) {
//        APISTR((LF_APICALL,"DragQueryPoint: returns BOOL FALSE\n"));
	return FALSE;
    }

    *lppt = lpDragInfo->ptMousePos;
    bRet = !lpDragInfo->fInNonClientArea;

    GlobalUnlock(hDrop);

//    APISTR((LF_APICALL,"DragQueryPoint: returns BOOL TRUE\n"));
    return bRet;
}

/*************************************************************************
 * DragFinish		[SHELL.12]
 */
void WINAPI DragFinish(HDROP hDrop)
{

//    APISTR((LF_APICALL,"DragFinish(HDROP=%x)\n",hDrop));

	MessageBox(0, "DragFinish", "DragFinish", MB_OK);

    GlobalFree(hDrop);

//    APISTR((LF_APIRET,"DragFinish: returns void\n"));
}
