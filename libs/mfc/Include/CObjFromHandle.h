/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CObjFromHandle.h
  Version    : 0.20
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the interface for CObjFromHandle, a class used to
  implement the "FromHandle()" member functions. Note that this
  must be super-fast - it will be called very often.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
11-09-03  0.10  wdh  Created.
05-30-04  0.20  wdh  Changed FindEntry() to fix problem supporting
    stock objects.

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

#ifndef COBJFROMHANDLE_H
#define COBJFROMHANDLE_H

typedef BOOL (*FPISHANDLEVALID)(HANDLE hObj);

typedef struct tagOBJANDHANDLE {
	CObject* pObject;
	BOOL     bTemp;
	DWORD    dwRUCount; // This indicates how recent the last use was
} OBJANDHANDLE;

class CObjFromHandle {
public:
	CObjFromHandle(CRuntimeClass* pRT);
	virtual ~CObjFromHandle();

protected:
	BOOL RehashTable();
	OBJANDHANDLE* FindEntry(HANDLE hObj,int* piEntry = NULL,
		CObject* pObject = NULL,BOOL bObject = FALSE);
	OBJANDHANDLE* intAddEntry(CObject* pObject,BOOL bTemp,DWORD dwRUCount);

public:
	BOOL AddEntry(CObject* pObject);    // Call in Attach()
	void RemoveEntry(CObject* pObject); // Call in destructor/Detach()
	CObject* GetObject(HANDLE hObj);    // Find an existing entry
	CObject* FromHandle(HANDLE hObj);   // Find entry or create temporary
	void RemoveOldTemps(DWORD dwLimit = -1); // Remove "old" entries

protected:
	CRuntimeClass* m_pRT;
	int            m_nTabEntries;
	int            m_nTabSizeIdx;
	OBJANDHANDLE*  m_pTable;
	DWORD          m_dwRUCount;
	BOOL           m_bTemp; // TRUE iff FromHandle is creating a temporary
};

#endif // COBJFROMHANDLE_H
