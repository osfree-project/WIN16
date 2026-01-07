#include <user.h>

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
	 
	 char send;

//	FUNCTION_START
	SetDS(USER_HeapSel);

	  _asm{
		  mov EventCodes,	AX 
		  mov hMouse, 		BX
		  mov vMouse,		CX
		  mov NumButts,		DX
	  }

	dwMouseX+=hMouse;
	dwMouseY+=vMouse;

	MoveCursor(dwMouseX, dwMouseY);
//	CheckCursor(); must be called by timer event
	  
//	 printf("\r\n %d MOUSE.drv: EventCodes[%d], hMouse[%d], vMouse[%d],  NumButts[%d] ",EventCodes,EventCodes,hMouse,vMouse,NumButts );

	// send event to System Message Queue
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
