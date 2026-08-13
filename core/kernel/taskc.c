#include <windows.h>
#include <win_private.h>

static int nTaskCount = 0;

NE_MODULE FAR *NE_GetPtr( HMODULE hModule );
HQUEUE WINAPI GetTaskQueue( HTASK hTask );

/***********************************************************************
 *           TASK_LinkTask
 */
static void TASK_LinkTask( HTASK hTask )
{
    HTASK *prevTask;
    TDB far *pTask;

	FUNCTIONSTART;

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

	FUNCTIONSTART;

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
	FUNCTIONEND;
}

/***********************************************************************
 *           Yield  (KERNEL.29)
 */
void WINAPI Yield(void)
{
    TDB far *pCurTask = MAKELP(GetCurrentTask(), 0);

	FUNCTIONSTART;

    pCurTask->hYieldTo=0;

    if (pCurTask->hQueue)
    {
        HMODULE mod = GetModuleHandle( "user.dll" );
        if (mod)
        {
            void (WINAPI far *pUserYield)(void);
            pUserYield = (void far *)GetProcAddress( mod, "#332" ); // UserYield()
            if (pUserYield)
            {
                pUserYield();
                return;
            }
        }
    } 
    OldYield();
	FUNCTIONEND;
}


BOOL WINAPI IsTask(HTASK w)
{
  WORD far * lpwMaybeTask;

	FUNCTIONSTART;

  if (!w)
    return FALSE;

  if (GetSelectorLimit(w) < (OFS_TD_SIGN+2))
    return FALSE;

  lpwMaybeTask=(WORD far *) MAKELP(w, OFS_TD_SIGN);

	FUNCTIONEND;

  return (*lpwMaybeTask == TD_SIGN);

}

/***********************************************************************
 *           SetPriority  (KERNEL.32)
 */
void WINAPI SetPriority( HTASK hTask, int delta )
{
    TDB far *pTask;
    int newpriority;

	FUNCTIONSTART;

    if (!hTask) hTask = GetCurrentTask();
    if (!(pTask = MAKELP( hTask,0 ))) return;
    newpriority = pTask->priority + delta;
    if (newpriority < -32) newpriority = -32;
    else if (newpriority > 15) newpriority = 15;

    pTask->priority = newpriority + 1;
    TASK_UnlinkTask( pTask->hSelf );
    TASK_LinkTask( pTask->hSelf );
    pTask->priority--;
	FUNCTIONEND;
}

/***********************************************************************
 *           GetNumTasks   (KERNEL.152)
 */
UINT WINAPI GetNumTasks(void)
{
	FUNCTIONSTART;
	FUNCTIONEND;
    return nTaskCount;
}

/***********************************************************************
 *           GetInstanceData   (KERNEL.54)
 */
int WINAPI GetInstanceData( HINSTANCE instance, BYTE NEAR * buffer, int len )
{
    LPSTR ptr;
    LPSTR ptr1;

	FUNCTIONSTART;
    ptr = GlobalLock(instance);

    if (!ptr || !len) return 0;
    if (((DWORD)buffer + len) >= 0x10000) len = 0x10000 - (WORD)buffer;
    LongPtrAdd((DWORD)ptr, (DWORD)buffer);
    ptr1=GlobalLock(GetDS());
    LongPtrAdd((DWORD)ptr1, (DWORD)buffer);
    memcpy( ptr1, ptr, len );
	FUNCTIONEND;
    return len;
}

/***********************************************************************
 *           LockCurrentTask  (KERNEL.33)
 */
HTASK WINAPI LockCurrentTask( BOOL bLock )
{
	FUNCTIONSTART;
    if (bLock) TH_LOCKTDB = GetCurrentTask();
    else TH_LOCKTDB = 0;
	FUNCTIONEND;
    return TH_LOCKTDB;
}

/***********************************************************************
 *           PostEvent  (KERNEL.31)
 */
void WINAPI PostEvent( HTASK hTask )
{
    TDB far *pTask;

	FUNCTIONSTART;

    if (!hTask) hTask = GetCurrentTask();
    if (!(pTask = MAKELP( hTask, 0 ))) return;

    pTask->nEvents++;
	FUNCTIONEND;
}

/***********************************************************************
 *           SetTaskSignalProc   (KERNEL.38)
 */
FARPROC WINAPI SetTaskSignalProc( HTASK hTask, FARPROC proc )
{
    TDB far *pTask;
    FARPROC oldProc;

	FUNCTIONSTART;

    if (!hTask) hTask = GetCurrentTask();
    if (!(pTask = MAKELP( hTask, 0 ))) return NULL;
    oldProc = pTask->userhandler;
    pTask->userhandler = proc;
	FUNCTIONEND;
    return oldProc;
}

/***********************************************************************
 *           GetTaskDS   (KERNEL.155)
 *
 * Note: this function apparently returns a DWORD with LOWORD == HIWORD.
 * I don't think we need to bother with this.
 */
HINSTANCE WINAPI GetTaskDS(void)
{
    TDB far *pTask;

	FUNCTIONSTART;

    if (!(pTask = MAKELP(GetCurrentTask(),0))) return 0;

	FUNCTIONEND;

    return pTask->hInstance;
}

/***********************************************************************
 *           GetCurPID   (KERNEL.157)
 */
DWORD WINAPI GetCurPID( DWORD unused )
{
	FUNCTIONSTART;
	FUNCTIONEND;
    return 0;
}

/**********************************************************************
 *	    TASK_GetCodeSegment
 *
 * Helper function for GetCodeHandle/GetCodeInfo: Retrieve the module
 * and logical segment number of a given code segment.
 *
 * 'proc' either *is* already a pair of module handle and segment number,
 * in which case there's nothing to do.  Otherwise, it is a pointer to
 * a function, and we need to retrieve the code segment.  If the pointer
 * happens to point to a thunk, we'll retrieve info about the code segment
 * where the function pointed to by the thunk resides, not the thunk itself.
 *
 * FIXME: if 'proc' is a SNOOP16 return stub, we should retrieve info about
 *        the function the snoop code will return to ...
 *
 */
static BOOL TASK_GetCodeSegment( FARPROC proc, NE_MODULE far **ppModule,
                                 SEGTABLEENTRY FAR **ppSeg, int *pSegNr )
{
    NE_MODULE far *pModule = NULL;
    SEGTABLEENTRY *pSeg = NULL;
    int segNr=0;

	FUNCTIONSTART;

    /* Try pair of module handle / segment number */
    pModule = (NE_MODULE far *)GlobalLock( HIWORD( proc ) );
    if ( pModule && pModule->ne_magic == IMAGE_OS2_SIGNATURE )
    {
        segNr = LOWORD( proc );
        if ( segNr && segNr <= pModule->ne_cseg )
            pSeg = NE_SEG_TABLE( pModule ) + segNr-1;
    }

    /* Try thunk or function */
    else
    {
        BYTE far *thunk = (BYTE far *)proc;
        WORD selector;

        if ((thunk[0] == 0xb8) && (thunk[3] == 0xea))
            selector = thunk[6] + (thunk[7] << 8);
        else
            selector = HIWORD( proc );

        pModule = NE_GetPtr( GlobalHandle( selector ) );
        pSeg = pModule? NE_SEG_TABLE( pModule ) : NULL;

        if ( pModule )
            for ( segNr = 1; segNr <= pModule->ne_cseg; segNr++, pSeg++ )
                if ( GlobalHandleToSel(pSeg->hSeg) == selector )
                    break;

        if ( pModule && segNr > pModule->ne_cseg )
            pSeg = NULL;
    }

    /* Abort if segment not found */

    if ( !pModule || !pSeg )
        return FALSE;

    /* Return segment data */

    if ( ppModule ) *ppModule = pModule;
    if ( ppSeg    ) *ppSeg    = pSeg;
    if ( pSegNr   ) *pSegNr   = segNr;

	FUNCTIONEND;
    return TRUE;
}

/**********************************************************************
 *	    GetCodeHandle    (KERNEL.93)
 */
HGLOBAL WINAPI GetCodeHandle( FARPROC proc )
{
    SEGTABLEENTRY FAR *pSeg;

	FUNCTIONSTART;

    if ( !TASK_GetCodeSegment( proc, NULL, &pSeg, NULL ) )
        return 0;

	FUNCTIONEND;

    return MAKELONG( pSeg->hSeg, GlobalHandleToSel(pSeg->hSeg) );
}

/**********************************************************************
 *	    GetCodeInfo    (KERNEL.104)
 */
//@todo check Wine returns BOOL but in Watcem headers is void
void WINAPI GetCodeInfo( FARPROC proc, SEGINFO far *segInfo )
{
    NE_MODULE far *pModule;
    SEGTABLEENTRY FAR *pSeg;
    int segNr;

	FUNCTIONSTART;

    if ( !TASK_GetCodeSegment( proc, &pModule, &pSeg, &segNr ) )
        return /*FALSE*/;

    /* Fill in segment information */

    segInfo->offSegment = pSeg->filepos;
    segInfo->cbSegment  = pSeg->size;
    segInfo->flags      = pSeg->flags;
    segInfo->cbAlloc    = pSeg->minsize;
    segInfo->h          = pSeg->hSeg;
    segInfo->alignShift = pModule->ne_align;

    if ( segNr == pModule->ne_autodata )
        segInfo->cbAlloc += pModule->ne_heap + pModule->ne_stack;

    /* Return module handle in %es */
//    SetES(GlobalHandleToSel( pModule->self ));
    SetES(SELECTOROF(pModule));

	FUNCTIONEND;
//    return TRUE;
}


/***********************************************************************
 *           GetTaskQueueDS  (KERNEL.118)
 */
void WINAPI GetTaskQueueDS(void)
{
	FUNCTIONSTART;
    SetDS(GlobalHandleToSel(GetTaskQueue(0)));
	FUNCTIONEND;
}


/***********************************************************************
 *           GetTaskQueueES  (KERNEL.119)
 */
void WINAPI GetTaskQueueES(void)
{
	FUNCTIONSTART;
    SetES(GlobalHandleToSel(GetTaskQueue(0)));
	FUNCTIONEND;
}

/***********************************************************************
 *           GetCurrentPDB   (KERNEL.37)
 *
 * UNDOC: returns PSP of KERNEL in high word
 */
DWORD WINAPI GetCurrentPDBReal(void)
{
	TDB FAR *pTask;

	FUNCTIONSTART;

	if (!(pTask = MAKELP(GetCurrentTask(), 0))) return 0;

	FUNCTIONEND;

	return MAKELONG(pTask->hPDB, TH_TOPPDB);
}

/***********************************************************************
 *           GetDOSEnvironment     (KERNEL.131)
 *
 * FIXME!!! Here we must get environment of current task...
 *
 * Format of a 16-bit environment block:
 * ASCIIZ   string 1 (xx=yy format)
 * ...
 * ASCIIZ   string n
 * BYTE     0
 * WORD     1
 * ASCIIZ   program name (e.g. C:\WINDOWS\SYSTEM\KRNL386.EXE)
 */
LPSTR WINAPI GetDOSEnvironment(void)
{
	LPSTR lpEnv;

	FUNCTIONSTART;

	lpEnv=MAKELP(LOWORD(GetCurrentPDBReal()), 0x0080);

	FUNCTIONEND;

	return MAKELP(*lpEnv,0);
}

#if 0
GetDOSEnvironment proc far pascal uses es
	invoke GetCurrentPDBReal
	mov es,ax
	mov dx,es:[ENVIRON]
	xor ax,ax
	ret
GetDOSEnvironment endp
#endif


/***********************************************************************
 *           TASK_CreateThunks
 *
 * Create a thunk free-list in segment 'handle', starting from offset 'offset'
 * and containing 'count' entries.
 */
static void TASK_CreateThunks( HGLOBAL handle, WORD offset, WORD count )
{
    int i;
    THUNKS FAR *pThunk;

	FUNCTIONSTART;

    pThunk = (THUNKS FAR *)((BYTE FAR *)GlobalLock( handle ) + offset);
    pThunk->next = 0;
    pThunk->magic = THUNK_MAGIC;
    pThunk->free = FIELDOFFSET( THUNKS, thunks );
    for (i = 0; i < count-1; i++)
        *(WORD FAR *)&pThunk->thunks[i] = FIELDOFFSET( THUNKS, thunks[i+1] );
    *(WORD FAR *)&pThunk->thunks[i] = 0;  /* Last thunk */
	FUNCTIONEND;
}


/***********************************************************************
 *           TASK_AllocThunk
 *
 * Allocate a thunk for MakeProcInstance().
 */
static void far * TASK_AllocThunk(void)
{
    TDB far *pTask;
    THUNKS far *pThunk;
    WORD sel, base;

	FUNCTIONSTART;

    if (!(pTask = MAKELP(GetCurrentTask(), 0))) return 0;
    sel = pTask->hCSAlias;
    pThunk = (THUNKS FAR *)pTask->thunks;
    base = (char FAR *)pThunk - (char FAR *)pTask;
    while (!pThunk->free)
    {
        sel = pThunk->next;
        if (!sel)  /* Allocate a new segment */
        {
            sel = GlobalAlloc( GMEM_FIXED, FIELDOFFSET( THUNKS, thunks[MIN_THUNKS] )
                              /*  pTask->hPDB, LDT_FLAGS_CODE */ ); // @todo fix owner and code flag!!!
            if (!sel) return 0;
            TASK_CreateThunks( sel, 0, MIN_THUNKS );
            pThunk->next = sel;
        }
        pThunk = (THUNKS far *)GlobalLock( sel );
        base = 0;
    }
    base += pThunk->free;
    pThunk->free = *(WORD FAR *)((BYTE FAR *)pThunk + pThunk->free);
	FUNCTIONEND;
    return MAKELP( sel, base );
}


/***********************************************************************
 *           MakeProcInstance  (KERNEL.51)
 */
FARPROC WINAPI MakeProcInstance( FARPROC func, HANDLE hInstance )
{
    struct thunk far *thunk;
    BYTE far *lfunc;
    void far * thunkaddr;
    WORD hInstanceSelector;

	FUNCTIONSTART;

    hInstanceSelector = GlobalHandleToSel(hInstance);

//    TRACE("(%p, %04x);\n", func, hInstance);

    if (!HIWORD(func)) {
      /* Win95 actually protects via SEH, but this is better for debugging */
//      WARN("Ouch ! Called with invalid func %p !\n", func);
      return NULL;
    }

    if ( (GlobalHandleToSel(GetDS()) != hInstanceSelector)
      && (hInstance != 0)
      && (hInstance != 0xffff) )
    {
	/* calling MPI with a foreign DSEG is invalid ! */
//        WARN("Problem with hInstance? Got %04x, using %04x instead\n",
//                   hInstance,CURRENT_DS);
    }

    /* Always use the DSEG that MPI was entered with.
     * We used to set hInstance to GetTaskDS16(), but this should be wrong
     * as CURRENT_DS provides the DSEG value we need.
     * ("calling" DS, *not* "task" DS !) */
    hInstanceSelector = GetDS();
    hInstance = GlobalHandle(hInstanceSelector);

    /* no thunking for DLLs */
    if (NE_GetPtr(FarGetOwner(hInstance))->ne_flags & NE_FFLAGS_LIBMODULE)
	return func;

    thunkaddr = TASK_AllocThunk();
    if (!thunkaddr) return NULL;
    thunk =  thunkaddr ;
    lfunc =  (BYTE far *)func ;

//    TRACE("(%p,%04x): got thunk %08lx\n", func, hInstance, thunkaddr );
    if (((lfunc[0]==0x8c) && (lfunc[1]==0xd8)) || /* movw %ds, %ax */
    	((lfunc[0]==0x1e) && (lfunc[1]==0x58))    /* pushw %ds, popw %ax */
    ) {
//    	WARN("This was the (in)famous \"thunk useless\" warning. We thought we have to overwrite with nop;nop;, but this isn't true.\n");
    }

    thunk->movw     = 0xb8;    /* movw instance, %ax */
    thunk->instance = hInstanceSelector;
    thunk->ljmp     = 0xea;    /* ljmp func */
    thunk->func     = func;
	FUNCTIONEND;
    return (FARPROC)thunkaddr;
    /* CX reg indicates if thunkaddr != NULL, implement if needed */
}


/***********************************************************************
 *           FreeProcInstance  (KERNEL.52)
 */
void WINAPI FreeProcInstance( FARPROC func )
{
    TDB far *pTask;
    THUNKS far *pThunk;
    WORD sel, base;

	FUNCTIONSTART;

//    TRACE("(%p)\n", func );

    if (!(pTask = MAKELP(GetCurrentTask(), 0))) return;
    sel = pTask->hCSAlias;
    pThunk = (THUNKS FAR *)pTask->thunks;
    base = (char FAR *)pThunk - (char FAR *)pTask;
    while (sel && (sel != HIWORD(func)))
    {
        sel = pThunk->next;
        pThunk = (THUNKS far *)GlobalLock( sel );
        base = 0;
    }
    if (!sel) return;
    *(WORD FAR *)((BYTE FAR *)pThunk + LOWORD(func) - base) = pThunk->free;
    pThunk->free = LOWORD(func) - base;
	FUNCTIONEND;
}

#if 0
LoadModule proc far pascal uses ds lpszModuleName:far ptr byte, lpParameterBlock:far ptr
	@SetKernelDS
	mov [fLoadMod],1	;use a asciiz command line
	lds dx, lpszModuleName
	les bx, lpParameterBlock
	@Exec
	@SetKernelDS
	mov [fLoadMod],0
	ret
LoadModule endp
#endif

/**********************************************************************
 *          LoadModule      (KERNEL.45)
 *
 * Note: HX-DOS expects in parameter block ASCIIZ cmdline. But not found (yet)
 * any real example of such paramblock. So fLoadMod flag removed. (May be in WinExec???)
 */
HINSTANCE WINAPI LoadModule(LPCSTR lpszModuleName, LPVOID lpParameterBlock)
{
	FUNCTIONSTART;
	SaveDS();
	_asm {
		lds dx, lpszModuleName
		les bx, lpParameterBlock
		mov ax, 4b00h
	}
	Dos3Call();
	RestoreDS();
	FUNCTIONEND;
}
