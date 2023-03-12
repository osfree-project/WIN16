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

/**************************************************************************
 *              GetWindowText   (USER.36)
 */
int WINAPI GetWindowText( HWND hwnd, LPSTR lpString, int nMaxCount )
{
    return SendMessage( hwnd, WM_GETTEXT, nMaxCount, (LPARAM)lpString );
}


/**************************************************************************
 *              SetWindowText   (USER.37)
 */
VOID WINAPI SetWindowText( HWND hwnd, LPCSTR lpString )
{
    SendMessage( hwnd, WM_SETTEXT, 0, (LPARAM)lpString );
}


/**************************************************************************
 *              GetWindowTextLength   (USER.38)
 */
int WINAPI GetWindowTextLength( HWND hwnd )
{
    return SendMessage( hwnd, WM_GETTEXTLENGTH, 0, 0 );
}

/**************************************************************************
 *              UpdateWindow   (USER.124)
 */
void WINAPI UpdateWindow( HWND hwnd )
{
    RedrawWindow( hwnd, NULL, 0, RDW_UPDATENOW | RDW_ALLCHILDREN );
}


/**************************************************************************
 *              InvalidateRect   (USER.125)
 */
void WINAPI InvalidateRect( HWND hwnd, const RECT far *rect, BOOL erase )
{
    RedrawWindow( hwnd, rect, 0, RDW_INVALIDATE | (erase ? RDW_ERASE : 0) );
}


/**************************************************************************
 *              InvalidateRgn   (USER.126)
 */
void WINAPI InvalidateRgn( HWND hwnd, HRGN hrgn, BOOL erase )
{
    RedrawWindow( hwnd, NULL, hrgn, RDW_INVALIDATE | (erase ? RDW_ERASE : 0) );
}


/**************************************************************************
 *              ValidateRect   (USER.127)
 */
void WINAPI ValidateRect( HWND hwnd, const RECT far *rect )
{
    RedrawWindow( hwnd, rect, 0, RDW_VALIDATE | RDW_NOCHILDREN );
}


/**************************************************************************
 *              ValidateRgn   (USER.128)
 */
void WINAPI ValidateRgn( HWND hwnd, HRGN hrgn )
{
    RedrawWindow( hwnd, NULL, hrgn, RDW_VALIDATE | RDW_NOCHILDREN );
}
