/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : OfcTestDoc.cpp
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  Test program for OFC.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
05-30-04  0.10  wdh  Created using VC 6.0 and adapted for OFC use.

 --[ How to compile ]----------------------------------------------------

  This file was developed under DevCpp, free software from
  Bloodshed Software, http://www.bloodshed.net/

 --[ Where to get help/information ]-------------------------------------

  The author              : shadowdog@users.sourceforge.net

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
  Copyright (c) 2000-04 The Open Foundation Classes
  Copyright (c) 2004 William D. Herndon
/************************************************************************/

#include "stdafx.h"
#include "OfcTest.h"

#include "OfcTestDoc.h"


/////////////////////////////////////////////////////////////////////////////
// COfcTestDoc

IMPLEMENT_DYNCREATE(COfcTestDoc, CDocument)

BEGIN_MESSAGE_MAP(COfcTestDoc, CDocument)
	//{{AFX_MSG_MAP(COfcTestDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COfcTestDoc construction/destruction

COfcTestDoc::COfcTestDoc()
{
	// TODO: add one-time construction code here

}

COfcTestDoc::~COfcTestDoc()
{
}

BOOL COfcTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// COfcTestDoc serialization

void COfcTestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// COfcTestDoc diagnostics

#ifdef _DEBUG
void COfcTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void COfcTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COfcTestDoc commands
