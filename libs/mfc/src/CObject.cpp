/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CObject.cpp
  Version    : 0.30
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  Implementation of the class CObject and CRuntimeClass.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  who  ver   What
10-05-03  wdh  0.10  Created.
11-09-03  wdh  0.20  Moved bodies of MFC support functions here,
    added FromHandle() overridable functions GetObjHandle() and Attach().
05-30-04  wdh  0.30  Changed so m_pfnConstruct does the new - previously
    the v-table was not properly initialized.

 --[ How to compile ]----------------------------------------------------

  This file was developed under DevC++ 4.9.8

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

	char g_szCObject[] = "CObject";

// We are the bottom of the class hierarchy,
// so we cannot use IMPLEMENT_DYNAMIC: it does RUNTIME_CLASS()
// on the base class passed, which would croak.

	CRuntimeClass CObject::rtcCObject =
		{ g_szCObject, sizeof(CObject), NULL, NULL };

CRuntimeClass* CObject::GetRuntimeClass() const
{
	return( &CObject::rtcCObject );
}

BOOL CObject::IsKindOf(const CRuntimeClass* pClass) const
{
	if (this == NULL || pClass == NULL)
		return( FALSE );

	CRuntimeClass* pTestClass = GetRuntimeClass();
	while (pTestClass != NULL) {
		if (pTestClass == pClass)
			return( TRUE );
		pTestClass = pTestClass->m_pBaseClass;
	}
	return( FALSE );
}

// MFC support functions. We should probably do something with these.
void CObject::Serialize(CArchive& ar)
{
}

void CObject::AssertValid() const
{
}

void CObject::Dump(CDumpContext& dc) const
{
}

// Support functions for CObjFromHandle() -
// need only be implemented in those classes.
HANDLE CObject::GetObjHandle() const
{
	// Class derived from CObject must implement this
	ASSERT(0);
	return( NULL );
}

BOOL CObject::Attach(HANDLE hObj)
{
	// Class derived from CObject must implement this
	ASSERT(0);
	return( FALSE );
}

// CreateObject - create the object by allocating a memory block
// of the right size and calling ConstructObject().
CObject* CRuntimeClass::CreateObject()
{
	void* pRet = NULL;
	BOOL  bSuccess = FALSE;

	// No exceptions.
//	TRY {
		(*m_pfnConstruct)(&pRet);
//	} CATCH_ALL(e) {
//	} END_CATCH_ALL

	return( (CObject*)pRet );
}
