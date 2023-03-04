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

/***********************************************************************
 *		CreateWindow (USER.41)
 */
HWND WINAPI CreateWindow( LPCSTR className, LPCSTR windowName,
                              DWORD style, int x, int y, int width,
                              int height, HWND parent, HMENU menu,
                              HINSTANCE instance, LPVOID data )
{
    return CreateWindowEx( 0, className, windowName, style,
                             x, y, width, height, parent, menu, instance, data );
}
