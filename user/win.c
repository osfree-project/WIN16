/*
 * Window related functions
 *
 * Copyright 1993, 1994 Alexandre Julliard
 */

#include "user.h"
#include "dce.h"
#include "queue.h"

#define CallEnumTaskWndProc( func, hwnd, lParam ) \
    (*func)( hwnd, lParam )
#define CallEnumWindowsProc( func, hwnd, lParam ) \
    (*func)( hwnd, lParam )


extern HCURSOR CURSORICON_IconToCursor(HICON);

/***********************************************************************
 *           WIN_FindWndPtr
 *
 * Return a pointer to the WND structure corresponding to a HWND.
 */
WND * WIN_FindWndPtr( HWND hwnd )
{
	WND * ptr=NULL;

	if (hwnd)
	{
		ptr = (WND *) LocalLock(hwnd);
		if (ptr)
		{
			if ((ptr->dwMagic != WND_MAGIC) || (ptr->hwndSelf != hwnd))
			{
				ptr = NULL;
				LocalUnlock(hwnd);
			}
		}
	}
	return ptr;
}

/*****************************************************************
 *         WIN_GetTopParent
 *
 * Get the top-level parent for a child window.
 */
HWND WIN_GetTopParent( HWND hwnd )
{
    WND *wndPtr = WIN_FindWndPtr( hwnd );
    while (wndPtr && (wndPtr->dwStyle & WS_CHILD)) wndPtr = wndPtr->parent;
    return wndPtr ? wndPtr->hwndSelf : 0;
}

#if 0
/***********************************************************************
 *           WIN_DumpWindow
 *
 * Dump the content of a window structure to stderr.
 */
void WIN_DumpWindow( HWND hwnd )
{
    CLASS *classPtr;
    WND *ptr;
    char className[80];
    int i;

    if (!(ptr = WIN_FindWndPtr( hwnd )))
    {
        TRACE("%04x is not a window handle\n", hwnd );
        return;
    }

    if (!GetClassName( hwnd, className, sizeof(className ) ))
        lstrcpy( className, "#NULL#" );

    TRACE("Window %04x (%p):\n", hwnd, ptr );
    TRACE(
             "next=%p  child=%p  parent=%p  owner=%p  class=%04x '%s'\n"
             "inst=%04x  taskQ=%04x  updRgn=%04x  active=%04x hdce=%04x  idmenu=%04x\n"
             "style=%08lx  exstyle=%08lx  wndproc=%08lx  text=%04x '%s'\n"
             "client=%d,%d-%d,%d  window=%d,%d-%d,%d  iconpos=%d,%d  maxpos=%d,%d\n"
             "sysmenu=%04x  flags=%04x  props=%04x  vscroll=%04x  hscroll=%04x\n",
             ptr->next, ptr->child, ptr->parent, ptr->owner,
             ptr->hClass, className, ptr->hInstance, ptr->hmemTaskQ,
             ptr->hrgnUpdate, ptr->hwndLastActive, ptr->hdce, ptr->wIDmenu,
             ptr->dwStyle, ptr->dwExStyle, (DWORD)ptr->lpfnWndProc, ptr->hText,
             ptr->hText ? (char*)LocalLock(ptr->hText) : "",
             ptr->rectClient.left, ptr->rectClient.top, ptr->rectClient.right,
             ptr->rectClient.bottom, ptr->rectWindow.left, ptr->rectWindow.top,
             ptr->rectWindow.right, ptr->rectWindow.bottom, ptr->ptIconPos.x,
             ptr->ptIconPos.y, ptr->ptMaxPos.x, ptr->ptMaxPos.y, ptr->hSysMenu,
             ptr->flags, ptr->hProp, ptr->hVScroll, ptr->hHScroll );

    if ((classPtr = CLASS_FindClassPtr( ptr->hClass )) &&
        classPtr->wc.cbWndExtra)
    {
        TRACE("extra bytes:" );
        for (i = 0; i < classPtr->wc.cbWndExtra; i++)
            TRACE(" %02x", *((BYTE*)ptr->wExtra+i) );
    }
}


/***********************************************************************
 *           WIN_WalkWindows
 *
 * Walk the windows tree and print each window on stderr.
 */
void WIN_WalkWindows( HWND hwnd, int indent )
{
    WND *ptr;
    CLASS *classPtr;
    char className[80];

    ptr = hwnd ? WIN_FindWndPtr( hwnd ) : WIN_FindWndPtr(HWndDesktop);
    if (!ptr)
    {
        TRACE("*** Invalid window handle\n" );
        return;
    }

    if (!indent)  /* first time around */
        TRACE("%-16.16s %-8.8s %-6.6s %-17.17s %-8.8s %s\n",
                 "hwnd", " wndPtr", "queue", "Class Name", " Style", " WndProc");

    while (ptr)
    {
        TRACE("%*s%04x%*s", indent, "", ptr->hwndSelf, 13-indent,"");
        
        if (!(classPtr = CLASS_FindClassPtr( ptr->hClass ))) lstrcpy( className, "#NULL#" );
        else GlobalGetAtomName( classPtr->atomName, className, sizeof(className) );
        
        TRACE("%08lx %-6.4x %-17.17s %08x %04x:%04x\n",
                 (DWORD)ptr, ptr->hmemTaskQ, className,
                 (unsigned) ptr->dwStyle,
                 HIWORD(ptr->lpfnWndProc),
                 LOWORD(ptr->lpfnWndProc));
        
        if (ptr->child) WIN_WalkWindows( ptr->child->hwndSelf, indent+1 );
        ptr = ptr->next;
    }
}
#endif

/***********************************************************************
 *           WIN_UnlinkWindow
 *
 * Remove a window from the siblings linked list.
 */
BOOL WIN_UnlinkWindow( HWND hwnd )
{    
    WND *wndPtr, **ppWnd;

    if (!(wndPtr = WIN_FindWndPtr( hwnd )) || !wndPtr->parent) return FALSE;
    ppWnd = &wndPtr->parent->child;
    while (*ppWnd != wndPtr) ppWnd = &(*ppWnd)->next;
    *ppWnd = wndPtr->next;
    return TRUE;
}


/***********************************************************************
 *           WIN_LinkWindow
 *
 * Insert a window into the siblings linked list.
 * The window is inserted after the specified window, which can also
 * be specified as HWND_TOP or HWND_BOTTOM.
 */
BOOL WIN_LinkWindow( HWND hwnd, HWND hwndInsertAfter )
{    
    WND *wndPtr, **ppWnd;

    if (!(wndPtr = WIN_FindWndPtr( hwnd )) || !wndPtr->parent) return FALSE;

    if ((hwndInsertAfter == HWND_TOP) || (hwndInsertAfter == HWND_BOTTOM))
    {
        ppWnd = &wndPtr->parent->child;  /* Point to first sibling hwnd */
	if (hwndInsertAfter == HWND_BOTTOM)  /* Find last sibling hwnd */
	    while (*ppWnd) ppWnd = &(*ppWnd)->next;
    }
    else  /* Normal case */
    {
	WND * afterPtr = WIN_FindWndPtr( hwndInsertAfter );
	if (!afterPtr) return FALSE;
        ppWnd = &afterPtr->next;
    }
    wndPtr->next = *ppWnd;
    *ppWnd = wndPtr;
    return TRUE;
}


/***********************************************************************
 *           WIN_FindWinToRepaint
 *
 * Find a window that needs repaint.
 */
HWND WIN_FindWinToRepaint( HWND hwnd, HQUEUE hQueue )
{
    HWND hwndRet;
    WND *pWnd;

    /* Note: the desktop window never gets WM_PAINT messages */
    pWnd = hwnd ? WIN_FindWndPtr( hwnd ) : WIN_FindWndPtr(HWndDesktop)->child;

    for ( ; pWnd ; pWnd = pWnd->next )
    {
        if (!(pWnd->dwStyle & WS_VISIBLE) || (pWnd->flags & WIN_NO_REDRAW))
        {
            TRACE("FindWinToRepaint: skipping window %04x\n",
                         pWnd->hwndSelf );
            continue;
        }
        if ((pWnd->hmemTaskQ == hQueue) &&
            (pWnd->hrgnUpdate || (pWnd->flags & WIN_INTERNAL_PAINT))) break;
        
        if (pWnd->child )
            if ((hwndRet = WIN_FindWinToRepaint( pWnd->child->hwndSelf, hQueue )) )
                return hwndRet;
    }
    
    if (!pWnd) return 0;
    
    hwndRet = pWnd->hwndSelf;

    /* look among siblings if we got a transparent window */
    while (pWnd && ((pWnd->dwExStyle & WS_EX_TRANSPARENT) ||
                    !(pWnd->hrgnUpdate || (pWnd->flags & WIN_INTERNAL_PAINT))))
    {
        pWnd = pWnd->next;
    }
    if (pWnd) hwndRet = pWnd->hwndSelf;
//    TRACE("FindWinToRepaint: found %04x\n",hwndRet);
    return hwndRet;
}


/***********************************************************************
 *           WIN_SendParentNotify
 *
 * Send a WM_PARENTNOTIFY to all ancestors of the given window, unless
 * the window has the WS_EX_NOPARENTNOTIFY style.
 */
void WIN_SendParentNotify( HWND hwnd, WORD event, WORD idChild, LONG lValue )
{
    LPPOINT  lppt = (LPPOINT)(void*)(&lValue);
    WND     *wndPtr = WIN_FindWndPtr( hwnd );
    BOOL     bMouse = ((event <= WM_MOUSELAST) && (event >= WM_MOUSEFIRST));

    /* if lValue contains cursor coordinates they have to be
     * mapped to the client area of parent window */

    if (bMouse) MapWindowPoints(0, hwnd, lppt, 1);
#ifndef WINELIB32
    else lValue = MAKELONG( LOWORD(lValue), idChild );
#endif

    while (wndPtr)
    {
        if ((wndPtr->dwExStyle & WS_EX_NOPARENTNOTIFY) ||
	   !(wndPtr->dwStyle & WS_CHILD)) break;

        if (bMouse)
        {
	    lppt->x += wndPtr->rectClient.left;
	    lppt->y += wndPtr->rectClient.top;
        }

        wndPtr = wndPtr->parent;
#ifdef WINELIB32
	SendMessage( wndPtr->hwndSelf, WM_PARENTNOTIFY, 
		     MAKEWPARAM( event, idChild ), lValue );
#else
	SendMessage( wndPtr->hwndSelf, WM_PARENTNOTIFY, event, (LPARAM)lValue);
#endif
    }
}


/***********************************************************************
 *           WIN_DestroyWindow
 *
 * Destroy storage associated to a window
 */
static void WIN_DestroyWindow( HWND hwnd )
{
    WND *wndPtr = WIN_FindWndPtr( hwnd );
    CLASS *classPtr = CLASS_FindClassPtr( wndPtr->hClass );

#ifdef CONFIG_IPC
    if (main_block)
	DDE_DestroyWindow(hwnd);
#endif  /* CONFIG_IPC */
	
    if (!wndPtr || !classPtr) return;
    WIN_UnlinkWindow( hwnd ); /* Remove the window from the linked list */
    wndPtr->dwMagic = 0;  /* Mark it as invalid */
    wndPtr->hwndSelf = 0;
    if ((wndPtr->hrgnUpdate) || (wndPtr->flags & WIN_INTERNAL_PAINT))
    {
        if (wndPtr->hrgnUpdate) DeleteObject( wndPtr->hrgnUpdate );
        QUEUE_DecPaintCount( wndPtr->hmemTaskQ );
    }
    if (!(wndPtr->dwStyle & WS_CHILD))
    {
        if (wndPtr->wIDmenu) DestroyMenu( (HMENU)wndPtr->wIDmenu );
    }
    if (wndPtr->hSysMenu) DestroyMenu( wndPtr->hSysMenu );
//    if (wndPtr->window) XDestroyWindow( display, wndPtr->window );
    if (classPtr->wc.style & CS_OWNDC) DCE_FreeDCE( wndPtr->hdce );
    classPtr->cWindows--;
    LocalFree( hwnd );
}


/***********************************************************************
 *           WIN_CreateDesktopWindow
 *
 * Create the desktop window.
 */
BOOL WIN_CreateDesktopWindow(void)
{
    HCLASS hclass;
    CLASS *classPtr;
    HDC hdc;
	WND * pWndDesktop;

    if (!(hclass = CLASS_FindClassByName(DESKTOP_CLASS_ATOM, 0, &classPtr )))
	return FALSE;

    HWndDesktop = LocalAlloc(LMEM_FIXED, sizeof(WND)+classPtr->wc.cbWndExtra);
    if (!HWndDesktop) return FALSE;
    pWndDesktop = (WND *) LocalLock( HWndDesktop );

    pWndDesktop->next              = NULL;
    pWndDesktop->child             = NULL;
    pWndDesktop->parent            = NULL;
    pWndDesktop->owner             = NULL;
    pWndDesktop->dwMagic           = WND_MAGIC;
    pWndDesktop->hwndSelf          = HWndDesktop;
    pWndDesktop->hClass            = hclass;
    pWndDesktop->hInstance         = 0;
    pWndDesktop->rectWindow.left   = 0;
    pWndDesktop->rectWindow.top    = 0;
    pWndDesktop->rectWindow.right  = GetSystemMetrics(SM_CXSCREEN);
    pWndDesktop->rectWindow.bottom = GetSystemMetrics(SM_CYSCREEN);
    pWndDesktop->rectClient        = pWndDesktop->rectWindow;
    pWndDesktop->rectNormal        = pWndDesktop->rectWindow;
    pWndDesktop->ptIconPos.x       = -1;
    pWndDesktop->ptIconPos.y       = -1;
    pWndDesktop->ptMaxPos.x        = -1;
    pWndDesktop->ptMaxPos.y        = -1;
    pWndDesktop->hmemTaskQ         = 0; /* Desktop does not belong to a task */
    pWndDesktop->hrgnUpdate        = 0;
    pWndDesktop->hwndLastActive    = HWndDesktop;
    pWndDesktop->lpfnWndProc       = classPtr->wc.lpfnWndProc;
    pWndDesktop->dwStyle           = WS_VISIBLE | WS_CLIPCHILDREN |
                                     WS_CLIPSIBLINGS;
    pWndDesktop->dwExStyle         = 0;
    pWndDesktop->hdce              = 0;
    pWndDesktop->hVScroll          = 0;
    pWndDesktop->hHScroll          = 0;
    pWndDesktop->wIDmenu           = 0;
    pWndDesktop->hText             = 0;
    pWndDesktop->flags             = 0;
    pWndDesktop->hSysMenu          = 0;
    pWndDesktop->hProp	      = 0;
    SendMessage( HWndDesktop, WM_NCCREATE, 0, 0 );
    if ((hdc = GetDC( HWndDesktop )) != 0)
    {
        SendMessage( HWndDesktop, WM_ERASEBKGND, hdc, 0 );
        ReleaseDC( HWndDesktop, hdc );
    }
    return TRUE;
}


/***********************************************************************
 *           CreateWindowEx   (USER.452)
 */
HWND WINAPI CreateWindowEx( DWORD exStyle, LPCSTR className, LPCSTR windowName,
		     DWORD style, int x, int y, int width, int height,
		     HWND parent, HMENU menu, HANDLE instance, LPVOID data ) 
{
    HANDLE class, hwnd;
    CLASS *classPtr;
    WND *wndPtr;
    POINT maxSize, maxPos, minTrack, maxTrack;
    CREATESTRUCT createStruct;
    int wmcreate;

//    TRACE("CreateWindowEx: " );
//    if (HIWORD(windowName))
//{
//	TRACE("'%S' ", windowName);
//}
//    else
//{
//	TRACE("%04x ", LOWORD(windowName) );
//}

//    if (HIWORD(className))
//{
//        TRACE("'%S' ", className);
//}
//    else
//{
//        TRACE("%04x ", LOWORD(className) );
//}

//    TRACE("%08lx %08lx %d,%d %dx%d %04x %04x %04x %08lx\n",
//		exStyle, style, x, y, width, height,
//		parent, menu, instance, (DWORD)data);

    if (x == CW_USEDEFAULT) x = y = 0;
    if (width == CW_USEDEFAULT)
    {
	width = 600;
	height = 400;
    }

      /* Find the parent and class */

    if (parent)
    {
	/* Make sure parent is valid */
        if (!IsWindow( parent )) {
	    TRACE("CreateWindowEx: Parent %04x is not a window\n", parent);
	    return 0;
	}
    }
    else 
    {
	if (style & WS_CHILD) {
	    TRACE("CreateWindowEx: no parent\n");
	    return 0;  /* WS_CHILD needs a parent */
	}
    }

    if (!(class = CLASS_FindClassByName( className, GetExePtr(instance),
                                         &classPtr )))
    {
        TRACE("CreateWindow BAD CLASSNAME " );
        if (HIWORD(className))
	{
	  TRACE("'%S'\n", className);
	}
        else 
	{
		TRACE("%04x\n", LOWORD(className) );
	}
        return 0;
    }
      /* Correct the window style */

    if (!(style & (WS_POPUP | WS_CHILD)))  /* Overlapped window */
	style |= WS_CAPTION | WS_CLIPSIBLINGS;
    if (exStyle & WS_EX_DLGMODALFRAME) style &= ~WS_THICKFRAME;

      /* Create the window structure */

    hwnd = LocalAlloc(LMEM_FIXED, sizeof(WND)+classPtr->wc.cbWndExtra );
    if (!hwnd) {
	TRACE("CreateWindowEx: Out of memory\n");
	return 0;
    }

      /* Fill the structure */

    wndPtr = (WND *) LocalLock( hwnd );
    wndPtr->next           = NULL;
    wndPtr->child          = NULL;
    wndPtr->parent         = (style & WS_CHILD) ? WIN_FindWndPtr( parent ) : WIN_FindWndPtr(HWndDesktop);
    wndPtr->owner          = (style & WS_CHILD) ? NULL : WIN_FindWndPtr(WIN_GetTopParent(parent));
    wndPtr->dwMagic        = WND_MAGIC;
    wndPtr->hwndSelf       = hwnd;
    wndPtr->hClass         = class;
    wndPtr->hInstance      = instance;
    wndPtr->ptIconPos.x    = -1;
    wndPtr->ptIconPos.y    = -1;
    wndPtr->ptMaxPos.x     = -1;
    wndPtr->ptMaxPos.y     = -1;
    wndPtr->hmemTaskQ      = GetTaskQueue(0);
    wndPtr->hrgnUpdate     = 0;
    wndPtr->hwndLastActive = hwnd;
    wndPtr->lpfnWndProc    = classPtr->wc.lpfnWndProc;
    wndPtr->dwStyle        = style & ~WS_VISIBLE;
    wndPtr->dwExStyle      = exStyle;
    wndPtr->wIDmenu        = 0;
    wndPtr->hText          = 0;
    wndPtr->flags          = 0;
    wndPtr->hVScroll       = 0;
    wndPtr->hHScroll       = 0;
    wndPtr->hSysMenu       = 0;
    wndPtr->hProp          = 0;

    if (classPtr->wc.cbWndExtra)
	_fmemset( wndPtr->wExtra, 0, classPtr->wc.cbWndExtra );
    classPtr->cWindows++;

      /* Get class or window DC if needed */

    if (classPtr->wc.style & CS_OWNDC)
        wndPtr->hdce = DCE_AllocDCE( DCE_WINDOW_DC );
    else if (classPtr->wc.style & CS_CLASSDC)
        wndPtr->hdce = classPtr->hdce;
    else
        wndPtr->hdce = 0;


      /* Insert the window in the linked list */

    WIN_LinkWindow( hwnd, HWND_TOP );

      /* Send the WM_GETMINMAXINFO message and fix the size if needed */

    NC_GetMinMaxInfo( hwnd, &maxSize, &maxPos, &minTrack, &maxTrack );

    if (maxSize.x < width) width = maxSize.x;
    if (maxSize.y < height) height = maxSize.y;
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;

    wndPtr->rectWindow.left   = x;
    wndPtr->rectWindow.top    = y;
    wndPtr->rectWindow.right  = x + width;
    wndPtr->rectWindow.bottom = y + height;
    wndPtr->rectClient        = wndPtr->rectWindow;
    wndPtr->rectNormal        = wndPtr->rectWindow;

    
    if ((style & WS_CAPTION) && !(style & WS_CHILD))
    {
        if (menu) SetMenu(hwnd, menu);
        else if (classPtr->wc.lpszMenuName)
            SetMenu( hwnd, LoadMenu( instance, classPtr->wc.lpszMenuName ) );
    }
    else wndPtr->wIDmenu = (UINT)menu;


//@todo    GetSystemMenu( hwnd, TRUE );  /* Create a copy of the system menu */


      /* Send the WM_CREATE message */

    createStruct.lpCreateParams = (LPSTR)data;
    createStruct.hInstance      = instance;
    createStruct.hMenu          = menu;
    createStruct.hwndParent     = parent;
    createStruct.cx             = width;
    createStruct.cy             = height;
    createStruct.x              = x;
    createStruct.y              = y;
    createStruct.style          = style;
    createStruct.lpszName       = windowName;
    createStruct.lpszClass      = className;
    createStruct.dwExStyle      = 0;


    wmcreate = SendMessage( hwnd, WM_NCCREATE, 0, (LPARAM)(CREATESTRUCT FAR *)(&createStruct) );


    if (!wmcreate)
    {
	TRACE("CreateWindowEx: WM_NCCREATE return 0\n");
	wmcreate = -1;
    }
    else
    {
	WINPOS_SendNCCalcSize( hwnd, FALSE, &wndPtr->rectWindow,
			       NULL, NULL, NULL, &wndPtr->rectClient );
	wmcreate = SendMessage(hwnd, WM_CREATE, 0, (LPARAM)(CREATESTRUCT FAR *)(&createStruct));
    }

    if (wmcreate == -1)
    {
	  /* Abort window creation */
	TRACE("CreateWindowEx: wmcreate==-1, aborting\n");
        WIN_DestroyWindow( hwnd );
	return 0;
    }

    WIN_SendParentNotify( hwnd, WM_CREATE, wndPtr->wIDmenu, (LONG)hwnd );

    /* Show the window, maximizing or minimizing if needed */

    if (wndPtr->dwStyle & WS_MINIMIZE)
    {
        wndPtr->dwStyle &= ~WS_MAXIMIZE;
        WINPOS_FindIconPos( hwnd );
        SetWindowPos( hwnd, 0, wndPtr->ptIconPos.x, wndPtr->ptIconPos.y,
                      GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
                      SWP_FRAMECHANGED |
                      (style & WS_VISIBLE) ? SWP_SHOWWINDOW : 0 );
    }
    else if (wndPtr->dwStyle & WS_MAXIMIZE)
    {
        SetWindowPos( hwnd, 0, maxPos.x, maxPos.y, maxSize.x, maxSize.y,
                      SWP_FRAMECHANGED |
                      (style & WS_VISIBLE) ? SWP_SHOWWINDOW : 0 );
    }
    else if (style & WS_VISIBLE) ShowWindow( hwnd, SW_SHOW );

//    TRACE("CreateWindowEx: return %04x\n", hwnd);
    return hwnd;
}


/***********************************************************************
 *           DestroyWindow   (USER.53)
 */
BOOL WINAPI DestroyWindow( HWND hwnd )
{
    WND * wndPtr;
    CLASS * classPtr;

    TRACE("DestroyWindow(%04x)\n", hwnd);
    
      /* Initialisation */

    if (hwnd == HWndDesktop) return FALSE; /* Can't destroy desktop*/
    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return FALSE;
    if (!(classPtr = CLASS_FindClassPtr( wndPtr->hClass ))) return FALSE;

      /* Hide the window */

    if (wndPtr->dwStyle & WS_VISIBLE)
	SetWindowPos( hwnd, 0, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOACTIVATE |
		      SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE );
    if ((hwnd == GetCapture()) || IsChild( hwnd, GetCapture() ))
	ReleaseCapture();
    WIN_SendParentNotify( hwnd, WM_DESTROY, wndPtr->wIDmenu, (LONG)hwnd );

      /* Recursively destroy owned windows */

    for (;;)
    {
        WND *siblingPtr = wndPtr->parent->child;  /* First sibling */
        while (siblingPtr)
        {
            if (siblingPtr->owner == wndPtr) break;
            siblingPtr = siblingPtr->next;
        }
        if (siblingPtr) DestroyWindow( siblingPtr->hwndSelf );
        else break;
    }

      /* Send destroy messages and destroy children */

    SendMessage( hwnd, WM_DESTROY, 0, 0 );
    while (wndPtr->child)  /* The child removes itself from the list */
	DestroyWindow( wndPtr->child->hwndSelf );
    SendMessage( hwnd, WM_NCDESTROY, 0, 0 );

      /* Destroy the window */

    WIN_DestroyWindow( hwnd );
    return TRUE;
}

/***********************************************************************
 *           CloseWindow   (USER.43)
 */
VOID WINAPI CloseWindow(HWND hWnd)
{
    WND * wndPtr = WIN_FindWndPtr(hWnd);
    if (wndPtr->dwStyle & WS_CHILD) return ;//TRUE;
    ShowWindow(hWnd, SW_MINIMIZE);
    return ;//TRUE;
}

 
/***********************************************************************
 *           OpenIcon   (USER.44)
 */
BOOL WINAPI OpenIcon(HWND hWnd)
{
    if (!IsIconic(hWnd)) return FALSE;
    ShowWindow(hWnd, SW_SHOWNORMAL);
    return(TRUE);
}

 
/***********************************************************************
 *           FindWindow   (USER.50)
 */
HWND WINAPI FindWindow( LPCSTR ClassMatch, LPCSTR TitleMatch )
{
    HCLASS hclass;
    CLASS *classPtr;
    WND *wndPtr;

    if (ClassMatch)
    {
	hclass = CLASS_FindClassByName( ClassMatch, (HINSTANCE)0xffff,
                                        &classPtr );
	if (!hclass) return 0;
    }
    else hclass = 0;

    wndPtr = ((WND *)LocalLock(HWndDesktop))->child;
    while (wndPtr)
    {
	if (!hclass || (wndPtr->hClass == hclass))
	{
	      /* Found matching class */
	    if (!TitleMatch) return wndPtr->hwndSelf;
	    if (wndPtr->hText)
	    {
		char *textPtr = (char *) LocalLock(wndPtr->hText);
		if (!lstrcmp( textPtr, TitleMatch )) return wndPtr->hwndSelf;
	    }
	}
        wndPtr = wndPtr->next;
    }
    return 0;
}
 

/**********************************************************************
 *           WIN_GetDesktop
 */
WND *WIN_GetDesktop(void)
{
    return (WND *)LocalLock(HWndDesktop);
}


/**********************************************************************
 *           GetDesktopWindow   (USER.286)
 */
HWND WINAPI GetDesktopWindow(void)
{
    return HWndDesktop;
}


/**********************************************************************
 *           GetDesktopHwnd   (USER.278)
 *
 * Exactly the same thing as GetDesktopWindow(), but not documented.
 * Don't ask me why...
 */
HWND WINAPI GetDesktopHwnd(void)
{
    return HWndDesktop;
}


/*******************************************************************
 *           EnableWindow   (USER.34)
 */
BOOL WINAPI EnableWindow( HWND hwnd, BOOL enable )
{
    WND *wndPtr;

    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return FALSE;
    if (enable && (wndPtr->dwStyle & WS_DISABLED))
    {
	  /* Enable window */
	wndPtr->dwStyle &= ~WS_DISABLED;
	SendMessage( hwnd, WM_ENABLE, TRUE, 0 );
	return TRUE;
    }
    else if (!enable && !(wndPtr->dwStyle & WS_DISABLED))
    {
	  /* Disable window */
	wndPtr->dwStyle |= WS_DISABLED;
	if ((hwnd == GetFocus()) || IsChild( hwnd, GetFocus() ))
	    SetFocus( 0 );  /* A disabled window can't have the focus */
	if ((hwnd == GetCapture()) || IsChild( hwnd, GetCapture() ))
	    ReleaseCapture();  /* A disabled window can't capture the mouse */
	SendMessage( hwnd, WM_ENABLE, FALSE, 0 );
	return FALSE;
    }
    return ((wndPtr->dwStyle & WS_DISABLED) != 0);
}


/***********************************************************************
 *           IsWindowEnabled   (USER.35)
 */ 
BOOL WINAPI IsWindowEnabled(HWND hWnd)
{
    WND * wndPtr; 

    if (!(wndPtr = WIN_FindWndPtr(hWnd))) return FALSE;
    return !(wndPtr->dwStyle & WS_DISABLED);
}


/**********************************************************************
 *	     GetWindowWord    (USER.133)
 */
WORD WINAPI GetWindowWord( HWND hwnd, int offset )
{
    WND * wndPtr = WIN_FindWndPtr( hwnd );
	FUNCTION_START
    if (!wndPtr) return 0;
    if (offset >= 0) return *(WORD *)(((char *)wndPtr->wExtra) + offset);
    switch(offset)
    {
	case GWW_ID:         return wndPtr->wIDmenu;
#ifdef WINELIB32
        case GWW_HWNDPARENT:
        case GWW_HINSTANCE: 
            fprintf(stderr,"GetWindowWord called with offset %d.\n",offset);
            return 0;
#else
	case GWW_HWNDPARENT: return wndPtr->parent ? wndPtr->parent->hwndSelf : 0;
	case GWW_HINSTANCE:  return (WORD)wndPtr->hInstance;
#endif
    }
    return 0;
}


/**********************************************************************
 *	     WIN_GetWindowInstance
 */
HINSTANCE WIN_GetWindowInstance(HWND hwnd)
{
    WND * wndPtr = WIN_FindWndPtr( hwnd );
	FUNCTION_START
    if (!wndPtr) return (HINSTANCE)0;
    return wndPtr->hInstance;
}


/**********************************************************************
 *	     SetWindowWord    (USER.134)
 */
WORD WINAPI SetWindowWord( HWND hwnd, int offset, WORD newval )
{
    WORD *ptr, retval;
    WND * wndPtr = WIN_FindWndPtr( hwnd );
	FUNCTION_START
    if (!wndPtr) return 0;
    if (offset >= 0) ptr = (WORD *)(((char *)wndPtr->wExtra) + offset);
    else switch(offset)
    {
#ifdef WINELIB32
	case GWW_ID:
	case GWW_HINSTANCE:
            fprintf(stderr,"SetWindowWord called with offset %d.\n",offset);
            return 0;
#else
	case GWW_ID:        ptr = &wndPtr->wIDmenu;   break;
	case GWW_HINSTANCE: ptr = (WORD*)&wndPtr->hInstance; break;
#endif
	default: return 0;
    }
    retval = *ptr;
    *ptr = newval;
    return retval;
}


/**********************************************************************
 *	     GetWindowLong    (USER.135)
 */
LONG WINAPI GetWindowLong( HWND hwnd, int offset )
{
    WND * wndPtr = WIN_FindWndPtr( hwnd );
	FUNCTION_START
    if (!wndPtr) return 0;
    if (offset >= 0) return *(LONG *)(((char *)wndPtr->wExtra) + offset);
    switch(offset)
    {
	case GWL_STYLE:   return wndPtr->dwStyle;
        case GWL_EXSTYLE: return wndPtr->dwExStyle;
	case GWL_WNDPROC: return (LONG)wndPtr->lpfnWndProc;
#ifdef WINELIB32
	case GWW_HWNDPARENT: return wndPtr->parent ? wndPtr->parent->hwndSelf : 0;
	case GWW_HINSTANCE:  return wndPtr->hInstance;
#endif
    }
    return 0;
}


/**********************************************************************
 *	     SetWindowLong    (USER.136)
 */
LONG WINAPI SetWindowLong( HWND hwnd, int offset, LONG newval )
{
    LONG *ptr, retval;
    WND * wndPtr = WIN_FindWndPtr( hwnd );
	FUNCTION_START
    if (!wndPtr) return 0;
    if (offset >= 0) ptr = (LONG *)(((char *)wndPtr->wExtra) + offset);
    else switch(offset)
    {
	case GWL_STYLE:   ptr = &wndPtr->dwStyle; break;
        case GWL_EXSTYLE: ptr = &wndPtr->dwExStyle; break;
	case GWL_WNDPROC: ptr = (LONG *)&wndPtr->lpfnWndProc; break;
	default: return 0;
    }
    retval = *ptr;
    *ptr = newval;
    return retval;
}


/*******************************************************************
 *         GetWindowText          (USER.36)
 */

int WINAPI GetWindowText( HWND hwnd, LPSTR lpString, int nMaxCount )
{
    int len;

    len = (int)SendMessage( hwnd, WM_GETTEXT, (WPARAM)nMaxCount, 
                            (LPARAM)lpString);
    return len;
}


/*******************************************************************
 *         SetWindowText          (USER.37)
 */
void WINAPI SetWindowText( HWND hwnd, LPCSTR lpString )
{
      /* We have to allocate a buffer on the USER heap */
      /* to be able to pass its address to 16-bit code */
    SendMessage( hwnd, WM_SETTEXT, 0, (LPARAM)lpString);
}


/*******************************************************************
 *         GetWindowTextLength    (USER.38)
 */
int WINAPI GetWindowTextLength(HWND hwnd)
{
    return (int)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0 );
}


/*******************************************************************
 *         IsWindow    (USER.47)
 */
BOOL WINAPI IsWindow( HWND hwnd )
{
    WND * wndPtr = WIN_FindWndPtr( hwnd );
    return ((wndPtr != NULL) && (wndPtr->dwMagic == WND_MAGIC));
}


/*****************************************************************
 *         GetParent              (USER.46)
 */
HWND WINAPI GetParent(HWND hwnd)
{
    WND *wndPtr = WIN_FindWndPtr(hwnd);
    if (!wndPtr) return 0;
    wndPtr = (wndPtr->dwStyle & WS_CHILD) ? wndPtr->parent : wndPtr->owner;
    return wndPtr ? wndPtr->hwndSelf : 0;
}


/*****************************************************************
 *         SetParent              (USER.233)
 */
HWND WINAPI SetParent(HWND hwndChild, HWND hwndNewParent)
{
    HWND oldParent;

    WND *wndPtr = WIN_FindWndPtr(hwndChild);
    WND *pWndParent = WIN_FindWndPtr( hwndNewParent );
    if (!wndPtr || !pWndParent || !(wndPtr->dwStyle & WS_CHILD)) return 0;

    oldParent = wndPtr->parent->hwndSelf;

    WIN_UnlinkWindow(hwndChild);
    if (hwndNewParent) wndPtr->parent = pWndParent;
    WIN_LinkWindow(hwndChild, HWND_BOTTOM);
    
    if (IsWindowVisible(hwndChild)) UpdateWindow(hwndChild);
    
    return oldParent;
}


/*******************************************************************
 *         IsChild    (USER.48)
 */
BOOL WINAPI IsChild( HWND parent, HWND child )
{
    WND * wndPtr = WIN_FindWndPtr( child );
    while (wndPtr && (wndPtr->dwStyle & WS_CHILD))
    {
        wndPtr = wndPtr->parent;
	if (wndPtr->hwndSelf == parent) return TRUE;
    }
    return FALSE;
}


/***********************************************************************
 *           IsWindowVisible   (USER.49)
 */
BOOL WINAPI IsWindowVisible( HWND hwnd )
{
    WND *wndPtr = WIN_FindWndPtr( hwnd );
    while (wndPtr && (wndPtr->dwStyle & WS_CHILD))
    {
        if (!(wndPtr->dwStyle & WS_VISIBLE)) return FALSE;
        wndPtr = wndPtr->parent;
    }
    return (wndPtr && (wndPtr->dwStyle & WS_VISIBLE));
}


/*******************************************************************
 *         GetTopWindow    (USER.229)
 */
HWND WINAPI GetTopWindow( HWND hwnd )
{
    WND * wndPtr = WIN_FindWndPtr( hwnd );
    if (wndPtr && wndPtr->child) return wndPtr->child->hwndSelf;
    else return 0;
}


/*******************************************************************
 *         GetWindow    (USER.262)
 */
HWND WINAPI GetWindow( HWND hwnd, UINT rel )
{
    WND * wndPtr = WIN_FindWndPtr( hwnd );
    if (!wndPtr) return 0;
    switch(rel)
    {
    case GW_HWNDFIRST:
        if (wndPtr->parent) return wndPtr->parent->child->hwndSelf;
	else return 0;
	
    case GW_HWNDLAST:
	if (!wndPtr->parent) return 0;  /* Desktop window */
	while (wndPtr->next) wndPtr = wndPtr->next;
        return wndPtr->hwndSelf;
	
    case GW_HWNDNEXT:
        if (!wndPtr->next) return 0;
	return wndPtr->next->hwndSelf;
	
    case GW_HWNDPREV:
        if (!wndPtr->parent) return 0;  /* Desktop window */
        wndPtr = wndPtr->parent->child;  /* First sibling */
        if (wndPtr->hwndSelf == hwnd) return 0;  /* First in list */
        while (wndPtr->next)
        {
            if (wndPtr->next->hwndSelf == hwnd) return wndPtr->hwndSelf;
            wndPtr = wndPtr->next;
        }
        return 0;
	
    case GW_OWNER:
	return wndPtr->owner ? wndPtr->owner->hwndSelf : 0;

    case GW_CHILD:
	return wndPtr->child ? wndPtr->child->hwndSelf : 0;
    }
    return 0;
}


/*******************************************************************
 *         GetNextWindow    (USER.230)
 */
HWND WINAPI WINAPI GetNextWindow( HWND hwnd, UINT flag )
{
    if ((flag != GW_HWNDNEXT) && (flag != GW_HWNDPREV)) return 0;
    return GetWindow( hwnd, flag );
}

/*******************************************************************
 *         ShowOwnedPopups  (USER.265)
 */
void WINAPI ShowOwnedPopups( HWND owner, BOOL fShow )
{
    WND *pWnd = (WND *)LocalLock(HWndDesktop);
	pWnd=pWnd->child;
	LocalUnlock(HWndDesktop);
    while (pWnd)
    {
        if (pWnd->owner && (pWnd->owner->hwndSelf == owner) &&
            (pWnd->dwStyle & WS_POPUP))
            ShowWindow( pWnd->hwndSelf, fShow ? SW_SHOW : SW_HIDE );
        pWnd = pWnd->next;
    }
}


/*******************************************************************
 *         GetLastActivePopup    (USER.287)
 */
HWND WINAPI GetLastActivePopup(HWND hwnd)
{
    WND *wndPtr;
    wndPtr = WIN_FindWndPtr(hwnd);
    if (wndPtr == NULL) return hwnd;
    return wndPtr->hwndLastActive;
}


/*******************************************************************
 *           EnumWindows   (USER.54)
 */
BOOL WINAPI EnumWindows( WNDENUMPROC lpEnumFunc, LPARAM lParam )
{
    WND *wndPtr;
    WND *pWndDesktop;
    HWND *list, *pWnd;
    HLOCAL hlist;
    int count;

    /* We have to build a list of all windows first, to avoid */
    /* unpleasant side-effects, for instance if the callback  */
    /* function changes the Z-order of the windows.           */

      /* First count the windows */

    pWndDesktop=(WND *)LocalLock(HWndDesktop);
    
    count = 0;
    for (wndPtr = pWndDesktop->child; wndPtr; wndPtr = wndPtr->next) count++;
    if (!count) return TRUE;

      /* Now build the list of all windows */
    hlist=LocalAlloc(LMEM_FIXED, sizeof(HWND) * count );
    if (!(pWnd = list = (HWND *)LocalLock(hlist))) return FALSE;
    for (wndPtr = pWndDesktop->child; wndPtr; wndPtr = wndPtr->next)
        *pWnd++ = wndPtr->hwndSelf;

      /* Now call the callback function for every window */

    for (pWnd = list; count > 0; count--, pWnd++)
    {
          /* Make sure that window still exists */
        if (!IsWindow(*pWnd)) continue;
        if (!CallEnumWindowsProc( lpEnumFunc, *pWnd, lParam )) break;
    }
    LocalUnlock(hlist);
    LocalFree(hlist );
    return TRUE;
}


/**********************************************************************
 *           EnumTaskWindows   (USER.225)
 */
BOOL WINAPI EnumTaskWindows( HTASK hTask, WNDENUMPROC lpEnumFunc, LPARAM lParam )
{
    WND *wndPtr;
    WND *pWndDesktop=(WND *)LocalLock(HWndDesktop);
    HWND *list, *pWnd;
    HLOCAL hlist;
    HANDLE hQueue = GetTaskQueue( hTask );
    int count;

    /* This function is the same as EnumWindows(),    */
    /* except for an added check on the window queue. */

      /* First count the windows */

    count = 0;
    for (wndPtr = pWndDesktop->child; wndPtr; wndPtr = wndPtr->next)
        if (wndPtr->hmemTaskQ == hQueue) count++;
    if (!count) return TRUE;

      /* Now build the list of all windows */
    hlist=LocalAlloc(LMEM_FIXED, sizeof(HWND) * count );
    if (!(pWnd = list = (HWND *)LocalLock(hlist))) return FALSE;
    for (wndPtr = pWndDesktop->child; wndPtr; wndPtr = wndPtr->next)
        if (wndPtr->hmemTaskQ == hQueue) *pWnd++ = wndPtr->hwndSelf;

      /* Now call the callback function for every window */

    for (pWnd = list; count > 0; count--, pWnd++)
    {
          /* Make sure that window still exists */
        if (!IsWindow(*pWnd)) continue;
        if (!CallEnumTaskWndProc( lpEnumFunc, *pWnd, lParam )) break;
    }
    LocalUnlock(hlist);
    LocalFree(hlist);
    return TRUE;
}


/*******************************************************************
 *    WIN_EnumChildWin
 *
 *   o hwnd is the first child to use, loop until all next windows
 *     are processed
 * 
 *   o call wdnenumprc
 *
 *   o call ourselves with the next child window
 * 
 */
static BOOL WIN_EnumChildWin( WND *wndPtr, FARPROC wndenumprc, LPARAM lParam )
{
    WND *pWndNext, *pWndChild;
    while (wndPtr)
    {
        pWndNext = wndPtr->next;    /* storing hwnd is a way to avoid.. */
        pWndChild = wndPtr->child;  /* ..side effects after wndenumprc  */
        if (!CallEnumWindowsProc( wndenumprc, wndPtr->hwndSelf, lParam ))
            return 0;
        if (pWndChild && IsWindow(pWndChild->hwndSelf))
            if (!WIN_EnumChildWin(pWndChild, wndenumprc, lParam)) return 0;
        wndPtr = pWndNext;
    } 
    return 1;
}


/*******************************************************************
 *    EnumChildWindows        (USER.55)
 *
 *   o gets the first child of hwnd
 *
 *   o calls WIN_EnumChildWin to do a recursive decent of child windows
 */
BOOL WINAPI EnumChildWindows(HWND hwnd, WNDENUMPROC wndenumprc, LPARAM lParam)
{
    WND *wndPtr;

//    dprintf_enum(stddeb,"EnumChildWindows\n");

    if (hwnd == 0) return 0;
    if (!(wndPtr = WIN_FindWndPtr(hwnd))) return 0;
    return WIN_EnumChildWin(wndPtr->child, wndenumprc, lParam);
}


/*******************************************************************
 *           AnyPopup   (USER.52)
 */
BOOL WINAPI AnyPopup(void)
{
    WND *wndPtr;
    WND *pWndDesktop=(WND *)LocalLock(HWndDesktop);

    for (wndPtr = pWndDesktop->child; wndPtr; wndPtr = wndPtr->next)
        if (wndPtr->owner && (wndPtr->dwStyle & WS_VISIBLE)) return TRUE;
    return FALSE;
}

/*******************************************************************
 *			FlashWindow		[USER.105]
 */
BOOL WINAPI FlashWindow(HWND hWnd, BOOL bInvert)
{
    WND *wndPtr = WIN_FindWndPtr(hWnd);

//    dprintf_win(stddeb,"FlashWindow: %04x\n", hWnd);

    if (!wndPtr) return FALSE;

    if (wndPtr->dwStyle & WS_MINIMIZE)
    {
        if (bInvert && !(wndPtr->flags & WIN_NCACTIVATED))
        {
            HDC hDC = GetDC(hWnd);
            
            if (!SendMessage( hWnd, WM_ERASEBKGND, (WPARAM)hDC, (LPARAM)0 ))
                wndPtr->flags |= WIN_NEEDS_ERASEBKGND;
            
            ReleaseDC( hWnd, hDC );
            wndPtr->flags |= WIN_NCACTIVATED;
        }
        else
        {
            RedrawWindow( hWnd, 0, 0, RDW_INVALIDATE | RDW_ERASE |
                          RDW_UPDATENOW | RDW_FRAME );
            wndPtr->flags &= ~WIN_NCACTIVATED;
        }
        return TRUE;
    }
    else
    {
        WPARAM wparam;
        if (bInvert) wparam = !(wndPtr->flags & WIN_NCACTIVATED);
        else wparam = (hWnd == GetActiveWindow());

        SendMessage( hWnd, WM_NCACTIVATE, wparam, (LPARAM)0 );
        return wparam;
    }
}


/*******************************************************************
 *			SetSysModalWindow		[USER.188]
 */
HWND WINAPI SetSysModalWindow(HWND hWnd)
{
    HWND hWndOldModal = hwndSysModal;
    hwndSysModal = hWnd;
//    dprintf_win(stdnimp,"EMPTY STUB !! SetSysModalWindow(%04x) !\n", hWnd);
    return hWndOldModal;
}

#if 0

/*******************************************************************
 *			DRAG_QueryUpdate
 *
 * recursively find a child that contains spDragInfo->pt point 
 * and send WM_QUERYDROPOBJECT
 */
BOOL DRAG_QueryUpdate( HWND hQueryWnd, SEGPTR spDragInfo )
{
 BOOL		wParam,bResult = 0;
 POINT		pt;
 LPDRAGINFO	ptrDragInfo = (LPDRAGINFO) PTR_SEG_TO_LIN(spDragInfo);
 WND 	       *ptrQueryWnd = WIN_FindWndPtr(hQueryWnd),*ptrWnd;
 RECT		tempRect;	/* this sucks */

 if( !ptrQueryWnd || !ptrDragInfo ) return 0;

 pt 		= ptrDragInfo->pt;

 GetWindowRect(hQueryWnd,&tempRect); 

 if( !PtInRect(&tempRect,pt) ||
     (ptrQueryWnd->dwStyle & WS_DISABLED) )
	return 0;

 if( !(ptrQueryWnd->dwStyle & WS_MINIMIZE) ) 
   {
     tempRect = ptrQueryWnd->rectClient;
     if(ptrQueryWnd->dwStyle & WS_CHILD)
        MapWindowPoints(ptrQueryWnd->parent->hwndSelf,0,(LPPOINT)&tempRect,2);

     if( PtInRect(&tempRect,pt) )
	{
	 wParam = 0;
         
	 for (ptrWnd = ptrQueryWnd->child; ptrWnd ;ptrWnd = ptrWnd->next)
             if( ptrWnd->dwStyle & WS_VISIBLE )
	     {
                 GetWindowRect(ptrWnd->hwndSelf,&tempRect);

                 if( PtInRect(&tempRect,pt) ) 
                     break;
	     }

	 if(ptrWnd)
         {
	    dprintf_msg(stddeb,"DragQueryUpdate: hwnd = %04x, %d %d - %d %d\n",
                        ptrWnd->hwndSelf, ptrWnd->rectWindow.left, ptrWnd->rectWindow.top,
			ptrWnd->rectWindow.right, ptrWnd->rectWindow.bottom );
            if( !(ptrWnd->dwStyle & WS_DISABLED) )
	        bResult = DRAG_QueryUpdate(ptrWnd->hwndSelf, spDragInfo);
         }

	 if(bResult) return bResult;
	}
     else wParam = 1;
   }
 else wParam = 1;

 ScreenToClient(hQueryWnd,&ptrDragInfo->pt);

 ptrDragInfo->hScope = hQueryWnd;

 bResult = SendMessage( hQueryWnd ,WM_QUERYDROPOBJECT ,
                       (WPARAM)wParam ,(LPARAM) spDragInfo );
 if( !bResult ) 
      ptrDragInfo->pt = pt;

 return bResult;
}

/*******************************************************************
 *                      DragDetect ( USER.465 )
 *
 */
BOOL DragDetect(HWND hWnd, POINT pt)
{
  MSG   msg;
  RECT  rect;

  rect.left = pt.x - wDragWidth;
  rect.right = pt.x + wDragWidth;

  rect.top = pt.y - wDragHeight;
  rect.bottom = pt.y + wDragHeight;

  SetCapture(hWnd);

  while(1)
   {
        while(PeekMessage(&msg ,0 ,WM_MOUSEFIRST ,WM_MOUSELAST ,PM_REMOVE))
         {
           if( msg.message == WM_LBUTTONUP )
                {
                  ReleaseCapture();
                  return 0;
                }
           if( msg.message == WM_MOUSEMOVE )
		{
		  POINT pt = { LOWORD(msg.lParam), HIWORD(msg.lParam) };
                  if( !PtInRect( &rect, pt ) )
                    {
                      ReleaseCapture();
                      return 1;
                    }
		}
         }
        WaitMessage();
   }

  return 0;
}

/******************************************************************************
 *                              DragObject ( USER.464 )
 *
 */
DWORD DragObject(HWND hwndScope, HWND hWnd, WORD wObj, HANDLE hOfStruct,
                WORD szList , HCURSOR hCursor)
{
 MSG	 	msg;
 LPDRAGINFO	lpDragInfo;
 SEGPTR		spDragInfo;
 HCURSOR 	hDragCursor=0, hOldCursor=0, hBummer=0;
 HANDLE		hDragInfo  = GlobalAlloc( GMEM_SHARE | GMEM_ZEROINIT, 2*sizeof(DRAGINFO));
 WND           *wndPtr = WIN_FindWndPtr(hWnd);
 DWORD		dwRet = 0;
 short	 	dragDone = 0;
 HCURSOR	hCurrentCursor = 0;
 HWND		hCurrentWnd = 0;
 WORD	        btemp;

 lpDragInfo = (LPDRAGINFO) GlobalLock(hDragInfo);
 spDragInfo = (SEGPTR) WIN16_GlobalLock(hDragInfo);

 if( !lpDragInfo || !spDragInfo ) return 0L;

 hBummer = LoadCursor(0,IDC_BUMMER);

 if( !hBummer || !wndPtr )
   {
        GlobalFree(hDragInfo);
        return 0L;
   }

 if(hCursor)
   {
	if( !(hDragCursor = CURSORICON_IconToCursor(hCursor)) )
	  {
	   GlobalFree(hDragInfo);
	   return 0L;
	  }

	if( hDragCursor == hCursor ) hDragCursor = 0;
	else hCursor = hDragCursor;

	hOldCursor = SetCursor(hDragCursor);
   }

 lpDragInfo->hWnd   = hWnd;
 lpDragInfo->hScope = 0;
 lpDragInfo->wFlags = wObj;
 lpDragInfo->hList  = szList; /* near pointer! */
 lpDragInfo->hOfStruct = hOfStruct;
 lpDragInfo->l = 0L; 

 SetCapture(hWnd);
 ShowCursor(1);

 while( !dragDone )
  {
    WaitMessage();

    if( !PeekMessage(&msg,0,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE) )
	 continue;

   *(lpDragInfo+1) = *lpDragInfo;

    lpDragInfo->pt = msg.pt;

    /* update DRAGINFO struct */
    dprintf_msg(stddeb,"drag: lpDI->hScope = %04x\n",lpDragInfo->hScope);

    if( (btemp = (WORD)DRAG_QueryUpdate(hwndScope, spDragInfo)) > 0 )
	 hCurrentCursor = hCursor;
    else
        {
         hCurrentCursor = hBummer;
         lpDragInfo->hScope = 0;
	}
    if( hCurrentCursor )
        SetCursor(hCurrentCursor);

    dprintf_msg(stddeb,"drag: got %04x\n",btemp);

    /* send WM_DRAGLOOP */
    SendMessage( hWnd, WM_DRAGLOOP, (WPARAM)(hCurrentCursor != hBummer) , 
	                            (LPARAM) spDragInfo );
    /* send WM_DRAGSELECT or WM_DRAGMOVE */
    if( hCurrentWnd != lpDragInfo->hScope )
	{
	 if( hCurrentWnd )
	   SendMessage( hCurrentWnd, WM_DRAGSELECT, 0, 
		       (LPARAM)MAKELONG(LOWORD(spDragInfo)+sizeof(DRAGINFO),
				        HIWORD(spDragInfo)) );
	 hCurrentWnd = lpDragInfo->hScope;
	 if( hCurrentWnd )
           SendMessage( hCurrentWnd, WM_DRAGSELECT, 1, (LPARAM)spDragInfo); 
	}
    else
	if( hCurrentWnd )
	   SendMessage( hCurrentWnd, WM_DRAGMOVE, 0, (LPARAM)spDragInfo);


    /* check if we're done */
    if( msg.message == WM_LBUTTONUP || msg.message == WM_NCLBUTTONUP )
	dragDone = TRUE;
  }

 ReleaseCapture();
 ShowCursor(0);

 if( hCursor )
   {
     SetCursor(hOldCursor);
     if( hDragCursor )
	 DestroyCursor(hDragCursor);
   }

 if( hCurrentCursor != hBummer ) 
	dwRet = SendMessage( lpDragInfo->hScope, WM_DROPOBJECT, 
			     (WPARAM)hWnd, (LPARAM)spDragInfo );
 GlobalFree(hDragInfo);

 return dwRet;
}

#endif


/**********************************************************************
 *	     CallWindowProc    (USER.122)
 */
LRESULT WINAPI CallWindowProc( FARPROC func, HWND hwnd, UINT message,
		     WPARAM wParam, LPARAM lParam )
{
//    FUNCTIONALIAS *a;
    WND *wndPtr = WIN_FindWndPtr( hwnd );
	LRESULT res;

    if (!wndPtr) return 0;

#if 0

    /* check if we have something better than 16 bit relays */
    if(!ALIAS_UseAliases || !(a=ALIAS_LookupAlias((DWORD)func)) ||
        (!a->wine && !a->win32))
        return CallWndProc( (FARPROC)func, wndPtr->hInstance, 
                        hwnd, message, wParam, lParam );
    if(a->wine)
#endif
        res=((WNDPROC)func)(hwnd,message,wParam,lParam);
//TRACE("%Fp", func);
//for(;;);
	return res;
#if 0
    if(!a->win32)
        fprintf(stderr,"Where is the Win32 callback?\n");
//    if (UsesLParamPtr(message))
//	return RELAY32_CallWindowProcConvStruct(a->win32,hwnd,message,wParam,lParam);
//    return CallWndProc32( (FARPROC)a->win32, hwnd, message, wParam, lParam );
#endif
}

/***********************************************************************
 *           WIN_UpdateNCArea
 *
 */
void WIN_UpdateNCArea(WND* wnd, BOOL bUpdate)
{
    RECT rect = wnd->rectClient;
    HRGN hClip = 1;

//    TRACE("NCUpdate: hwnd %04x, hrgnUpdate %04x", 
//                      wnd->hwndSelf, wnd->hrgnUpdate );

    /* desktop windows doesn't have nonclient area */
    if(wnd == WIN_GetDesktop()) 
    {
        wnd->flags &= ~WIN_NEEDS_NCPAINT;
        return;
    }

    if( wnd->hrgnUpdate > 1 )
    {
        MapWindowPoints(wnd->parent->hwndSelf, 0, (POINT*)&rect, 2);

        hClip = CreateRectRgn( 0, 0, 0, 0 );
        if (!CombineRgn(hClip, wnd->hrgnUpdate, 0, RGN_COPY) )
        {
            DeleteObject(hClip);
            hClip = 1;
        }

        if (bUpdate)
        {
            HRGN hrgn = CreateRectRgnIndirect(&rect);
            if (hrgn && (CombineRgn(wnd->hrgnUpdate, wnd->hrgnUpdate,
                                    hrgn, RGN_AND) == NULLREGION))
            {
                DeleteObject(wnd->hrgnUpdate);
                wnd->hrgnUpdate = 1;
            }
            DeleteObject( hrgn );
        }
    }

    wnd->flags &= ~WIN_NEEDS_NCPAINT;

    if ((wnd->hwndSelf == GetActiveWindow()) &&
        !(wnd->flags & WIN_NCACTIVATED))
    {
        wnd->flags |= WIN_NCACTIVATED;
        if( hClip > 1) DeleteObject(hClip);
        hClip = 1;
    }

    if (hClip) SendMessage( wnd->hwndSelf, WM_NCPAINT, hClip, 0L );

    if (hClip > 1) DeleteObject( hClip );
}

/*******************************************************************
 *			GetSysModalWindow		[USER.189]
 */
HWND WINAPI GetSysModalWindow(void)
{
    return hwndSysModal;
}
