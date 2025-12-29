#include <user.h>

/***********************************************************************
 *		keybd_event (USER.???)
 */
int WINAPI keybd_event(VOID)
{
	FUNCTION_START
	// send event to System Message Queue
	FUNCTION_END
}

/***********************************************************************
 *		mouse_event (USER.299)
 */
int WINAPI mouse_event(VOID)
{
	FUNCTION_START
	// send event to System Message Queue
	FUNCTION_END
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
