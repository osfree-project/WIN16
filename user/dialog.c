/*
 * Dialog functions
 *
 * Copyright 1993, 1994 Alexandre Julliard
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
 *
 */

#include "user.h"
#include "dialog.h"

  /* Dialog base units */
WORD xBaseUnit = 0, yBaseUnit = 0;


#pragma code_seg( "INIT_TEXT" );

/***********************************************************************
 *           DIALOG_Init
 *
 * Initialisation of the dialog manager.
 */
BOOL DIALOG_Init()
{
    TEXTMETRIC tm;
    HDC hdc;

	FUNCTION_START    
      /* Calculate the dialog base units */

    if (!(hdc = GetDC( 0 ))) return FALSE;
    GetTextMetrics( hdc, &tm );
    ReleaseDC( 0, hdc );
    xBaseUnit = tm.tmAveCharWidth;
    yBaseUnit = tm.tmHeight;

      /* Dialog units are based on a proportional system font */
      /* so we adjust them a bit for a fixed font. */
    if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH) xBaseUnit = xBaseUnit * 5 / 4;

    TRACE("DIALOG_Init: base units = %d,%d", xBaseUnit, yBaseUnit );

    return TRUE;
}

#pragma code_seg();

/***********************************************************************
 *           DIALOG_GetFirstTabItem
 *
 * Return the first item of the dialog that has the WS_TABSTOP style.
 */
HWND FAR DIALOG_GetFirstTabItem( HWND hwndDlg )
{
    WND *pWnd = WIN_FindWndPtr( hwndDlg );
    for (pWnd = pWnd->child; pWnd; pWnd = pWnd->next)
        if (pWnd->dwStyle & WS_TABSTOP) return pWnd->hwndSelf;
    return 0;
}

#if 0

/***********************************************************************
 *           DIALOG_GetControl
 *
 * Return the class and text of the control pointed to by ptr,
 * and return a pointer to the next control.
 */
static LPCSTR DIALOG_GetControl( LPCSTR ptr, LPCSTR FAR *class, LPCSTR FAR *text )
{
    unsigned char *base = (unsigned char *)ptr;
    LPBYTE p = base;

	FUNCTION_START

    p += 14;  /* size of control header */
    if (*p & 0x80)
    {
        *class = MAKEINTRESOURCE( *p );
        p++;
    }
    else 
    {
	*class = ptr + (WORD)(p - base);
	p += lstrlen(p) + 1;
    }

    if (*p == 0xff)
    {
	  /* Integer id, not documented (?). Only works for SS_ICON controls */
	*text = MAKEINTRESOURCE( p[1] + 256 * p[2] );
	p += 4;
    }
    else
    {
	*text = ptr + (WORD)(p - base);
	p += lstrlen(p) + 2;
    }
	FUNCTION_END
    return ptr + (WORD)(p - base);
}

#endif

/***********************************************************************
 *           DIALOG_GetControl
 *
 * Return the class and text of the control pointed to by ptr,
 * and return a pointer to the next control.
 */
static LPCSTR DIALOG_GetControl( LPCSTR ptr, LPCSTR FAR *class, LPCSTR FAR *text )
{
    unsigned char far *base = (unsigned char far *)ptr;
    unsigned char far *p = base;
    WORD len;

    FUNCTION_START

    TRACE("--- DIALOG_GetControl: element at %Fp ---", ptr);

    p += 14;

    TRACE("After header, offset=%u, byte=0x%02X", (WORD)(p-base), *p);

    /* --- Чтение класса --- */
    if (*p & 0x80)   /* предопределённый класс */
    {
        *class = MAKEINTRESOURCE( *p );
        TRACE("  Predefined class: id=%u\n", *p);
        p++;
    }
    else
    {
        *class = ptr + (WORD)(p - base);
        TRACE("  Class string at offset %u: \"%S\"", (WORD)(p-base), (LPCSTR)p);
        p += lstrlen((LPCSTR)p) + 1;
    }

    /* --- Чтение текста --- */
    TRACE("Text offset=%u, first byte=0x%02X", (WORD)(p-base), *p);

    if (*p == 0xff)   /* идентификатор иконки */
    {
        *text = MAKEINTRESOURCE( p[1] + 256 * p[2] );
        TRACE("  Icon ID: %u", p[1] + 256 * p[2]);
        p += 4;   /* 0xff + 2 байта id */
    }
    else
    {
        *text = ptr + (WORD)(p - base);
        len = lstrlen((LPCSTR)p);
        TRACE("  Text string at offset %u: \"%S\" (len=%u)", (WORD)(p-base), (LPCSTR)p, len);
        p += len + 2;
    }

    TRACE("  class -> %p, text -> %p", *class, *text);
    TRACE("--- Next element offset: %u ---", (WORD)(p - base));

    FUNCTION_END
    return ptr + (WORD)(p - base);
}

/***********************************************************************
 *           DIALOG_ParseTemplate
 *
 * Fill a DLGTEMPLATE structure from the dialog template, and return
 * a pointer to the first control.
 */
static LPCSTR DIALOG_ParseTemplate( LPCSTR template, DLGTEMPLATE FAR * result )
{
    unsigned char far *base = (unsigned char far *)(template);
    unsigned char far * p = base;

	FUNCTION_START
 
    result->header = *(DLGTEMPLATEHEADER far *)p;
    p += 13;

    /* Get the menu name */

    if (*p == 0xff)
    {
        result->menuName = MAKEINTRESOURCE( p[1] + 256 * p[2] );
        p += 3;
    }
    else if (*p)
    {
        result->menuName = template + (WORD)(p - base);
        p += lstrlen(p) + 1;
    }
    else
    {
        result->menuName = 0;
        p++;
    }

    /* Get the class name */

    if (*p) result->className = template + (WORD)(p - base);
    else result->className = DIALOG_CLASS_ATOM;
    p += lstrlen(p) + 1;

    /* Get the window caption */

    result->caption = template + (WORD)(p - base);
    p += lstrlen(p) + 1;

    /* Get the font name */

    if (result->header.style & DS_SETFONT)
    {
	result->pointSize = *(WORD FAR *)p;
        p += sizeof(WORD);
	result->faceName = template + (WORD)(p - base);
        p += lstrlen(p) + 1;
    }

    return template + (WORD)(p - base);
}

#if 0
static LPCSTR DIALOG_ParseTemplate( LPCSTR template, DLGTEMPLATE FAR * result )
{
    unsigned char far *base = (unsigned char far *)(template);
    unsigned char far * p = base;

    FUNCTION_START

    result->header = *(DLGTEMPLATEHEADER far *)p;
    p += 13;

    /* --- Меню --- */
    if (*p == 0xff)
    {
        result->menuName = MAKEINTRESOURCE( p[1] + 256 * p[2] );
        p += 3;
    }
    else if (*p)
    {
        result->menuName = template + (WORD)(p - base);
        p += lstrlen((LPCSTR)p) + 1;
        if ( ((WORD)(p - base)) & 1 )
            p++;
    }
    else
    {
        result->menuName = 0;
        p++;
    }

    /* --- Класс --- */
    if (*p)
    {
        result->className = template + (WORD)(p - base);
        p += lstrlen((LPCSTR)p) + 1;
        if ( ((WORD)(p - base)) & 1 )
            p++;
    }
    else
    {
        result->className = DIALOG_CLASS_ATOM;
        p++;
    }

    /* --- Заголовок --- */
    result->caption = template + (WORD)(p - base);
    p += lstrlen((LPCSTR)p) + 1;
    if ( ((WORD)(p - base)) & 1 )
        p++;

    /* --- Шрифт (если есть) --- */
    if (result->header.style & DS_SETFONT)
    {

        result->pointSize = *(WORD FAR *)p;
        p += sizeof(WORD);

        result->faceName = template + (WORD)(p - base);
        p += lstrlen((LPCSTR)p) + 1;

    }

    return template + (WORD)(p - base);
}

#endif

/***********************************************************************
 *           DIALOG_DisplayTemplate
 */
static void DIALOG_DisplayTemplate( DLGTEMPLATE FAR * result )
{
    TRACE("DIALOG %d, %d, %d, %d", result->header.x, result->header.y,
	    result->header.cx, result->header.cy );
    TRACE(" STYLE %08lx", result->header.style );
    TRACE(" CAPTION '%S'", (result->caption) );

    if (HIWORD(result->className))
    {
        TRACE("CLASS '%S'", result->className);
    } else
        TRACE("CLASS #%04x", LOWORD(result->className) );

    if (HIWORD(result->menuName))
    {
        TRACE(" MENU '%S'", (result->menuName) );
    } else if (LOWORD(result->menuName))
	TRACE(" MENU %04x", LOWORD(result->menuName) );

    if (result->header.style & DS_SETFONT)
	TRACE(" FONT %d,'%S'", result->pointSize, result->faceName );
}


/***********************************************************************
 *           CreateDialog   (USER.89)
 */
HWND WINAPI CreateDialog( HINSTANCE hInst, LPCSTR dlgTemplate,
		   HWND owner, DLGPROC dlgProc )
{
    return CreateDialogParam( hInst, dlgTemplate, owner, dlgProc, 0 );
}


/***********************************************************************
 *           CreateDialogParam   (USER.241)
 */
HWND WINAPI CreateDialogParam( HINSTANCE hInst, LPCSTR dlgTemplate,
		        HWND owner, DLGPROC dlgProc, LPARAM param )
{
	HWND hwnd;
	HRSRC hRsrc;
	HGLOBAL hmem;
	LPBYTE data;

	TRACE("CreateDialogParam: %04x,%08lx,%04x,%08lx,%ld\n",
                   hInst, (DWORD)dlgTemplate, owner, (DWORD)dlgProc, param );

	hwnd=0;

	if ((hRsrc = FindResource( hInst, dlgTemplate, RT_DIALOG )))
	{
    		if ((hmem = LoadResource( hInst, hRsrc )))
		{
			if ((data = LockResource( hmem )))
			{
				hwnd = CreateDialogIndirectParam(hInst, data, owner, dlgProc, param);
				UnlockResource( hmem );
			}
			FreeResource( hmem );
		}
	}
	FUNCTION_END
	return hwnd;
}


/***********************************************************************
 *           CreateDialogIndirect   (USER.219)
 */
HWND WINAPI CreateDialogIndirect( HINSTANCE hInst, const void FAR * dlgTemplate,
			   HWND owner, DLGPROC dlgProc )
{
    return CreateDialogIndirectParam( hInst, dlgTemplate, owner, dlgProc, 0 );
}


/***********************************************************************
 *           CreateDialogIndirectParam   (USER.242)
 */
HWND WINAPI CreateDialogIndirectParam( HINSTANCE hInst, const void FAR * dlgTemplate,
			        HWND owner, DLGPROC dlgProc, LPARAM param )
{
    HMENU hMenu = 0;
    HFONT hFont = 0;
    HWND hwnd, hwndCtrl;
    RECT rect;
    WND * wndPtr;
    int i;
    DLGTEMPLATE template;
    LPCSTR headerPtr;
    DIALOGINFO FAR * dlgInfo;
    DWORD exStyle = 0;
    WORD xUnit = xBaseUnit;
    WORD yUnit = yBaseUnit;

	FUNCTION_START
      /* Parse dialog template */

    if (!dlgTemplate) return 0;
    headerPtr = DIALOG_ParseTemplate( dlgTemplate, &template );
    
    DIALOG_DisplayTemplate( &template );

      /* Load menu */

    if (template.menuName) hMenu = LoadMenu( hInst, template.menuName );

      /* Create custom font if needed */

    if (template.header.style & DS_SETFONT)
    {
          /* The font height must be negative as it is a point size */
          /* (see CreateFont() documentation in the Windows SDK).   */
	hFont = CreateFont( -template.pointSize, 0, 0, 0, FW_DONTCARE,
			    FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
			    DEFAULT_QUALITY, FF_DONTCARE,
                            template.faceName );
	if (hFont)
	{
	    TEXTMETRIC tm;
	    HFONT oldFont;
	    HDC hdc;

	    hdc = GetDC(0);
	    oldFont = SelectObject( hdc, hFont );
	    GetTextMetrics( hdc, &tm );
	    SelectObject( hdc, oldFont );
	    ReleaseDC( 0, hdc );
	    xUnit = tm.tmAveCharWidth;
	    yUnit = tm.tmHeight;
            if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH)
                xBaseUnit = xBaseUnit * 5 / 4;  /* See DIALOG_Init() */
	}
    }
    
      /* Create dialog main window */

    rect.left = rect.top = 0;
    rect.right = template.header.cx * xUnit / 4;
    rect.bottom = template.header.cy * yUnit / 8;
    if (template.header.style & DS_MODALFRAME) exStyle |= WS_EX_DLGMODALFRAME;
    AdjustWindowRectEx( &rect, template.header.style, 
			hMenu ? TRUE : FALSE , exStyle );
    rect.right -= rect.left;
    rect.bottom -= rect.top;

    if ((int)template.header.x == CW_USEDEFAULT)
        rect.left = rect.top = CW_USEDEFAULT;
    else
    {
        rect.left += template.header.x * xUnit / 4;
        rect.top += template.header.y * yUnit / 8;
        if (!(template.header.style & DS_ABSALIGN))
            ClientToScreen( owner, (POINT *)&rect );
    }

    hwnd = CreateWindowEx( exStyle, template.className, template.caption, 
			   template.header.style & ~WS_VISIBLE,
			   rect.left, rect.top, rect.right, rect.bottom,
			   owner, hMenu, hInst, 0 );
    if (!hwnd)
    {
	if (hFont) DeleteObject( hFont );
	if (hMenu) DestroyMenu( hMenu );
	return 0;
    }
    wndPtr = WIN_FindWndPtr( hwnd );

      /* Purge junk from system menu */
      /* FIXME: this doesn't belong here */

    DeleteMenu(wndPtr->hSysMenu,SC_SIZE,MF_BYCOMMAND);
    if (!(wndPtr->dwStyle & WS_MAXIMIZEBOX) )
    {
        DeleteMenu(wndPtr->hSysMenu,SC_MAXIMIZE,MF_BYCOMMAND);
        if( !(wndPtr->dwStyle & WS_MINIMIZEBOX) )
        {
            DeleteMenu(wndPtr->hSysMenu,SC_MINIMIZE,MF_BYCOMMAND);
            DeleteMenu(wndPtr->hSysMenu,SC_RESTORE,MF_BYCOMMAND);
        }
    }
    else
        if (!(wndPtr->dwStyle & WS_MINIMIZEBOX) )
            DeleteMenu(wndPtr->hSysMenu,SC_MINIMIZE,MF_BYCOMMAND);

      /* Create control windows */

    TRACE(" BEGIN\n" );

    dlgInfo = (DIALOGINFO FAR *)wndPtr->wExtra;
    dlgInfo->msgResult = 0;  /* This is used to store the default button id */
    dlgInfo->hDialogHeap = 0;

    for (i = 0; i < template.header.nbItems; i++)
    {
	DLGCONTROLHEADER FAR *header;
	LPCSTR className, winName;
        HWND hwndDefButton = 0;
        char buffer[10];

        header = (DLGCONTROLHEADER FAR *)headerPtr;
	headerPtr = DIALOG_GetControl( headerPtr, &className, &winName );

        if (!HIWORD(className))
        {
            switch(LOWORD(className))
            {
            case 0x80: lstrcpy( buffer, "BUTTON" ); break;
            case 0x81: lstrcpy( buffer, "EDIT" ); break;
            case 0x82: lstrcpy( buffer, "STATIC" ); break;
            case 0x83: lstrcpy( buffer, "LISTBOX" ); break;
            case 0x84: lstrcpy( buffer, "SCROLLBAR" ); break;
            case 0x85: lstrcpy( buffer, "COMBOBOX" ); break;
            default:   buffer[0] = '\0'; break;
            }
            className = buffer;
        }

        if (HIWORD(className))
	{
            TRACE("   '%S' ", className);
	}
        else TRACE("   %04x ", LOWORD(className) );

	if (HIWORD(winName))
	{
            TRACE("'%S'", winName);
	} else TRACE("%04x", LOWORD(winName) );

	TRACE(" %d, %d, %d, %d, %d, %08lx\n", 
                       header->id, header->x, header->y, 
                       header->cx, header->cy, header->style );

	if (HIWORD(className) &&
            !lstrcmp( className, "EDIT") &&
            ((header->style & DS_LOCALEDIT) != DS_LOCALEDIT))
        {
	    if (!dlgInfo->hDialogHeap)
            {
		dlgInfo->hDialogHeap = GlobalAlloc(GMEM_FIXED, 0x10000);
		if (!dlgInfo->hDialogHeap)
                {
		    TRACE("CreateDialogIndirectParam: Insufficient memory to create heap for edit control");
		    continue;
		}
		// @todo rework to moveable?
		LocalInit(dlgInfo->hDialogHeap, 0, 0xffff);
	    }
	    hwndCtrl = CreateWindowEx(WS_EX_NOPARENTNOTIFY, className, winName,
                                      header->style | WS_CHILD,
                                      header->x * xUnit / 4,
                                      header->y * yUnit / 8,
                                      header->cx * xUnit / 4,
                                      header->cy * yUnit / 8,
                                      hwnd, (HMENU)header->id,
                                      dlgInfo->hDialogHeap, 0 );
	}
	else
        {
	    hwndCtrl = CreateWindowEx(WS_EX_NOPARENTNOTIFY, className, winName,
                                      header->style | WS_CHILD,
                                      header->x * xUnit / 4,
                                      header->y * yUnit / 8,
                                      header->cx * xUnit / 4,
                                      header->cy * yUnit / 8,
                                      hwnd, (HMENU)header->id,
                                      hInst, 0 );
	}

        /* Make the control last one in Z-order, so that controls remain
           in the order in which they were created */
	SetWindowPos( hwndCtrl, HWND_BOTTOM, 0, 0, 0, 0,
                      SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );

            /* Send initialisation messages to the control */
        if (hFont) SendMessage( hwndCtrl, WM_SETFONT, (WPARAM)hFont, 0 );
        if (SendMessage( hwndCtrl, WM_GETDLGCODE, 0, 0 ) & DLGC_DEFPUSHBUTTON)
        {
              /* If there's already a default push-button, set it back */
              /* to normal and use this one instead. */
            if (hwndDefButton)
                SendMessage( hwndDefButton, BM_SETSTYLE, BS_PUSHBUTTON, FALSE);
            hwndDefButton = hwndCtrl;
            dlgInfo->msgResult = GetWindowWord( hwndCtrl, GWW_ID );
        }
    }    

    TRACE(" END" );
    
      /* Initialise dialog extra data */

    dlgInfo->dlgProc   = (WNDPROC)dlgProc;
    dlgInfo->hUserFont = hFont;
    dlgInfo->hMenu     = hMenu;
    dlgInfo->xBaseUnit = xUnit;
    dlgInfo->yBaseUnit = yUnit;
    dlgInfo->hwndFocus = DIALOG_GetFirstTabItem( hwnd );

      /* Send initialisation messages and set focus */

    if (dlgInfo->hUserFont)
	SendMessage( hwnd, WM_SETFONT, (WPARAM)dlgInfo->hUserFont, 0 );
    if (SendMessage( hwnd, WM_INITDIALOG, (WPARAM)dlgInfo->hwndFocus, param ))
	SetFocus( dlgInfo->hwndFocus );
    if (template.header.style & WS_VISIBLE) ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}


/***********************************************************************
 *           DIALOG_DoDialogBox
 */
int DIALOG_DoDialogBox( HWND hwnd, HWND owner )
{
    WND * wndPtr;
    DIALOGINFO * dlgInfo;
    HANDLE msgHandle;
    MSG* lpmsg;
    int retval;

      /* Owner must be a top-level window */
    owner = WIN_GetTopParent( owner );
    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return -1;
    if (!(msgHandle = LocalAlloc (LMEM_FIXED, sizeof(MSG) ))) return -1;
    lpmsg = (MSG *) LocalLock( msgHandle );
    dlgInfo = (DIALOGINFO *)wndPtr->wExtra;
    EnableWindow( owner, FALSE );
    ShowWindow( hwnd, SW_SHOW );

    while (MSG_InternalGetMessage( (MSG FAR *)LocalLock(msgHandle), hwnd, owner,
                                   MSGF_DIALOGBOX, PM_REMOVE,
                                   !(wndPtr->dwStyle & DS_NOIDLEMSG) ))
    {
	if (!IsDialogMessage( hwnd, lpmsg))
	{
	    TranslateMessage( lpmsg );
	    DispatchMessage( lpmsg );
	}
	if (dlgInfo->fEnd) break;
    }
    retval = dlgInfo->msgResult;
    DestroyWindow( hwnd );
    LocalFree( msgHandle );
    EnableWindow( owner, TRUE );
    return retval;
}


/***********************************************************************
 *           DialogBox   (USER.87)
 */
int WINAPI DialogBox( HINSTANCE hInst, LPCSTR dlgTemplate, HWND owner, DLGPROC dlgProc )
{
    return DialogBoxParam( hInst, dlgTemplate, owner, dlgProc, 0 );
}


/***********************************************************************
 *           DialogBoxParam   (USER.239)
 */
int WINAPI DialogBoxParam( HINSTANCE hInst, LPCSTR dlgTemplate,
		    HWND owner, DLGPROC dlgProc, LPARAM param )
{
	HWND hWnd;
	int retVal=-1;

	FUNCTION_START    
	TRACE("DialogBoxParam: %04x,%08lx,%04x,%08lx,%ld", hInst, (DWORD)dlgTemplate, owner, (DWORD)dlgProc, param );

	hWnd = CreateDialogParam( hInst, dlgTemplate, owner, dlgProc, param );
	if (hWnd) retVal=DIALOG_DoDialogBox(hWnd, owner);

	FUNCTION_END

	return retVal;
}


/***********************************************************************
 *           DialogBoxIndirect   (USER.218)
 */
int WINAPI DialogBoxIndirect( HINSTANCE hInst, HANDLE dlgTemplate,
		       HWND owner, DLGPROC dlgProc )
{
    return DialogBoxIndirectParam( hInst, dlgTemplate, owner, dlgProc, 0 );
}


/***********************************************************************
 *           DialogBoxIndirectParam   (USER.240)
 */
int WINAPI DialogBoxIndirectParam( HINSTANCE hInst, HANDLE dlgTemplate,
			    HWND owner, DLGPROC dlgProc, LPARAM param )
{
    HWND hwnd;
    LPBYTE ptr;

    if (!(ptr = (LPBYTE)GlobalLock( dlgTemplate ))) return -1;
    hwnd = CreateDialogIndirectParam( hInst, ptr, owner, dlgProc, param );
    GlobalUnlock( dlgTemplate );
    if (hwnd) return DIALOG_DoDialogBox( hwnd, owner );
    return -1;
}


/***********************************************************************
 *           EndDialog   (USER.88)
 */
VOID WINAPI EndDialog( HWND hwnd, int retval )
{
    WND * wndPtr = WIN_FindWndPtr( hwnd );
    DIALOGINFO * dlgInfo = (DIALOGINFO *)wndPtr->wExtra;
    dlgInfo->msgResult = retval;
    dlgInfo->fEnd = TRUE;
    TRACE("EndDialog: %04x %d\n", hwnd, retval );
    return ;
}


/***********************************************************************
 *           IsDialogMessage   (USER.90)
 */
BOOL WINAPI IsDialogMessage( HWND hwndDlg, LPMSG msg )
{
    WND * wndPtr;
    int dlgCode;

    if (!(wndPtr = WIN_FindWndPtr( hwndDlg ))) return FALSE;
    if ((hwndDlg != msg->hwnd) && !IsChild( hwndDlg, msg->hwnd )) return FALSE;

      /* Only the key messages get special processing */
    if ((msg->message != WM_KEYDOWN) &&
        (msg->message != WM_SYSCHAR) &&
	(msg->message != WM_CHAR))
        return FALSE;

    dlgCode = SendMessage( msg->hwnd, WM_GETDLGCODE, 0, 0 );
    if (dlgCode & DLGC_WANTMESSAGE)
    {
        DispatchMessage( msg );
        return TRUE;
    }

    switch(msg->message)
    {
    case WM_KEYDOWN:
        if (dlgCode & DLGC_WANTALLKEYS) break;
        switch(msg->wParam)
        {
        case VK_TAB:
            if (!(dlgCode & DLGC_WANTTAB))
            {
                SendMessage( hwndDlg, WM_NEXTDLGCTL,
                             (GetKeyState(VK_SHIFT) & 0x80), 0 );
                return TRUE;
            }
            break;
            
        case VK_RIGHT:
        case VK_DOWN:
            if (!(dlgCode & DLGC_WANTARROWS))
            {
                SetFocus(GetNextDlgGroupItem(hwndDlg,GetFocus(),FALSE));
                return TRUE;
            }
            break;

        case VK_LEFT:
        case VK_UP:
            if (!(dlgCode & DLGC_WANTARROWS))
            {
                SetFocus(GetNextDlgGroupItem(hwndDlg,GetFocus(),TRUE));
                return TRUE;
            }
            break;

        case VK_ESCAPE:
            SendMessage( hwndDlg, WM_COMMAND, IDCANCEL,
                         MAKELPARAM( GetDlgItem(hwndDlg,IDCANCEL), 0 ));
            break;

        case VK_RETURN:
            {
                DWORD dw = SendMessage( hwndDlg, DM_GETDEFID, 0, 0 );
                if (HIWORD(dw) == DC_HASDEFID)
                    SendMessage( hwndDlg, WM_COMMAND, LOWORD(dw),
                                 MAKELPARAM( GetDlgItem( hwndDlg, LOWORD(dw) ),
                                             BN_CLICKED ));
                else
                    SendMessage( hwndDlg, WM_COMMAND, IDOK,
                                 MAKELPARAM( GetDlgItem(hwndDlg,IDOK), 0 ));
            }
            break;

	default: 
	    TranslateMessage( msg );
        }
        break;  /* case WM_KEYDOWN */

        
    case WM_CHAR:
        if (dlgCode & (DLGC_WANTALLKEYS | DLGC_WANTCHARS)) break;
        break;

    case WM_SYSCHAR:
        if (dlgCode & DLGC_WANTALLKEYS) break;
        break;
    }

      /* If we get here, the message has not been treated specially */
      /* and can be sent to its destination window. */
    DispatchMessage( msg );
    return TRUE;
}


/****************************************************************
 *         GetDlgCtrlID           (USER.277)
 */
int WINAPI GetDlgCtrlID( HWND hwnd )
{
    WND *wndPtr = WIN_FindWndPtr(hwnd);
    if (wndPtr) return wndPtr->wIDmenu;
    else return 0;
}
 

/***********************************************************************
 *           GetDlgItem   (USER.91)
 */
HWND WINAPI GetDlgItem( HWND hwndDlg, int id )
{
    WND *pWnd;

    if (!(pWnd = WIN_FindWndPtr( hwndDlg ))) return 0;
    for (pWnd = pWnd->child; pWnd; pWnd = pWnd->next)
        if (pWnd->wIDmenu == id) return pWnd->hwndSelf;
    return 0;
}


/*******************************************************************
 *           SendDlgItemMessage   (USER.101)
 */
LRESULT WINAPI SendDlgItemMessage(HWND hwnd, int id, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndCtrl = GetDlgItem( hwnd, id );
    if (hwndCtrl) return SendMessage( hwndCtrl, msg, wParam, lParam );
    else return 0;
}


/*******************************************************************
 *           SetDlgItemText   (USER.92)
 */
VOID WINAPI SetDlgItemText( HWND hwnd, int id, LPCSTR lpString )
{
    SendDlgItemMessage( hwnd, id, WM_SETTEXT, 0, (DWORD)lpString );
}


/***********************************************************************
 *           GetDlgItemText   (USER.93)
 */
int WINAPI GetDlgItemText( HWND hwnd, int id, LPSTR str, int max )
{
    return (int)SendDlgItemMessage( hwnd, id, WM_GETTEXT, max, (DWORD)str );
}


/*******************************************************************
 *           SetDlgItemInt   (USER.94)
 */
VOID WINAPI SetDlgItemInt( HWND hwnd, int id, UINT value, BOOL fSigned )
{
    char str[20];

	if (fSigned)
		lstrcpy(str, itoa((int)value));
	else
		lstrcpy(str, uitoa((unsigned int)value));

//    if (fSigned) sprintf( str, "%d", (int)value );
//    else sprintf( str, "%u", value );
    SendDlgItemMessage( hwnd, id, WM_SETTEXT, 0, (LPARAM)str );
}

#if 0
static long WINAPI strtol(LPCSTR str, LPSTR FAR *endptr)
{
    long result = 0;
    int sign = 1;
    LPCSTR p = str;

    /* Skip leading spaces and tabs */
    while (*p == ' ' || *p == '\t')
        p++;

    /* Check for sign */
    if (*p == '-')
    {
        sign = -1;
        p++;
    }
    else if (*p == '+')
    {
        p++;
    }

    /* If no digit follows, conversion fails */
    if (!(*p >= '0' && *p <= '9'))
    {
        if (endptr)
            *endptr = (LPSTR)str;   /* point to original string, no conversion */
        return 0;
    }

    /* Convert digits */
    while (*p >= '0' && *p <= '9')
    {
        result = result * 10 + (*p - '0');
        p++;
    }

    /* Set endptr to first character after number */
    if (endptr)
        *endptr = (LPSTR)p;

    return sign * result;
}

/***********************************************************************
 *           GetDlgItemInt   (USER.95)
 */
UINT WINAPI GetDlgItemInt( HWND hwnd, int id, BOOL FAR * translated, BOOL fSigned )
{
    char str[30];
    long result = 0;
    
    if (translated) *translated = FALSE;
    if (SendDlgItemMessage( hwnd, id, WM_GETTEXT, 30, (LPARAM)str ))
    {
	char FAR * endptr;
	result = strtol(str, &endptr);
	if (endptr && (endptr != str))  /* Conversion was successful */
	{
	    if (fSigned)
	    {
		if ((result < -32767) || (result > 32767)) result = 0;
		else if (translated) *translated = TRUE;
	    }
	    else
	    {
		if ((result < 0) || (result > 65535)) result = 0;
		else if (translated) *translated = TRUE;
	    }
	}
    }
    return (WORD)result;
}

#endif
/***********************************************************************
 *           GetDlgItemInt   (USER.95)
 */
UINT WINAPI GetDlgItemInt( HWND hwnd, int id, BOOL FAR * translated, BOOL fSigned )
{
    char str[30];
    long result = 0;
    LPSTR p;
    int sign = 1;
    unsigned int accum = 0;
    unsigned int max_val;
    int overflow = 0;
    
    if (translated) *translated = FALSE;
    if (!SendDlgItemMessage( hwnd, id, WM_GETTEXT, 30, (LPARAM)str ))
        return 0;
    
    p = str;
    /* Skip leading spaces and tabs */
    while (*p == ' ' || *p == '\t')
        p++;
    
    /* Handle sign */
    if (*p == '-')
    {
        if (!fSigned)
            return 0;               /* negative not allowed for unsigned */
        sign = -1;
        p++;
    }
    else if (*p == '+')
    {
        p++;
    }
    
    /* Must have at least one digit */
    if (!(*p >= '0' && *p <= '9'))
        return 0;
    
    /* Set maximum absolute value */
    max_val = fSigned ? 32767U : 65535U;
    
    /* Convert digits with overflow detection */
    while (*p >= '0' && *p <= '9')
    {
        unsigned int digit = *p - '0';
        
        if (accum > max_val / 10)
        {
            overflow = 1;   /* would overflow on multiplication */
        }
        else
        {
            accum = accum * 10;
            if (accum > max_val - digit)
            {
                overflow = 1;   /* would overflow on addition */
                /* Continue scanning digits to update p */
            }
            else
            {
                accum = accum + digit;
            }
        }
        p++;
    }
    
    if (overflow)
        return 0;           /* number out of range */
    
    result = (long)(sign * (int)accum);
    
    /* Final range checks (already enforced, but keep for clarity) */
    if (fSigned)
    {
        if ((result < -32767) || (result > 32767))
            result = 0;
        else if (translated)
            *translated = TRUE;
    }
    else
    {
        if ((result < 0) || (result > 65535))
            result = 0;
        else if (translated)
            *translated = TRUE;
    }
    
    return (WORD)result;
}

/***********************************************************************
 *           CheckDlgButton   (USER.97)
 */
BOOL WINAPI CheckDlgButtonReal( HWND hwnd, int id, UINT check );
BOOL WINAPI CheckDlgButtonReal( HWND hwnd, int id, UINT check )
{
    SendDlgItemMessage( hwnd, id, BM_SETCHECK, check, 0 );
    return TRUE;
}


/***********************************************************************
 *           IsDlgButtonChecked   (USER.98)
 */
UINT WINAPI IsDlgButtonChecked( HWND hwnd, int id )
{
    return (UINT)SendDlgItemMessage( hwnd, id, BM_GETCHECK, 0, 0 );
}


/***********************************************************************
 *           CheckRadioButton   (USER.96)
 */
BOOL WINAPI CheckRadioButtonReal( HWND hwndDlg, int firstID, int lastID, int checkID );
BOOL WINAPI CheckRadioButtonReal( HWND hwndDlg, int firstID, int lastID, int checkID )
{
    WND *pWnd = WIN_FindWndPtr( hwndDlg );
    if (!pWnd) return FALSE;

    for (pWnd = pWnd->child; pWnd; pWnd = pWnd->next)
        if ((pWnd->wIDmenu == firstID) || (pWnd->wIDmenu == lastID)) break;
    if (!pWnd) return FALSE;

    if (pWnd->wIDmenu == lastID)
        lastID = firstID;  /* Buttons are in reverse order */
    while (pWnd)
    {
	SendMessage(pWnd->hwndSelf, BM_SETCHECK, (pWnd->wIDmenu == checkID),0);
        if (pWnd->wIDmenu == lastID) break;
	pWnd = pWnd->next;
    }
    return TRUE;
}


/***********************************************************************
 *           GetDialogBaseUnits   (USER.243)
 */
DWORD WINAPI GetDialogBaseUnits()
{
    return MAKELONG( xBaseUnit, yBaseUnit );
}


/***********************************************************************
 *           MapDialogRect   (USER.103)
 */
void WINAPI MapDialogRect( HWND hwnd, LPRECT rect )
{
    DIALOGINFO * dlgInfo;
    WND * wndPtr = WIN_FindWndPtr( hwnd );
    if (!wndPtr) return;
    dlgInfo = (DIALOGINFO *)wndPtr->wExtra;
    rect->left   = (rect->left * dlgInfo->xBaseUnit) / 4;
    rect->right  = (rect->right * dlgInfo->xBaseUnit) / 4;
    rect->top    = (rect->top * dlgInfo->yBaseUnit) / 8;
    rect->bottom = (rect->bottom * dlgInfo->yBaseUnit) / 8;
}


/***********************************************************************
 *           GetNextDlgGroupItem   (USER.227)
 */
HWND WINAPI GetNextDlgGroupItem( HWND hwndDlg, HWND hwndCtrl, BOOL fPrevious )
{
    WND *pWnd, *pWndStart, *pWndCtrl, *pWndDlg;

    if (!(pWndDlg = WIN_FindWndPtr( hwndDlg ))) return 0;
    if (!(pWndCtrl = WIN_FindWndPtr( hwndCtrl ))) return 0;
    if (pWndCtrl->parent != pWndDlg) return 0;

    if (!fPrevious && pWndCtrl->next)  /* Check if next control is in group */
    {
        if (!(pWndCtrl->next->dwStyle & WS_GROUP))
            return pWndCtrl->next->hwndSelf;
    }

      /* Now we will have to find the start of the group */

    for (pWnd = pWndStart = pWndDlg->child; pWnd; pWnd = pWnd->next)
    {
        if (pWnd->dwStyle & WS_GROUP) pWndStart = pWnd;  /* Start of a group */
	if (pWnd == pWndCtrl) break;
    }

    if (!pWnd) TRACE("GetNextDlgGroupItem: hwnd not in dialog!");

      /* only case left for forward search: wraparound */
    if (!fPrevious) return pWndStart->hwndSelf;

    pWnd = pWndStart->next;
    while (pWnd && (pWnd != pWndCtrl))
    {
        if (pWnd->dwStyle & WS_GROUP) break;
        pWndStart = pWnd;
        pWnd = pWnd->next;
    }
    return pWndStart->hwndSelf;
}


/***********************************************************************
 *           GetNextDlgTabItem   (USER.228)
 */
HWND WINAPI GetNextDlgTabItem( HWND hwndDlg, HWND hwndCtrl, BOOL fPrevious )
{
    WND *pWnd, *pWndLast, *pWndCtrl, *pWndDlg;

    if (!(pWndDlg = WIN_FindWndPtr( hwndDlg ))) return 0;
    if (!(pWndCtrl = WIN_FindWndPtr( hwndCtrl ))) return 0;
    if (pWndCtrl->parent != pWndDlg) return 0;

    pWndLast = pWndCtrl;
    pWnd = pWndCtrl->next;
    while (1)
    {
        if (!pWnd) pWnd = pWndDlg->child;
        if (pWnd == pWndCtrl) break;
	if ((pWnd->dwStyle & WS_TABSTOP) && (pWnd->dwStyle & WS_VISIBLE))
	{
            pWndLast = pWnd;
	    if (!fPrevious) break;
	}
        pWnd = pWnd->next;
    }
    return pWndLast->hwndSelf;
}
