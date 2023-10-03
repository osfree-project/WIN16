#include <windows.h>

BOOL WINAPI
GrayString(HDC hDC, HBRUSH hBr, GRAYSTRINGPROC gsprc, LPARAM lParam,
		int cch, int x, int y, int cx, int cy)
{
    COLORREF	rgbSystemGray;
    COLORREF	crTextOld;
    BOOL	bRet;
//    HDC32	hDC32;

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

    rgbSystemGray = GetSysColor(COLOR_GRAYTEXT);
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
    return 0;
}

/***********************************************************************
 *		DisableOEMLayer (USER.4)
 */
void FAR PASCAL DisableOEMLayer()
{
}

/***********************************************************************
 *		FinalUserInit (USER.400)
 */
void WINAPI FinalUserInit( void )
{
    /* FIXME: Should chain to FinalGdiInit */
}

/***********************************************************************
 *		PostMessage  (USER.110)
 */
BOOL WINAPI PostMessage( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    return 0;
}

/***********************************************************************
 *		SignalProc (USER.314)
 */
void WINAPI SignalProc( HANDLE hModule, UINT code,
                          UINT uExitFn, HINSTANCE hInstance, UINT /*HQUEUE*/ hQueue )
{
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


void WINAPI StringFunc(void)
{
}

