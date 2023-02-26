#include <windows.h>

/***********************************************************************
 *		OldExitWindows (USER.2)
 */
void WINAPI
OldExitWindows(void)
{
        ExitWindows(0,0);
}
