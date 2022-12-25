/*
 * osFree Windows Kernel
 *
 * Selector manipulation functions
 *
 * Ported to Watcom C & DPMI by Yuri Prokushev
 *
 * Copyright 1995 Alexandre Julliard
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

#include <win16.h>

#include <dpmi.h>

#define LDT_FLAGS_DATA      0x13  /* Data segment */
#define LDT_FLAGS_CODE      0x1b  /* Code segment */
#define LDT_FLAGS_32BIT     0x40  /* Segment is 32-bit (code or stack) */

static LDT_ENTRY ldt_make_entry( const void *base, unsigned long limit, unsigned char flags )
{
    LDT_ENTRY entry;

    entry.BaseLow                   = (WORD)(DWORD)base;
    entry.HighWord.Bits.BaseMid     = (BYTE)((DWORD)base >> 16);
    entry.HighWord.Bits.BaseHi      = (BYTE)((DWORD)base >> 24);
    if ((entry.HighWord.Bits.Granularity = (limit >= 0x100000))) limit >>= 12;
    entry.LimitLow                  = (WORD)limit;
    entry.HighWord.Bits.LimitHi     = limit >> 16;
    entry.HighWord.Bits.Dpl         = 3;
    entry.HighWord.Bits.Pres        = 1;
    entry.HighWord.Bits.Type        = flags;
    entry.HighWord.Bits.Sys         = 0;
    entry.HighWord.Bits.Reserved_0  = 0;
    entry.HighWord.Bits.Default_Big = (flags & LDT_FLAGS_32BIT) != 0;
    return entry;
}

/* get the number of selectors needed to cover up to the selector limit */
static inline WORD get_sel_count( WORD sel )
{
    return (GetSelectorLimit( sel ) >> 16) + 1;
}

/***********************************************************************
 *           AllocSelectorArray   (KERNEL.206)
 */
WORD WINAPI AllocSelectorArray( WORD count )
{
    WORD i, sel = DPMI_AllocDesc( count );

    if (sel)
    {
        LDT_ENTRY entry = ldt_make_entry( 0, 1, LDT_FLAGS_DATA ); /* avoid 0 base and limit */
        for (i = 0; i < count; i++) DPMI_SetDescriptor( sel + (i << 3), &entry );
    }
    return sel;
}

/***********************************************************************
 *           AllocSelector   (KERNEL.175)
 */
UINT WINAPI AllocSelector( UINT sel )
{
    WORD newsel, count, i;

    count = sel ? get_sel_count(sel) : 1;
    newsel = DPMI_AllocDesc( count );
//    TRACE("(%04x): returning %04x\n", sel, newsel );
    if (!newsel) return 0;
    if (!sel) return newsel;  /* nothing to copy */
    for (i = 0; i < count; i++)
    {
        LDT_ENTRY entry;
        if (!DPMI_GetDescriptor( sel + (i << 3), &entry )) break;
        DPMI_SetDescriptor( newsel + (i << 3), &entry );
    }
    return newsel;
}

/***********************************************************************
 *           FreeSelector   (KERNEL.176)
 */
UINT WINAPI FreeSelector( UINT sel )
{
    DPMI_FreeDesc(sel);
    return 0;
}

/***********************************************************************
 *             GetSelectorBase   (KERNEL.186)
 */
DWORD WINAPI GetSelectorBase( UINT sel )
{
    return DPMI_GetBase( sel );
}

/***********************************************************************
 *             SetSelectorBase   (KERNEL.187)
 */
UINT WINAPI SetSelectorBase( UINT sel, DWORD base )
{
    DPMI_SetBase(sel, base);
    return sel;
}

/***********************************************************************
 *           GetSelectorLimit   (KERNEL.188)
 */
DWORD WINAPI GetSelectorLimit( UINT sel )
{
    LDT_ENTRY entry;

    if (!DPMI_GetDescriptor(sel, &entry)) return 0;

    return (entry.HighWord.Bits.LimitHi<<16+entry.LimitLow);
}


/***********************************************************************
 *           SetSelectorLimit   (KERNEL.189)
 */
UINT WINAPI SetSelectorLimit( UINT sel, DWORD limit )
{
    DPMI_SetLimit(sel, limit);
    return sel;
}


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
 *           AllocCStoDSAlias   (KERNEL.170)
 *           AllocAlias         (KERNEL.172)
 */
WORD WINAPI AllocCStoDSAlias( WORD sel )
{
    return DPMI_CreateCSAlias( sel);
}

/***********************************************************************
 *           AllocDStoCSAlias   (KERNEL.171)
 */
UINT WINAPI AllocDStoCSAlias( UINT sel )
{
    WORD newsel;
    LDT_ENTRY entry;

//    if (!ldt_is_valid( sel )) return 0;
    newsel = AllocSelector( 0 );
//    TRACE("(%04x): returning %04x\n", sel, newsel );
    if (!newsel) return 0;
    entry=ldt_make_entry((void *) GetSelectorBase(sel), GetSelectorLimit(sel), LDT_FLAGS_CODE );
    DPMI_SetDescriptor(newsel, &entry);
    return newsel;
}

/***********************************************************************
 *           PrestoChangoSelector   (KERNEL.177)
 */
UINT WINAPI PrestoChangoSelector( UINT selSrc, UINT selDst )
{
    LDT_ENTRY entry;

//    if (!ldt_is_valid( selSrc )) return selDst;

    DPMI_GetDescriptor( selSrc, &entry);
    /* toggle the executable bit */
    entry.HighWord.Bytes.Flags1=entry.HighWord.Bytes.Flags1 ^ (LDT_FLAGS_CODE ^ LDT_FLAGS_DATA);

    DPMI_SetDescriptor( selDst, &entry);
    return selDst;
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

void WINAPI LongPtrAdd(DWORD dwLongPtr, DWORD dwAdd)
{
  WORD wSel = SELECTOROF(dwLongPtr);
  SetSelectorBase(wSel, GetSelectorBase(wSel)+dwAdd);
}

