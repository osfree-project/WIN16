#include <user.h>
#include <queue.h>

static HWND 	captureWnd = 0;

/***********************************************************************
 *		keybd_event (USER.???)
 */
VOID WINAPI keybd_event(VOID)
{
//	FUNCTION_START
	// send event to System Message Queue
//	FUNCTION_END
}

/***********************************************************************
 *		mouse_event (USER.299)
 */
VOID WINAPI mouse_event(VOID)
{
    /* Register values:
     * AX = mouse event
     * BX = horizontal displacement if AX & ME_MOVE
     * CX = vertical displacement if AX & ME_MOVE
     * DX = button state (?)
     * SI = mouse event flags (?)
     * SI  ExtraMessageInfo for 3.1
     * DI  ExtraMessageInfo for 3.1
     */

	 WORD EventCodes;   // The event codes. This is a bit-packed value that describes the various events being reported.Bit Description
						// The mouse has moved.
						//	1 The left button was pressed.
						//	2 The left button was released.
						//	3 The right button was pressed.
						//	4 The right button was released.
						//	5-14 Reserved
						//	15 The position values in BX and CX are in absolute coordinates.
	 WORD 	hMouse; 	//  Horizontal position.
	 WORD 	vMouse;		//  Vertical position.
	 WORD 	NumButts;	//  Number of buttons on the mouse (typically, 2).
	 
	 WORD	KeyState=0;	// MK_*

	  _asm{
		  mov EventCodes,	AX 
		  mov hMouse, 		BX
		  mov vMouse,		CX
		  mov NumButts,		DX
	  }

	PushDS();
	SetDS(USER_HeapSel);

//printf("%04x", EventCodes);
    if (EventCodes & ME_MOVE)
    {
	dwMouseX+=hMouse;
	dwMouseY+=vMouse;
	MoveCursor(dwMouseX, dwMouseY);
	hardware_event( WM_MOUSEMOVE, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0 );
	PopDS();
	return;
    }
    if (EventCodes & ME_LDOWN)
        hardware_event( WM_LBUTTONDOWN, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0);

    if (EventCodes & ME_LUP)
        hardware_event( WM_LBUTTONUP, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0);

    if (EventCodes & ME_RDOWN)
        hardware_event( WM_RBUTTONDOWN, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0);

    if (EventCodes & ME_RUP)
        hardware_event( WM_RBUTTONUP, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0);
  
	PopDS();
//	FUNCTION_END
}

/***********************************************************************
 *		GetMouseEventProc (USER.337)
 */
FARPROC WINAPI GetMouseEventProc(void)
{
	HMODULE hmodule;
	FUNCTION_START

	hmodule = GetModuleHandle("USER");
	return GetProcAddress(hmodule, "mouse_event");
}

/***********************************************************************
 *		IsUserIdle (USER.333)
 */
BOOL WINAPI IsUserIdle(void)
{
	FUNCTION_START
    if ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
        return FALSE;
    if ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 )
        return FALSE;
    if ( GetAsyncKeyState( VK_MBUTTON ) & 0x8000 )
        return FALSE;
    /* Should check for screen saver activation here ... */
    return TRUE;
}

/***********************************************************************
 *		UserYield (USER.332)
 */
void WINAPI UserYield(void)
{
    MSG msg;
	FUNCTION_START
    //PeekMessage( &msg, 0, 0, 0, PM_REMOVE | PM_QS_SENDMESSAGE );
}

/**********************************************************************
 *		SetCapture 	(USER.18)
 */
HWND WINAPI SetCapture( HWND hwnd )
{
//    Window win;
    HWND old_capture_wnd = captureWnd;

    if (!hwnd)
    {
        ReleaseCapture();
        return old_capture_wnd;
    }
//    if (!(win = WIN_GetXWindow( hwnd ))) return 0;
/*    if (XGrabPointer(display, win, False, 
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                     GrabModeAsync, GrabModeAsync,
                     None, None, CurrentTime ) == GrabSuccess)
    {
	dprintf_win(stddeb, "SetCapture: %04x\n", hwnd);
*/	captureWnd   = hwnd;
	return old_capture_wnd;
//    }
//    else */return 0;
}


/**********************************************************************
 *		ReleaseCapture	(USER.19)
 */
void WINAPI ReleaseCapture()
{
    if (captureWnd == 0) return;
//    XUngrabPointer( display, CurrentTime );
    captureWnd = 0;
//    dprintf_win(stddeb, "ReleaseCapture\n");
}

/**********************************************************************
 *		GetCapture 	(USER.236)
 */
HWND WINAPI GetCapture()
{
    return captureWnd;
}
