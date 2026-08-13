/************************************************************************
   T h e   O p e n   F o u n d a t i o n   C l a s s e s
 ------------------------------------------------------------------------
   Filename   : CObject.h
   Version    : 0.40
   Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file declares the classes CObject and CRuntimeClass and several
  crucial macros. CObject is the basis of almost all other classes in
  OFC / MFC. CRuntimeClass is a class that is used at runtime to
  test the class of a CObject derivative and sometimes to actually
  create objects dynamically (see IMPLEMENT_DYNCREATE).

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
10-05-03  0.10  wdh  Created.
11-09-03  0.20  wdh  Added virtual overridables needed by FromHandle(),
    moved bodies of virtual functions to cpp file.
11-25-03  0.30  wdh  Fixed IMPLEMENT_DYNCREATE() macro.
05-30-04  0.40  wdh  Changed Construct() to do the new itself: otherwise
    the v-table is not correctly initialized.

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
 Copyright (c) 2000-04 by The Open Foundation Classes
 Copyright (c) 2003-04 by William D. Herndon
 ************************************************************************/

#ifndef COBJECT_H
#define COBJECT_H

class CObject;
class CArchive;
class CDumpContext;

// RUNTIME_CLASS() returns the CRuntimeClass* for the given class
#define RUNTIME_CLASS(className) \
	(&className::rtc##className)

// DECLARE_DYNAMIC() is included in the class declaration to declare
// the CRuntimeClass* associated with the class and GetRuntimeClass()
#define DECLARE_DYNAMIC(className) \
public: \
	static CRuntimeClass rtc##className; \
	virtual CRuntimeClass* GetRuntimeClass() const;
// End of DECLARE_DYNAMIC


// DECLARE_DYNCREATE() is like DECLARE_DYNAMIC(), but is for CObjects
// that are constructed using the runtime class
#define DECLARE_DYNCREATE(className) \
	DECLARE_DYNAMIC(className) \
	static void Construct(void** ppRet);
// End of DECLARE_DYNCREATE

// _IMPLEMENT_RUNTIMECLASS() is the common part of IMPLEMENT_DYNAMIC()
// and IMPLEMENT_DYNCREATE: it implements the CRuntimeClass associated
// with the class and GetRuntimeClass()
#define _IMPLEMENT_RUNTIMECLASS(className, baseClassName, pfnNew) \
	static char g_psz##className[] = #className; \
	CRuntimeClass className::rtc##className = { \
		g_psz##className, sizeof(className), pfnNew, \
		RUNTIME_CLASS(baseClassName) }; \
	CRuntimeClass* className::GetRuntimeClass() const \
		{ return( &className::rtc##className ); };
// End of _IMPLEMENT_RUNTIMECLASS

// IMPLEMENT_DYNAMIC() is simply _IMPLEMENT_RUNTIMECLASS(),
// with NULL for the constructor parameter.
#define IMPLEMENT_DYNAMIC(className, baseClassName) \
	_IMPLEMENT_RUNTIMECLASS(className, baseClassName, NULL)
// End of IMPLEMENT_DYNAMIC

// IMPLEMENT_DYNCREATE() implements a constructor for the class, and
// then passes it as the constructor parameter to _IMPLEMENT_RUNTIMECLASS.
#define IMPLEMENT_DYNCREATE(className, baseClassName) \
	void className::Construct(void** ppRet) { *ppRet = new className; }; \
	_IMPLEMENT_RUNTIMECLASS(className, baseClassName, \
		className::Construct)
// End of IMPLEMENT_DYNCREATE

// CRuntimeClass: 
// Note that the order of the entries
// here must match the order used in the macros above!
class CRuntimeClass {
public:
	LPCSTR   m_pszClassName;
	int      m_nObjectSize;
	void    (*m_pfnConstruct)(void**);
	CRuntimeClass* m_pBaseClass;

	CObject* CreateObject();
	void Store(CArchive& ar);
static CRuntimeClass* Load(CArchive& ar);
};

// CObject: the basis for most classes, no object is ever this class
// by itself, a derived class is always used.
class CObject {
	DECLARE_DYNAMIC(CObject)
	virtual ~CObject() { };

protected:
	CObject() { };
public:
	BOOL IsSerializable() const;
	BOOL IsKindOf(const CRuntimeClass* pClass) const;

// Overridables
	virtual void Serialize(CArchive& ar);

// Diagnostic Support: nothing at the moment - this is declared to remain
// MFC compatible, we should probably do something with this.
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;

	// Support functions for CObjFromHandle() -
	// need only be implemented in those classes.
	virtual HANDLE GetObjHandle() const;
	virtual BOOL Attach(HANDLE hObj);
};

#endif // COBJECT_H
