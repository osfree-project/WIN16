#include <windows.h>

void WINAPI PaintRect( HWND hwndParent, HWND hwnd, HDC hdc,
                         HBRUSH hbrush, const RECT far *rect);

/**************************************************************************
 *              FillWindow   (USER.324)
 */
void WINAPI FillWindow( HWND hwndParent, HWND hwnd, HDC hdc, HBRUSH hbrush )
{
    RECT rect;
    GetClientRect( hwnd, &rect );
    DPtoLP( hdc, (LPPOINT)&rect, 2 );
    PaintRect( hwndParent, hwnd, hdc, hbrush, &rect );
}
