/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CObjFromHandle.cpp
  Version    : 0.30
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the interface for CObjFromHandle, a class used to
  implement the "FromHandle()" member functions. Note that this
  must be super-fast - it will be called very often. We use a
  hash table to make access fast.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
11-09-03  0.10  wdh  First created.
11-25-03  0.20  wdh  Fixed hash in FindEntry (needed unsigned in recast).
05-30-04  0.30  wdh  Fixed crash: if a stock object (e.g. WHITE_BRUSH) was
    used twice, then we ran into real trouble.

 --[ How to compile ]----------------------------------------------------

  This file was developed under DevC++ 4

 --[ License ] ----------------------------------------------------------

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 ------------------------------------------------------------------------
  Copyright (c) 2000-04 Open Foundation Classes
  Copyright (c) 2003-04 William D. Herndon
 ************************************************************************/
#include <afxwin.h>
#include <stdlib>
#include <cstring>

int g_viTabSizes[] = { 101, 307, 929, 2789, 8369, 25111, 0 };

#define LOCK_CRITICAL_SECTION()   /*!! We need this for multi-threading! */
#define UNLOCK_CRITICAL_SECTION() /*!! We need this for multi-threading! */

CObjFromHandle::CObjFromHandle(CRuntimeClass* pRT)
{
	m_nTabEntries = 0;
	m_nTabSizeIdx = 0;
	int nTabSize = g_viTabSizes[m_nTabSizeIdx];
	m_pTable = (OBJANDHANDLE*)malloc(nTabSize*sizeof(OBJANDHANDLE));
	// @todo memset(m_pTable, 0, nTabSize*sizeof(OBJANDHANDLE));
	m_pRT = pRT;
	m_dwRUCount= 0;
	m_bTemp = FALSE;
}

CObjFromHandle::~CObjFromHandle()
{
	RemoveOldTemps(0);
	free(m_pTable);
}

// Rehash the table.
// This is only called internally when we have the critical section locked.
BOOL CObjFromHandle::RehashTable()
{
	// Get the new table size
	int nNewTabSize = g_viTabSizes[m_nTabSizeIdx+1];

	// If this assert is thrown, we have hit the sentinel
	// and the table is too big: something is seriously
	// wrong if we need a table bigger than our biggest size.
	ASSERT(nNewTabSize != 0);
	if (nNewTabSize == 0)
		return( FALSE );

	// Allocate the new table
	OBJANDHANDLE* pNewTable = (OBJANDHANDLE*)
		malloc(nNewTabSize*sizeof(OBJANDHANDLE));
	if (pNewTable == NULL)
		return( FALSE );
	// If that succeeded, keep the old table in other variables
	// and set the new table as our table.
	OBJANDHANDLE* pOldTable = m_pTable;
	OBJANDHANDLE* pEntry;
	int nOldTabSize = g_viTabSizes[m_nTabSizeIdx];
	int ii;
	m_pTable = pNewTable;
	// @todo memset(m_pTable, 0, nNewTabSize*sizeof(OBJANDHANDLE));
	m_nTabSizeIdx++;

	// Add all current entries to the new table
	for (ii=0,pEntry=pOldTable ; ii<nOldTabSize ; ii++,pEntry++) {
		if (pEntry->pObject != NULL)
			intAddEntry(pEntry->pObject,
				pEntry->bTemp, pEntry->dwRUCount);
	}

	// Free the old table
	free(pOldTable);
	return( TRUE );
}

// FindEntry - find the entry from the handle
// If we do not find the entry, we return a pointer to an
// empty entry where it can be filled in. If bObject is TRUE
// then the pObject must also match: this is needed because
// we sometimes have double entries (e.g. stock-objects which
// are attached to two different instances).
OBJANDHANDLE* CObjFromHandle::FindEntry(
	HANDLE   hObj,
	int*     piEntry,
	CObject* pObject,
	BOOL     bObject
) {
	// Find the entry at the address or rehash
	int nTabSize = g_viTabSizes[m_nTabSizeIdx];
	int iEntry = (((int)hObj) % nTabSize);
	OBJANDHANDLE* pEntry = &m_pTable[iEntry];
	while (pEntry->pObject != NULL &&
		((bObject && pEntry->pObject != pObject) ||
		pEntry->pObject->GetObjHandle() != hObj))
	{
		iEntry++;
		pEntry++;
		if (iEntry >= nTabSize) {
			iEntry = 0;
			pEntry = m_pTable;
		}
	}
	if (piEntry != NULL)
		*piEntry = iEntry;
	return( pEntry );
}

// intAddEntry - internal add a handle/object entry -
// without locking or accounting
OBJANDHANDLE* CObjFromHandle::intAddEntry(CObject* pObject,
	BOOL  bTemp,DWORD dwRUCount)
{
	OBJANDHANDLE* pEntry;
	HANDLE hObj = pObject->GetObjHandle();
	int  nTabSize = g_viTabSizes[m_nTabSizeIdx];

	// If the table is too full, rehash it
	if (m_nTabEntries+1 > nTabSize/3) {
		if ( !RehashTable() )
			return( NULL );
		nTabSize = g_viTabSizes[m_nTabSizeIdx];
	}

	// Find the entry or the first empty (should be empty)
	// Correction: with stock objects, we may get the same
	// object twice, so we force looking for an empty here.
	pEntry = FindEntry(hObj, NULL, NULL, TRUE);

	// If this assert is thrown, then we are adding an
	// object where there is an entry with the same handle.
	ASSERT(pEntry->pObject == NULL);

	if (pEntry->pObject == NULL) {
		// Fill in the entry
		pEntry->pObject = pObject;
		pEntry->bTemp = bTemp;
		pEntry->dwRUCount = dwRUCount;
	}

	return( pEntry );
}

// AddEntry - exported function to add an object entry
// Should only be called by the Attach() function of the object itself.
BOOL CObjFromHandle::AddEntry(CObject* pObject)
{
	OBJANDHANDLE* pEntry;
	BOOL bRet;
	LOCK_CRITICAL_SECTION();
	pEntry = intAddEntry(pObject, m_bTemp, m_dwRUCount);
	bRet = (pEntry != NULL);
	if ( bRet ) {
		pEntry->dwRUCount = m_dwRUCount++;
		m_nTabEntries++;
	}
	UNLOCK_CRITICAL_SECTION();
	return( bRet );
}

// RemoveEntry - remove the given object.
// Should only be called by the destructor/Detach of the object itself.
void CObjFromHandle::RemoveEntry(CObject* pObject)
{
	LOCK_CRITICAL_SECTION();
	HANDLE hObj = pObject->GetObjHandle();
	int iEntry;
	OBJANDHANDLE* pEntry = FindEntry(hObj, &iEntry, pObject, TRUE);
	OBJANDHANDLE  entTmp;
	BOOL bFound = FALSE;
	int nTabSize = g_viTabSizes[m_nTabSizeIdx];

	// Removing an entry can be complicated if there are
	// entries directly following it: they may or may not
	// be in the right place. The easiest is to redo them.
	while (pEntry->pObject != NULL) {
		if ( !bFound ) {
			if (pEntry->pObject->GetObjHandle() == hObj) {
				pEntry->pObject = NULL;
				bFound = TRUE;
				m_nTabEntries--;
			}
		} else {
			// Copy the entry, remove it and add it again:
			// it may or may not shift forward, possibly more
			// than one if there were intermediate entries
			entTmp = *pEntry;
			pEntry->pObject = NULL;
			intAddEntry(entTmp.pObject,
				entTmp.bTemp, entTmp.dwRUCount);
		}
		iEntry++;
		pEntry++;
		if (iEntry >= nTabSize) {
			iEntry = 0;
			pEntry = m_pTable;
		}
	}
	UNLOCK_CRITICAL_SECTION();
}

// GetObject - exported function to find the object from the handle
CObject* CObjFromHandle::GetObject(HANDLE hObj)
{
	OBJANDHANDLE* pEntry = FindEntry(hObj);
	// Note: if it was not found, then FindEntry
	// returns a pointer to an "empty" (null) entry.
	if (pEntry->pObject != NULL)
		pEntry->dwRUCount = ++m_dwRUCount;
	return( pEntry->pObject );
}

// FromHandle - all of the fuss is to support this function, which
// given a handle will return the corresponding CObject, whether or
// not it existed before.
CObject* CObjFromHandle::FromHandle(HANDLE hObj)
{
	CObject* pRet;
	LOCK_CRITICAL_SECTION();
	OBJANDHANDLE* pEntry = FindEntry(hObj);
	if (pEntry->pObject == NULL) {
		m_bTemp = TRUE;
		pEntry->pObject = m_pRT->CreateObject();
		pEntry->pObject->Attach(hObj);
		m_bTemp = FALSE;
	}
	pEntry->dwRUCount = ++m_dwRUCount;
	pRet = pEntry->pObject;
	UNLOCK_CRITICAL_SECTION();
	return( pRet );
}

// Find objects that are 'older' than the given limit and remove them
// 0 = all temporary objects, -1 = half the temporaries that would
// cause a rehash of the hash table are kept.
// This whole concept is kind of dangerous, but we did not invent
// the stupidity of temporary class objects, we just
// have to support it as best we can.
// We should call this function for all our tables
// during idle time.
void CObjFromHandle::RemoveOldTemps(DWORD dwLimit /* = -1 */)
{
	LOCK_CRITICAL_SECTION();
	OBJANDHANDLE* pEntry;
	;
	int   nTabSize = g_viTabSizes[m_nTabSizeIdx];
	int   ii;

	// We will free up objects until we have half of
	// what would cause a rehash - hash tables get
	// rehashed when they are more than 1/3 full.
	if (dwLimit == -1)
		dwLimit = nTabSize / 6;
	for (ii=0,pEntry=m_pTable ; ii<nTabSize ; ) {
		if (pEntry->pObject != NULL && pEntry->bTemp &&
			m_dwRUCount-pEntry->dwRUCount > dwLimit)
		{
			// deleting the object will cause it to remove itself
			delete (pEntry->pObject);
		} else {
			ii++;
			pEntry++;
		}
	}
	UNLOCK_CRITICAL_SECTION();
}
