/*
 * Atom table functions - 16 bit variant
 *
 * Copyright 1993, 1994, 1995 Alexandre Julliard
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

/*
 * Warning: The code assumes that LocalAlloc() returns a block aligned
 * on a 4-bytes boundary (because of the shifting done in
 * HANDLETOATOM).  If this is not the case, the allocation code will
 * have to be changed.
 */

#include <windows.h>
#include <win_private.h>

/***********************************************************************
 *           ATOM_GetTable
 *
 * Return a pointer to the atom table of a given segment, creating
 * it if necessary.
 *
 * RETURNS
 *	Pointer to table: Success
 *	NULL: Failure
 */
static LPATOMTABLE ATOM_GetTable( BOOL create  /* [in] Create */ )
{
    LPINSTANCEDATA ptr;

    FUNCTIONSTART;

    ptr=MAKELP(GetDS(), 0);

    if (ptr->atomtable)
    {
        LPATOMTABLE table = (LPATOMTABLE)((LPSTR)ptr + ptr->atomtable);
        if (table->size) return table;
    }
    if (!create) return NULL;
    if (!InitAtomTable( 0 )) return NULL;
//    /* Reload ptr in case it moved in linear memory */
    ptr=MAKELP(GetDS(), 0);

    FUNCTIONEND;

    return (LPATOMTABLE)((LPSTR)ptr + ptr->atomtable);
}


/***********************************************************************
 *           ATOM_Hash
 * RETURNS
 *	The hash value for the input string
 */
static WORD ATOM_Hash(
            WORD entries, /* [in] Total number of entries */
            LPCSTR str,   /* [in] Pointer to string to hash */
            WORD len      /* [in] Length of string */
) {
    WORD i, hash = 0;

    FUNCTIONSTART;
//    TRACE("%x, %s, %x\n", entries, str, len);

    for (i = 0; i < len; i++) hash ^= toupper(str[i]) + i;

    FUNCTIONEND;

    return hash % entries;
}


/***********************************************************************
 *           ATOM_IsIntAtomA
 */
static BOOL ATOM_IsIntAtomA(LPCSTR atomstr,WORD *atomid)
{
    UINT atom = 0;

    FUNCTIONSTART;

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

    FUNCTIONEND;

    return TRUE;
}


/***********************************************************************
 *           ATOM_MakePtr
 *
 * Make an ATOMENTRY pointer from a handle (obtained from GetAtomHandle()).
 */
LPATOMENTRY ATOM_MakePtr( HANDLE handle /* [in] Handle */ )
{

    FUNCTIONSTART;

    FUNCTIONEND;

    return MAKELP(GetDS(), handle);
}


/***********************************************************************
 *           InitAtomTable   (KERNEL.68)
 *
 * BRIEF:
 *  Initialize atom hash table and set its size
 *
 * ENTRY:
 *  entries - number of entries in atom table
 *
 * RETURNS:
 *  TRUE  - success (near address of atom table)
 *  FALSE - failure
 *
 */
BOOL WINAPI InitAtomTable(int nSize)
{
    int i;
    HANDLE handle = ((LPINSTANCEDATA) MAKELP(GetDS(), 0))->atomtable;

    FUNCTIONSTART;

    /* sanity check */
    if (!nSize) nSize = DEFAULT_ATOMTABLE_SIZE;  

    /* Don't init twice */
    if (!handle)
    {
        /* Allocate the table */
        handle = LocalAlloc(LPTR, FIELDOFFSET(ATOMTABLE, entries[nSize]));
        if (!handle)
        {
          ((PATOMTABLE)handle)->size = nSize;

          /* Store a pointer to the table in the instance data */
          ((LPINSTANCEDATA) MAKELP(GetDS(), 0))->atomtable = handle;
        }
    }

    FUNCTIONEND;

    /* return result in AX and CX */
    SetCX(handle);
    return handle;
}

/***********************************************************************
 *           GetAtomHandle   (KERNEL.73)
 *
 * BRIEF:
 *  Return handle of string atom
 *
 */
HLOCAL WINAPI GetAtomHandle(ATOM atom)
{

    FUNCTIONSTART;

    if (atom < MAXINTATOM) return 0;

    FUNCTIONEND;

    return ATOMTOHANDLE(atom);
}


/***********************************************************************
 *           AddAtom   (KERNEL.70)
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
ATOM WINAPI AddAtom(LPCSTR str)
{
    char buffer[MAX_ATOM_LEN+1];
    WORD hash;
    HANDLE entry;
    LPATOMENTRY entryPtr;
    LPATOMTABLE table;
    int len, ae_len;
    WORD iatom;

    FUNCTIONSTART;

    if (ATOM_IsIntAtomA( str, &iatom )) return iatom;

//    TRACE("%s\n",debugstr_a(str));

    if (!(table = ATOM_GetTable( TRUE ))) return 0;

    /* Make a copy of the string to be sure it doesn't move in linear memory. */
    lstrcpyn( buffer, str, sizeof(buffer) );

    len = lstrlen( buffer );
    hash = ATOM_Hash( table->size, buffer, len );
    entry = table->entries[hash];
    while (entry)
    {
        entryPtr = ATOM_MakePtr( entry );
        if ((entryPtr->length == len) &&
            (!strnicmp( entryPtr->str, buffer, len )))
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
    table = ATOM_GetTable( FALSE );
    entryPtr = ATOM_MakePtr( entry );
    entryPtr->next = table->entries[hash];
    entryPtr->refCount = 1;
    entryPtr->length = len;
    memcpy( entryPtr->str, buffer, len);
    /* Some applications _need_ the '\0' padding provided by memset */
    /* Note that 1 byte of the str is accounted for in the ATOMENTRY struct */
    memset( entryPtr->str+len, 0, ae_len - sizeof(ATOMENTRY) - (len - 1));
    table->entries[hash] = entry;
//    TRACE("-- new 0x%x\n", entry);

    FUNCTIONEND;

    return HANDLETOATOM( entry );
}


/***********************************************************************
 *           DeleteAtom   (KERNEL.71)
 */
ATOM WINAPI DeleteAtom( ATOM atom )
{
    LPATOMENTRY entryPtr;
    LPATOMTABLE table;
    HANDLE entry, *prevEntry;
    WORD hash;

    FUNCTIONSTART;

    if (atom < MAXINTATOM) return 0;  /* Integer atom */

//    TRACE("0x%x\n",atom);

    if (!(table = ATOM_GetTable( FALSE ))) return 0;
    entry = ATOMTOHANDLE( atom );
    entryPtr = ATOM_MakePtr( entry );

    /* Find previous atom */
    hash = ATOM_Hash( table->size, entryPtr->str, entryPtr->length );
    prevEntry = &table->entries[hash];
    while (*prevEntry && *prevEntry != entry)
    {
        LPATOMENTRY prevEntryPtr = ATOM_MakePtr( *prevEntry );
        prevEntry = &prevEntryPtr->next;
    }
    if (!*prevEntry) return atom;

    /* Delete atom */
    if (--entryPtr->refCount == 0)
    {
        *prevEntry = entryPtr->next;
        LocalFree( entry );
    }

    FUNCTIONEND;

    return 0;
}


/***********************************************************************
 *           FindAtom   (KERNEL.69)
 */
ATOM WINAPI FindAtom( LPCSTR str )
{
    LPATOMTABLE table;
    WORD hash,iatom;
    HANDLE entry;
    int len;

    FUNCTIONSTART;

//    TRACE("%s\n",debugstr_a(str));

    if (ATOM_IsIntAtomA( str, &iatom )) return iatom;
    if ((len = lstrlen( str )) > 255) len = 255;
    if (!(table = ATOM_GetTable( FALSE ))) return 0;
    hash = ATOM_Hash( table->size, str, len );
    entry = table->entries[hash];
    while (entry)
    {
        LPATOMENTRY entryPtr = ATOM_MakePtr( entry );
        if ((entryPtr->length == len) &&
            (!strnicmp( entryPtr->str, str, len )))
        {
//            TRACE("-- found %x\n", entry);
            return HANDLETOATOM( entry );
        }
        entry = entryPtr->next;
    }

    FUNCTIONEND;

    return 0;
}


/***********************************************************************
 *           GetAtomName   (KERNEL.72)
 */
UINT WINAPI GetAtomName( ATOM atom, LPSTR buffer, int count )
{
    LPATOMENTRY entryPtr;
    HANDLE entry;
    LPSTR strPtr;
    int len;
    char text[8];

    FUNCTIONSTART;
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
        if (!ATOM_GetTable( FALSE )) return 0;
        entry = ATOMTOHANDLE( atom );
        entryPtr = ATOM_MakePtr( entry );
        len = entryPtr->length;
        strPtr = entryPtr->str;
    }
    if (len >= count) len = count-1;
    memcpy( (void FAR *)buffer, strPtr, len );
    buffer[len] = '\0';

    FUNCTIONEND;

    return len;
}
