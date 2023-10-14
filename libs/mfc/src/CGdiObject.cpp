/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CGdiObject.cpp
  Version    : 0.20
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the implementation of the GDI object classes:
  CGdiObject, CBitmap, CBrush, CFont, CPen and CDC.
  All of these objects except CDC are based on CGdiObject, allowing
  the basic functions, such as Attach(), Detach(), DeleteObject().
  Most of the function implementations are so simple that they
  are implemented inline in CGdiObject_inline.h

  WARNING: For correct FromHandle() support, never access m_hObject or
  m_hDC directly, except in the access functions Attach(), Detach() and
  GetObjHandle().

 --[ Developers ]--------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

 --[ History ] ----------------------------------------------------------

mm-dd-yy  ver   who  what
11-01-03  0.10  wdh  Original implementation.
11-09-03  0.20  wdh  Added FromHandle() functionality.

 --[ To Do ]-------------------------------------------------------------
  Format functions, operators >> and <<, Unicode and MBCS support.

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
  Copyright (c) 2000-03 Open Foundation Classes
  Copyright (c) 2003 William D. Herndon
/************************************************************************/

#include <afxwin.h>

CObjFromHandle g_ofhGDI(RUNTIME_CLASS(CGdiObject));
CObjFromHandle g_ofhDC(RUNTIME_CLASS(CDC));

IMPLEMENT_DYNAMIC(CGdiObject, CObject)

CGdiObject::~CGdiObject()
{
	DeleteObject();
}

// Support functions for FromHandle()
HANDLE CGdiObject::GetObjHandle() const
{
	return( m_hObject );
}

BOOL CGdiObject::Attach(HGDIOBJ hObject)
{
	if (hObject == NULL)
		return( FALSE );
	if (m_hObject != NULL)
		Detach();
	m_hObject = hObject;
	g_ofhGDI.AddEntry(this);
	return( TRUE );
}

HGDIOBJ CGdiObject::Detach()
{
	HGDIOBJ hRet = m_hObject;
	g_ofhGDI.RemoveEntry(this);
	m_hObject = NULL;
	return( hRet );
}

/*static*/ CGdiObject* CGdiObject::FromHandle(HGDIOBJ hObj)
{
	return( (CGdiObject*)g_ofhGDI.FromHandle(hObj) );
}



IMPLEMENT_DYNAMIC(CBitmap, CGdiObject)


IMPLEMENT_DYNAMIC(CBrush, CGdiObject)


IMPLEMENT_DYNAMIC(CFont, CGdiObject)


IMPLEMENT_DYNAMIC(CPen, CGdiObject)


IMPLEMENT_DYNAMIC(CDC, CObject)

// Support functions for FromHandle()
HANDLE CDC::GetObjHandle() const
{
	return( m_hDC );
}

BOOL CDC::Attach(HANDLE hDC)
{
	if (hDC == NULL)
		return( FALSE );
	m_hDC = (HDC)hDC;
	g_ofhDC.AddEntry(this);
	return( TRUE );
}

HDC CDC::Detach()
{
	HDC hRet = m_hDC;
	g_ofhDC.RemoveEntry(this);
	m_hDC = NULL;
	return( hRet );
}

/*static*/ CDC* CDC::FromHandle(HDC hdc)
{
	return( (CDC*)g_ofhDC.FromHandle(hdc) );
}


// We have to return a pointer to an object and
// automatically dispose of it later without being
// able to test if it is still in use ...
// wonderful MS architecture.
// I implement this by keeping a pointer to the last
// item selected, initialized to a member variable of appropriate
// type at the beginning. The first time it is called, I return a
// pointer to the member variable, where I fill in the handle with
// the original GDI handle. Voila! -wdh

CDC::CDC()
{
	m_hDC = NULL;
	m_pBmpPrev   = &m_bmpOrig;
	m_pBrushPrev = &m_brushOrig;
	m_pFontPrev  = &m_fontOrig;
	m_pPenPrev   = &m_penOrig;
}

CDC::~CDC()
{
	if (m_hDC != NULL)
		DeleteDC();
}

CBitmap* CDC::SelectObject(CBitmap* pBmp)
{
	HANDLE hObj;
	CBitmap* pBmpPrev = m_pBmpPrev;
	m_pBmpPrev = pBmp;
	hObj = ::SelectObject(m_hDC, pBmp->GetObjHandle());
	if (m_bmpOrig.GetObjHandle() == NULL)
		m_bmpOrig.Attach(hObj);
	return( pBmpPrev );
}

CBrush* CDC::SelectObject(CBrush* pBrush)
{
	HANDLE hObj;
	CBrush* pBrushPrev = m_pBrushPrev;
	m_pBrushPrev = pBrush;
	hObj = ::SelectObject(m_hDC, pBrush->GetObjHandle());
	if (m_brushOrig.GetObjHandle() == NULL)
		m_brushOrig.Attach(hObj);
	return( pBrushPrev );
}

CFont* CDC::SelectObject(CFont* pFont)
{
	HANDLE hObj;
	CFont* pFontPrev = m_pFontPrev;
	m_pFontPrev = pFont;
	hObj = ::SelectObject(m_hDC, pFont->GetObjHandle());
	if (m_fontOrig.GetObjHandle() == NULL)
		m_fontOrig.Attach(hObj);
	return( pFontPrev );
}

CPen* CDC::SelectObject(CPen* pPen)
{
	HANDLE hObj;
	CPen* pPenPrev = m_pPenPrev;
	m_pPenPrev = pPen;
	hObj = ::SelectObject(m_hDC, pPen->GetObjHandle());
	if (m_penOrig.GetObjHandle() == NULL)
		m_penOrig.Attach(hObj);
	return( pPenPrev );
}

