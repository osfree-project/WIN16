/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : afxwin.h
  Version    : 0.60
  Author(s)  : William D. Herndon
 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This is the main include file for projects using OFC: all it really
  does is include the other exported include files. It has the same name
  as the main include file for MFC, so projects will not need to change.

  --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
10-05-03  0.10  wdh  Created.
10-12-03  0.20  wdh  Changed from windef.h to windows.h to be MS compatible.
11-02-03  0.30  wdh  Added CGdiObject*.h includes.
11-09-03  0.40  wdh  Added ofcglobals.h and CObjFromHandle.h
11-25-03  0.50  wdh  Added CCmdUI and CCmdTarget.
05-30-04  0.60  wdh  Added CArchive, CWnd, CDocument, CDocTemplate, CView,
    CFrameWnd, CWinApp and the CCommandLineInfo hack and CCreateContext.

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
#ifndef __AFXWIN_H__
#define __AFXWIN_H__


#include <windows.h> // Standard Windows includes from CygWin or MS




#include <ofcglobals.h>
#include <CPoint.h>
#include <CSize.h>
#include <CRect.h>
#include <CString.h>
#include <CString_inline.h>
#include <CObject.h>
#include <CException.h>
#include <CFile.h>
#include <CArchive.h>
#include <CArchive_inline.h>
#include <CObjFromHandle.h>
#include <CGdiObject.h>
#include <CGdiObject_inline.h>
#include <CCmdUI.h>
#include <CCmdTarget.h>
#include <CCmdTarget_inline.h>
#include <CWnd.h>
#include <CWnd_inline.h>
#include <CDocument.h>
#include <CDocument_inline.h>
#include <CDocTemplate.h>
#include <CDocTemplate_inline.h>
#include <CView.h>
#include <CView_inline.h>
#include <CFrameWnd.h>
#include <CFrameWnd_inline.h>

#if 1 // Quick hack
class CCommandLineInfo {
public:
    int m_iGarbage;
};

class CCreateContext {
public:
	CCreateContext(CRuntimeClass* pViewClass,CDocument* pDoc,
		CDocTemplate* pNewDocTemplate)
	{
		m_pNewViewClass   = pViewClass;
		m_pCurrentDoc     = pDoc;
		m_pNewDocTemplate = pNewDocTemplate;
	};

	CRuntimeClass* m_pNewViewClass;
	CDocument*     m_pCurrentDoc;
	CDocTemplate*  m_pNewDocTemplate;
};

#endif

#include <CWinApp.h>
#include <CWinApp_inline.h>

#endif // __AFXWIN_H__
