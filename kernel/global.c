#include <windows.h>

#include <win_private.h>

#include "dpmi.h"

//@todo implement pGlobalArena
/***********************************************************************
 *           GlobalFreeAll   (KERNEL.26)
 */
void WINAPI GlobalFreeAll(HGLOBAL owner)
{
//    int i;
//    GLOBALARENA *pArena;

//    pArena = pGlobalArena;
//    for (i = 0; i < globalArenaSize; i++, pArena++)
//    {
//        if ((pArena->size != 0) && (pArena->hOwner == owner))
//            GlobalFree( pArena->handle );
//    }
}

//#define VALID_HANDLE(handle) (((handle)>>__AHSHIFT)<globalArenaSize)

// @todo implementa pGlobalArena
/***********************************************************************
 *           GlobalFlags     (KERNEL.22)
 *
 * Get information about a global memory object.
 *
 * NOTES
 *	Should this return GMEM_INVALID_HANDLE instead of 0 on invalid
 *	handle?
 *
 * RETURNS
 *	Value specifying flags and lock count
 *	GMEM_INVALID_HANDLE: Invalid handle
 */
UINT WINAPI GlobalFlags(
              HGLOBAL handle /* [in] Handle of global memory object */
) {
//    GLOBALARENA *pArena;

//    TRACE("%04x\n", handle );
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to GlobalFlags16!\n",handle);
	return 0;
//    }
//    pArena = GET_ARENA_PTR(handle);
//    return pArena->lockCount |
//           ((pArena->flags & GA_DISCARDABLE) ? GMEM_DISCARDABLE : 0) |
//           ((pArena->base == 0) ? GMEM_DISCARDED : 0);
}

/***********************************************************************
 *           GlobalWire     (KERNEL.111)
 */
char FAR * WINAPI GlobalWire( HGLOBAL handle )
{
    return GlobalLock( handle );
}


/***********************************************************************
 *           GlobalUnWire     (KERNEL.112)
 */
BOOL WINAPI GlobalUnWire( HGLOBAL handle )
{
    return !GlobalUnlock( handle );
}

/***********************************************************************
 *           GlobalNotify   (KERNEL.154)
 *
 * Note that GlobalNotify does _not_ return the old NotifyProc
 * -- contrary to LocalNotify !!
 */
VOID WINAPI GlobalNotify( FARPROC proc )
{
    TDB far *pTask;

    if (!(pTask = MAKELP(GetCurrentTask(), 0))) return;
    pTask->discardhandler = proc;
}

/***********************************************************************
 *           LimitEMSPages   (KERNEL.156)
 */
DWORD WINAPI LimitEMSPages( DWORD unused )
{
    return 0;
}

/***********************************************************************
 *           A20Proc   (KERNEL.165)
 */
void WINAPI A20Proc( WORD unused )
{
    /* this is also a NOP in Windows */
}


//@todo implement globalArena
/***********************************************************************
 *           GlobalHandleToSel
 *
 */
WORD WINAPI GlobalHandleToSel(HGLOBAL handle)
{
    if (!handle) return 0;
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to GlobalHandleToSel!\n",handle);
//	return 0;
//    }
    if (!(handle & 7))
    {
//        WARN("Program attempted invalid selector conversion\n" );
        return handle - 1;
    }
    return handle | 7;
}

/***********************************************************************
 *           LockSegment   (KERNEL.23)
 */
HGLOBAL WINAPI LockSegment( HGLOBAL handle )
{
//    TRACE("%04x\n", handle );
    if (handle == (HGLOBAL)-1) handle = GetDS();
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to LockSegment16!\n",handle);
//	return 0;
//    }
//    GET_ARENA_PTR(handle)->lockCount++;
    return handle;
}


/***********************************************************************
 *           UnlockSegment   (KERNEL.24)
 */
void WINAPI UnlockSegment( HGLOBAL handle )
{
//    TRACE("%04x\n", handle );
    if (handle == (HGLOBAL)-1) handle = GetDS();
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to UnlockSegment16!\n",handle);
//	return;
//    }
//    GET_ARENA_PTR(handle)->lockCount--;
    /* FIXME: this ought to return the lock count in CX (go figure...) */
//    SetCX(GET_ARENA_PTR(handle)->lockCount);
}

/************************************************************************
 *              GlobalMasterHandle (KERNEL.28)
 *
 * Should return selector and handle of the information structure for
 * the global heap. selector and handle are stored in the THHOOK as
 * pGlobalHeap and hGlobalHeap.
 *
 */

DWORD WINAPI GlobalMasterHandle(void)
{
	SetKernelDS();
    return MAKELONG(TH_HGLOBALHEAP, TH_PGLOBALHEAP);
}

/***********************************************************************
 *           GlobalFix     (KERNEL.197)
 */
WORD WINAPI GlobalFixReal( HGLOBAL handle )
{
    //TRACE("%04x\n", handle );
    //if (!VALID_HANDLE(handle)) {
	//WARN("Invalid handle 0x%04x passed to GlobalFix16!\n",handle);
	//return 0;
    //}
    //GET_ARENA_PTR(handle)->lockCount++;

    return GlobalHandleToSel(handle);
}


/***********************************************************************
 *           GlobalUnfix     (KERNEL.198)
 */
void WINAPI GlobalUnfix(HGLOBAL handle )
{
//    TRACE("%04x\n", handle );
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to GlobalUnfix16!\n",handle);
//	return;
//    }
//    GET_ARENA_PTR(handle)->lockCount--;
}

#if 0
;--- DWORD GlobalDOSAlloc(DWORD size)
;--- returns selector in ax, segment in dx

GlobalDOSAlloc proc far pascal
	pop bx
	pop cx
	pop ax			;get size into DX:AX
	pop dx
	push cx
	push bx
	mov cl,al
	shr ax,4
	shl dx,12		;skip bits 4-15 of DX
	or ax,dx
	test cl,0Fh
	jz @F
	inc ax
@@:
	@DPMI_DOSALLOC ax	;alloc dos memory
	xchg ax,dx
	jnc @F
	xor ax,ax
@@:
	@return
GlobalDOSAlloc endp

#endif
/***********************************************************************
 *           GlobalDOSAlloc   (KERNEL.184)
 *
 * Allocate memory in the first MB.
 *
 * RETURNS
 *	Address (HW=Paragraph segment; LW=Selector)
 */
DWORD WINAPI GlobalDOSAlloc(
             DWORD size /* [in] Number of bytes to be allocated */
) {
   UINT    uParagraph;
   //LPVOID    lpBlock = DOSMEM_AllocBlock( size, &uParagraph );

   //if( lpBlock )
   {
//       HMODULE hModule = GetModuleHandle("KERNEL");
//       WORD	 wSelector;
//       GLOBALARENA FAR *pArena;

//       wSelector = GLOBAL_CreateBlock(GMEM_FIXED, lpBlock, size, hModule, LDT_FLAGS_DATA );
//       pArena = GET_ARENA_PTR(wSelector);
//       pArena->flags |= GA_DOSMEM;
     //  return MAKELONG(wSelector,uParagraph);
   }
   return 0;
}


/***********************************************************************
 *           GlobalDOSFree      (KERNEL.185)
 *
 * Free memory allocated with GlobalDOSAlloc
 *
 * RETURNS
 *	NULL: Success
 *	sel: Failure
 */
WORD WINAPI GlobalDOSFree(WORD sel /* [in] Selector */)
{
	return DPMI_FreeDOSMem(sel);
}
