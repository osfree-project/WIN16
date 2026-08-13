/* 
 *
 * Menu definitions


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

#ifndef MENU_H
#define MENU_H

#define MENU_MAGIC   0x554d  /* 'MU' */

extern BOOL FAR MENU_Init(VOID);
extern UINT MENU_GetMenuBarHeight( HWND hwnd, UINT menubarWidth,
				   int orgX, int orgY );         /* menu.c */
extern void MENU_TrackMouseMenuBar( HWND hwnd, POINT pt );       /* menu.c */
extern void MENU_TrackKbdMenuBar( WND*, UINT wParam, int vkey);  /* menu.c */
extern UINT MENU_DrawMenuBar( HDC hDC, LPRECT lprect,
			      HWND hwnd, BOOL suppress_draw );   /* menu.c */
extern LRESULT WINAPI PopupMenuWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam );

typedef struct tagMENUITEM
{
    WORD	item_flags;    /* Item flags */
    UINT	item_id;       /* Item or popup id */
    RECT	rect;          /* Item area (relative to menu window) */
    WORD        xTab;          /* X position of text after Tab */
    HBITMAP	hCheckBit;     /* Bitmap for checked item */
    HBITMAP	hUnCheckBit;   /* Bitmap for unchecked item */
    HANDLE      hText;	       /* Handle to item string or bitmap */
} MENUITEM, FAR * LPMENUITEM, NEAR * NPMENUITEM, * PMENUITEM;


typedef struct tagPOPUPMENU
{
    HMENU       hNext;        /* Next menu (compatibility only, always 0) */
    WORD        wFlags;       /* Menu flags (MF_POPUP, MF_SYSMENU) */
    WORD        wMagic;       /* Magic number */
    HANDLE      hTaskQ;       /* Task queue for this menu */
    WORD	Width;        /* Width of the whole menu */
    WORD	Height;       /* Height of the whole menu */
    WORD	nItems;       /* Number of items in the menu */
    HWND	hWnd;	      /* Window containing the menu */
    HANDLE      hItems;       /* Handle to the items array */
    UINT	FocusedItem;  /* Currently focused item */
} POPUPMENU, FAR*LPPOPUPMENU;

typedef struct
{
    WORD	version;		/* Should be zero		  */
    WORD	reserved;		/* Must be zero			  */
} MENU_HEADER;

typedef struct
{
    WORD	item_flags;		/* See windows.h		  */
    char	item_text[1];		/* Text for menu item		  */
} MENU_POPUPITEM;

#if 0
typedef struct
{
    WORD	item_flags;		/* See windows.h		  */
    WORD	item_id;		/* Control Id for menu item	  */
    char	item_text[1];		/* Text for menu item		  */
} MENUITEMTEMPLATE;
#endif

#endif /* MENU_H */
