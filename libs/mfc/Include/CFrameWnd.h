/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CFrameWnd.h
  Version    : 0.10
  Author(s)  : William D. Herndon
 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the include for the window class CFrameWnd.
  The CFrameWnd window is a parent for a CView window.

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
  Copyright (c) 2000-2004 Open Foundation Classes
  Copyright (c) 2004 William D. Herndon
 ************************************************************************/
#ifndef CFRAMEWND_H
#define CFRAMEWND_H

class CFrameWnd : public CWnd {
	DECLARE_DYNCREATE(CFrameWnd)
	virtual ~CFrameWnd();
protected:
	CView* m_pView;
public:
	virtual BOOL LoadFrame(UINT nIDResource,DWORD dwDefStyle,
		CWnd* pwndParent,CCreateContext* pContext);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT pCS);
	afx_msg void OnSize(UINT nType,int cx,int cy);

	DECLARE_MESSAGE_MAP();
};

#endif // CFRAMEWND_H
