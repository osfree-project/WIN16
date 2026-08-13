/*
 * Focus functions
 *
 * Copyright 1993 David Metcalfe
 *           1994 Alexandre Julliard
 * 	     1995 Alex Korobka
 *
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

#include "user.h"

/*****************************************************************
 *	         FOCUS_SwitchFocus 
 */
void FOCUS_SwitchFocus(HWND hFocusFrom, HWND hFocusTo)
{
    hwndFocus = hFocusTo;

    if (hFocusFrom) SendMessage( hFocusFrom, WM_KILLFOCUS, (WPARAM)hFocusTo, 0L);
    if( !hFocusTo || hFocusTo != hwndFocus )
	return;

    SendMessage( hFocusTo, WM_SETFOCUS, (WPARAM)hFocusFrom, 0L);
//    FOCUS_SetXFocus( hFocusTo );
}


/*****************************************************************
 *               SetFocus            (USER.22)
 */
HWND WINAPI SetFocus(HWND hwnd)
{
    HWND hWndPrevFocus, hwndTop = hwnd;
    WND *wndPtr = WIN_FindWndPtr( hwnd );

    if (wndPtr)
    {
	  /* Check if we can set the focus to this window */

	while ( (wndPtr->dwStyle & (WS_CHILD | WS_POPUP)) == WS_CHILD  )
	{
	    if ( wndPtr->dwStyle & ( WS_MINIMIZE | WS_DISABLED) )
		 return 0;
            if (!(wndPtr = wndPtr->parent)) return 0;
	    hwndTop = wndPtr->hwndSelf;
	}

	if( hwnd == hwndFocus ) return hwnd;

	/* call hooks */
//@todo	if( HOOK_CallHooks( WH_CBT, HCBT_SETFOCUS, (WPARAM)hwnd, (LPARAM)hwndFocus) )
//	    return 0;

        /* activate hwndTop if needed. */
	if (hwndTop != GetActiveWindow())
	{
	    if (!WINPOS_SetActiveWindow(hwndTop, 0, 0)) return 0;

	    if (!IsWindow( hwnd )) return 0;  /* Abort if window destroyed */
	}
    }
    else ;//@todo if( HOOK_CallHooks( WH_CBT, HCBT_SETFOCUS, 0, (LPARAM)hwndFocus ) )
            // return 0;

      /* Change focus and send messages */
    hWndPrevFocus = hwndFocus;

    FOCUS_SwitchFocus( hwndFocus , hwnd );

    return hWndPrevFocus;
}


/*****************************************************************
 *               GetFocus            (USER.23)
 */
HWND WINAPI GetFocus(void)
{
    return hwndFocus;
}


