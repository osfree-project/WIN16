/*
 * Copyright 2026 Yuri Prokushev
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
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Global Atom Table implementation
 *
 * The Global Atom Table reuses the standard Atom Table functions that operate
 * on the local heap (using DS).  We temporarily switch DS to the Global Atom
 * Heap segment, call the original functions and then return DS to its original
 * value.
 *
 * All WINAPI functions preserve DS automatically through their prolog/epilog.
 *
 * General Atom Table Information:
 *  - Atom tables store strings and return 16-bit integer identifiers ("atoms").
 *  - Global atoms are available to all processes and serve as the foundation for
 *    Dynamic Data Exchange (DDE), allowing applications to share item-name and
 *    topic-name strings without passing the actual strings.
 *  - String atoms returned by GlobalAddAtom are in the range 0xC000-0xFFFF,
 *    while integer atoms (created via MAKEINTATOM) are in the range 0x0001-0xBFFF.
 *  - Each string atom has a reference count. The string persists in the global
 *    atom table until its reference count reaches zero.
 *  - The maximum string length for global atoms is 255 characters (not including
 *    the null terminator). Differing only in case is considered identical, and
 *    the case of the first string added to the table is preserved.
 *
 * References:
 *   - Pietrek, M. Windows Internals. Addison-Wesley, 1993, p. 68.
 *   - Schulman, A., Maxey, D., Pietrek, M. Undocumented Windows.
 *     Addison-Wesley, 1992, p. 220.
 */

#include <windows.h>
#include "user.h"


/**
 * @brief Size of the memory segment allocated for the Global Atom Table.
 *
 * @note This is a fixed-size allocation. The global atom table cannot be
 *       dynamically resized once created.
 */
#define GLOBALATOM_SEGMENT_SIZE  0xFA

/**
 * @brief End address of the local heap inside the atom table segment.
 *
 * The first 16 bytes (Instance Data) are reserved for system use.
 */
#define GLOBALATOM_HEAP_END      (GLOBALATOM_SEGMENT_SIZE - 0x10)

/** @brief Trace an atom string: far string or integer atom. */
#define TRACE_ATOM_STRING(s)          \
    {                               \
        if (HIWORD(s))                 \
            TRACE("'%S' ", (s))       \
        else                           \
            TRACE("%04x ", LOWORD(s)) \
    }

#pragma code_seg( "INIT_TEXT" );

/**
 *	InitGlobalAtomTable
 *
 * @brief Initializes the Global Atom Table.
 *
 * Allocates a moveable, zero-initialized, DDE-sharable segment for the
 * Global Atom Table.  On Windows 3.0 the GMEM_NOT_BANKED flag is added
 * automatically; later versions ignore it.
 *
 * After successful allocation the function locks the segment, switches
 * DS to it, initializes a local heap inside it and creates an atom hash
 * table with 37 buckets.  The segment is unlocked afterwards.
 *
 * This function is placed in INIT_TEXT so that its code can be discarded
 * after startup.
 *
 * @note This function must be called once during USER.EXE initialization.
 * @note The global variable ghGlobalAtomTable holds the HGLOBAL handle.
 *
 * @see GlobalAddAtom, GlobalDeleteAtom, GlobalFindAtom, GlobalGetAtomName
 */
VOID WINAPI InitGlobalAtomTable(void)
{
    FUNCTION_START

    ghGlobalAtomTable = GlobalAlloc(
        (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE) |
        (LOWORD(GetVersion()) < 0x030A ? GMEM_NOT_BANKED : 0),
        GLOBALATOM_SEGMENT_SIZE
    );

    if (ghGlobalAtomTable)
    {
        PushDS();

        SetDS(SELECTOROF(GlobalLock(ghGlobalAtomTable)));

        LocalInit(0, 0, GLOBALATOM_HEAP_END);
        InitAtomTable(0x25);

        PopDS();

        GlobalUnlock(ghGlobalAtomTable);
    }

    FUNCTION_END
}

#pragma code_seg( "ATOM_TEXT" );

/**
 *	SetGlobalAtomTableDS
 *
 * @brief Temporarily sets DS to the Global Atom Table segment.
 *
 * If the Global Atom Table has not been allocated yet (ghGlobalAtomTable == 0),
 * the function returns 0 immediately.  Otherwise it locks the table, loads
 * DS with the segment selector, unlocks the table and returns 1 (TRUE).
 *
 * The handle value is captured before switching DS to prevent accessing
 * the global variable through the wrong data segment.
 *
 * This is a near-cdecl function.  It does not save/restore DS, therefore
 * the changed DS remains in effect until the WINAPI caller returns.
 *
 * @return 0 if the table is not allocated, 1 (TRUE) if DS was successfully set.
 */
WORD __near __cdecl SetGlobalAtomTableDS(void)
{
    HGLOBAL hTable = ghGlobalAtomTable;

    if (!hTable)
        return 0;

    SetDS(SELECTOROF(GlobalLock(hTable)));
    GlobalUnlock(hTable);
    return 1;
}

/**
 * @defgroup global_atom_api Global Atom API (USER.268 - USER.271)
 * @brief Functions to manage the system-wide global atom table.
 *
 * These functions allow applications to add, find, delete and retrieve
 * strings from the global atom table, which is shared across all processes.
 *
 * The table is initialized once by InitGlobalAtomTable() during USER startup
 * and must not be called directly by applications.
 *
 * @note String atoms are automatically reference-counted. The string remains
 *       in the table until GlobalDeleteAtom has been called as many times as
 *       GlobalAddAtom.
 * @note Integer atoms (created via MAKEINTATOM) are in the range 0x0001-0xBFFF
 *       and are not stored in the table.
 */

/**
 *	GlobalAddAtom (USER.268)
 *
 * @brief Adds a string to the global atom table.
 *
 * If the string already exists, its reference count is incremented.
 * Otherwise a new atom with reference count 1 is created.
 *
 * @param lpstr Null-terminated string (max 255 chars).
 * @return Atom in range 0xC000-0xFFFF, or 0 on failure.
 *
 * @ingroup global_atom_api
 */
ATOM WINAPI GlobalAddAtom(LPCSTR lpstr)
{
    ATOM res = 0;

    FUNCTION_START

    TRACE_ATOM_STRING(lpstr);

    if (SetGlobalAtomTableDS())
        res = AddAtom(lpstr);

    FUNCTION_END
    return res;
}

/**
 *	GlobalDeleteAtom (USER.269)
 *
 * @brief Decrements the reference count of a global string atom.
 *
 * When the reference count reaches zero, the string is removed from the table.
 * Has no effect on integer atoms.
 *
 * @param atom Atom previously returned by GlobalAddAtom.
 * @return Always 0.
 *
 * @ingroup global_atom_api
 */
ATOM WINAPI GlobalDeleteAtom(ATOM atom)
{
    ATOM res = 0;

    FUNCTION_START

    if (SetGlobalAtomTableDS())
        res = DeleteAtom(atom);

    FUNCTION_END
    return res;
}

/**
 *	GlobalFindAtom (USER.270)
 *
 * @brief Searches the global atom table for a string.
 *
 * The search is case-insensitive.
 *
 * @param lpstr Null-terminated string to find.
 * @return The associated atom, or 0 if not found.
 *
 * @ingroup global_atom_api
 */
ATOM WINAPI GlobalFindAtom(LPCSTR lpstr)
{
    ATOM res = 0;

    FUNCTION_START

    TRACE_ATOM_STRING(lpstr);

    if (SetGlobalAtomTableDS())
        res = FindAtom(lpstr);

    FUNCTION_END
    return res;
}

/**
 *	GlobalGetAtomName (USER.271)
 *
 * @brief Retrieves the string associated with a global atom.
 *
 * @param atom     Global atom.
 * @param lpszbuf  Buffer to receive the null-terminated string.
 * @param len      Size of buffer in characters.
 * @return Number of characters copied, or 0 on failure.
 *
 * @ingroup global_atom_api
 */
UINT WINAPI GlobalGetAtomName(ATOM atom, LPSTR lpszbuf, int len)
{
    UINT res = 0;

    FUNCTION_START

    if (SetGlobalAtomTableDS())
        res = GetAtomName(atom, lpszbuf, len);

    FUNCTION_END
    return res;
}

#pragma code_seg();
