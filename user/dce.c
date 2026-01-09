/*
 * USER DCE functions
 *
 * Copyright 1993 Alexandre Julliard
 *
*/

#include "dce.h"
#include "user.h"

// DCE lives in USER local heap. So, to access it we need to switch to USER local heap first and return DS back
// after it. See. Undocumented Windows p. 428

#define NB_DCE    5  /* Number of DCEs created at startup */

static HDC defaultDCstate = 0;

void DumpHeader(LPGDIOBJHDR goh)
	{
int b31=0;
int bDebug=0;
	if (b31)
		{
		printf(
			"GDIOBJHDR:\n\r"
			"hNext\t\t: %04X\twMagic\t\t: %04X (\"%c%c\")\n\r"
			"\tdwCount\t\t: %ld\n\r"
			"\twMetaList\t: %04X\n\r",
			goh->hNext, goh->wMagic,
			goh->wMagic & 0xff, goh->wMagic >> 8,
			goh->dwCount, goh->wMetaList);
		if (bDebug)
			printf(
				"\twSelCount\t: %d\n\r"
				"\thOwner\t\t: %04X\n\r",
				((LPGDIOBJDBG) goh)->wSelCount,
				((LPGDIOBJDBG) goh)->hOwner);
		}
	else
		{
		printf(
			"GDIOBJHDR:\n\r"
			"wFlags\t\t: %04X\twObjType\t: %d\tdwCount\t\t: %ld\twMetaList\t: %04X\n\r",
			goh->hNext, goh->wMagic & 0xff,
			goh->dwCount, goh->wMetaList);
		}
		
	}

VOID DumpDC(HDC hdc)
{
int b31=0;
int bDebug=0;
  DC FAR * dc;
  WORD gdi = 0;
	PushDS();
	gdi=SELECTOROF(GlobalLock(hGDI));
//	gdi &= 0xfffc; 
//	gdi |= 1;
	SetDS(gdi);
	dc=MK_FP(gdi, LocalLock(hdc));
	DumpHeader((LPGDIOBJHDR) &(dc->header));
	if (bDebug && b31) dc = (LPDC)((LPBYTE)dc+4);
	printf(
		"DC:\n\r"
		"byFlags\t\t:%02X\tbyFlags2\t:%02X\thMetaFile\t:%04X\n\r"
		"hrgnClip\t:%04X\thPDevice\t:%04XthLPen\t\t: %04X\n\r"
		"hLBrush\t\t:%04X\thLFont\t\t:%04X\thBitmap\t\t:%04X\n\r"
		"dchPal\t\t:%04X\thLDevice\t:%04X\n\r\thRaoClip\t:%04X\n\r"
		"hPDeviceBlock\t: %04X\thPPen\t\t: %04X\n\r"
		"hPBrush\t\t: %04XthPFontTrans\t: %04X\n\r"
		"hPFont\t\t: %04X\tlpPDevice\t: %Fp\n\r"
		"pLDevice\t: %04X\tpRaoClip\t: %04X\n\r",
		dc->byFlags, dc->byFlags2, dc->hMetaFile, dc->hrgnClip,
		dc->hPDevice, dc->hLPen, dc->hLBrush, dc->hLFont,
		dc->hBitmap, dc->dchPal, dc->hLDevice, dc->hRaoClip,
		dc->hPDeviceBlock, dc->hPPen, dc->hPBrush, dc->hPFontTrans,
		dc->hPFont, dc->lpPDevice, dc->pLDevice,
		dc->pRaoClip);
		
	printf(
		"pPDeviceBlock\t: %04X\tpPPen\t\t: %04X\n\r"
		"pPBrush\t\t: %04X\tpPFontTrans\t: %04X\n\r"
		"lpPFont\t\t: %Fp\tnPFTIndex\t: %04X\n\r"
		"fnTransform\t: %Fp\twROP2\t\t: %04X\n\r"
		"wBkMode\t\t: %04X\tdwBkColor\t: %08lX\n\r"
		"dwTextColor\t: %08lX\tnTBreakExtra\t: %d\n\r"
		"nBreakExtra\t: %d\twBreakErr\t: %04X\n\r"
		"nBreakRem\t: %d\tnBreakCount\t: %d\n\r"
		"nCharExtra\t: %d\tcrLbkColor\t: %08lX\n\r"
		"crLTextColor\t: %08lX\tLCursPosX\t: %d\n\r"
		"LCursPosY\t: %d\n\r",
		dc->pPDeviceBlock, dc->pPPen, dc->pPBrush, dc->pPFontTrans,
		dc->lpPFont, dc->nPFTIndex, dc->Transform, dc->wROP2,
		dc->wBkMode, dc->dwBkColor, dc->dwTextColor, dc->nTBreakExtra,
		dc->nBreakExtra, dc->wBreakErr, dc->nBreakRem, dc->nBreakCount,
		dc->nCharExtra, dc->crLbkColor, dc->crLTextColor, dc->LCursPosX,
		dc->LCursPosY);
		
	printf(
		"WndOrgX\t\t: %d\tWndOrgX\t\t: %d\n\r"
		"WndExtX\t\t: %d\tWndExtY\t\t: %d\n\r"
		"VportOrgX\t: %d\tVportOrgY\t: %d\n\r"
		"VportExtX\t: %d\tVportExtY\t: %d\n\r"
		"UserVptOrgX\t: %d\tUserVptOrgY\t: %d\n\r"
		"wMapMode\t: %04X\twXFormFlags\t: %04X\n\r"
		"wRelAbs\t\t: %04X\twPolyFillMode\t: %04X\n\r"
		"wStretchBltMode\t: %04X\tbyPlanes\t: %d\n\r"
		"byBitsPix\t: %d\twPenWidth\t: %d\n\r"
		"wPenHeight\t: %d\twTextAlign\t: %04X\n\r"
		"dwMapperFlags\t: %08lX\n\r",
		dc->WndOrgX, dc->WndOrgX, dc->WndExtX, dc->WndExtY,
		dc->VportOrgX, dc->VportOrgY, dc->VportExtX, dc->VportExtY,
		dc->UserVptOrgX, dc->UserVptOrgY, dc->wMapMode, dc->wXFormFlags,
		dc->wRelAbs, dc->wPolyFillMode, dc->wStretchBltMode, dc->byPlanes,
		dc->byBitsPix, dc->wPenWidth, dc->wPenHeight, dc->wTextAlign,
		dc->dwMapperFlags);
		
	printf(
		"wBrushOrgX\t:%d\twBrushOrgY\t:%d\twFontAspectX\t:%d\n\r"
		"wFontAspectY\t:%d\thFontWeights\t:%d\twDCSaveLevel\t:%d\n\r"
		"wcDCLocks\t: %d\thVisRgn\t\t: %04X\twDCOrgX\t\t:%d\n\r"
		"wDCOrgY\t\t:%d\tlpfnPrint\t:%Fp\twDCLogAtom\t:%04X\n\r"
		"wDCPhysAtom\t: %04X\twDCFileAtom\t: %04X\n\r"
		"wPostScaleX\t: %d\twPostScaleY\t: %d\n\r",
		dc->wBrushOrgX, dc->wBrushOrgY,
		dc->wFontAspectX, dc->wFontAspectY,
		dc->hFontWeights, dc->wDCSaveLevel,
		dc->wcDCLocks, dc->hVisRgn,
		dc->wDCOrgX, dc->wDCOrgY, dc->lpfnPrint, dc->wDCLogAtom,
		dc->wDCPhysAtom, dc->wDCFileAtom,
		dc->wPostScaleX, dc->wPostScaleY);
	if (b31)
		{
		printf(
			"\trectBounds\t: (%d, %d, %d, %d)\trectLVB\t\t: (%d, %d, %d, %d)\n\r"
			"\tlpfnNotify\t: %Fp\tlpHookData\t: %Fp\n\r"
			"\twDCGlobFlags\t: %04X\n\r",
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
			printf(
				"\thDCNext\t\t: %04X\n\r",
				dc->dc_tail.tail_3_1.hDCNext);
		}
	else
		{
		printf(
			"wB4\t\t:%04X\trectB6\t\t:(%d, %d, %d, %d)\twDCGlobFlags\t:%04X\twC0\t\t:%04X\n\r",
			dc->dc_tail.tail_3_0.wB4,
			dc->dc_tail.tail_3_0.rectB6.left,
			dc->dc_tail.tail_3_0.rectB6.top,
			dc->dc_tail.tail_3_0.rectB6.right,
			dc->dc_tail.tail_3_0.rectB6.bottom,
			dc->dc_tail.tail_3_0.wDCGlobFlags,
			dc->dc_tail.tail_3_0.wC0);
		}
	LocalUnlock(hdc);
	GlobalUnlock(hGDI);
	PopDS();
}

/***********************************************************************
 *           DCE_AllocDCE
 *
 * Allocate a new DCE.
 */
HANDLE DCE_AllocDCE( DCE_TYPE type )
{
    DCE * dce;
    HANDLE handle = LocalAlloc(LMEM_FIXED, sizeof(DCE));
    if (!handle) return 0;
    dce = (DCE *) LocalLock( handle );
    if (!(dce->hDC = CreateDC( "DISPLAY", NULL, NULL, NULL )))
    {
	LocalFree( handle );
	return 0;
    }

	DumpDC(dce->hDC);
    dce->hwndCurr = 0;
    dce->byFlags  = type;
    dce->byInUse = (type != DCE_CACHE_DC);
    dce->xOrigin = 0;
    dce->yOrigin = 0;
    dce->hdceNext = firstDCE;
    firstDCE = handle;
    return handle;
}


/***********************************************************************
 *           DCE_FreeDCE
 */
void DCE_FreeDCE( HANDLE hdce )
{
    DCE * dce;
    HANDLE *handle = &firstDCE;

    if (!(dce = (DCE *) LocalLock( hdce ))) return;
    while (*handle && (*handle != hdce))
    {
	DCE * prev = (DCE *) LocalLock( *handle );	
	handle = &prev->hdceNext;
    }
    if (*handle == hdce) *handle = dce->hdceNext;
    DeleteDC( dce->hDC );
    LocalFree( hdce );
}


/***********************************************************************
 *           DCE_Init
 */
void DCE_Init()
{
    int i;
    HANDLE handle;
    DCE * dce;
        
    for (i = 0; i < NB_DCE; i++)
    {
	if (!(handle = DCE_AllocDCE( DCE_CACHE_DC ))) return;
	dce = (DCE *) LocalLock( handle );	
	if (!defaultDCstate) defaultDCstate = GetDCState( dce->hDC );
    }
}

#if 0
/***********************************************************************
 *           DCE_GetVisRect
 *
 * Calc the visible rectangle of a window, i.e. the client or
 * window area clipped by the client area of all ancestors.
 * Return FALSE if the visible region is empty.
 */
static BOOL DCE_GetVisRect( WND *wndPtr, BOOL clientArea, RECT *lprect )
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


/***********************************************************************
 *           DCE_ClipWindows
 *
 * Go through the linked list of windows from hwndStart to hwndEnd,
 * removing from the given region the rectangle of each window offset
 * by a given amount.  The new region is returned, and the original one
 * is destroyed.  Used to implement DCX_CLIPSIBLINGS and
 * DCX_CLIPCHILDREN styles.
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


/***********************************************************************
 *           DCE_GetVisRgn
 *
 * Return the visible region of a window, i.e. the client or window area
 * clipped by the client area of all ancestors, and then optionally
 * by siblings and children.
 */
HRGN DCE_GetVisRgn( HWND hwnd, WORD flags )
{
    RECT rect;
    HRGN hrgn;
    int xoffset, yoffset;
    WND *wndPtr = WIN_FindWndPtr( hwnd );

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

#endif

/***********************************************************************
 *           DCE_SetDrawable
 *
 * Set the origin and dimensions for the DC associated to
 * a given window.
 */
static void DCE_SetDrawable( WND *wndPtr, DC FAR *dc, WORD flags )
{
    if (!wndPtr)  /* Get a DC for the whole screen */
    {
        dc->wDCOrgX = 0;
        dc->wDCOrgY = 0;
    }
    else
    {
        if (flags & DCX_WINDOW)
        {
            dc->wDCOrgX  = wndPtr->rectWindow.left;
            dc->wDCOrgY  = wndPtr->rectWindow.top;
        }
        else
        {
            dc->wDCOrgX  = wndPtr->rectClient.left;
            dc->wDCOrgY  = wndPtr->rectClient.top;
        }

        dc->wDCOrgX -= wndPtr->rectWindow.left;
        dc->wDCOrgY -= wndPtr->rectWindow.top;
    }
}


/***********************************************************************
 *           GetDCEx    (USER.359)
 *
 * Unimplemented flags: DCX_LOCKWINDOWUPDATE
 */
HDC WINAPI GetDCEx( HWND hwnd, HRGN hrgnClip, DWORD flags )
{
    HANDLE hdce;
    HRGN hrgnVisible;
    HDC hdc = 0;
    DCE * dce;
    DC FAR * dc;
    WND * wndPtr;
    
    if (hwnd)
    {
	//!!TMP if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return 0;
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
	PushDS();
	SetDS(USER_HeapSel);
	for (hdce = firstDCE; (hdce); hdce = dce->hdceNext)
	{
		if (!(dce = (DCE *) LocalLock( hdce ))) 
		{
			PopDS();
			return 0;
		}
		if ((dce->byFlags == DCE_CACHE_DC) && (!dce->byInUse))
		{
			LocalUnlock(hdce);
			break;
		}
		LocalUnlock(hdce);
	}
	PopDS();
    }
    else hdce = wndPtr->hdce;

    if (!hdce) 
	{
		return 0;
	}

	// Fill DCE in USER local heap
	// So, switch DS to USER local heap and return back later
	PushDS();
	SetDS(USER_HeapSel);
	dce = (DCE *) LocalLock( hdce );

	dce->hwndCurr = hwnd;
	dce->byInUse       = TRUE;
	hdc = dce->hDC;
	LocalUnlock(hdce);
	PopDS();
    
    /* Initialize DC */
#if 0    
	// DC lives in GDI local heap, so we need to switch DS to GDI heap,
	// do all things and switch DS back
	PushDS();
	if (!(dc = (DC *) GDI_GetObjPtr( hdc, DC_MAGIC ))) return 0;

	DCE_SetDrawable( wndPtr, dc, flags );
	PopDS();
#endif
    if (hwnd)
    {
#if 0
        if (flags & DCX_PARENTCLIP)  /* Get a VisRgn for the parent */
        {
            WND *parentPtr = wndPtr->parent;
            DWORD newflags = flags & ~(DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN |
                                       DCX_WINDOW);
            if (parentPtr->dwStyle & WS_CLIPSIBLINGS)
                newflags |= DCX_CLIPSIBLINGS;
            hrgnVisible = DCE_GetVisRgn( parentPtr->hwndSelf, newflags );
            if (flags & DCX_WINDOW)
                OffsetRgn( hrgnVisible, -wndPtr->rectWindow.left,
                                        -wndPtr->rectWindow.top );
            else OffsetRgn( hrgnVisible, -wndPtr->rectClient.left,
                                         -wndPtr->rectClient.top );
        }
        else hrgnVisible = DCE_GetVisRgn( hwnd, flags );
#endif
    }
    else  /* Get a VisRgn for the whole screen */
    {
        hrgnVisible = CreateRectRgn( 0, 0, CXScreen, CYScreen);
    }

      /* Intersect VisRgn with the given region */

    if ((flags & DCX_INTERSECTRGN) || (flags & DCX_EXCLUDERGN))
    {
        CombineRgn( hrgnVisible, hrgnVisible, hrgnClip,
                    (flags & DCX_INTERSECTRGN) ? RGN_AND : RGN_DIFF );
    }
    SelectVisRgn( hdc, hrgnVisible );
    DeleteObject( hrgnVisible );

    TRACE("GetDCEx(%04x,%04x,0x%lx): returning %04x", 
	       hwnd, hrgnClip, flags, hdc);
    return hdc;
}


/***********************************************************************
 *           GetDC    (USER.66)
 */
HDC WINAPI GetDC(HWND hwnd)
{
	HDC res;
	res=GetDCEx(hwnd, 0, DCX_USESTYLE);
    return res;
}

#if 0

/***********************************************************************
 *           GetWindowDC    (USER.67)
 */
HDC GetWindowDC( HWND hwnd )
{
    int flags = DCX_CACHE | DCX_WINDOW;
    if (hwnd)
    {
	WND * wndPtr;
	if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return 0;
/*	if (wndPtr->dwStyle & WS_CLIPCHILDREN) flags |= DCX_CLIPCHILDREN; */
	if (wndPtr->dwStyle & WS_CLIPSIBLINGS) flags |= DCX_CLIPSIBLINGS;
    }
    return GetDCEx( hwnd, 0, flags );
}

#endif

/***********************************************************************
 *           ReleaseDC    (USER.68)
 */
int WINAPI ReleaseDC( HWND hwnd, HDC hdc )
{
	HANDLE hdce;
	DCE * dce = NULL;
    
	TRACE("ReleaseDC: %04x %04x\n", hwnd, hdc );

	// Here we need to search DCE which lives in USER local heap
	// So, switch to USER local heap, do things and switch back.
	PushDS();
	SetDS(USER_HeapSel);

	for (hdce = firstDCE; (hdce); hdce = dce->hdceNext)
	{
		if (!(dce = (DCE *) LocalLock( hdce )))
		{
			PopDS();
			return 0;
		}

		if (dce->byInUse && (dce->hDC == hdc))
		{
			break;
		}
		LocalUnlock(hdce);
	}

	if (!hdce) 
	{
		PopDS();
		return 0;
	}

	if (dce->byFlags == DCE_CACHE_DC)
	{
		SetDCState(dce->hDC, defaultDCstate);
		dce->byInUse = FALSE;
	}

	LocalUnlock(hdce);
	PopDS();
	return 1;
}
