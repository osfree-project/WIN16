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

#include <user.h>

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
 */
BOOL WINAPI ExitWindows( DWORD dwReturnCode, UINT wReserved )
{
	FUNCTION_START
    return 0;
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


/**********************************************************************
 *		GetAsyncKeyState (USER.249)
 */
int WINAPI GetAsyncKeyState( int key )
{
	FUNCTION_START
    return 0;
}


/**********************************************************************
 *		GetKeyState (USER.106)
 */
int WINAPI GetKeyState(int vkey)
{
	FUNCTION_START
    return 0;//GetKeyState(vkey);
}


/***********************************************************************
 *           MessageBeep    (USER.104)
 */
VOID WINAPI MessageBeep(UINT i)
{
}
