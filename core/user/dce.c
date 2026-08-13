/*
 * osFree Janus
 * USER DCE functions
 *
 * Copyright 1993 Alexandre Julliard
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
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

/**
 * @file dce.c
 * @brief USER DCE (Device Context Element) management
 *
 * This module handles the creation, caching, and management of Device Context
 * Elements (DCEs) in the USER subsystem. DCEs are lightweight structures that
 * associate a GDI device context (DC) with a specific window, caching the DC's
 * origin and clipping information. They are stored in the USER local heap and
 * are used to implement GetDC, ReleaseDC, and related functions in a 16-bit
 * Windows-compatible environment.
 *
 * Key features:
 * - Allocation and freeing of DCEs (both cache and window-specific)
 * - Visible region calculation for windows, considering window styles
 *   (WS_CLIPCHILDREN, WS_CLIPSIBLINGS) and DCX flags
 * - Origin setting for DCs based on window client or whole-window coordinates
 * - Caching of DCs to avoid repeated creation/destruction
 * - Support for DCX_USESTYLE, DCX_PARENTCLIP, DCX_INTERSECTRGN, etc.
 *
 * The module maintains a linked list of all DCEs (firstDCE). Cache DCEs are
 * created at startup (NB_DCE = 5) and reused as needed.
 */

// Public headers
#include "dce.h"
#include "user.h"

// Private headers
#include "dce.ih"

#pragma code_seg( "DCE_TEXT" );

//@todo b31 and bDebug must be global. May be move hGDI to DATA segment too?

// DCE lives in USER local heap. So, to access it we need to switch to USER local heap first and return DS back
// after it. See. Undocumented Windows p. 428

#define NB_DCE    5  /* Number of DCEs created at startup */

/* DCE Heap operation macros */
#define DCE_HEAP_ENTER()  { PushDS(); SetUserHeapDS(); }
#define DCE_HEAP_LEAVE()  { PopDS(); }
#define DCE_HEAP_ALLOC(size)   LocalAlloc(LHND, (size))
#define DCE_HEAP_LOCK(handle)  LocalLock(handle)
#define DCE_HEAP_UNLOCK(handle) LocalUnlock(handle)
#define DCE_HEAP_FREE(handle)  LocalFree(handle)

void DumpHeader(LPGDIOBJHDR goh)
	{
int b31=0;
int bDebug=0;
	if (b31)
		{
		TRACE(
			"GDIOBJHDR:\n"
			"hNext\t\t: %04X\twMagic\t\t: %04X (\"%c%c\")\n"
			"\tdwCount\t\t: %ld\n"
			"\twMetaList\t: %04X",
			goh->hNext, goh->wMagic,
			goh->wMagic & 0xff, goh->wMagic >> 8,
			goh->dwCount, goh->wMetaList);
		if (bDebug)
			TRACE(
				"\twSelCount\t: %d\n"
				"\thOwner\t\t: %04X\n",
				((LPGDIOBJDBG) goh)->wSelCount,
				((LPGDIOBJDBG) goh)->hOwner);
		}
	else
		{
		TRACE(
			"GDIOBJHDR:\n"
			"wFlags\t\t:%04X\twObjType\t:%04X\n"
			"dwCount\t:%ld\twMetaList\t:%04X",
			goh->hNext, goh->wMagic & 0xff,
			goh->dwCount, goh->wMetaList);
		}
		
	}

VOID FAR DumpDC(HDC hdc)
{
int b31=0;
int bDebug=0;
  DC FAR * dc;
  WORD gdi = 0;
return;
	gdi=SELECTOROF(GlobalLock(hGDI));
//	gdi &= 0xfffc; 
//	gdi |= 1;
	PushDS();
	SetDS(gdi);
	dc=MK_FP(gdi, LocalLock(hdc));
	PopDS();
	DumpHeader((LPGDIOBJHDR) &(dc->header));
	if (bDebug && b31) dc = (LPDC)((LPBYTE)dc+4);
	TRACE(
		"DC=%04x:\n"
		"byFlags\t\t:%02X\tbyFlags2\t:%02X\thMetaFile\t:%04X\n"
		"hrgnClip\t:%04X\thPDevice\t:%04X\thLPen\t\t: %04X\n"
		"hLBrush\t\t:%04X\thLFont\t\t:%04X\thBitmap\t\t:%04X\n"
		"dchPal\t\t:%04X\thLDevice\t:%04X\thRaoClip\t:%04X\n"
		"hPDeviceBlock\t: %04X\thPPen\t\t: %04X\n"
		"hPBrush\t\t: %04X\thPFontTrans\t: %04X\n"
		"hPFont\t\t: %04X\tlpPDevice\t: %Fp\n"
		"pLDevice\t: %04X\tpRaoClip\t: %04X", hdc,
		dc->byFlags, dc->byFlags2, dc->hMetaFile, dc->hrgnClip,
		dc->hPDevice, dc->hLPen, dc->hLBrush, dc->hLFont,
		dc->hBitmap, dc->dchPal, dc->hLDevice, dc->hRaoClip,
		dc->hPDeviceBlock, dc->hPPen, dc->hPBrush, dc->hPFontTrans,
		dc->hPFont, dc->lpPDevice, dc->pLDevice,
		dc->pRaoClip);
		
	TRACE(
		"pPDeviceBlock\t: %04X\tpPPen\t\t: %04X\n"
		"pPBrush\t\t: %04X\tpPFontTrans\t: %04X\n"
		"lpPFont\t\t: %Fp\tnPFTIndex\t: %04X\n"
		"fnTransform\t: %Fp\twROP2\t\t: %04X\n"
		"wBkMode\t\t: %04X\tdwBkColor\t: %08lX\n"
		"dwTextColor\t: %08lX\tnTBreakExtra\t: %d\n"
		"nBreakExtra\t: %d\twBreakErr\t: %04X\n"
		"nBreakRem\t: %d\tnBreakCount\t: %d\n"
		"nCharExtra\t: %d\tcrLbkColor\t: %08lX\n"
		"crLTextColor\t: %08lX\tLCursPosX\t: %d\n"
		"LCursPosY\t: %d",
		dc->pPDeviceBlock, dc->pPPen, dc->pPBrush, dc->pPFontTrans,
		dc->lpPFont, dc->nPFTIndex, dc->Transform, dc->wROP2,
		dc->wBkMode, dc->dwBkColor, dc->dwTextColor, dc->nTBreakExtra,
		dc->nBreakExtra, dc->wBreakErr, dc->nBreakRem, dc->nBreakCount,
		dc->nCharExtra, dc->crLbkColor, dc->crLTextColor, dc->LCursPosX,
		dc->LCursPosY);
		
	TRACE(
		"WndOrgX\t\t: %d\tWndOrgX\t\t: %d\n"
		"WndExtX\t\t: %d\tWndExtY\t\t: %d\n"
		"VportOrgX\t: %d\tVportOrgY\t: %d\n"
		"VportExtX\t: %d\tVportExtY\t: %d\n"
		"UserVptOrgX\t: %d\tUserVptOrgY\t: %d\n"
		"wMapMode\t: %04X\twXFormFlags\t: %04X\n"
		"wRelAbs\t\t: %04X\twPolyFillMode\t: %04X\n"
		"wStretchBltMode\t: %04X\tbyPlanes\t: %d\n"
		"byBitsPix\t: %d\twPenWidth\t: %d\n"
		"wPenHeight\t: %d\twTextAlign\t: %04X\n"
		"dwMapperFlags\t: %08lX",
		dc->WndOrgX, dc->WndOrgX, dc->WndExtX, dc->WndExtY,
		dc->VportOrgX, dc->VportOrgY, dc->VportExtX, dc->VportExtY,
		dc->UserVptOrgX, dc->UserVptOrgY, dc->wMapMode, dc->wXFormFlags,
		dc->wRelAbs, dc->wPolyFillMode, dc->wStretchBltMode, dc->byPlanes,
		dc->byBitsPix, dc->wPenWidth, dc->wPenHeight, dc->wTextAlign,
		dc->dwMapperFlags);
		
	TRACE(
		"wBrushOrgX\t:%d\twBrushOrgY\t:%d\twFontAspectX\t:%d\n"
		"wFontAspectY\t:%d\thFontWeights\t:%d\twDCSaveLevel\t:%d\n"
		"wcDCLocks\t: %d\thVisRgn\t\t: %04X\twDCOrgX\t\t:%d\n"
		"wDCOrgY\t\t:%d\tlpfnPrint\t:%Fp\twDCLogAtom\t:%04X\n"
		"wDCPhysAtom\t: %04X\twDCFileAtom\t: %04X\n"
		"wPostScaleX\t: %d\twPostScaleY\t: %d",
		dc->wBrushOrgX, dc->wBrushOrgY,
		dc->wFontAspectX, dc->wFontAspectY,
		dc->hFontWeights, dc->wDCSaveLevel,
		dc->wcDCLocks, dc->hVisRgn,
		dc->wDCOrgX, dc->wDCOrgY, dc->lpfnPrint, dc->wDCLogAtom,
		dc->wDCPhysAtom, dc->wDCFileAtom,
		dc->wPostScaleX, dc->wPostScaleY);
	if (b31)
		{
		TRACE(
			"\trectBounds\t: (%d, %d, %d, %d)\trectLVB\t\t: (%d, %d, %d, %d)\n"
			"\tlpfnNotify\t: %Fp\tlpHookData\t: %Fp\n"
			"\twDCGlobFlags\t: %04X",
			dc->dc_tail.tail_3_1.rectBounds.left,
			dc->dc_tail.tail_3_1.rectBounds.top,
			dc->dc_tail.tail_3_1.rectBounds.right,
			dc->dc_tail.tail_3_1.rectBounds.bottom,
			dc->dc_tail.tail_3_1.rectLVB.left,
			dc->dc_tail.tail_3_1.rectLVB.top,
			dc->dc_tail.tail_3_1.rectLVB.right,
			dc->dc_tail.tail_3_1.rectLVB.bottom,
			dc->dc_tail.tail_3_1.lpfnNotify,
			dc->dc_tail.tail_3_1.lpHookData,
			dc->dc_tail.tail_3_1.wDCGlobFlags);
		if (bDebug)
			TRACE(
				"\thDCNext\t\t: %04X\n",
				dc->dc_tail.tail_3_1.hDCNext);
		}
	else
		{
		TRACE(
			"wB4\t\t:%04X\trectB6\t\t:(%d, %d, %d, %d)\twDCGlobFlags\t:%04X\twC0\t\t:%04X\n",
			dc->dc_tail.tail_3_0.wB4,
			dc->dc_tail.tail_3_0.rectB6.left,
			dc->dc_tail.tail_3_0.rectB6.top,
			dc->dc_tail.tail_3_0.rectB6.right,
			dc->dc_tail.tail_3_0.rectB6.bottom,
			dc->dc_tail.tail_3_0.wDCGlobFlags,
			dc->dc_tail.tail_3_0.wC0);
		}
	PushDS();
	SetDS(gdi);
	LocalUnlock(hdc);
	PopDS();
	GlobalUnlock(hGDI);
}


/***********************************************************************
 *           DCE_AllocDCE
 *
 * @brief Allocate a new DCE (Device Context Element)
 *
 * Creates a new DCE structure in the USER local heap. A GDI DC is also created
 * using CreateDC for the display. The DCE is initialised with the given type
 * and added to the global linked list (firstDCE).
 *
 * @param type  Type of DCE (e.g., DCE_CACHE_DC for cache DCs)
 * @return Handle to the newly allocated DCE, or 0 on failure
 */
HANDLE FAR DCE_AllocDCE(DCE_TYPE type)
{
	DCE * dce;
	HANDLE retVal;

	FUNCTION_START

	retVal = LocalAlloc(LHND, sizeof(DCE));
	if (retVal)
	{
		dce = (DCE *) LocalLock(retVal);
		if (dce)
		{

			dce->hDC = CreateDC(DISPLAY, NULL, NULL, NULL);
			TRACE("hDC=0x%04x", dce->hDC);

			if (dce->hDC)
			{
				DumpDC(dce->hDC);
				dce->hwndCurr = 0;
				dce->byFlags  = type;
				dce->byInUse = (type != DCE_CACHE_DC);
				dce->xOrigin = 0;
				dce->yOrigin = 0;
				dce->hdceNext = firstDCE;
				LocalUnlock(retVal);
				firstDCE = retVal;
			} else {
				LocalUnlock(retVal);
				LocalFree(retVal);
				retVal=0;
			}
		} else {
			LocalFree(retVal);
			retVal=0;
		}
	}

	FUNCTION_END

	return retVal;
}


/**
 *           DCE_FreeDCE
 *
 * @brief Free a DCE
 *
 * Removes the DCE from the global linked list, deletes the associated GDI DC,
 * and frees the DCE memory from the USER local heap.
 *
 * @param hdce  Handle to the DCE to free
 */
VOID FAR DCE_FreeDCE(HANDLE hdce)
{
	DCE * dce;
	HANDLE *handle = &firstDCE;

	FUNCTION_START

	if (!(dce = (DCE *) LocalLock(hdce))) return;

	while (*handle && (*handle != hdce))
	{
		DCE * prev = (DCE *) LocalLock( *handle );	
		handle = &prev->hdceNext;
	}
	if (*handle == hdce) *handle = dce->hdceNext;

	DeleteDC( dce->hDC );
	LocalUnlock(hdce);
	LocalFree(hdce);

	FUNCTION_END
}

/**
 *           DCE_GetVisRect
 *
 * @brief Calculate the visible rectangle of a window
 *
 * Computes the visible portion of a window (client or whole-window area) after
 * clipping by all ancestor windows. The result is in window-relative coordinates.
 *
 * @param wndPtr      Pointer to the window structure (must be locked)
 * @param clientArea  If TRUE, use client area; otherwise use whole-window area
 * @param lprect      Output rectangle (window-relative visible area)
 * @return TRUE if visible area is non-empty, FALSE otherwise
 */
static BOOL DCE_GetVisRect( WND *wndPtr, BOOL clientArea, RECT FAR *lprect )
{
    int xoffset, yoffset;

    *lprect = clientArea ? wndPtr->rectClient : wndPtr->rectWindow;
    xoffset = lprect->left;
    yoffset = lprect->top;

    if (!(wndPtr->dwStyle & WS_VISIBLE) || (wndPtr->flags & WIN_NO_REDRAW))
    {
        SetRectEmpty( lprect );  /* Clip everything */
        return FALSE;
    }

    while (wndPtr->parent)
    {
        wndPtr = wndPtr->parent;
        if (!(wndPtr->dwStyle & WS_VISIBLE) ||
            (wndPtr->flags & WIN_NO_REDRAW) ||
            (wndPtr->dwStyle & WS_ICONIC))
        {
            SetRectEmpty( lprect );  /* Clip everything */
            return FALSE;
        }
	xoffset += wndPtr->rectClient.left;
	yoffset += wndPtr->rectClient.top;
	OffsetRect( lprect, wndPtr->rectClient.left,
		    wndPtr->rectClient.top );

	  /* Warning!! we assume that IntersectRect() handles the case */
	  /* where the destination is the same as one of the sources.  */
	if (!IntersectRect( lprect, lprect, &wndPtr->rectClient ))
            return FALSE;  /* Visible rectangle is empty */
    }
    OffsetRect( lprect, -xoffset, -yoffset );
    return TRUE;
}

/**
 *           DCE_ClipWindows
 *
 * @brief Subtract sibling/child windows from a region
 *
 * Iterates through a range of windows (from pWndStart to pWndEnd, exclusive
 * of pWndEnd) and subtracts each visible window's rectangle from the given
 * region. The rectangles are offset by (xoffset, yoffset) before subtraction.
 *
 * @param pWndStart  First window in the range (inclusive)
 * @param pWndEnd    Window at which to stop (exclusive)
 * @param hrgn       Region to modify (destroyed on failure)
 * @param xoffset    X offset for window rectangles
 * @param yoffset    Y offset for window rectangles
 * @return Modified region on success, 0 on failure
 */
static HRGN DCE_ClipWindows( WND *pWndStart, WND *pWndEnd,
                             HRGN hrgn, int xoffset, int yoffset )
{
    HRGN hrgnNew;

    if (!pWndStart) return hrgn;
    if (!(hrgnNew = CreateRectRgn( 0, 0, 0, 0 )))
    {
        DeleteObject( hrgn );
        return 0;
    }
    for (; pWndStart != pWndEnd; pWndStart = pWndStart->next)
    {
        if (!(pWndStart->dwStyle & WS_VISIBLE)) continue;
        SetRectRgn( hrgnNew, pWndStart->rectWindow.left + xoffset,
                    pWndStart->rectWindow.top + yoffset,
                    pWndStart->rectWindow.right + xoffset,
                    pWndStart->rectWindow.bottom + yoffset );
        if (!CombineRgn( hrgn, hrgn, hrgnNew, RGN_DIFF )) break;
    }
    DeleteObject( hrgnNew );
    if (pWndStart != pWndEnd)  /* something went wrong */
    {
        DeleteObject( hrgn );
        return 0;
    }
    return hrgn;
}


/**
 *           DCE_GetVisRgn
 *
 * @brief Generate the visible region of a window
 *
 * Creates a region representing the visible area of a window, taking into
 * account clipping by ancestors and, optionally, siblings and children as
 * specified by the DCX flags.
 *
 * @param hwnd   Handle to the window
 * @param flags  DCX_* flags (DCX_WINDOW, DCX_CLIPCHILDREN, DCX_CLIPSIBLINGS)
 * @return Handle to a new region (caller must delete it), or 0 on failure
 */
HRGN DCE_GetVisRgn( HWND hwnd, WORD flags )
{
    RECT rect;
    HRGN hrgn;
    int xoffset, yoffset;
    WND *wndPtr;

	FUNCTION_START
	TRACE("hwnd=%04x", hwnd);
	wndPtr = WIN_FindWndPtr( hwnd );
	TRACE("wndPtr=%p", wndPtr);

      /* Get visible rectangle and create a region with it */
    if (!DCE_GetVisRect( wndPtr, !(flags & DCX_WINDOW), &rect ))
    {
        return CreateRectRgn( 0, 0, 0, 0 );  /* Visible region is empty */
    }
    if (!(hrgn = CreateRectRgnIndirect( &rect ))) return 0;

      /* Clip all children from the visible region */

    if (flags & DCX_CLIPCHILDREN)
    {
        if (flags & DCX_WINDOW)
        {
            xoffset = wndPtr->rectClient.left - wndPtr->rectWindow.left;
            yoffset = wndPtr->rectClient.top - wndPtr->rectWindow.top;
        }
        else xoffset = yoffset = 0;
        hrgn = DCE_ClipWindows( wndPtr->child, NULL, hrgn, xoffset, yoffset );
        if (!hrgn) return 0;
    }

      /* Clip siblings placed above this window */

    if (flags & DCX_WINDOW)
    {
        xoffset = -wndPtr->rectWindow.left;
        yoffset = -wndPtr->rectWindow.top;
    }
    else
    {
        xoffset = -wndPtr->rectClient.left;
        yoffset = -wndPtr->rectClient.top;
    }
    if (flags & DCX_CLIPSIBLINGS)
    {
        hrgn = DCE_ClipWindows( wndPtr->parent ? wndPtr->parent->child : NULL,
                                wndPtr, hrgn, xoffset, yoffset );
        if (!hrgn) return 0;
    }

      /* Clip siblings of all ancestors that have the WS_CLIPSIBLINGS style */

    while (wndPtr->dwStyle & WS_CHILD)
    {
        wndPtr = wndPtr->parent;
        xoffset -= wndPtr->rectClient.left;
        yoffset -= wndPtr->rectClient.top;
        hrgn = DCE_ClipWindows( wndPtr->parent->child, wndPtr,
                                hrgn, xoffset, yoffset );
        if (!hrgn) return 0;
    }
    return hrgn;
}

/**
 *           DCE_SetDrawable
 *
 * @brief Set the drawable origin for a DC based on its associated window
 *
 * Computes the screen origin for the DC's drawing surface (window client area
 * or whole window) and sets it using SetDCOrg. Also stores the origin in the
 * DCE for later use (e.g., region offsetting).
 *
 * @param dce     Pointer to the DCE (must be locked)
 * @param wndPtr  Pointer to the window structure (or NULL for screen DC)
 * @param hdc     Handle to the GDI DC
 * @param flags   DCX_* flags (DCX_WINDOW, etc.)
 */
static void DCE_SetDrawable(DCE *dce, WND *wndPtr, HDC hdc, WORD flags )
{
    WORD wNewOrgX, wNewOrgY;

	FUNCTION_START

    if (!wndPtr)  /* Get a DC for the whole screen */
    {
        wNewOrgX = 0;
        wNewOrgY = 0;
    }
    else
    {
	TRACE("DCE_SetDrawable: hwnd=%04x, flags=%04x, rectWindow=(%d,%d,%d,%d), rectClient=(%d,%d,%d,%d)",
      wndPtr->hwndSelf, flags,
      wndPtr->rectWindow.left, wndPtr->rectWindow.top,
      wndPtr->rectWindow.right, wndPtr->rectWindow.bottom,
      wndPtr->rectClient.left, wndPtr->rectClient.top,
      wndPtr->rectClient.right, wndPtr->rectClient.bottom);

        if (flags & DCX_WINDOW)
        {
		wNewOrgX  = wndPtr->rectWindow.left;
		wNewOrgY  = wndPtr->rectWindow.top;
		TRACE("DCX_WINDOW: origin=(%d,%d)", wNewOrgX, wNewOrgY);
        }
        else
        {
		wNewOrgX  = wndPtr->rectClient.left;
		wNewOrgY  = wndPtr->rectClient.top;
		TRACE("!DCX_WINDOW: origin=(%d,%d)", wNewOrgX, wNewOrgY);
        }

	if ((wndPtr->parent) && (wndPtr->parent->hwndSelf!=hwndDesktop)) 
	{
		TRACE("Starting parent loop, parent hwnd=%04x", wndPtr->parent->hwndSelf);
        	while (wndPtr->parent)
	        {
	            wndPtr = wndPtr->parent;
			TRACE("Parent %04x: rectClient=(%d,%d)", wndPtr->hwndSelf,
				wndPtr->rectClient.left, wndPtr->rectClient.top);
	            wNewOrgX += wndPtr->rectClient.left;
	            wNewOrgY += wndPtr->rectClient.top;
	        }
	
	        wNewOrgX -= wndPtr->rectWindow.left;
	        wNewOrgY -= wndPtr->rectWindow.top;
		TRACE("After parent loop: origin=(%d,%d)", wNewOrgX, wNewOrgY);
	}
    }


	TRACE("Final origin: (%d,%d)", wNewOrgX, wNewOrgY);
	SetDCOrg(hdc, wNewOrgX, wNewOrgY);

	dce->xOrigin = wNewOrgX;
	dce->yOrigin = wNewOrgY;
	FUNCTION_END
}

/**
 *           GetDCEx    (USER.359)
 *
 * @brief Extended GetDC function (USER.359)
 *
 * Retrieves a device context for the specified window, with advanced options
 * for clipping, caching, and region intersection. This is the core function
 * behind GetDC and GetWindowDC.
 *
 * Supported flags (DCX_*):
 * - DCX_WINDOW: Use whole window instead of client area
 * - DCX_CACHE: Use a cached DC rather than a window-specific one
 * - DCX_CLIPCHILDREN: Exclude child windows from the visible region
 * - DCX_CLIPSIBLINGS: Exclude overlapping siblings
 * - DCX_PARENTCLIP: Use parent's visible region (for child windows)
 * - DCX_USESTYLE: Derive flags from window class/style
 * - DCX_INTERSECTRGN / DCX_EXCLUDERGN: Combine with a supplied clip region
 * - DCX_LOCKWINDOWUPDATE (unimplemented)
 *
 * @param hwnd       Handle to the window (NULL for screen DC)
 * @param hrgnClip   Handle to a region for intersection/exclusion (optional)
 * @param flags      Combination of DCX_* flags
 * @return Handle to the device context, or 0 on failure
 */
HDC WINAPI GetDCEx( HWND hwnd, HRGN hrgnClip, DWORD flags )
{
	HANDLE hdce;
	HRGN hrgnVisible;
	HDC hdc = 0;
	DCE * dce;
	WND * wndPtr;
	int xOrigin, yOrigin;

	PushDS();
	SetUserHeapDS();
	FUNCTION_START

	if (hwnd)
	{
		if(!(wndPtr = WIN_FindWndPtr( hwnd ))) return 0;
	}
	else wndPtr = NULL;

    if (flags & DCX_USESTYLE)
    {
        /* Set the flags according to the window style. */
	/* Not sure if this is the real meaning of the DCX_USESTYLE flag... */
	flags &= ~(DCX_CACHE | DCX_CLIPCHILDREN |
                   DCX_CLIPSIBLINGS | DCX_PARENTCLIP);                           	

	if (wndPtr)
	{
            if (!(WIN_CLASS_STYLE(wndPtr) & (CS_OWNDC | CS_CLASSDC)))
		flags |= DCX_CACHE;
            if (WIN_CLASS_STYLE(wndPtr) & CS_PARENTDC) flags |= DCX_PARENTCLIP;
	    if (wndPtr->dwStyle & WS_CLIPCHILDREN) flags |= DCX_CLIPCHILDREN;
	    if (wndPtr->dwStyle & WS_CLIPSIBLINGS) flags |= DCX_CLIPSIBLINGS;
	}
	else flags |= DCX_CACHE;
    }

      /* Can only use PARENTCLIP on child windows */
    if (!wndPtr || !(wndPtr->dwStyle & WS_CHILD)) flags &= ~DCX_PARENTCLIP;

      /* Whole window DC implies using cache DC and not clipping children */
    if (flags & DCX_WINDOW) flags = (flags & ~DCX_CLIPCHILDREN) | DCX_CACHE;

    if (flags & DCX_CACHE)
    {
	// Search free DCE in USER local heap
	// So, switch DS to USER local heap and return back later
    for (hdce = firstDCE; hdce; )
    {
	HANDLE next;
        dce = (DCE *)LocalLock(hdce);  // LocalLock возвращает указатель
        if (!dce) 
        {
            PopDS();
            return 0;
        }
        
        // Сохраняем следующий элемент ПЕРЕД разблокировкой
        next = dce->hdceNext;
        
        if ((dce->byFlags == DCE_CACHE_DC) && (!dce->byInUse))
        {
            LocalUnlock(hdce);
            break;
        }
        
        LocalUnlock(hdce);
        hdce = next;  // Используем сохраненный handle
    }

    }
    else hdce = wndPtr->hdce;

	if (!hdce) 
	{
		PopDS();
		return 0;
	}


	// Fill DCE in USER local heap
	// So, switch DS to USER local heap and return back later
	dce = (DCE *) LocalLock( hdce );

	dce->hwndCurr = hwnd;
	dce->byInUse       = TRUE;
	hdc = dce->hDC;
	DCE_SetDrawable(dce, wndPtr, hdc, flags);

	// Сохраняем xOrigin и yOrigin для последующего использования
	xOrigin = dce->xOrigin;
	yOrigin = dce->yOrigin;

	LocalUnlock(hdce);

	TRACE(__FUNCTION__ " 0");
    

    if (hwnd)
    {
	TRACE(__FUNCTION__ " 0.0");
        if (wndPtr && wndPtr->parent && (flags & DCX_PARENTCLIP))  /* Get a VisRgn for the parent */
        {
            WND *parentPtr = wndPtr->parent;
            DWORD newflags = flags & ~(DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN |
                                       DCX_WINDOW);
	TRACE(__FUNCTION__ " 0.1");
            if (parentPtr->dwStyle & WS_CLIPSIBLINGS)
                newflags |= DCX_CLIPSIBLINGS;
            hrgnVisible = DCE_GetVisRgn( parentPtr->hwndSelf, newflags );
            if (flags & DCX_WINDOW)
                OffsetRgn( hrgnVisible, -wndPtr->rectWindow.left,
                                        -wndPtr->rectWindow.top );
            else OffsetRgn( hrgnVisible, -wndPtr->rectClient.left,
                                         -wndPtr->rectClient.top );
        }
        else 
	{
		TRACE(__FUNCTION__ " 0.x1");
		hrgnVisible = DCE_GetVisRgn( hwnd, flags );
	}
    }
    else  /* Get a VisRgn for the whole screen */
    {
        hrgnVisible = CreateRectRgn( 0, 0, GETSYSTEMMETRICS(SM_CXSCREEN), GETSYSTEMMETRICS(SM_CYSCREEN));
    }

	TRACE(__FUNCTION__ " 1");
      /* Intersect VisRgn with the given region */

    if ((flags & DCX_INTERSECTRGN) || (flags & DCX_EXCLUDERGN))
    {
        CombineRgn( hrgnVisible, hrgnVisible, hrgnClip,
                    (flags & DCX_INTERSECTRGN) ? RGN_AND : RGN_DIFF );
    }

	TRACE(__FUNCTION__ " 2");
    // После получения hrgnVisible и всех операций с ним:
    if (hwnd)  // Только для окон, не для экрана
    {
      // Смещаем регион из экранных координат в логические координаты DC
      OffsetRgn(hrgnVisible, xOrigin, yOrigin);
    }

    SelectVisRgn( hdc, hrgnVisible );
	TRACE(__FUNCTION__ " 3");
	DumpDC(dce->hDC);
	// @todo А точно надо удалять тут?
    DeleteObject( hrgnVisible );


    TRACE("GetDCEx(%04x,%04x,0x%lx): returning %04x", 
	       hwnd, hrgnClip, flags, hdc);
	PopDS();
	return hdc;
}


/**
 *           GetDC    (USER.66)
 *
 * @brief Get a device context for a window (USER.66)
 *
 * Equivalent to GetDCEx(hwnd, 0, DCX_USESTYLE).
 *
 * @param hwnd  Handle to the window (NULL for screen DC)
 * @return Handle to the device context
 */
HDC WINAPI GetDC(HWND hwnd)
{
	HDC res;
	FUNCTION_START
	res=GetDCEx(hwnd, 0, DCX_USESTYLE);
	FUNCTION_END
	return res;
}


/**
 *           GetWindowDC    (USER.67)
 *
 * @brief Get a device context for the whole window (including non-client area) (USER.67)
 *
 * Equivalent to GetDCEx with DCX_CACHE | DCX_WINDOW, and optionally
 * DCX_CLIPSIBLINGS if the window style indicates it.
 *
 * @param hWnd  Handle to the window
 * @return Handle to the device context
 */
HDC WINAPI GetWindowDC(/* in */ HWND hWnd)
{
	int flags = DCX_CACHE | DCX_WINDOW;
	HDC retVal=0;
	WND * wndPtr;

	PushDS();
	SetUserHeapDS();
	FUNCTION_START

	if (hWnd)
	{
		if ((wndPtr = WIN_FindWndPtr( hWnd )))
		{
			/* if (wndPtr->dwStyle & WS_CLIPCHILDREN) flags |= DCX_CLIPCHILDREN; */
			if (wndPtr->dwStyle & WS_CLIPSIBLINGS) flags |= DCX_CLIPSIBLINGS;
			LocalUnlock(hWnd);
		}
	}

	retVal=GetDCEx(hWnd, 0, flags);

	PopDS();
	FUNCTION_END
	return retVal;
}


/**
 * @brief Release a device context obtained via GetDC or GetWindowDC (USER.68)
 *
 * If the DC belongs to a cache DCE, it is marked as unused and its state is
 * reset to the saved default. For other DCEs (e.g., private DCs), this function
 * may have no effect (they are not released back to a pool).
 *
 * @param hwnd  Handle to the window (may be ignored)
 * @param hdc   Handle to the device context to release
 * @return 1 on success, 0 on failure
 */

int WINAPI ReleaseDC(HWND hwnd, HDC hdc)
{
	HANDLE hdce;
	DCE * dce;
    
	TRACE("ReleaseDC: %04x %04x\n", hwnd, hdc );

	// Here we need to search DCE which lives in USER local heap
	// So, switch to USER local heap, do things and switch back.
	PushDS();
	SetUserHeapDS();

	for (hdce = firstDCE; (hdce); hdce = dce->hdceNext)
	{
		if (!(dce = (DCE *) LocalLock(hdce)))
		{
			PopDS();
			return 0;
		}

		if (dce->byInUse && (dce->hDC == hdc))
		{
			LocalUnlock(hdce);
			break;
		}
		LocalUnlock(hdce);
	}

	if (!hdce) 
	{
		PopDS();
		return 0;
	}

	dce = (DCE *) LocalLock(hdce);

	if (dce->byFlags == DCE_CACHE_DC)
	{
		DumpDC(dce->hDC);
		SetDCState(dce->hDC, defaultDCstate);
		DumpDC(dce->hDC);
		SetMapMode(dce->hDC, MM_TEXT);
		SetDCOrg(dce->hDC, 0, 0);
		TRACE("6");
		DumpDC(dce->hDC);
		TRACE("defaultDCstate=0x%04x", defaultDCstate);
		dce->byInUse = FALSE;
	}

	LocalUnlock(hdce);

	FUNCTION_END
	PopDS();
	return 1;
}

#pragma code_seg( "INIT_TEXT" );

/**
 * @brief Initialise the DCE subsystem
 *
 * Called during USER initialisation. Creates NB_DCE (5) cache DCEs and saves
 * the default DC state (defaultDCstate) from the first created DC for later
 * resetting of cache DCs.
 */
VOID FAR DCE_Init()
{
    int i;
    HANDLE handle;
    DCE * dce;

	FUNCTION_START        
    for (i = 0; i < NB_DCE; i++)
    {
	if (!(handle = DCE_AllocDCE(DCE_CACHE_DC))) return;
	dce = (DCE *)LocalLock(handle);
	if (!defaultDCstate)
	{
		defaultDCstate = GetDCState(dce->hDC);
		TRACE("defaultDCstate=0x%04x", defaultDCstate);
	}
	LocalUnlock(handle);
    }
	FUNCTION_END
}

#pragma code_seg();
