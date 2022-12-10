#include <win16.h>

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

