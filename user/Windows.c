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

	FUNCTION_START
	GetClientRect( hwnd, &rect );
//TRACE("left=%d top=%d right=%d bottom=%d", rect.left, rect.top, rect.right, rect.bottom);
	DPtoLP( hdc, (LPPOINT)&rect, 2 );
TRACE("hdc=%d, hbr=%d left=%d top=%d right=%d bottom=%d", hdc, hbrush, rect.left, rect.top, rect.right, rect.bottom);
	PaintRect(hwndParent, hwnd, hdc, hbrush, &rect );
	FUNCTION_END
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
 *              UpdateWindow   (USER.124)
 */
void WINAPI UpdateWindow( HWND hwnd )
{
	FUNCTION_START
	RedrawWindow( hwnd, NULL, 0, RDW_UPDATENOW | RDW_ALLCHILDREN );
	FUNCTION_END
}


/**************************************************************************
 *              InvalidateRect   (USER.125)
 */
void WINAPI InvalidateRect( HWND hwnd, const RECT far *rect, BOOL erase )
{
	FUNCTION_START
	RedrawWindow( hwnd, rect, 0, RDW_INVALIDATE | (erase ? RDW_ERASE : 0) );
	FUNCTION_END
}


/**************************************************************************
 *              InvalidateRgn   (USER.126)
 */
void WINAPI InvalidateRgn( HWND hwnd, HRGN hrgn, BOOL erase )
{
	FUNCTION_START
	RedrawWindow( hwnd, NULL, hrgn, RDW_INVALIDATE | (erase ? RDW_ERASE : 0) );
	FUNCTION_END
}


/**************************************************************************
 *              ValidateRect   (USER.127)
 */
void WINAPI ValidateRect( HWND hwnd, const RECT far *rect )
{
	FUNCTION_START
	RedrawWindow( hwnd, rect, 0, RDW_VALIDATE | RDW_NOCHILDREN );
	FUNCTION_END
}


/**************************************************************************
 *              ValidateRgn   (USER.128)
 */
void WINAPI ValidateRgn( HWND hwnd, HRGN hrgn )
{
	FUNCTION_START
	RedrawWindow( hwnd, NULL, hrgn, RDW_VALIDATE | RDW_NOCHILDREN );
	FUNCTION_END
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
void WINAPI ScrollWindow( HWND hwnd, int dx, int dy, const RECT FAR *rect,
                            const RECT FAR *clipRect )
{
	FUNCTION_START
}


