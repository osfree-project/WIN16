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
 *		PostMessage  (USER.110)
 */
BOOL WINAPI PostMessage( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	FUNCTION_START
    return 0;
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
 *		SendMessage  (USER.111)
 */
LRESULT WINAPI SendMessage( HWND hwnd16, UINT msg, WPARAM wparam, LPARAM lparam )
{
	FUNCTION_START
	#if 0
    LRESULT result;
    HWND hwnd = WIN_Handle32( hwnd16 );

    if (hwnd != HWND_BROADCAST &&
        GetWindowThreadProcessId( hwnd, NULL ) == GetCurrentThreadId())
    {
        /* call 16-bit window proc directly */
        WNDPROC16 winproc;

        /* first the WH_CALLWNDPROC hook */
        call_WH_CALLWNDPROC_hook( hwnd16, msg, wparam, lparam );

        if (!(winproc = (WNDPROC16)GetWindowLong16( hwnd16, GWLP_WNDPROC ))) return 0;

        TRACE_(message)("(0x%04x) [%04x] wp=%04x lp=%08Ix\n", hwnd16, msg, wparam, lparam );
        result = CallWindowProc16( winproc, hwnd16, msg, wparam, lparam );
        TRACE_(message)("(0x%04x) [%04x] wp=%04x lp=%08Ix returned %08Ix\n",
                        hwnd16, msg, wparam, lparam, result );
    }
    else  /* map to 32-bit unicode for inter-thread/process message */
    {
        WINPROC_CallProc16To32A( send_message_callback, hwnd16, msg, wparam, lparam, &result, NULL );
    }
    return result;
#endif
	return 0;
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
 *		IsMenu    (USER.358)
 */
BOOL WINAPI IsMenu( HMENU hmenu )
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
 *		GetMessage  (USER.108)
 */
BOOL WINAPI GetMessage( MSG *msg, HWND hwnd, UINT first, UINT last )
{
	FUNCTION_START
    return 0;//GetMessage32_16( (MSG32_16 *)msg, hwnd, first, last, FALSE );
}

/***********************************************************************
 *		TranslateMessage (USER.113)
 */
BOOL WINAPI TranslateMessage( const MSG *msg )
{
	FUNCTION_START
    return 0;//TranslateMessage32_16( (const MSG32_16 *)msg, FALSE );
}

/***********************************************************************
 *		DispatchMessage (USER.114)
 */
LONG WINAPI DispatchMessage( const MSG* msg )
{
	FUNCTION_START
	#if 0
    WNDPROC winproc;
    LRESULT retval;

      /* Process timer messages */
    if ((msg->message == WM_TIMER) || (msg->message == WM_SYSTIMER))
    {
        if (msg->lParam)
            return CallWindowProc( (WNDPROC)msg->lParam, msg->hwnd,
                                     msg->message, msg->wParam, GetTickCount() );
    }

    if (!(winproc = (WNDPROC)GetWindowLong( msg->hwnd, GWLP_WNDPROC )))
    {
        //SetLastError( ERROR_INVALID_WINDOW_HANDLE );
        return 0;
    }
    //TRACE_(message)("(0x%04x) [%04x] wp=%04x lp=%08Ix\n", msg->hwnd, msg->message, msg->wParam, msg->lParam );
    retval = CallWindowProc( winproc, msg->hwnd, msg->message, msg->wParam, msg->lParam );
    //TRACE_(message)("(0x%04x) [%04x] wp=%04x lp=%08Ix returned %08Ix\n",
                    //msg->hwnd, msg->message, msg->wParam, msg->lParam, retval );
    return retval;
	#endif
	return 0;
}

/***********************************************************************
 *		PeekMessage  (USER.109)
 */
BOOL WINAPI PeekMessage( MSG *msg, HWND hwnd,
                             UINT first, UINT last, UINT flags )
{
	FUNCTION_START
	FUNCTION_END
    return 0;//PeekMessage32_16( (MSG32_16 *)msg, hwnd, first, last, flags, FALSE );
}

void WINAPI
SetDoubleClickTime(UINT uInterval)
{
	FUNCTION_START
	FUNCTION_END
//    APISTR((LF_APICALL,"SetDoubleClickTime(UINT=%x)\n",uInterval));
//    (void)DRVCALL_EVENTS(PEH_DBLCLKTIME,1,uInterval,0);
//    APISTR((LF_APIRET,"SetDoubleClickTime: returns void\n"));
}
