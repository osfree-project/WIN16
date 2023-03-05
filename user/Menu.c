#include <ctype.h>
#include <string.h>

#ifndef OEMRESOURCE
#define OEMRESOURCE	/* for OBM_CLOSE */
#endif
#include <windows.h>
#undef OEMRESOURCE

#include "menu.h"
#include "Listbox.h"


#define GetWindowStyle(hwnd)    ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#define	SetWindowMenu(hWnd,hMenu) ((HMENU)SetWindowLong(hWnd,GWL_HMENU,(LONG)hMenu))
#define	GetWindowSysMenu(hWnd)	((HMENU)GetWindowLong(hWnd,GWL_HSYSMENU))
#define	SetWindowSysMenu(hWnd,hSysMenu) ((HMENU)SetWindowLong(hWnd,GWL_HSYSMENU,(LONG)hSysMenu))

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

LONG LBoxAPI(HMENU hMenu32, UINT uiAction, LPARAM lParam);

#define	GetWindowMenu(hWnd)	((HMENU)GetWindowLong(hWnd,GWL_HMENU))
#define	GetWindowFrame(hWnd)	((HWND)GetWindowLong(hWnd,GWL_HWNDMENU))

//@todo Need to be sure about offsets
#define GWL_HWNDFOCUS   (-36)
#define GWL_DRVDATA	(-44)
#define	GWL_HMENU	(-48)
#define	GWL_HWNDMENU	(-52)
#define	GWL_HDC		(-56)
#define GWL_UPDATE	(-60)
#define	GWL_HSYSMENU	(-64)
#define	GWL_HWNDHZSCROLL (-68)
#define	GWL_HWNDVTSCROLL (-72)

static HBITMAP SystemBitmaps[8];

#define SB_OBM_CLOSE_L	0
#define SB_OBM_SYSMENU	1
#define SB_OBM_RESTORE  2
#define SB_OBM_REDUCE	3
#define SB_OBM_ZOOM	4
#define SB_OBM_RESTORED 5
#define SB_OBM_REDUCED	6
#define SB_OBM_ZOOMD	7

#define	OM_MASK		0x4000

int latoi(char far *h)
{
  char far *s = h;
  int  i = 0;
  int  j, k, l;
  char c;
  int  base;

  if (s[0] == '0' && s[1] == 'x') {
    base = 16;
    s += 2; // Delete "0x"
  } else {
    base = 10;
  }

  l = lstrlen(s) - 1;

  while (*s) {
    c = tolower(*s);

    if ('a' <= c && c <= 'f') {
      if (base == 16) {
        c = c - 'a' + 10;
      } else {
        return 0;
      }
    } else if ('0' <= c && c <= '9') {
      c -= '0';
    } else {
      return 0;
    }

    for (j = 0, k = c; j < l; j++)
      k *= base;

    i += k;
    s++;
    l--;
  }

  return i;
}

HMODULE WINAPI GetExePtr( HANDLE handle );

/**********************************************************************
 *	    LoadMenu    (USER.150)
 */
HMENU WINAPI LoadMenu( HINSTANCE instance, LPCSTR name )
{
    HRSRC hRsrc;
    HGLOBAL handle;
    HMENU hMenu;

    if (HIWORD(name) && name[0] == '#') name = (char far *)latoi( (char far *)name + 1 );
    if (!name) return 0;

    instance = GetExePtr( instance );
    if (!(hRsrc = FindResource( instance, name, (LPSTR)RT_MENU ))) return 0;
    if (!(handle = LoadResource( instance, hRsrc ))) return 0;
    hMenu = LoadMenuIndirect(LockResource(handle));
    FreeResource( handle );
    return hMenu;
}

/*******************************************************************
 *         ChangeMenu    (USER.153)
 */
BOOL WINAPI ChangeMenu( HMENU hMenu, UINT pos, const char far * data,
                            UINT id, UINT flags )
{
    if (flags & MF_APPEND) return AppendMenu( hMenu, flags & ~MF_APPEND, id, data );

    /* FIXME: Word passes the item id in 'pos' and 0 or 0xffff as id */
    /* for MF_DELETE. We should check the parameters for all others */
    /* MF_* actions also (anybody got a doc on ChangeMenu?). */

    if (flags & MF_DELETE) return DeleteMenu(hMenu, pos, flags & ~MF_DELETE);
    if (flags & MF_CHANGE) return ModifyMenu(hMenu, pos, flags & ~MF_CHANGE, id, data );
    if (flags & MF_REMOVE) return RemoveMenu(hMenu, flags & MF_BYPOSITION ? pos : id,
                                               flags & ~MF_REMOVE );
    /* Default: MF_INSERT */
    return InsertMenu( hMenu, pos, flags, id, data );
}

#define GET_BYTE(ptr)  (*(const BYTE *)(ptr))
#define GET_WORD(ptr)  (*(const WORD *)(ptr))

static LPCSTR parse_menu_resource( LPCSTR res, HMENU hMenu, BOOL oldFormat )
{
    WORD flags, id = 0;
    LPCSTR str;
    BOOL end_flag;

    do
    {
        /* Windows 3.00 and later use a WORD for the flags, whereas 1.x and 2.x use a BYTE. */
        if (oldFormat)
        {
            flags = GET_BYTE(res);
            res += sizeof(BYTE);
        }
        else
        {
            flags = GET_WORD(res);
            res += sizeof(WORD);
        }

        end_flag = flags & MF_END;
        /* Remove MF_END because it has the same value as MF_HILITE */
        flags &= ~MF_END;
        if (!(flags & MF_POPUP))
        {
            id = GET_WORD(res);
            res += sizeof(WORD);
        }
        str = res;
        res += lstrlen(str) + 1;
        if (flags & MF_POPUP)
        {
            HMENU hSubMenu = CreatePopupMenu();
            if (!hSubMenu) return NULL;
            if (!(res = parse_menu_resource( res, hSubMenu, oldFormat ))) return NULL;
            AppendMenu( hMenu, flags, hSubMenu, str );
        }
        else  /* Not a popup */
        {
            AppendMenu( hMenu, flags, id, *str ? str : NULL );
        }
    } while (!end_flag);
    return res;
}

WORD WINAPI GetExeVersion();

/**********************************************************************
 *	    LoadMenuIndirect    (USER.220)
 */
HMENU WINAPI LoadMenuIndirect( VOID const far * template )
{
    BOOL oldFormat;
    HMENU hMenu;
    WORD version, offset;
    LPCSTR p = template;

//    TRACE("(%p)\n", template );

    /* Windows 1.x and 2.x menus have a slightly different menu format from 3.x menus */
    oldFormat = (GetExeVersion() < 0x0300);

    /* Windows 3.00 and later menu items are preceded by a MENUITEMTEMPLATEHEADER structure */
    if (!oldFormat)
    {
        version = GET_WORD(p);
        p += sizeof(WORD);
        if (version)
        {
//            WARN("version must be 0 for Win16 >= 3.00 applications\n" );
            return 0;
        }
        offset = GET_WORD(p);
        p += sizeof(WORD) + offset;
    }

    if (!(hMenu = CreateMenu())) return 0;
    if (!parse_menu_resource( p, hMenu, oldFormat ))
    {
        DestroyMenu( hMenu );
        return 0;
    }
    return hMenu;
}

HMENU WINAPI		/* API */
CreateMenu()
{
    MENUCREATESTRUCT mc;
    DWORD dwRet;

    mc.hFont = GetStockObject(SYSTEM_FONT);
    mc.dwStyle = LBS_PRELOADED|LBS_OWNERDRAWVARIABLE|
		 LBS_HASSTRINGS|LBS_NOTIFY;
    dwRet = GetMenuCheckMarkDimensions();
    mc.dwIndents = MAKELONG(LOWORD(dwRet)+2,LOWORD(dwRet));
    return (HMENU)LBoxAPI(0,LBA_CREATE,(LPARAM)&mc);
}

BOOL WINAPI		/* API */
DestroyMenu(HMENU hMenu)
{
    return LBoxAPI(hMenu,LBA_DESTROY,0);
}

void
MenuDrawItem(HDC hDC, HMENU hMenu, HWND hWndOwner, WORD wPos, WORD wAction)
{
    HFONT hLast,hFont;
    MENUITEMSTRUCT mnis;
    LONG lFlags;
    RECT rcItemRect;
    LPSTR lpItemData;
    DRAWITEMSTRUCT dis;
    HDC hdcMemory;
    HBITMAP hBitmap,hBmpOld;
    BITMAP bm;
    BOOL bInvert;

    mnis.wPosition = (WORD)-1;
    mnis.wAction = LCA_FONT;
    if ((hFont = (HFONT)LBoxAPI(hMenu,LBA_GETDATA,(LPARAM)&mnis)))
	hLast = SelectObject(hDC, hFont);
    else 
	hLast = 0;

    mnis.wPosition = wPos;
    mnis.wItemFlags = MF_BYPOSITION;
    mnis.wAction = LCA_GET | LCA_FLAGS;
    lFlags = LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
    if (lFlags & MF_OWNERDRAW) {
	dis.CtlType = ODT_MENU;
	dis.CtlID = 0;
	mnis.wAction = LCA_GET | LCA_ITEMID;
	dis.itemID = (UINT)((int)((short)LOWORD(LBoxAPI
			(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis))));
	dis.itemAction = wAction;
	dis.itemState = (lFlags & MF_CHECKED)?ODS_CHECKED:0;
	dis.itemState |= (lFlags & MF_DISABLED)?ODS_DISABLED:0;
	dis.itemState |= (lFlags & MF_GRAYED)?ODS_GRAYED:0;
	dis.itemState |= (lFlags & MF_HILITE)?ODS_SELECTED:0;
	dis.hwndItem = ((LPOBJHEAD)hMenu)->hObj;
	dis.hDC = hDC;
	mnis.wAction = LCA_GET | LCA_RECT;
	mnis.lpItemData = (LPSTR)&dis.rcItem;
	LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
	mnis.wAction = LCA_GET | LCA_CONTENTS;
	dis.itemData = (DWORD)LBoxAPI
			(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
	SendMessage(hWndOwner,WM_DRAWITEM,0,(LPARAM)&dis);
    }
    else {
	mnis.wAction = LCA_GET | LCA_RECT;
	mnis.lpItemData = (LPSTR)&rcItemRect;
	LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
	mnis.wAction = LCA_GET | LCA_CONTENTS;
	lpItemData = (LPSTR)LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
	SetBkMode(hDC, TRANSPARENT);
	if (lFlags & MF_BITMAP) {
	    if ((DWORD)lpItemData == SB_OBM_RESTORE) {
		hBitmap = (lFlags & MF_HILITE)?SystemBitmaps[SB_OBM_RESTORED]:
						SystemBitmaps[SB_OBM_RESTORE];
		bInvert = FALSE;
	    }
	    else {
		hBitmap = (HBITMAP)(DWORD)lpItemData;
		bInvert = (lFlags & MF_HILITE) ? TRUE : FALSE;
	    }

	    hdcMemory = CreateCompatibleDC(hDC);
	    GetObject(hBitmap,sizeof(BITMAP),&bm);
	    hBmpOld = SelectObject(hdcMemory,hBitmap);
	    BitBlt(hDC,
			rcItemRect.left,
			rcItemRect.top,
			rcItemRect.right-rcItemRect.left,
			rcItemRect.bottom-rcItemRect.top,
			hdcMemory,0,0,
			SRCCOPY);
	    SelectObject(hdcMemory,hBmpOld);
	    DeleteDC(hdcMemory);
	    if (bInvert) 
		InvertRect(hDC,&rcItemRect);
	}
	else {
	    if (!(lFlags & MF_HILITE)) {
		HBRUSH hbr;
		hbr=CreateSolidBrush ( GetSysColor (COLOR_MENU));
		FillRect(hDC,&rcItemRect,hbr);
		DeleteObject(hbr);
		SetTextColor(hDC,
		    (lFlags & MF_GRAYED)?
			GetSysColor(COLOR_GRAYTEXT):
			GetSysColor(COLOR_MENUTEXT));
	    }
	    else {
		HBRUSH hbr;
		hbr=CreateSolidBrush ( GetSysColor (COLOR_HIGHLIGHT));
		FillRect(hDC,&rcItemRect,hbr);
		DeleteObject(hbr);
		SetTextColor(hDC,
		    (lFlags & MF_GRAYED)?
			GetSysColor(COLOR_GRAYTEXT):
			GetSysColor(COLOR_HIGHLIGHTTEXT));
	    }
	    DrawText(hDC,lpItemData,lstrlen(lpItemData),&rcItemRect,
			DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_EXPANDTABS);
	}
    }
    if(hLast)
	SelectObject(hDC, hLast);
}

DWORD
CalcBorders(DWORD dwStyle, DWORD dwExStyle)
{
    int nXBorder,nYBorder;	

    nXBorder = nYBorder = 0;

    if ((dwStyle & WS_BORDER) ||
        (!(dwStyle & WS_POPUP) && !(dwStyle & WS_CHILD))) {
	nXBorder = GetSystemMetrics(SM_CXBORDER);
	nYBorder = GetSystemMetrics(SM_CYBORDER);
    }

    if ((dwStyle & WS_THICKFRAME) || (dwExStyle & WS_EX_DLGMODALFRAME)) {
	nXBorder = GetSystemMetrics(SM_CXFRAME);
	nYBorder = GetSystemMetrics(SM_CYFRAME);
    }

    if ((dwStyle & WS_DLGFRAME) && !(dwStyle & WS_BORDER)) {
	nXBorder = GetSystemMetrics(SM_CXDLGFRAME);
	nYBorder = GetSystemMetrics(SM_CYDLGFRAME);
    }

    return MAKELONG(nXBorder,nYBorder);
}

void
CalcExpectedNC(LPRECT lpRect, DWORD dwStyle, DWORD dwExStyle)
{
    DWORD dwXYBorders;

    SetRectEmpty(lpRect);

    dwXYBorders = CalcBorders(dwStyle,dwExStyle);
    lpRect->top = lpRect->bottom = HIWORD(dwXYBorders);
    lpRect->left = lpRect->right = LOWORD(dwXYBorders);
    if ((dwStyle & WS_CAPTION) == WS_CAPTION)
	lpRect->top += GetSystemMetrics(SM_CYCAPTION);
}

WORD
MeasureWindowMenu(HWND hWnd, HWND hWndMenu)
{
    HMENU hMenu;
//    HMENU32 hMenu32;
    RECT rcWnd, rcNC, rcItemRect;
    int nMenuWidth,nLineWidth;
    MEASUREITEMSTRUCT mis;
    MENUITEMSTRUCT mnis;
    HBITMAP hBitmap;
    BITMAP bm;
    int nCount,i;
    HDC hDC;
    HFONT hFont;
    TEXTMETRIC tm;
    LONG lFlags;
    WORD wSpacing, wItemHeight, wItemWidth = 0;
    DWORD dwExtent;
    WORD X=0, wLine=0;

    if (!(hMenu = GetWindowMenu(hWnd)))
	return 0;
//    if (!(hMenu32 = GetMenuHandle32(hMenu)))
//	return 0;

    GetWindowRect(hWnd, &rcWnd);
    CalcExpectedNC(&rcNC,GetWindowLong(hWnd,GWL_STYLE),
			 GetWindowLong(hWnd,GWL_EXSTYLE));
    nMenuWidth = rcWnd.right - rcWnd.left - rcNC.left - rcNC.right;
    nLineWidth = nMenuWidth;
    hDC = GetDC(hWndMenu);
    mnis.wAction = LCA_FONT;
    mnis.wPosition = (WORD)-1;
    if ((hFont = (HFONT)LBoxAPI(hMenu,LBA_GETDATA,(LPARAM)&mnis)))
	hFont = SelectObject(hDC, hFont);
    else
	hFont = 0;
    GetTextMetrics(hDC, &tm);
    wSpacing = LOWORD(GetTextExtent(hDC, " ", 1));
    wItemHeight = GetSystemMetrics(SM_CYMENU) - 1;

    mnis.wAction = LCA_ITEMCOUNT;
    nCount = LBoxAPI(hMenu,LBA_GETDATA,(LPARAM)&mnis);
    for (i=0; i<nCount; i++) {
	mnis.wPosition = (WORD)i;
	mnis.wAction = LCA_GET | LCA_FLAGS;
	mnis.wItemFlags = MF_BYPOSITION;
	lFlags = LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
	if (lFlags < 0)
	    continue;
	if (lFlags & MF_OWNERDRAW)
	{
		mis.CtlType = ODT_MENU; /* add some stuff here */
		SendMessage(hWnd, WM_MEASUREITEM, 0, (DWORD)&mis);
	}
	else if (lFlags & MF_BITMAP)
	{
		mnis.wAction = LCA_GET | LCA_CONTENTS;
		mnis.lpItemData = (LPSTR)LBoxAPI(hMenu,LBA_MODIFYITEM,
			(LPARAM)&mnis);
		hBitmap = (HBITMAP)(DWORD)mnis.lpItemData;
		if (hBitmap == SB_OBM_RESTORE) 
			hBitmap = SystemBitmaps[SB_OBM_RESTORE];
		if (!GetObject((HGDIOBJ)hBitmap,sizeof(BITMAP),(LPVOID)&bm))
			continue;
		wItemWidth = bm.bmWidth;
	}
	else
	{
		mnis.wAction = LCA_GET | LCA_CONTENTS;
		mnis.lpItemData = (LPSTR)LBoxAPI(hMenu,LBA_MODIFYITEM,
			(LPARAM)&mnis);
		/* compute item string extent */
		if(mnis.lpItemData)
		{
			char itemstr[256], *pitem;
			int flag;
			/* replace leading '\b' with MF_HELP flag */
			if (*mnis.lpItemData == '\b')
			{
				mnis.lpItemData++;
				lFlags |= MF_HELP;
			}
			/* copy item string (but discard first '&') */
			for (flag = 0, pitem = itemstr;
			     (*pitem++ = *mnis.lpItemData++);
			    )
			{
				if (flag || (pitem[-1] != '&')) continue;
				pitem--;
				flag = 1;
			}
			mnis.lpItemData = itemstr;
			/* get text extent (without '\b' and '&') */
			dwExtent = GetTextExtent(hDC, itemstr,
				lstrlen(itemstr));
		}
		else
			dwExtent = 0;
		wItemWidth = 2*wSpacing+LOWORD(dwExtent);
	}
	if ((int)wItemWidth > nLineWidth)
	{
		wLine++;
		nLineWidth = nMenuWidth;
		X = 0;
	}
	nLineWidth -= wItemWidth;
	if (lFlags & MF_HELP)
	{
		SetRect(&rcItemRect,nMenuWidth-wItemWidth,wLine*wItemHeight,
			nMenuWidth,(wLine+1)*wItemHeight);
	}
	else
	{ 
		SetRect(&rcItemRect,X,wLine*wItemHeight,
			X+wItemWidth,(wLine+1)*wItemHeight);
		X += wItemWidth;
	}
	mnis.wAction = LCA_SET | LCA_RECT;
	mnis.lpItemData = (LPSTR)&rcItemRect;
	LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
    }

    ReleaseDC(hWndMenu,hDC);
//    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    return ((wLine+1)*wItemHeight + 1);
}

/**************************************************************************
 *              DrawMenuBar   (USER.160)
 */
void  WINAPI
DrawMenuBar(HWND hWnd)
{
    HMENU hMenu;
//    HMENU32 hMenu32;
    HWND hWndMenu;
    MENUITEMSTRUCT mnis;
    RECT rcWnd,rc;
    HDC hDC;
    int nCount,i;
    WORD wMenuHeight;
	HBRUSH hbr;

    if (!(hMenu = GetWindowMenu(hWnd)))
	return;
//    if (!(hMenu32 = GetMenuHandle32(hMenu)))
//	return;

    hWndMenu = GetWindowFrame(hWnd);
    GetClientRect(hWndMenu, &rcWnd);
    wMenuHeight = MeasureWindowMenu(hWnd,hWndMenu);
    if (wMenuHeight < (WORD)rcWnd.bottom) {
	//@todo Implement this!!! SetWindowMenuHeight(hWnd,wMenuHeight);
	GetClientRect(hWndMenu, &rcWnd);
    }
    hDC = GetDC(hWndMenu);
    SetRect(&rc,rcWnd.left,rcWnd.top,rcWnd.right,rcWnd.bottom-1);
    hbr=CreateSolidBrush ( GetSysColor (COLOR_MENU));
    FillRect(hDC, &rc, hbr);
    DeleteObject(hbr);
    mnis.wAction = LCA_ITEMCOUNT;
    mnis.wPosition = (WORD)-1;
    nCount = LBoxAPI(hMenu,LBA_GETDATA,(LPARAM)&mnis);
    for (i=0; i<nCount; i++) 
	MenuDrawItem(hDC,hMenu,hWnd,(WORD)i,ODA_DRAWENTIRE);

    SelectObject(hDC, GetStockObject(BLACK_PEN));
    SelectObject(hDC, GetStockObject(NULL_BRUSH));
    MoveTo(hDC,rcWnd.left,rcWnd.bottom-1);
    LineTo(hDC,rcWnd.right,rcWnd.bottom-1);

    ReleaseDC(hWndMenu,hDC);
//    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
}

/**************************************************************************
 *              GetMenu   (USER.157)
 */
HMENU WINAPI
GetMenu(HWND hWnd)
{
    HMENU hMenu;

    hMenu = GetWindowMenu(hWnd);
    return hMenu;
}

/**************************************************************************
 *              SetMenu   (USER.158)
 */
BOOL WINAPI
SetMenu(HWND hWnd, HMENU hMenu)
{
//    HMENU32 hMenu32;
    LPTRACKPOPUPSTRUCT lptps;
    HWND hWndMenu;

//@todo Need to implement
/*    if (!IsTopLevel(hWnd))
	return FALSE;*/
    if (hMenu /*&& !(hMenu32 = GetMenuHandle32(hMenu))*/)
	return FALSE;
    if (hMenu == SetWindowMenu(hWnd, hMenu)) {
        //RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	return TRUE;
    }
    if (!(hWndMenu = GetWindowFrame(hWnd))) {
        //RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	return FALSE;
    }
    lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong(hWndMenu,LWD_LPMENUDATA);
    if (hMenu == 0) {
	if (lptps) {
	    SetWindowLong(hWndMenu, LWD_LPMENUDATA, 0L);
	    GlobalFreePtr((LPSTR)lptps);
	}
	//RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	return TRUE;
    }
    else if (lptps == NULL) {
	lptps = (LPTRACKPOPUPSTRUCT)GlobalAllocPtr(GPTR, sizeof(TRACKPOPUPSTRUCT));
	if (!lptps) {
	    //RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return FALSE;
	}
	_fmemset((LPSTR)lptps,'\0',sizeof(TRACKPOPUPSTRUCT));
	SetWindowLong(hWndMenu, LWD_LPMENUDATA, (LONG)lptps);
    }
    lptps->hMenu = hMenu;
    lptps->hWndOwner = hWnd;
    lptps->uiFlags |= TP_MENUBAR;

//@todo implement this
//    SetWindowMenuHeight(hWnd,MeasureWindowMenu(hWnd,hWndMenu));

//@todo call redraw here
//    TWIN_RedrawWindow(hWnd,NULL,0,RDW_FRAME|RDW_INVALIDATE);
//    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);

    return TRUE;
}

/**************************************************************************
 *              GetSystemMenu   (USER.156)
 */
HMENU WINAPI
GetSystemMenu(HWND hWnd, BOOL bRevert)
{
    HMENU hSysMenu;
    DWORD dwWinStyle;

    hSysMenu = GetWindowSysMenu(hWnd);
    if (!bRevert)
	return hSysMenu;
    else {
	if (hSysMenu)
	    DestroyMenu(hSysMenu);
	dwWinStyle = GetWindowStyle(hWnd);
	hSysMenu = LoadMenu(0, (dwWinStyle & WS_CHILD)?
		"CHILDSYSMENU":"SYSMENU");
	SetWindowSysMenu(hWnd, hSysMenu);
	return (HMENU)0;
    }
}

/**********************************************************************
 *         GetSubMenu    (USER.159)
 */
HMENU WINAPI
GetSubMenu(HMENU hMenu, int nPos)
{
    UINT uiItemID;
//    HMENU32 hMenu32;
    MENUITEMSTRUCT mis;

  //  if (!(hMenu32 = GetMenuHandle32(hMenu)))
//	return 0;
    memset((char *)&mis, '\0', sizeof(MENUITEMSTRUCT));
    mis.wItemFlags = MF_BYPOSITION;
    mis.wPosition = (WORD)nPos;
    mis.wAction = LCA_GET | LCA_ITEMID;
    uiItemID = (UINT)LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mis);
//    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    if (IsMenu((HMENU)uiItemID))
	return (HMENU)uiItemID;
    else
	return 0;
}


/*  WARNING:  This has a reference-counting problem:  If the item is in a
    POPUP menu, that popup's LBOXINFO will be ref-counted when gotten
    and must be "del-ref'd" when the caller is done with it. */
HMENU
TWIN_FindMenuItem(HMENU hMenu, UINT uiIDItem)
{
    UINT uiItem;
    LONG lFlags;
    MENUITEMSTRUCT mnis;
    int nCount,i;
    HMENU hMenuRet;

    if (!hMenu)
	return 0L;
    memset((char *)&mnis, '\0', sizeof(MENUITEMSTRUCT));
    mnis.wPosition = (WORD)-1;
    mnis.wAction = LCA_ITEMCOUNT;
    nCount = (int)LBoxAPI(hMenu,LBA_GETDATA,(LPARAM)&mnis);
    mnis.wItemFlags = MF_BYPOSITION;
    for (i=0; i<nCount; i++) {
	mnis.wPosition = (WORD)i;
	mnis.wAction = LCA_GET|LCA_ITEMID;
	uiItem = (UINT)LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
	if (uiItem == uiIDItem)
	    return hMenu;
	mnis.wAction = LCA_GET|LCA_FLAGS;
	lFlags = LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mnis);
	if (LOWORD(lFlags) & MF_POPUP) {
	    if ((hMenuRet = TWIN_FindMenuItem
			((HMENU)uiItem,uiIDItem)))
		return hMenuRet;
	}
    }
    return (HMENU)0;
}

static HBITMAP
LoadSysMenuBitmap(void)
{
	HBITMAP hSysMenuBitmap,hBitmap,hOldBitmap;
	HDC hDCDest,hDCSrc;
	BITMAP bm;

	if (SystemBitmaps[SB_OBM_SYSMENU])
	    return SystemBitmaps[SB_OBM_SYSMENU];

	hDCDest = CreateCompatibleDC(0);
	hDCSrc = CreateCompatibleDC(0);

	hBitmap = LoadBitmap(0,(LPSTR)OBM_CLOSE);
	GetObject(hBitmap,sizeof(BITMAP),(LPVOID)&bm);
	hOldBitmap = SelectObject(hDCSrc,hBitmap);
	hSysMenuBitmap = CreateCompatibleBitmap(hDCSrc,
				bm.bmWidth/2,bm.bmHeight);
	SelectObject(hDCDest,hSysMenuBitmap);
	BitBlt(hDCDest,0,0,bm.bmWidth/2,bm.bmHeight,
			hDCSrc,bm.bmWidth/2,0,
			SRCCOPY);

	/* The bitmaps will be unselected by GDI */
	DeleteDC(hDCDest);
	DeleteDC(hDCSrc);
	DeleteObject(hBitmap);

	SystemBitmaps[SB_OBM_SYSMENU] = hSysMenuBitmap;
	return hSysMenuBitmap;
}

static BOOL
ModifyMenuEx(HMENU hMenu, UINT uiPosition, UINT uiFlags,
			UINT uiIDNewItem, LPCSTR lpNewItem, UINT uiAction)
{
    MENUITEMSTRUCT mis;
    DWORD dwIndents;
    LONG lFlags = 0;
    WORD wIndex;
    BOOL rc;
    LPSTR lpItem = (LPSTR)lpNewItem;
    HMENU hMenuorig = hMenu;

    if (!hMenu)
	return FALSE;
    memset((char *)&mis, '\0', sizeof(MENUITEMSTRUCT));
    mis.wItemFlags = uiFlags;
    if (uiAction == LBA_MODIFYITEM) {
	mis.wPosition = (WORD)uiPosition;
	mis.wIDNewItem = (WORD)uiPosition;
    }
    else {
	if (uiFlags & MF_BYPOSITION)
	    mis.wPosition = (WORD)uiPosition;
	else 
	    mis.wPosition = (WORD)uiIDNewItem;
	mis.wIDNewItem = (WORD)uiIDNewItem;
    }
    if ((uiAction != LBA_APPENDITEM) && (!((uiAction == LBA_INSERTITEM) &&
		(uiFlags & MF_BYPOSITION)))) {
	mis.wAction = LCA_GET|LCA_FLAGS;
	lFlags = LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mis);
	if (lFlags == (LONG)-1) {
	    if (((uiAction == LBA_INSERTITEM) || (uiAction == LBA_MODIFYITEM))
			 && (!(uiFlags & MF_BYPOSITION))) 
		hMenu = TWIN_FindMenuItem(hMenu,uiPosition);
	    else
		hMenu = TWIN_FindMenuItem(hMenu,uiIDNewItem);
	    if (!hMenu) {
		return FALSE;
	    }
	    mis.wItemFlags &= ~MF_BYPOSITION;
	    mis.wAction = LCA_GET|LCA_FLAGS;
	    lFlags = LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mis);
	    if (lFlags == (LONG)-1) {
	        if (hMenu != hMenuorig)
		  {};//RELEASELBOXINFO((LPLISTBOXINFO)hMenu32); 
		return FALSE;
	    }
	}
    }
    uiFlags |= MF_BYPOSITION;
    if (uiAction == LBA_MODIFYITEM) {
	wIndex = mis.wPosition;
	mis.wPosition = (WORD)-1;
	mis.wAction = LCA_INDENTS;
	dwIndents = (DWORD)LBoxAPI(hMenu,LBA_GETDATA,(LPARAM)&mis); 
	mis.wLeftIndent = LOWORD(dwIndents);
	mis.wRightIndent = HIWORD(dwIndents);
	mis.wIDNewItem = (WORD)uiIDNewItem;
	mis.wPosition = wIndex;
	mis.wAction = LCA_SET|LCA_ALL;
    }
    else if (uiAction == LBA_DELETEITEM) {
	if (lFlags & MF_POPUP) {
	    mis.wItemFlags = MF_BYPOSITION;
	    mis.wAction = LCA_GET | LCA_ITEMID;
	    DestroyMenu(LOWORD(LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mis)));
	}
    }
    if (uiFlags & MF_BITMAP) {
	if (!((DWORD)lpNewItem & OM_MASK)) {
	/* special case -- pre-defined system bitmap (undocumented) */
	    switch (LOWORD((DWORD)lpNewItem)) {
		case SB_OBM_CLOSE_L:
		case SB_OBM_REDUCE:
		case SB_OBM_ZOOM:
		    break;
		case SB_OBM_SYSMENU:
		/* Here we can substitute the bitmap right away */
		    if (!SystemBitmaps[SB_OBM_SYSMENU])
			LoadSysMenuBitmap();
		    lpItem = (LPSTR)(DWORD)SystemBitmaps[SB_OBM_SYSMENU];
		    break;
		case SB_OBM_RESTORE:
		/* This one we leave as special because it uses */
		/* different bitmaps when pressed/unpressed */
		    if (!SystemBitmaps[SB_OBM_RESTORE]) {
			SystemBitmaps[SB_OBM_RESTORE] =
				LoadBitmap(0,(LPSTR)OBM_RESTORE);
			SystemBitmaps[SB_OBM_RESTORED] =
				LoadBitmap(0,(LPSTR)OBM_RESTORED);
		    }
		    break;
		default:
		    break; /* error */
	    }
	}
    }
    mis.wItemFlags = (WORD)uiFlags;
    mis.lpItemData = lpItem;
    rc = (BOOL)LBoxAPI(hMenu,uiAction,(LPARAM)&mis);
    if (hMenu != hMenuorig)
      {};//RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    return rc;
}

/*******************************************************************
 *         InsertMenu    (USER.410)
 */
BOOL WINAPI		/* API */
InsertMenu(HMENU hMenu, UINT uiPosition, UINT uiFlags,
			UINT uiIDNewItem, LPCSTR lpNewItem)
{
    BOOL bResult;
//    HMENU32 hMenu32;
    bResult = ModifyMenuEx(hMenu,
			uiPosition,uiFlags,uiIDNewItem,lpNewItem,
			LBA_INSERTITEM);
    return bResult;
}

/*******************************************************************
 *         AppendMenu    (USER.411)
 */
BOOL WINAPI		/* API */
AppendMenu(HMENU hMenu, UINT uiFlags, UINT uiIDNewItem, LPCSTR lpNewItem)
{
    BOOL bResult;
    bResult = ModifyMenuEx(hMenu,
			(UINT)-1,uiFlags,uiIDNewItem,lpNewItem,
			LBA_APPENDITEM);
    return bResult;
}

/**********************************************************************
 *         RemoveMenu   (USER.412)
 */
BOOL WINAPI		/* API */
RemoveMenu(HMENU hMenu, UINT idItem, UINT uiFlags)
{
    BOOL bResult;
    if (uiFlags & MF_BYPOSITION) {
	bResult = ModifyMenuEx(hMenu,idItem,uiFlags,
			       0,NULL,LBA_REMOVEITEM);
    } else
    {
	bResult = ModifyMenuEx(hMenu,0,uiFlags,
			       idItem,NULL,LBA_REMOVEITEM);
    }
    return bResult;
}

/**********************************************************************
 *         DeleteMenu    (USER.413)
 */
BOOL WINAPI		/* API */
DeleteMenu(HMENU hMenu, UINT idItem, UINT uiFlags)
{
    BOOL bResult;
    if (uiFlags & MF_BYPOSITION) {
	bResult =  ModifyMenuEx(hMenu,idItem,
				uiFlags,0,NULL,LBA_DELETEITEM);
    }
    else {
        bResult =  ModifyMenuEx(hMenu,0,
				uiFlags,idItem,NULL, LBA_DELETEITEM);
    }
    return bResult;
}

/*******************************************************************
 *         ModifyMenu    (USER.414)
 */
BOOL WINAPI 		/* API */
ModifyMenu(HMENU hMenu, UINT uiPosition, UINT uiFlags,
			UINT uiIDNewItem, LPCSTR lpNewItem)
{
    BOOL bResult;
    bResult =  ModifyMenuEx(hMenu,
			uiPosition,uiFlags,uiIDNewItem,lpNewItem,
			LBA_MODIFYITEM);
    return bResult;
}



static BOOL
ChangeMIFlags(HMENU hMenu, UINT uiItem, UINT uiFlags, UINT uiMask)
{
    MENUITEMSTRUCT mis;
    LONG lFlags;
    HMENU hMenuorig = hMenu;

    if (!hMenu)
	return -1;
    memset((char *)&mis, '\0', sizeof(MENUITEMSTRUCT));
    mis.wPosition = (WORD)uiItem;
    mis.wAction = LCA_GET | LCA_FLAGS;
    mis.wItemFlags = uiFlags;
    lFlags = LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mis);
    if (lFlags < 0) { 
	if (uiFlags & MF_BYPOSITION) 
	    return -1;
	hMenu = TWIN_FindMenuItem(hMenu,uiItem);
	if (!hMenu)
	    return -1;
	mis.wAction = LCA_GET|LCA_FLAGS;
	lFlags = LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mis);
	if (lFlags < 0) {
	    if (hMenu != hMenuorig)
	      {};//RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return -1;
	}
    }
    mis.wItemFlags = LOWORD(lFlags) | MF_BYPOSITION;
    if (uiFlags & uiMask)
	mis.wItemFlags |= uiMask;
    else
	mis.wItemFlags &= ~uiMask;
    if (uiMask == MF_DISABLED) 
	if (uiFlags & MF_GRAYED)
	    mis.wItemFlags |= MF_GRAYED;
	else
	    mis.wItemFlags &= ~MF_GRAYED;
    mis.wAction = LCA_SET | LCA_FLAGS;
    if (LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mis) < 0) {
        if (hMenu != hMenuorig)
	    {};//RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	return -1;
    }
    if (hMenu != hMenuorig)
      {};//RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    return (LOWORD(lFlags) & uiMask);
}
	

/*******************************************************************
 *         CheckMenuItem    (USER.154)
 */

BOOL WINAPI		/* API */
CheckMenuItem(HMENU hMenu, UINT uiIDCheckItem, UINT uiCheck)
{
    BOOL rc;

    rc = (BOOL)ChangeMIFlags(hMenu,
			     uiIDCheckItem,uiCheck,MF_CHECKED);
//    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    return rc;
}

/**********************************************************************
 *         EnableMenuItem    (USER.155)
 */
BOOL WINAPI		/* API */
EnableMenuItem(HMENU hMenu, UINT uiIDEnableItem, UINT uiEnable)
{
    LONG retcode;
//    HMENU32 hMenu32;

    retcode = ChangeMIFlags(hMenu,
			    uiIDEnableItem, uiEnable,MF_DISABLED);
//    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    if (retcode < 0)
	return -1;
    return ((retcode)?0:1);
}

/**************************************************************************
 *              HiliteMenuItem   (USER.162)
 */
BOOL WINAPI		/* API */
HiliteMenuItem(HWND hWnd, HMENU hMenu, UINT uiIDHiliteItem, UINT uiHilite)
{
    BOOL bResult;
//    HMENU32 hMenu32;
    bResult =  ((ChangeMIFlags(hMenu,
			       uiIDHiliteItem,uiHilite,MF_HILITE)
		 <0)?
	      FALSE:TRUE);
//    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    return bResult;
}

/*******************************************************************
 *         GetMenuString    (USER.161)
 */
int WINAPI
GetMenuString(HMENU hMenu, UINT uiIDItem, LPSTR lpString,
			int nMaxCount, UINT uiFlags)
{
//    HMENU32 hMenu32;
    HMENU hMenua;
    MENUITEMSTRUCT mis;
    LPSTR lpItemString;

//    if (!(hMenu32 = GetMenuHandle32(hMenu)))
//	return (UINT)-1;
    memset((char *)&mis, '\0', sizeof(MENUITEMSTRUCT));
    mis.wAction = LCA_GET | LCA_CONTENTS;
    mis.wItemFlags = uiFlags;
    mis.wPosition = (WORD)uiIDItem;
    lpItemString = (LPSTR)LBoxAPI(hMenu,LBA_MODIFYITEM,(LPARAM)&mis);
    if (((LONG)lpItemString != (LONG)-1) && HIWORD(lpItemString)) {
	lstrcpyn(lpString,lpItemString,nMaxCount);
//	RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	return lstrlen(lpString);
    }
    if (lpItemString == NULL || !HIWORD(lpItemString)) {
//        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	return 0;
    }
    if (uiFlags & MF_BYPOSITION) {
//        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	return 0;
    }
    hMenua = TWIN_FindMenuItem(hMenu,uiIDItem);
    if (!hMenua) {
//        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
        return 0;
    }
    hMenu = ((LPOBJHEAD)hMenua)->hObj;
    if (hMenua != hMenu)
      {};//RELEASELBOXINFO((LPLISTBOXINFO)hMenu32a);
//    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    return GetMenuString(hMenu,uiIDItem,lpString,nMaxCount,uiFlags);
}
