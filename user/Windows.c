#include <user.h>
#include <queue.h>

VOID WIN_UpdateNCArea(WND* wnd, BOOL bUpdate);

VOID WINAPI PaintRect( HWND hwndParent, HWND hwnd, HDC hdc,
                         HBRUSH hbrush, const RECT far *rect);

/**************************************************************************
 *              FillWindow   (USER.324)
 */
void WINAPI FillWindow(HWND hwndParent, HWND hwnd, HDC hdc, HBRUSH hbrush)
{
    RECT rect;
    HBRUSH hBrushRed;
	HPEN hPenBlue;

//	FUNCTION_START

	GetClientRect( hwnd, &rect );
//TRACE("left=%d top=%d right=%d bottom=%d", rect.left, rect.top, rect.right, rect.bottom);
	DPtoLP( hdc, (LPPOINT)&rect, 2 );
//TRACE("left=%d top=%d right=%d bottom=%d", rect.left, rect.top, rect.right, rect.bottom);
	PaintRect( hwndParent, hwnd, hdc, hbrush, &rect );
//	FUNCTION_END
}

/***********************************************************************
 *		CreateWindow (USER.41)
 */
HWND WINAPI CreateWindow( LPCSTR className, LPCSTR windowName,
                              DWORD style, int x, int y, int width,
                              int height, HWND parent, HMENU menu,
                              HINSTANCE instance, LPVOID data )
{
//	FUNCTION_START
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
    RedrawWindow( hwnd, NULL, 0, RDW_UPDATENOW | RDW_ALLCHILDREN );
}


/**************************************************************************
 *              InvalidateRect   (USER.125)
 */
void WINAPI InvalidateRect( HWND hwnd, const RECT far *rect, BOOL erase )
{
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
//	FUNCTION_START
    return 0;
}

/**************************************************************************
 *              SysErrorBox   (USER.320)
 */
int FAR PASCAL SysErrorBox(LPSTR lpszMsg,        LPSTR lpszTitle, WORD wButton1, WORD wButton2, WORD wButton3)
{
//	FUNCTION_START
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

#if 0
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
#endif

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
	return 0;
}

#if 0
/**************************************************************************
 *              GetClientRect   (USER.33)
 */
void WINAPI GetClientRect( HWND hwnd, LPRECT rect )
{
	FUNCTION_START
	FUNCTION_END
}

#endif

/**************************************************************************
 *              EnableScrollBar   (USER.482)
 */
BOOL WINAPI EnableScrollBar( HWND hwnd, int nBar, UINT flags )
{
	FUNCTION_START
    return 0;//EnableScrollBar( WIN_Handle32(hwnd), nBar, flags );
}

/***********************************************************************
 *           RedrawWindow    (USER.290)
 */
BOOL WINAPI RedrawWindow( HWND hwnd, const RECT FAR * rectUpdate, HRGN hrgnUpdate, UINT flags )
{
    HRGN hrgn;
    RECT rectClient;
    WND * wndPtr;

    if (!hwnd) hwnd = GetDesktopWindow();
    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return FALSE;
    if (!IsWindowVisible(hwnd) || (wndPtr->flags & WIN_NO_REDRAW))
        return TRUE;  /* No redraw needed */

//    if (rectUpdate)
//    {
//        TRACE("RedrawWindow: %04x %d,%d-%d,%d %04x flags=%04x\n",
//                    hwnd, rectUpdate->left, rectUpdate->top,
//                    rectUpdate->right, rectUpdate->bottom, hrgnUpdate, flags );
//    }
//    else
//    {
//        TRACE("RedrawWindow: %04x NULL %04x flags=%04x\n",
//                     hwnd, hrgnUpdate, flags);
//    }
    GetClientRect( hwnd, &rectClient );


    if (flags & RDW_INVALIDATE)  /* Invalidate */
    {
        if (wndPtr->hrgnUpdate)  /* Is there already an update region? */
        {
            if ((hrgn = hrgnUpdate) == 0)
                hrgn = CreateRectRgnIndirect( rectUpdate ? rectUpdate :
                                              &rectClient );
            CombineRgn( wndPtr->hrgnUpdate, wndPtr->hrgnUpdate, hrgn, RGN_OR );
            if (!hrgnUpdate) DeleteObject( hrgn );
        }
        else  /* No update region yet */
        {
            if (!(wndPtr->flags & WIN_INTERNAL_PAINT))
                QUEUE_IncPaintCount( wndPtr->hmemTaskQ );
            if (hrgnUpdate)
            {
                wndPtr->hrgnUpdate = CreateRectRgn( 0, 0, 0, 0 );
                CombineRgn( wndPtr->hrgnUpdate, hrgnUpdate, 0, RGN_COPY );
            }
            else wndPtr->hrgnUpdate = CreateRectRgnIndirect( rectUpdate ?
                                                    rectUpdate : &rectClient );
        }
        if (flags & RDW_FRAME) wndPtr->flags |= WIN_NEEDS_NCPAINT;
        if (flags & RDW_ERASE) wndPtr->flags |= WIN_NEEDS_ERASEBKGND;
	flags |= RDW_FRAME;  /* Force invalidating the frame of children */
    }
    else if (flags & RDW_VALIDATE)  /* Validate */
    {
          /* We need an update region in order to validate anything */
        if (wndPtr->hrgnUpdate)
        {
            if (!hrgnUpdate && !rectUpdate)
            {
                  /* Special case: validate everything */
                DeleteObject( wndPtr->hrgnUpdate );
                wndPtr->hrgnUpdate = 0;
            }
            else
            {
                if ((hrgn = hrgnUpdate) == 0)
                    hrgn = CreateRectRgnIndirect( rectUpdate );
                if (CombineRgn( wndPtr->hrgnUpdate, wndPtr->hrgnUpdate,
                                hrgn, RGN_DIFF ) == NULLREGION)
                {
                    DeleteObject( wndPtr->hrgnUpdate );
                    wndPtr->hrgnUpdate = 0;
                }
                if (!hrgnUpdate) DeleteObject( hrgn );
            }
            if (!wndPtr->hrgnUpdate)  /* No more update region */
		if (!(wndPtr->flags & WIN_INTERNAL_PAINT))
		    QUEUE_DecPaintCount( wndPtr->hmemTaskQ );
        }
        if (flags & RDW_NOFRAME) wndPtr->flags &= ~WIN_NEEDS_NCPAINT;
	if (flags & RDW_NOERASE) wndPtr->flags &= ~WIN_NEEDS_ERASEBKGND;
    }

      /* Set/clear internal paint flag */

    if (flags & RDW_INTERNALPAINT)
    {
	if (!wndPtr->hrgnUpdate && !(wndPtr->flags & WIN_INTERNAL_PAINT))
	    QUEUE_IncPaintCount( wndPtr->hmemTaskQ );
	wndPtr->flags |= WIN_INTERNAL_PAINT;	    
    }
    else if (flags & RDW_NOINTERNALPAINT)
    {
	if (!wndPtr->hrgnUpdate && (wndPtr->flags & WIN_INTERNAL_PAINT))
	    QUEUE_DecPaintCount( wndPtr->hmemTaskQ );
	wndPtr->flags &= ~WIN_INTERNAL_PAINT;
    }


      /* Erase/update window */

    if (flags & RDW_UPDATENOW) SendMessage( hwnd, WM_PAINT, 0, 0 );
    else if (flags & RDW_ERASENOW)
    {
        if (wndPtr->flags & WIN_NEEDS_NCPAINT)
	    WIN_UpdateNCArea( wndPtr, FALSE);

        if (wndPtr->flags & WIN_NEEDS_ERASEBKGND)
        {
            HDC hdc = GetDCEx( hwnd, wndPtr->hrgnUpdate,
                               DCX_INTERSECTRGN | DCX_USESTYLE );
            if (hdc)
            {
              /* Don't send WM_ERASEBKGND to icons */
              /* (WM_ICONERASEBKGND is sent during processing of WM_NCPAINT) */
                if (!(wndPtr->dwStyle & WS_MINIMIZE)
                    || !WIN_CLASS_INFO(wndPtr).hIcon)
                {
                    if (SendMessage( hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0 ))
                        wndPtr->flags &= ~WIN_NEEDS_ERASEBKGND;
                }
                ReleaseDC( hwnd, hdc );
            }
        }
    }

      /* Recursively process children */

    if (!(flags & RDW_NOCHILDREN) &&
	((flags & RDW_ALLCHILDREN) || !(wndPtr->dwStyle & WS_CLIPCHILDREN)))
    {
	if (hrgnUpdate)
	{
	    HRGN hrgn = CreateRectRgn( 0, 0, 0, 0 );
	    if (!hrgn) return TRUE;
	    for (wndPtr = wndPtr->child; wndPtr; wndPtr = wndPtr->next)
	    {
		CombineRgn( hrgn, hrgnUpdate, 0, RGN_COPY );
		OffsetRgn( hrgn, -wndPtr->rectClient.left,
			         -wndPtr->rectClient.top );
		RedrawWindow( wndPtr->hwndSelf, NULL, hrgn, flags );
	    }
	    DeleteObject( hrgn );
	}
	else
	{
	    RECT rect;		
	    for (wndPtr = wndPtr->child; wndPtr; wndPtr = wndPtr->next)
	    {
		if (rectUpdate)
		{
		    rect = *rectUpdate;
		    OffsetRect( &rect, -wndPtr->rectClient.left,
			               -wndPtr->rectClient.top );
		    RedrawWindow( wndPtr->hwndSelf, &rect, 0, flags );
		}
		else RedrawWindow( wndPtr->hwndSelf, NULL, 0, flags );
	    }
	}
    }
    return TRUE;
}


/**************************************************************************
 *              GetDlgCtrlID   (USER.277)
 */
int WINAPI GetDlgCtrlID( HWND hwnd )
{
	FUNCTION_START
    return 0;//GetDlgCtrlID( WIN_Handle32(hwnd) );
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

#if 0
/**************************************************************************
 *              DestroyWindow   (USER.53)
 */
BOOL WINAPI DestroyWindow( HWND hwnd )
{
	FUNCTION_START
    return FALSE;//DestroyWindow( WIN_Handle32(hwnd) );
}
#endif


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

#if 0
/**************************************************************************
 *              IsWindowVisible   (USER.49)
 */
BOOL WINAPI IsWindowVisible( HWND hwnd )
{
	FUNCTION_START
    return FALSE;
}
#endif

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
 *              SetCapture   (USER.18)
 */
HWND WINAPI SetCapture( HWND hwnd )
{
	FUNCTION_START
    return 0;
}


#if 0
/**************************************************************************
 *              ScreenToClient   (USER.29)
 */
void WINAPI ScreenToClient( HWND hwnd, LPPOINT lppnt )
{
	FUNCTION_START
    MapWindowPoints( 0, hwnd, lppnt, 1 );
}

#endif
