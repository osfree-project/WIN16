/*

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

#include <user.h>

DWORD WINAPI GetSystemMSecCount(void);

DWORD WINAPI GetCurrentTime()
{
	FUNCTION_START
    return GetSystemMSecCount();
}

DWORD WINAPI GetTickCount()
{
//	FUNCTION_START
    return GetSystemMSecCount();
}

#define SetSystemTimer BEAR11

/***********************************************************************
 *		SetSystemTimer (USER.11)
 */
UINT WINAPI SetSystemTimer( HWND hwnd, UINT id, UINT timeout, TIMERPROC proc )
{
	FUNCTION_START
    //TIMERPROC proc32 = (TIMERPROC)WINPROC_AllocProc16( (WNDPROC16)proc );
    //return SetTimer( WIN_Handle32(hwnd), (UINT_PTR)id | SYSTEM_TIMER_FLAG, timeout, proc32 );
	return 0;
}

#define KillSystemTimer BEAR182

/**************************************************************************
 *              KillSystemTimer   (USER.182)
 */
BOOL WINAPI KillSystemTimer( HWND hwnd, UINT id )
{
	FUNCTION_START
    return FALSE;//KillTimer( WIN_Handle32(hwnd), (UINT_PTR)id | SYSTEM_TIMER_FLAG );
}
