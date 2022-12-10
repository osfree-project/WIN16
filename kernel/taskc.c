#include <win16.h>
#include <win_private.h>

#define QS_SENDMESSAGE	0x0040
#define PM_QS_SENDMESSAGE (QS_SENDMESSAGE << 16)

/***********************************************************************
 *           Yield  (KERNEL.29)
 */
// @todo check it with Matt Pietrek info. Direct port from Wine seems to be not correct...
void WINAPI Yield(void)
{
    TDB far *pCurTask = MAKELP(GetCurrentTask(), 0);

    if (pCurTask && pCurTask->hQueue)
    {
        HMODULE mod = GetModuleHandle( "user.dll" );
        if (mod)
        {
            BOOL (WINAPI *pPeekMessage)( MSG *msg, HWND hwnd, UINT first, UINT last, UINT flags );
            pPeekMessage = (void far *)GetProcAddress( mod, "PeekMessage" );
            if (pPeekMessage)
            {
                MSG msg;
                pPeekMessage( &msg, 0, 0, 0, PM_REMOVE | PM_QS_SENDMESSAGE );
                return;
            }
        }
    }
// @todo Implement OldYield
//    OldYield();
}
