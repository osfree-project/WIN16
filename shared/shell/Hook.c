#include "Shell.h"



/*************************************************************************
 * ShellHookProc		[SHELL.103]
 * System-wide WH_SHELL hook.
 */
LRESULT CALLBACK ShellHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (SHELL_hWnd)
    {
        switch( code )
        {
        case HSHELL_WINDOWCREATED:
            PostMessage( SHELL_hWnd, uMsgWndCreated, wParam, 0 );
            break;
        case HSHELL_WINDOWDESTROYED:
            PostMessage( SHELL_hWnd, uMsgWndDestroyed, wParam, 0 );
            break;
        case HSHELL_ACTIVATESHELLWINDOW:
            PostMessage( SHELL_hWnd, uMsgShellActivate, wParam, 0 );
            break;
        }
    }
    return CallNextHookEx( SHELL_hHook, code, wParam, lParam );
}


/*************************************************************************
 * RegisterShellHook	[SHELL.102]
 *

From xoblite:
		RegisterShellHook(NULL, true);

                if(osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                        RegisterShellHook(hMainWnd, 1);
                else
                        RegisterShellHook(hMainWnd, 3);

-----------
        if (RegisterShellHook) RegisterShellHook(hMainWnd, 0);


 */


BOOL WINAPI RegisterShellHook(HWND hWnd, UINT uAction)
{
	MessageBox(0, "RegisterShellHook", "RegisterShellHook", MB_OK);
    //APISTR((LF_APISTUB,"RegisterShellHook(HWND=%x,int=%d)\n",hWnd,foo));

    switch( uAction )
    {
    case RSH_DEREGISTER:
        if (SHELL_hHook)
        {
		UnhookWindowsHook(WH_SHELL, (HOOKPROC)SHELL_lpProc);
		FreeProcInstance(SHELL_lpProc);
		SHELL_hHook = 0;
		return TRUE;
        }
	break;
    case RSH_REGISTER:
        break;
    case RSH_REGISTER_PROGMAN:  /* register hWnd as a shell window */
        if( !SHELL_hHook )
        {
		SHELL_lpProc= MakeProcInstance((FARPROC)ShellHookProc, Globals.hInstance);
            SHELL_hHook = SetWindowsHookEx( WH_SHELL, SHELL_lpProc, Globals.hInstance, 0 );
            if ( SHELL_hHook )
            {
                uMsgWndCreated = RegisterWindowMessage( lpstrMsgWndCreated );
                uMsgWndDestroyed = RegisterWindowMessage( lpstrMsgWndDestroyed );
                uMsgShellActivate = RegisterWindowMessage( lpstrMsgShellActivate );
            }
            else  
                MessageBox(0, "-- unable to install ShellHookProc()!", "", MB_OK);
        }

        if ( SHELL_hHook )
            return ((SHELL_hWnd = hWnd) != 0);
        break;

    case RSH_REGISTER_TASKMAN:
        break;

    default:
//        WARN("-- unknown code %i\n", uAction );
        SHELL_hWnd = 0; /* just in case */
    }
    return FALSE;
}
