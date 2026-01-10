#include <user.h>

#define GET_WM_CTLCOLOR_MSG(type)		    (UINT)(WM_CTLCOLOR)
#define GET_WM_CTLCOLOR_MPS(hdc, hwnd, type) \
        (WPARAM)(hdc), MAKELONG(hwnd, type)

/**************************************************************************
 *              GetControlBrush   (USER.326)
 */
HBRUSH WINAPI
GetControlBrush(HWND hWnd, HDC hDC, WORD wType)
{
    HWND hWndParent;
    HBRUSH hBrush;

//	FUNCTION_START

    hWndParent = GetParent(hWnd);
    if (!hWndParent)
	hWndParent = hWnd;
    hBrush =  (HBRUSH)SendMessage(hWndParent,GET_WM_CTLCOLOR_MSG(wType),
		GET_WM_CTLCOLOR_MPS(hDC,hWnd,wType));
    if (hBrush == 0)
	hBrush = GetStockObject(LTGRAY_BRUSH);
    return hBrush;
}
