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

VOID WINAPI mouse_event_impl(WORD EventCodes, WORD hMouse, WORD vMouse, WORD NumButts)
{
//	WORD EventCodes;
//	WORD hMouse;
//	WORD vMouse;
//	WORD NumButts;
	WORD KeyState;
    
//	_asm{
//		mov EventCodes, AX 
//		mov hMouse, BX
//		mov vMouse, CX
//		mov NumButts, DX
//	}

//	PushDS();
	SetUserHeapDS();


//TRACE("mouse_event: AX=0x%04x (MOVE=%d LEFTDOWN=%d LEFTUP=%d RIGHTDOWN=%d RIGHTUP=%d) BX=%d CX=%d",
//          EventCodes,
//          (EventCodes & MOUSEEVENTF_MOVE) ? 1 : 0,
//          (EventCodes & MOUSEEVENTF_LEFTDOWN) ? 1 : 0,
//          (EventCodes & MOUSEEVENTF_LEFTUP) ? 1 : 0,
//          (EventCodes & MOUSEEVENTF_RIGHTDOWN) ? 1 : 0,
//          (EventCodes & MOUSEEVENTF_RIGHTUP) ? 1 : 0,
//          hMouse, vMouse);

#if 1
	KeyState=0;
	if (EventCodes & MOUSEEVENTF_LEFTDOWN)
	{
		KeyState |= MK_LBUTTON;
		MouseButtonsStates[0] = TRUE;
		AsyncMouseButtonsStates[0] = TRUE;
	}
	if (EventCodes & MOUSEEVENTF_LEFTUP)
	{
		KeyState &= ~MK_LBUTTON;
		MouseButtonsStates[0] = FALSE;
		AsyncMouseButtonsStates[0] = FALSE;
	}
	if (EventCodes & MOUSEEVENTF_RIGHTDOWN)
	{
		KeyState |= MK_RBUTTON;
		MouseButtonsStates[2] = TRUE;
		AsyncMouseButtonsStates[2] = TRUE;
	}
	if (EventCodes & MOUSEEVENTF_RIGHTUP)
	{
		KeyState &= ~MK_RBUTTON;
		MouseButtonsStates[2] = FALSE;
		AsyncMouseButtonsStates[2] = FALSE;
	}
    
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
        
		if (wMouseX > (GETSYSTEMMETRICS(SM_CXSCREEN) - 1)) {
	            wMouseX = (GETSYSTEMMETRICS(SM_CXSCREEN) - 1);
		}

		if (wMouseY > (GETSYSTEMMETRICS(SM_CYSCREEN) - 1)) {
			wMouseY = (GETSYSTEMMETRICS(SM_CYSCREEN) - 1);
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
#endif    
//	PopDS();
}

//@todo Это вход из драйвера мыши. Надо сохранить сначала стек, потом переключиться на свой и при возврате - аернуть стек обратно.
__declspec(naked) void WINAPI mouse_event(void)
{
    _asm {
        ; на входе AX=EventCodes, BX=hMouse, CX=vMouse, DX=NumButts
	cli
        push ax          ; EventCodes
        push bx          ; hMouse
        push cx          ; vMouse
        push dx          ; NumButts
        call mouse_event_impl
	sti
        retf             ; FAR return
    }
}

#pragma code_seg();


/***********************************************************************
 *		GetMouseEventProc (USER.337)
 */
FARPROC WINAPI GetMouseEventProc(void)
{
//	HMODULE hModule;
	FARPROC retVal;

	PushDS();
	SetUserHeapDS();
	FUNCTION_START

	retVal=GetProcAddress(hModuleWin, "mouse_event");

	FUNCTION_END
	PopDS();
	return retVal;
}

/***********************************************************************
 *		IsUserIdle (USER.333)
 * p. 467 Undocumented Windows:
 * BOOL FAR PASCAL IsUserldle(void);
 * USER.59 !!! BUG IN BOOK!!!
 * This function, which indicates the level of end-user keyboard and mouse activity, is
 * called (via a function pointer returned from GetProcAddress) from the KERNEL
 * scheduler. It allows Windows to perform housekeeping while the user is not typing or
 * using the mouse. The KERNEL scheduler uses the return value from IsUserIdle() to
 * determine whether to set the Win_Idle_Mouse_Busy_Bit when calling the Wm_Ker-
 * nel_Idle (INT 2Fh AX=1689h) function (see INT2FAPI.INC in the DDK).
 * IsUserIdle() can be called asynchronously, for example, from within a Create-
 * SystemTimer callback function (see chapter 9).
 * In 3.1, this function is responsible for triggering screen-saver applications. When
 * no activity is detected (IsUserldle() returns TRUE), and the duration since the time of
 * last activity exceeds that specified in the ScreenSaveTimeOut= entry in WIN.INI, a
 * WM_SYSCOMMAND with wParam=SC_SCREENSAVE is sent to the screen-saver
 * application window. If, on the other hand, activity is detected, the time of last activity
 * is reset to the current time. (For more on screen savers, see chapter 14 of the 3.1 SDK
 * Overviews manual.)
 * Return: FALSE if input is pending; TRUE if no input
 * Support: 3.0, 3.1
 * p.425 Windows Internals:
 * Besides being called by Reschedul() to determine if anything is happening in the messaging
 * system, IsUserIdleO is also called by the internal IdleTime() function. IdleTimer() calls
 * IsUserIdle to give it a chance to activate the sere ens aver (added in Windows 3.1). During its
 * first invocation, InitAppO calls SetSystemTimerO, telling it to call IdleTimerO every 10 sec-
 * onds. The implication is that the minimum screensaver delay is 10 seconds.
 * Of some note are the tests in Is U serIdleO that determine whether to activate the
 * screensaver. In order to start the screensaver, the following conditions must be met:
 * - Enough time has elapsed.
 * - There's an active window. See Chapter 5 for details on what this entails.
 * - The active window cannot be a DOS application running in a window if running in
 * Enhanced mode.
 * If all these tests are passed, IsUserIdleO posts a WM_SYSCOMMAND message with an
 * SC_SCREENSAVE wParam. Assuming the active window doesn't handle the message,
 * DefWindowProcO gets it and calls the internal SysCommandO function to load the
 * screensaver module specified in the SYSTEM.INI file.
 * 
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
	/* Should check for screen saver activation here for win 3.1+ */

	#if 0

	// Here's where we find out if we need to post the
	// SC_SCREENSAVE message. We won't bother to do this if
	// there's a system modaL window up, or if the screen save
	// time intervaL is set to 0_
	if ( (HWndSysModal==0) && (IScreenSaveTimeout > 0)
		&& (TimeLastInputMessage != 0) )
	{
		// TimeLastInputMessage is a USER gLobal variabLe, which
		// remembers the time (in miLLiseconds) when the Last
		// input message (mouse or keyboard) was received.
		timeDiff = GetCurrentTime() - TimeLastInputMessage;
		// Has enough time eLapsed? Get out now if not.
		if ( (IScreenSaveTimeout * 1000) >= timeDiff )
			goto IsUserIdLe_done;
		if ( HWndActive == 0 )
			goto IsUserIdLe_done;
		// If no active window, don't
		// bother posting the message
		// If running in Standard mode, we'LL aLways post
		// the SC_SCREENSAVE message if we get this far.
		if ( (WinFLags & WF_ENHANCED) = 0)
			goto IsUserIdle_PostMessage;
		// Running in Enhanced mode. See if the active
		// window is for a DOS appLication in a window. Don't
		// post the SC_SCREENSAVE message if so.
		if ( IsWinOLdApTask(GetWindowTask(HWndActive)))
			goto IsUserIdLe_done;
IsUserIdLe_PostMessage:
		PostMessage( HWndActive, WM_SYSCOMMAND, // Post the
			SC_SCREENSAVE, 0 ); // screensave msg
		TimeLastInputMessage = 0; // Reset the idLe timer
	}
IsUserIdLe_done:
	#endif

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
	FUNCTION_END
}
