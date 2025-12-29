#include <user.h>

void WINAPI PaintRect( HWND hwndParent, HWND hwnd, HDC hdc,
                         HBRUSH hbrush, const RECT far *rect);

/**************************************************************************
 *              FillWindow   (USER.324)
 */
void WINAPI FillWindow( HWND hwndParent, HWND hwnd, HDC hdc, HBRUSH hbrush )
{
    RECT rect;
	FUNCTION_START
    GetClientRect( hwnd, &rect );
    DPtoLP( hdc, (LPPOINT)&rect, 2 );
    PaintRect( hwndParent, hwnd, hdc, hbrush, &rect );
}

/***********************************************************************
 *		CreateWindow (USER.41)
 */
HWND WINAPI CreateWindow( LPCSTR className, LPCSTR windowName,
                              DWORD style, int x, int y, int width,
                              int height, HWND parent, HMENU menu,
                              HINSTANCE instance, LPVOID data )
{
	FUNCTION_START
    return CreateWindowEx( 0, className, windowName, style,
                             x, y, width, height, parent, menu, instance, data );
}

/**************************************************************************
 *              GetWindowText   (USER.36)
 */
int WINAPI GetWindowText( HWND hwnd, LPSTR lpString, int nMaxCount )
{
	FUNCTION_START
    return SendMessage( hwnd, WM_GETTEXT, nMaxCount, (LPARAM)lpString );
}


/**************************************************************************
 *              SetWindowText   (USER.37)
 */
VOID WINAPI SetWindowText( HWND hwnd, LPCSTR lpString )
{
	FUNCTION_START
    SendMessage( hwnd, WM_SETTEXT, 0, (LPARAM)lpString );
}


/**************************************************************************
 *              GetWindowTextLength   (USER.38)
 */
int WINAPI GetWindowTextLength( HWND hwnd )
{
	FUNCTION_START
    return SendMessage( hwnd, WM_GETTEXTLENGTH, 0, 0 );
}

/**************************************************************************
 *              UpdateWindow   (USER.124)
 */
void WINAPI UpdateWindow( HWND hwnd )
{
	FUNCTION_START
    RedrawWindow( hwnd, NULL, 0, RDW_UPDATENOW | RDW_ALLCHILDREN );
}


/**************************************************************************
 *              InvalidateRect   (USER.125)
 */
void WINAPI InvalidateRect( HWND hwnd, const RECT far *rect, BOOL erase )
{
	FUNCTION_START
    RedrawWindow( hwnd, rect, 0, RDW_INVALIDATE | (erase ? RDW_ERASE : 0) );
}


/**************************************************************************
 *              InvalidateRgn   (USER.126)
 */
void WINAPI InvalidateRgn( HWND hwnd, HRGN hrgn, BOOL erase )
{
	FUNCTION_START
    RedrawWindow( hwnd, NULL, hrgn, RDW_INVALIDATE | (erase ? RDW_ERASE : 0) );
}


/**************************************************************************
 *              ValidateRect   (USER.127)
 */
void WINAPI ValidateRect( HWND hwnd, const RECT far *rect )
{
	FUNCTION_START
    RedrawWindow( hwnd, rect, 0, RDW_VALIDATE | RDW_NOCHILDREN );
}


/**************************************************************************
 *              ValidateRgn   (USER.128)
 */
void WINAPI ValidateRgn( HWND hwnd, HRGN hrgn )
{
	FUNCTION_START
    RedrawWindow( hwnd, NULL, hrgn, RDW_VALIDATE | RDW_NOCHILDREN );
}

/**************************************************************************
 *              MessageBox   (USER.1)
 */
int WINAPI MessageBox( HWND hwnd, LPCSTR text, LPCSTR title, UINT type )
{
	FUNCTION_START
    return 0;
}

/**************************************************************************
 *              SysErrorBox   (USER.320)
 */
int FAR PASCAL SysErrorBox(LPSTR lpszMsg,        LPSTR lpszTitle, WORD wButton1, WORD wButton2, WORD wButton3)
{
	FUNCTION_START
	  return 0;
}

/**************************************************************************
 *              GetFocus   (USER.23)
 */
HWND WINAPI GetFocus(void)
{
	FUNCTION_START
    return 0;
}

/**************************************************************************
 *              SetFocus   (USER.22)
 */
HWND WINAPI SetFocus( HWND hwnd )
{
	FUNCTION_START
    return 0;
}

/***********************************************************************
 *		GetWindowTask   (USER.224)
 */
HTASK WINAPI GetWindowTask( HWND hwnd )
{
	FUNCTION_START
//    DWORD tid = GetWindowThreadProcessId( HWND_32(hwnd), NULL );
    //if (!tid) return 0;
    //return HTASK_16(tid);
	return 0;
}

/**************************************************************************
 *              IsWindow   (USER.47)
 */
BOOL WINAPI IsWindow( HWND hwnd )
{
	FUNCTION_START
    //CURRENT_STACK16->es = USER_HeapSel;
    /* don't use WIN_Handle32 here, we don't care about the full handle */
//    return IsWindow( HWND_32(hwnd) );
  return 0;
}

/**************************************************************************
 *              GetWindowWord   (USER.133)
 */
WORD WINAPI GetWindowWord( HWND hwnd, int offset )
{
	FUNCTION_START
	return 0;
}

/**************************************************************************
 *              SetWindowWord   (USER.134)
 */
WORD WINAPI SetWindowWord( HWND hwnd, int offset, WORD newval )
{
	FUNCTION_START
	return 0;
}

/***********************************************************************
 *		CreateWindowEx (USER.452)
 */
HWND WINAPI CreateWindowEx( DWORD exStyle, LPCSTR className,
                                LPCSTR windowName, DWORD style, int x,
                                int y, int width, int height,
                                HWND parent, HMENU menu,
                                HINSTANCE instance, LPVOID data )
{
	FUNCTION_START
	return 0;
	#if 0
    CREATESTRUCTA cs;
    char buffer[256];
    HWND hwnd;

    /* Fix the coordinates */

    cs.x  = (x == CW_USEDEFAULT16) ? CW_USEDEFAULT : (INT)x;
    cs.y  = (y == CW_USEDEFAULT16) ? CW_USEDEFAULT : (INT)y;
    cs.cx = (width == CW_USEDEFAULT16) ? CW_USEDEFAULT : (INT)width;
    cs.cy = (height == CW_USEDEFAULT16) ? CW_USEDEFAULT : (INT)height;

    /* Create the window */

    cs.lpCreateParams = data;
    cs.hInstance      = HINSTANCE_32(instance);
    cs.hMenu          = HMENU_32(menu);
    cs.hwndParent     = WIN_Handle32( parent );
    cs.style          = style;
    cs.lpszName       = windowName;
    cs.lpszClass      = className;
    cs.dwExStyle      = exStyle;

    /* load the menu */
    if (!menu && (style & (WS_CHILD | WS_POPUP)) != WS_CHILD)
    {
        WNDCLASSA class;
        HINSTANCE16 module = GetExePtr( instance );

        if (GetClassInfoA( HINSTANCE_32(module), className, &class ))
            cs.hMenu = HMENU_32( LoadMenu16( module, class.lpszMenuName ));
    }

    if (!IS_INTRESOURCE(className))
    {
        WCHAR bufferW[256];

        if (!MultiByteToWideChar( CP_ACP, 0, className, -1, bufferW, ARRAY_SIZE(bufferW)))
            return 0;
        hwnd = create_window16( (CREATESTRUCTW *)&cs, bufferW, HINSTANCE_32(instance), FALSE );
    }
    else
    {
        if (!GlobalGetAtomNameA( LOWORD(className), buffer, sizeof(buffer) )) return 0;
        cs.lpszClass = buffer;
        hwnd = create_window16( (CREATESTRUCTW *)&cs, (LPCWSTR)className, HINSTANCE_32(instance), FALSE );
    }
    return HWND_16( hwnd );
	#endif
}

/**********************************************************************
 *		GetWindowLong (USER.135)
 */
LONG WINAPI GetWindowLong( HWND hwnd16, int offset )
{
	FUNCTION_START
#if 0
    HWND hwnd = WIN_Handle32( hwnd16 );
    LONG_PTR retvalue;
    BOOL is_winproc = (offset == GWLP_WNDPROC);

    if (offset >= 0)
    {
        int cbWndExtra = GetClassLongA( hwnd, GCL_CBWNDEXTRA );

        if (offset > (int)(cbWndExtra - sizeof(LONG)))
        {
            /*
             * Some programs try to access last element from 16 bit
             * code using illegal offset value. Hopefully this is
             * what those programs really expect.
             */
            if (cbWndExtra >= 4 && offset == cbWndExtra - sizeof(WORD))
            {
                offset = cbWndExtra - sizeof(LONG);
            }
            else
            {
                SetLastError( ERROR_INVALID_INDEX );
                return 0;
            }
        }
        else if (offset == DWLP_DLGPROC)
            is_winproc = (wow_handlers32.get_dialog_info( hwnd, FALSE ) != NULL);
    }
    retvalue = GetWindowLongA( hwnd, offset );
    if (is_winproc) retvalue = (LONG_PTR)WINPROC_GetProc16( (WNDPROC)retvalue, FALSE );
    return retvalue;
#endif
	return 0;
}

/**********************************************************************
 *		SetWindowLong (USER.136)
 */
LONG WINAPI SetWindowLong( HWND hwnd16, int offset, LONG newval )
{
	FUNCTION_START
	#if 0
    HWND hwnd = WIN_Handle32( hwnd16 );
    BOOL is_winproc = (offset == GWLP_WNDPROC);

    if (offset == DWLP_DLGPROC)
        is_winproc = (wow_handlers32.get_dialog_info( hwnd, FALSE ) != NULL);

    if (is_winproc)
    {
        WNDPROC new_proc = WINPROC_AllocProc16( (WNDPROC16)newval );
        WNDPROC old_proc = (WNDPROC)SetWindowLongPtrA( hwnd, offset, (LONG_PTR)new_proc );
        return (LONG)WINPROC_GetProc16( old_proc, FALSE );
    }
    else return SetWindowLongA( hwnd, offset, newval );
	#endif
	return 0;
}

/**************************************************************************
 *              GetClientRect   (USER.33)
 */
void WINAPI GetClientRect( HWND hwnd, LPRECT rect )
{
	FUNCTION_START
	#if 0
    RECT rect32;

    GetClientRect( WIN_Handle32(hwnd), &rect32 );
    rect->left   = rect32.left;
    rect->top    = rect32.top;
    rect->right  = rect32.right;
    rect->bottom = rect32.bottom;
	#endif
}

/**************************************************************************
 *              GetDC   (USER.66)
 */
HDC WINAPI GetDC( HWND hwnd )
{
	FUNCTION_START
    return 0;//HDC_16(GetDC( WIN_Handle32(hwnd) ));
}

/**************************************************************************
 *              ReleaseDC   (USER.68)
 */
int WINAPI ReleaseDC( HWND hwnd, HDC hdc )
{
	FUNCTION_START
    //INT16 ret = (INT16)ReleaseDC( WIN_Handle32(hwnd), HDC_32(hdc) );
    //NtUserEnableDC( HDC_32(hdc) );
    return 0;//ret;
}

/**************************************************************************
 *              EnableScrollBar   (USER.482)
 */
BOOL WINAPI EnableScrollBar( HWND hwnd, int nBar, UINT flags )
{
	FUNCTION_START
    return 0;//EnableScrollBar( WIN_Handle32(hwnd), nBar, flags );
}

/**************************************************************************
 *              RedrawWindow   (USER.290)
 */
BOOL WINAPI RedrawWindow( HWND hwnd, const RECT *rectUpdate,
                              HRGN hrgnUpdate, UINT flags )
{
	FUNCTION_START
    if (rectUpdate)
    {
        RECT r;
        r.left   = rectUpdate->left;
        r.top    = rectUpdate->top;
        r.right  = rectUpdate->right;
        r.bottom = rectUpdate->bottom;
        return 0;//RedrawWindow(WIN_Handle32(hwnd), &r, HRGN_32(hrgnUpdate), flags);
    }
    return 0;//RedrawWindow(WIN_Handle32(hwnd), NULL, HRGN_32(hrgnUpdate), flags);
}

/***********************************************************************
 *		DefWindowProc (USER.107)
 */
LRESULT WINAPI DefWindowProc( HWND hwnd16, UINT msg, WPARAM wParam, LPARAM lParam )
{
	FUNCTION_START
    //LRESULT result;
//    WINPROC_CallProc16To32A( defwnd_proc_callback, hwnd16, msg, wParam, lParam, &result, 0 );
    return 0;//result;
}

/**************************************************************************
 *              GetDlgCtrlID   (USER.277)
 */
int WINAPI GetDlgCtrlID( HWND hwnd )
{
	FUNCTION_START
    return 0;//GetDlgCtrlID( WIN_Handle32(hwnd) );
}

/*******************************************************************
 *		MapWindowPoints (USER.258)
 */
void WINAPI MapWindowPoints( HWND hwndFrom, HWND hwndTo, LPPOINT lppt, UINT count )
{
	FUNCTION_START
}

/**************************************************************************
 *              ReleaseCapture   (USER.19)
 * Defined as VOID in Watcom headers
 */
VOID /*BOOL*/ WINAPI ReleaseCapture(void)
{
	FUNCTION_START
    //return FALSE;//ReleaseCapture();
}

/**************************************************************************
 *              DestroyWindow   (USER.53)
 */
BOOL WINAPI DestroyWindow( HWND hwnd )
{
	FUNCTION_START
    return FALSE;//DestroyWindow( WIN_Handle32(hwnd) );
}

/**************************************************************************
 *              GetWindowRect   (USER.32)
 */
void WINAPI GetWindowRect( HWND hwnd, LPRECT rect )
{
	FUNCTION_START
}

/**************************************************************************
 *              SetScrollRange   (USER.64)
 */
void WINAPI SetScrollRange( HWND hwnd, int nBar, int MinVal, int MaxVal, BOOL redraw )
{
	FUNCTION_START
    /* Invalid range -> range is set to (0,0) */
//    if (MaxVal - MinVal > 0x7fff) MinVal = MaxVal = 0;
//    SetScrollRange( WIN_Handle32(hwnd), nBar, MinVal, MaxVal, redraw );
}

/**************************************************************************
 *              SetScrollPos   (USER.62)
 */
int WINAPI SetScrollPos( HWND hwnd, int nBar, int nPos, BOOL redraw )
{
	FUNCTION_START
    return 0;//SetScrollPos( WIN_Handle32(hwnd), nBar, nPos, redraw );
}


/**************************************************************************
 *              GetScrollPos   (USER.63)
 */
int WINAPI GetScrollPos( HWND hwnd, int nBar )
{
	FUNCTION_START
    return 0;//GetScrollPos( WIN_Handle32(hwnd), nBar );
}

/**************************************************************************
 *              ShowScrollBar   (USER.267)
 */
void WINAPI ShowScrollBar( HWND hwnd, int nBar, BOOL fShow )
{
	FUNCTION_START
    
}

/**************************************************************************
 *              IsWindowVisible   (USER.49)
 */
BOOL WINAPI IsWindowVisible( HWND hwnd )
{
	FUNCTION_START
    return FALSE;
}

/**************************************************************************
 *              GetScrollRange   (USER.65)
 * VOID in watcom headers
 */
VOID /*BOOL*/ WINAPI GetScrollRange( HWND hwnd, int nBar, LPINT lpMin, LPINT lpMax)
{
	FUNCTION_START
	//return FALSE;
}

/**************************************************************************
 *              ScrollWindow   (USER.61)
 */
void WINAPI ScrollWindow( HWND hwnd, int dx, int dy, const RECT *rect,
                            const RECT *clipRect )
{
	FUNCTION_START
}

/**************************************************************************
 *              GetParent   (USER.46)
 */
HWND WINAPI GetParent( HWND hwnd )
{
	FUNCTION_START
    return 0;
}

/**************************************************************************
 *              SetCapture   (USER.18)
 */
HWND WINAPI SetCapture( HWND hwnd )
{
	FUNCTION_START
    return 0;
}

/**************************************************************************
 *              SetWindowPos   (USER.232)
 */
BOOL WINAPI SetWindowPos( HWND hwnd, HWND hwndInsertAfter,
                              int x, int y, int cx, int cy, UINT flags)
{
	FUNCTION_START
    return FALSE;
}

/***********************************************************************
 *		BeginPaint (USER.39)
 */
HDC WINAPI BeginPaint( HWND hwnd, LPPAINTSTRUCT lps )
{
	FUNCTION_START
    return 0;
}

/***********************************************************************
 *		EndPaint (USER.40)
 * VOID in watcom headers
 */
VOID /*BOOL*/ WINAPI EndPaint( HWND hwnd, const PAINTSTRUCT FAR *lps )
{
	FUNCTION_START
    //return FALSE;
}

/**************************************************************************
 *              ScreenToClient   (USER.29)
 */
void WINAPI ScreenToClient( HWND hwnd, LPPOINT lppnt )
{
	FUNCTION_START
    MapWindowPoints( 0, hwnd, lppnt, 1 );
}
