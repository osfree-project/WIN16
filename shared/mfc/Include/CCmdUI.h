/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CCmdTarget.h
  Version    : 0.20
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the interface to the CCmdTarget object class
  CCmdTarget is used as the basis for other classes that accept
  commands, most notably CWnd.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
11-25-03  0.10  wdh  Created.
05-30-04  0.20  wdh  Added some commentary for m_bContinueRouting.

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
 ************************************************************************/

#ifndef CCMDUI_H
#define CCMDUI_H

class CCmdUI {
public:
    // This is part of a hack allowing ExecMsg()
    // to fail when ContinueRouting() is called.
	BOOL  m_bContinueRouting;

	CCmdUI(HWND hwTool);
	CCmdUI(HMENU hMenu,UINT wMenuItem);

	virtual void Enable(BOOL bOn = TRUE);
	virtual void SetCheck(int iCheck = 1);
	virtual void SetRadio(BOOL bOn = TRUE);
	virtual void SetText(LPCSTR pszText);
	virtual void ContinueRouting();
private:
	// If it is a tool
	HWND  m_hwTool;

	// If it is a menu
	HMENU m_hMenu;
	UINT  m_wMenuItem;
};

#endif // CCMDUI_H
