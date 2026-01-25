/*
 * Windows 16-bit hook functions
 *
 * Copyright 1994, 1995, 2002 Alexandre Julliard
 * Copyright 1996 Andrew Lewycky
 *
 * Based on investigations by Alex Korobka
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

//#include <stdarg.h>

#include "user.h"


/***********************************************************************
 *		SetWindowsHook (USER.121)
 */
FARPROC WINAPI SetWindowsHook( int id, HOOKPROC proc )
{
    HINSTANCE hInst = FarGetOwner( HIWORD(proc) );

    /* WH_MSGFILTER is the only task-specific hook for SetWindowsHook() */
    HTASK hTask = (id == WH_MSGFILTER) ? GetCurrentTask() : 0;

	FUNCTION_START

    return (FARPROC)SetWindowsHookEx( id, proc, hInst, hTask );
}



/***********************************************************************
 *		DefHookProc (USER.235)
 */
LRESULT WINAPI DefHookProc( int code, WPARAM wparam, LPARAM lparam, HOOKPROC far *hhook )
{
	FUNCTION_START
    return CallNextHookEx( (HHOOK)*hhook, code, wparam, lparam );
}

/***********************************************************************
 *		CallMsgFilter (USER.123)
 */
BOOL WINAPI CallMsgFilter( MSG FAR *msg, int code )
{
	FUNCTION_START
    return 0;//CallMsgFilter32_16( (MSG32_16 *)msg, code, FALSE );
}

/***********************************************************************
 *		SetWindowsHookEx (USER.291)
 */
HHOOK WINAPI SetWindowsHookEx( int id, HOOKPROC proc, HINSTANCE hInst, HTASK hTask )
{
	FUNCTION_START
	#if 0
    struct hook16_queue_info *info;
    HHOOK hook;
    int index = id - WH_MINHOOK;

    if (id < WH_MINHOOK || id > WH_MAXHOOK16) return 0;
    if (!hook_procs[index])
    {
        FIXME( "hook type %d broken in Win16\n", id );
        return 0;
    }
    if (!hTask) FIXME( "System-global hooks (%d) broken in Win16\n", id );
    else if (hTask != GetCurrentTask())
    {
        FIXME( "setting hook (%d) on other task not supported\n", id );
        return 0;
    }

    if (!(info = get_hook_info( TRUE ))) return 0;
    if (info->hook[index])
    {
        FIXME( "Multiple hooks (%d) for the same task not supported yet\n", id );
        return 0;
    }
    if (!(hook = SetWindowsHookExA( id, hook_procs[index], 0, GetCurrentThreadId() ))) return 0;
    info->hook[index] = hook;
    info->proc[index] = proc;
    return hook;
	#endif
	return 0;
}

/***********************************************************************
 *		CallNextHookEx (USER.293)
 */
LRESULT WINAPI CallNextHookEx( HHOOK hhook, int code, WPARAM wparam, LPARAM lparam )
{
	FUNCTION_START
	#if 0
    struct hook16_queue_info *info;
    LRESULT ret = 0;

    if (!(info = get_hook_info( FALSE ))) return 0;

    switch (info->id)
    {
    case WH_MSGFILTER:
    {
        MSG16 *msg16 = MapSL(lparam);
        MSG msg32;

        map_msg_16_to_32( msg16, &msg32 );
        ret = CallNextHookEx( hhook, code, wparam, (LPARAM)&msg32 );
        break;
    }

    case WH_GETMESSAGE:
    {
        MSG16 *msg16 = MapSL(lparam);
        MSG msg32;

        map_msg_16_to_32( msg16, &msg32 );
        ret = CallNextHookEx( hhook, code, wparam, (LPARAM)&msg32 );
        map_msg_32_to_16( &msg32, msg16 );
        break;
    }

    case WH_CALLWNDPROC:
    {
        CWPSTRUCT16 *cwp16 = MapSL(lparam);
        LRESULT result;
        struct wndproc_hook_params params;

        params.hhook  = hhook;
        params.code   = code;
        params.wparam = wparam;
        ret = WINPROC_CallProc16To32A( wndproc_hook_callback, cwp16->hwnd, cwp16->message,
                                       cwp16->wParam, cwp16->lParam, &result, &params );
        break;
    }

    case WH_CBT:
        switch (code)
        {
        case HCBT_CREATEWND:
            {
                CBT_CREATEWNDA cbtcw32;
                CREATESTRUCTA cs32;
                CBT_CREATEWND16 *cbtcw16 = MapSL(lparam);
                CREATESTRUCT16 *cs16 = MapSL( (SEGPTR)cbtcw16->lpcs );

                cbtcw32.lpcs = &cs32;
                cbtcw32.hwndInsertAfter = WIN_Handle32( cbtcw16->hwndInsertAfter );

                cs32.lpCreateParams = (LPVOID)cs16->lpCreateParams;
                cs32.hInstance      = HINSTANCE_32(cs16->hInstance);
                cs32.hMenu          = HMENU_32(cs16->hMenu);
                cs32.hwndParent     = WIN_Handle32(cs16->hwndParent);
                cs32.cy             = cs16->cy;
                cs32.cx             = cs16->cx;
                cs32.y              = cs16->y;
                cs32.x              = cs16->x;
                cs32.style          = cs16->style;
                cs32.lpszName       = MapSL( cs16->lpszName );
                cs32.lpszClass      = MapSL( cs16->lpszClass );
                cs32.dwExStyle      = cs16->dwExStyle;

                ret = CallNextHookEx( hhook, code, wparam, (LPARAM)&cbtcw32 );
                cbtcw16->hwndInsertAfter = HWND_16( cbtcw32.hwndInsertAfter );
                break;
            }
        case HCBT_ACTIVATE:
            {
                CBTACTIVATESTRUCT16 *cas16 = MapSL(lparam);
                CBTACTIVATESTRUCT cas32;
                cas32.fMouse = cas16->fMouse;
                cas32.hWndActive = WIN_Handle32(cas16->hWndActive);
                ret = CallNextHookEx( hhook, code, wparam, (LPARAM)&cas32 );
                break;
            }
        case HCBT_CLICKSKIPPED:
            {
                MOUSEHOOKSTRUCT16 *ms16 = MapSL(lparam);
                MOUSEHOOKSTRUCT ms32;

                ms32.pt.x = ms16->pt.x;
                ms32.pt.y = ms16->pt.y;
                /* wHitTestCode may be negative, so convince compiler to do
                   correct sign extension. Yay. :| */
                ms32.wHitTestCode = (INT)(INT16)ms16->wHitTestCode;
                ms32.dwExtraInfo = ms16->dwExtraInfo;
                ms32.hwnd = WIN_Handle32( ms16->hwnd );
                ret = CallNextHookEx( hhook, code, wparam, (LPARAM)&ms32 );
                break;
            }
        case HCBT_MOVESIZE:
            {
                RECT16 *rect16 = MapSL(lparam);
                RECT rect32;

                rect32.left   = rect16->left;
                rect32.top    = rect16->top;
                rect32.right  = rect16->right;
                rect32.bottom = rect16->bottom;
                ret = CallNextHookEx( hhook, code, wparam, (LPARAM)&rect32 );
                break;
            }
        }
        break;

    case WH_MOUSE:
    {
        MOUSEHOOKSTRUCT16 *ms16 = MapSL(lparam);
        MOUSEHOOKSTRUCT ms32;

        ms32.pt.x = ms16->pt.x;
        ms32.pt.y = ms16->pt.y;
        /* wHitTestCode may be negative, so convince compiler to do
           correct sign extension. Yay. :| */
        ms32.wHitTestCode = (INT)((INT16)ms16->wHitTestCode);
        ms32.dwExtraInfo = ms16->dwExtraInfo;
        ms32.hwnd = WIN_Handle32(ms16->hwnd);
        ret = CallNextHookEx( hhook, code, wparam, (LPARAM)&ms32 );
        break;
    }

    case WH_SHELL:
    case WH_KEYBOARD:
        ret = CallNextHookEx( hhook, code, wparam, lparam );
        break;

    case WH_HARDWARE:
    case WH_FOREGROUNDIDLE:
    case WH_CALLWNDPROCRET:
    case WH_SYSMSGFILTER:
    case WH_JOURNALRECORD:
    case WH_JOURNALPLAYBACK:
    default:
        FIXME("\t[%i] 16to32 translation unimplemented\n", info->id);
        ret = CallNextHookEx( hhook, code, wparam, lparam );
        break;
    }
    return ret;
	#endif
	return 0;
}
