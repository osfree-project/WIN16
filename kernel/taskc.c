#include <win16.h>
#include <win_private.h>

#define QS_SENDMESSAGE	0x0040
#define PM_QS_SENDMESSAGE (QS_SENDMESSAGE << 16)

extern HTASK pascal TH_HEADTDB;
extern HTASK pascal TH_LOCKTDB;

static int nTaskCount = 0;

void memcpy(void far * s1, void far * s2, unsigned length);
void WINAPI LongPtrAdd(DWORD dwLongPtr, DWORD dwAdd);

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

/***********************************************************************
 *           TASK_LinkTask
 */
static void TASK_LinkTask( HTASK hTask )
{
    HTASK *prevTask;
    TDB far *pTask;

    if (!(pTask = MAKELP( hTask , 0))) return;
    prevTask = &TH_HEADTDB;
    while (*prevTask)
    {
        TDB far *prevTaskPtr = MAKELP( *prevTask, 0 );
        if (prevTaskPtr->priority >= pTask->priority) break;
        prevTask = &prevTaskPtr->hNext;
    }
    pTask->hNext = *prevTask;
    *prevTask = hTask;
    nTaskCount++;
}

/***********************************************************************
 *           TASK_UnlinkTask
 */
static void TASK_UnlinkTask( HTASK hTask )
{
    HTASK *prevTask;
    TDB far *pTask;

    prevTask = &TH_HEADTDB;
    while (*prevTask && (*prevTask != hTask))
    {
        pTask = MAKELP( *prevTask, 0 );
        prevTask = &pTask->hNext;
    }
    if (*prevTask)
    {
        pTask = MAKELP( *prevTask, 0 );
        *prevTask = pTask->hNext;
        pTask->hNext = 0;
        nTaskCount--;
    }
}

/***********************************************************************
 *           Yield  (KERNEL.29)
 */

void WINAPI Yield(void)
{
    TDB far *pCurTask = MAKELP(GetCurrentTask(), 0);

    pCurTask->hYieldTo=0;

    if (pCurTask->hQueue)
    {
        HMODULE mod = GetModuleHandle( "user.dll" );
        if (mod)
        {
            void (WINAPI *pUserYield)(void);
            pUserYield = (void far *)GetProcAddress( mod, "#332" ); // UserYield()
            if (pUserYield)
            {
                pUserYield();
                return;
            }
        }
    } 
// @todo Implement OldYield
//    OldYield();
}

#define TD_SIGN 0x4454   /* "TD" = Task Database */
#define OFS_TD_SIGN 0xFA /* location of "TD" signature in Task DB */

BOOL WINAPI IsTask(HTASK w)
{
  WORD far * lpwMaybeTask;

  if (!w)
    return FALSE;

  if (GetSelectorLimit(w) < (OFS_TD_SIGN+2))
    return FALSE;

  lpwMaybeTask=(WORD far *) MAKELP(w, OFS_TD_SIGN);

  return (*lpwMaybeTask == TD_SIGN);

}

/***********************************************************************
 *           SetPriority  (KERNEL.32)
 */
void WINAPI SetPriority( HTASK hTask, int delta )
{
    TDB far *pTask;
    int newpriority;

    if (!hTask) hTask = GetCurrentTask();
    if (!(pTask = MAKELP( hTask,0 ))) return;
    newpriority = pTask->priority + delta;
    if (newpriority < -32) newpriority = -32;
    else if (newpriority > 15) newpriority = 15;

    pTask->priority = newpriority + 1;
    TASK_UnlinkTask( pTask->hSelf );
    TASK_LinkTask( pTask->hSelf );
    pTask->priority--;
}

/***********************************************************************
 *           GetNumTasks   (KERNEL.152)
 */
UINT WINAPI GetNumTasks(void)
{
    return nTaskCount;
}

/***********************************************************************
 *           GetInstanceData   (KERNEL.54)
 */
int WINAPI GetInstanceData( HINSTANCE instance, BYTE NEAR * buffer, int len )
{
    char far *ptr = GlobalLock( instance );
    char far *ptr1;
    if (!ptr || !len) return 0;
    if (((WORD)buffer + len) >= 0x10000) len = 0x10000 - (WORD)buffer;
    LongPtrAdd((DWORD)ptr, (DWORD)buffer);
    ptr1=GlobalLock(GetDS());
    LongPtrAdd((DWORD)ptr1, (DWORD)buffer);
    memcpy( ptr1, ptr, len );
    return len;
}

/***********************************************************************
 *           LockCurrentTask  (KERNEL.33)
 */
HTASK WINAPI LockCurrentTask( BOOL bLock )
{
    if (bLock) TH_LOCKTDB = GetCurrentTask();
    else TH_LOCKTDB = 0;
    return TH_LOCKTDB;
}

/***********************************************************************
 *           PostEvent  (KERNEL.31)
 */
void WINAPI PostEvent( HTASK hTask )
{
    TDB far *pTask;
    if (!hTask) hTask = GetCurrentTask();
    if (!(pTask = MAKELP( hTask, 0 ))) return;

    pTask->nEvents++;
}

/***********************************************************************
 *           SetTaskSignalProc   (KERNEL.38)
 */
FARPROC WINAPI SetTaskSignalProc( HTASK hTask, FARPROC proc )
{
    TDB far *pTask;
    FARPROC oldProc;

    if (!hTask) hTask = GetCurrentTask();
    if (!(pTask = MAKELP( hTask, 0 ))) return NULL;
    oldProc = pTask->userhandler;
    pTask->userhandler = proc;
    return oldProc;
}
