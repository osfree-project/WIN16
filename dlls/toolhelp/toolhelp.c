/*
 * Toolhelp functions
 *
 * Copyright 1996 Marcus Meissner
 * Copyright 2009 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <dos.h>

#include <windows.h>

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

#include "toolhelp.h"

#pragma pack(push,1)

typedef struct
{
    void     *base;          /* Base address (0 if discarded) */
    DWORD     size;          /* Size in bytes (0 indicates a free block) */
    HGLOBAL   handle;        /* Handle for this block */
    HGLOBAL   hOwner;        /* Owner of this block */
    BYTE      lockCount;     /* Count of GlobalFix() calls */
    BYTE      pageLockCount; /* Count of GlobalPageLock() calls */
    BYTE      flags;         /* Allocation flags */
    BYTE      selCount;      /* Number of selectors allocated for this block */
} GLOBALARENA;

#define GLOBAL_MAX_COUNT  8192        /* Max number of allocated blocks */

/*
   Wine doesn't support BURGERMASTER and uses GLOBALARENA as BURGERMASTER.
   THHOOK hGlobalHeap and pGlobalHeap contains pointer to first GLOBALARENA
   structure intead of handle and selector to BURGERMASTER.
   
*/

typedef struct tagBURGERMASTER_KRNL286
{
    BOOL bCheck;
    BOOL bFreeze;
    WORD wEntries;
    WORD nwFirst;
    WORD nwLast;
    BYTE byComp;
    BYTE byDestrLev;
    WORD nwDestrBytes;
    HANDLE hTable;
    HANDLE hFree;
    WORD wDelta;
    WORD pExpand;
    WORD pStats;
    WORD wLRUAcess;
    WORD wLRUChain;
    WORD wLRUCount;
    WORD nwDestrPara;
    WORD nwDestrChain;
    WORD wFree;
} BURGERMASTER_KRNL286;
typedef BURGERMASTER_KRNL286 FAR * LPBURGERMASTER_KRNL286;

typedef struct tagBURGERMASTER_KRNL386
{
    BOOL bCheck;
    BOOL bFreeze;
    WORD wEntries;
    DWORD nwFirst;
    DWORD nwLast;
    BYTE byComp;
    BYTE byDestrLev;
    DWORD nwDestrBytes;
    WORD reserved[5];
    WORD wLRUAcess;
    DWORD dwLRUChain;
    WORD wLRUCount;
    DWORD nwDestrPara;
    DWORD nwDestrChain;
    WORD wFree;
} BURGERMASTER_KRNL386;
typedef BURGERMASTER_KRNL386 FAR * LPBURGERMASTER_KRNL386;

typedef struct
{
    WORD check;                 /* 00 Heap checking flag */
    WORD freeze;                /* 02 Heap frozen flag */
    WORD items;                 /* 04 Count of items on the heap */
    WORD first;                 /* 06 First item of the heap */
    WORD pad1;                  /* 08 Always 0 */
    WORD last;                  /* 0a Last item of the heap */
    WORD pad2;                  /* 0c Always 0 */
    BYTE ncompact;              /* 0e Compactions counter */
    BYTE dislevel;              /* 0f Discard level */
    DWORD distotal;             /* 10 Total bytes discarded */
    WORD htable;                /* 14 Pointer to handle table */
    WORD hfree;                 /* 16 Pointer to free handle table */
    WORD hdelta;                /* 18 Delta to expand the handle table */
    WORD expand;                /* 1a Pointer to expand function (unused) */
    WORD pstat;                 /* 1c Pointer to status structure (unused) */
    FARPROC   notify;           /* 1e Pointer to LocalNotify() function */
    WORD lock;                  /* 22 Lock count for the heap */
    WORD extra;                 /* 24 Extra bytes to allocate when expanding */
    WORD minsize;               /* 26 Minimum size of the heap */
    WORD magic;                 /* 28 Magic number */
} LOCALHEAPINFO;

typedef struct
{
/* Arena header */
    WORD prev;          /* Previous arena | arena type */
    WORD next;          /* Next arena */
/* Start of the memory block or free-list info */
    WORD size;          /* Size of the free block */
    WORD free_prev;     /* Previous free block */
    WORD free_next;     /* Next free block */
} LOCALARENA;

#define LOCAL_ARENA_HEADER_SIZE      4
#define LOCAL_ARENA_HEADER( handle) ((handle) - LOCAL_ARENA_HEADER_SIZE)
#define LOCAL_ARENA_PTR(ptr,arena)  ((LOCALARENA *)((char *)(ptr)+(arena)))

#define MOVEABLE_PREFIX sizeof(HLOCAL)

/* Layout of a handle entry table
 *
 * WORD                     count of entries
 * LOCALHANDLEENTRY[count]  entries
 * WORD                     near ptr to next table
 */
typedef struct
{
    WORD addr;                /* Address of the MOVEABLE block */
    BYTE flags;               /* Flags for this block */
    BYTE lock;                /* Lock count */
} LOCALHANDLEENTRY;


typedef struct
{
    WORD null;        /* Always 0 */
    DWORD old_ss_sp;  /* Stack pointer; used by SwitchTaskTo() */
    WORD heap;        /* Pointer to the local heap information (if any) */
    WORD atomtable;   /* Pointer to the local atom table (if any) */
    WORD stacktop;    /* Top of the stack */
    WORD stackmin;    /* Lowest stack address used so far */
    WORD stackbottom; /* Bottom of the stack */
} INSTANCEDATA;

typedef struct _THHOOK
{
    HANDLE     hGlobalHeap;         /* 00 (handle BURGERMASTER) */
    WORD       pGlobalHeap;         /* 02 (selector BURGERMASTER) */
    HMODULE    hExeHead;            /* 04 hFirstModule */
    HMODULE    hExeSweep;           /* 06 (unused) */
    HANDLE     TopPDB;              /* 08 (handle of KERNEL PDB) */
    HANDLE     HeadPDB;             /* 0A (first PDB in list) */
    HANDLE     TopSizePDB;          /* 0C (unused) */
    HTASK      HeadTDB;             /* 0E hFirstTask */
    HTASK      CurTDB;              /* 10 hCurrentTask */
    HTASK      LoadTDB;             /* 12 (unused) */
    HTASK      LockTDB;             /* 14 hLockedTask */
} THHOOK;

typedef struct _NE_MODULE
{
    WORD      ne_magic;         /* 00 'NE' signature */
    WORD      count;            /* 02 Usage count (ne_ver/ne_rev on disk) */
    WORD      ne_enttab;        /* 04 Near ptr to entry table */
    HMODULE   next;             /* 06 Selector to next module (ne_cbenttab on disk) */
    WORD      dgroup_entry;     /* 08 Near ptr to segment entry for DGROUP (ne_crc on disk) */
    WORD      fileinfo;         /* 0a Near ptr to file info (OFSTRUCT) (ne_crc on disk) */
    WORD      ne_flags;         /* 0c Module flags */
    WORD      ne_autodata;      /* 0e Logical segment for DGROUP */
    WORD      ne_heap;          /* 10 Initial heap size */
    WORD      ne_stack;         /* 12 Initial stack size */
    DWORD     ne_csip;          /* 14 Initial cs:ip */
    DWORD     ne_sssp;          /* 18 Initial ss:sp */
    WORD      ne_cseg;          /* 1c Number of segments in segment table */
    WORD      ne_cmod;          /* 1e Number of module references */
    WORD      ne_cbnrestab;     /* 20 Size of non-resident names table */
    WORD      ne_segtab;        /* 22 Near ptr to segment table */
    WORD      ne_rsrctab;       /* 24 Near ptr to resource table */
    WORD      ne_restab;        /* 26 Near ptr to resident names table */
    WORD      ne_modtab;        /* 28 Near ptr to module reference table */
    WORD      ne_imptab;        /* 2a Near ptr to imported names table */
    DWORD     ne_nrestab;       /* 2c File offset of non-resident names table */
    WORD      ne_cmovent;       /* 30 Number of moveable entries in entry table*/
    WORD      ne_align;         /* 32 Alignment shift count */
    WORD      ne_cres;          /* 34 # of resource segments */
    BYTE      ne_exetyp;        /* 36 Operating system flags */
    BYTE      ne_flagsothers;   /* 37 Misc. flags */
    HANDLE    dlls_to_init;     /* 38 List of DLLs to initialize (ne_pretthunks on disk) */
    HANDLE    nrname_handle;    /* 3a Handle to non-resident name table (ne_psegrefbytes on disk) */
    WORD      ne_swaparea;      /* 3c Min. swap area size */
    WORD      ne_expver;        /* 3e Expected Windows version */
#if 0
    /* From here, these are extra fields not present in normal Windows */
    HMODULE   module32;         /* PE module handle for Win32 modules */
    HMODULE   owner32;          /* PE module containing this one for 16-bit builtins */
    HMODULE   self;             /* Handle for this module */
    WORD      self_loading_sel; /* Selector used for self-loading apps. */
    LPVOID    rsrc32_map;       /* HRSRC 16->32 map (for 32-bit modules) */
    LPCVOID   mapping;          /* mapping of the binary file */
    SIZE_T    mapping_size;     /* size of the file mapping */
#endif
} NE_MODULE;

#pragma pack(pop)

#define TDB_MAGIC    ('T' | ('D' << 8))

/* FIXME: to make this work, we have to call back all these registered
 * functions from all over the WINE code. Someone with more knowledge than
 * me please do that. -Marcus
 */

static struct notify
{
    HTASK     htask;
    FARPROC   lpfnCallback;
    WORD     wFlags;
} * far notifys = NULL;

static int nrofnotifys = 0;

static THHOOK far *get_thhook(void)
{
    static THHOOK far *thhook;

    if (!thhook) thhook = (THHOOK far *)GetProcAddress( GetModuleHandle("KERNEL"), (LPCSTR)332 );
    return thhook;
}

static BYTE KernelType;
#define KT_KERNEL 1
#define KT_KRNL286 2
#define KT_KRNL386 3
#define KT_WINE 4

static GLOBALARENA far *get_global_arena(void)
{
	THHOOK far * lpTH=get_thhook();
	
	if (KernelType==KT_WINE) 
	{
		return *(GLOBALARENA far **)lpTH;
	}
	if (KernelType==KT_KRNL286) 
	{
		LPBURGERMASTER_KRNL286 lpBM = MAKELP(lpTH->pGlobalHeap, 0);
		return (MAKELP(lpBM->nwFirst, 0));
	}
	if (KernelType==KT_KRNL386) 
	{
		LPBURGERMASTER_KRNL386 lpBM = MAKELP(lpTH->pGlobalHeap, 0);
		return (MAKELP(lpTH->pGlobalHeap, lpBM->nwFirst));
	}
}

static LOCALHEAPINFO far *get_local_heap( HANDLE ds )
{
    INSTANCEDATA far *ptr = MAKELP(ds, 0);

    if (!ptr || !ptr->heap) return NULL;
    return (LOCALHEAPINFO far *)((char far *)ptr + ptr->heap);
}


/***********************************************************************
 *           GlobalHandleToSel   (TOOLHELP.50)
 */
WORD WINAPI GlobalHandleToSel( HGLOBAL handle )
{
    if (!handle) return 0;
    if (!(handle & 7)) return handle - 1;
    return handle | 7;
}


/***********************************************************************
 *           GlobalFirst   (TOOLHELP.51)
 */
BOOL WINAPI GlobalFirst( LPGLOBALENTRY lpGlobal, WORD wFlags )
{
    if (wFlags == GLOBAL_LRU) return FALSE;
    lpGlobal->dwNext = 0;
    return GlobalNext( lpGlobal, wFlags );
}


/***********************************************************************
 *           GlobalNext   (TOOLHELP.52)
 */
BOOL WINAPI GlobalNext( LPGLOBALENTRY lpGlobal, WORD wFlags)
{
    GLOBALARENA far *pGlobalArena = get_global_arena();
    GLOBALARENA far *pArena;

    if (lpGlobal->dwNext >= GLOBAL_MAX_COUNT) return FALSE;
    pArena = pGlobalArena + lpGlobal->dwNext;
    if (wFlags == GLOBAL_FREE)  /* only free blocks */
    {
        int i;
        for (i = lpGlobal->dwNext; i < GLOBAL_MAX_COUNT; i++, pArena++)
            if (pArena->size == 0) break;  /* block is free */
        if (i >= GLOBAL_MAX_COUNT) return FALSE;
        lpGlobal->dwNext = i;
    }

    lpGlobal->dwAddress    = (DWORD)pArena->base;
    lpGlobal->dwBlockSize  = pArena->size;
    lpGlobal->hBlock       = pArena->handle;
    lpGlobal->wcLock       = pArena->lockCount;
    lpGlobal->wcPageLock   = pArena->pageLockCount;
    lpGlobal->wFlags       = (GetCurrentPDB() == pArena->hOwner);
    lpGlobal->wHeapPresent = FALSE;
    lpGlobal->hOwner       = pArena->hOwner;
    lpGlobal->wType        = GT_UNKNOWN;
    lpGlobal->wData        = 0;
    lpGlobal->dwNext++;
    return TRUE;
}


/***********************************************************************
 *           GlobalInfo   (TOOLHELP.53)
 */
BOOL WINAPI GlobalInfo( LPGLOBALINFO lpInfo )
{
    GLOBALARENA far *pGlobalArena = get_global_arena();
    GLOBALARENA far *pArena;
    int i;

    lpInfo->wcItems = GLOBAL_MAX_COUNT;
    lpInfo->wcItemsFree = 0;
    lpInfo->wcItemsLRU = 0;
    for (i = 0, pArena = pGlobalArena; i < GLOBAL_MAX_COUNT; i++, pArena++)
        if (pArena->size == 0) lpInfo->wcItemsFree++;
    return TRUE;
}

// @todo fix to use imported value
#define __AHSHIFT  3  /* don't change! */

/***********************************************************************
 *           GlobalEntryHandle   (TOOLHELP.54)
 */
BOOL WINAPI GlobalEntryHandle( LPGLOBALENTRY lpGlobal, HGLOBAL hItem )
{
    GLOBALARENA far *pGlobalArena = get_global_arena();
    GLOBALARENA far *pArena = pGlobalArena + (hItem >> __AHSHIFT);

    lpGlobal->dwAddress    = (DWORD)pArena->base;
    lpGlobal->dwBlockSize  = pArena->size;
    lpGlobal->hBlock       = pArena->handle;
    lpGlobal->wcLock       = pArena->lockCount;
    lpGlobal->wcPageLock   = pArena->pageLockCount;
    lpGlobal->wFlags       = (GetCurrentPDB() == pArena->hOwner);
    lpGlobal->wHeapPresent = FALSE;
    lpGlobal->hOwner       = pArena->hOwner;
    lpGlobal->wType        = GT_UNKNOWN;
    lpGlobal->wData        = 0;
    lpGlobal->dwNext++;
    return TRUE;
}


/***********************************************************************
 *           GlobalEntryModule   (TOOLHELP.55)
 */
BOOL WINAPI GlobalEntryModule( LPGLOBALENTRY lpGlobal, HMODULE hModule,
                                 WORD wSeg )
{
//    FIXME("(%p, 0x%04x, 0x%04x), stub.\n", pGlobal, hModule, wSeg);
    return FALSE;
}


/***********************************************************************
 *           LocalInfo   (TOOLHELP.56)
 */
BOOL WINAPI LocalInfo( LPLOCALINFO lpLocalInfo, HGLOBAL handle )
{
    LOCALHEAPINFO far *pInfo = get_local_heap( SELECTOROF(GlobalLock(handle)) );
    if (!pInfo) return FALSE;
    lpLocalInfo->wcItems = pInfo->items;
    return TRUE;
}


/***********************************************************************
 *           LocalFirst   (TOOLHELP.57)
 */
BOOL WINAPI LocalFirst( LPLOCALENTRY lpLocalEntry, HGLOBAL handle )
{
    WORD ds = GlobalHandleToSel( handle );
    char far *ptr = MAKELP( ds, 0 );
    LOCALHEAPINFO far *pInfo = get_local_heap( ds );
    if (!pInfo) return FALSE;

    lpLocalEntry->hHandle   = pInfo->first + LOCAL_ARENA_HEADER_SIZE;
    lpLocalEntry->wAddress  = lpLocalEntry->hHandle;
    lpLocalEntry->wFlags    = LF_FIXED;
    lpLocalEntry->wcLock    = 0;
    lpLocalEntry->wType     = LT_NORMAL;
    lpLocalEntry->hHeap     = handle;
    lpLocalEntry->wHeapType = NORMAL_HEAP;
    lpLocalEntry->wNext     = LOCAL_ARENA_PTR(ptr,pInfo->first)->next;
    lpLocalEntry->wSize     = lpLocalEntry->wNext - lpLocalEntry->hHandle;
    return TRUE;
}


/***********************************************************************
 *           LocalNext   (TOOLHELP.58)
 */
BOOL WINAPI LocalNext( LPLOCALENTRY lpLocalEntry )
{
    WORD ds = GlobalHandleToSel( lpLocalEntry->hHeap );
    LPSTR ptr = MAKELP( ds, 0 );
    LOCALARENA *pArena;
    WORD table, lhandle;
    LOCALHEAPINFO far *pInfo = get_local_heap( ds );

    if (!pInfo) return FALSE;
    if (!lpLocalEntry->wNext) return FALSE;
    table = pInfo->htable;
    pArena = LOCAL_ARENA_PTR( ptr, lpLocalEntry->wNext );
    lpLocalEntry->wAddress  = lpLocalEntry->wNext + LOCAL_ARENA_HEADER_SIZE;
    lpLocalEntry->wFlags    = (pArena->prev & 3) + 1;
    lpLocalEntry->wcLock    = 0;
    /* Find the address in the entry tables */
    lhandle = lpLocalEntry->wAddress;
    while (table)
    {
        WORD count = *(WORD *)(ptr + table);
        LOCALHANDLEENTRY *pEntry = (LOCALHANDLEENTRY*)(ptr+table+sizeof(WORD));
        for (; count > 0; count--, pEntry++)
            if (pEntry->addr == lhandle + MOVEABLE_PREFIX)
            {
                lhandle = (HLOCAL)((char *)pEntry - ptr);
                table = 0;
                lpLocalEntry->wAddress  = pEntry->addr;
                lpLocalEntry->wFlags    = pEntry->flags;
                lpLocalEntry->wcLock    = pEntry->lock;
                break;
            }
        if (table) table = *(WORD *)pEntry;
    }
    lpLocalEntry->hHandle   = lhandle;
    lpLocalEntry->wType     = LT_NORMAL;
    if (pArena->next != lpLocalEntry->wNext)  /* last one? */
        lpLocalEntry->wNext = pArena->next;
    else
        lpLocalEntry->wNext = 0;
    lpLocalEntry->wSize     = lpLocalEntry->wNext - lpLocalEntry->hHandle;
    return TRUE;
}


/**********************************************************************
 *	    ModuleFirst    (TOOLHELP.59)
 */
BOOL WINAPI ModuleFirst( LPMODULEENTRY lpme )
{
    lpme->wNext = get_thhook()->hExeHead;
    return ModuleNext( lpme );
}


/**********************************************************************
 *	    ModuleNext    (TOOLHELP.60)
 */
BOOL WINAPI ModuleNext( LPMODULEENTRY lpme )
{
    NE_MODULE far *pModule;
    char far *name;

    if (!lpme->wNext) return FALSE;
    if (!(pModule = (NE_MODULE far *)GlobalLock( GetExePtr(lpme->wNext) ))) return FALSE;
    name = (char far *)pModule + pModule->ne_restab;
    _fmemcpy( lpme->szModule, name + 1, min(*name, MAX_MODULE_NAME) );
    lpme->szModule[min(*name, MAX_MODULE_NAME)] = '\0';
    lpme->hModule = lpme->wNext;
    lpme->wcUsage = pModule->count;
    name = ((OFSTRUCT far *)((char far *)pModule + pModule->fileinfo))->szPathName;
    lstrcpyn( lpme->szExePath, name, sizeof(lpme->szExePath) );
    lpme->wNext = pModule->next;
    return TRUE;
}


/**********************************************************************
 *	    ModuleFindName    (TOOLHELP.61)
 */
BOOL WINAPI ModuleFindName( LPMODULEENTRY lpme, LPCSTR name )
{
    lpme->wNext = GetModuleHandle( name );
    return ModuleNext( lpme );
}


/**********************************************************************
 *	    ModuleFindHandle    (TOOLHELP.62)
 */
BOOL WINAPI ModuleFindHandle( LPMODULEENTRY lpme, HMODULE hModule )
{
    hModule = GetExePtr( hModule );
    lpme->wNext = hModule;
    return ModuleNext( lpme );
}


/***********************************************************************
 *           TaskFirst   (TOOLHELP.63)
 */
BOOL WINAPI TaskFirst( LPTASKENTRY lpte )
{
    lpte->hNext = get_thhook()->HeadTDB;
    return TaskNext( lpte );
}


/***********************************************************************
 *           TaskNext   (TOOLHELP.64)
 */
BOOL WINAPI TaskNext( LPTASKENTRY lpte )
{
    TDB far *pTask;
    INSTANCEDATA far *pInstData;

//    TRACE_(toolhelp)("(%p): task=%04x\n", lpte, lpte->hNext );
    if (!lpte->hNext) return FALSE;

    /* make sure that task and hInstance are valid (skip initial Wine task !) */
    while (1) {
        pTask = (TDB far *)GlobalLock( lpte->hNext );
        if (!pTask || pTask->magic != TDB_MAGIC) return FALSE;
        if (pTask->hInstance)
            break;
        lpte->hNext = pTask->hNext;
    }
    pInstData = MAKELP( GlobalHandleToSel(pTask->hInstance), 0 );
    lpte->hTask         = lpte->hNext;
    lpte->hTaskParent   = pTask->hParent;
    lpte->hInst         = pTask->hInstance;
    lpte->hModule       = pTask->hModule;
// @todo fix SS:SP for win16
    lpte->wSS           = 0;//SELECTOROF( pTask->teb->SystemReserved1[0] );
    lpte->wSP           = 0;//OFFSETOF( pTask->teb->SystemReserved1[0] );
    lpte->wStackTop     = pInstData->stacktop;
    lpte->wStackMinimum = pInstData->stackmin;
    lpte->wStackBottom  = pInstData->stackbottom;
    lpte->wcEvents      = pTask->nEvents;
    lpte->hQueue        = pTask->hQueue;
    lstrcpyn( lpte->szModule, pTask->module_name, sizeof(lpte->szModule) );
    lpte->wPSPOffset    = 0x100;  /*??*/
    lpte->hNext         = pTask->hNext;
    return TRUE;
}


/***********************************************************************
 *           TaskFindHandle   (TOOLHELP.65)
 */
BOOL WINAPI TaskFindHandle( LPTASKENTRY lpte, HTASK hTask )
{
    lpte->hNext = hTask;
    return TaskNext( lpte );
}


/***********************************************************************
 *           MemManInfo   (TOOLHELP.72)
 */
BOOL WINAPI MemManInfo( LPMEMMANINFO info )
{
//    SYSTEM_BASIC_INFORMATION sbi;
//    MEMORYSTATUS status;

    /*
     * Not unsurprisingly although the documentation says you
     * _must_ provide the size in the dwSize field, this function
     * (under Windows) always fills the structure and returns true.
     */
/*
    NtQuerySystemInformation( SystemBasicInformation, &sbi, sizeof(sbi), NULL );
    GlobalMemoryStatus( &status );
    info->wPageSize            = sbi.PageSize;
    info->dwLargestFreeBlock   = status.dwAvailVirtual;
    info->dwMaxPagesAvailable  = info->dwLargestFreeBlock / info->wPageSize;
    info->dwMaxPagesLockable   = info->dwMaxPagesAvailable;
    info->dwTotalLinearSpace   = status.dwTotalVirtual / info->wPageSize;
    info->dwTotalUnlockedPages = info->dwTotalLinearSpace;
    info->dwFreePages          = info->dwMaxPagesAvailable;
    info->dwTotalPages         = info->dwTotalLinearSpace;
    info->dwFreeLinearSpace    = info->dwMaxPagesAvailable;
    info->dwSwapFilePages      = status.dwTotalPageFile / info->wPageSize;
*/
    return TRUE;
}


/***********************************************************************
 *		NotifyRegister (TOOLHELP.73)
 */
BOOL WINAPI NotifyRegister( HTASK htask, FARPROC lpfnCallback,
                              WORD wFlags )
{
    int	i;

//    FIXME("(%x,%lx,%x), semi-stub.\n",
//                      htask, (DWORD)lpfnCallback, wFlags );
    if (!htask) htask = GetCurrentTask();
    for (i=0;i<nrofnotifys;i++)
        if (notifys[i].htask==htask)
            break;
    if (i==nrofnotifys) {
        if (notifys==NULL)
            notifys=(struct notify * far)GlobalAllocPtr(GPTR, sizeof(struct notify) );
        else
            notifys=(struct notify * far)GlobalLock(GlobalReAlloc(GlobalPtrHandle(notifys), sizeof(struct notify)*(nrofnotifys+1),0));
        if (!notifys) return FALSE;
        nrofnotifys++;
    }
    notifys[i].htask=htask;
    notifys[i].lpfnCallback=lpfnCallback;
    notifys[i].wFlags=wFlags;
    return TRUE;
}

/***********************************************************************
 *		NotifyUnregister (TOOLHELP.74)
 */
BOOL WINAPI NotifyUnregister( HTASK htask )
{
    int	i;

//    FIXME("(%x), semi-stub.\n", htask );
    if (!htask) htask = GetCurrentTask();
    for (i=nrofnotifys;i--;)
        if (notifys[i].htask==htask)
            break;
    if (i==-1)
        return FALSE;
    memcpy(notifys+i,notifys+(i+1),sizeof(struct notify)*(nrofnotifys-i-1));
    notifys=(struct notify * far)GlobalLock(GlobalReAlloc(GlobalPtrHandle(notifys), (nrofnotifys-1)*sizeof(struct notify), 0));
    nrofnotifys--;
    return TRUE;
}

/***********************************************************************
 *		StackTraceCSIPFirst (TOOLHELP.67)
 */
BOOL WINAPI StackTraceCSIPFirst(LPSTACKTRACEENTRY ste, WORD wSS, WORD wCS, WORD wIP, WORD wBP)
{
//    FIXME("(%p, ss %04x, cs %04x, ip %04x, bp %04x): stub.\n", ste, wSS, wCS, wIP, wBP);
    return TRUE;
}

/***********************************************************************
 *		StackTraceFirst (TOOLHELP.66)
 */
BOOL WINAPI StackTraceFirst(LPSTACKTRACEENTRY ste, HTASK Task)
{
//    FIXME("(%p, %04x), stub.\n", ste, Task);
    return TRUE;
}

/***********************************************************************
 *		StackTraceNext (TOOLHELP.68)
 */
BOOL WINAPI StackTraceNext(LPSTACKTRACEENTRY ste)
{
//    FIXME("(%p), stub.\n", ste);
    return TRUE;
}

/***********************************************************************
 *		InterruptRegister (TOOLHELP.75)
 */
BOOL WINAPI InterruptRegister( HTASK task, FARPROC callback )
{
//    FIXME("(%04x, %p), stub.\n", task, callback);
    return TRUE;
}

/***********************************************************************
 *		InterruptUnRegister (TOOLHELP.76)
 */
BOOL WINAPI InterruptUnRegister( HTASK task )
{
//    FIXME("(%04x), stub.\n", task);
    return TRUE;
}

/***********************************************************************
 *           TerminateApp   (TOOLHELP.77)
 *
 * See "Undocumented Windows".
 */
void WINAPI TerminateApp(HTASK hTask, WORD wFlags)
{
  union REGS in, out;

    if (hTask && hTask != GetCurrentTask())
    {
//        FIXME("cannot terminate task %x\n", hTask);
        return;
    }

#if 0  /* FIXME */
    /* check undocumented flag */
    if (!(wFlags & 0x8000))
        TASK_CallTaskSignalProc( USIG16_TERMINATION, hTask );
#endif

    /* UndocWin says to call int 0x21/0x4c exit=0xff here,
       but let's just call ExitThread */
//    ExitThread(0xff);
    in.h.al = 0xff;
    in.h.ah = 0x4c; /* установка номера функции */
    int86 (0x21, &in, & out);
}

/***********************************************************************
 *           MemoryRead   (TOOLHELP.78)
 */
DWORD WINAPI MemoryRead( WORD sel, DWORD offset, void FAR *buffer, DWORD count )
{
    char FAR *base = (char FAR *)GetSelectorBase( sel );
    DWORD limit = GetSelectorLimit( sel );

    if (offset > limit) return 0;
    if (offset + count > limit + 1) count = limit + 1 - offset;
    _fmemcpy( buffer, base + offset, count );
    return count;
}


/***********************************************************************
 *           MemoryWrite   (TOOLHELP.79)
 */
DWORD WINAPI MemoryWrite( WORD sel, DWORD offset, void FAR *buffer, DWORD count )
{
    char FAR *base = (char FAR *)GetSelectorBase( sel );
    DWORD limit = GetSelectorLimit( sel );

    if (offset > limit) return 0;
    if (offset + count > limit) count = limit + 1 - offset;
    _fmemcpy( base + offset, buffer, count );
    return count;
}

/***********************************************************************
 *           TimerCount   (TOOLHELP.80)
 */
BOOL WINAPI TimerCount( LPTIMERINFO lpTimerInfo )
{
    /* FIXME
     * In standard mode, dwmsSinceStart = dwmsThisVM
     *
     * I tested this, under Windows in enhanced mode, and
     * if you never switch VM (ie start/stop DOS) these
     * values should be the same as well.
     *
     * Also, Wine should adjust for the hardware timer
     * to reduce the amount of error to ~1ms.
     * I can't be bothered, can you?
     */
    lpTimerInfo->dwmsSinceStart = lpTimerInfo->dwmsThisVM = GetTickCount();
    return TRUE;
}

/***********************************************************************
 *           SystemHeapInfo   (TOOLHELP.71)
 */
BOOL WINAPI SystemHeapInfo( LPSYSHEAPINFO lpHeapInfo )
{
    HANDLE oldDS = GetDS();
    WORD user = LoadLibrary( "USER.EXE" );
    WORD gdi = LoadLibrary( "GDI.EXE" );

    SetDS(user);
    lpHeapInfo->wUserFreePercent = (int)LocalCountFree() * 100 / LocalHeapSize();
    SetDS(gdi);
    lpHeapInfo->wGDIFreePercent  = (int)LocalCountFree() * 100 / LocalHeapSize();
    SetDS(oldDS);
    lpHeapInfo->hUserSegment = user;
    lpHeapInfo->hGDISegment  = gdi;
    FreeLibrary( user );
    FreeLibrary( gdi );
    return TRUE;
}

/***********************************************************************
 *           Local32Info   (TOOLHELP.84)
 */
BOOL WINAPI Local32Info( LOCAL32INFO *pLocal32Info, HGLOBAL handle )
{
//    FIXME( "Call Local32Info16 in kernel\n" );
    return FALSE;
}

/***********************************************************************
 *           Local32First   (TOOLHELP.85)
 */
BOOL WINAPI Local32First( LOCAL32ENTRY *pLocal32Entry, HGLOBAL handle )
{
//    FIXME( "Call Local32First16 in kernel\n" );
    return FALSE;
}

/***********************************************************************
 *           Local32Next   (TOOLHELP.86)
 */
BOOL WINAPI Local32Next( LOCAL32ENTRY *pLocal32Entry )
{
//    FIXME( "Call Local32Next16 in kernel\n" );
    return FALSE;
}

BOOL FAR PASCAL LibMain( HINSTANCE hInstance, WORD wDataSegment,
                         WORD wHeapSize, LPSTR lpszCmdLine )
{
    const char * (CDECL *pwine_get_version)(void);
    
    HMODULE hntdll = GetModuleHandle("ntdll.dll");

	if (hntdll) {
		pwine_get_version = (void *)GetProcAddress(hntdll, "wine_get_version");
    
		if (pwine_get_version) {
			KernelType=KT_WINE;
		} else {
			// NT kernel here @todo
		}
	} else {
		LONG lFlags = GetWinFlags();
		// Detect krnl286/krnl386
		if (GetVersion() == 0x0003) // Windows 3.0: mode-dependent
		{
			if (lFlags & WF_STANDARD)       KernelType=KT_KRNL286;
			else if (lFlags & WF_ENHANCED)  KernelType=KT_KRNL386;
			else /* yuk! real mode! */      KernelType=KT_KERNEL;
		}
		else    // Windows 3.1+: processor-dependent
			KernelType= (lFlags & WF_CPU286) ? KT_KRNL286 : KT_KRNL386;
	}
	
	return( 1 );
}

DWORD  WINAPI TaskSetCSIP(HTASK hTask, WORD wCS, WORD wIP)
{
  return 0;
}

DWORD  WINAPI TaskGetCSIP(HTASK hTask)
{
  return 0;
}

BOOL WINAPI TaskSwitch(HTASK hTask, DWORD dwNewCSIP)
{
  return 0;
}

BOOL WINAPI ClassFirst( LPCLASSENTRY lpClassEntry )
{
  return FALSE;
}

BOOL WINAPI ClassNext( LPCLASSENTRY lpClassEntry )
{
  return FALSE;
}
