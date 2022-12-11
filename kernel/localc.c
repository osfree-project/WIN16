#include <win16.h>
#include <win_private.h>

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
    FARPROC notify;           /* 1e Pointer to LocalNotify() function */
    WORD lock;                  /* 22 Lock count for the heap */
    WORD extra;                 /* 24 Extra bytes to allocate when expanding */
    WORD minsize;               /* 26 Minimum size of the heap */
    WORD magic;                 /* 28 Magic number */
} LOCALHEAPINFO;

#define HANDLE_MOVEABLE(handle) (((handle) & 3) == 2)

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

/***********************************************************************
 *           LocalFlags   (KERNEL.12)
 */
UINT WINAPI LocalFlags( HLOCAL handle )
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
    LOCALHEAPINFO *pInfo;

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

