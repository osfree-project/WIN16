#include <win16.h>
#include <win_private.h>

void far * memset (void far *start, int c, int len);

/* LocalHeap main structure. Differs for KRNL286 and KRNL386. First column - 386 offsets, second one is 286 offsets */
typedef struct
{
    WORD check;                 /* 00 00 Heap checking flag */
    WORD freeze;                /* 02 02 Heap frozen flag */
    WORD items;                 /* 04 04 Count of items on the heap */
    WORD first;                 /* 06 06 First item of the heap */
#ifdef __386__
    WORD pad1;                  /* 08    Always 0 */	// missed in KRNL286
#endif
    WORD last;                  /* 0a 08 Last item of the heap */
#ifdef __386__
    WORD pad2;                  /* 0c    Always 0 */	// missed in KRNL286
#endif
    BYTE ncompact;              /* 0e 0a Compactions counter */
    BYTE dislevel;              /* 0f 0b Discard level */
#ifdef __386__
    DWORD distotal;             /* 10 0c Total bytes discarded */	// WORD in KRNL286, DWORD in KRNL386
#else
    WORD distotal;              /* 10 0c Total bytes discarded */	// WORD in KRNL286, DWORD in KRNL386
#endif
    WORD htable;                /* 14 0f Near Pointer to handle table */
    WORD hfree;                 /* 16 10 Near Pointer to free handle table */
    WORD hdelta;                /* 18 12 Delta to expand the handle table */
    WORD expand;                /* 1a 14 Near Pointer to expand function (unused) */
    WORD pstat;                 /* 1c 15 Near Pointer to status structure (unused) */
    FARPROC notify;             /* 1e 18 Far Pointer to LocalNotify() function */
    WORD lock;                  /* 22 1a Lock count for the heap */
    WORD extra;                 /* 24 1c Extra bytes to allocate when expanding */
    WORD minsize;               /* 26 1e Minimum size of the heap */
    WORD magic;                 /* 28 1f Magic number */
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


/* determine whether the handle belongs to a fixed or a moveable block */
#define HANDLE_FIXED(handle) (((handle) & 3) == 0)
#define HANDLE_MOVEABLE(handle) (((handle) & 3) == 2)

  /* All local heap allocations are aligned on 4-byte boundaries */
#define LALIGN(word)          (((word) + 3) & ~3)

#define ARENA_HEADER_SIZE      4
#define ARENA_HEADER( handle) ((handle) - ARENA_HEADER_SIZE)
#define ARENA_PTR(ptr,arena)       ((LOCALARENA *)((char *)(ptr)+(arena)))

  /* Arena types (stored in 'prev' field of the arena) */
#define LOCAL_ARENA_FREE       0
#define LOCAL_ARENA_FIXED      1

#define LOCAL_HEAP_MAGIC  0x484c  /* 'LH' */

/* This function returns current DS value */
extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

/***********************************************************************
 *           LocalFlags   (KERNEL.12)
 */
UINT WINAPI LocalFlags(HLOCAL handle)
{
    LPSTR ptr = MAKELP(GetDS(), 0);

    if (HANDLE_MOVEABLE(handle))
    {
        LOCALHANDLEENTRY *pEntry = (LOCALHANDLEENTRY *)(ptr + handle);
//        TRACE("(%04x,%04x): returning %04x\n",
//                       ds, handle, pEntry->lock | (pEntry->flags << 8) );
        return pEntry->lock | (pEntry->flags << 8);
    }
    else
    {
//        TRACE("(%04x,%04x): returning 0\n",
//                       ds, handle );
        return 0;
    }
}

/***********************************************************************
 *           LocalHandle   (KERNEL.11)
 */
HLOCAL WINAPI LocalHandle( UINT addr )
{
    INSTANCEDATA far * ptr = MAKELP(GetDS(), 0 );
    LOCALHEAPINFO far * pInfo;
    WORD table;

    if (!(pInfo = (LOCALHEAPINFO *)((char *)ptr + ptr->heap)))
    {
//        ERR("(%04x): Local heap not found\n", ds );
//	LOCAL_PrintHeap( ds );
	return 0;
    }

    /* Find the address in the entry tables */

    table = pInfo->htable;
    while (table)
    {
        WORD count = *(WORD *)(ptr + table);
        LOCALHANDLEENTRY *pEntry = (LOCALHANDLEENTRY*)(ptr+table+sizeof(WORD));
        for (; count > 0; count--, pEntry++)
            if (pEntry->addr == addr) return (HLOCAL)((char *)pEntry - (char *)ptr);
        table = *(WORD *)pEntry;
    }

    return (HLOCAL)addr;  /* Fixed block handle is addr */
}

/***********************************************************************
 *           LocalHandleDelta   (KERNEL.310)
 */
WORD WINAPI LocalHandleDelta( WORD delta )
{
    INSTANCEDATA far * ptr = MAKELP(GetDS(), 0 );
    LOCALHEAPINFO far *pInfo;

    if (!(pInfo = (LOCALHEAPINFO *)((char *)ptr + ptr->heap)))
    {
//        ERR("Local heap not found\n");
//	LOCAL_PrintHeap( CURRENT_DS );
	return 0;
    }
    if (delta) pInfo->hdelta = delta;
//    TRACE("returning %04x\n", pInfo->hdelta);
    return pInfo->hdelta;
}

/***********************************************************************
 *           LocalHeapSize   (KERNEL.162)
 */
WORD WINAPI LocalHeapSize(void)
{
    INSTANCEDATA far * ptr = MAKELP(GetDS(), 0 );
    LOCALHEAPINFO far * pInfo = (LOCALHEAPINFO *)((char *)ptr + ptr->heap);
    return pInfo ? pInfo->last - pInfo->first : 0;
}

/***********************************************************************
 *           LocalCountFree   (KERNEL.161)
 */
WORD WINAPI LocalCountFree(void)
{
    WORD arena, total;
    LOCALARENA *pArena;
    INSTANCEDATA far * ptr = MAKELP(GetDS(), 0);
    LOCALHEAPINFO far *pInfo = (LOCALHEAPINFO *)((char *)ptr + ptr->heap);

    if (!(pInfo))
    {
//        ERR("(%04x): Local heap not found\n", ds );
//	LOCAL_PrintHeap( ds );
	return 0;
    }

    total = 0;
    arena = pInfo->first;
    pArena = ARENA_PTR( ptr, arena );
    for (;;)
    {
        arena = pArena->free_next;
        pArena = ARENA_PTR( ptr, arena );
	if (arena == pArena->free_next) break;
        total += pArena->size;
    }
//    TRACE("(%04x): returning %d\n", ds, total);
    return total;
}

/***********************************************************************
 *           LocalInit   (KERNEL.4)
 */
BOOL WINAPI LocalInit(HANDLE selector, UINT start, UINT end)
{
    char far *ptr;
    WORD heapInfoArena, freeArena, lastArena;
    LOCALHEAPINFO *pHeapInfo;
    LOCALARENA *pArena, *pFirstArena, *pLastArena;
    BOOL ret = FALSE;

      /* The initial layout of the heap is: */
      /* - first arena         (FIXED)      */
      /* - heap info structure (FIXED)      */
      /* - large free block    (FREE)       */
      /* - last arena          (FREE)       */

//    TRACE("%04x %04x-%04x\n", selector, start, end);
    if (!selector) selector = GetDS();

//    if (TRACE_ON(local))
//    {
        /* If TRACE_ON(local) is set, the global heap blocks are */
        /* cleared before use, so we can test for double initialization. */
//        if (LOCAL_GetHeap(selector))
//        {
//            ERR("Heap %04x initialized twice.\n", selector);
//            LOCAL_PrintHeap(selector);
//        }
//    }

    if (start == 0)
    {
        /* start == 0 means: put the local heap at the end of the segment */

        DWORD size = GlobalSize(GlobalHandle(selector));
	start = (WORD)(size > 0xffff ? 0xffff : size) - 1;
        if ( end > 0xfffe ) end = 0xfffe;
        start -= end;
        end += start;
    }
    ptr = MAKELP(selector, 0);

    start = LALIGN( max( start, sizeof(INSTANCEDATA) ) );
    heapInfoArena = LALIGN(start + sizeof(LOCALARENA) );
    freeArena = LALIGN( heapInfoArena + ARENA_HEADER_SIZE
                        + sizeof(LOCALHEAPINFO) );
    lastArena = (end - sizeof(LOCALARENA)) & ~3;

      /* Make sure there's enough space.       */

    if (freeArena + sizeof(LOCALARENA) >= lastArena) goto done;

      /* Initialise the first arena */

    pFirstArena = ARENA_PTR( ptr, start );
    pFirstArena->prev      = start | LOCAL_ARENA_FIXED;
    pFirstArena->next      = heapInfoArena;
    pFirstArena->size      = LALIGN(sizeof(LOCALARENA));
    pFirstArena->free_prev = start;  /* this one */
    pFirstArena->free_next = freeArena;

      /* Initialise the arena of the heap info structure */

    pArena = ARENA_PTR( ptr, heapInfoArena );
    pArena->prev = start | LOCAL_ARENA_FIXED;
    pArena->next = freeArena;

      /* Initialise the heap info structure */

    pHeapInfo = (LOCALHEAPINFO *) (ptr + heapInfoArena + ARENA_HEADER_SIZE );
    memset( pHeapInfo, 0, sizeof(LOCALHEAPINFO) );
    pHeapInfo->items   = 4;
    pHeapInfo->first   = start;
    pHeapInfo->last    = lastArena;
    pHeapInfo->htable  = 0;
    pHeapInfo->hdelta  = 0x20;
    pHeapInfo->extra   = 0x200;
    pHeapInfo->minsize = lastArena - freeArena;
    pHeapInfo->magic   = LOCAL_HEAP_MAGIC;

      /* Initialise the large free block */

    pArena = ARENA_PTR( ptr, freeArena );
    pArena->prev      = heapInfoArena | LOCAL_ARENA_FREE;
    pArena->next      = lastArena;
    pArena->size      = lastArena - freeArena;
    pArena->free_prev = start;
    pArena->free_next = lastArena;

      /* Initialise the last block */

    pLastArena = ARENA_PTR( ptr, lastArena );
    pLastArena->prev      = freeArena | LOCAL_ARENA_FREE;
    pLastArena->next      = lastArena;  /* this one */
    pLastArena->size      = LALIGN(sizeof(LOCALARENA));
    pLastArena->free_prev = freeArena;
    pLastArena->free_next = lastArena;  /* this one */

      /* Store the local heap address in the instance data */

    ((INSTANCEDATA *)ptr)->heap = heapInfoArena + ARENA_HEADER_SIZE;
//    LOCAL_PrintHeap( selector );
    ret = TRUE;

 done:
//    CURRENT_STACK16->ecx = ret;  /* must be returned in cx too */
    return ret;
}
