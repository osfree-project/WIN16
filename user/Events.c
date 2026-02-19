#include <user.h>

// All this function in fixed code segment, not moveable and discardable

#pragma code_seg( "FIXED_TEXT" );

/***********************************************************************
 *           hardware_event
 *
 * Add an event to the system message queue.
 */
void hardware_event( WORD message, WORD wParam, LONG lParam,
		     WORD xPos, WORD yPos, DWORD time, DWORD extraInfo )
{
	MSG FAR *msg;
	int pos;

//	TRACE("msg=%04x", message);
  
	if (!sysMsgQueue) return;
	pos = sysMsgQueue->nextFreeMessage;

	/* Merge with previous event if possible */

	if ((message == WM_MOUSEMOVE) && sysMsgQueue->msgCount)
	{
		if (pos > 0) pos--;
		else pos = sysMsgQueue->queueSize - 1;
		msg = &sysMsgQueue->messages[pos].msg;
		if ((msg->message == message) && (msg->wParam == wParam))
		sysMsgQueue->msgCount--;  /* Merge events */
	else
		pos = sysMsgQueue->nextFreeMessage;  /* Don't merge */
	}

	/* Check if queue is full */

	if ((pos == sysMsgQueue->nextMessage) && sysMsgQueue->msgCount)
	{
		/* Queue is full, beep (but not on every mouse motion...) */
		if (message != WM_MOUSEMOVE) MessageBeep(0);
		return;
	}

	/* Store message */

	msg = &sysMsgQueue->messages[pos].msg;
	msg->hwnd    = 0;
	msg->message = message;
	msg->wParam  = wParam;
	msg->lParam  = lParam;
	msg->time    = time;
	msg->pt.x    = xPos & 0xffff;
	msg->pt.y    = yPos & 0xffff;
	sysMsgQueue->messages[pos].extraInfo = extraInfo;
	if (pos < sysMsgQueue->queueSize - 1) pos++;
	else pos = 0;
	sysMsgQueue->nextFreeMessage = pos;
	sysMsgQueue->msgCount++;

//	QUEUE_DumpQueue(hmemSysMsgQueue);
}


/***********************************************************************
 *		keybd_event (USER.???)
 */
VOID WINAPI keybd_event(VOID)
{
//	FUNCTION_START
	// send event to System Message Queue
//	FUNCTION_END
}


/*
 * Register values:
 * AX = mouse event
 * BX = horizontal displacement if AX & ME_MOVE
 * CX = vertical displacement if AX & ME_MOVE
 * DX = buttons number
 * 
 * SI  ExtraMessageInfo for 3.1
 * DI  ExtraMessageInfo for 3.1
 */

VOID WINAPI mouse_event(VOID)
{
	WORD EventCodes;
	WORD hMouse;
	WORD vMouse;
	WORD NumButts;
	WORD KeyState;
    
	_asm{
		mov EventCodes, AX 
		mov hMouse, BX
		mov vMouse, CX
		mov NumButts, DX
	}

	PushDS();
	SetUserHeapDS();
    
	KeyState=0;
    
    // Определяем состояние клавиш-модификаторов
//    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
//        KeyState |= MK_SHIFT;
//        TRACE("SHIFT key is pressed");
//    }
//    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
//        KeyState |= MK_CONTROL;
//        TRACE("CONTROL key is pressed");
//    }

	if (EventCodes & MOUSEEVENTF_MOVE)
	{
		// @todo Handle absolute position

		if (((long)wMouseX + (int)hMouse)< 0) {
			wMouseX = 0;
		} else {
			wMouseX += (int)hMouse;
		}

		if (((long)wMouseY+(int)vMouse) < 0) {
			wMouseY = 0;
		} else {
			wMouseY += (int)vMouse;
		}
        
		if (wMouseX > (SysMetricsDef[SM_CXSCREEN] - 1)) {
	            wMouseX = (SysMetricsDef[SM_CXSCREEN] - 1);
		}

		if (wMouseY > (SysMetricsDef[SM_CYSCREEN] - 1)) {
			wMouseY = (SysMetricsDef[SM_CYSCREEN] - 1);
		}
		MoveCursor(wMouseX, wMouseY);
		hardware_event(WM_MOUSEMOVE, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
	}
    
	if (EventCodes & MOUSEEVENTF_LEFTDOWN) {
		hardware_event(WM_LBUTTONDOWN, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
	}
    
	if (EventCodes & MOUSEEVENTF_LEFTUP) {
		hardware_event(WM_LBUTTONUP, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
	}
    
	if (EventCodes & MOUSEEVENTF_RIGHTDOWN) {
		hardware_event(WM_RBUTTONDOWN, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
	}
    
	if (EventCodes & MOUSEEVENTF_RIGHTUP) {
		hardware_event(WM_RBUTTONUP, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
	}
    
	PopDS();
}

#pragma code_seg();


/***********************************************************************
 *		GetMouseEventProc (USER.337)
 */
FARPROC WINAPI GetMouseEventProc(void)
{
	HMODULE hModule;

	hModule = GetModuleHandle("USER");
	return GetProcAddress(hModule, "mouse_event");
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

	FUNCTION_END
	return TRUE;
}

/***********************************************************************
 *		UserYield (USER.332)
 */
VOID WINAPI UserYield(VOID)
{
	MSG msg;
	FUNCTION_START
	//PeekMessage( &msg, 0, 0, 0, PM_REMOVE | PM_QS_SENDMESSAGE );
}
