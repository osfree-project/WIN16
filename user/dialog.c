/*
 * 16-bit dialog functions
 *
 * Copyright 1993, 1994, 1996, 2003 Alexandre Julliard
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

#include <stdarg.h>

#include <windows.h>
//#include "winbase.h"
//#include "wownt32.h"
//#include "wine/winuser16.h"
//#include "user_private.h"
//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(dialog);

#define GET_WORD(ptr)  (*(const WORD *)(ptr))
#define GET_DWORD(ptr) (*(const DWORD *)(ptr))

/* Dialog control information */
typedef struct
{
    DWORD      style;
    int      x;
    int      y;
    int      cx;
    int      cy;
    UINT       id;
    LPCSTR     className;
    LPCSTR     windowName;
    const void far * data;
} DLG_CONTROL_INFO;

  /* Dialog template */
typedef struct
{
    DWORD      style;
    UINT     nbItems;
    int      x;
    int      y;
    int      cx;
    int      cy;
    LPCSTR     menuName;
    LPCSTR     className;
    LPCSTR     caption;
    int      pointSize;
    LPCSTR     faceName;
} DLG_TEMPLATE;

#define DIALOG_CLASS_ATOM MAKEINTATOM(32770)

/***********************************************************************
 *           DIALOG_EnableOwner
 *
 * Helper function for modal dialogs to enable again the
 * owner of the dialog box.
 */
#if 0
static void DIALOG_EnableOwner( HWND hOwner )
{
    /* Owner must be a top-level window */
    if (hOwner)
        hOwner = GetAncestor( hOwner, GA_ROOT );
    if (!hOwner) return;
    EnableWindow( hOwner, TRUE );
}


/***********************************************************************
 *           DIALOG_DisableOwner
 *
 * Helper function for modal dialogs to disable the
 * owner of the dialog box. Returns TRUE if owner was enabled.
 */
static BOOL DIALOG_DisableOwner( HWND hOwner )
{
    /* Owner must be a top-level window */
    if (hOwner)
        hOwner = GetAncestor( hOwner, GA_ROOT );
    if (!hOwner) return FALSE;
    if (IsWindowEnabled( hOwner ))
    {
        EnableWindow( hOwner, FALSE );
        return TRUE;
    }
    else
        return FALSE;
}
#endif

/***********************************************************************
 *           DIALOG_GetControl16
 *
 * Return the class and text of the control pointed to by ptr,
 * fill the header structure and return a pointer to the next control.
 */
static LPCSTR DIALOG_GetControl16( LPCSTR p, DLG_CONTROL_INFO *info )
{
    static char buffer[10];
    int int_id;

    info->x       = GET_WORD(p);  p += sizeof(WORD);
    info->y       = GET_WORD(p);  p += sizeof(WORD);
    info->cx      = GET_WORD(p);  p += sizeof(WORD);
    info->cy      = GET_WORD(p);  p += sizeof(WORD);
    info->id      = GET_WORD(p);  p += sizeof(WORD);
    info->style   = GET_DWORD(p); p += sizeof(DWORD);

    if (*p & 0x80)
    {
        switch((BYTE)*p)
        {
            case 0x80: lstrcpy( buffer, "BUTTON" ); break;
            case 0x81: lstrcpy( buffer, "EDIT" ); break;
            case 0x82: lstrcpy( buffer, "STATIC" ); break;
            case 0x83: lstrcpy( buffer, "LISTBOX" ); break;
            case 0x84: lstrcpy( buffer, "SCROLLBAR" ); break;
            case 0x85: lstrcpy( buffer, "COMBOBOX" ); break;
            default:   buffer[0] = '\0'; break;
        }
        info->className = buffer;
        p++;
    }
    else
    {
        info->className = p;
        p += lstrlen(p) + 1;
    }

    int_id = ((BYTE)*p == 0xff);
    if (int_id)
    {
        /* Integer id, not documented (?). Only works for SS_ICON controls */
        info->windowName = MAKEINTRESOURCE(GET_WORD(p+1));
        p += 3;
    }
    else
    {
        info->windowName = p;
        p += lstrlen(p) + 1;
    }

    if (*p) info->data = p + 1;
    else info->data = NULL;

    p += *p + 1;

//    TRACE("   %s %s %d, %d, %d, %d, %d, %08lx, %p\n",
//          debugstr_a(info->className),  debugstr_a(info->windowName),
//          info->id, info->x, info->y, info->cx, info->cy,
//          info->style, info->data );

    return p;
}

#if 0
// @todo Doesn't know nothing about get_dialog_info yet...

/***********************************************************************
 *           DIALOG_CreateControls16
 *
 * Create the control windows for a dialog.
 */
static BOOL DIALOG_CreateControls16( HWND hwnd, LPCSTR template,
                                     const DLG_TEMPLATE *dlgTemplate, HINSTANCE hInst )
{
    DIALOGINFO *dlgInfo = wow_handlers32.get_dialog_info( hwnd, TRUE );
    DLG_CONTROL_INFO info;
    HWND hwndCtrl, hwndDefButton = 0;
    INT items = dlgTemplate->nbItems;

    TRACE(" BEGIN\n" );
    while (items--)
    {
        char *caption, caption_buf[4];
        HINSTANCE16 instance = hInst;
        SEGPTR segptr;

        template = DIALOG_GetControl16( template, &info );
        segptr = MapLS( (void *)info.data );

        caption = (char *)info.windowName;
        if (caption && IS_INTRESOURCE(caption))
        {
            caption_buf[0] = 0xff;
            caption_buf[1] = PtrToUlong( caption );
            caption_buf[2] = PtrToUlong( caption ) >> 8;
            caption_buf[3] = 0;
            caption = caption_buf;
        }

        hwndCtrl = WIN_Handle32( CreateWindowEx16( WS_EX_NOPARENTNOTIFY,
                                                   info.className, caption,
                                                   info.style | WS_CHILD,
                                                   MulDiv(info.x, dlgInfo->xBaseUnit, 4),
                                                   MulDiv(info.y, dlgInfo->yBaseUnit, 8),
                                                   MulDiv(info.cx, dlgInfo->xBaseUnit, 4),
                                                   MulDiv(info.cy, dlgInfo->yBaseUnit, 8),
                                                   HWND_16(hwnd), (HMENU16)info.id,
                                                   instance, (LPVOID)segptr ));
        UnMapLS( segptr );

        if (!hwndCtrl)
        {
            if (dlgTemplate->style & DS_NOFAILCREATE) continue;
            return FALSE;
        }

            /* Send initialisation messages to the control */
        if (dlgInfo->hUserFont) SendMessageA( hwndCtrl, WM_SETFONT,
                                             (WPARAM)dlgInfo->hUserFont, 0 );
        if (SendMessageA(hwndCtrl, WM_GETDLGCODE, 0, 0) & DLGC_DEFPUSHBUTTON)
        {
              /* If there's already a default push-button, set it back */
              /* to normal and use this one instead. */
            if (hwndDefButton)
                SendMessageA( hwndDefButton, BM_SETSTYLE,
                                BS_PUSHBUTTON,FALSE );
            hwndDefButton = hwndCtrl;
            dlgInfo->idResult = GetWindowLongPtrA( hwndCtrl, GWLP_ID );
        }
    }
    TRACE(" END\n" );
    return TRUE;
}

#endif

/***********************************************************************
 *           DIALOG_ParseTemplate16
 *
 * Fill a DLG_TEMPLATE structure from the dialog template, and return
 * a pointer to the first control.
 */
static LPCSTR DIALOG_ParseTemplate16( LPCSTR p, DLG_TEMPLATE * result )
{
    result->style   = GET_DWORD(p); p += sizeof(DWORD);
    result->nbItems = (unsigned char) *p++;
    result->x       = GET_WORD(p);  p += sizeof(WORD);
    result->y       = GET_WORD(p);  p += sizeof(WORD);
    result->cx      = GET_WORD(p);  p += sizeof(WORD);
    result->cy      = GET_WORD(p);  p += sizeof(WORD);

//    TRACE("DIALOG %d, %d, %d, %d\n", result->x, result->y, result->cx, result->cy );
//    TRACE(" STYLE %08lx\n", result->style );

    /* Get the menu name */

    switch( (BYTE)*p )
    {
    case 0:
        result->menuName = 0;
        p++;
        break;
    case 0xff:
        result->menuName = MAKEINTRESOURCE(GET_WORD( p + 1 ));
        p += 3;
//        TRACE(" MENU %04x\n", LOWORD(result->menuName) );
        break;
    default:
        result->menuName = p;
//        TRACE(" MENU '%s'\n", p );
        p += lstrlen(p) + 1;
        break;
    }

    /* Get the class name */

    if (*p)
    {
        result->className = p;
//        TRACE(" CLASS '%s'\n", result->className );
    }
    else result->className = (LPCSTR)DIALOG_CLASS_ATOM;
    p += lstrlen(p) + 1;

    /* Get the window caption */

    result->caption = p;
    p += lstrlen(p) + 1;
//    TRACE(" CAPTION '%s'\n", result->caption );

    /* Get the font name */

    result->pointSize = 0;
    result->faceName = NULL;

    if (result->style & DS_SETFONT)
    {
        result->pointSize = GET_WORD(p);
        p += sizeof(WORD);
        result->faceName = p;
        p += lstrlen(p) + 1;
//        TRACE(" FONT %d,'%s'\n", result->pointSize, result->faceName );
    }
    return p;
}

#if 0

/***********************************************************************
 *           DIALOG_CreateIndirect16
 *
 * Creates a dialog box window
 *
 * modal = TRUE if we are called from a modal dialog box.
 * (it's more compatible to do it here, as under Windows the owner
 * is never disabled if the dialog fails because of an invalid template)
 */
static HWND DIALOG_CreateIndirect16( HINSTANCE hInst, LPCVOID dlgTemplate,
                                     HWND owner, DLGPROC dlgProc, LPARAM param,
                                     BOOL modal )
{
    HWND hwnd;
    RECT rect;
    POINT pos;
    SIZE size;
    DLG_TEMPLATE template;
    DIALOGINFO * dlgInfo;
    BOOL ownerEnabled = TRUE;
    DWORD exStyle = 0;
    DWORD units = GetDialogBaseUnits();
    HMENU16 hMenu = 0;
    HFONT hUserFont = 0;
    UINT flags = 0;
    UINT xBaseUnit = LOWORD(units);
    UINT yBaseUnit = HIWORD(units);

      /* Parse dialog template */

    dlgTemplate = DIALOG_ParseTemplate16( dlgTemplate, &template );

      /* Load menu */

    if (template.menuName) hMenu = LoadMenu16( hInst, template.menuName );

      /* Create custom font if needed */

    if (template.style & DS_SETFONT)
    {
          /* We convert the size to pixels and then make it -ve.  This works
           * for both +ve and -ve template.pointSize */
        HDC dc;
        int pixels;
        dc = GetDC(0);
        pixels = MulDiv(template.pointSize, GetDeviceCaps(dc , LOGPIXELSY), 72);
        hUserFont = CreateFontA( -pixels, 0, 0, 0, FW_DONTCARE,
                                 FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                                 PROOF_QUALITY, FF_DONTCARE, template.faceName );
        if (hUserFont)
        {
            SIZE charSize;
            HFONT hOldFont = SelectObject( dc, hUserFont );
            charSize.cx = GdiGetCharDimensions( dc, NULL, &charSize.cy );
            if (charSize.cx)
            {
                xBaseUnit = charSize.cx;
                yBaseUnit = charSize.cy;
            }
            SelectObject( dc, hOldFont );
        }
        ReleaseDC(0, dc);
        TRACE("units = %d,%d\n", xBaseUnit, yBaseUnit );
    }

    /* Create dialog main window */

    rect.left = rect.top = 0;
    rect.right = MulDiv(template.cx, xBaseUnit, 4);
    rect.bottom =  MulDiv(template.cy, yBaseUnit, 8);
    if (template.style & DS_MODALFRAME) exStyle |= WS_EX_DLGMODALFRAME;
    AdjustWindowRectEx( &rect, template.style, (hMenu != 0), exStyle );
    pos.x = rect.left;
    pos.y = rect.top;
    size.cx = rect.right - rect.left;
    size.cy = rect.bottom - rect.top;

    if (template.x == CW_USEDEFAULT16)
    {
        pos.x = pos.y = CW_USEDEFAULT16;
    }
    else
    {
        HMONITOR monitor = 0;
        MONITORINFO mon_info;

        mon_info.cbSize = sizeof(mon_info);
        if (template.style & DS_CENTER)
        {
            monitor = MonitorFromWindow( owner ? owner : GetActiveWindow(), MONITOR_DEFAULTTOPRIMARY );
            GetMonitorInfoW( monitor, &mon_info );
            pos.x = (mon_info.rcWork.left + mon_info.rcWork.right - size.cx) / 2;
            pos.y = (mon_info.rcWork.top + mon_info.rcWork.bottom - size.cy) / 2;
        }
        else if (template.style & DS_CENTERMOUSE)
        {
            GetCursorPos( &pos );
            monitor = MonitorFromPoint( pos, MONITOR_DEFAULTTOPRIMARY );
            GetMonitorInfoW( monitor, &mon_info );
        }
        else
        {
            pos.x += MulDiv(template.x, xBaseUnit, 4);
            pos.y += MulDiv(template.y, yBaseUnit, 8);
            if (!(template.style & (WS_CHILD|DS_ABSALIGN))) ClientToScreen( owner, &pos );
        }
        if ( !(template.style & WS_CHILD) )
        {
            INT dX, dY;

            /* try to fit it into the desktop */

            if (!monitor)
            {
                SetRect( &rect, pos.x, pos.y, pos.x + size.cx, pos.y + size.cy );
                monitor = MonitorFromRect( &rect, MONITOR_DEFAULTTOPRIMARY );
                GetMonitorInfoW( monitor, &mon_info );
            }
            if ((dX = pos.x + size.cx + GetSystemMetrics(SM_CXDLGFRAME) - mon_info.rcWork.right) > 0)
                pos.x -= dX;
            if ((dY = pos.y + size.cy + GetSystemMetrics(SM_CYDLGFRAME) - mon_info.rcWork.bottom) > 0)
                pos.y -= dY;
            if( pos.x < mon_info.rcWork.left ) pos.x = mon_info.rcWork.left;
            if( pos.y < mon_info.rcWork.top ) pos.y = mon_info.rcWork.top;
        }
    }

    if (modal)
    {
        ownerEnabled = DIALOG_DisableOwner( owner );
        if (ownerEnabled) flags |= DF_OWNERENABLED;
    }

    hwnd = WIN_Handle32( CreateWindowEx16(exStyle, template.className,
                                          template.caption, template.style & ~WS_VISIBLE,
                                          pos.x, pos.y, size.cx, size.cy,
                                          HWND_16(owner), hMenu, hInst, NULL ));
    if (!hwnd)
    {
        if (hUserFont) DeleteObject( hUserFont );
        if (hMenu) DestroyMenu16( hMenu );
        if (modal && (flags & DF_OWNERENABLED)) DIALOG_EnableOwner(owner);
        return 0;
    }
    dlgInfo = wow_handlers32.get_dialog_info( hwnd, TRUE );
    dlgInfo->hwndFocus   = 0;
    dlgInfo->hUserFont   = hUserFont;
    dlgInfo->hMenu       = HMENU_32( hMenu );
    dlgInfo->xBaseUnit   = xBaseUnit;
    dlgInfo->yBaseUnit   = yBaseUnit;
    dlgInfo->idResult    = 0;
    dlgInfo->flags       = flags;

    SetWindowLong16( HWND_16(hwnd), DWLP_DLGPROC, (LONG)dlgProc );

    if (hUserFont)
        SendMessageA( hwnd, WM_SETFONT, (WPARAM)hUserFont, 0 );

    /* Create controls */

    if (DIALOG_CreateControls16( hwnd, dlgTemplate, &template, hInst ))
    {
        HWND hwndPreInitFocus;

        /* Send initialisation messages and set focus */

        dlgInfo->hwndFocus = GetNextDlgTabItem( hwnd, 0, FALSE );

        hwndPreInitFocus = GetFocus();
        if (SendMessageA( hwnd, WM_INITDIALOG, (WPARAM)dlgInfo->hwndFocus, param ))
        {
            /* check where the focus is again,
	     * some controls status might have changed in WM_INITDIALOG */
            dlgInfo->hwndFocus = GetNextDlgTabItem( hwnd, 0, FALSE);
            if( dlgInfo->hwndFocus )
                SetFocus( dlgInfo->hwndFocus );
        }
        else
        {
            /* If the dlgproc has returned FALSE (indicating handling of keyboard focus)
               but the focus has not changed, set the focus where we expect it. */
            if ((GetFocus() == hwndPreInitFocus) &&
                (GetWindowLongW( hwnd, GWL_STYLE ) & WS_VISIBLE))
            {
                dlgInfo->hwndFocus = GetNextDlgTabItem( hwnd, 0, FALSE);
                if( dlgInfo->hwndFocus )
                    SetFocus( dlgInfo->hwndFocus );
            }
        }

        if (template.style & WS_VISIBLE && !(GetWindowLongW( hwnd, GWL_STYLE ) & WS_VISIBLE))
        {
            ShowWindow( hwnd, SW_SHOWNORMAL );  /* SW_SHOW doesn't always work */
        }
        return hwnd;
    }
    if( IsWindow(hwnd) ) DestroyWindow( hwnd );
    if (modal && ownerEnabled) DIALOG_EnableOwner(owner);
    return 0;
}
#endif

/***********************************************************************
 *		DialogBox (USER.87)
 */
int WINAPI DialogBox( HINSTANCE hInst, LPCSTR dlgTemplate,
                          HWND owner, DLGPROC dlgProc )
{
    return DialogBoxParam( hInst, dlgTemplate, owner, dlgProc, 0 );
}

/***********************************************************************
 *		CreateDialog (USER.89)
 */
HWND WINAPI CreateDialog( HINSTANCE hInst, LPCSTR dlgTemplate,
                              HWND owner, DLGPROC dlgProc )
{
    return CreateDialogParam( hInst, dlgTemplate, owner, dlgProc, 0 );
}


/**************************************************************************
 *              SetDlgItemText   (USER.92)
 */
void WINAPI SetDlgItemText( HWND hwnd, int id, LPCSTR lpString )
{
    SendDlgItemMessage( hwnd, id, WM_SETTEXT, 0, (LONG)lpString );
}

/**************************************************************************
 *              GetDlgItemText   (USER.93)
 */
int WINAPI GetDlgItemText( HWND hwnd, int id, LPSTR str, int len )
{
    return SendDlgItemMessage( hwnd, id, WM_GETTEXT, len, (LONG)str );
}


/**************************************************************************
 *              CheckDlgButton   (USER.97)
 */
VOID WINAPI CheckDlgButton( HWND hwnd, int id, UINT check )
{
    SendDlgItemMessage( hwnd, id, BM_SETCHECK, check, 0 );
//    return TRUE;
}


/**************************************************************************
 *              IsDlgButtonChecked   (USER.98)
 */
UINT WINAPI IsDlgButtonChecked( HWND hwnd, int id )
{
    return (UINT)SendDlgItemMessage( hwnd, id, BM_GETCHECK, 0, 0 );
}


/**************************************************************************
 *              DlgDirSelect   (USER.99)
 */
BOOL WINAPI DlgDirSelect( HWND hwnd, LPSTR str, int id )
{
    return DlgDirSelectEx( hwnd, str, 128, id );
}



/**************************************************************************
 *              SendDlgItemMessage   (USER.101)
 */
LRESULT WINAPI SendDlgItemMessage( HWND hwnd, int id, UINT msg,
                                     WPARAM wParam, LPARAM lParam )
{
    HWND hwndCtrl = GetDlgItem( hwnd, id );
    if (hwndCtrl) return SendMessage( hwndCtrl, msg, wParam, lParam );
    else return 0;
}


/**************************************************************************
 *              DlgDirSelectComboBox   (USER.194)
 */
BOOL WINAPI DlgDirSelectComboBox( HWND hwnd, LPSTR str, int id )
{
    return DlgDirSelectComboBoxEx( hwnd, str, 128, id );
}



/***********************************************************************
 *		DialogBoxIndirect (USER.218)
 */
int WINAPI DialogBoxIndirect( HINSTANCE hInst, HANDLE dlgTemplate,
                                  HWND owner, DLGPROC dlgProc )
{
    return DialogBoxIndirectParam( hInst, dlgTemplate, owner, dlgProc, 0 );
}


/***********************************************************************
 *		CreateDialogIndirect (USER.219)
 */
HWND WINAPI CreateDialogIndirect( HINSTANCE hInst, const void far * dlgTemplate,
                                      HWND owner, DLGPROC dlgProc )
{
    return CreateDialogIndirectParam( hInst, dlgTemplate, owner, dlgProc, 0);
}


#if 0
/***********************************************************************
 *		DialogBoxParam (USER.239)
 */
INT16 WINAPI DialogBoxParam16( HINSTANCE16 hInst, LPCSTR template,
                               HWND16 owner16, DLGPROC16 dlgProc, LPARAM param )
{
    HWND hwnd = 0;
    HRSRC16 hRsrc;
    HGLOBAL16 hmem;
    LPCVOID data;
    int ret = -1;

    if (!(hRsrc = FindResource16( hInst, template, (LPSTR)RT_DIALOG ))) return 0;
    if (!(hmem = LoadResource16( hInst, hRsrc ))) return 0;
    if ((data = LockResource16( hmem )))
    {
        HWND owner = WIN_Handle32(owner16);
        hwnd = DIALOG_CreateIndirect16( hInst, data, owner, dlgProc, param, TRUE );
        if (hwnd) ret = wow_handlers32.dialog_box_loop( hwnd, owner  );
        GlobalUnlock16( hmem );
    }
    FreeResource16( hmem );
    return ret;
}


/***********************************************************************
 *		DialogBoxIndirectParam (USER.240)
 */
INT16 WINAPI DialogBoxIndirectParam16( HINSTANCE16 hInst, HANDLE16 dlgTemplate,
                                       HWND16 owner16, DLGPROC16 dlgProc, LPARAM param )
{
    HWND hwnd, owner = WIN_Handle32( owner16 );
    LPCVOID ptr;

    if (!(ptr = GlobalLock16( dlgTemplate ))) return -1;
    hwnd = DIALOG_CreateIndirect16( hInst, ptr, owner, dlgProc, param, TRUE );
    GlobalUnlock16( dlgTemplate );
    if (hwnd) return wow_handlers32.dialog_box_loop( hwnd, owner );
    return -1;
}

#endif

/***********************************************************************
 *		CreateDialogParam (USER.241)
 */
HWND WINAPI CreateDialogParam( HINSTANCE hInst, LPCSTR dlgTemplate,
                                   HWND owner, DLGPROC dlgProc, LPARAM param )
{
    HWND hwnd = 0;
    HRSRC hRsrc;
    HGLOBAL hmem;
    const void far * data;

//    TRACE("%04x,%s,%04x,%p,%Ix\n",
//          hInst, debugstr_a(dlgTemplate), owner, dlgProc, param );

    if (!(hRsrc = FindResource( hInst, dlgTemplate, (LPSTR)RT_DIALOG ))) return 0;
    if (!(hmem = LoadResource( hInst, hRsrc ))) return 0;
    if (!(data = LockResource( hmem ))) hwnd = 0;
    else hwnd = CreateDialogIndirectParam( hInst, data, owner, dlgProc, param );
    FreeResource( hmem );
    return hwnd;
}


#if 0
/***********************************************************************
 *		CreateDialogIndirectParam (USER.242)
 */
HWND16 WINAPI CreateDialogIndirectParam16( HINSTANCE16 hInst, LPCVOID dlgTemplate,
                                           HWND16 owner, DLGPROC16 dlgProc, LPARAM param )
{
    if (!dlgTemplate) return 0;
    return HWND_16( DIALOG_CreateIndirect16( hInst, dlgTemplate, WIN_Handle32(owner),
                                             dlgProc, param, FALSE ));
}

#endif
