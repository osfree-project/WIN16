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

#define GET_WM_CTLCOLOR_MSG(type)		    (UINT)(WM_CTLCOLOR)
#define GET_WM_CTLCOLOR_MPS(hdc, hwnd, type) \
        (WPARAM)(hdc), MAKELONG(hwnd, type)

/**************************************************************************
 *              GetControlBrush   (USER.326)
 */
HBRUSH WINAPI GetControlBrush(HWND hWnd, HDC hDC, WORD wType)
{
	HWND hWndParent;
	HBRUSH hBrush;

	FUNCTION_START

	hWndParent = GetParent(hWnd);
    	if (!hWndParent)
		hWndParent = hWnd;
	hBrush =  (HBRUSH)SendMessage(hWndParent,GET_WM_CTLCOLOR_MSG(wType),
		GET_WM_CTLCOLOR_MPS(hDC,hWnd,wType));
	if (hBrush == 0)
		hBrush = GetStockObject(LTGRAY_BRUSH);

	FUNCTION_END
	return hBrush;
}
