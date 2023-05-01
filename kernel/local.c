#include <win16.h>
#include <win_private.h>

void far * memset (void far *start, int c, int len);
void memcpy(void far * s1, void far * s2, unsigned length);

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
} LOCALHEAPINFO, * PLOCALHEAPINFO, FAR * LPLOCALHEAPINFO;

typedef struct tagLOCALARENA
{
/* Arena header */
    WORD prev;          /* Previous arena | arena type */
    WORD next;          /* Next arena */
/* Start of the memory block or free-list info */
    WORD size;          /* Size of the free block */
    WORD free_prev;     /* Previous free block */
    WORD free_next;     /* Next free block */
} LOCALARENA, * PLOCALARENA, FAR * LPLOCALARENA;

/* determine whether the handle belongs to a fixed or a moveable block */
#define HANDLE_FIXED(handle) (((handle) & 3) == 0)
#define HANDLE_MOVEABLE(handle) (((handle) & 3) == 2)

  /* All local heap allocations are aligned on 4-byte boundaries */
#define LALIGN(word)          (((word) + 3) & ~3)

#define ARENA_HEADER_SIZE      4
#define ARENA_HEADER( handle) ((handle) - ARENA_HEADER_SIZE)
#define ARENA_PTR(ptr,arena)       ((LPLOCALARENA)((LPSTR)(ptr)+(arena)))

  /* Arena types (stored in 'prev' field of the arena) */
#define LOCAL_ARENA_FREE       0
#define LOCAL_ARENA_FIXED      1

#define LOCAL_HEAP_MAGIC  0x484c  /* 'LH' */

/*
 * We make addr = 4n + 2 and set *((WORD *)addr - 1) = &addr like Windows does
 * in case something actually relies on this.
 *
 * An unused handle has lock = flags = 0xff. In windows addr is that of next
 * free handle, at the moment in wine we set it to 0.
 *
 * A discarded block's handle has lock = addr = 0 and flags = 0x40
 * (LMEM_DISCARDED >> 8)
 */

#define MOVEABLE_PREFIX sizeof(HLOCAL)

/* LocalNotify() msgs */

#define LN_OUTOFMEM	0
#define LN_MOVE		1
#define LN_DISCARD	2

/* This function returns current DS value */
extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

/* This function sets current CX value */
extern  void          SetCX( unsigned short );
#pragma aux SetCX               = \
        "mov    cx,ax"          \
        parm                   [ax];

static inline BOOL call_notify_func( FARPROC proc, WORD msg, HLOCAL handle, WORD arg )
{
    DWORD ret;
    WORD args[3];

    if (!proc) return FALSE;
    args[2] = msg;
    args[1] = handle;
    args[0] = arg;
//    WOWCallback16Ex( (DWORD)proc, WCB16_PASCAL, sizeof(args), args, &ret );
    return LOWORD(ret);
}

/***********************************************************************
 *           LOCAL_GetHeap
 *
 * Return a pointer to the local heap, making sure it exists.
 */
static LPLOCALHEAPINFO LOCAL_GetHeap( HANDLE ds )
{
    LPINSTANCEDATA ptr = MAKELP(ds, 0);
    LPLOCALHEAPINFO lpInfo;
//    TRACE("Heap at %p, %04x\n", ptr, (ptr != NULL ? ptr->heap : 0xFFFF));
    if (!ptr || !ptr->null || !ptr->heap) return NULL;

    if (IsBadReadPtr( MAKELP(ds, ptr->heap), sizeof(LOCALHEAPINFO)))
    {
//	WARN("Bad pointer\n");
        return NULL;
    }

    lpInfo = (LPLOCALHEAPINFO)((LPSTR)ptr + ptr->heap);
    if (lpInfo->magic != LOCAL_HEAP_MAGIC)
    {
//	WARN("Bad magic\n");
	return NULL;
    }
    return lpInfo;
}

/***********************************************************************
 *           LOCAL_FindFreeBlock
 */
static HLOCAL LOCAL_FindFreeBlock( HANDLE ds, WORD size )
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap( ds );
    LPLOCALARENA lpArena;
    WORD arena;

    if (!(lpInfo))
    {
//        ERR("Local heap not found\n" );
//	LOCAL_PrintHeap(ds);
	return 0;
    }

    arena = lpInfo->first;
    lpArena = ARENA_PTR( ptr, arena );
    for (;;) {
        arena = lpArena->free_next;
        lpArena = ARENA_PTR( ptr, arena );
	if (arena == lpArena->free_next) break;
        if (lpArena->size >= size) return arena;
    }
//    TRACE("not enough space\n" );
//    LOCAL_PrintHeap(ds);
    return 0;
}




/***********************************************************************
 *           LOCAL_MakeBlockFree
 *
 * Make a block free, inserting it in the free-list.
 * 'block' is the handle of the block arena; 'baseptr' points to
 * the beginning of the data segment containing the heap.
 */
static void LOCAL_MakeBlockFree(LPSTR baseptr, WORD block)
{
    LPLOCALARENA lpArena, lpNext;
    WORD next;

      /* Mark the block as free */

    lpArena = ARENA_PTR( baseptr, block );
    lpArena->prev = (lpArena->prev & ~3) | LOCAL_ARENA_FREE;
    lpArena->size = lpArena->next - block;

      /* Find the next free block (last block is always free) */

    next = lpArena->next;
    for (;;)
    {
        lpNext = ARENA_PTR( baseptr, next );
        if ((lpNext->prev & 3) == LOCAL_ARENA_FREE) break;
        next = lpNext->next;
    }

//    TRACE("%04x, next %04x\n", block, next );
      /* Insert the free block in the free-list */

    lpArena->free_prev = lpNext->free_prev;
    lpArena->free_next = next;
    ARENA_PTR(baseptr,lpNext->free_prev)->free_next = block;
    lpNext->free_prev  = block;
}

/***********************************************************************
 *           LOCAL_RemoveFreeBlock
 *
 * Remove a block from the free-list.
 * 'block' is the handle of the block arena; 'baseptr' points to
 * the beginning of the data segment containing the heap.
 */
static void LOCAL_RemoveFreeBlock( char far *baseptr, WORD block )
{
      /* Mark the block as fixed */

    LPLOCALARENA lpArena = ARENA_PTR( baseptr, block );
    lpArena->prev = (lpArena->prev & ~3) | LOCAL_ARENA_FIXED;

      /* Remove it from the list */

    ARENA_PTR(baseptr,lpArena->free_prev)->free_next = lpArena->free_next;
    ARENA_PTR(baseptr,lpArena->free_next)->free_prev = lpArena->free_prev;
}

/***********************************************************************
 *           LOCAL_AddBlock
 *
 * Insert a new block in the heap.
 * 'new' is the handle of the new block arena; 'baseptr' points to
 * the beginning of the data segment containing the heap; 'prev' is
 * the block before the new one.
 */
static void LOCAL_AddBlock(LPSTR baseptr, WORD prev, WORD new )
{
    LPLOCALARENA lpPrev = ARENA_PTR( baseptr, prev );
    LPLOCALARENA lpNew  = ARENA_PTR( baseptr, new );

    lpNew->prev = (prev & ~3) | LOCAL_ARENA_FIXED;
    lpNew->next = lpPrev->next;
    ARENA_PTR(baseptr,lpPrev->next)->prev &= 3;
    ARENA_PTR(baseptr,lpPrev->next)->prev |= new;
    lpPrev->next = new;
}


/***********************************************************************
 *           LOCAL_GetFreeSpace
 */
static WORD LOCAL_GetFreeSpace(WORD ds, WORD countdiscard)
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap( ds );
    LPLOCALARENA lpArena;
    WORD arena;
    WORD freespace = 0;

    if (!(lpInfo))
    {
//        ERR("Local heap not found\n" );
//        LOCAL_PrintHeap(ds);
        return 0;
    }
    arena = lpInfo->first;
    lpArena = ARENA_PTR( ptr, arena );
    while (arena != lpArena->free_next)
    {
        arena = lpArena->free_next;
        lpArena = ARENA_PTR( ptr, arena );
        if (lpArena->size >= freespace) freespace = lpArena->size;
    }
    /* FIXME doesn't yet calculate space that would become free if everything
       were discarded when countdiscard == 1 */
    if (freespace < ARENA_HEADER_SIZE) freespace = 0;
    else freespace -= ARENA_HEADER_SIZE;
    return freespace;
}

/***********************************************************************
 *           LOCAL_RemoveBlock
 *
 * Remove a block from the heap.
 * 'block' is the handle of the block arena; 'baseptr' points to
 * the beginning of the data segment containing the heap.
 */
static void LOCAL_RemoveBlock(LPSTR baseptr, WORD block )
{
    LPLOCALARENA lpArena, lpTmp;

      /* Remove the block from the free-list */

//    TRACE("\n");
    lpArena = ARENA_PTR( baseptr, block );
    if ((lpArena->prev & 3) == LOCAL_ARENA_FREE)
        LOCAL_RemoveFreeBlock( baseptr, block );

      /* If the previous block is free, expand its size */

    lpTmp = ARENA_PTR( baseptr, lpArena->prev & ~3 );
    if ((lpTmp->prev & 3) == LOCAL_ARENA_FREE)
        lpTmp->size += lpArena->next - block;

      /* Remove the block from the linked list */

    lpTmp->next = lpArena->next;
    lpTmp = ARENA_PTR( baseptr, lpArena->next );
    lpTmp->prev = (lpTmp->prev & 3) | (lpArena->prev & ~3);
}


/***********************************************************************
 *           LOCAL_FreeArena
 */
static HLOCAL LOCAL_FreeArena( WORD ds, WORD arena )
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap( ds );
    LPLOCALARENA lpArena, lpPrev;

//    TRACE("%04x ds=%04x\n", arena, ds );
    if (!lpInfo) return arena;

    lpArena = ARENA_PTR( ptr, arena );
    if ((lpArena->prev & 3) == LOCAL_ARENA_FREE)
    {
	/* shouldn't happen */
//        ERR("Trying to free block %04x twice!\n",
//                 arena );
//	LOCAL_PrintHeap( ds );
	return arena;
    }

      /* Check if we can merge with the previous block */

    lpPrev = ARENA_PTR( ptr, lpArena->prev & ~3 );
    if ((lpPrev->prev & 3) == LOCAL_ARENA_FREE)
    {
        arena  = lpArena->prev & ~3;
        lpArena = lpPrev;
        LOCAL_RemoveBlock( ptr, lpPrev->next );
        lpInfo->items--;
    }
    else  /* Make a new free block */
    {
        LOCAL_MakeBlockFree( ptr, arena );
    }

      /* Check if we can merge with the next block */

    if ((lpArena->next == lpArena->free_next) &&
        (lpArena->next != lpInfo->last))
    {
        LOCAL_RemoveBlock( ptr, lpArena->next );
        lpInfo->items--;
    }
    return 0;
}

/***********************************************************************
 *           LOCAL_ShrinkArena
 *
 * Shrink an arena by creating a free block at its end if possible.
 * 'size' includes the arena header, and must be aligned.
 */
static void LOCAL_ShrinkArena( WORD ds, WORD arena, WORD size )
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALARENA lpArena = ARENA_PTR( ptr, arena );

    if (arena + size + LALIGN(sizeof(LOCALARENA)) < lpArena->next)
    {
        LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap( ds );
        if (!lpInfo) return;
        LOCAL_AddBlock( ptr, arena, arena + size );
        lpInfo->items++;
        LOCAL_FreeArena( ds, arena + size );
    }
}

/***********************************************************************
 *           LOCAL_GrowArenaDownward
 *
 * Grow an arena downward by using the previous arena (must be free).
 */
static void LOCAL_GrowArenaDownward( WORD ds, WORD arena, WORD newsize )
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap( ds );
    LPLOCALARENA lpArena = ARENA_PTR( ptr, arena );
    WORD prevArena = lpArena->prev & ~3;
    LPLOCALARENA lpPrevArena = ARENA_PTR( ptr, prevArena );
    WORD offset, size;
    LPSTR p;

    if (!(lpInfo)) return;
    offset = lpPrevArena->size;
    size = lpArena->next - arena - ARENA_HEADER_SIZE;
    LOCAL_RemoveFreeBlock( ptr, prevArena );
    LOCAL_RemoveBlock( ptr, arena );
    lpInfo->items--;
    p = (LPSTR)lpPrevArena + ARENA_HEADER_SIZE;
    while (offset < size)
    {
        memcpy( p, p + offset, offset );
        p += offset;
        size -= offset;
    }
    if (size) memcpy( p, p + offset, size );
    LOCAL_ShrinkArena( ds, prevArena, newsize );
}

/***********************************************************************
 *           LOCAL_Compact
 */
static int LOCAL_Compact( HANDLE ds, UINT minfree, UINT flags )
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap( ds );
    LPLOCALARENA lpArena, lpMoveArena, lpFinalArena;
    WORD arena, movearena, finalarena, table;
    WORD count, movesize, size;
    WORD freespace;
    LPLOCALHANDLEENTRY lpEntry;

    if (!lpInfo)
    {
//        ERR("Local heap not found\n" );
//        LOCAL_PrintHeap(ds);
        return 0;
    }
//    TRACE("ds = %04x, minfree = %04x, flags = %04x\n",
//		 ds, minfree, flags);
    freespace = LOCAL_GetFreeSpace(ds, minfree ? 0 : 1);
    if(freespace >= minfree || (flags & LMEM_NOCOMPACT))
    {
//        TRACE("Returning %04x.\n", freespace);
        return freespace;
    }
//    TRACE("Compacting heap %04x.\n", ds);
    table = lpInfo->htable;
    while(table)
    {
        lpEntry = (LPLOCALHANDLEENTRY)(ptr + table + sizeof(WORD));
        for(count = *(WORD FAR *)(ptr + table); count > 0; count--, lpEntry++)
        {
            if((lpEntry->lock == 0) && (lpEntry->flags != (LMEM_DISCARDED >> 8)))
            {
                /* OK we can move this one if we want */
//                TRACE("handle %04x (block %04x) can be moved.\n",
//			     (WORD)((char *)pEntry - ptr), pEntry->addr);
                movearena = ARENA_HEADER(lpEntry->addr - MOVEABLE_PREFIX);
                lpMoveArena = ARENA_PTR(ptr, movearena);
                movesize = lpMoveArena->next - movearena;
                arena = lpInfo->first;
                lpArena = ARENA_PTR(ptr, arena);
                size = 0xffff;
                finalarena = 0;
                /* Try to find the smallest arena that will do, */
                /* which is below us in memory */
                for(;;)
                {
                    arena = lpArena->free_next;
                    lpArena = ARENA_PTR(ptr, arena);
                    if(arena >= movearena)
                        break;
                    if(arena == lpArena->free_next)
                        break;
                    if((lpArena->size >= movesize) && (lpArena->size < size))
                    {
                        size = lpArena->size;
                        finalarena = arena;
                    }
                }
                if (finalarena) /* Actually got somewhere to move */
                {
//                    TRACE("Moving it to %04x.\n", finalarena);
                    lpFinalArena = ARENA_PTR(ptr, finalarena);
                    size = lpFinalArena->size;
                    LOCAL_RemoveFreeBlock(ptr, finalarena);
                    LOCAL_ShrinkArena( ds, finalarena, movesize );
                    /* Copy the arena to its new location */
                    memcpy((char far *)lpFinalArena + ARENA_HEADER_SIZE,
                           (char far *)lpMoveArena + ARENA_HEADER_SIZE,
                           movesize - ARENA_HEADER_SIZE );
                    /* Free the old location */
                    LOCAL_FreeArena(ds, movearena);

                    call_notify_func(lpInfo->notify, LN_MOVE,
                                     (WORD)((char far *)lpEntry - ptr), lpEntry->addr);
                    /* Update handle table entry */
                    lpEntry->addr = finalarena + ARENA_HEADER_SIZE + MOVEABLE_PREFIX;
                }
                else if((ARENA_PTR(ptr, lpMoveArena->prev & ~3)->prev & 3)
			       == LOCAL_ARENA_FREE)
                {
                    /* Previous arena is free (but < movesize)  */
                    /* so we can 'slide' movearena down into it */
                    finalarena = lpMoveArena->prev & ~3;
                    LOCAL_GrowArenaDownward( ds, movearena, movesize );
                    /* Update handle table entry */
                    lpEntry->addr = finalarena + ARENA_HEADER_SIZE + MOVEABLE_PREFIX;
                }
            }
        }
        table = *(WORD FAR *)lpEntry;
    }
    freespace = LOCAL_GetFreeSpace(ds, minfree ? 0 : 1);
    if(freespace >= minfree || (flags & LMEM_NODISCARD))
    {
//        TRACE("Returning %04x.\n", freespace);
        return freespace;
    }

    table = lpInfo->htable;
    while(table)
    {
        lpEntry = (LPLOCALHANDLEENTRY)(ptr + table + sizeof(WORD));
        for(count = *(WORD FAR *)(ptr + table); count > 0; count--, lpEntry++)
        {
            if(lpEntry->addr && lpEntry->lock == 0 &&
	     (lpEntry->flags & (LMEM_DISCARDABLE >> 8)))
	    {
//                TRACE("Discarding handle %04x (block %04x).\n",
//                              (char *)pEntry - ptr, pEntry->addr);
                LOCAL_FreeArena(ds, ARENA_HEADER(lpEntry->addr - MOVEABLE_PREFIX));
                call_notify_func(lpInfo->notify, LN_DISCARD, (char far *)lpEntry - ptr, lpEntry->flags);
                lpEntry->addr = 0;
                lpEntry->flags = (LMEM_DISCARDED >> 8);
            }
        }
        table = *(WORD FAR *)lpEntry;
    }
    return LOCAL_Compact(ds, 0xffff, LMEM_NODISCARD);
}

/***********************************************************************
 *           LOCAL_GrowHeap
 */
static BOOL LOCAL_GrowHeap( HANDLE ds )
{
    HANDLE hseg;
    LONG oldsize;
    LONG end;
    LPLOCALHEAPINFO lpHeapInfo;
    WORD freeArena, lastArena;
    LPLOCALARENA lpArena, lpLastArena;
    LPSTR ptr;

    hseg = GlobalHandle( ds );
    /* maybe mem allocated by Virtual*() ? */
    if (!hseg) return FALSE;

    oldsize = GlobalSize( hseg );
    /* if nothing can be gained, return */
    if (oldsize > 0xfff0) return FALSE;
    hseg = GlobalReAlloc( hseg, 0x10000, GMEM_FIXED );
    ptr = MAKELP( ds, 0 ) ;
    lpHeapInfo = LOCAL_GetHeap( ds );
    if (lpHeapInfo == NULL) {
//	ERR("Heap not found\n" );
	return FALSE;
    }
    end = GlobalSize( hseg );
    lastArena = (end - sizeof(LOCALARENA)) & ~3;

      /* Update the HeapInfo */
    lpHeapInfo->items++;
    freeArena = lpHeapInfo->last;
    lpHeapInfo->last = lastArena;
    lpHeapInfo->minsize += end - oldsize;

      /* grow the old last block */
    lpArena = ARENA_PTR( ptr, freeArena );
    lpArena->size      = lastArena - freeArena;
    lpArena->next      = lastArena;
    lpArena->free_next = lastArena;

      /* Initialise the new last block */

    lpLastArena = ARENA_PTR( ptr, lastArena );
    lpLastArena->prev      = freeArena | LOCAL_ARENA_FREE;
    lpLastArena->next      = lastArena;  /* this one */
    lpLastArena->size      = LALIGN(sizeof(LOCALARENA));
    lpLastArena->free_prev = freeArena;
    lpLastArena->free_next = lastArena;  /* this one */

    /* If block before freeArena is also free then merge them */
    if((ARENA_PTR(ptr, (lpArena->prev & ~3))->prev & 3) == LOCAL_ARENA_FREE)
    {
        LOCAL_RemoveBlock(ptr, freeArena);
        lpHeapInfo->items--;
    }

//    TRACE("Heap expanded\n" );
//    LOCAL_PrintHeap( ds );
    return TRUE;
}

/***********************************************************************
 *           LOCAL_GetBlock
 * The segment may get moved around in this function, so all callers
 * should reset their pointer variables.
 */
static HLOCAL LOCAL_GetBlock(HANDLE ds, WORD size, WORD flags )
{
    LPSTR ptr = MAKELP(ds, 0);
    LPLOCALHEAPINFO lpInfo;
    LPLOCALARENA lpArena;
    WORD arena;

    if (!(lpInfo = LOCAL_GetHeap(ds)))
    {
//        ERR("Local heap not found\n");
//	LOCAL_PrintHeap(ds);
	return 0;
    }

    size += ARENA_HEADER_SIZE;
    size = LALIGN( max( size, sizeof(LOCALARENA) ) );

#if 0
notify_done:
#endif
      /* Find a suitable free block */
    arena = LOCAL_FindFreeBlock( ds, size );
    if (arena == 0) {
	/* no space: try to make some */
	LOCAL_Compact( ds, size, flags );
	arena = LOCAL_FindFreeBlock( ds, size );
    }
    if (arena == 0) {
	/* still no space: try to grow the segment */
	if (!(LOCAL_GrowHeap( ds )))
	{
#if 0
	    /* FIXME: doesn't work correctly yet */
	    if (call_notify_func(pInfo->notify, LN_OUTOFMEM, ds - 20, size)) /* FIXME: "size" correct ? (should indicate bytes needed) */
		goto notify_done;
#endif
//            ERR( "not enough space in %s heap %04x for %d bytes\n",
//                 get_heap_name(ds), ds, size );
	    return 0;
	}
	ptr = MAKELP( ds, 0 );
	lpInfo = LOCAL_GetHeap( ds );
	arena = LOCAL_FindFreeBlock( ds, size );
    }
    if (arena == 0) {
//        ERR( "not enough space in %s heap %04x for %d bytes\n",
//             get_heap_name(ds), ds, size );
#if 0
        /* FIXME: "size" correct ? (should indicate bytes needed) */
        if (call_notify_func(pInfo->notify, LN_OUTOFMEM, ds, size)) goto notify_done;
#endif
	return 0;
    }

      /* Make a block out of the free arena */
    lpArena = ARENA_PTR( ptr, arena );
//    TRACE("size = %04x, arena %04x size %04x\n", size, arena, pArena->size );
    LOCAL_RemoveFreeBlock( ptr, arena );
    LOCAL_ShrinkArena( ds, arena, size );

    if (flags & LMEM_ZEROINIT)
	memset((char FAR *)lpArena + ARENA_HEADER_SIZE, 0, size-ARENA_HEADER_SIZE);
    return arena + ARENA_HEADER_SIZE;
}

/***********************************************************************
 *           LOCAL_NewHTable
 */
static BOOL LOCAL_NewHTable( HANDLE ds )
{
    LPSTR ptr;
    LPLOCALHEAPINFO lpInfo;
    LPLOCALHANDLEENTRY lpEntry;
    HLOCAL handle;
    int i;

//    TRACE("\n" );
    if (!(lpInfo = LOCAL_GetHeap( ds )))
    {
//        ERR("Local heap not found\n");
//        LOCAL_PrintHeap(ds);
        return FALSE;
    }

    if (!(handle = LOCAL_GetBlock( ds, lpInfo->hdelta * sizeof(LOCALHANDLEENTRY)
                                   + 2 * sizeof(WORD), LMEM_FIXED )))
        return FALSE;
    if (!(ptr = MAKELP( ds, 0 ) ))
    {
    //    ERR("ptr == NULL after GetBlock.\n");
    }
    if (!(lpInfo = LOCAL_GetHeap( ds )))
    {
//        ERR("pInfo == NULL after GetBlock.\n");
    }

    /* Fill the entry table */

    *(WORD FAR *)(ptr + handle) = lpInfo->hdelta;
    lpEntry = (LPLOCALHANDLEENTRY)(ptr + handle + sizeof(WORD));
    for (i = lpInfo->hdelta; i > 0; i--, lpEntry++) {
	lpEntry->lock = lpEntry->flags = 0xff;
	lpEntry->addr = 0;
    }
    *(WORD FAR *)lpEntry = lpInfo->htable;
    lpInfo->htable = handle;
    return TRUE;
}

/***********************************************************************
 *           LOCAL_GetNewHandleEntry
 */
static HLOCAL LOCAL_GetNewHandleEntry( HANDLE ds )
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHEAPINFO lpInfo;
    LPLOCALHANDLEENTRY lpEntry = NULL;
    WORD table;

    if (!(lpInfo = LOCAL_GetHeap( ds )))
    {
//        ERR("Local heap not found\n");
//	LOCAL_PrintHeap(ds);
	return 0;
    }

    /* Find a free slot in existing tables */

    table = lpInfo->htable;
    while (table)
    {
        WORD count = *(WORD FAR *)(ptr + table);
        lpEntry = (LPLOCALHANDLEENTRY)(ptr + table + sizeof(WORD));
        for (; count > 0; count--, lpEntry++)
            if (lpEntry->lock == 0xff) break;
        if (count) break;
        table = *(WORD FAR *)lpEntry;
    }

    if (!table)  /* We need to create a new table */
    {
        if (!LOCAL_NewHTable( ds )) return 0;
	ptr = MAKELP( ds, 0 );
	lpInfo = LOCAL_GetHeap( ds );
        lpEntry = (LPLOCALHANDLEENTRY)(ptr + lpInfo->htable + sizeof(WORD));
    }

    /* Now allocate this entry */

    lpEntry->lock = 0;
    lpEntry->flags = 0;
//    TRACE("(%04x): %04x\n", ds, ((char *)pEntry - ptr) );
    return (HLOCAL)((char FAR *)lpEntry - ptr);
}

/***********************************************************************
 *           LOCAL_GrowArenaUpward
 *
 * Grow an arena upward by using the next arena (must be free and big
 * enough). Newsize includes the arena header and must be aligned.
 */
static void LOCAL_GrowArenaUpward( WORD ds, WORD arena, WORD newsize )
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHEAPINFO lpInfo;
    LPLOCALARENA lpArena = ARENA_PTR( ptr, arena );
    WORD nextArena = lpArena->next;

    if (!(lpInfo = LOCAL_GetHeap( ds ))) return;
    LOCAL_RemoveBlock( ptr, nextArena );
    lpInfo->items--;
    LOCAL_ShrinkArena( ds, arena, newsize );
}


/***********************************************************************
 *           LOCAL_InternalLock
 */
static HLOCAL LOCAL_InternalLock( LPSTR heap, HLOCAL handle )
{
    HLOCAL old_handle = handle;

    if (HANDLE_MOVEABLE(handle))
    {
        LPLOCALHANDLEENTRY lpEntry = (LPLOCALHANDLEENTRY)(heap + handle);
        if (lpEntry->flags == (LMEM_DISCARDED >> 8)) return 0;
        if (lpEntry->lock < 0xfe) lpEntry->lock++;
        handle = lpEntry->addr;
    }
//    TRACE("%04x returning %04x\n", old_handle, handle );
    return handle;
}

/***********************************************************************
 *           LOCAL_FreeHandleEntry
 *
 * Free a handle table entry.
 */
static void LOCAL_FreeHandleEntry( HANDLE ds, HLOCAL handle )
{
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHANDLEENTRY lpEntry = (LPLOCALHANDLEENTRY)(ptr + handle);
    LPLOCALHEAPINFO lpInfo;
    WORD FAR * lpTable;
    WORD table, count, i;

    if (!(lpInfo = LOCAL_GetHeap( ds ))) return;

    /* Find the table where this handle comes from */

    lpTable = &lpInfo->htable;
    while (*lpTable)
    {
        WORD size = (*(WORD FAR *)(ptr + *lpTable)) * sizeof(LOCALHANDLEENTRY);
        if ((handle >= *lpTable + sizeof(WORD)) &&
            (handle < *lpTable + sizeof(WORD) + size)) break;  /* Found it */
        lpTable = (WORD FAR *)(ptr + *lpTable + sizeof(WORD) + size);
    }
    if (!*lpTable)
    {
//        ERR("Invalid entry %04x\n", handle);
//        LOCAL_PrintHeap( ds );
        return;
    }

    /* Make the entry free */

    lpEntry->addr = 0;  /* just in case */
    lpEntry->lock = 0xff;
    lpEntry->flags = 0xff;
    /* Now check if all entries in this table are free */

    table = *lpTable;
    lpEntry = (LPLOCALHANDLEENTRY)(ptr + table + sizeof(WORD));
    count = *(WORD FAR *)(ptr + table);
    for (i = count; i > 0; i--, lpEntry++) if (lpEntry->lock != 0xff) return;

    /* Remove the table from the linked list and free it */

//    TRACE("(%04x): freeing table %04x\n", ds, table);
    *lpTable = *(WORD FAR *)lpEntry;
    LOCAL_FreeArena( ds, ARENA_HEADER( table ) );
}

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
    LPINSTANCEDATA ptr = MAKELP(GetDS(), 0 );
    LPLOCALHEAPINFO lpInfo;
    WORD table;

    if (!(lpInfo = LOCAL_GetHeap(GetDS())))
    {
//        ERR("(%04x): Local heap not found\n", ds );
//	LOCAL_PrintHeap( ds );
	return 0;
    }

    /* Find the address in the entry tables */

    table = lpInfo->htable;
    while (table)
    {
        WORD count = *(WORD FAR *)(ptr + table);
        LPLOCALHANDLEENTRY lpEntry = (LPLOCALHANDLEENTRY)(ptr+table+sizeof(WORD));
        for (; count > 0; count--, lpEntry++)
            if (lpEntry->addr == addr) return (HLOCAL)((char FAR *)lpEntry - (char FAR *)ptr);
        table = *(WORD FAR *)lpEntry;
    }

    return (HLOCAL)addr;  /* Fixed block handle is addr */
}

/***********************************************************************
 *           LocalHandleDelta   (KERNEL.310)
 */
WORD WINAPI LocalHandleDelta( WORD delta )
{
    LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap(GetDS());

    if (!lpInfo)
    {
//        ERR("Local heap not found\n");
//	LOCAL_PrintHeap( CURRENT_DS );
	return 0;
    }
    if (delta) lpInfo->hdelta = delta;
//    TRACE("returning %04x\n", pInfo->hdelta);
    return lpInfo->hdelta;
}

/***********************************************************************
 *           LocalHeapSize   (KERNEL.162)
 */
WORD WINAPI LocalHeapSize(void)
{
    LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap(GetDS());
    return lpInfo ? lpInfo->last - lpInfo->first : 0;
}

/***********************************************************************
 *           LocalCountFree   (KERNEL.161)
 */
WORD WINAPI LocalCountFree(void)
{
    WORD arena, total;
    LPLOCALARENA lpArena;
    LPINSTANCEDATA ptr = MAKELP(GetDS(), 0);
    LPLOCALHEAPINFO lpInfo = LOCAL_GetHeap(GetDS());

    if (!(lpInfo))
    {
//        ERR("(%04x): Local heap not found\n", ds );
//	LOCAL_PrintHeap( ds );
	return 0;
    }

    total = 0;
    arena = lpInfo->first;
    lpArena = ARENA_PTR( ptr, arena );
    for (;;)
    {
        arena = lpArena->free_next;
        lpArena = ARENA_PTR( ptr, arena );
	if (arena == lpArena->free_next) break;
        total += lpArena->size;
    }
//    TRACE("(%04x): returning %d\n", ds, total);
    return total;
}

/***********************************************************************
 *           LocalInit   (KERNEL.4)
 */
BOOL WINAPI LocalInit(HANDLE selector, UINT start, UINT end)
{
    LPSTR ptr;
    WORD heapInfoArena, freeArena, lastArena;
    LPLOCALHEAPINFO lpHeapInfo;
    LPLOCALARENA lpArena, lpFirstArena, lpLastArena;
    BOOL ret = FALSE;

      /* The initial layout of the heap is: */
      /* - first arena         (FIXED)      */
      /* - heap info structure (FIXED)      */
      /* - large free block    (FREE)       */
      /* - last arena          (FREE)       */

//    TRACE("%04x %04x-%04x\n", selector, start, end);
    if (!selector) selector = GetDS();

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

    lpFirstArena = ARENA_PTR( ptr, start );
    lpFirstArena->prev      = start | LOCAL_ARENA_FIXED;
    lpFirstArena->next      = heapInfoArena;
    lpFirstArena->size      = LALIGN(sizeof(LOCALARENA));
    lpFirstArena->free_prev = start;  /* this one */
    lpFirstArena->free_next = freeArena;

      /* Initialise the arena of the heap info structure */

    lpArena = ARENA_PTR( ptr, heapInfoArena );
    lpArena->prev = start | LOCAL_ARENA_FIXED;
    lpArena->next = freeArena;

      /* Initialise the heap info structure */

    lpHeapInfo = (LPLOCALHEAPINFO) (ptr + heapInfoArena + ARENA_HEADER_SIZE );
    memset( lpHeapInfo, 0, sizeof(LOCALHEAPINFO) );
    lpHeapInfo->items   = 4;
    lpHeapInfo->first   = start;
    lpHeapInfo->last    = lastArena;
    lpHeapInfo->htable  = 0;
    lpHeapInfo->hdelta  = 0x20;
    lpHeapInfo->extra   = 0x200;
    lpHeapInfo->minsize = lastArena - freeArena;
    lpHeapInfo->magic   = LOCAL_HEAP_MAGIC;

      /* Initialise the large free block */

    lpArena = ARENA_PTR( ptr, freeArena );
    lpArena->prev      = heapInfoArena | LOCAL_ARENA_FREE;
    lpArena->next      = lastArena;
    lpArena->size      = lastArena - freeArena;
    lpArena->free_prev = start;
    lpArena->free_next = lastArena;

      /* Initialise the last block */

    lpLastArena = ARENA_PTR( ptr, lastArena );
    lpLastArena->prev      = freeArena | LOCAL_ARENA_FREE;
    lpLastArena->next      = lastArena;  /* this one */
    lpLastArena->size      = LALIGN(sizeof(LOCALARENA));
    lpLastArena->free_prev = freeArena;
    lpLastArena->free_next = lastArena;  /* this one */

      /* Store the local heap address in the instance data */

    ((LPINSTANCEDATA)ptr)->heap = heapInfoArena + ARENA_HEADER_SIZE;
//    LOCAL_PrintHeap( selector );
    ret = TRUE;

 done:
    SetCX(ret);  /* must be returned in cx too */
    return ret;
}

/***********************************************************************
 *           LocalAlloc   (KERNEL.5)
 */
HLOCAL WINAPI LocalAlloc(UINT flags, UINT size)
{
    HANDLE ds = GetDS();
    HLOCAL handle = 0;
    LPSTR ptr;

//    TRACE("%04x %d ds=%04x\n", flags, size, ds );

    if(size > 0 && size <= 4) size = 5;
    if (flags & LMEM_MOVEABLE)
    {
	LPLOCALHANDLEENTRY lplhe;
	HLOCAL hmem;

	if(size)
	{
	    if (!(hmem = LOCAL_GetBlock( ds, size + MOVEABLE_PREFIX, flags )))
		goto exit;
        }
	else /* We just need to allocate a discarded handle */
	    hmem = 0;
	if (!(handle = LOCAL_GetNewHandleEntry( ds )))
        {
//	    WARN("Couldn't get handle.\n");
	    if(hmem)
		LOCAL_FreeArena( ds, ARENA_HEADER(hmem) );
	    goto exit;
	}
	ptr = MAKELP(ds, 0);
	lplhe = (LPLOCALHANDLEENTRY)(ptr + handle);
	lplhe->lock = 0;
	if(hmem)
	{
	    lplhe->addr = hmem + MOVEABLE_PREFIX;
	    lplhe->flags = (BYTE)((flags & 0x0f00) >> 8);
	    *(HLOCAL FAR *)(ptr + hmem) = handle;
	}
	else
	{
	    lplhe->addr = 0;
	    lplhe->flags = LMEM_DISCARDED >> 8;
        }
    }
    else /* FIXED */
    {
	if(size) handle = LOCAL_GetBlock( ds, size, flags );
    }

exit:
    SetCX(handle);  /* must be returned in cx too */
    return handle;
}

/***********************************************************************
 *           LocalReAlloc   (KERNEL.6)
 */
HLOCAL WINAPI LocalReAlloc( HLOCAL handle, UINT size, UINT flags )
{
    HANDLE ds = GetDS();
    LPSTR ptr = MAKELP( ds, 0 );
    LPLOCALHEAPINFO lpInfo;
    LPLOCALARENA lpArena, lpNext;
    LPLOCALHANDLEENTRY lpEntry = NULL;
    WORD arena, oldsize;
    HLOCAL hmem, blockhandle;
    LONG nextarena;

    if (!handle) return 0;
    if(HANDLE_MOVEABLE(handle) &&
     ((LPLOCALHANDLEENTRY)(ptr + handle))->lock == 0xff) /* An unused handle */
	return 0;

//    TRACE("%04x %d %04x ds=%04x\n", handle, size, flags, ds );
    if (!(lpInfo = LOCAL_GetHeap( ds ))) return 0;

    if (HANDLE_FIXED( handle ))
	blockhandle = handle;
    else
    {
	lpEntry = (LPLOCALHANDLEENTRY) (ptr + handle);
	if(lpEntry->flags == (LMEM_DISCARDED >> 8))
        {
	    HLOCAL hl;
	    if(lpEntry->addr)
            {
//		WARN("Dicarded block has non-zero addr.\n");
            }
//	    TRACE("ReAllocating discarded block\n");
	    if(size <= 4) size = 5;
	    if (!(hl = LOCAL_GetBlock( ds, size + MOVEABLE_PREFIX, flags)))
		return 0;
            ptr = MAKELP( ds, 0 );  /* Reload ptr */
            lpEntry = (LPLOCALHANDLEENTRY) (ptr + handle);
	    lpEntry->addr = hl + MOVEABLE_PREFIX;
            lpEntry->flags = 0;
            lpEntry->lock = 0;
	    *(HLOCAL FAR *)(ptr + hl) = handle;
            return handle;
	}
	if (((blockhandle = lpEntry->addr - MOVEABLE_PREFIX) & 3) != 0)
	{
//	    ERR("(%04x,%04x): invalid handle\n",
//                     ds, handle );
	    return 0;
        }
	if (*(HLOCAL FAR *)(ptr + blockhandle) != handle) {
//	    ERR("Back ptr to handle is invalid\n");
	    return 0;
        }
    }

    if (flags & LMEM_MODIFY)
    {
        if (HANDLE_MOVEABLE(handle))
	{
	    lpEntry = (LPLOCALHANDLEENTRY)(ptr + handle);
	    lpEntry->flags = (flags & 0x0f00) >> 8;
//	    TRACE("Changing flags to %x.\n", pEntry->flags);
	}
	return handle;
    }

    if (!size)
    {
        if (flags & LMEM_MOVEABLE)
        {
	    if (HANDLE_FIXED(handle))
	    {
//                TRACE("Freeing fixed block.\n");
                return LocalFree( handle );
            }
	    else /* Moveable block */
	    {
		lpEntry = (LPLOCALHANDLEENTRY)(ptr + handle);
		if (lpEntry->lock == 0)
		{
		    /* discards moveable blocks */
//                    TRACE("Discarding block\n");
                    LOCAL_FreeArena(ds, ARENA_HEADER(lpEntry->addr - MOVEABLE_PREFIX));
                    lpEntry->addr = 0;
                    lpEntry->flags = (LMEM_DISCARDED >> 8);
                    return handle;
	        }
	    }
	    return 0;
        }
        else if(flags == 0)
        {
            lpEntry = (LPLOCALHANDLEENTRY)(ptr + handle);
            if (lpEntry->lock == 0)
            {
		/* Frees block */
		return LocalFree( handle );
	    }
        }
        return 0;
    }

    arena = ARENA_HEADER( blockhandle );
//    TRACE("arena is %04x\n", arena );
    lpArena = ARENA_PTR( ptr, arena );

    if(size <= 4) size = 5;
    if(HANDLE_MOVEABLE(handle)) size += MOVEABLE_PREFIX;
    oldsize = lpArena->next - arena - ARENA_HEADER_SIZE;
    nextarena = LALIGN(blockhandle + size);

      /* Check for size reduction */

    if (nextarena <= lpArena->next)
    {
//	TRACE("size reduction, making new free block\n");
	LOCAL_ShrinkArena(ds, arena, nextarena - arena);
//        TRACE("returning %04x\n", handle );
        return handle;
    }

      /* Check if the next block is free and large enough */

    lpNext = ARENA_PTR( ptr, lpArena->next );
    if (((lpNext->prev & 3) == LOCAL_ARENA_FREE) &&
        (nextarena <= lpNext->next))
    {
//	TRACE("size increase, making new free block\n");
        LOCAL_GrowArenaUpward(ds, arena, nextarena - arena);
        if (flags & LMEM_ZEROINIT)
        {
            LPSTR oldend = (LPSTR)lpArena + ARENA_HEADER_SIZE + oldsize;
            LPSTR newend = ptr + lpArena->next;
//            TRACE("Clearing memory from %p to %p (DS -> %p)\n", oldend, newend, ptr);
            memset(oldend, 0, newend - oldend);
        }

//        TRACE("returning %04x\n", handle );
        return handle;
    }

    /* Now we have to allocate a new block, but not if (fixed block or locked
       block) and no LMEM_MOVEABLE */

    if (!(flags & LMEM_MOVEABLE))
    {
	if (HANDLE_FIXED(handle))
        {
//            ERR("Needed to move fixed block, but LMEM_MOVEABLE not specified.\n");
            return 0;
        }
	else
	{
	    if(((LPLOCALHANDLEENTRY)(ptr + handle))->lock != 0)
	    {
//		ERR("Needed to move locked block, but LMEM_MOVEABLE not specified.\n");
		return 0;
	    }
        }
    }

    hmem = LOCAL_GetBlock( ds, size, flags );
    ptr = MAKELP( ds, 0 );  /* Reload ptr                             */
    if(HANDLE_MOVEABLE(handle))         /* LOCAL_GetBlock might have triggered    */
    {                                   /* a compaction, which might in turn have */
      blockhandle = lpEntry->addr - MOVEABLE_PREFIX; /* moved the very block we are resizing */
      arena = ARENA_HEADER( blockhandle );   /* thus, we reload arena, too        */
    }
    if (!hmem)
    {
// @todo We have no Heap* functions. Use Global Heap for temporary buffer?
#if 0
        /* Remove the block from the heap and try again */
        LPSTR buffer = HeapAlloc( GetProcessHeap(), 0, oldsize );
        if (!buffer) return 0;
        memcpy( buffer, ptr + arena + ARENA_HEADER_SIZE, oldsize );
        LOCAL_FreeArena( ds, arena );
        if (!(hmem = LOCAL_GetBlock( ds, size, flags )))
        {
            if (!(hmem = LOCAL_GetBlock( ds, oldsize, flags )))
            {
//                ERR("Can't restore saved block\n" );
                HeapFree( GetProcessHeap(), 0, buffer );
                return 0;
            }
            size = oldsize;
        }
        ptr = MAKELP( ds, 0 );  /* Reload ptr */
        memcpy( ptr + hmem, buffer, oldsize );
        HeapFree( GetProcessHeap(), 0, buffer );
#endif
    }
    else
    {
        memcpy( ptr + hmem, ptr + (arena + ARENA_HEADER_SIZE), oldsize );
        LOCAL_FreeArena( ds, arena );
    }
    if (HANDLE_MOVEABLE( handle ))
    {
	//TRACE("fixing handle\n");
        lpEntry = (LPLOCALHANDLEENTRY)(ptr + handle);
        lpEntry->addr = hmem + MOVEABLE_PREFIX;
	/* Back ptr should still be correct */
	if(*(HLOCAL FAR *)(ptr + hmem) != handle)
        {
//	    ERR("back ptr is invalid.\n");
        }
	hmem = handle;
    }
    if (size == oldsize) hmem = 0;  /* Realloc failed */
//    TRACE("returning %04x\n", hmem );
    return hmem;
}

/***********************************************************************
 *           LocalCompact   (KERNEL.13)
 */
UINT WINAPI LocalCompact( UINT minfree )
{
//    TRACE("%04x\n", minfree );
    return LOCAL_Compact( GetDS(), minfree, 0 );
}

/***********************************************************************
 *           LocalSize   (KERNEL.10)
 */
UINT WINAPI LocalSize( HLOCAL handle )
{
    LPSTR ptr = MAKELP( GetDS(), 0 );
    LPLOCALARENA lpArena;

//    TRACE("%04x ds=%04x\n", handle, ds );

    if (!handle) return 0;
    if (HANDLE_MOVEABLE( handle ))
    {
        handle = *(WORD FAR *)(ptr + handle);
        if (!handle) return 0;
        lpArena = ARENA_PTR( ptr, ARENA_HEADER(handle - MOVEABLE_PREFIX) );
    }
    else
        lpArena = ARENA_PTR( ptr, ARENA_HEADER(handle) );

    return lpArena->next - handle;
}


/***********************************************************************
 *           LocalLock   (KERNEL.8)
 *
 */
char NEAR * WINAPI LocalLock( HLOCAL handle )
{
    LPSTR ptr = MAKELP( GetDS(), 0 );
//    return MAKELP( ds, LOCAL_InternalLock( ptr, handle ) );
    return (char NEAR *)LOCAL_InternalLock( ptr, handle );
}

/***********************************************************************
 *           LocalUnlock   (KERNEL.9)
 */
BOOL WINAPI LocalUnlock( HLOCAL handle )
{
    HANDLE ds = GetDS();
    LPSTR ptr = MAKELP( ds, 0 );

//    TRACE("%04x\n", handle );
    if (HANDLE_MOVEABLE(handle))
    {
        LPLOCALHANDLEENTRY lpEntry = (LPLOCALHANDLEENTRY)(ptr + handle);
        if (!lpEntry->lock || (lpEntry->lock == 0xff)) return FALSE;
        /* For moveable block, return the new lock count */
        /* (see _Windows_Internals_ p. 197) */
        return --lpEntry->lock;
    }
    else return FALSE;
}

/***********************************************************************
 *           LocalNotify   (KERNEL.14)
 *
 * Installs a callback function that is called for local memory events
 * Callback function prototype is
 * BOOL NotifyFunc(WORD wMsg, HLOCAL hMem, WORD wArg)
 * wMsg:
 * - LN_OUTOFMEM
 *   NotifyFunc seems to be responsible for allocating some memory,
 *   returns TRUE for success.
 *   wArg = number of bytes needed additionally
 * - LN_MOVE
 *   hMem = handle; wArg = old mem location
 * - LN_DISCARD
 *   NotifyFunc seems to be strongly encouraged to return TRUE,
 *   otherwise LogError() gets called.
 *   hMem = handle; wArg = flags
 */
FARPROC WINAPI LocalNotify( FARPROC func )
{
    LPLOCALHEAPINFO lpInfo;
    FARPROC oldNotify;
    HANDLE ds = GetDS();

    if (!(lpInfo = LOCAL_GetHeap( ds )))
    {
//        ERR("(%04x): Local heap not found\n", ds );
//	LOCAL_PrintHeap( ds );
	return 0;
    }
//    TRACE("(%04x): %p\n", ds, func );
//    FIXME("Half implemented\n");
    oldNotify = lpInfo->notify;
    lpInfo->notify = func;
    return oldNotify;
}

/***********************************************************************
 *           LocalFree   (KERNEL.7)
 */
HLOCAL WINAPI LocalFree( HLOCAL handle )
{
    HANDLE ds = GetDS();
    LPSTR ptr = MAKELP( ds, 0 );

//    TRACE("%04x ds=%04x\n", handle, ds );

    if (!handle) 
    {
      // WARN("Handle is 0.\n" ); 
      return 0; 
    }
    if (HANDLE_FIXED( handle ))
    {
        if (!LOCAL_FreeArena( ds, ARENA_HEADER( handle ) )) return 0;  /* OK */
        else return handle;  /* couldn't free it */
    }
    else
    {
        LPLOCALHANDLEENTRY lpEntry = (LPLOCALHANDLEENTRY)(ptr + handle);
        if (lpEntry->flags != (LMEM_DISCARDED >> 8))
        {
    //        TRACE("real block at %04x\n", pEntry->addr );
            if (LOCAL_FreeArena( ds, ARENA_HEADER(lpEntry->addr - MOVEABLE_PREFIX) ))
                return handle; /* couldn't free it */
        }
        LOCAL_FreeHandleEntry( ds, handle );
        return 0;  /* OK */
    }
}
