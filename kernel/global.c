#include <windows.h>

#include <win_private.h>

#include "dpmi.h"

//@todo implement pGlobalArena
/***********************************************************************
 *           GlobalFreeAll   (KERNEL.26)
 */
void WINAPI GlobalFreeAll(HGLOBAL owner)
{
	FUNCTIONSTART;
//    int i;
//    GLOBALARENA *pArena;

//    pArena = pGlobalArena;
//    for (i = 0; i < globalArenaSize; i++, pArena++)
//    {
//        if ((pArena->size != 0) && (pArena->hOwner == owner))
//            GlobalFree( pArena->handle );
//    }
	FUNCTIONEND;
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
	FUNCTIONSTART;
//    GLOBALARENA *pArena;

//    TRACE("%04x\n", handle );
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to GlobalFlags16!\n",handle);
	FUNCTIONEND;
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
char FAR * WINAPI GlobalWire(HGLOBAL handle)
{
	LPSTR lock;
	FUNCTIONSTART;
	lock=GlobalLock(handle);
	FUNCTIONEND;
    return lock;
}


/***********************************************************************
 *           GlobalUnWire     (KERNEL.112)
 */
BOOL WINAPI GlobalUnWire(HGLOBAL handle)
{
	BOOL res;

	FUNCTIONSTART;

	res=!GlobalUnlock(handle);

	FUNCTIONEND;
    return res;
}

/***********************************************************************
 *           GlobalNotify   (KERNEL.154)
 *
 * Note that GlobalNotify does _not_ return the old NotifyProc
 * -- contrary to LocalNotify !!
 */
VOID WINAPI GlobalNotify( FARPROC proc )
{
    TDB FAR *pTask;

	FUNCTIONSTART;

    if (!(pTask = MAKELP(GetCurrentTask(), 0))) return;
    pTask->discardhandler = proc;
	FUNCTIONEND;
}

/***********************************************************************
 *           LimitEMSPages   (KERNEL.156)
 */
DWORD WINAPI LimitEMSPages( DWORD unused )
{
	FUNCTIONSTART;
	FUNCTIONEND;
    return 0;
}

/***********************************************************************
 *           A20Proc   (KERNEL.165)
 */
void WINAPI A20Proc( WORD unused )
{
	FUNCTIONSTART;
	FUNCTIONEND;
    /* this is also a NOP in Windows */
}

/***********************************************************************
 *		ValidateCodeSegments (KERNEL.100)
 */
void WINAPI ValidateCodeSegments(void)
{
	FUNCTIONSTART;
	FUNCTIONEND;
}


//@todo implement globalArena
/***********************************************************************
 *           GlobalHandleToSel
 *
 */
WORD WINAPI GlobalHandleToSel(HGLOBAL handle)
{
	FUNCTIONSTART;
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
	FUNCTIONEND;
    return handle | 7;
}

/***********************************************************************
 *           LockSegment   (KERNEL.23)
 */
HGLOBAL WINAPI LockSegment( HGLOBAL handle )
{
	FUNCTIONSTART;
//    TRACE("%04x\n", handle );
    if (handle == (HGLOBAL)-1) handle = GetDS();
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to LockSegment16!\n",handle);
//	return 0;
//    }
//    GET_ARENA_PTR(handle)->lockCount++;
	FUNCTIONEND;
    return handle;
}


/***********************************************************************
 *           UnlockSegment   (KERNEL.24)
 */
void WINAPI UnlockSegment( HGLOBAL handle )
{
	FUNCTIONSTART;
//    TRACE("%04x\n", handle );
    if (handle == (HGLOBAL)-1) handle = GetDS();
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to UnlockSegment16!\n",handle);
//	return;
//    }
//    GET_ARENA_PTR(handle)->lockCount--;
    /* FIXME: this ought to return the lock count in CX (go figure...) */
//    SetCX(GET_ARENA_PTR(handle)->lockCount);
	FUNCTIONEND;
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
	FUNCTIONSTART;
	SetKernelDS();
	FUNCTIONEND;
    return MAKELONG(TH_HGLOBALHEAP, TH_PGLOBALHEAP);
}

/***********************************************************************
 *           GlobalFix     (KERNEL.197)
 */
WORD WINAPI GlobalFixReal(HGLOBAL handle)
{
	WORD sel;
	FUNCTIONSTART;
    //TRACE("%04x\n", handle );
    //if (!VALID_HANDLE(handle)) {
	//WARN("Invalid handle 0x%04x passed to GlobalFix16!\n",handle);
	//return 0;
    //}
    //GET_ARENA_PTR(handle)->lockCount++;
	sel=GlobalHandleToSel(handle);

	FUNCTIONEND;
    return sel;
}


/***********************************************************************
 *           GlobalUnfix     (KERNEL.198)
 */
void WINAPI GlobalUnfix(HGLOBAL handle )
{
	FUNCTIONSTART;
//    TRACE("%04x\n", handle );
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to GlobalUnfix16!\n",handle);
//	return;
//    }
//    GET_ARENA_PTR(handle)->lockCount--;
	FUNCTIONEND;
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
DWORD WINAPI GlobalDOSAlloc(DWORD size /* [in] Number of bytes to be allocated */)
{
	UINT    uParagraph;
	FUNCTIONSTART;
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
	FUNCTIONEND;
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
	WORD	res;
	
	FUNCTIONSTART;
	res=DPMI_FreeDOSMem(sel);
	FUNCTIONEND;
	return res;
}

#if 0
GetFreeMemInfo proc far pascal uses ds
local buf[30h]: BYTE
	@SetKernelDS
	mov ax, -1
	mov dx, -1
	test word ptr eWinFlags, WF_PAGING
	jz exit
	lea di, buf
	push ss
	pop es
	@DPMI_GETFREEMEMINFO
	jc exit
	mov ax, es:[di][10h]
	mov bx, es:[di][14h]
exit:
	ret
GetFreeMemInfo endp
#endif
/***********************************************************************
 *           GetFreeMemInfo   (KERNEL.316)
 */
DWORD WINAPI GetFreeMemInfo(void)
{
	FUNCTIONSTART;
//    SYSTEM_BASIC_INFORMATION info;
//    MEMORYSTATUS status;
//
//    NtQuerySystemInformation( SystemBasicInformation, &info, sizeof(info), NULL );
//    GlobalMemoryStatus( &status );
//    return MAKELONG( status.dwTotalVirtual / info.PageSize, status.dwAvailVirtual / info.PageSize );
	FUNCTIONEND;
}

#if 0
GetFreeSpace proc far pascal
	push es
	push di
	sub sp,48
	mov di,sp
	push ss
	pop es
	@DPMI_GETFREEMEMINFO
	pop ax		;get the first dword in DX:AX
	pop dx
	add sp,48-4
	pop di
	pop es
	retf 2
GetFreeSpace endp
#endif

/***********************************************************************
 *           GetFreeSpace   (KERNEL.169)
 */
DWORD WINAPI GetFreeSpace( UINT wFlags )
{
	FUNCTIONSTART;
//    MEMORYSTATUS ms;
//    GlobalMemoryStatus( &ms );
//    return min( ms.dwAvailVirtual, MAXLONG );
	FUNCTIONEND;
}

#if 0
GlobalSize proc far pascal
	pop bx
	pop cx
	pop ax
	push cx
	push bx
if ?32BIT
	lsl eax,eax
	jnz @F
	push eax
	pop ax
	pop dx
else
	xor dx,dx
	lsl ax,ax
	jnz @F
endif
	add ax,1
	adc dx,0
	jmp exit
@@:
	xor ax,ax
	cwd
exit:
	@return
GlobalSize endp
#endif

/***********************************************************************
 *           GlobalSize     (KERNEL.20)
 * 
 * Get the current size of a global memory object.
 *
 * RETURNS
 *	Size in bytes of object
 *	0: Failure
 */
DWORD WINAPI GlobalSize(
             HGLOBAL handle /* [in] Handle of global memory object */
) {
	FUNCTIONSTART;
//    TRACE("%04x\n", handle );
    if (!handle) return 0;
//    if (!VALID_HANDLE(handle))
//	return 0;
//    return GET_ARENA_PTR(handle)->size;
	FUNCTIONEND;
}

#if 0
GlobalHandle proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	mov dx,ax
	@return
GlobalHandle endp
#endif

/***********************************************************************
 *           GlobalHandle   (KERNEL.21)
 *
 * Get the handle associated with a pointer to the global memory block.
 *
 * NOTES
 *	Why is GlobalHandleToSel used here with the sel as input?
 *
 * RETURNS
 *	Handle: Success
 *	NULL: Failure
 */
DWORD WINAPI GlobalHandle(
             UINT sel /* [in] Address of global memory block */
) {
	FUNCTIONSTART;
    //TRACE("%04x\n", sel );
    //if (!VALID_HANDLE(sel)) {
	//WARN("Invalid handle 0x%04x passed to GlobalHandle!\n",sel);
	//return 0;
    //}
    //return MAKELONG( GET_ARENA_PTR(sel)->handle, GlobalHandleToSel(sel) );
	FUNCTIONEND;
}

char FAR * WINAPI K32WOWGlobalLock( HGLOBAL handle )
{
	FUNCTIONSTART;
//    WORD sel = GlobalHandleToSel( handle );
//    TRACE("(%04x) -> %08lx\n", handle, MAKELONG( 0, sel ) );

    //if (handle)
    //{
	//if (handle == (HGLOBAL)-1) handle = GetDS();

	//if (!VALID_HANDLE(handle)) {
	    //WARN("Invalid handle 0x%04x passed to WIN16_GlobalLock16!\n",handle);
	    //sel = 0;
	//}
	//else if (!GET_ARENA_PTR(handle)->base)
            //sel = 0;
        //else
            //GET_ARENA_PTR(handle)->lockCount++;
    //}
//
    //return MAKESEGPTR( sel, 0 );
	FUNCTIONEND;
}

#if 0
GlobalLock proc far pascal
	pop cx
	pop bx
	pop dx
	push bx
	push cx
	xor ax,ax
	verr dx
	jnz @F
	retf
@@:
	xor dx,dx
	retf
GlobalLock endp
#endif

/***********************************************************************
 *           GlobalLock   (KERNEL.18)
 *
 * This is the GlobalLock16() function used by 16-bit code.
 */
char FAR * WINAPI GlobalLock( HGLOBAL handle )
{
	FUNCTIONSTART;
//    SEGPTR ret = K32WOWGlobalLock( handle );
//    CURRENT_STACK16->ecx = SELECTOROF(ret);  /* selector must be returned in CX as well */
//    return ret;
	FUNCTIONEND;
}

#if 0
GlobalUnlock proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	@return
GlobalUnlock endp

#endif
/***********************************************************************
 *           GlobalUnlock     (KERNEL.19)
 * NOTES
 *	Should the return values be cast to booleans?
 *
 * RETURNS
 *	TRUE: Object is still locked
 *	FALSE: Object is unlocked
 */
BOOL WINAPI GlobalUnlock(
              HGLOBAL handle /* [in] Handle of global memory object */
) {
	FUNCTIONSTART;
    //GLOBALARENA *pArena = GET_ARENA_PTR(handle);
    //if (!VALID_HANDLE(handle)) {
	//WARN("Invalid handle 0x%04x passed to GlobalUnlock16!\n",handle);
        //return FALSE;
    //}
    //TRACE("%04x\n", handle );
    //if (pArena->lockCount) pArena->lockCount--;
    //return pArena->lockCount;
	FUNCTIONEND;
}
