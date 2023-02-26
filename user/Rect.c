/*
 * Misc 16-bit USER functions
 *
 * Copyright 1993, 1996 Alexandre Julliard
 * Copyright 2002 Patrik Stridvall
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

/*    
	Rect.c	2.5
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

#include <string.h>

#include <windows.h>
//#include "windowsx.h"
//#include "Log.h"

/***********************************************************************
 *		EqualRect (USER.244)
 */
BOOL WINAPI
EqualRect(const RECT far *r1, const RECT far *r2)
{
	return ((r1->left   == r2->left) &&
		(r1->top    == r2->top)  &&
		(r1->right  == r2->right) &&
		(r1->bottom == r2->bottom));

}

/***********************************************************************
 *		CopyRect (USER.74)
 */
void WINAPI
CopyRect(LPRECT d, const RECT far *s)
{
	d->left 	= s->left;
	d->top 		= s->top;
	d->right 	= s->right;
	d->bottom 	= s->bottom;
	return;  // @todo seems returns TRUE
}

/***********************************************************************
 *		InflateRect (USER.78)
 */
void WINAPI
InflateRect(LPRECT r, int x, int y)
{
	r->left   -= x;
	r->right  += x;
	r->top    -= y;
	r->bottom += y;
}

/***********************************************************************
 *		OffsetRect (USER.77)
 */
void WINAPI
OffsetRect(LPRECT r, int x, int y)
{
	r->left   += x;
	r->right  += x;
	r->top    += y;
	r->bottom += y;
}

/***********************************************************************
 *		PtInRect (USER.76)
 */
BOOL WINAPI
PtInRect(const RECT far *lpr,POINT pt)
{
    BOOL rc;

    rc = ((lpr->left <= pt.x) && (pt.x < lpr->right) &&
	  (lpr->top  <= pt.y) && (pt.y < lpr->bottom));

    return rc;
}

/***********************************************************************
 *		IntersectRect (USER.79)
 */
BOOL WINAPI
IntersectRect(LPRECT lpDestRect, const RECT far *lpSrc1Rect,
	      const RECT far *lpSrc2Rect)
{
    lpDestRect->left = max(lpSrc1Rect->left, lpSrc2Rect->left);
    lpDestRect->top = max(lpSrc1Rect->top, lpSrc2Rect->top);
    lpDestRect->right = min(lpSrc1Rect->right, lpSrc2Rect->right);
    lpDestRect->bottom = min(lpSrc1Rect->bottom, lpSrc2Rect->bottom);

    if ((lpDestRect->right <= lpDestRect->left) ||
	(lpDestRect->bottom <= lpDestRect->top)) {
	SetRectEmpty(lpDestRect);
	return FALSE;
    }
    else
	return TRUE;
}
 
/***********************************************************************
 *		SubtractRect (USER.373)
 */
BOOL    WINAPI 
SubtractRect(RECT FAR* lpDestRect, const RECT FAR*lprc1, const RECT FAR*lprc2)
{
	RECT	rc;

	if(IntersectRect(&rc,lprc1,lprc2)) {
		/* where is it anchored */
		if(rc.left == lprc1->left && rc.top == lprc1->top) {
			if(rc.right == lprc1->right) {
				lpDestRect->left = lprc1->left;
				lpDestRect->top  = rc.bottom;
				lpDestRect->right = lprc1->right;
				lpDestRect->bottom = lprc1->bottom;
				return TRUE;
			}
			if(rc.bottom == lprc1->bottom) {
				lpDestRect->left = rc.right;
				lpDestRect->top  = lprc1->top;
				lpDestRect->right = lprc1->right;
				lpDestRect->bottom = lprc1->bottom;
				return TRUE;
			}
		}
		if(rc.right == lprc1->right && rc.bottom == lprc1->bottom) {
			if(rc.top == lprc1->top) {
				lpDestRect->left = lprc1->left;
				lpDestRect->top  = lprc1->top;
				lpDestRect->right = rc.right;
				lpDestRect->bottom = lprc1->bottom;
				return TRUE;
			}
			if(rc.left == lprc1->left) {
				lpDestRect->left = lprc1->left;
				lpDestRect->top  = lprc1->top;
				lpDestRect->right = lprc1->right;
				lpDestRect->bottom = rc.top;
				return TRUE;
			}
		}
		
	}

	CopyRect(lpDestRect,lprc1);
	return FALSE;
}

/***********************************************************************
 *		IsRectEmpty (USER.75)
 *
 * Bug compat: Windows checks for 0 or negative width/height.
 */
BOOL WINAPI
IsRectEmpty(const RECT far *lprcRect)
{
    if (!lprcRect) return(TRUE);
    return ((lprcRect->right <= lprcRect->left) ||
        (lprcRect->bottom <= lprcRect->top));
}
 
/***********************************************************************
 *		SetRect (USER.72)
 */
void WINAPI
SetRect(LPRECT r, int left, int top, int right, int bottom)
{
    r->left     = left;
    r->top      = top;
    r->right    = right;
    r->bottom   = bottom;
}
 
/***********************************************************************
 *		SetRectEmpty (USER.73)
 */
void WINAPI
SetRectEmpty(LPRECT lpRect)
{
    lpRect->left = lpRect->top = lpRect->right = lpRect->bottom = 0;
}
 
/***********************************************************************
 *		UnionRect (USER.80)
 */
int WINAPI
UnionRect(LPRECT lpDestRect, const RECT far *lpSrc1Rect,
	  const RECT far *lpSrc2Rect)
{
    BOOL bSrc1Empty, bSrc2Empty;
 
    bSrc1Empty = IsRectEmpty(lpSrc1Rect);
    bSrc2Empty = IsRectEmpty(lpSrc2Rect);
 
    if (bSrc1Empty && bSrc2Empty)
        return(0);
    if (bSrc1Empty) {
        _fmemcpy((LPSTR)lpDestRect, (LPSTR)lpSrc2Rect, sizeof(RECT));
        return(1);
    }
    if (bSrc2Empty) {
        _fmemcpy((LPSTR)lpDestRect, (LPSTR)lpSrc1Rect, sizeof(RECT));
        return(1);
    }
    lpDestRect->top = min(lpSrc1Rect->top, lpSrc2Rect->top);
    lpDestRect->left = min(lpSrc1Rect->left, lpSrc2Rect->left);
    lpDestRect->right = max(lpSrc1Rect->right, lpSrc2Rect->right);
    lpDestRect->bottom = max(lpSrc1Rect->bottom, lpSrc2Rect->bottom);
    return(1);
}

/***********************************************************************
 *		FillRect (USER.81)
 * NOTE
 *   The Win16 variant doesn't support special color brushes like
 *   the Win32 one, despite the fact that Win16, as well as Win32,
 *   supports special background brushes for a window class.
 */
int WINAPI FillRect( HDC hdc, const RECT far *rect, HBRUSH hbrush )
{
    HBRUSH prevBrush;

    /* coordinates are logical so we cannot fast-check 'rect',
     * it will be done later in the PatBlt().
     */

    if (!(prevBrush = SelectObject( hdc, hbrush ))) return 0;
    PatBlt( hdc, rect->left, rect->top,
              rect->right - rect->left, rect->bottom - rect->top, PATCOPY );
    SelectObject( hdc, prevBrush );
    return 1;
}

/***********************************************************************
 *		InvertRect (USER.82)
 */
void WINAPI InvertRect( HDC hdc, const RECT far *rect )
{
    PatBlt( hdc, rect->left, rect->top,
              rect->right - rect->left, rect->bottom - rect->top, DSTINVERT );
}

/***********************************************************************
 *		FrameRect (USER.83)
 */
BOOL WINAPI
FrameRect(HDC hDC, const RECT far *lpRect, HBRUSH hBrush)
{
    HBRUSH hBrushOld;
    BOOL bRet;

    if (!lpRect)
	return FALSE;

//    APISTR((LF_API,"FrameRect: hDC=%x, rect %d,%d %d,%d, hBrush %x\n",
//	hDC,
//	lpRect->left,lpRect->top,lpRect->right,lpRect->bottom,
//	hBrush));

    if (!(hBrushOld = SelectObject(hDC,hBrush)))
	return FALSE;

    /* do the top bar */	
    bRet = PatBlt(hDC, lpRect->left, lpRect->top, 
		lpRect->right - lpRect->left,
		1, PATCOPY);

    if (bRet) {
	/* do the bottom bar */	
	PatBlt(hDC, lpRect->left, lpRect->bottom-1, 
		lpRect->right - lpRect->left,
		1, PATCOPY);

	/* do the left bar */	
	PatBlt(hDC, lpRect->left, lpRect->top, 
		1,
		lpRect->bottom-lpRect->top, PATCOPY);

	/* do the right bar */	
	PatBlt(hDC, lpRect->right-1, lpRect->top, 
		1,
		lpRect->bottom-lpRect->top, PATCOPY);
    }

    SelectObject(hDC,hBrushOld);

    return bRet;
}
