#include <windows.h>

/***********************************************************************
 *		mouse_event (USER.299)
 */
void WINAPI mouse_event(VOID)
{
//    mouse_event( LOWORD(context->Eax), LOWORD(context->Ebx), LOWORD(context->Ecx),
//                 LOWORD(context->Edx), MAKELONG(context->Esi, context->Edi) );
}

/***********************************************************************
 *		GetMouseEventProc (USER.337)
 */
FARPROC WINAPI GetMouseEventProc(void)
{
    HMODULE hmodule = GetModuleHandle("USER");
    return GetProcAddress( hmodule, "mouse_event" );
}

/***********************************************************************
 *		IsUserIdle (USER.333)
 */
BOOL WINAPI IsUserIdle(void)
{
    if ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
        return FALSE;
    if ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 )
        return FALSE;
    if ( GetAsyncKeyState( VK_MBUTTON ) & 0x8000 )
        return FALSE;
    /* Should check for screen saver activation here ... */
    return TRUE;
}
