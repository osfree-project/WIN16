/*
 * Menu functions
 *
 * Copyright 1993 Martin Ayotte
 * Copyright 1994 Alexandre Julliard

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

/*
 * Note: the style MF_MOUSESELECT is used to mark popup items that
 * have been selected, i.e. their popup menu is currently displayed.
 * This is probably not the meaning this style has in MS-Windows.
 */

#include "user.h"
#include "menu.h"
#include "syscolor.h"


  /* Dimension of the menu bitmaps */
static WORD check_bitmap_width = 0, check_bitmap_height = 0;
static WORD arrow_bitmap_width = 0, arrow_bitmap_height = 0;

  /* Flag set by EndMenu() to force an exit from menu tracking */
static BOOL fEndMenuCalled = FALSE;

  /* Space between 2 menu bar items */
#define MENU_BAR_ITEMS_SPACE  16

  /* Minimum width of a tab character */
#define MENU_TAB_SPACE        8

  /* Height of a separator item */
#define SEPARATOR_HEIGHT      5

  /* Values for menu->FocusedItem */
  /* (other values give the position of the focused item) */
#define NO_SELECTED_ITEM  0xffff
#define SYSMENU_SELECTED  0xfffe  /* Only valid on menu-bars */

#define IS_STRING_ITEM(flags) (!((flags) & (MF_BITMAP | MF_OWNERDRAW | \
			     MF_MENUBARBREAK | MF_MENUBREAK | MF_SEPARATOR)))

#define SET_OWNERDRAW_DATA(item,data)  \
  ((item)->hText = LOWORD((DWORD)(data)), (item)->xTab = HIWORD((DWORD)(data)))

#define GET_OWNERDRAW_DATA(item)  \
  ((DWORD)MAKELONG( (WORD)(item)->hText, (item)->xTab ))


extern void NC_DrawSysButton(HWND hwnd, HDC hdc, BOOL down);  /* nonclient.c */


/***********************************************************************
 *           MENU_Init
 *
 * Menus initialisation.
 */
BOOL FAR MENU_Init()
{
	BITMAP bm;

	FUNCTION_START

	/* Load bitmaps */
	if (!(hbitmapStdCheck = LoadBitmap( 0, MAKEINTRESOURCE(OBM_CHECK) )))
		return FALSE;
	GetObject( hbitmapStdCheck, sizeof(BITMAP), (LPSTR)&bm );
	check_bitmap_width = bm.bmWidth;
	check_bitmap_height = bm.bmHeight;
	if (!(hbitmapStdMnArrow = LoadBitmap( 0, MAKEINTRESOURCE(OBM_MNARROW) )))
		return FALSE;
	GetObject( hbitmapStdMnArrow, sizeof(BITMAP), (LPSTR)&bm );
	arrow_bitmap_width = bm.bmWidth;
	arrow_bitmap_height = bm.bmHeight;

	FUNCTION_END
	return TRUE;
}


/***********************************************************************
 *           MENU_HasSysMenu
 *
 * Check whether the window owning the menu bar has a system menu.
 */
static BOOL MENU_HasSysMenu( POPUPMENU *menu )
{
    WND *wndPtr;

    if (menu->wFlags & MF_POPUP) return FALSE;
    if (!(wndPtr = WIN_FindWndPtr( menu->hWnd ))) return FALSE;
    return (wndPtr->dwStyle & WS_SYSMENU) != 0;
}


/**********************************************************************
 *           MENU_CopySysMenu
 */
static HMENU MENU_CopySysMenu(void)
{
	HMENU hMenu;
	HGLOBAL handle;
	POPUPMENU *menu;
	LPVOID template;
	HRSRC hrsrc;
	
	FUNCTION_START

	hMenu=0;
	hrsrc = FindResource(USER_HeapSel, MAKEINTRESOURCE(IDM_SYSMENU), RT_MENU);
	if (!hrsrc) 
	{
		TRACE("Resource not found");
		FUNCTION_END
		return 0;
	}

	handle = LoadResource(USER_HeapSel, hrsrc);

	TRACE("handle=%04x", handle);
	if (handle)
	{
		template = LockResource(handle);
		TRACE("template=%Fp", template);
		hMenu = LoadMenuIndirect(template);
		UnlockResource(handle);
		FreeResource(handle);
		if (!hMenu)
		{
			TRACE("No SYSMENU");
			return 0;
		}
		menu = (POPUPMENU *) LocalLock(hMenu);
		menu->wFlags |= MF_SYSMENU | MF_POPUP;
		LocalUnlock(hMenu);
	}

	FUNCTION_END
//TRACE("CopySysMenu hMenu=%04x !\n", hMenu);
	return hMenu;
}


/***********************************************************************
 *           MENU_IsInSysMenu
 *
 * Check whether the point (in screen coords) is in the system menu
 * of the window owning the given menu.
 */
static BOOL MENU_IsInSysMenu( POPUPMENU *menu, POINT pt )
{
    WND *wndPtr;

    if (menu->wFlags & MF_POPUP) return FALSE;
    if (!(wndPtr = WIN_FindWndPtr( menu->hWnd ))) return FALSE;
    if (!(wndPtr->dwStyle & WS_SYSMENU)) return FALSE;
    if ((pt.x < wndPtr->rectClient.left) ||
	(pt.x >= wndPtr->rectClient.left+GETSYSTEMMETRICS(SM_CXSIZE)+GETSYSTEMMETRICS(SM_CXBORDER)))
	return FALSE;
    if ((pt.y >= wndPtr->rectClient.top - menu->Height) ||
	(pt.y < wndPtr->rectClient.top - menu->Height -
	              GETSYSTEMMETRICS(SM_CYSIZE) - GETSYSTEMMETRICS(SM_CYBORDER))) return FALSE;
    return TRUE;
}


/***********************************************************************
 *           MENU_FindItem
 *
 * Find a menu item. Return a pointer on the item, and modifies *hmenu
 * in case the item was in a sub-menu.
 */
static MENUITEM *MENU_FindItem( HMENU FAR *hmenu, UINT FAR *nPos, UINT wFlags )
{
    POPUPMENU *menu;
    MENUITEM *item;
    int i;

    if (!(menu = (POPUPMENU *) LocalLock(*hmenu))) return NULL;
    item = (MENUITEM *) LocalLock( menu->hItems );
    if (wFlags & MF_BYPOSITION)
    {
	if (*nPos >= menu->nItems) return NULL;
	return &item[*nPos];
    }
    else
    {
	for (i = 0; i < menu->nItems; i++, item++)
	{
	    if (item->item_id == *nPos)
	    {
		*nPos = i;
		return item;
	    }
	    else if (item->item_flags & MF_POPUP)
	    {
		HMENU hsubmenu = (HMENU)item->item_id;
		MENUITEM *subitem = MENU_FindItem( &hsubmenu, nPos, wFlags );
		if (subitem)
		{
		    *hmenu = hsubmenu;
		    return subitem;
		}
	    }
	}
    }
    return NULL;
}


/***********************************************************************
 *           MENU_FindItemByCoords
 *
 * Find the item at the specified coordinates (screen coords).
 */
static MENUITEM *MENU_FindItemByCoords( POPUPMENU *menu, int x, int y, UINT FAR *pos )
{
    MENUITEM *item;
    WND *wndPtr;
    int i;

    if (!(wndPtr = WIN_FindWndPtr( menu->hWnd ))) return NULL;
    x -= wndPtr->rectWindow.left;
    y -= wndPtr->rectWindow.top;
    item = (MENUITEM *) LocalLock( menu->hItems );
    for (i = 0; i < menu->nItems; i++, item++)
    {
	if ((x >= item->rect.left) && (x < item->rect.right) &&
	    (y >= item->rect.top) && (y < item->rect.bottom))
	{
	    if (pos) *pos = i;
	    return item;
	}
    }
    return NULL;
}


/***********************************************************************
 *           MENU_FindItemByKey
 *
 * Find the menu item selected by a key press.
 * Return item id, -1 if none, -2 if we should close the menu.
 */
static UINT MENU_FindItemByKey( HWND hwndOwner, HMENU hmenu, UINT key )
{
    POPUPMENU *menu;
    MENUITEM *item;
    int i;
    LONG menuchar;

    if (!IsMenu( hmenu )) hmenu = GetSystemMenu( hwndOwner, FALSE);
    if (!hmenu) return -1;

    menu = (POPUPMENU *) LocalLock( hmenu );
    item = (MENUITEM *) LocalLock( menu->hItems );
    key = toupper(key);
    for (i = 0; i < menu->nItems; i++, item++)
    {
	if (IS_STRING_ITEM(item->item_flags))
	{
	    char FAR *p = lstrchr(LocalLock(item->hText), '&');
	    if (p && (p[1] != '&') && (toupper(p[1]) == key)) return i;
	}
    }
    menuchar = SendMessage( hwndOwner, WM_MENUCHAR, key,
			    MAKELONG( menu->wFlags, hmenu ) );
    if (HIWORD(menuchar) == 2) return LOWORD(menuchar);
    if (HIWORD(menuchar) == 1) return -2;
    return -1;
}


/***********************************************************************
 *           MENU_CalcItemSize
 *
 * Calculate the size of the menu item and store it in lpitem->rect.
 */
static void MENU_CalcItemSize( HDC hdc, LPMENUITEM lpitem, HWND hwndOwner,
			       int orgX, int orgY, BOOL menuBar )
{
    DWORD dwSize;
    char FAR *p;

    SetRect( &lpitem->rect, orgX, orgY, orgX, orgY );

    if (lpitem->item_flags & MF_OWNERDRAW)
    {
        MEASUREITEMSTRUCT mis;
        mis.CtlType    = ODT_MENU;
        mis.itemID     = lpitem->item_id;
        mis.itemData   = GET_OWNERDRAW_DATA(lpitem);
        mis.itemHeight = 16;
        mis.itemWidth  = 30;
        SendMessage( hwndOwner, WM_MEASUREITEM, 0, (LPARAM)(MEASUREITEMSTRUCT FAR *)(&mis) );
        lpitem->rect.bottom += mis.itemHeight;
        lpitem->rect.right  += mis.itemWidth;
//        dprintf_menu( stddeb, "DrawMenuItem: MeasureItem %04x %dx%d!\n",
//                      lpitem->item_id, mis.itemWidth, mis.itemHeight );
        return;
    } 

    if (lpitem->item_flags & MF_SEPARATOR)
    {
	lpitem->rect.bottom += SEPARATOR_HEIGHT;
	return;
    }

    if (!menuBar)
    {
	lpitem->rect.right += 2 * check_bitmap_width;
	if (lpitem->item_flags & MF_POPUP)
	    lpitem->rect.right += arrow_bitmap_width;
    }

    if (lpitem->item_flags & MF_BITMAP)
    {
	BITMAP bm;
        if (GetObject( (HBITMAP)lpitem->hText, sizeof(BITMAP), (LPSTR)&bm ))
        {
            lpitem->rect.right  += bm.bmWidth;
            lpitem->rect.bottom += bm.bmHeight;
        }
	return;
    }
    
    /* If we get here, then it must be a text item */

    if (IS_STRING_ITEM( lpitem->item_flags ))
    {
        const char *text = (const char *)LocalLock( lpitem->hText );
        dwSize = GetTextExtent( hdc, text, lstrlen(text) );
        lpitem->rect.right  += LOWORD(dwSize);
        lpitem->rect.bottom += max( HIWORD(dwSize), GETSYSTEMMETRICS(SM_CYMENU) );
        lpitem->xTab = 0;

        if (menuBar) lpitem->rect.right += MENU_BAR_ITEMS_SPACE;
        else if ((p = lstrchr( text, '\t' )) != NULL)
        {
            /* Item contains a tab (only meaningful in popup menus) */
            lpitem->xTab = check_bitmap_width + MENU_TAB_SPACE + 
                LOWORD( GetTextExtent( hdc, text, (int)(p - text) ));
            lpitem->rect.right += MENU_TAB_SPACE;
        }
        else
        {
            if (lstrchr( text, '\b' )) lpitem->rect.right += MENU_TAB_SPACE;
            lpitem->xTab = lpitem->rect.right - check_bitmap_width 
                           - arrow_bitmap_width;
        }
    }
}


/***********************************************************************
 *           MENU_PopupMenuCalcSize
 *
 * Calculate the size of a popup menu.
 */
static void MENU_PopupMenuCalcSize( LPPOPUPMENU lppop, HWND hwndOwner )
{
    MENUITEM  *items;
    MENUITEM  *lpitem;
    HDC hdc;
    int start, i;
    int orgX, orgY, maxX, maxTab, maxTabWidth;

    lppop->Width = lppop->Height = 0;
    if (lppop->nItems == 0) return;
    items = (MENUITEM *)LocalLock( lppop->hItems );
    hdc = GetDC( 0 );
    maxX = start = 0;
    while (start < lppop->nItems)
    {
	lpitem = &items[start];
	orgX = maxX;
	orgY = 0;
	maxTab = maxTabWidth = 0;

	  /* Parse items until column break or end of menu */
	for (i = start; i < lppop->nItems; i++, lpitem++)
	{
	    if ((i != start) &&
		(lpitem->item_flags & (MF_MENUBREAK | MF_MENUBARBREAK))) break;
	    MENU_CalcItemSize( hdc, lpitem, hwndOwner, orgX, orgY, FALSE );
            if (lpitem->item_flags & MF_MENUBARBREAK) orgX++;
	    maxX = max( maxX, lpitem->rect.right );
	    orgY = lpitem->rect.bottom;
	    if (IS_STRING_ITEM(lpitem->item_flags) && lpitem->xTab)
	    {
		maxTab = max( maxTab, lpitem->xTab );
		maxTabWidth = max(maxTabWidth,lpitem->rect.right-lpitem->xTab);
	    }
	}

	  /* Finish the column (set all items to the largest width found) */
	maxX = max( maxX, maxTab + maxTabWidth );
	for (lpitem = &items[start]; start < i; start++, lpitem++)
	{
	    lpitem->rect.right = maxX;
	    if (IS_STRING_ITEM(lpitem->item_flags) && lpitem->xTab)
                lpitem->xTab = maxTab;
	}
	lppop->Height = max( lppop->Height, orgY );
    }

    lppop->Width  = maxX;
    ReleaseDC( 0, hdc );
}


/***********************************************************************
 *           MENU_MenuBarCalcSize
 *
 * Calculate the size of the menu bar.
 */
static void MENU_MenuBarCalcSize( HDC hdc, LPRECT lprect, POPUPMENU * lppop,
				  HWND hwndOwner )
{
    MENUITEM * lpitem;
    MENUITEM * items;
    int start, i, orgX, orgY, maxY, helpPos;

    if ((lprect == NULL) || (lppop == NULL)) return;
    if (lppop->nItems == 0) return;
//    dprintf_menu(stddeb,"MENU_MenuBarCalcSize left=%d top=%d right=%d bottom=%d\n", 
//                 lprect->left, lprect->top, lprect->right, lprect->bottom);
    items = (MENUITEM *)LocalLock( lppop->hItems );
    lppop->Width  = lprect->right - lprect->left;
    lppop->Height = 0;
    maxY = lprect->top;
    start = 0;
    helpPos = -1;
    while (start < lppop->nItems)
    {
	lpitem = &items[start];
	orgX = lprect->left;
	orgY = maxY;

	  /* Parse items until line break or end of menu */
	for (i = start; i < lppop->nItems; i++, lpitem++)
	{
	    if ((helpPos == -1) && (lpitem->item_flags & MF_HELP)) helpPos = i;
	    if ((i != start) &&
		(lpitem->item_flags & (MF_MENUBREAK | MF_MENUBARBREAK))) break;
	    MENU_CalcItemSize( hdc, lpitem, hwndOwner, orgX, orgY, TRUE );
	    if (lpitem->rect.right > lprect->right)
	    {
		if (i != start) break;
		else lpitem->rect.right = lprect->right;
	    }
	    maxY = max( maxY, lpitem->rect.bottom );
	    orgX = lpitem->rect.right;
	}

	  /* Finish the line (set all items to the largest height found) */
	while (start < i) items[start++].rect.bottom = maxY;
    }

    lprect->bottom = maxY;
    lppop->Height = lprect->bottom - lprect->top;

      /* Flush right all items between the MF_HELP and the last item */
      /* (if several lines, only move the last line) */
    if (helpPos != -1)
    {
	lpitem = &items[lppop->nItems-1];
	orgY = lpitem->rect.top;
	orgX = lprect->right;
	for (i = lppop->nItems - 1; i >= helpPos; i--, lpitem--)
	{
	    if (lpitem->rect.top != orgY) break;    /* Other line */
	    if (lpitem->rect.right >= orgX) break;  /* Too far right already */
	    lpitem->rect.left += orgX - lpitem->rect.right;
	    lpitem->rect.right = orgX;
	    orgX = lpitem->rect.left;
	}
    }
}


/***********************************************************************
 *           MENU_DrawMenuItem
 *
 * Draw a single menu item.
 */
static void MENU_DrawMenuItem( HWND hwnd, HDC hdc, LPMENUITEM lpitem,
			       UINT height, BOOL menuBar )
{
	RECT rect;
	HDC hdcMem;
	HBITMAP hOldBitmap;

	FUNCTION_START

    if (lpitem->item_flags & MF_OWNERDRAW)
    {
        DRAWITEMSTRUCT dis;

	TRACE("DrawMenuItem: Ownerdraw!");
        dis.CtlType   = ODT_MENU;
        dis.itemID    = lpitem->item_id;
        dis.itemData  = GET_OWNERDRAW_DATA(lpitem);
        dis.itemState = 0;
        if (lpitem->item_flags & MF_CHECKED) dis.itemState |= ODS_CHECKED;
        if (lpitem->item_flags & MF_GRAYED)  dis.itemState |= ODS_GRAYED;
        if (lpitem->item_flags & MF_HILITE)  dis.itemState |= ODS_SELECTED;
        dis.itemAction = ODA_DRAWENTIRE | ODA_SELECT | ODA_FOCUS;
        dis.hwndItem   = hwnd;
        dis.hDC        = hdc;
        dis.rcItem     = lpitem->rect;
        SendMessage( hwnd, WM_DRAWITEM, 0, (LPARAM)(DRAWITEMSTRUCT FAR *)(&dis) );
        return;
    }

    if (menuBar && (lpitem->item_flags & MF_SEPARATOR)) return;
    rect = lpitem->rect;

      /* Draw the background */

    if (lpitem->item_flags & MF_HILITE)
	FillRect( hdc, &rect, GETSYSCOLORBRUSH(hbrushHighlight));
    else FillRect( hdc, &rect, GETSYSCOLORBRUSH(hbrushMenu));
    SetBkMode( hdc, TRANSPARENT );

      /* Draw the separator bar (if any) */

    if (!menuBar && (lpitem->item_flags & MF_MENUBARBREAK))
    {
	SelectObject( hdc, GETSYSCOLORBRUSH(hpenWindowFrame));
	MoveTo( hdc, rect.left, 0 );
	LineTo( hdc, rect.left, height );
    }
    if (lpitem->item_flags & MF_SEPARATOR)
    {
	SelectObject( hdc, GETSYSCOLORBRUSH(hpenWindowFrame));
	MoveTo( hdc, rect.left, rect.top + SEPARATOR_HEIGHT/2 );
	LineTo( hdc, rect.right, rect.top + SEPARATOR_HEIGHT/2 );
	return;
    }

      /* Setup colors */

    if (lpitem->item_flags & MF_HILITE)
    {
	if (lpitem->item_flags & MF_GRAYED)
	    SetTextColor( hdc, GETSYSCOLOR( COLOR_GRAYTEXT ) );
	else
	    SetTextColor( hdc, GETSYSCOLOR( COLOR_HIGHLIGHTTEXT ) );
	SetBkColor( hdc, GETSYSCOLOR( COLOR_HIGHLIGHT ) );
    }
    else
    {
	if (lpitem->item_flags & MF_GRAYED)
	    SetTextColor( hdc, GETSYSCOLOR( COLOR_GRAYTEXT ) );
	else
	    SetTextColor( hdc, GETSYSCOLOR( COLOR_MENUTEXT ) );
	SetBkColor( hdc, GETSYSCOLOR( COLOR_MENU ) );
    }

    if (!menuBar)
    {
	  /* Draw the check mark */

	if (lpitem->item_flags & MF_CHECKED)
	{
            HBITMAP hCheckBmp = lpitem->hCheckBit ? lpitem->hCheckBit : hbitmapStdCheck;
            hdcMem = CreateCompatibleDC(hdc);
            hOldBitmap = SelectObject(hdcMem, hCheckBmp);
            BitBlt(hdc, 
                   rect.left,
                   (rect.top + rect.bottom - check_bitmap_height) / 2,
                   check_bitmap_width,
                   check_bitmap_height,
                   hdcMem,
                   0,
                   0,
                   SRCCOPY);
            SelectObject(hdcMem, hOldBitmap);
            DeleteDC(hdcMem);
//	    GRAPH_DrawBitmap(hdc, lpitem->hCheckBit ? lpitem->hCheckBit :
//			     hbitmapStdCheck, rect.left,
//			     (rect.top+rect.bottom-check_bitmap_height) / 2,
//			     0, 0, check_bitmap_width, check_bitmap_height );
	}
	else if (lpitem->hUnCheckBit != 0)  /* Not checked */
	{
            hdcMem = CreateCompatibleDC(hdc);
            hOldBitmap = SelectObject(hdcMem, lpitem->hUnCheckBit);
            BitBlt(hdc,
                   rect.left,
                   (rect.top + rect.bottom - check_bitmap_height) / 2,
                   check_bitmap_width,
                   check_bitmap_height,
                   hdcMem,
                   0,
                   0,
                   SRCCOPY);
            SelectObject(hdcMem, hOldBitmap);
            DeleteDC(hdcMem);
//	    GRAPH_DrawBitmap(hdc, lpitem->hUnCheckBit, rect.left,
//			     (rect.top+rect.bottom-check_bitmap_height) / 2,
//			     0, 0, check_bitmap_width, check_bitmap_height );
	}

	  /* Draw the popup-menu arrow */

	if (lpitem->item_flags & MF_POPUP)
	{
            hdcMem = CreateCompatibleDC(hdc);
            hOldBitmap = SelectObject(hdcMem, hbitmapStdMnArrow);
            BitBlt(hdc,
                   rect.right - arrow_bitmap_width - 1,
                   (rect.top + rect.bottom - arrow_bitmap_height) / 2,
                   arrow_bitmap_width,
                   arrow_bitmap_height,
                   hdcMem,
                   0,
                   0,
                   SRCCOPY);
            SelectObject(hdcMem, hOldBitmap);
            DeleteDC(hdcMem);
//	    GRAPH_DrawBitmap( hdc, hbitmapStdMnArrow,
//			      rect.right-arrow_bitmap_width-1,
//			      (rect.top+rect.bottom-arrow_bitmap_height) / 2,
//                              0, 0, arrow_bitmap_width, arrow_bitmap_height );
	}

	rect.left += check_bitmap_width;
	rect.right -= arrow_bitmap_width;
    }

      /* Draw the item text or bitmap */

    if (lpitem->item_flags & MF_BITMAP)
    {
        hdcMem = CreateCompatibleDC(hdc);
        hOldBitmap = SelectObject(hdcMem, (HBITMAP)lpitem->hText);
        BitBlt(hdc,
               rect.left,
               rect.top,
               rect.right - rect.left,
               rect.bottom - rect.top,
               hdcMem,
               0,
               0,
               SRCCOPY);
        SelectObject(hdcMem, hOldBitmap);
        DeleteDC(hdcMem);
//	GRAPH_DrawBitmap( hdc, (HBITMAP)lpitem->hText, rect.left, rect.top,
//                          0, 0, rect.right-rect.left, rect.bottom-rect.top );
	return;
    }
    /* No bitmap - process text if present */
    else if (IS_STRING_ITEM(lpitem->item_flags))
    {
	register int i;
        const char *text = (const char *)LocalLock( lpitem->hText );

	if (menuBar)
	{
	    rect.left += MENU_BAR_ITEMS_SPACE / 2;
	    rect.right -= MENU_BAR_ITEMS_SPACE / 2;
	    i = lstrlen( text );
	}
	else
	{
	    for (i = 0; text[i]; i++)
                if ((text[i] == '\t') || (text[i] == '\b')) break;
	}
	
	DrawText( hdc, text, i, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE );

	if (text[i])  /* There's a tab or flush-right char */
	{
	    if (text[i] == '\t')
	    {
		rect.left = lpitem->xTab;
		DrawText( hdc, text + i + 1, -1, &rect,
			  DT_LEFT | DT_VCENTER | DT_SINGLELINE );
	    }
	    else DrawText( hdc, text + i + 1, -1, &rect,
			   DT_RIGHT | DT_VCENTER | DT_SINGLELINE );
	}
    }
	FUNCTION_END
}


/***********************************************************************
 *           MENU_DrawPopupMenu
 *
 * Paint a popup menu.
 */
static VOID MENU_DrawPopupMenu(HWND hWnd, HDC hdc, HMENU hMenu)
{
	POPUPMENU *menu;
	MENUITEM *item;
	RECT rect;
	int i;

	GetClientRect(hWnd, &rect);
	FillRect(hdc, &rect, GETSYSCOLORBRUSH(hbrushMenu));
	menu = (POPUPMENU *)LocalLock( hMenu );
	if (menu)
	{
		if (menu->nItems)
		{
			item = (MENUITEM *)LocalLock( menu->hItems );
			for (i = menu->nItems; i > 0; i--, item++)
				MENU_DrawMenuItem(hWnd, hdc, item, menu->Height, FALSE);
			LocalUnlock(menu->hItems);
		}
		LocalUnlock(hMenu);
	}
}


/***********************************************************************
 *           MENU_DrawMenuBar
 *
 * Paint a menu bar. Returns the height of the menu bar.
 */
UINT MENU_DrawMenuBar(HDC hDC, LPRECT lprect, HWND hwnd, BOOL suppress_draw)
{
	POPUPMENU * lppop;
	MENUITEM * lpitem;
	int i;
	WND *wndPtr;

	PushDS();
	SetDS(USER_HeapSel);
	FUNCTION_START    

	TRACE("MENU_DrawMenuBar(%04x, %p, %p)", hDC, lprect, lppop);

	wndPtr = WIN_FindWndPtr( hwnd );

	lppop = (POPUPMENU *) LocalLock((HMENU)wndPtr->wIDmenu );
	if (lppop == NULL || lprect == NULL) return GETSYSTEMMETRICS(SM_CYMENU);
	if (lppop->Height == 0) MENU_MenuBarCalcSize(hDC, lprect, lppop, hwnd);
	lprect->bottom = lprect->top + lppop->Height;
	if (suppress_draw) return lppop->Height;

	TRACE("Fill start");    
	FillRect(hDC, lprect, GETSYSCOLORBRUSH(hbrushMenu));
	SelectObject( hDC, GETSYSCOLORBRUSH(hpenWindowFrame));
	MoveTo( hDC, lprect->left, lprect->bottom );
	LineTo( hDC, lprect->right, lprect->bottom );
	TRACE("Fill end");    

	if (lppop->nItems == 0) return GETSYSTEMMETRICS(SM_CYMENU);
	lpitem = (MENUITEM *) LocalLock(lppop->hItems );
	for (i = 0; i < lppop->nItems; i++, lpitem++)
	{
		MENU_DrawMenuItem( hwnd, hDC, lpitem, lppop->Height, TRUE );
	}

	FUNCTION_END
	PopDS();

	return lppop->Height;
} 


/***********************************************************************
 *           MENU_ShowPopup
 *
 * Display a popup menu.
 */
static BOOL MENU_ShowPopup(HWND hwndOwner, HMENU hmenu, UINT id, int x, int y)
{
    POPUPMENU *menu;

    if (!(menu = (POPUPMENU *) LocalLock( hmenu ))) return FALSE;
    if (menu->FocusedItem != NO_SELECTED_ITEM)
    {
	MENUITEM *item = (MENUITEM *) LocalLock( menu->hItems );
	item[menu->FocusedItem].item_flags &= ~(MF_HILITE | MF_MOUSESELECT);
	menu->FocusedItem = NO_SELECTED_ITEM;
    }
    SendMessage( hwndOwner, WM_INITMENUPOPUP, (WPARAM)hmenu,
		 MAKELONG( id, (menu->wFlags & MF_SYSMENU) ? 1 : 0 ));
    MENU_PopupMenuCalcSize( menu, hwndOwner );
    if (!menu->hWnd)
    {
	WND *wndPtr = WIN_FindWndPtr( hwndOwner );
	if (!wndPtr) return FALSE;
	menu->hWnd = CreateWindow( POPUPMENU_CLASS_ATOM, 0,
				   WS_POPUP | WS_BORDER, x, y, 
				   menu->Width + 2*GETSYSTEMMETRICS(SM_CXBORDER),
				   menu->Height + 2*GETSYSTEMMETRICS(SM_CYBORDER),
				   0, 0, wndPtr->hInstance, (LPVOID)hmenu );
	if (!menu->hWnd) return FALSE;
    }
    else SetWindowPos( menu->hWnd, 0, x, y,
		       menu->Width + 2*GETSYSTEMMETRICS(SM_CXBORDER),
		       menu->Height + 2*GETSYSTEMMETRICS(SM_CYBORDER),
		       SWP_NOACTIVATE | SWP_NOZORDER );

      /* Display the window */

    SetWindowPos( menu->hWnd, HWND_TOP, 0, 0, 0, 0,
		  SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );
    UpdateWindow( menu->hWnd );
    return TRUE;
}


/***********************************************************************
 *           MENU_SelectItem
 */
static void MENU_SelectItem( HWND hwndOwner, HMENU hmenu, UINT wIndex )
{
    MENUITEM *items;
    POPUPMENU * ppop;
    HDC hdc;

    ppop = (POPUPMENU *) LocalLock( hmenu );
    if (!ppop->nItems) return;
    items = (MENUITEM *) LocalLock( ppop->hItems );
    if ((wIndex != NO_SELECTED_ITEM) && 
	(wIndex != SYSMENU_SELECTED) &&
	(items[wIndex].item_flags & MF_SEPARATOR))
	wIndex = NO_SELECTED_ITEM;
    if (ppop->FocusedItem == wIndex) return;
    if (ppop->wFlags & MF_POPUP) hdc = GetDC( ppop->hWnd );
    else hdc = GetDCEx( ppop->hWnd, 0, DCX_CACHE | DCX_WINDOW);

      /* Clear previous highlighted item */
    if (ppop->FocusedItem != NO_SELECTED_ITEM) 
    {
	if (ppop->FocusedItem == SYSMENU_SELECTED)
	    NC_DrawSysButton( ppop->hWnd, hdc, FALSE );
	else
	{
	    items[ppop->FocusedItem].item_flags &=~(MF_HILITE|MF_MOUSESELECT);
	    MENU_DrawMenuItem( ppop->hWnd, hdc, &items[ppop->FocusedItem], ppop->Height,
			       !(ppop->wFlags & MF_POPUP) );
	}
    }

      /* Highlight new item (if any) */
    ppop->FocusedItem = wIndex;
    if (ppop->FocusedItem != NO_SELECTED_ITEM) 
    {
	if (ppop->FocusedItem == SYSMENU_SELECTED)
        {
	    NC_DrawSysButton( ppop->hWnd, hdc, TRUE );
            SendMessage( hwndOwner, WM_MENUSELECT,
                         GetSystemMenu( ppop->hWnd, FALSE ),
                         MAKELONG( ppop->wFlags | MF_MOUSESELECT, hmenu ) );
        }
	else
	{
	    items[ppop->FocusedItem].item_flags |= MF_HILITE;
	    MENU_DrawMenuItem( ppop->hWnd, hdc, &items[ppop->FocusedItem], ppop->Height,
			       !(ppop->wFlags & MF_POPUP) );
	    SendMessage( hwndOwner, WM_MENUSELECT,
                         items[ppop->FocusedItem].item_id,
		         MAKELONG( items[ppop->FocusedItem].item_flags | MF_MOUSESELECT, hmenu));
	}
    }
    else SendMessage( hwndOwner, WM_MENUSELECT, hmenu,
                      MAKELONG( ppop->wFlags | MF_MOUSESELECT, hmenu ) );

    ReleaseDC( ppop->hWnd, hdc );
}


/***********************************************************************
 *           MENU_SelectNextItem
 */
static void MENU_SelectNextItem( HWND hwndOwner, HMENU hmenu )
{
    int i;
    MENUITEM *items;
    POPUPMENU *menu;

    menu = (POPUPMENU *) LocalLock( hmenu );
    if (!menu->nItems) return;
    items = (MENUITEM *) LocalLock( menu->hItems );
    if ((menu->FocusedItem != NO_SELECTED_ITEM) &&
	(menu->FocusedItem != SYSMENU_SELECTED))
    {
	for (i = menu->FocusedItem+1; i < menu->nItems; i++)
	{
	    if (!(items[i].item_flags & MF_SEPARATOR))
	    {
		MENU_SelectItem( hwndOwner, hmenu, i );
		return;
	    }
	}
	if (MENU_HasSysMenu( menu ))
	{
	    MENU_SelectItem( hwndOwner, hmenu, SYSMENU_SELECTED );
	    return;
	}
    }
    for (i = 0; i < menu->nItems; i++)
    {
	if (!(items[i].item_flags & MF_SEPARATOR))
	{
	    MENU_SelectItem( hwndOwner, hmenu, i );
	    return;
	}
    }
    if (MENU_HasSysMenu( menu ))
        MENU_SelectItem( hwndOwner, hmenu, SYSMENU_SELECTED );
}


/***********************************************************************
 *           MENU_SelectPrevItem
 */
static void MENU_SelectPrevItem( HWND hwndOwner, HMENU hmenu )
{
    int i;
    MENUITEM *items;
    POPUPMENU *menu;

    menu = (POPUPMENU *) LocalLock( hmenu );
    if (!menu->nItems) return;
    items = (MENUITEM *) LocalLock( menu->hItems );
    if ((menu->FocusedItem != NO_SELECTED_ITEM) &&
	(menu->FocusedItem != SYSMENU_SELECTED))
    {
	for (i = menu->FocusedItem - 1; i >= 0; i--)
	{
	    if (!(items[i].item_flags & MF_SEPARATOR))
	    {
		MENU_SelectItem( hwndOwner, hmenu, i );
		return;
	    }
	}
	if (MENU_HasSysMenu( menu ))
	{
	    MENU_SelectItem( hwndOwner, hmenu, SYSMENU_SELECTED );
	    return;
	}
    }
    for (i = menu->nItems - 1; i > 0; i--)
    {
	if (!(items[i].item_flags & MF_SEPARATOR))
	{
	    MENU_SelectItem( hwndOwner, hmenu, i );
	    return;
	}
    }
    if (MENU_HasSysMenu( menu ))
        MENU_SelectItem( hwndOwner, hmenu, SYSMENU_SELECTED );
}


/**********************************************************************
 *         MENU_SetItemData
 *
 * Set an item flags, id and text ptr.
 */
static BOOL MENU_SetItemData( MENUITEM *item, UINT flags, UINT id, LPCSTR data)
{
    HANDLE hPrevText = IS_STRING_ITEM(item->item_flags) ? item->hText : 0;

    if (IS_STRING_ITEM(flags))
    {
        if (!data)
        {
            flags |= MF_SEPARATOR;
            item->hText = 0;
        }
        else
        {
            LPCSTR str = data;
            HANDLE hText;
            
            /* Item beginning with a backspace is a help item */
            if (*str == '\b')
            {
                flags |= MF_HELP;
                str++;
            }
            if (!(hText = LocalAlloc(LMEM_FIXED, lstrlen(str)+1 ))) return FALSE;
            item->hText = hText;
            lstrcpy( (char *)LocalLock( hText ), str );
        }
    }
    else if (flags & MF_BITMAP) item->hText = (HANDLE)data;
    else if (flags & MF_OWNERDRAW) SET_OWNERDRAW_DATA( item, data );
    else item->hText = 0;

    item->item_flags = flags & ~(MF_HILITE | MF_MOUSESELECT);
    item->item_id    = id;
    SetRectEmpty( &item->rect );
    if (hPrevText) LocalFree( hPrevText );
    return TRUE;
}


/**********************************************************************
 *         MENU_InsertItem
 *
 * Insert a new item into a menu.
 */
static MENUITEM *MENU_InsertItem( HMENU hMenu, UINT pos, UINT flags )
{
    HANDLE hNewItems;
    MENUITEM *newItems;
    POPUPMENU *menu;

	FUNCTION_START

    if (!(menu = (POPUPMENU *)LocalLock(hMenu))) 
    {
//        dprintf_menu( stddeb, "MENU_InsertItem: %04x not a menu handle\n",
//                      hMenu );
        return NULL;
    }

    /* Find where to insert new item */

    if ((flags & MF_BYPOSITION) &&
        ((pos == (UINT)-1) || (pos == menu->nItems)))
    {
        /* Special case: append to menu */
        /* Some programs specify the menu length to do that */
        pos = menu->nItems;
    }
    else
    {
        if (!MENU_FindItem( &hMenu, &pos, flags )) 
        {
//            dprintf_menu( stddeb, "MENU_InsertItem: item %x not found\n",
//                          pos );
            return NULL;
        }
        if (!(menu = (POPUPMENU *) LocalLock(hMenu)))
        {
//            dprintf_menu(stddeb,"MENU_InsertItem: %04x not a menu handle\n",
//                         hMenu);
            return NULL;
        }
    }

    /* Create new items array */

    hNewItems = LocalAlloc(LMEM_FIXED, sizeof(MENUITEM) * (menu->nItems+1) );
    if (!hNewItems)
    {
//        dprintf_menu( stddeb, "MENU_InsertMenu: allocation failed\n" );
        return NULL;
    }
    newItems = (MENUITEM *) LocalLock( hNewItems );
    if (menu->nItems > 0)
    {
	  /* Copy the old array into the new */
	MENUITEM *oldItems = (MENUITEM *) LocalLock( menu->hItems );
	if (pos > 0) _fmemcpy( newItems, oldItems, pos * sizeof(MENUITEM) );
	if (pos < menu->nItems) _fmemcpy( &newItems[pos+1], &oldItems[pos],
					(menu->nItems-pos)*sizeof(MENUITEM) );

	LocalFree( menu->hItems );
    }
    menu->hItems = hNewItems;
    menu->nItems++;
    _fmemset( &newItems[pos], 0, sizeof(*newItems) );
	FUNCTION_END
    return &newItems[pos];
}


/**********************************************************************
 *         MENU_ParseResource
 *
 * Parse a menu resource and add items to the menu.
 * Return a pointer to the end of the resource.
 */
static LPCSTR MENU_ParseResource( LPCSTR res, HMENU hMenu )
{
    WORD flags, id = 0;
    LPCSTR data;
	FUNCTION_START
    do
    {
        flags = *(WORD FAR *)res;
        res += sizeof(WORD);
        if (!(flags & MF_POPUP))
        {
            id = *(WORD FAR *)res;
            res += sizeof(WORD);
        }
        data = res;
        res += lstrlen( data ) + 1;
        if (!IS_STRING_ITEM(flags)) 
            TRACE("MENU_ParseResource: not a string item %04x\n", flags );

        if (flags & MF_POPUP)
        {
            HMENU hSubMenu = CreatePopupMenu();
            if (!hSubMenu) return NULL;
            if (!(res = MENU_ParseResource( res, hSubMenu ))) return NULL;
            AppendMenu( hMenu, flags, (UINT)hSubMenu, data );
        }
        else
        {
            if (!*data) data = 0;
            AppendMenu( hMenu, flags, id, data );
        }
    } while (!(flags & MF_END));

	FUNCTION_END
	return res;
}


/***********************************************************************
 *           MENU_GetSubPopup
 *
 * Return the handle of the selected sub-popup menu (if any).
 */
static HMENU MENU_GetSubPopup( HMENU hmenu )
{
    POPUPMENU *menu;
    MENUITEM *item;

	FUNCTION_START
    menu = (POPUPMENU *) LocalLock( hmenu );
    if (menu->FocusedItem == NO_SELECTED_ITEM) return 0;
    else if (menu->FocusedItem == SYSMENU_SELECTED)
	return GetSystemMenu( menu->hWnd, FALSE );

    item = ((MENUITEM *)LocalLock(menu->hItems)) + menu->FocusedItem;
    if (!(item->item_flags & MF_POPUP) || !(item->item_flags & MF_MOUSESELECT))
	return 0;
    return (HMENU)item->item_id;
}


/***********************************************************************
 *           MENU_HideSubPopups
 *
 * Hide the sub-popup menus of this menu.
 */
static void MENU_HideSubPopups( HWND hwndOwner, HMENU hmenu )
{
    MENUITEM *item;
    POPUPMENU *menu, *submenu;
    HMENU hsubmenu;

    if (!(menu = (POPUPMENU *) LocalLock( hmenu ))) return;
    if (menu->FocusedItem == NO_SELECTED_ITEM) return;
    if (menu->FocusedItem == SYSMENU_SELECTED)
    {
	hsubmenu = GetSystemMenu( menu->hWnd, FALSE );
    }
    else
    {
	item = ((MENUITEM *)LocalLock(menu->hItems)) + menu->FocusedItem;
	if (!(item->item_flags & MF_POPUP) ||
	    !(item->item_flags & MF_MOUSESELECT)) return;
	item->item_flags &= ~MF_MOUSESELECT;
	hsubmenu = (HMENU)item->item_id;
    }
    submenu = (POPUPMENU *) LocalLock( hsubmenu );
    MENU_HideSubPopups( hwndOwner, hsubmenu );
    if (submenu->hWnd) ShowWindow( submenu->hWnd, SW_HIDE );
    MENU_SelectItem( hwndOwner, hsubmenu, NO_SELECTED_ITEM );
}


/***********************************************************************
 *           MENU_ShowSubPopup
 *
 * Display the sub-menu of the selected item of this menu.
 * Return the handle of the submenu, or hmenu if no submenu to display.
 */
static HMENU MENU_ShowSubPopup( HWND hwndOwner, HMENU hmenu, BOOL selectFirst )
{
    POPUPMENU *menu;
    MENUITEM *item;
    WND *wndPtr;

    if (!(menu = (POPUPMENU *) LocalLock( hmenu ))) return hmenu;
    if (!(wndPtr = WIN_FindWndPtr( menu->hWnd ))) return hmenu;
    if (menu->FocusedItem == NO_SELECTED_ITEM) return hmenu;
    if (menu->FocusedItem == SYSMENU_SELECTED)
    {
	MENU_ShowPopup(hwndOwner, wndPtr->hSysMenu, 0, wndPtr->rectClient.left,
		wndPtr->rectClient.top - menu->Height - 2*GETSYSTEMMETRICS(SM_CYBORDER));
	if (selectFirst) MENU_SelectNextItem( hwndOwner, wndPtr->hSysMenu );
	return wndPtr->hSysMenu;
    }
    item = ((MENUITEM *)LocalLock(menu->hItems)) + menu->FocusedItem;
    if (!(item->item_flags & MF_POPUP) ||
	(item->item_flags & (MF_GRAYED | MF_DISABLED))) return hmenu;
    item->item_flags |= MF_MOUSESELECT;
    if (menu->wFlags & MF_POPUP)
    {
	MENU_ShowPopup( hwndOwner, (HMENU)item->item_id, menu->FocusedItem,
		 wndPtr->rectWindow.left + item->rect.right-arrow_bitmap_width,
		 wndPtr->rectWindow.top + item->rect.top );
    }
    else
    {
	MENU_ShowPopup( hwndOwner, (HMENU)item->item_id, menu->FocusedItem,
		        wndPtr->rectWindow.left + item->rect.left,
		        wndPtr->rectWindow.top + item->rect.bottom );
    }
    if (selectFirst) MENU_SelectNextItem( hwndOwner, (HMENU)item->item_id );
    return (HMENU)item->item_id;
}


/***********************************************************************
 *           MENU_FindMenuByCoords
 *
 * Find the menu containing a given point (in screen coords).
 */
static HMENU MENU_FindMenuByCoords( HMENU hmenu, POINT pt )
{
    POPUPMENU *menu;
    HWND hwnd;

	FUNCTION_START
    if (!(hwnd = WindowFromPoint( pt ))) return 0;
    while (hmenu)
    {
	menu = (POPUPMENU *) LocalLock( hmenu );
	if (menu->hWnd == hwnd)
	{
	    if (!(menu->wFlags & MF_POPUP))
	    {
		  /* Make sure it's in the menu bar (or in system menu) */
		WND *wndPtr = WIN_FindWndPtr( menu->hWnd );
		if ((pt.x < wndPtr->rectClient.left) ||
		    (pt.x >= wndPtr->rectClient.right) ||
		    (pt.y >= wndPtr->rectClient.top)) return 0;
		if (pt.y < wndPtr->rectClient.top - menu->Height)
		{
		    if (!MENU_IsInSysMenu( menu, pt )) return 0;
		}
		/* else it's in the menu bar */
	    }
		FUNCTION_END
	    return hmenu;
	}
	hmenu = MENU_GetSubPopup( hmenu );
    }
		FUNCTION_END
    return 0;
}


/***********************************************************************
 *           MENU_ExecFocusedItem
 *
 * Execute a menu item (for instance when user pressed Enter).
 * Return TRUE if we can go on with menu tracking.
 */
static BOOL MENU_ExecFocusedItem( HWND hwndOwner, HMENU hmenu,
				  HMENU FAR *hmenuCurrent )
{
    MENUITEM *item;
    POPUPMENU *menu = (POPUPMENU *) LocalLock( hmenu );
    if (!menu || !menu->nItems || (menu->FocusedItem == NO_SELECTED_ITEM) ||
	(menu->FocusedItem == SYSMENU_SELECTED)) return TRUE;
    item = ((MENUITEM *)LocalLock(menu->hItems)) + menu->FocusedItem;
    if (!(item->item_flags & MF_POPUP))
    {
	if (!(item->item_flags & (MF_GRAYED | MF_DISABLED)))
	{
	    PostMessage( hwndOwner, (menu->wFlags & MF_SYSMENU) ? 
			WM_SYSCOMMAND : WM_COMMAND, item->item_id, 0 );
	    return FALSE;
	}
	else return TRUE;
    }
    else
    {
	*hmenuCurrent = MENU_ShowSubPopup( hwndOwner, hmenu, TRUE );
	return TRUE;
    }
}


/***********************************************************************
 *           MENU_ButtonDown
 *
 * Handle a button-down event in a menu. Point is in screen coords.
 * hmenuCurrent is the top-most visible popup.
 * Return TRUE if we can go on with menu tracking.
 */
static BOOL MENU_ButtonDown( HWND hwndOwner, HMENU hmenu, HMENU FAR *hmenuCurrent,
			     POINT pt )
{
    POPUPMENU *menu;
    MENUITEM *item;
    UINT id;

	FUNCTION_START
    if (!hmenu) return FALSE;  /* Outside all menus */
    menu = (POPUPMENU *) LocalLock( hmenu );
    item = MENU_FindItemByCoords( menu, pt.x, pt.y, &id );
    if (!item)  /* Maybe in system menu */
    {
	if (!MENU_IsInSysMenu( menu, pt )) return FALSE;
	id = SYSMENU_SELECTED;
    }	

    if (menu->FocusedItem == id)
    {
	if (id == SYSMENU_SELECTED) return FALSE;
	if (item->item_flags & MF_POPUP)
	{
	    if (item->item_flags & MF_MOUSESELECT)
	    {
		if (menu->wFlags & MF_POPUP)
		{
		    MENU_HideSubPopups( hwndOwner, hmenu );
		    *hmenuCurrent = hmenu;
		}
		else 
		{
			FUNCTION_END
			return FALSE;
		}
	    }
	    else *hmenuCurrent = MENU_ShowSubPopup( hwndOwner, hmenu, FALSE );
	}
    }
    else
    {
	MENU_HideSubPopups( hwndOwner, hmenu );
	MENU_SelectItem( hwndOwner, hmenu, id );
	*hmenuCurrent = MENU_ShowSubPopup( hwndOwner, hmenu, FALSE );
    }
	FUNCTION_END
    return TRUE;
}


/***********************************************************************
 *           MENU_ButtonUp
 *
 * Handle a button-up event in a menu. Point is in screen coords.
 * hmenuCurrent is the top-most visible popup.
 * Return TRUE if we can go on with menu tracking.
 */
static BOOL MENU_ButtonUp( HWND hwndOwner, HMENU hmenu, HMENU FAR *hmenuCurrent,
			   POINT pt )
{
    POPUPMENU *menu;
    MENUITEM *item;
    HMENU hsubmenu = 0;
    UINT id;

    if (!hmenu) return FALSE;  /* Outside all menus */
    menu = (POPUPMENU *) LocalLock( hmenu );
    item = MENU_FindItemByCoords( menu, pt.x, pt.y, &id );
    if (!item)  /* Maybe in system menu */
    {
	if (!MENU_IsInSysMenu( menu, pt )) return FALSE;
	id = SYSMENU_SELECTED;
	hsubmenu = GetSystemMenu( menu->hWnd, FALSE );
    }	

    if (menu->FocusedItem != id) return FALSE;

    if (id != SYSMENU_SELECTED)
    {
	if (!(item->item_flags & MF_POPUP))
	{
	    return MENU_ExecFocusedItem( hwndOwner, hmenu, hmenuCurrent );
	}
	hsubmenu = (HMENU)item->item_id;
    }
      /* Select first item of sub-popup */
    MENU_SelectItem( hwndOwner, hsubmenu, NO_SELECTED_ITEM );
    MENU_SelectNextItem( hwndOwner, hsubmenu );
    return TRUE;
}


/***********************************************************************
 *           MENU_MouseMove
 *
 * Handle a motion event in a menu. Point is in screen coords.
 * hmenuCurrent is the top-most visible popup.
 * Return TRUE if we can go on with menu tracking.
 */
static BOOL MENU_MouseMove( HWND hwndOwner, HMENU hmenu, HMENU FAR *hmenuCurrent,
			    POINT pt )
{
    MENUITEM *item;
    POPUPMENU *menu = (POPUPMENU *) LocalLock( hmenu );
    UINT id = NO_SELECTED_ITEM;

	FUNCTION_START
    if (hmenu)
    {
	item = MENU_FindItemByCoords( menu, pt.x, pt.y, &id );
	if (!item)  /* Maybe in system menu */
	{
	    if (!MENU_IsInSysMenu( menu, pt ))
		id = NO_SELECTED_ITEM;  /* Outside all items */
	    else id = SYSMENU_SELECTED;
	}
    }	
    if (id == NO_SELECTED_ITEM)
    {
	MENU_SelectItem( hwndOwner, *hmenuCurrent, NO_SELECTED_ITEM );
    }
    else if (menu->FocusedItem != id)
    {
	MENU_HideSubPopups( hwndOwner, hmenu );
	MENU_SelectItem( hwndOwner, hmenu, id );
	*hmenuCurrent = MENU_ShowSubPopup( hwndOwner, hmenu, FALSE );
    }
	FUNCTION_END
    return TRUE;
}


/***********************************************************************
 *           MENU_KeyLeft
 *
 * Handle a VK_LEFT key event in a menu.
 * hmenuCurrent is the top-most visible popup.
 */
static void MENU_KeyLeft( HWND hwndOwner, HMENU hmenu, HMENU FAR *hmenuCurrent )
{
    POPUPMENU *menu;
    HMENU hmenutmp, hmenuprev;

    menu = (POPUPMENU *) LocalLock( hmenu );
    hmenuprev = hmenutmp = hmenu;
    while (hmenutmp != *hmenuCurrent)
    {
	hmenutmp = MENU_GetSubPopup( hmenuprev );
	if (hmenutmp != *hmenuCurrent) hmenuprev = hmenutmp;
    }
    MENU_HideSubPopups( hwndOwner, hmenuprev );

    if ((hmenuprev == hmenu) && !(menu->wFlags & MF_POPUP))
    {
	  /* Select previous item on the menu bar */
	MENU_SelectPrevItem( hwndOwner, hmenu );
	if (*hmenuCurrent != hmenu)
	{
	      /* A popup menu was displayed -> display the next one */
	    *hmenuCurrent = MENU_ShowSubPopup( hwndOwner, hmenu, TRUE );
	}
    }
    else *hmenuCurrent = hmenuprev;
}


/***********************************************************************
 *           MENU_KeyRight
 *
 * Handle a VK_RIGHT key event in a menu.
 * hmenuCurrent is the top-most visible popup.
 */
static void MENU_KeyRight( HWND hwndOwner, HMENU hmenu, HMENU FAR *hmenuCurrent )
{
    POPUPMENU *menu;
    HMENU hmenutmp;

    menu = (POPUPMENU *) LocalLock( hmenu );

    if ((menu->wFlags & MF_POPUP) || (*hmenuCurrent != hmenu))
    {
	  /* If already displaying a popup, try to display sub-popup */
	hmenutmp = MENU_ShowSubPopup( hwndOwner, *hmenuCurrent, TRUE );
	if (hmenutmp != *hmenuCurrent)  /* Sub-popup displayed */
	{
	    *hmenuCurrent = hmenutmp;
	    return;
	}
    }

      /* If on menu-bar, go to next item */
    if (!(menu->wFlags & MF_POPUP))
    {
	MENU_HideSubPopups( hwndOwner, hmenu );
	MENU_SelectNextItem( hwndOwner, hmenu );
	if (*hmenuCurrent != hmenu)
	{
	      /* A popup menu was displayed -> display the next one */
	    *hmenuCurrent = MENU_ShowSubPopup( hwndOwner, hmenu, TRUE );
	}
    }
    else if (*hmenuCurrent != hmenu)  /* Hide last level popup */
    {
	HMENU hmenuprev;
	hmenuprev = hmenutmp = hmenu;
	while (hmenutmp != *hmenuCurrent)
	{
	    hmenutmp = MENU_GetSubPopup( hmenuprev );
	    if (hmenutmp != *hmenuCurrent) hmenuprev = hmenutmp;
	}
	MENU_HideSubPopups( hwndOwner, hmenuprev );
	*hmenuCurrent = hmenuprev;
    }
}


/***********************************************************************
 *           MENU_TrackMenu
 *
 * Menu tracking code.
 * If 'x' and 'y' are not 0, we simulate a button-down event at (x,y)
 * before beginning tracking. This is to help menu-bar tracking.
 */
static BOOL MENU_TrackMenu( HMENU hmenu, UINT wFlags, int x, int y,
			    HWND hwnd, const RECT FAR * lprect )
{
    MSG *msg;
    HLOCAL hMsg;
    POPUPMENU *menu;
    HMENU hmenuCurrent = hmenu;
    BOOL fClosed = FALSE, fRemove;
    UINT pos;

	FUNCTION_START
    fEndMenuCalled = FALSE;
    if (!(menu = (POPUPMENU *) LocalLock( hmenu ))) return FALSE;
    if (x && y)
    {
	POINT pt;
	pt.x = x;
	pt.y = y;
	MENU_ButtonDown( hwnd, hmenu, &hmenuCurrent, pt );
    }
    SetCapture( hwnd );
    hMsg = LocalAlloc(LMEM_FIXED, sizeof(MSG) );
    msg = (MSG *)LocalLock( hMsg );
    while (!fClosed)
    {
	if (!MSG_InternalGetMessage( msg/*LocalLock(hMsg)*/, 0,
                                     hwnd, MSGF_MENU, 0, TRUE ))
	    break;
//TRACE("track 1");
        fRemove = FALSE;
TRACE("track 2 msg=%04x", msg->message);
	if ((msg->message >= WM_MOUSEFIRST) && (msg->message <= WM_MOUSELAST))
	{
	      /* Find the sub-popup for this mouse event (if any) */
	    HMENU hsubmenu = MENU_FindMenuByCoords( hmenu, msg->pt );

TRACE("track 3");

	    switch(msg->message)
	    {
	    case WM_RBUTTONDOWN:
	    case WM_NCRBUTTONDOWN:
		if (!(wFlags & TPM_RIGHTBUTTON)) break;
		/* fall through */
	    case WM_LBUTTONDOWN:
	    case WM_NCLBUTTONDOWN:
		fClosed = !MENU_ButtonDown( hwnd, hsubmenu,
					    &hmenuCurrent, msg->pt );
		fRemove = TRUE;   // <-- добавить эту строку
		break;

	    case WM_RBUTTONUP:
	    case WM_NCRBUTTONUP:
		if (!(wFlags & TPM_RIGHTBUTTON)) break;
		/* fall through */
	    case WM_LBUTTONUP:
	    case WM_NCLBUTTONUP:
		  /* If outside all menus but inside lprect, ignore it */
		if (!hsubmenu && lprect && PtInRect( lprect, msg->pt )) break;
		fClosed = !MENU_ButtonUp( hwnd, hsubmenu,
					  &hmenuCurrent, msg->pt );
                fRemove = TRUE;  /* Remove event even if outside menu */
		break;

		
/*	    case WM_MOUSEMOVE:
	    case WM_NCMOUSEMOVE:
		if ((msg->wParam & MK_LBUTTON) ||
		    ((wFlags & TPM_RIGHTBUTTON) && (msg->wParam & MK_RBUTTON)))
		{
		    fClosed = !MENU_MouseMove( hwnd, hsubmenu,
					       &hmenuCurrent, msg->pt );
		}
		break;
*/
case WM_MOUSEMOVE:
case WM_NCMOUSEMOVE:
TRACE("track 3.3");
    MENU_MouseMove( hwnd, hsubmenu, &hmenuCurrent, msg->pt );
    fRemove = TRUE;
    break;
	    }
	}
	else if ((msg->message >= WM_KEYFIRST) && (msg->message <= WM_KEYLAST))
	{
            fRemove = TRUE;  /* Keyboard messages are always removed */
	    switch(msg->message)
	    {
	    case WM_KEYDOWN:
		switch(msg->wParam)
		{
		case VK_HOME:
		    MENU_SelectItem( hwnd, hmenuCurrent, NO_SELECTED_ITEM );
		    MENU_SelectNextItem( hwnd, hmenuCurrent );
		    break;

		case VK_END:
		    MENU_SelectItem( hwnd, hmenuCurrent, NO_SELECTED_ITEM );
		    MENU_SelectPrevItem( hwnd, hmenuCurrent );
		    break;

		case VK_UP:
		    MENU_SelectPrevItem( hwnd, hmenuCurrent );
		    break;

		case VK_DOWN:
		      /* If on menu bar, pull-down the menu */
		    if (!(menu->wFlags & MF_POPUP) && (hmenuCurrent == hmenu))
			hmenuCurrent = MENU_ShowSubPopup( hwnd, hmenu, TRUE );
		    else
			MENU_SelectNextItem( hwnd, hmenuCurrent );
		    break;

		case VK_LEFT:
		    MENU_KeyLeft( hwnd, hmenu, &hmenuCurrent );
		    break;
		    
		case VK_RIGHT:
		    MENU_KeyRight( hwnd, hmenu, &hmenuCurrent );
		    break;
		    
		case VK_SPACE:
		case VK_RETURN:
		    fClosed = !MENU_ExecFocusedItem( hwnd, hmenuCurrent,
						     &hmenuCurrent );
		    break;

		case VK_ESCAPE:
		    fClosed = TRUE;
		    break;

		default:
		    break;
		}
		break;  /* WM_KEYDOWN */

	    case WM_SYSKEYDOWN:
		switch(msg->wParam)
		{
		case VK_MENU:
		    fClosed = TRUE;
		    break;
		    
		}
		break;  /* WM_SYSKEYDOWN */

	    case WM_CHAR:
		{
		      /* Hack to avoid control chars. */
		      /* We will find a better way real soon... */
		    if ((msg->wParam <= 32) || (msg->wParam >= 127)) break;
		    pos = MENU_FindItemByKey( hwnd, hmenuCurrent, msg->wParam );
		    if (pos == (UINT)-2) fClosed = TRUE;
		    else if (pos == (UINT)-1) MessageBeep(0);
		    else
		    {
			MENU_SelectItem( hwnd, hmenuCurrent, pos );
			fClosed = !MENU_ExecFocusedItem( hwnd, hmenuCurrent,
							 &hmenuCurrent );
			
		    }
		}		    
		break;  /* WM_CHAR */
	    }  /* switch(msg->message) */
	}
	else
	{
	    DispatchMessage( msg );
	}
	if (fEndMenuCalled) fClosed = TRUE;
	if (!fClosed) fRemove = TRUE;

        if (fRemove)  /* Remove the message from the queue */
	    PeekMessage( msg, 0, msg->message, msg->message, PM_REMOVE );
    }
    LocalFree( hMsg );
    ReleaseCapture();
    MENU_HideSubPopups( hwnd, hmenu );
    if (menu->wFlags & MF_POPUP) ShowWindow( menu->hWnd, SW_HIDE );
    MENU_SelectItem( hwnd, hmenu, NO_SELECTED_ITEM );
    SendMessage( hwnd, WM_MENUSELECT, 0, MAKELONG( 0xffff, 0 ) );
    fEndMenuCalled = FALSE;
	FUNCTION_END
    return TRUE;
}


/***********************************************************************
 *           MENU_TrackMouseMenuBar
 *
 * Menu-bar tracking upon a mouse event. Called from NC_HandleSysCommand().
 */
VOID MENU_TrackMouseMenuBar( HWND hWnd, POINT pt )
{
	WND *wndPtr;

	PushDS();
	SetUserHeapDS();
	FUNCTION_START

	wndPtr = WIN_FindWndPtr( hWnd );

	HideCaret(0);
	SendMessage( hWnd, WM_ENTERMENULOOP, 0, 0 );
	SendMessage( hWnd, WM_INITMENU, wndPtr->wIDmenu, 0 );
	MENU_TrackMenu( (HMENU)wndPtr->wIDmenu, TPM_LEFTALIGN | TPM_LEFTBUTTON,
		    pt.x, pt.y, hWnd, NULL );
	SendMessage( hWnd, WM_EXITMENULOOP, 0, 0 );
	ShowCaret(0);

	LocalUnlock(hWnd);
	FUNCTION_END
	PopDS();
}


/***********************************************************************
 *           MENU_TrackKbdMenuBar
 *
 * Menu-bar tracking upon a keyboard event. Called from NC_HandleSysCommand().
 */
void MENU_TrackKbdMenuBar( WND* wndPtr, UINT wParam, int vkey)
{
    UINT uItem = NO_SELECTED_ITEM;
   HMENU hTrackMenu; 

    /* find window that has a menu 
     */
 
    if( !(wndPtr->dwStyle & WS_CHILD) )
      {
	  wndPtr = WIN_FindWndPtr( GetActiveWindow() );
          if( !wndPtr ) return;
      }
    else
      while( wndPtr->dwStyle & WS_CHILD && 
           !(wndPtr->dwStyle & WS_SYSMENU) )
           if( !(wndPtr = wndPtr->parent) ) return;
          
    if( wndPtr->dwStyle & WS_CHILD || !wndPtr->wIDmenu )
      if( !(wndPtr->dwStyle & WS_SYSMENU) )
	return;

    hTrackMenu = ( IsMenu( wndPtr->wIDmenu ) )? wndPtr->wIDmenu:
                                                wndPtr->hSysMenu;

    HideCaret(0);
    SendMessage( wndPtr->hwndSelf, WM_ENTERMENULOOP, 0, 0 );
    SendMessage( wndPtr->hwndSelf, WM_INITMENU, wndPtr->wIDmenu, 0 );

    /* find suitable menu entry 
     */

    if( vkey == VK_SPACE )
        uItem = SYSMENU_SELECTED;
    else if( vkey )
      {
        uItem = MENU_FindItemByKey( wndPtr->hwndSelf, wndPtr->wIDmenu, vkey );
	if( uItem >= 0xFFFE )
	  {
	    if( uItem == 0xFFFF ) 
                MessageBeep(0);
	    SendMessage( wndPtr->hwndSelf, WM_EXITMENULOOP, 0, 0 );
            ShowCaret(0);
	    return;
	  }
      }

    MENU_SelectItem( wndPtr->hwndSelf, hTrackMenu, uItem );
    if( uItem == NO_SELECTED_ITEM )
      MENU_SelectNextItem( wndPtr->hwndSelf, hTrackMenu );
    else
      PostMessage( wndPtr->hwndSelf, WM_KEYDOWN, VK_DOWN, 0L );

    MENU_TrackMenu( hTrackMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON,
		    0, 0, wndPtr->hwndSelf, NULL );

    SendMessage( wndPtr->hwndSelf, WM_EXITMENULOOP, 0, 0 );
    ShowCaret(0);
}


/**********************************************************************
 *           TrackPopupMenu   (USER.416)
 */
BOOL WINAPI TrackPopupMenu( HMENU hMenu, UINT wFlags, int x, int y,
		     int nReserved, HWND hWnd, const RECT FAR * lpRect )
{
    BOOL ret;
    HideCaret(0);
    if (!MENU_ShowPopup( hWnd, hMenu, 0, x, y )) 
	ret = FALSE;
    else
	ret = MENU_TrackMenu( hMenu, wFlags, 0, 0, hWnd, lpRect );
    ShowCaret(0);
    return ret;
}


/***********************************************************************
 *           PopupMenuWndProc
 */
LRESULT WINAPI PopupMenuWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{    
    switch(message)
    {
    case WM_CREATE:
	{
	    CREATESTRUCT FAR *createStruct = (CREATESTRUCT FAR *)lParam;
	    HMENU hmenu = (HMENU) ((int)createStruct->lpCreateParams & 0xffff);
	    SetWindowWord( hwnd, 0, hmenu );
	    return 0;
	}

    case WM_MOUSEACTIVATE:  /* We don't want to be activated */
	return MA_NOACTIVATE;

    case WM_PAINT:
	{
	    PAINTSTRUCT ps;
	    BeginPaint( hwnd, &ps );
	    MENU_DrawPopupMenu(hwnd, ps.hdc, (HMENU)GetWindowWord( hwnd, 0 ));
	    EndPaint( hwnd, &ps );
	    return 0;
	}

    default:
	return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}


/***********************************************************************
 *           MENU_GetMenuBarHeight
 *
 * Compute the size of the menu bar height. Used by NC_HandleNCCalcSize().
 */
UINT MENU_GetMenuBarHeight( HWND hwnd, UINT menubarWidth, int orgX, int orgY )
{
    HDC hdc;
    RECT rectBar;
    WND *wndPtr;
    POPUPMENU *ppop;

    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return 0;
    if (!(ppop = (POPUPMENU *)LocalLock((HMENU)wndPtr->wIDmenu)))
      return 0;
    hdc = GetDC( hwnd );
    SetRect( &rectBar, orgX, orgY, orgX+menubarWidth, orgY+GETSYSTEMMETRICS(SM_CYMENU) );
    MENU_MenuBarCalcSize( hdc, &rectBar, ppop, hwnd );
    ReleaseDC( hwnd, hdc );
	LocalUnlock((HMENU)wndPtr->wIDmenu);
    return ppop->Height;
}


/*******************************************************************
 *         ChangeMenu    (USER.153)
 */
BOOL WINAPI ChangeMenu( HMENU hMenu, UINT pos, LPCSTR data, UINT id, UINT flags )
{
//    dprintf_menu( stddeb,"ChangeMenu: menu=%04x pos=%d data=%08lx id=%04x flags=%04x\n",
//                  hMenu, pos, (DWORD)data, id, flags );
    if (flags & MF_APPEND)
    {
        return AppendMenu( hMenu, flags & ~MF_APPEND, id, data );
    }
    if (flags & MF_DELETE)
    {
        /* FIXME: Word passes the item id in 'pos' and 0 or 0xffff as id */
        /* for MF_DELETE. We should check the parameters for all others */
        /* MF_* actions also (anybody got a doc on ChangeMenu?). */
        return DeleteMenu( hMenu, pos, flags & ~MF_DELETE );
    }
    if (flags & MF_CHANGE)
    {
        return ModifyMenu( hMenu, pos, flags & ~MF_CHANGE, id, data );
    }
    if (flags & MF_REMOVE)
    {
        return RemoveMenu( hMenu, flags & MF_BYPOSITION ? pos : id,
                           flags & ~MF_REMOVE );
    }
    /* Default: MF_INSERT */
    return InsertMenu( hMenu, pos, flags, id, data );
}


/*******************************************************************
 *         CheckMenuItem    (USER.154)
 */
int WINAPI CheckMenuItem( HMENU hMenu, UINT id, UINT flags )
{
    MENUITEM *item;
    int ret;

//    dprintf_menu( stddeb,"CheckMenuItem: %04x %04x %04x\n", hMenu, id, flags );
    if (!(item = MENU_FindItem( &hMenu, &id, flags ))) return -1;
    ret = item->item_flags & MF_CHECKED;
    if (flags & MF_CHECKED) item->item_flags |= MF_CHECKED;
    else item->item_flags &= ~MF_CHECKED;
    return ret;
}


/**********************************************************************
 *			EnableMenuItem		[USER.155]
 */
BOOL WINAPI EnableMenuItem(HMENU hMenu, UINT wItemID, UINT wFlags)
{
    LPMENUITEM 	lpitem;
//    dprintf_menu(stddeb,"EnableMenuItem (%04x, %04X, %04X) !\n", 
//		 hMenu, wItemID, wFlags);
    if (!(lpitem = MENU_FindItem( &hMenu, &wItemID, wFlags ))) return FALSE;

      /* We can't have MF_GRAYED and MF_DISABLED together */
    if (wFlags & MF_GRAYED)
    {
	lpitem->item_flags = (lpitem->item_flags & ~MF_DISABLED) | MF_GRAYED;
    }
    else if (wFlags & MF_DISABLED)
    {
	lpitem->item_flags = (lpitem->item_flags & ~MF_GRAYED) | MF_DISABLED;
    }
    else   /* MF_ENABLED */
    {
	lpitem->item_flags &= ~(MF_GRAYED | MF_DISABLED);
    }
    return TRUE;
}


/*******************************************************************
 *         GetMenuString    (USER.161)
 */
int WINAPI GetMenuString( HMENU hMenu, UINT wItemID,
                   LPSTR str, int nMaxSiz, UINT wFlags )
{
    LPMENUITEM lpitem;

//    dprintf_menu( stddeb, "GetMenuString: menu=%04x item=%04x ptr=%p len=%d flags=%04x\n",
//                 hMenu, wItemID, str, nMaxSiz, wFlags );
    if (!str || !nMaxSiz) return 0;
    str[0] = '\0';
    if (!(lpitem = MENU_FindItem( &hMenu, &wItemID, wFlags ))) return 0;
    if (!IS_STRING_ITEM(lpitem->item_flags)) return 0;
    lstrcpyn( str, (char *)LocalLock(lpitem->hText), nMaxSiz );
//    dprintf_menu( stddeb, "GetMenuString: returning '%s'\n", str );
    return lstrlen(str);
}


/**********************************************************************
 *			HiliteMenuItem		[USER.162]
 */
BOOL WINAPI HiliteMenuItem(HWND hWnd, HMENU hMenu, UINT wItemID, UINT wHilite)
{
    POPUPMENU * menu;
    LPMENUITEM  lpitem;
//    dprintf_menu(stddeb,"HiliteMenuItem(%04x, %04x, %04x, %04x);\n", 
//                 hWnd, hMenu, wItemID, wHilite);
    if (!(lpitem = MENU_FindItem( &hMenu, &wItemID, wHilite ))) return FALSE;
    if (!(menu = (POPUPMENU *) LocalLock(hMenu))) return FALSE;
    if (menu->FocusedItem == wItemID) return TRUE;
    MENU_HideSubPopups( hWnd, hMenu );
    MENU_SelectItem( hWnd, hMenu, wItemID );
    return TRUE;
}


/**********************************************************************
 *			GetMenuState		[USER.250]
 */
UINT WINAPI GetMenuState(HMENU hMenu, UINT wItemID, UINT wFlags)
{
    LPMENUITEM lpitem;
//    dprintf_menu(stddeb,"GetMenuState(%04x, %04x, %04x);\n", 
//		 hMenu, wItemID, wFlags);
    if (!(lpitem = MENU_FindItem( &hMenu, &wItemID, wFlags ))) return -1;
    if (lpitem->item_flags & MF_POPUP)
    {
	POPUPMENU *menu = (POPUPMENU *) LocalLock( (HMENU)lpitem->item_id );
	if (!menu) return -1;
	else return (menu->nItems << 8) | (menu->wFlags & 0xff);
    }
    else return lpitem->item_flags;
}


/**********************************************************************
 *			GetMenuItemCount		[USER.263]
 */
int WINAPI GetMenuItemCount(HMENU hMenu)
{
	POPUPMENU	* menu;
//	dprintf_menu(stddeb,"GetMenuItemCount(%04x);\n", hMenu);
	menu = (POPUPMENU *) LocalLock(hMenu);
	if (menu == NULL) return -1;
//	dprintf_menu(stddeb,"GetMenuItemCount(%04x) return %d \n", 
//		     hMenu, menu->nItems);
	return menu->nItems;
}


/**********************************************************************
 *			GetMenuItemID			[USER.264]
 */
UINT WINAPI GetMenuItemID(HMENU hMenu, int nPos)
{
    POPUPMENU	* menu;
    MENUITEM *item;

//    dprintf_menu(stddeb,"GetMenuItemID(%04x, %d);\n", hMenu, nPos);
    if (!(menu = (POPUPMENU *) LocalLock(hMenu))) return -1;
    if ((nPos < 0) || (nPos >= menu->nItems)) return -1;
    item = (MENUITEM *) LocalLock( menu->hItems );
    if (item[nPos].item_flags & MF_POPUP) return -1;
    return item[nPos].item_id;
}


/*******************************************************************
 *         InsertMenu    (USER.410)
 */
BOOL WINAPI InsertMenu( HMENU hMenu, UINT pos, UINT flags, UINT id, LPCSTR data )
{
    MENUITEM *item;

	FUNCTION_START
//    if (IS_STRING_ITEM(flags) && data)
//        dprintf_menu( stddeb, "InsertMenu: %04x %d %04x %04x '%s'\n",
//                      hMenu, pos, flags, id, (char *)PTR_SEG_TO_LIN(data) );
//    else dprintf_menu( stddeb, "InsertMenu: %04x %d %04x %04x %08lx\n",
//                       hMenu, pos, flags, id, (DWORD)data );

    if (!(item = MENU_InsertItem( hMenu, pos, flags ))) return FALSE;

    if (!(MENU_SetItemData( item, flags, id, data )))
    {
        RemoveMenu( hMenu, pos, flags );
        return FALSE;
    }

    if (flags & MF_POPUP)  /* Set the MF_POPUP flag on the popup-menu */
	((POPUPMENU *)LocalLock((HMENU)id))->wFlags |= MF_POPUP;

    item->hCheckBit   = hbitmapStdCheck;
    item->hUnCheckBit = 0;
	FUNCTION_END
    return TRUE;
}


/*******************************************************************
 *         AppendMenu    (USER.411)
 */
BOOL WINAPI AppendMenu( HMENU hMenu, UINT flags, UINT id, LPCSTR data )
{
    return InsertMenu( hMenu, -1, flags | MF_BYPOSITION, id, data );
}


/**********************************************************************
 *			RemoveMenu		[USER.412]
 */
BOOL WINAPI RemoveMenu(HMENU hMenu, UINT nPos, UINT wFlags)
{
    POPUPMENU	* menu;
    LPMENUITEM 	lpitem;
	FUNCTION_START
//	dprintf_menu(stddeb,"RemoveMenu (%04x, %04x, %04x) !\n", 
//		     hMenu, nPos, wFlags);
    if (!(lpitem = MENU_FindItem( &hMenu, &nPos, wFlags ))) return FALSE;
    if (!(menu = (POPUPMENU *) LocalLock(hMenu))) return FALSE;
    
      /* Remove item */

    if (IS_STRING_ITEM(lpitem->item_flags) && lpitem->hText)
	LocalFree(lpitem->hText);
    if (--menu->nItems == 0)
    {
	LocalFree( menu->hItems );
	menu->hItems = 0;
    }
    else
    {
	while(nPos < menu->nItems)
	{
	    *lpitem = *(lpitem+1);
	    lpitem++;
	    nPos++;
	}
	menu->hItems = LocalReAlloc( menu->hItems,
					  menu->nItems * sizeof(MENUITEM), LMEM_MODIFY  );
    }
	FUNCTION_END
    return TRUE;
}


/**********************************************************************
 *			DeleteMenu		[USER.413]
 */
BOOL WINAPI DeleteMenu(HMENU hMenu, UINT nPos, UINT wFlags)
{
    MENUITEM *item = MENU_FindItem( &hMenu, &nPos, wFlags );
    if (!item) return FALSE;
    if (item->item_flags & MF_POPUP) DestroyMenu( (HMENU)item->item_id );
      /* nPos is now the position of the item */
    RemoveMenu( hMenu, nPos, wFlags | MF_BYPOSITION );
    return TRUE;
}


/*******************************************************************
 *         ModifyMenu    (USER.414)
 */
BOOL WINAPI ModifyMenu( HMENU hMenu, UINT pos, UINT flags, UINT id, LPCSTR data )
{
    MENUITEM *item;

    if (IS_STRING_ITEM(flags))
    {
//	dprintf_menu( stddeb, "ModifyMenu: %04x %d %04x %04x '%s'\n",
//                      hMenu, pos, flags, id,
//                      data ? (char *)PTR_SEG_TO_LIN(data) : "#NULL#");
        if (!data) return FALSE;
    }
    else ;
//	dprintf_menu( stddeb, "ModifyMenu: %04x %d %04x %04x %08lx\n",
//                      hMenu, pos, flags, id, (DWORD)data );
    if (!(item = MENU_FindItem( &hMenu, &pos, flags ))) return FALSE;

    return MENU_SetItemData( item, flags, id, data );
}


/**********************************************************************
 *			CreatePopupMenu		[USER.415]
 */
HMENU WINAPI CreatePopupMenu()
{
	HMENU hmenu;
	POPUPMENU *menu;

	FUNCTION_START

	if (!(hmenu = CreateMenu())) return 0;
	menu = (POPUPMENU *) LocalLock( hmenu );
	menu->wFlags |= MF_POPUP;
	LocalUnlock(hmenu);

	FUNCTION_END
	return hmenu;
}


/**********************************************************************
 *			GetMenuCheckMarkDimensions	[USER.417]
 */
DWORD WINAPI GetMenuCheckMarkDimensions()
{
	DWORD retVal;
	
	PushDS();
	SetUserHeapDS();
	
	retVal=MAKELONG(check_bitmap_width, check_bitmap_height);

	PopDS();
	return retVal;
}


/**********************************************************************
 *			SetMenuItemBitmaps	[USER.418]
 */
BOOL WINAPI SetMenuItemBitmaps(HMENU hMenu, UINT nPos, UINT wFlags,
                        HBITMAP hNewUnCheck, HBITMAP hNewCheck)
{
    LPMENUITEM lpitem;
//   dprintf_menu(stddeb,"SetMenuItemBitmaps(%04x, %04x, %04x, %04x, %04x)\n",
//                hMenu, nPos, wFlags, hNewCheck, hNewUnCheck);
    if (!(lpitem = MENU_FindItem( &hMenu, &nPos, wFlags ))) return FALSE;

    if (!hNewCheck && !hNewUnCheck)
    {
	  /* If both are NULL, restore default bitmaps */
	lpitem->hCheckBit   = hbitmapStdCheck;
	lpitem->hUnCheckBit = 0;
	lpitem->item_flags &= ~MF_USECHECKBITMAPS;
    }
    else  /* Install new bitmaps */
    {
	lpitem->hCheckBit   = hNewCheck;
	lpitem->hUnCheckBit = hNewUnCheck;
	lpitem->item_flags |= MF_USECHECKBITMAPS;
    }
    return TRUE;
}


/**********************************************************************
 *			CreateMenu		[USER.151]
 */
HMENU WINAPI CreateMenu()
{
	HMENU hMenu;
	LPPOPUPMENU lpMenu;

	PushDS();
	SetUserHeapDS();
	FUNCTION_START

	hMenu = LocalAlloc(LHND, sizeof(POPUPMENU));
    	if (hMenu)
	{
    		lpMenu = (LPPOPUPMENU) LocalLock(hMenu);
    		lpMenu->wMagic = MENU_MAGIC;
    		lpMenu->FocusedItem = NO_SELECTED_ITEM;
		LocalUnlock(hMenu);
	}

	FUNCTION_END
	PopDS();
	return hMenu;
}


/**********************************************************************
 *			DestroyMenu		[USER.152]
 */
BOOL WINAPI DestroyMenu(HMENU hMenu)
{
    POPUPMENU * lppop;
	FUNCTION_START
//    dprintf_menu(stddeb,"DestroyMenu (%04x) !\n", hMenu);
    if (hMenu == 0) return FALSE;
    lppop = (POPUPMENU *) LocalLock(hMenu);
    if (!lppop || (lppop->wMagic != MENU_MAGIC)) return FALSE;
    lppop->wMagic = 0;  /* Mark it as destroyed */
    if ((lppop->wFlags & MF_POPUP) && lppop->hWnd)
        DestroyWindow( lppop->hWnd );

    if (lppop->hItems)
    {
        int i;
        MENUITEM *item = (MENUITEM *) LocalLock( lppop->hItems );
        for (i = lppop->nItems; i > 0; i--, item++)
        {
            if (item->item_flags & MF_POPUP)
                DestroyMenu( (HMENU)item->item_id );
	    if (IS_STRING_ITEM(item->item_flags) && item->hText)
		LocalFree(item->hText);
        }
        LocalFree( lppop->hItems );
    }
    LocalFree( hMenu );
//    dprintf_menu(stddeb,"DestroyMenu (%04x) // End !\n", hMenu);
	FUNCTION_END
    return TRUE;
}

/**********************************************************************
 *			GetSystemMenu		[USER.156]
 */
HMENU WINAPI GetSystemMenu(HWND hWnd, BOOL bRevert)
{
	WND *wndPtr;
	HMENU retVal;

	PushDS();
	SetDS(USER_HeapSel);

	FUNCTION_START

	retVal=0;
	wndPtr = WIN_FindWndPtr( hWnd );
    	if (!wndPtr) 
	{
		LocalUnlock(hWnd);
		PopDS();
		return 0;
	}

    	if (!bRevert) 
	{
		LocalUnlock(hWnd);
		PopDS();
		return wndPtr->hSysMenu;
	}

	if (wndPtr->hSysMenu) DestroyMenu(wndPtr->hSysMenu);
	wndPtr->hSysMenu = MENU_CopySysMenu();

	retVal=wndPtr->hSysMenu;
	LocalUnlock(hWnd);

	FUNCTION_END
	PopDS();

	return retVal;
}


/*******************************************************************
 *         SetSystemMenu    (USER.280)
 */
BOOL WINAPI SetSystemMenu( HWND hwnd, HMENU hMenu )
{
    WND *wndPtr;

    if (!(wndPtr = WIN_FindWndPtr(hwnd))) return FALSE;
    if (wndPtr->hSysMenu) DestroyMenu( wndPtr->hSysMenu );
    wndPtr->hSysMenu = hMenu;
    return TRUE;
}


/**********************************************************************
 *			GetMenu		[USER.157]
 */
HMENU WINAPI GetMenu(HWND hWnd) 
{ 
	WND * wndPtr;
	HMENU retVal;

	PushDS();
	SetDS(USER_HeapSel);
	FUNCTION_START
	
	retVal=0;
	wndPtr = WIN_FindWndPtr(hWnd);
	if (wndPtr)
	{
		retVal=(HMENU)wndPtr->wIDmenu;
	}

	LocalUnlock(hWnd);

	FUNCTION_END
	PopDS();

	return retVal;
}


/**********************************************************************
 * 			SetMenu 	[USER.158]
 */
BOOL WINAPI SetMenu(HWND hWnd, HMENU hMenu)
{
	POPUPMENU *lpmenu;
	WND * wndPtr = WIN_FindWndPtr(hWnd);
	if (wndPtr == NULL) {
//		fprintf(stderr,"SetMenu(%04x, %04x) // Bad window handle !\n",
//			hWnd, hMenu);
		return FALSE;
		}
//	dprintf_menu(stddeb,"SetMenu(%04x, %04x);\n", hWnd, hMenu);
	if (GetCapture() == hWnd) ReleaseCapture();
	wndPtr->wIDmenu = (UINT)hMenu;
	if (hMenu != 0)
	{
	    lpmenu = (POPUPMENU *) LocalLock(hMenu);
	    if (lpmenu == NULL) {
//		fprintf(stderr,"SetMenu(%04x, %04x) // Bad menu handle !\n", 
//			hWnd, hMenu);
		return FALSE;
		}
	    lpmenu->hWnd = hWnd;
	    lpmenu->wFlags &= ~MF_POPUP;  /* Can't be a popup */
	    lpmenu->Height = 0;  /* Make sure we recalculate the size */
	}
	SetWindowPos( hWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE |
		      SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED );
	return TRUE;
}



/**********************************************************************
 *			GetSubMenu		[USER.159]
 */
HMENU WINAPI GetSubMenu(HMENU hMenu, int nPos)
{
    POPUPMENU * lppop;
    MENUITEM *	lpitem;
//    dprintf_menu(stddeb,"GetSubMenu (%04x, %04X) !\n", hMenu, nPos);
    if (!(lppop = (POPUPMENU *) LocalLock(hMenu))) return 0;
    if ((UINT)nPos >= lppop->nItems) return 0;
    lpitem = (MENUITEM *) LocalLock( lppop->hItems );
    if (!(lpitem[nPos].item_flags & MF_POPUP)) return 0;
    return (HMENU)lpitem[nPos].item_id;
}


/**********************************************************************
 *			DrawMenuBar		[USER.160]
 */
VOID WINAPI DrawMenuBar(HWND hWnd)
{
	WND *wndPtr;
	LPPOPUPMENU lppop;

	FUNCTION_START
//	dprintf_menu(stddeb,"DrawMenuBar (%04x)\n", hWnd);
	wndPtr = WIN_FindWndPtr(hWnd);
	if (wndPtr && (wndPtr->dwStyle & WS_CHILD) == 0 && 
		wndPtr->wIDmenu != 0) {
//		dprintf_menu(stddeb,"DrawMenuBar wIDmenu=%04X \n", 
//			     wndPtr->wIDmenu);
		lppop = (POPUPMENU *) LocalLock((HMENU)wndPtr->wIDmenu);
		if (lppop == NULL) return;

		lppop->Height = 0; /* Make sure we call MENU_MenuBarCalcSize */
		SetWindowPos( hWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE |
			    SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED );
	    }
	FUNCTION_END
}


/***********************************************************************
 *           EndMenu   (USER.187)
 */
void WINAPI EndMenu(void)
{
    /* FIXME: this won't work when we have multiple tasks... */
    fEndMenuCalled = TRUE;
}


/***********************************************************************
 *           LookupMenuHandle   (USER.217)
 */
HMENU WINAPI LookupMenuHandle( HMENU hMenu, UINT id )
{
    if (!MENU_FindItem( &hMenu, &id, MF_BYCOMMAND )) return 0;
    else return hMenu;
}


/**********************************************************************
 *	    LoadMenu    (USER.150)
 */
HMENU WINAPI LoadMenu( HINSTANCE instance, LPCSTR name )
{
	HRSRC hRsrc;
	HGLOBAL handle;
	HMENU hMenu=0;

	FUNCTION_START

	if (HIWORD(name))
	{
		const char FAR *str = name;
		TRACE("LoadMenu(%04x,'%s')\n", instance, str );
		if (str[0] == '#') name = (LPSTR)latoi( str + 1 );
	}
	else
		TRACE("LoadMenu(%04x,%04x)\n",instance,LOWORD(name));

	if (!name) return 0;
    
	instance = GetExePtr( instance );

    /* check for Win32 module */
//    if(((NE_MODULE*)GlobalLock(instance))->flags & NE_FFLAGS_WIN32)
//        return WIN32_LoadMenuA(instance,PTR_SEG_TO_LIN(name));

	if (!(hRsrc = FindResource( instance, name, RT_MENU ))) return 0;
	if (!(handle = LoadResource( instance, hRsrc ))) return 0;

	hMenu = LoadMenuIndirect( LockResource(handle));

	UnlockResource(handle);
	FreeResource( handle );

	FUNCTION_END
	return hMenu;
}


/**********************************************************************
 *	    LoadMenuIndirect    (USER.220)
 */
HMENU WINAPI LoadMenuIndirect( const VOID FAR * template )
{
	HMENU hMenu;

	FUNCTION_START

	TRACE("LoadMenuIndirect: %x:%x\n", template);

	hMenu = CreateMenu();

	if (hMenu)
	{
		template = (LPBYTE)template+sizeof(MENU_HEADER);
		if (!MENU_ParseResource(template, hMenu ))
		{
			DestroyMenu( hMenu );
			FUNCTION_END
			return (HMENU)0;
		}
	}
	FUNCTION_END
	return hMenu;
}


/**********************************************************************
 *		IsMenu    (USER.358)
 */
BOOL WINAPI IsMenu(HMENU hMenu)
{
	LPPOPUPMENU lpMenu;
	BOOL retVal=FALSE;
	
	PushDS();
	SetUserHeapDS();
	FUNCTION_START

	if ((lpMenu = (LPPOPUPMENU)LocalLock( hMenu ))) 
	{
		retVal=(lpMenu->wMagic == MENU_MAGIC);
		LocalUnlock(hMenu);
	}

	FUNCTION_END
	PopDS();
	return retVal;
}

