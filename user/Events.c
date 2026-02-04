#include <user.h>
#include <queue.h>

// All this function in fixed segment, not moveable and discardable

#pragma code_seg( "FIXED_TEXT" );

/***********************************************************************
 *		keybd_event (USER.???)
 */
VOID WINAPI keybd_event(VOID)
{
//	FUNCTION_START
	// send event to System Message Queue
//	FUNCTION_END
}


VOID WINAPI mouse_event(VOID)
{
    /* Register values:
     * AX = mouse event
     * BX = horizontal displacement if AX & ME_MOVE
     * CX = vertical displacement if AX & ME_MOVE
     * DX = buttons number
     * 
     * SI  ExtraMessageInfo for 3.1
     * DI  ExtraMessageInfo for 3.1
     */

    WORD EventCodes;
    WORD hMouse;
    WORD vMouse;
    WORD NumButts;
    WORD KeyState = 0;
    
    _asm{
        mov EventCodes, AX 
        mov hMouse, BX
        mov vMouse, CX
        mov NumButts, DX
    }

    PushDS();
    SetDS(USER_HeapSel);
    
    
    // Определяем состояние клавиш-модификаторов
//    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
//        KeyState |= MK_SHIFT;
//        TRACE("SHIFT key is pressed");
//    }
//    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
//        KeyState |= MK_CONTROL;
//        TRACE("CONTROL key is pressed");
//    }
    
//    // Состояние кнопок мыши из NumButts
//    if (NumButts & 0x01) {
//        KeyState |= MK_LBUTTON;
//        TRACE("LEFT mouse button is pressed");
//    }
//    if (NumButts & 0x02) {
//        KeyState |= MK_RBUTTON;
//        TRACE("RIGHT mouse button is pressed");
//    }
//    if (NumButts & 0x04) {
//        KeyState |= MK_MBUTTON;
//        TRACE("MIDDLE mouse button is pressed");
//    }
    
//    TRACE("Final KeyState = 0x%04X", KeyState);

    if (EventCodes & ME_MOVE)
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
        
        if (wMouseX > (CXScreen/*GetSystemMetrics(SM_CXSCREEN)*/ - 1)) {
            wMouseX = (CXScreen/*GetSystemMetrics(SM_CXSCREEN)*/ - 1);
        }
        if (wMouseY > (CYScreen/*GetSystemMetrics(SM_CYSCREEN)*/ - 1)) {
            wMouseY = (CYScreen/*GetSystemMetrics(SM_CYSCREEN)*/ - 1);
        }
        
        hardware_event(WM_MOUSEMOVE, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
        MoveCursor(wMouseX, wMouseY);
    }
    
    if (EventCodes & ME_LDOWN) {
        hardware_event(WM_LBUTTONDOWN, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
    }
    
    if (EventCodes & ME_LUP) {
        hardware_event(WM_LBUTTONUP, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
    }
    
    if (EventCodes & ME_RDOWN) {
        hardware_event(WM_RBUTTONDOWN, KeyState, 0, wMouseX, wMouseY, GetTickCount(), 0);
    }
    
    if (EventCodes & ME_RUP) {
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
    HWND old_capture_wnd = captureWnd;

    if (!hwnd)
    {
        ReleaseCapture();
        return old_capture_wnd;
    }
	captureWnd   = hwnd;
	return old_capture_wnd;
}


/**********************************************************************
 *		ReleaseCapture	(USER.19)
 */
void WINAPI ReleaseCapture()
{
    if (captureWnd == 0) return;
    captureWnd = 0;
}

/**********************************************************************
 *		GetCapture 	(USER.236)
 */
HWND WINAPI GetCapture()
{
    return captureWnd;
}

