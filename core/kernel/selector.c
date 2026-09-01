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

#include <windows.h>

#include <stdarg.h>

#include "dpmi.h"
#include "win_private.h"

/* DPMI-хэндл Burgermaster (32 бита) */
DWORD hBMDPMI = 0;


#define LDT_FLAGS_DATA      0x13  /* Data segment */
#define LDT_FLAGS_CODE      0x1b  /* Code segment */
#define LDT_FLAGS_32BIT     0x40  /* Segment is 32-bit (code or stack) */

static WORD FirstFreeSel = 0;   /* первый свободный селектор */
static WORD CountFreeSel = 0;   /* количество свободных селекторов */

/**
 * @brief Construct an LDT descriptor entry from a base address, limit and flags.
 *
 * Builds a 32-bit ring-3 segment descriptor (present, DPL=3, not system).
 * If the limit is >= 0x100000 the granularity bit is set and the limit is
 * scaled down to 4K units. The `Default_Big` bit is set when `LDT_FLAGS_32BIT`
 * is present in `flags`.
 *
 * @param base    Linear base address of the segment.
 * @param limit   Segment limit in bytes (or 4K pages if >= 0x100000).
 * @param flags   Type flags (e.g. LDT_FLAGS_DATA, LDT_FLAGS_CODE, LDT_FLAGS_32BIT).
 * @return        Initialised LDT_ENTRY structure.
 */
static LDT_ENTRY ldt_make_entry( const void *base, unsigned long limit, unsigned char flags )
{
	LDT_ENTRY entry;

	FUNCTIONSTART;

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

	FUNCTIONEND;

	return entry;
}


/**
 * @brief Return a selector to the free pool or to DPMI.
 *
 * If the selector index is outside the local selector table, it is released
 * directly with DPMI_FreeDesc(). Otherwise the selector is linked into the
 * internal free list: the current `FirstFreeSel` value is stored in the
 * descriptor’s LimitLow field, `FirstFreeSel` is updated to this selector,
 * and `CountFreeSel` is incremented.
 *
 * @param sel   Selector to free.
 */
static void Free_Sel(WORD sel)
{
    LDT_ENTRY entry;

    if ((sel >> 3) >= (SelTableLen / 4))
    {
        DPMI_FreeDesc(sel);
        return;
    }

    if (DPMI_GetDescriptor(sel, &entry) == 0)
    {
        entry.LimitLow = FirstFreeSel;
        DPMI_SetDescriptor(sel, &entry);
        FirstFreeSel = sel;
        CountFreeSel++;
    }
}

/**
 * @brief Allocate one or more consecutive selectors.
 *
 * For a single selector, first tries the internal free list; if empty, calls
 * DPMI_AllocDesc(1). For `count > 1`, searches the free list for a contiguous
 * run of `count` selectors, unlinks it, and returns the first selector. If no
 * such run exists, falls back to DPMI_AllocDesc(count). When the free pool is
 * low (fewer than 0x100 selectors) and the system is running in enhanced mode,
 * an additional block of 0x100 selectors is allocated and added to the pool.
 * Finally, if the returned selectors would exceed the local selector table,
 * they are freed via DPMI and zero is returned.
 *
 * @param count   Number of consecutive selectors required.
 * @return        First selector of the allocated block, or 0 on failure.
 */
static WORD Get_Sel(WORD count)
{
    WORD ret = 0;
    LDT_ENTRY entry;
    WORD cur, prev, start, run, expected, prev_before_start, next;
    BOOL found;

    if (count == 1)
    {
        if (FirstFreeSel)
        {
            ret = FirstFreeSel;
            if (DPMI_GetDescriptor(ret, &entry) == 0)
                FirstFreeSel = entry.LimitLow;
            else
                ret = 0;
        }
        else
        {
            ret = DPMI_AllocDesc(1);
        }
    }
    else
    {
        /* Поиск непрерывного блока в списке */
        cur = FirstFreeSel;
        prev = 0;
        start = 0;
        run = 0;
        expected = 0;
        prev_before_start = 0;
        found = FALSE;

        while (cur)
        {
            if (!DPMI_GetDescriptor(cur, &entry))
                break;

            if (run == 0)
            {
                start = cur;
                run = 1;
                expected = cur + __AHSHIFT;
                prev_before_start = prev;
            }
            else if (cur == expected)
            {
                run++;
                expected += __AHSHIFT;
                if (run == count)
                {
                    found = TRUE;
                    break;
                }
            }
            else
            {
                start = cur;
                run = 1;
                expected = cur + __AHSHIFT;
                prev_before_start = prev;
            }

            prev = cur;
            cur = entry.LimitLow;
        }

        if (found)
        {
            WORD last = start + (count - 1) * __AHSHIFT;
            if (!DPMI_GetDescriptor(last, &entry))
                return 0;
            next = entry.LimitLow;

            if (start == FirstFreeSel)
                FirstFreeSel = next;
            else
            {
                LDT_ENTRY prevEntry;
                if (!DPMI_GetDescriptor(prev_before_start, &prevEntry))
                    return 0;
                prevEntry.LimitLow = next;
                DPMI_SetDescriptor(prev_before_start, &prevEntry);
            }
            CountFreeSel -= count;
            ret = start;
        }
        else
        {
            ret = DPMI_AllocDesc(count);
        }
    }

    /* Пополнение пула при необходимости */
    if (CountFreeSel < 0x100 && (WinFlags & WF_ENHANCED))
    {
        WORD block = DPMI_AllocDesc(0x100);
        if (block)
        {
            WORD i;
            for (i = 0; i < 0x100; i++)
                Free_Sel(block + i * __AHSHIFT);
        }
    }

    /* Проверка выхода за пределы таблицы селекторов */
    if (ret && ((DWORD)(ret >> 3) + count > (SelTableLen / 4)))
    {
        WORD i;
        for (i = 0; i < count; i++)
        {
            WORD s = ret + i * __AHSHIFT;
            /* Освобождаем все селекторы через DPMI, как в оригинале */
            DPMI_FreeDesc(s);
        }
        ret = 0;
    }

    return ret;
}


/**
 * @brief Store the arena offset for a selector in the selector table.
 *
 * Writes `arenaOffset` into the 32-bit slot of the global heap’s selector
 * table that corresponds to `sel`. The table resides at `SelTableStart`
 * inside the Burgermaster segment (`TH_PGLOBALHEAP`). Selectors outside the
 * table bounds are ignored.
 *
 * @param sel          Selector whose arena offset is to be set.
 * @param arenaOffset  Offset of the corresponding arena within the global heap.
 */
void SetSelectorArena(WORD sel, DWORD arenaOffset)
{
    DWORD index;
    DWORD far *table;

    if (!TH_PGLOBALHEAP || !SelTableLen)
        return;

    index = (sel >> 3);
    if (index >= (SelTableLen / 4))
        return;

    table = (DWORD far *)MAKELP(TH_PGLOBALHEAP, SelTableStart);
    table[index] = arenaOffset;
}

/**
 * @brief Retrieve the arena offset stored for a selector.
 *
 * Reads the 32-bit value from the selector table. Returns 0 if the selector
 * lies outside the table or if the table is not initialised.
 *
 * @param sel   Selector to query.
 * @return      Arena offset, or 0 if unavailable.
 */
DWORD GetSelectorArena(WORD sel)
{
    DWORD index;
    DWORD far *table;

    if (!TH_PGLOBALHEAP || !SelTableLen)
        return 0;

    index = (sel >> 3);
    if (index >= (SelTableLen / 4))
        return 0;

    table = (DWORD far *)MAKELP(TH_PGLOBALHEAP, SelTableStart);
    return table[index];
}


/***********************************************************************
 *           AllocSelectorArray   (KERNEL.206)
 *
 * @brief Allocate a block of consecutive selectors.
 *
 * Allocates `count` adjacent selectors and initialises each one as a data
 * segment with base 0, limit 1 (bytes), DPL=3, and present. The selectors
 * are spaced by `__AHINCR` bytes as required by the Windows selector
 * architecture. Each selector’s arena entry is set to 0.
 *
 * @param count   Number of selectors to allocate (must be > 0).
 * @return        First selector of the array, or 0 on failure.
 */
WORD WINAPI AllocSelectorArray(WORD count)
{
	WORD i, sel;

	FUNCTIONSTART;

	sel = Get_Sel(count);

	if (sel)
	{
		LDT_ENTRY entry = ldt_make_entry(0, 1, LDT_FLAGS_DATA );
		for (i = 0; i < count; i++)
		{
			DPMI_SetDescriptor( sel + (i << __AHSHIFT), &entry );
			SetSelectorArena(sel + (i << __AHSHIFT), 0);
		}
	}
	FUNCTIONEND;
	return sel;
}

/***********************************************************************
 *           AllocSelector   (KERNEL.175)
 *
 * @brief Allocate a new selector, optionally copying an existing one.
 *
 * If `sel` is 0, allocates one uninitialised selector (or a block large enough
 * to cover the default limit) and returns it. Otherwise determines how many
 * 64K “tiles” are needed to represent the limit of `sel`
 * (`(limit >> 16) + 1`), allocates that many consecutive selectors, and copies
 * the descriptor of each tile. For each tile the base address is adjusted to
 * `base + i*0x10000`, so that the whole range is covered by consecutive
 * selectors. Arena entries are reset to 0.
 *
 * @param sel   Selector to copy, or 0 for a new uninitialised selector.
 * @return      New selector (or first of a block), or 0 on failure.
 */
UINT WINAPI AllocSelector(UINT sel)
{
	WORD newsel, count, i;
	DWORD base;

	FUNCTIONSTART;

	/* get the number of selectors needed to cover up to the selector limit */
	count = sel ? ((GetSelectorLimit(sel) >> 16) + 1) : 1;
	newsel = Get_Sel(count);
//    TRACE("(%04x): returning %04x\n", sel, newsel );
	if (!newsel) return 0;
	if (!sel)
	{
		for (i = 0; i < count; i++)
			SetSelectorArena(newsel + (i << __AHSHIFT), 0);
		return newsel;  /* nothing to copy */
	}

	base = GetSelectorBase(sel);
	for (i = 0; i < count; i++)
	{
		LDT_ENTRY entry;
		if (!DPMI_GetDescriptor( sel + (i << __AHSHIFT ), &entry )) break;
		DPMI_SetDescriptor( newsel + (i << __AHSHIFT ), &entry );
		/* Тайлинг баз: каждый следующий селектор +64К */
		SetSelectorBase( newsel + (i << __AHSHIFT ), base + i * 0x10000 );
		SetSelectorArena(newsel + (i << __AHSHIFT), 0);	}
	FUNCTIONEND;
	return newsel;
}

/***********************************************************************
 *           FreeSelector   (KERNEL.176)
 *
 * @brief Free a selector (or a block of tiled selectors).
 *
 * Determines how many consecutive selectors were allocated together. If the
 * descriptor is present, the number is derived from the limit
 * (`(limit >> 16) + 1`). If the segment is not present (discarded), the
 * count is stored in the low byte of the base field (shifted left by 8).
 * Frees all involved selectors using `Free_Sel` and clears their arena slots.
 *
 * @param sel   First selector of the block to free.
 * @return      0 on success, or `sel` if an error occurred (e.g. invalid selector).
 */
UINT WINAPI FreeSelector( UINT sel )
{
	WORD count = 1;
	DWORD limit;
	DWORD base;
	WORD i;
	LDT_ENTRY entry;

	FUNCTIONSTART;

	if (!sel) return 0;

	/* Проверяем, присутствует ли сегмент в памяти */
	if (DPMI_GetDescriptor(sel, &entry) != 0)
	{
		/* Ошибка получения дескриптора — освобождаем только один */
		count = 1;
	}
	else
	{
		if (entry.HighWord.Bits.Pres)
		{
			/* Сегмент присутствует — считаем по лимиту */
			limit = GetSelectorLimit(sel);
			if (limit)
				count = (limit >> 16) + 1;
		}
		else
		{
			/* Сегмент отсутствует (discarded) — число селекторов хранится в базе */
			base = DPMI_GetBase(sel);
			count = (WORD)(base) >> 8;
		}
	}

	for (i = 0; i < count; i++)
	{
		SetSelectorArena(sel + (i << __AHSHIFT), 0);
		Free_Sel( sel + (i << __AHSHIFT) );
	}
	FUNCTIONEND;

	return 0;
}

/***********************************************************************
 *             GetSelectorBase   (KERNEL.186)
 *
 * @brief Return the 32-bit linear base address of a selector.
 *
 * @param sel   Selector to query.
 * @return      Base address, or 0 if the selector is invalid.
 */
DWORD WINAPI GetSelectorBase(UINT sel)
{
	DWORD res;

	FUNCTIONSTART;

	res=DPMI_GetBase(sel);

	FUNCTIONEND;

	return res;
}

/***********************************************************************
 *             SetSelectorBase   (KERNEL.187)
 *
 * @brief Set the 32-bit linear base address of a selector.
 *
 * @param sel    Selector to modify.
 * @param base   New base address.
 * @return       The selector on success, or 0 if the selector is invalid.
 */
UINT WINAPI SetSelectorBase( UINT sel, DWORD base )
{
	FUNCTIONSTART;

	DPMI_SetBase(sel, base);

	FUNCTIONEND;

	return sel;
}

/***********************************************************************
 *           GetSelectorLimit   (KERNEL.188)
 *
 * @brief Return the raw 20-bit limit field of a selector.
 *
 * Reads the descriptor and combines the high and low limit parts. Note that
 * this function does **not** apply the granularity bit – the returned value
 * is always the raw 20-bit limit field (0 – 0xFFFFF). For a limit set with
 * granularity enabled (>= 1 MB), the actual byte limit would be
 * `(raw_limit + 1) * 4096 - 1`.
 *
 * @param sel   Selector to query.
 * @return      Raw limit field, or 0 if the selector is invalid.
 */
DWORD WINAPI GetSelectorLimit( UINT sel )
{
	LDT_ENTRY entry;

	FUNCTIONSTART;

	if (!DPMI_GetDescriptor(sel, &entry)) return 0;

	FUNCTIONEND;

	return ((DWORD)entry.HighWord.Bits.LimitHi << 16) + entry.LimitLow;
}


/***********************************************************************
 *           SetSelectorLimit   (KERNEL.189)
 *
 * @brief Set the limit of a selector.
 *
 * Calls DPMI to update the descriptor’s limit. The granularity is automatically
 * adjusted by DPMI if necessary.
 *
 * @param sel     Selector to modify.
 * @param limit   New limit in bytes.
 * @return        The selector on success, or 0 if the selector is invalid.
 */
UINT WINAPI SetSelectorLimit( UINT sel, DWORD limit )
{
	FUNCTIONSTART;

	DPMI_SetLimit(sel, limit);

	FUNCTIONEND;

	return sel;
}


/***********************************************************************
 *           SelectorAccessRights   (KERNEL.196)
 *
 * @brief Read or modify the access rights (type) byte(s) of a selector.
 *
 * When `op == 0`, returns the combined access rights word (bits 0–7 and the
 * high nibble of bits 8–15 from the descriptor’s Flags1/Flags2 fields).
 * When `op != 0`, the access rights are set from `val`: the low byte replaces
 * Flags1 (with the lower 4 bits forced to 0xF0), and the high nibble of
 * `val` is merged into the upper nibble of Flags2.
 *
 * @param sel   Selector to inspect or modify.
 * @param op    Operation: 0 = get, non-zero = set.
 * @param val   New access rights (used only when `op != 0`).
 * @return      Current access rights when `op == 0`; otherwise 0.
 */
WORD WINAPI SelectorAccessRights( WORD sel, WORD op, WORD val )
{
	LDT_ENTRY entry;

	FUNCTIONSTART;

	if (!DPMI_GetDescriptor(sel, &entry)) return 0;
	if (op == 0)  /* get */
	{
		FUNCTIONEND;
		return entry.HighWord.Bytes.Flags1 | ((entry.HighWord.Bytes.Flags2 & 0xf0) << 8);
	}
	else  /* set */
	{
		entry.HighWord.Bytes.Flags1 = LOBYTE(val) | 0xf0;
		entry.HighWord.Bytes.Flags2 = (entry.HighWord.Bytes.Flags2 & 0x0f) | (HIBYTE(val) & 0xf0);
		DPMI_SetDescriptor(sel, &entry);
		FUNCTIONEND;
		return 0;
	}
}

/***********************************************************************
 *           AllocCStoDSAlias   (KERNEL.170)
 *           AllocAlias         (KERNEL.172)
 *
 * @brief Create a data-segment alias for a code selector.
 *
 * Allocates a new selector from the internal pool, then initialises it as a
 * data segment with the same base and limit as the source code selector.
 * The arena entry of the new selector is set to 0.
 *
 * @param sel   Code selector for which a data alias is required.
 * @return      New data alias selector, or 0 on failure.
 */
WORD WINAPI AllocCStoDSAlias(WORD sel)
{
 	WORD newsel;
 	LDT_ENTRY entry;

	FUNCTIONSTART;

//	res = DPMI_CreateCSAlias(sel);

	/* Создаём data alias для кодового сегмента, используя собственный
	 * пул селекторов (Get_Sel). Это обеспечивает попадание селектора
	 * в free-list и таблицу селекторов, что необходимо для корректного
	 * освобождения через FreeSelector. Прямой вызов DPMI Create Alias
	 * (Int 31h AX=000Ah) дал бы селектор, не учтённый в пуле KERNEL,
	 * и при освобождении мог бы нарушить целостность списка. */
 	newsel = AllocSelector(0);
 	if (!newsel) return 0;
 
 	entry = ldt_make_entry(
 		(void *) GetSelectorBase(sel),
 		GetSelectorLimit(sel),
 		LDT_FLAGS_DATA
 	);
 	DPMI_SetDescriptor(newsel, &entry);
 	SetSelectorArena(newsel, 0);

	FUNCTIONEND;

 	return newsel;
}

/***********************************************************************
 *           AllocDStoCSAlias   (KERNEL.171)
 *
 * @brief Create a code-segment alias for a data selector.
 *
 * Verifies that `sel` refers to a block in the global heap. If the block is
 * movable and not locked, it is fixed first. Then a new selector is allocated
 * and set up as a code segment with the same base and limit as the original
 * data selector. The arena pointer is copied from the source selector.
 *
 * @param sel   Data selector for which a code alias is required.
 * @return      New code alias selector, or 0 on failure.
 */
UINT WINAPI AllocDStoCSAlias( UINT sel )
{
	WORD newsel;
	LDT_ENTRY entry;
	WORD dataHandle;

	FUNCTIONSTART;

	/* Проверяем, что селектор данных корректен и принадлежит глобальной куче */
	dataHandle = GlobalHandle(sel);
	if (!dataHandle)
		return 0;

	/* Если блок не FIXED и не заблокирован, фиксируем его */
	if (!(dataHandle & 1) && (LOBYTE(GlobalFlags(dataHandle)) == 0))
	{
		GlobalFixReal(dataHandle);
	}

    newsel = AllocSelector( 0 );
//    TRACE("(%04x): returning %04x\n", sel, newsel );
	if (!newsel) return 0;
	entry=ldt_make_entry((void *) GetSelectorBase(sel), GetSelectorLimit(sel), LDT_FLAGS_CODE );
	DPMI_SetDescriptor(newsel, &entry);
	SetSelectorArena(newsel, GetSelectorArena(sel));

	FUNCTIONEND;

	return newsel;
}

/***********************************************************************
 *           PrestoChangoSelector   (KERNEL.177)
 *
 * @brief Toggle the code/data bit of a selector and copy the result.
 *
 * Reads the descriptor of `selSrc`, flips the executable bit (bit 3 of the
 * type field), and writes the modified descriptor to `selDst`. The base,
 * limit, and other attributes remain unchanged.
 *
 * @param selSrc   Source selector whose type is to be toggled.
 * @param selDst   Destination selector that receives the modified descriptor.
 * @return         `selDst` on success, or 0 if `selSrc` is invalid.
 */
UINT WINAPI PrestoChangoSelector( UINT selSrc, UINT selDst )
{
    LDT_ENTRY entry;

	FUNCTIONSTART;

//    if (!ldt_is_valid( selSrc )) return selDst;

    DPMI_GetDescriptor( selSrc, &entry);
    /* toggle the executable bit */
    entry.HighWord.Bytes.Flags1=entry.HighWord.Bytes.Flags1 ^ (LDT_FLAGS_CODE ^ LDT_FLAGS_DATA);

    DPMI_SetDescriptor( selDst, &entry);

	FUNCTIONEND;

    return selDst;
}


/***********************************************************************
 *           IsBadCodePtr   (KERNEL.336)
 *
 * @brief Determine whether a far pointer is a valid code pointer.
 *
 * Checks that the selector is present, describes a code segment (ignoring
 * conforming, read-only, and accessed bits), and that the offset lies within
 * the segment limit.
 *
 * @param ptr   Far pointer to a code location.
 * @return      TRUE if the pointer is bad, FALSE if it is valid.
 */
BOOL WINAPI IsBadCodePtr( FARPROC ptr )
{
    WORD sel = SELECTOROF( ptr );
    LDT_ENTRY entry;

	FUNCTIONSTART;

    if (!DPMI_GetDescriptor(sel, &entry)) return FALSE;

    /* check for code segment, ignoring conforming, read-only and accessed bits */
    if ((entry.HighWord.Bytes.Flags1 ^ LDT_FLAGS_CODE) & 0x18) return TRUE;
    if (OFFSETOF(ptr) > GetSelectorLimit(sel)) return TRUE;
    return FALSE;
}


/***********************************************************************
 *           IsBadStringPtr   (KERNEL.337)
 *
 * @brief Determine whether a far pointer points to a valid string of at least `size` bytes.
 *
 * First checks that the selector is valid and describes a readable segment
 * (data or readable code). Then, if the actual length of the string (via
 * `lstrlen`) is less than `size`, the check is performed over the string
 * length plus one (for the terminating null). Finally verifies that
 * `offset + size - 1` does not exceed the segment limit.
 *
 * @param ptr    Far pointer to a null-terminated string.
 * @param size   Minimum number of bytes to validate.
 * @return       TRUE if the pointer is bad, FALSE otherwise.
 */
BOOL WINAPI IsBadStringPtr( const void far * ptr, UINT size )
{
    WORD sel = SELECTOROF( ptr );
    LDT_ENTRY entry;

	FUNCTIONSTART;

    if (!DPMI_GetDescriptor(sel, &entry)) return FALSE;

    /* check for data or readable code segment */
    if (!(entry.HighWord.Bytes.Flags1 & 0x10)) return TRUE;  /* system descriptor */
    if ((entry.HighWord.Bytes.Flags1 & 0x0a) == 0x08) return TRUE;  /* non-readable code segment */
    if (lstrlen(ptr) < size) size = lstrlen(ptr) + 1;
    if (size && (OFFSETOF(ptr) + size - 1 > GetSelectorLimit( sel ))) return TRUE;
    return FALSE;
}


/***********************************************************************
 *           IsBadHugeReadPtr   (KERNEL.346)
 *
 * @brief Determine whether a huge pointer can be read for `size` bytes.
 *
 * Checks the selector’s type (data or readable code, not system) and verifies
 * that the address range from the pointer’s offset to `offset + size - 1`
 * falls within the segment limit.
 *
 * @param ptr    Huge pointer to check.
 * @param size   Number of bytes that must be readable.
 * @return       TRUE if the pointer is bad, FALSE otherwise.
 */
BOOL WINAPI IsBadHugeReadPtr( const void huge * ptr, DWORD size )
{
    WORD sel = SELECTOROF( ptr );
    LDT_ENTRY entry;

	FUNCTIONSTART;

    if (!DPMI_GetDescriptor(sel, &entry)) return FALSE;

    /* check for data or readable code segment */
    if (!(entry.HighWord.Bytes.Flags1 & 0x10)) return TRUE;  /* system descriptor */
    if ((entry.HighWord.Bytes.Flags1 & 0x0a) == 0x08) return TRUE;  /* non-readable code segment */
    if (size && (OFFSETOF(ptr) + size - 1 > GetSelectorLimit( sel ))) return TRUE;
    return FALSE;
}


/***********************************************************************
 *           IsBadHugeWritePtr   (KERNEL.347)
 *
 * @brief Determine whether a huge pointer can be written for `size` bytes.
 *
 * Checks that the selector describes a writable data segment (ignoring
 * expand-down and accessed bits) and that the range up to `offset + size - 1`
 * is within the segment limit.
 *
 * @param ptr    Huge pointer to check.
 * @param size   Number of bytes that must be writable.
 * @return       TRUE if the pointer is bad, FALSE otherwise.
 */
BOOL WINAPI IsBadHugeWritePtr( void huge * ptr, DWORD size )
{
    WORD sel = SELECTOROF( ptr );
    LDT_ENTRY entry;

	FUNCTIONSTART;

    if (!DPMI_GetDescriptor(sel, &entry)) return FALSE;

    /* check for writable data segment, ignoring expand-down and accessed flags */
    if ((entry.HighWord.Bytes.Flags1 ^ LDT_FLAGS_DATA) & 0x1a) return TRUE;
    if (size && (OFFSETOF(ptr) + size - 1 > GetSelectorLimit( sel ))) return TRUE;
    return FALSE;
}

/***********************************************************************
 *           IsBadReadPtr   (KERNEL.334)
 *
 * @brief Wrapper around IsBadHugeReadPtr for far pointers.
 *
 * @param ptr    Far pointer to check.
 * @param size   Number of bytes that must be readable.
 * @return       TRUE if the pointer is bad, FALSE otherwise.
 */
BOOL WINAPI IsBadReadPtr(const void far * ptr, UINT size)
{
	BOOL res;

	FUNCTIONSTART;

	res=IsBadHugeReadPtr(ptr, size);

	FUNCTIONEND;

    return res;
}


/***********************************************************************
 *           IsBadWritePtr   (KERNEL.335)
 *
 * @brief Wrapper around IsBadHugeWritePtr for far pointers.
 *
 * @param ptr    Far pointer to check.
 * @param size   Number of bytes that must be writable.
 * @return       TRUE if the pointer is bad, FALSE otherwise.
 */
BOOL WINAPI IsBadWritePtr(void far * ptr, UINT size)
{
	BOOL res;

	FUNCTIONSTART;

	res=IsBadHugeWritePtr(ptr, size);

	FUNCTIONEND;
	return res;
}


/***********************************************************************
 *           IsBadFlatReadWritePtr   (KERNEL.627)
 *
 * @brief Check read or write access for a flat pointer.
 *
 * Depending on `bWrite`, calls either IsBadHugeWritePtr or IsBadHugeReadPtr.
 *
 * @param ptr     Flat pointer to check.
 * @param size    Number of bytes to validate.
 * @param bWrite  If TRUE, check for write access; otherwise read access.
 * @return        TRUE if the pointer is bad, FALSE otherwise.
 */
// @todo Not exported in this version
BOOL WINAPI IsBadFlatReadWritePtr( void far * ptr, DWORD size, BOOL bWrite )
{
	BOOL res;

	FUNCTIONSTART;

	res=bWrite? IsBadHugeWritePtr(ptr, size) : IsBadHugeReadPtr(ptr, size);

	FUNCTIONEND;

	return res;
}

/***********************************************************************
 *           LongPtrAdd   (KERNEL.180)
 *
 * @brief Add an offset to the base address of a selector in a long pointer.
 *
 * Extracts the selector from the low word of `dwLongPtr`, retrieves its
 * current base, adds `dwAdd`, and writes the new base back to the selector.
 *
 * @param dwLongPtr   Long pointer (selector:offset). The offset part is ignored.
 * @param dwAdd       Value to add to the selector’s base address.
 */
void WINAPI LongPtrAdd(DWORD dwLongPtr, DWORD dwAdd)
{
	WORD wSel = SELECTOROF(dwLongPtr);

	FUNCTIONSTART;

	SetSelectorBase(wSel, GetSelectorBase(wSel)+dwAdd);

	FUNCTIONEND;
}

/**
 * @brief Initialise selector-related global variables and the free selector pool.
 *
 * Reads the DPMI selector increment and computes the shift value (`__AHSHIFT`).
 * Creates descriptors for the standard BIOS and video segments (0x0000,
 * 0x0040, 0xA000, 0xB000, 0xB800, 0xC000, 0xD000, 0xE000, 0xF000) and stores
 * them in the corresponding global variables (`__0000H`, etc.). Sets the limit
 * of the `__0040H` selector to 0x2FF (BIOS data area). Finally, allocates a
 * block of 256 selectors from DPMI and adds them to the internal free list.
 */
void InitSelectors()
{
	WORD ahincr;

	FUNCTIONSTART;

	//@ todo InitKernel_ in kernel16.asm already do similar initialixation

	__AHINCR=DPMI_GetIncrement();;
        ahincr=__AHINCR;
	__AHSHIFT=0;
        while ((ahincr >>= 1) != 0) __AHSHIFT++;
        __0000H=DPMI_SegmentToDescriptor(0x0000);
	__0040H=DPMI_SegmentToDescriptor(0x0040);
	__A000H=DPMI_SegmentToDescriptor(0xA000);
	__B000H=DPMI_SegmentToDescriptor(0xB000);
	__B800H=DPMI_SegmentToDescriptor(0xB800);
	__C000H=DPMI_SegmentToDescriptor(0xC000);
	__D000H=DPMI_SegmentToDescriptor(0xD000);
	__E000H=DPMI_SegmentToDescriptor(0xE000);
	__F000H=DPMI_SegmentToDescriptor(0xF000);
	__ROMBIOS=DPMI_SegmentToDescriptor(0xF000);
	SetSelectorLimit(__0040H, 0x2FF);

	/* Инициализация free-list */
	{
		WORD block = DPMI_AllocDesc(256);
		WORD i;
		LDT_ENTRY entry;

		if (block)
		{
			for (i = 0; i < 256; i++)
			{
				WORD sel = block + i * __AHSHIFT;
				if (DPMI_GetDescriptor(sel, &entry) == 0)
				{
					entry.LimitLow = FirstFreeSel;
					DPMI_SetDescriptor(sel, &entry);
				}
				FirstFreeSel = sel;
				CountFreeSel++;
			}
		}
	}

	FUNCTIONEND;
}


/***********************************************************************
 *           InitBurgerMaster
 *
 * @brief Allocate and initialise the Burgermaster (global heap) segment.
 *
 * Creates a selector (`TH_PGLOBALHEAP`), determines the size of the selector
 * table based on the number of free pages (16 KB for <256 pages, 32 KB
 * otherwise), and allocates memory for the heap and selector table via DPMI.
 * The heap selector’s base and limit are set to cover the allocated memory,
 * and the selector table area is zeroed.
 *
 * @return   TRUE on success, FALSE if allocation fails.
 */
BOOL InitBurgerMaster(void)
{
    DWORD linear;
    DWORD size;
    WORD  totalPages;

    TH_PGLOBALHEAP = DPMI_AllocDesc(1);
    if (!TH_PGLOBALHEAP) return FALSE;

    totalPages = DPMI_GetFreePages();
    SelTableLen = (totalPages < 0x100) ? 16384 : 32768;

    SelTableStart = 0x80 + 0x8000;
    size = SelTableStart + SelTableLen;

    if (!DPMI_AllocMem(size, &hBMDPMI, &linear))
    {
        DPMI_FreeDesc(TH_PGLOBALHEAP);
        TH_PGLOBALHEAP = 0;
        TH_HGLOBALHEAP = TH_PGLOBALHEAP;
        return FALSE;
    }

    SetSelectorBase(TH_PGLOBALHEAP, linear);
    SetSelectorLimit(TH_PGLOBALHEAP, size - 1);

    /* В оригинале hGlobalHeap хранит селектор Burgermaster (PGlobalHeap) */
    TH_HGLOBALHEAP = TH_PGLOBALHEAP;

    memset(MAKELP(TH_PGLOBALHEAP, SelTableStart), 0, SelTableLen);

    return TRUE;
}

/* ============================================================
 * ТЕСТОВАЯ ЧАСТЬ (вставить в конец selector.c)
 * ============================================================ */

static int tests_passed = 0;
static int tests_failed = 0;
static BYTE testByte = 0;  /* объявлена раньше всех использующих функций */

/* Функция вывода символа через DOS INT 21h */
static void dos_putchar(char c);
#pragma aux dos_putchar = \
    "mov ah, 02h" \
    "int 21h" \
    parm [dl] \
    modify [ax];

/* Вывод строки */
void put_str(const char *s)
{
    while (*s) dos_putchar(*s++);
}

/* Вывод 16-битного числа в десятичном виде */
static void put_dec16(WORD val)
{
    char buf[6];
    int pos = 0;
    int i;
    if (val == 0) {
        dos_putchar('0');
        return;
    }
    while (val > 0) {
        buf[pos++] = '0' + (val % 10);
        val /= 10;
    }
    for (i = pos - 1; i >= 0; i--) dos_putchar(buf[i]);
}

/* Вывод 32-битного числа в шестнадцатеричном виде (без ведущих нулей) */
static void put_hex32(DWORD val)
{
    int i;
    int started = 0;
    for (i = 28; i >= 0; i -= 4) {
        int digit = (val >> i) & 0xF;
        if (digit || started || i == 0) {
            dos_putchar(digit < 10 ? '0' + digit : 'A' + digit - 10);
            started = 1;
        }
    }
}

/* Упрощённый printf: поддерживает %s, %d, %u, %x, %lX */
static void debug_printf(const char *fmt, ...)
{
    va_list args;
    const char *p;
    va_start(args, fmt);

    for (p = fmt; *p; p++) {
        if (*p != '%') {
            dos_putchar(*p);
            continue;
        }
        p++;
        switch (*p) {
            case 's': {
                char *s = va_arg(args, char *);
                put_str(s);
                break;
            }
            case 'd': {
                int v = va_arg(args, int);
                if (v < 0) {
                    dos_putchar('-');
                    v = -v;
                }
                put_dec16((WORD)v);
                break;
            }
            case 'u': {
                unsigned int v = va_arg(args, unsigned int);
                put_dec16((WORD)v);
                break;
            }
            case 'x': {
                unsigned int v = va_arg(args, unsigned int);
                put_hex32((DWORD)v);
                break;
            }
            case 'X': {
                unsigned int v = va_arg(args, unsigned int);
                put_hex32((DWORD)v);
                break;
            }
            case 'l': {
                if (p[1] == 'X') {
                    DWORD v = va_arg(args, DWORD);
                    put_hex32(v);
                    p++;
                }
                break;
            }
            default:
                dos_putchar('%');
                dos_putchar(*p);
                break;
        }
    }
    va_end(args);
}

#define TEST(condition, msg, ...) \
    do { \
        if (condition) { \
            tests_passed++; \
        } else { \
            tests_failed++; \
            put_str("FAILED: "); \
            debug_printf(msg, ##__VA_ARGS__); \
            put_str("\r\n"); \
        } \
    } while(0)

/* ---------- DPMI helpers ---------- */
static WORD g_selectorIncrement = 0;

DWORD DPMI_GetLimit(WORD sel)
{
    LDT_ENTRY entry;
    if (DPMI_GetDescriptor(sel, &entry) != 0) return 0xFFFFFFFFUL;
    return ((DWORD)entry.HighWord.Bits.LimitHi << 16) | entry.LimitLow;
}

BOOL DPMI_IsSelectorValid(WORD sel)
{
    LDT_ENTRY entry;
    return (DPMI_GetDescriptor(sel, &entry) == 0);
}

WORD DPMI_GetSelectorFullType(WORD sel)
{
    LDT_ENTRY entry;
    if (DPMI_GetDescriptor(sel, &entry) != 0) return 0xFFFF;
    return entry.HighWord.Bytes.Flags1;
}

WORD DPMI_GetSelectorType(WORD sel)
{
    return DPMI_GetSelectorFullType(sel) & 0x1F;
}

/* ============================================================
 * ТЕСТОВЫЕ ФУНКЦИИ
 * ============================================================ */

void test_SelectorIncrement(void)
{
    WORD inc = g_selectorIncrement;
    TEST(inc > 0, "DPMI selector increment должен быть > 0");
    TEST((inc & (inc - 1)) == 0, "DPMI selector increment не является степенью двойки");
    TEST(__AHINCR == inc, "__AHINCR (%d) не совпадает с DPMI increment (%d)", __AHINCR, inc);
    TEST((1 << __AHSHIFT) == inc, "(1 << __AHSHIFT) != DPMI increment");
}

void test_PredefinedSelectors(void)
{
    struct {
        WORD sel;
        DWORD expectedBase;
        DWORD expectedLimit;
        char *name;
    } predefined[10];
    int i;

    predefined[0].sel = __0000H;   predefined[0].expectedBase = 0x00000; predefined[0].expectedLimit = 0xFFFF; predefined[0].name = "__0000H";
    predefined[1].sel = __0040H;   predefined[1].expectedBase = 0x00400; predefined[1].expectedLimit = 0xFFFF; predefined[1].name = "__0040H";
    predefined[2].sel = __A000H;   predefined[2].expectedBase = 0xA0000; predefined[2].expectedLimit = 0xFFFF; predefined[2].name = "__A000H";
    predefined[3].sel = __B000H;   predefined[3].expectedBase = 0xB0000; predefined[3].expectedLimit = 0xFFFF; predefined[3].name = "__B000H";
    predefined[4].sel = __B800H;   predefined[4].expectedBase = 0xB8000; predefined[4].expectedLimit = 0xFFFF; predefined[4].name = "__B800H";
    predefined[5].sel = __C000H;   predefined[5].expectedBase = 0xC0000; predefined[5].expectedLimit = 0xFFFF; predefined[5].name = "__C000H";
    predefined[6].sel = __D000H;   predefined[6].expectedBase = 0xD0000; predefined[6].expectedLimit = 0xFFFF; predefined[6].name = "__D000H";
    predefined[7].sel = __E000H;   predefined[7].expectedBase = 0xE0000; predefined[7].expectedLimit = 0xFFFF; predefined[7].name = "__E000H";
    predefined[8].sel = __F000H;   predefined[8].expectedBase = 0xF0000; predefined[8].expectedLimit = 0xFFFF; predefined[8].name = "__F000H";
    predefined[9].sel = __ROMBIOS; predefined[9].expectedBase = 0xF0000; predefined[9].expectedLimit = 0x2FF;  predefined[9].name = "__ROMBIOS";

    for (i = 0; i < 10; i++) {
        WORD sel = predefined[i].sel;
        TEST(DPMI_IsSelectorValid(sel), "%s невалиден", predefined[i].name);
        TEST(DPMI_GetBase(sel) == predefined[i].expectedBase,
             "База %s неверна: %08lX", predefined[i].name, DPMI_GetBase(sel));
        TEST(DPMI_GetLimit(sel) == predefined[i].expectedLimit,
             "Лимит %s неверен: %08lX", predefined[i].name, DPMI_GetLimit(sel));
    }
}

void test_AllocSelector_CopyDS(void)
{
    WORD sample = GetDS();  /* используем сегмент данных */
    DWORD base = DPMI_GetBase(sample);
    DWORD limit = DPMI_GetLimit(sample);
    WORD fullType = DPMI_GetSelectorFullType(sample);

    WORD sel = AllocSelector(sample);
    TEST(sel != 0, "AllocSelector(DS) вернул 0");
    if (sel) {
        TEST(DPMI_IsSelectorValid(sel), "Выделенный селектор невалиден: %04X", sel);
        TEST(DPMI_GetBase(sel) == base, "База не скопирована");
        TEST(DPMI_GetLimit(sel) == limit, "Лимит не скопирован");
        TEST(DPMI_GetSelectorFullType(sel) == fullType, "Полный тип не скопирован");
        FreeSelector(sel);
    }
}

void test_AllocSelector_Null(void)
{
    WORD sel = AllocSelector(0);
    TEST(1, "AllocSelector(0) не вызвал краха");
    if (sel) {
        TEST(DPMI_IsSelectorValid(sel), "AllocSelector(0) вернул невалидный селектор");
        TEST(DPMI_GetBase(sel) == 0, "База нового селектора не 0");
        TEST(DPMI_GetLimit(sel) == 0, "Лимит нового селектора не 0");
        FreeSelector(sel);
    }
}

void test_AllocSelector_Invalid(void)
{
    WORD sel = AllocSelector(0xFFFF);
    TEST(1, "AllocSelector(0xFFFF) не вызвал краха");
    if (sel) FreeSelector(sel);
}

void test_Tiled_Limits(void)
{
    WORD sample = AllocSelector(GetDS());
    WORD s;
    DWORD limit;
    int i;

    if (!sample) return;

    /* Лимит 0 */
    DPMI_SetLimit(sample, 0);
    s = AllocSelector(sample);
    TEST(s != 0, "AllocSelector с лимитом 0 вернул 0");
    if (s) {
        TEST(DPMI_GetLimit(s) == 0, "Лимит не 0");
        FreeSelector(s);
    }

    /* Лимит 0x100 */
    DPMI_SetLimit(sample, 0x100);
    s = AllocSelector(sample);
    TEST(s != 0, "AllocSelector с лимитом 0x100 вернул 0");
    if (s) {
        TEST(DPMI_GetLimit(s) == 0x100, "Лимит не 0x100");
        FreeSelector(s);
    }

    /* Лимит 0xFFFF */
    DPMI_SetLimit(sample, 0xFFFF);
    s = AllocSelector(sample);
    TEST(s != 0, "AllocSelector с лимитом 0xFFFF вернул 0");
    if (s) {
        TEST(DPMI_GetLimit(s) == 0xFFFF, "Лимит не 0xFFFF");
        FreeSelector(s);
    }

    /* Лимит 0x10000 */
    DPMI_SetLimit(sample, 0x10000);
    s = AllocSelector(sample);
    TEST(s != 0, "AllocSelector с лимитом 0x10000 вернул 0");
    if (s) {
        limit = DPMI_GetLimit(s);
        TEST(limit == 0x10000 || limit == 0xFFFF, "Лимит не ожидаемый: %08lX", limit);
        TEST(!DPMI_IsSelectorValid(s + g_selectorIncrement), "Следующий селектор должен быть невалидным");
        FreeSelector(s);
    }

    /* Лимит 0x1FFFF */
    DPMI_SetLimit(sample, 0x1FFFF);
    s = AllocSelector(sample);
    TEST(s != 0, "AllocSelector с лимитом 0x1FFFF вернул 0");
    if (s) {
        WORD sel1 = s;
        WORD sel2 = s + g_selectorIncrement;
        TEST(DPMI_IsSelectorValid(sel1) && DPMI_IsSelectorValid(sel2), "Селекторы плитки невалидны");
        TEST(DPMI_GetBase(sel2) == DPMI_GetBase(sel1) + 0x10000, "Базы плиток неверны");
        TEST(DPMI_GetLimit(sel1) == 0xFFFF && DPMI_GetLimit(sel2) == 0xFFFF,
             "Лимиты плиток не 64К-1");
        FreeSelector(s);
    }

    /* Лимит 0x20000 */
    DPMI_SetLimit(sample, 0x20000);
    s = AllocSelector(sample);
    TEST(s != 0, "AllocSelector с лимитом 0x20000 вернул 0");
    if (s) {
        WORD sel1 = s;
        WORD sel2 = s + g_selectorIncrement;
        TEST(DPMI_IsSelectorValid(sel1), "Первый селектор невалиден");
        TEST(DPMI_IsSelectorValid(sel2), "Второй селектор невалиден (ожидалась плитка)");
        if (DPMI_IsSelectorValid(sel1) && DPMI_IsSelectorValid(sel2)) {
            TEST(DPMI_GetBase(sel2) == DPMI_GetBase(sel1) + 0x10000, "Базы неверны");
            limit = DPMI_GetLimit(sel1);
            TEST(limit == 0xFFFF || limit == 0x10000, "Лимит первой плитки неверен");
            limit = DPMI_GetLimit(sel2);
            TEST(limit == 0 || limit == 0xFFFF || limit == 0x10000, "Лимит второй плитки неверен");
        }
        FreeSelector(s);
    }

    /* Лимит 0xFFFFF => 16 плиток */
    DPMI_SetLimit(sample, 0xFFFFF);
    s = AllocSelector(sample);
    TEST(s != 0, "AllocSelector с лимитом 1МБ-1 вернул 0");
    if (s) {
        WORD first = s;
        DWORD base0 = DPMI_GetBase(first);
        for (i = 0; i < 16; i++) {
            WORD cur = first + i * g_selectorIncrement;
            TEST(DPMI_IsSelectorValid(cur), "Селектор %d невалиден", i);
            TEST(DPMI_GetBase(cur) == base0 + i * 0x10000, "База %d неверна", i);
            TEST(DPMI_GetLimit(cur) == 0xFFFF, "Лимит %d не 64К-1", i);
        }
        FreeSelector(s);
        for (i = 0; i < 16; i++) {
            WORD cur = first + i * g_selectorIncrement;
            TEST(!DPMI_IsSelectorValid(cur), "Селектор %d не освобождён", i);
        }
    }

    /* Лимит 0x1FFFFF => 32 плитки */
    DPMI_SetLimit(sample, 0x1FFFFF);
    s = AllocSelector(sample);
    TEST(s != 0, "AllocSelector с лимитом 2МБ-1 вернул 0");
    if (s) {
        WORD first = s;
        DWORD base0 = DPMI_GetBase(first);
        for (i = 0; i < 32; i++) {
            WORD cur = first + i * g_selectorIncrement;
            TEST(DPMI_IsSelectorValid(cur), "Селектор %d невалиден", i);
            TEST(DPMI_GetBase(cur) == base0 + i * 0x10000, "База %d неверна", i);
            TEST(DPMI_GetLimit(cur) == 0xFFFF, "Лимит %d не 64К-1", i);
        }
        FreeSelector(s);
    }

    FreeSelector(sample);
}

void test_AllocSelectorArray_Basic(void)
{
    WORD sel = AllocSelectorArray(1);
    TEST(sel != 0, "AllocSelectorArray(1) вернул 0");
    if (sel) { FreeSelector(sel); }

    sel = AllocSelectorArray(2);
    TEST(sel != 0, "AllocSelectorArray(2) вернул 0");
    if (sel) {
        TEST(DPMI_IsSelectorValid(sel) && DPMI_IsSelectorValid(sel + g_selectorIncrement),
             "Селекторы невалидны");
        FreeSelector(sel);
    }

    sel = AllocSelectorArray(0);
    TEST(sel == 0, "AllocSelectorArray(0) должен вернуть 0");

    sel = AllocSelectorArray(255);
    TEST(sel != 0, "AllocSelectorArray(255) вернул 0");
    if (sel) {
        int ok = 1;
        int i;
        for (i = 0; i < 255; i++) {
            if (!DPMI_IsSelectorValid(sel + i * g_selectorIncrement)) { ok = 0; break; }
        }
        TEST(ok, "Не все селекторы из 255 валидны");
        FreeSelector(sel);
    }
}

void test_AllocSelectorArray_Independence(void)
{
    WORD first = AllocSelectorArray(3);
    if (!first) return;
    {
        DWORD bases[3] = {0x100000, 0x200000, 0x300000};
        DWORD limits[3] = {0x1000, 0x2000, 0x3000};
        int i;
        for (i = 0; i < 3; i++) {
            DPMI_SetBase(first + i * g_selectorIncrement, bases[i]);
            DPMI_SetLimit(first + i * g_selectorIncrement, limits[i]);
        }
        for (i = 0; i < 3; i++) {
            WORD s = first + i * g_selectorIncrement;
            TEST(DPMI_GetBase(s) == bases[i], "База %d изменена", i);
            TEST(DPMI_GetLimit(s) == limits[i], "Лимит %d изменён", i);
        }
        {
            WORD s1 = first;
            WORD s2 = first + g_selectorIncrement;
            DWORD base2_before = DPMI_GetBase(s2);
            DPMI_SetBase(s1, 0x444444);
            TEST(DPMI_GetBase(s2) == base2_before, "Изменение s1 повлияло на s2");
        }
    }
    FreeSelector(first);
}

void test_FreeSelector_Cases(void)
{
    WORD sel = AllocSelector(GetDS());
    if (sel) {
        TEST(FreeSelector(sel) != 0, "FreeSelector не освободил");
        TEST(!DPMI_IsSelectorValid(sel), "Селектор остался валидным");
        TEST(FreeSelector(sel) == 0, "Повторное освобождение должно вернуть 0");
    }
    TEST(FreeSelector(0) == 0, "FreeSelector(0) должен вернуть 0");
    TEST(FreeSelector(GetCS()) == 0, "FreeSelector(CS) должен вернуть 0");

    TEST(FreeSelector(__B800H) == 0, "FreeSelector(__B800H) должен вернуть 0");

    {
        WORD arr = AllocSelectorArray(2);
        if (arr) {
            WORD second = arr + g_selectorIncrement;
            BOOL res = FreeSelector(second);
            TEST(res == 0 || DPMI_IsSelectorValid(second) == TRUE,
                 "FreeSelector(second) ведёт себя неожиданно");
            FreeSelector(arr);
        }
    }
}

void test_BaseLimit(void)
{
    WORD sel = AllocSelector(GetDS());
    if (!sel) return;

    DPMI_SetBase(sel, 0x12340000);
    DPMI_SetLimit(sel, 0x0000FFFF);
    TEST(GetSelectorBase(sel) == 0x12340000, "GetSelectorBase неверен");
    TEST(GetSelectorLimit(sel) == 0x0000FFFF, "GetSelectorLimit неверен");

    DPMI_SetLimit(sel, 0xFFFFF);
    TEST(GetSelectorLimit(sel) == 0xFFFFF, "Лимит не сохранился");

    DPMI_SetLimit(sel, 0x100000);
    TEST(GetSelectorLimit(sel) <= 0xFFFFF, "Лимит превысил 20 бит");

    DPMI_SetBase(sel, 0xFFFFFFFF);
    TEST(GetSelectorBase(sel) == 0xFFFFFFFF, "База не сохранилась");

    DPMI_SetBase(sel, 0);
    DPMI_SetLimit(sel, 0);
    TEST(GetSelectorBase(sel) == 0, "База не 0");
    TEST(GetSelectorLimit(sel) == 0, "Лимит не 0");

    FreeSelector(sel);

    TEST(SetSelectorBase(0xFFFF, 0x100000) == 0, "SetSelectorBase с невалидным селектором должен вернуть 0");
    TEST(SetSelectorLimit(0xFFFF, 0x1000) == 0, "SetSelectorLimit с невалидным селектором должен вернуть 0");
    TEST(GetSelectorBase(0xFFFF) == 0, "GetSelectorBase с невалидным селектором должен вернуть 0");
    TEST(GetSelectorLimit(0xFFFF) == 0, "GetSelectorLimit с невалидным селектором должен вернуть 0");
    TEST(SetSelectorBase(0, 0x100000) == 0, "SetSelectorBase(0) должен вернуть 0");
    TEST(SetSelectorLimit(0, 0x1000) == 0, "SetSelectorLimit(0) должен вернуть 0");
}

void test_SetNoNeighborEffect(void)
{
    WORD arr = AllocSelectorArray(2);
    if (!arr) return;
    {
        WORD s1 = arr;
        WORD s2 = arr + g_selectorIncrement;
        DWORD base1 = 0x100000, base2 = 0x200000;
        DPMI_SetBase(s1, base1);
        DPMI_SetBase(s2, base2);
        TEST(DPMI_GetBase(s1) == base1, "База s1 неверна");
        TEST(DPMI_GetBase(s2) == base2, "База s2 неверна");
        DPMI_SetBase(s1, 0x333333);
        TEST(DPMI_GetBase(s2) == base2, "Изменение s1 повлияло на s2");
    }
    FreeSelector(arr);
}

void test_Aliases(void)
{
    WORD dataSel = GetDS();
    WORD codeAlias = AllocDStoCSAlias(dataSel);
    TEST(codeAlias != 0, "AllocDStoCSAlias вернул 0");
    if (codeAlias) {
        TEST((DPMI_GetSelectorType(codeAlias) & 0x08) != 0, "Не кодовый");
        TEST(DPMI_GetBase(codeAlias) == DPMI_GetBase(dataSel), "База алиаса не совпадает");
        TEST(DPMI_GetLimit(codeAlias) == DPMI_GetLimit(dataSel), "Лимит алиаса не совпадает");
        FreeSelector(codeAlias);
        TEST(!DPMI_IsSelectorValid(codeAlias), "Алиас не освобождён");
    }

    {
        WORD codeSel = GetCS();
        WORD dataAlias = AllocCStoDSAlias(codeSel);
        TEST(dataAlias != 0, "AllocCStoDSAlias вернул 0");
        if (dataAlias) {
            TEST((DPMI_GetSelectorType(dataAlias) & 0x08) == 0, "Не данные");
            TEST(DPMI_GetBase(dataAlias) == DPMI_GetBase(codeSel), "База не совпадает");
            TEST(DPMI_GetLimit(dataAlias) == DPMI_GetLimit(codeSel), "Лимит не совпадает");
            FreeSelector(dataAlias);
            TEST(!DPMI_IsSelectorValid(dataAlias), "Алиас не освобождён");
        }
    }

    TEST(AllocDStoCSAlias(0xFFFF) == 0, "AllocDStoCSAlias(0xFFFF) должен вернуть 0");
    TEST(AllocCStoDSAlias(0xFFFF) == 0, "AllocCStoDSAlias(0xFFFF) должен вернуть 0");
}

void test_PrestoChangoSelector(void)
{
    WORD src = AllocSelector(GetDS());
    if (!src) return;
    {
        WORD dest = AllocSelector(GetDS());
        if (!dest) { FreeSelector(src); return; }
        {
            DWORD base = DPMI_GetBase(src);
            DWORD limit = DPMI_GetLimit(src);
            WORD fullTypeSrc = DPMI_GetSelectorFullType(src);

            WORD res = PrestoChangoSelector(src, dest);
            TEST(res == dest, "PrestoChangoSelector вернул не dest");
            if (res) {
                WORD fullTypeDest = DPMI_GetSelectorFullType(dest);
                TEST((fullTypeDest & 0x08) != (fullTypeSrc & 0x08), "Бит кода не изменился");
                TEST(DPMI_GetBase(dest) == base, "База не сохранилась");
                TEST(DPMI_GetLimit(dest) == limit, "Лимит не сохранился");
                TEST((fullTypeDest & 0xF0) == (fullTypeSrc & 0xF0), "Старшие биты типа изменились");
            }
        }

        {
            WORD dataDest = AllocSelector(GetDS());
            if (dataDest) {
                WORD res2 = PrestoChangoSelector(GetCS(), dataDest);
                TEST(res2 == dataDest, "PrestoChangoSelector(CS, dest) вернул не dest");
                if (res2) {
                    TEST((DPMI_GetSelectorType(dataDest) & 0x08) == 0, "Не стал данным");
                }
                FreeSelector(dataDest);
            }
        }

        {
            WORD same = AllocSelector(GetDS());
            if (same) {
                WORD origType = DPMI_GetSelectorType(same);
                WORD res3 = PrestoChangoSelector(same, same);
                TEST(res3 == same, "PrestoChangoSelector(same,same) вернул не same");
                if (res3) {
                    WORD newType = DPMI_GetSelectorType(same);
                    TEST((origType & 0x08) != (newType & 0x08), "Тип не изменился");
                }
                FreeSelector(same);
            }
        }

        FreeSelector(src);
        FreeSelector(dest);
    }
}

void test_Stress(void)
{
    #define N 100
    WORD sels[N];
    int ok = 1;
    int i;
    for (i = 0; i < N; i++) {
        sels[i] = AllocSelector(GetDS());
        if (!sels[i]) { ok = 0; break; }
    }
    TEST(ok, "Не удалось выделить 100 селекторов подряд");
    for (i = 0; i < N; i++) {
        if (sels[i]) FreeSelector(sels[i]);
    }
    {
        WORD again = AllocSelector(GetDS());
        TEST(again != 0, "Не удалось выделить после освобождения");
        if (again) FreeSelector(again);
    }
    #undef N
}

void test_StressMany(void)
{
    const int iterations = 1000;
    int success = 1;
    int i;
    for (i = 0; i < iterations; i++) {
        WORD sel = AllocSelector(GetDS());
        if (!sel) {
            success = 0;
            break;
        }
        FreeSelector(sel);
    }
    TEST(success, "Не удалось выполнить %d циклов выделения/освобождения", iterations);
}

void test_ExhaustSelectors(void)
{
    #define MAX_SELECTORS 1024
    WORD sels[MAX_SELECTORS];
    int count = 0;
    int i;

    while (count < MAX_SELECTORS) {
        WORD sel = AllocSelector(GetDS());
        if (!sel) break;
        sels[count++] = sel;
    }
    TEST(count > 0, "Не удалось выделить ни одного селектора");
    for (i = 0; i < count; i++) {
        FreeSelector(sels[i]);
    }
    #undef MAX_SELECTORS
}

void test_IncrementConsistency(void)
{
    WORD inc1 = DPMI_GetIncrement();
    WORD inc2 = DPMI_GetIncrement();
    TEST(inc1 == inc2, "DPMI increment меняется между вызовами: %d != %d", inc1, inc2);
    TEST(__AHINCR == inc1, "__AHINCR не совпадает с DPMI");
    TEST((1 << __AHSHIFT) == inc1, "(1 << __AHSHIFT) != DPMI increment");
}

void test_ModifyTileLimit(void)
{
    WORD sample = AllocSelector(GetDS());
    if (!sample) return;
    DPMI_SetLimit(sample, 0x1FFFF);
    {
        WORD tiled = AllocSelector(sample);
        if (!tiled) { FreeSelector(sample); return; }
        {
            WORD sel1 = tiled;
            WORD sel2 = tiled + g_selectorIncrement;
            DWORD limit2_before = DPMI_GetLimit(sel2);
            DPMI_SetLimit(sel1, 0x1234);
            TEST(DPMI_GetLimit(sel1) == 0x1234, "Лимит sel1 не изменился");
            TEST(DPMI_GetLimit(sel2) == limit2_before, "Изменение лимита sel1 повлияло на sel2");
            FreeSelector(tiled);
        }
    }
    FreeSelector(sample);
}

void test_RealAccess(void)
{
    WORD offset = OFFSETOF(&testByte);
    WORD newSel = AllocSelector(GetDS());
    if (!newSel) {
        TEST(0, "AllocSelector не удался");
        return;
    }

    DPMI_SetBase(newSel, DPMI_GetBase(GetDS()));
    DPMI_SetLimit(newSel, 0xFFFF);

    {
        BYTE far *fp = (BYTE far *)MAKELP(newSel, offset);
        *fp = 0xAB;
    }

    TEST(testByte == 0xAB, "Запись через новый селектор не отразилась в исходной переменной");

    FreeSelector(newSel);
}

/* Точка входа для тестов (вызывается из ядра) */
int test_Selectors(void)
{
    g_selectorIncrement = DPMI_GetIncrement();

    put_str("Запуск тестов Selector API (KERNEL mode)\r\n");

    test_SelectorIncrement();
    test_PredefinedSelectors();
    test_AllocSelector_CopyDS();
    test_AllocSelector_Null();
    test_AllocSelector_Invalid();
    test_Tiled_Limits();
    test_AllocSelectorArray_Basic();
    test_AllocSelectorArray_Independence();
    test_FreeSelector_Cases();
    test_BaseLimit();
    test_SetNoNeighborEffect();
    test_Aliases();
    test_PrestoChangoSelector();
    test_Stress();
    test_StressMany();
    test_ExhaustSelectors();
    test_IncrementConsistency();
    test_ModifyTileLimit();
    test_RealAccess();

{
    put_str("Тестов пройдено: ");
    put_dec16(tests_passed);
    put_str(", провалено: ");
    put_dec16(tests_failed);
    put_str("\r\n");
}

    return 0;
}
