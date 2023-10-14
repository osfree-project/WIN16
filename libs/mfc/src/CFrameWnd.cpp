/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CFrameWnd.cpp
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the implementation for the window class CFrameWnd.
  The CFrameWnd window can be the main window or the parent for a
  CView window or both (in single document mode).

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
05-30-04  0.10  wdh  Created.

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
  Copyright (c) 2004 William D. Herndon
/************************************************************************/
#include <afxwin.h>

IMPLEMENT_DYNCREATE(CFrameWnd, CWnd)

#define TARG_CLASS CFrameWnd
	BEGIN_MESSAGE_MAP(CFrameWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	END_MESSAGE_MAP()
#undef TARG_CLASS

CFrameWnd::~CFrameWnd()
{
	// If we are the main window, say goodbye to everybody!
	if (AfxGetApp()->m_pMainWnd == this)
		PostQuitMessage(0);
}

BOOL CFrameWnd::LoadFrame(
	UINT  nIDResource,
	DWORD dwDefStyle,
	CWnd* pwndParent,
	CCreateContext* pContext
) {
	// Try and load a menu using the resource ID
	HMENU hMenu = LoadMenu(AfxGetResourceHandle(),
		MAKEINTRESOURCE(nIDResource));

	// %% Try and load the shortcuts, deal with the icon, etc.

	if ( !CreateEx(0/*exstyle*/, NULL/*class*/, ""/*title*/,
		dwDefStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, pwndParent->GetSafeHwnd(),
		hMenu, pContext) )
	{
		return( FALSE );
	}
	m_bAutoDelete = TRUE;
	ShowWindow(SW_SHOWNORMAL);
	return( TRUE );
}

int CFrameWnd::OnCreate(LPCREATESTRUCT pCS)
{
	CCreateContext* pContext = (CCreateContext*)pCS->lpCreateParams;
	m_pView = NULL;

	// If we have an associated view, create it.
	if (pContext != NULL && pContext->m_pNewViewClass != NULL) {
		m_pView = (CView*)pContext->m_pNewViewClass->CreateObject();
		if (m_pView != NULL && m_pView->IsKindOf(RUNTIME_CLASS(CView)))
		{
			CRect rc;
			::GetClientRect(m_hWnd, &rc);
			m_pView->Create(NULL/*class*/, ""/*title*/,
				WS_CHILD|WS_VISIBLE, rc, this/*parent*/,
				(UINT)-1/*id*/, pContext);
		}
	}

	return( 0 );
}

void CFrameWnd::OnSize(UINT nType,int cx,int cy)
{
	// Update view size...
	if (m_pView != NULL) {
		HWND hwView = m_pView->GetSafeHwnd();
		if (hwView != NULL) {
			::SetWindowPos(hwView, NULL, 0, 0, cx, cy,
				SWP_NOZORDER);
		}
	}
}
