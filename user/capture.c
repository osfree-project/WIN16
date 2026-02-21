/*    
    	Copyright 2026 Yuri Prokushev

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

/**********************************************************************
 *		ReleaseCapture	(USER.19)
 */
VOID WINAPI ReleaseCapture()
{
	PushDS();
	SetUserHeapDS();

	captureWnd = 0;

	PopDS();
}

/**********************************************************************
 *		SetCapture 	(USER.18)
 */
HWND WINAPI SetCapture(HWND hWnd)
{
	HWND retVal;

	PushDS();
	SetUserHeapDS();
	FUNCTION_START

	retVal = captureWnd;
	captureWnd = hWnd;

	FUNCTION_END
	PopDS();
	return retVal;
}

/**********************************************************************
 *		GetCapture 	(USER.236)
 */
HWND WINAPI GetCapture()
{
	HWND retVal;

	PushDS();
	SetUserHeapDS();
//	FUNCTION_START

	retVal=captureWnd;

//	FUNCTION_END
	PopDS();
	return retVal;
}
