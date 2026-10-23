/*
 * 				Shell Library Functions
 *
 * Copyright 1993, 1994, 1995 Alexandre Julliard
 * Copyright 1997 Willows Software, Inc. 
 * Copyright 1998 Marcus Meissner
 * Copyright 2000 Juergen Schmied
 * Copyright 2002 Eric Pouech
 * Copyright 2023 Yuri Prokushev
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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

/*! @file atomex.c
    @brief Local atom table implementation (AddAtomEx/FindAtomEx family).
*/

#include "Shell.h"

/***********************************************************************
 *           ATOM_Hash
 * RETURNS
 *	The hash value for the input string
 */

/*! @brief Compute the hash bucket index for a string.

    @param[in] entries Total number of buckets.
    @param[in] str     Pointer to the string to hash.
    @param[in] len     Length of the string in bytes.

    @return The bucket index in [0, entries).
*/
static WORD ATOM_Hash(
            WORD entries, /* [in] Total number of entries */
            LPCSTR str,   /* [in] Pointer to string to hash */
            WORD len      /* [in] Length of string */
) {
    WORD i, hash = 0;

//    TRACE("%x, %s, %x\n", entries, str, len);

    for (i = 0; i < len; i++) hash ^= AnsiUpperChar(str[i]) + i;
    return hash % entries;
}


/***********************************************************************
 *           ATOM_IsIntAtom
 */

/*! @brief Check whether a string denotes an integer atom and decode it.

    @param[in]  atomstr String to inspect (LOWORD is the atom if HIWORD is 0).
    @param[out] atomid  Decoded integer atom value.

    @return TRUE if @p atomstr denotes an integer atom, FALSE otherwise.

    @retval TRUE  The string is an integer atom.
    @retval FALSE The string is not an integer atom.
*/
static BOOL ATOM_IsIntAtom(LPCSTR atomstr,WORD *atomid)
{
    UINT atom = 0;
    if (!HIWORD(atomstr)) atom = LOWORD(atomstr);
    else
    {
        if (*atomstr++ != '#') return FALSE;
        while (*atomstr >= '0' && *atomstr <= '9')
        {
            atom = atom * 10 + *atomstr - '0';
            atomstr++;
        }
        if (*atomstr) return FALSE;
    }
    if (atom >= MAXINTATOM)
    {
//        SetLastError( ERROR_INVALID_PARAMETER );
        atom = 0;
    }
    *atomid = atom;
    return TRUE;
}


/***********************************************************************
 *           ATOM_MakePtr
 *
 * Make an ATOMENTRY pointer from a handle (obtained from GetAtomHandle()).
 */

/*! @brief Convert an atom-table handle into a pointer in our data segment.

    @param[in] handle Handle obtained from GetAtomHandle().

    @return Pointer to the corresponding ATOMENTRY.

    @retval MAKELP Pointer to the ATOMENTRY in our data segment.
*/
LPATOMENTRY ATOM_MakePtr( HANDLE handle /* [in] Handle */ )
{
    return MAKELP(GetDS(), handle);
}


/***********************************************************************
 *           AddAtomEx
 *
 * Windows DWORD aligns the atom entry size.
 * The remaining unused string space created by the alignment
 * gets padded with '\0's in a certain way to ensure
 * that at least one trailing '\0' remains.
 *
 * RETURNS
 *	Atom: Success
 *	0: Failure
 */

/*! @brief Add a string to the atom table, or bump its reference count.

    Windows DWORD-aligns the atom entry size. The remaining unused string
    space created by the alignment gets padded with '\0's to ensure that
    at least one trailing '\0' remains.

    @param[in] atomtable Atom table to modify.
    @param[in] str       String to add.

    @return The atom on success, 0 on failure.

    @retval HANDLETOATOM The atom of the existing or newly added entry.
    @retval 0            Failure.
*/
ATOM WINAPI AddAtomEx(LPATOMTABLE atomtable, LPCSTR str )
{
    char buffer[MAX_ATOM_LEN+1];
    WORD hash;
    HANDLE entry;
    LPATOMENTRY entryPtr;
    LPATOMTABLE table;
    int len, ae_len;
    WORD iatom;

    if (ATOM_IsIntAtom( str, &iatom )) return iatom;

//    TRACE("%s\n",debugstr_a(str));

    if (!(table = atomtable)) return 0;

    /* Make a copy of the string to be sure it doesn't move in linear memory. */
    lstrcpyn( buffer, str, sizeof(buffer) );

    len = lstrlen( buffer );
    hash = ATOM_Hash( table->size, buffer, len );
    entry = table->entries[hash];
    while (entry)
    {
        entryPtr = ATOM_MakePtr( entry );
        if ((entryPtr->length == len) &&
            (!lstrnicmp( entryPtr->str, buffer, len )))
        {
            entryPtr->refCount++;
//            TRACE("-- existing 0x%x\n", entry);
            return HANDLETOATOM( entry );
        }
        entry = entryPtr->next;
    }

    ae_len = (sizeof(ATOMENTRY)+len+3) & ~3;
    entry = LocalAlloc( LMEM_FIXED, ae_len );
    if (!entry) return 0;
    /* Reload the table ptr in case it moved in linear memory */
    table = atomtable;
    entryPtr = ATOM_MakePtr( entry );
    entryPtr->next = table->entries[hash];
    entryPtr->refCount = 1;
    entryPtr->length = len;
    lmemcpy( entryPtr->str, buffer, len);
    /* Some applications _need_ the '\0' padding provided by memset */
    /* Note that 1 byte of the str is accounted for in the ATOMENTRY struct */
    lmemset( entryPtr->str+len, 0, ae_len - sizeof(ATOMENTRY) - (len - 1));
    table->entries[hash] = entry;
//    TRACE("-- new 0x%x\n", entry);
    return HANDLETOATOM( entry );
}


/***********************************************************************
 *           DeleteAtomEx
 */

/*! @brief Remove one reference to an atom and free it when unused.

    @param[in] atomtable Atom table to modify.
    @param[in] atom      Atom to delete.

    @return 0 on success or for an integer atom, the atom if not found.

    @retval 0 Deletion succeeded, or the atom was an integer atom.
*/
ATOM WINAPI DeleteAtomEx(LPATOMTABLE atomtable, ATOM atom )
{
    LPATOMENTRY entryPtr;
    LPATOMTABLE table;
    HANDLE entry, *prevEntry;
    WORD hash;

    if (atom < MAXINTATOM) return 0;  /* Integer atom */

//    TRACE("0x%x\n",atom);

    if (!(table = atomtable)) return 0;
    entry = ATOMTOHANDLE( atom );
    entryPtr = ATOM_MakePtr( entry );

    /* Find previous atom */
    hash = ATOM_Hash( table->size, entryPtr->str, entryPtr->length );
    prevEntry = &table->entries[hash];
    while (*prevEntry && *prevEntry != entry)
    {
        LPATOMENTRY  prevEntryPtr = ATOM_MakePtr( *prevEntry );
        prevEntry = &prevEntryPtr->next;
    }
    if (!*prevEntry) return atom;

    /* Delete atom */
    if (--entryPtr->refCount == 0)
    {
        *prevEntry = entryPtr->next;
        LocalFree( entry );
    }
    return 0;
}


/***********************************************************************
 *           FindAtomEx
 */

/*! @brief Look up a string in the atom table.

    @param[in] atomtable Atom table to search.
    @param[in] str       String to look up.

    @return The atom if found, 0 otherwise.

    @retval HANDLETOATOM The atom of the found entry.
    @retval 0            Atom not found.
*/
ATOM WINAPI FindAtomEx(LPATOMTABLE atomtable, LPCSTR str )
{
    LPATOMTABLE table;
    WORD hash,iatom;
    HANDLE entry;
    int len;

//    TRACE("%s\n",debugstr_a(str));

    if (ATOM_IsIntAtom( str, &iatom )) return iatom;
    if ((len = lstrlen( str )) > 255) len = 255;
    if (!(table = atomtable)) return 0;
    hash = ATOM_Hash( table->size, str, len );
    entry = table->entries[hash];
    while (entry)
    {
        LPATOMENTRY  entryPtr = ATOM_MakePtr( entry );
        if ((entryPtr->length == len) &&
            (!lstrnicmp( entryPtr->str, str, len )))
        {
//            TRACE("-- found %x\n", entry);
            return HANDLETOATOM( entry );
        }
        entry = entryPtr->next;
    }
//    TRACE("-- not found\n");
    return 0;
}


/***********************************************************************
 *           GetAtomNameEx
 */

/*! @brief Copy the string associated with an atom into a caller buffer.

    @param[in]  atomtable Atom table to query.
    @param[in]  atom      Atom to resolve.
    @param[out] buffer    Output buffer.
    @param[in]  count     Size of @p buffer in bytes.

    @return Length of the copied string (without the terminating NUL).

    @retval 0 Failure, or @p count was zero.
*/
UINT WINAPI GetAtomNameEx(LPATOMTABLE atomtable, ATOM atom, LPSTR buffer, int count )
{
    LPATOMENTRY  entryPtr;
    HANDLE entry;
    LPSTR strPtr;
    int len;
    char text[8];

//    TRACE("%x\n",atom);

    if (!count) return 0;
    if (atom < MAXINTATOM)
    {
        lstrcpy(text, "#");
        lstrcat(text, (LPSTR)atom);
        len = lstrlen(text);
        strPtr = text;
    }
    else
    {
        if (!atomtable) return 0;
        entry = ATOMTOHANDLE( atom );
        entryPtr = ATOM_MakePtr( entry );
        len = entryPtr->length;
        strPtr = entryPtr->str;
    }
    if (len >= count) len = count-1;
    lmemcpy( (void FAR *)buffer, strPtr, len );
    buffer[len] = '\0';
    return len;
}
