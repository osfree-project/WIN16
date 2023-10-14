/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : MainFrm.cpp
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
05-30-04  0.10  wdh  Created using VC 6.0 and adapted to catch menus.

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

#include "MainFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

#define TARG_CLASS CMainFrame
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND(IDM_SIMPLE_POINT, OnSimplePoint)
	ON_COMMAND(IDM_SIMPLE_SIZE, OnSimpleSize)
	ON_COMMAND(IDM_SIMPLE_RECT, OnSimpleRect)
	ON_COMMAND(IDM_SIMPLE_STRING, OnSimpleString)
	ON_COMMAND(IDM_OBJ_FILE, OnObjFile)
	ON_COMMAND(IDM_OBJ_GDI, OnObjGdi)
	ON_COMMAND(IDM_OBJ_CMDTARGET, OnObjCmdtarget)
	ON_COMMAND(IDM_WND_WND, OnWndWnd)
	ON_COMMAND(IDM_FILE_EXIT, OnFileExit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#undef TARG_CLASS

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnFileExit() 
{
	::PostMessage(m_hWnd, WM_CLOSE, 0, 0L);
}

void CMainFrame::OnSimplePoint() 
{
	TestCPoint();
}

void CMainFrame::OnSimpleSize() 
{
	TestCSize();
}

void CMainFrame::OnSimpleRect() 
{
	TestCRect();
}

void CMainFrame::OnSimpleString() 
{
	TestCString();
}

void CMainFrame::OnObjFile() 
{
	TestCFile();
}

void CMainFrame::OnObjGdi() 
{
	TestCGdiObject();
}

void CMainFrame::OnObjCmdtarget() 
{
	TestCCmdTarget();
}

void CMainFrame::OnWndWnd() 
{
	TestCWnd();
}

