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

#if 0
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
	 int 	hMouse; 	//  Horizontal position.
	 int 	vMouse;		//  Vertical position.
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

    if (EventCodes & ME_MOVE)
    {
	int maxX;
	int maxY;

	iMouseX+=hMouse;
	iMouseY+=vMouse;

	// Проверить границы
	maxX = GetSystemMetrics(SM_CXSCREEN) - 1;
	maxY = GetSystemMetrics(SM_CYSCREEN) - 1;
    
	if (iMouseX < 0) iMouseX = 0;
	if (iMouseX > maxX) iMouseX = maxX;
	if (iMouseY < 0) iMouseY = 0;
	if (iMouseY > maxY) iMouseY = maxY;

	MoveCursor(iMouseX, iMouseY);
	hardware_event( WM_MOUSEMOVE, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0 );
//По идее, лучше не выходить, а обработать другие события тоже.
//	PopDS();
//	return;
    }
    if (EventCodes & ME_LDOWN)
        hardware_event( WM_LBUTTONDOWN, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);

    if (EventCodes & ME_LUP)
        hardware_event( WM_LBUTTONUP, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);

    if (EventCodes & ME_RDOWN)
        hardware_event( WM_RBUTTONDOWN, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);

    if (EventCodes & ME_RUP)
        hardware_event( WM_RBUTTONUP, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);

#if 0

Old WIN 3.x constants
SF_MOVEMENT	equ	0001h		;Movement occured
SF_B1_DOWN	equ	0002h		;Button 1 (SW1) changed to down
SF_B1_UP	equ	0004h		;Button 1 (SW1) changed to up
SF_B2_DOWN	equ	0008h		;Button 2 (SW3) changed to down
SF_B2_UP	equ	0010h		;Button 2 (SW3) changed to up
SF_ABSOLUTE	equ	8000h		;BX,CX are normalized absolute coords

New Win32 constants

#define WM_MOUSEFIRST               0x0200
#define WM_MOUSEMOVE                0x0200
#define WM_LBUTTONDOWN              0x0201
#define WM_LBUTTONUP                0x0202
#define WM_LBUTTONDBLCLK            0x0203
#define WM_RBUTTONDOWN              0x0204
#define WM_RBUTTONUP                0x0205
#define WM_RBUTTONDBLCLK            0x0206
#define WM_MBUTTONDOWN              0x0207
#define WM_MBUTTONUP                0x0208
#define WM_MBUTTONDBLCLK            0x0209
#define WM_MOUSELAST                0x0209

#endif
  
	PopDS();
//	FUNCTION_END
}
#endif

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
    int  hMouse;
    int  vMouse;
    WORD NumButts;
    WORD KeyState = 0;
    static int callCount = 0;
    
    // Получаем значения из регистров
    _asm{
        mov EventCodes, AX 
        mov hMouse, BX
        mov vMouse, CX
        mov NumButts, DX
    }

    PushDS();
    SetDS(USER_HeapSel);
    
    // Увеличиваем счетчик вызовов
//    callCount++;
    
    // Подробный вывод отладки
//    TRACE("========== mouse_event #%d ==========", callCount);
//    TRACE("EventCodes (AX) = 0x%04X", EventCodes);
//    TRACE("hMouse (BX) = %d", hMouse);
//    TRACE("vMouse (CX) = %d", vMouse);
//    TRACE("NumButts (DX) = 0x%04X", NumButts);
    
    // Расшифровка EventCodes
//    TRACE("Event breakdown:");
//    if (EventCodes & ME_MOVE)       TRACE("  ME_MOVE (0x0001) - Mouse moved");
//    if (EventCodes & ME_LDOWN)      TRACE("  ME_LDOWN (0x0002) - Left button down");
//    if (EventCodes & ME_LUP)        TRACE("  ME_LUP (0x0004) - Left button up");
//    if (EventCodes & ME_RDOWN)      TRACE("  ME_RDOWN (0x0008) - Right button down");
//    if (EventCodes & ME_RUP)        TRACE("  ME_RUP (0x0010) - Right button up");
//    if (EventCodes & ME_ABSOLUTE)   TRACE("  ME_ABSOLUTE (0x8000) - Absolute coordinates");
//    if (EventCodes == 0)            TRACE("  NO EVENTS (0x0000)");
    
    // Текущая позиция мыши до обработки
//    TRACE("Current mouse position before: X=%d, Y=%d", iMouseX, iMouseY);
    
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
        int maxX, maxY;
        
        // Отладка перемещения
//        TRACE("Processing ME_MOVE:");
//        TRACE("  hMouse delta: %d", hMouse);
//        TRACE("  vMouse delta: %d", vMouse);
        
        // Обновляем позицию
        iMouseX += hMouse;
        iMouseY += vMouse;
        
//        TRACE("  New position (before bounds check): X=%d, Y=%d", iMouseX, iMouseY);

        // Проверить границы
        maxX = CXScreen/*GetSystemMetrics(SM_CXSCREEN)*/ - 1;
        maxY = CYScreen/*GetSystemMetrics(SM_CYSCREEN)*/ - 1;
        
//        TRACE("  Screen bounds: X:0-%d, Y:0-%d", maxX, maxY);
    
        if (iMouseX < 0) {
//            TRACE("  X < 0, clamping to 0");
            iMouseX = 0;
        }
        if (iMouseX > maxX) {
//            TRACE("  X > maxX, clamping to %d", maxX);
            iMouseX = maxX;
        }
        if (iMouseY < 0) {
//            TRACE("  Y < 0, clamping to 0");
            iMouseY = 0;
        }
        if (iMouseY > maxY) {
//            TRACE("  Y > maxY, clamping to %d", maxY);
            iMouseY = maxY;
        }
        
//        TRACE("  Final position: X=%d, Y=%d", iMouseX, iMouseY);
        
        // Отладка MoveCursor
//        TRACE("  Calling MoveCursor(%d, %d)", iMouseX, iMouseY);
        MoveCursor(iMouseX, iMouseY);
        
        // Отладка hardware_event
//        TRACE("  Calling hardware_event: WM_MOUSEMOVE, KeyState=0x%04X, pos=(%d,%d)", 
//              KeyState, iMouseX, iMouseY);
        hardware_event(WM_MOUSEMOVE, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);
    }
    
    // Обработка нажатий/отпусканий кнопок
    if (EventCodes & ME_LDOWN) {
//        TRACE("Processing ME_LDOWN, calling hardware_event: WM_LBUTTONDOWN");
        hardware_event(WM_LBUTTONDOWN, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);
    }
    
    if (EventCodes & ME_LUP) {
//        TRACE("Processing ME_LUP, calling hardware_event: WM_LBUTTONUP");
        hardware_event(WM_LBUTTONUP, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);
    }
    
    if (EventCodes & ME_RDOWN) {
//        TRACE("Processing ME_RDOWN, calling hardware_event: WM_RBUTTONDOWN");
        hardware_event(WM_RBUTTONDOWN, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);
    }
    
    if (EventCodes & ME_RUP) {
//        TRACE("Processing ME_RUP, calling hardware_event: WM_RBUTTONUP");
        hardware_event(WM_RBUTTONUP, KeyState, 0, iMouseX, iMouseY, GetTickCount(), 0);
    }
    
//    TRACE("========== End of mouse_event #%d ==========\n", callCount);
    
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

