/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CWnd.h
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the interface to the CWnd object class.
  Cwnd is a wrapper for window handles; this is pretty much
  the heart of MS Windows and hence also to OFC.

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

#ifndef CWND_H
#define CWND_H

class CCreateContext;

class CWnd : public CCmdTarget {
	DECLARE_DYNCREATE(CWnd)
public:
	CWnd();
	~CWnd();
	HWND    m_hWnd;
	WNDPROC m_fpOldWndProc; // Window procedure before sub-classing
	BOOL    m_bAutoDelete;

	// Management / FromHandle() support
	virtual HANDLE GetObjHandle() const;
static	CWnd* PASCAL FromHandle(HWND hwnd);
	virtual BOOL Attach(HANDLE hObj);
	virtual HWND Detach();

	HWND GetSafeHwnd() const;

	// Creation (with our hooking)
	BOOL CreateEx(DWORD dwExStyle,LPCTSTR pszClass,
		LPCTSTR pszTitle,DWORD dwStyle,
		int x,int y,int nWidth,int nHeight,
		HWND hwParent,HMENU hMenu,LPVOID pParam);
	BOOL Create(LPCSTR pszClass,LPCSTR pszTitle,DWORD dwStyle,
		const RECT& rc,CWnd* pParentWnd,UINT nID,
		CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();

	BOOL OnWndMsg(UINT msg,WPARAM wParam,LPARAM lParam,LRESULT* plRet);

	// Handle a message
	virtual LRESULT WindowProc(UINT message,WPARAM wParam,LPARAM lParam);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	// Clean up
	virtual void PostNcDestroy();

	// Painting
	CDC* BeginPaint(LPPAINTSTRUCT pPS);
	void EndPaint(LPPAINTSTRUCT pPS);
	void InvalidateRect(LPCRECT prc,BOOL bErase = TRUE);

	BOOL ShowWindow(int nCmdShow);
	void UpdateWindow();
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnDestroy();
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP();
};

#endif // CWND_H

