#include <user.h>
#include <queue.h>

/***********************************************************************
 *		keybd_event (USER.???)
 */
VOID WINAPI keybd_event(VOID)
{
	FUNCTION_START
	// send event to System Message Queue
	FUNCTION_END
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

//	FUNCTION_START
	SetDS(USER_HeapSel);

	  _asm{
		  mov EventCodes,	AX 
		  mov hMouse, 		BX
		  mov vMouse,		CX
		  mov NumButts,		DX
	  }


    if (EventCodes & ME_MOVE)
    {
	dwMouseX+=hMouse;
	dwMouseY+=vMouse;
	MoveCursor(dwMouseX, dwMouseY);
	hardware_event( WM_MOUSEMOVE, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0 );
    }
    if (EventCodes & ME_LDOWN)
        hardware_event( WM_LBUTTONDOWN, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0);

    if (EventCodes & ME_LUP)
        hardware_event( WM_LBUTTONUP, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0);

    if (EventCodes & ME_RDOWN)
        hardware_event( WM_RBUTTONDOWN, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0);

    if (EventCodes & ME_RUP)
        hardware_event( WM_RBUTTONUP, KeyState, 0, dwMouseX, dwMouseY, GetTickCount(), 0);

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
