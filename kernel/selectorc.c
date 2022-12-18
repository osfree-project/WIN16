#include <win16.h>

#include <dpmi.h>

#define LDT_FLAGS_DATA      0x13  /* Data segment */
#define LDT_FLAGS_CODE      0x1b  /* Code segment */

/***********************************************************************
 *           SelectorAccessRights   (KERNEL.196)
 */
WORD WINAPI SelectorAccessRights( WORD sel, WORD op, WORD val )
{
    LDT_ENTRY entry;

    if (!DPMI_GetDescriptor(sel, &entry)) return 0;
    if (op == 0)  /* get */
    {
        return entry.HighWord.Bytes.Flags1 | ((entry.HighWord.Bytes.Flags2 & 0xf0) << 8);
    }
    else  /* set */
    {
        entry.HighWord.Bytes.Flags1 = LOBYTE(val) | 0xf0;
        entry.HighWord.Bytes.Flags2 = (entry.HighWord.Bytes.Flags2 & 0x0f) | (HIBYTE(val) & 0xf0);
        DPMI_SetDescriptor(sel, &entry);
        return 0;
    }
}


/***********************************************************************
 *           IsBadCodePtr   (KERNEL.336)
 */
BOOL WINAPI IsBadCodePtr( FARPROC ptr )
{
    WORD sel = SELECTOROF( ptr );
    LDT_ENTRY entry;

    if (!DPMI_GetDescriptor(sel, &entry)) return 0;
    /* check for code segment, ignoring conforming, read-only and accessed bits */
    if ((entry.HighWord.Bytes.Flags1 ^ LDT_FLAGS_CODE) & 0x18) return TRUE;
    if (OFFSETOF(ptr) > GetSelectorLimit(sel)) return TRUE;
    return FALSE;
}


/***********************************************************************
 *           IsBadStringPtr   (KERNEL.337)
 */
BOOL WINAPI IsBadStringPtr( const void far * ptr, UINT size )
{
    WORD sel = SELECTOROF( ptr );
    LDT_ENTRY entry;

    if (!DPMI_GetDescriptor(sel, &entry)) return 0;

    /* check for data or readable code segment */
    if (!(entry.HighWord.Bytes.Flags1 & 0x10)) return TRUE;  /* system descriptor */
    if ((entry.HighWord.Bytes.Flags1 & 0x0a) == 0x08) return TRUE;  /* non-readable code segment */
    if (lstrlen(ptr) < size) size = lstrlen(ptr) + 1;
    if (size && (OFFSETOF(ptr) + size - 1 > GetSelectorLimit( sel ))) return TRUE;
    return FALSE;
}


/***********************************************************************
 *           IsBadHugeReadPtr   (KERNEL.346)
 */
BOOL WINAPI IsBadHugeReadPtr( const void huge * ptr, DWORD size )
{
    WORD sel = SELECTOROF( ptr );
    LDT_ENTRY entry;

    if (!DPMI_GetDescriptor(sel, &entry)) return 0;

    /* check for data or readable code segment */
    if (!(entry.HighWord.Bytes.Flags1 & 0x10)) return TRUE;  /* system descriptor */
    if ((entry.HighWord.Bytes.Flags1 & 0x0a) == 0x08) return TRUE;  /* non-readable code segment */
    if (size && (OFFSETOF(ptr) + size - 1 > GetSelectorLimit( sel ))) return TRUE;
    return FALSE;
}


/***********************************************************************
 *           IsBadHugeWritePtr   (KERNEL.347)
 */
BOOL WINAPI IsBadHugeWritePtr( void huge * ptr, DWORD size )
{
    WORD sel = SELECTOROF( ptr );
    LDT_ENTRY entry;

    if (!DPMI_GetDescriptor(sel, &entry)) return 0;

    /* check for writable data segment, ignoring expand-down and accessed flags */
    if ((entry.HighWord.Bytes.Flags1 ^ LDT_FLAGS_DATA) & 0x1a) return TRUE;
    if (size && (OFFSETOF(ptr) + size - 1 > GetSelectorLimit( sel ))) return TRUE;
    return FALSE;
}

/***********************************************************************
 *           IsBadReadPtr   (KERNEL.334)
 */
BOOL WINAPI IsBadReadPtr( const void far * ptr, UINT size )
{
    return IsBadHugeReadPtr( ptr, size );
}


/***********************************************************************
 *           IsBadWritePtr   (KERNEL.335)
 */
BOOL WINAPI IsBadWritePtr( void far * ptr, UINT size )
{
    return IsBadHugeWritePtr( ptr, size );
}


/***********************************************************************
 *           IsBadFlatReadWritePtr   (KERNEL.627)
 */
// @todo Not exported in this version
BOOL WINAPI IsBadFlatReadWritePtr( void far * ptr, DWORD size, BOOL bWrite )
{
    return bWrite? IsBadHugeWritePtr( ptr, size )
                 : IsBadHugeReadPtr( ptr, size );
}
