/*

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<https://www.gnu.org/licenses/>.

*/

#include "user.h"
#include "dce.ih"

BOOL WINAPI
GrayString(HDC hDC, HBRUSH hBr, GRAYSTRINGPROC gsprc, LPARAM lParam,
		int cch, int x, int y, int cx, int cy)
{
    COLORREF	rgbSystemGray;
    COLORREF	crTextOld;
    BOOL	bRet;
//    HDC32	hDC32;

	FUNCTION_START

//    APISTR((LF_APICALL,
//	"GrayString(HDC=%x,HBRUSH=%x,GRAYSTRINGPROC=%x,LPARAM=%x,int=%x,int=%x,int=%x,int=%x,int=%x)\n",
//	hDC,hBr,gsprc,lParam,cch,x,y,cx,cy));

//    ASSERT_HDC(hDC32,hDC,FALSE);

#ifdef LATER
    if (gsprc) {
//	APISTR((LF_APIFAIL,"GrayString: returns BOOL FAIL\n"));
	return FALSE;
    }
#endif

    rgbSystemGray = GETSYSCOLOR(COLOR_GRAYTEXT);
    crTextOld = SetTextColor(hDC,rgbSystemGray);

    bRet = TextOut(hDC,x,y,(LPCSTR)lParam,
	(cch)?cch:lstrlen((LPSTR)lParam));

    SetTextColor(hDC,crTextOld);

//    APISTR((LF_APIRET,"GrayString: returns BOOL %d\n",bRet));
    return bRet;
}

/***********************************************************************
 *		ExitWindows (USER.7)
 *@todo refer to pietrek for better implementation
 */
BOOL WINAPI ExitWindows( DWORD dwReturnCode, UINT wReserved )
{
    HWND hwndDesktop;
    WND *wndPtr;
    HWND *list, *pWnd;
    int count, i;
    BOOL result;
        
	FUNCTION_START

    //api_assert("ExitWindows", wReserved == 0);
    //api_assert("ExitWindows", HIWORD(dwReturnCode) == 0);

    /* We have to build a list of all windows first, as in EnumWindows */

    /* First count the windows */

    hwndDesktop = GetDesktopWindow();
    count = 0;
    for (wndPtr = WIN_GetDesktop()->child; wndPtr; wndPtr = wndPtr->next)
        count++;
    if (!count) /* No windows, we can exit at once */
	{
		Death(((DCE *)LocalLock(firstDCE))->hDC);//Switch back to text mode
		ExitKernel(wReserved, LOWORD(dwReturnCode));//EXEC_ExitWindows( LOWORD(dwReturnCode) );
	}

      /* Now build the list of all windows */

    if (!(pWnd = list = (HWND *)LocalAlloc(LMEM_FIXED, sizeof(HWND) * count ))) return FALSE;
    for (wndPtr = WIN_GetDesktop()->child; wndPtr; wndPtr = wndPtr->next)
        *pWnd++ = wndPtr->hwndSelf;

      /* Now send a WM_QUERYENDSESSION message to every window */

    for (pWnd = list, i = 0; i < count; i++, pWnd++)
    {
          /* Make sure that window still exists */
        if (!IsWindow(*pWnd)) continue;
	if (!SendMessage( *pWnd, WM_QUERYENDSESSION, 0, 0 )) break;
    }
    result = (i == count);

    /* Now notify all windows that got a WM_QUERYENDSESSION of the result */

    for (pWnd = list; i > 0; i--, pWnd++)
    {
	if (!IsWindow(*pWnd)) continue;
	SendMessage( *pWnd, WM_ENDSESSION, result, 0 );
    }
    LocalFree((UINT) list );

    if (result)
	{
		Death(((DCE *)LocalLock(firstDCE))->hDC);//Switch back to text mode
		ExitKernel(wReserved, LOWORD(dwReturnCode));//EXEC_ExitWindows( LOWORD(dwReturnCode) );
	}
    return FALSE;
}

/***********************************************************************
 *		DisableOEMLayer (USER.4)
 */
void FAR PASCAL DisableOEMLayer()
{
	FUNCTION_START
}

/***********************************************************************
 *		FinalUserInit (USER.400)
 */
void WINAPI FinalUserInit( void )
{
	FUNCTION_START
    /* FIXME: Should chain to FinalGdiInit */
	FUNCTION_END
}


/***********************************************************************
 *		SignalProc (USER.314)
 */
void WINAPI SignalProc( HANDLE hModule, UINT code,
                          UINT uExitFn, HINSTANCE hInstance, UINT /*HQUEUE*/ hQueue )
{
	FUNCTION_START
#if 0
    if (code == USIG16_DLL_UNLOAD)
    {
        hModule = GetExePtr(hModule);
        /* HOOK_FreeModuleHooks( hModule ); */
        free_module_classes( hModule );
        free_module_icons( hModule );
    }
#endif
}

/***********************************************************************
 *           MessageBeep    (USER.104)
 */
VOID WINAPI MessageBeep(UINT i)
{
}
