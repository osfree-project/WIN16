/*
 * DISPLAY driver
 *
 * Copyright 1998 Ulrich Weigand
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

#include "wine/debug.h"
#include "windef.h"
#include "winbase.h"
#include "wine/winuser16.h"

#include "pshpack1.h"
typedef struct tagCURSORINFO
{
    WORD wXMickeys;
    WORD wYMickeys;
} CURSORINFO, *PCURSORINFO, far *LPCURSORINFO;
#include "poppack.h"

/***********************************************************************
 *           Inquire			(DISPLAY.101)
 */
WORD WINAPI Inquire(LPCURSORINFO16 lpCursorInfo)
{
    lpCursorInfo->wXMickeys = 1;
    lpCursorInfo->wYMickeys = 1;

    return sizeof(CURSORINFO);
}

/***********************************************************************
 *           SetCursor			(DISPLAY.102)
 */
VOID WINAPI DISPLAY_SetCursor( struct tagCURSORICONINFO *lpCursor )
{
    FIXME("stub\n" );
}

/***********************************************************************
 *           MoveCursor			(DISPLAY.103)
 */
VOID WINAPI MoveCursor( WORD wAbsX, WORD wAbsY )
{
//    SetCursorPos( wAbsX, wAbsY );
}

/***********************************************************************
 *           CheckCursor                  (DISPLAY.104)
 */
VOID WINAPI CheckCursor16( void )
{
    TRACE("stub\n" );
}

/***********************************************************************
 *           GetDriverResourceID                  (DISPLAY.450)
 *
 * Used by USER to check if driver contains better version of a builtin
 * resource than USER (yes, our DISPLAY does !).
 * wQueriedResID is the ID USER asks about.
 * lpsResName does often contain "OEMBIN".
 */
DWORD WINAPI GetDriverResourceID( WORD wQueriedResID, LPSTR lpsResName )
{
	if (wQueriedResID == 3)
		return (DWORD)1;

	return (DWORD)wQueriedResID;
}

/***********************************************************************
 *           UserRepaintDisable			(DISPLAY.500)
 */
VOID WINAPI UserRepaintDisable( BOOL disable )
{
    FIXME("stub\n");
}
