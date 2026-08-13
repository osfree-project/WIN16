/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CCmdUI.cpp
  Version    : 0.20
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the implementation of the CCmdUI class.
  CCmdUI is passed to update functions on the message map to allow
  update of the command source, e.g. menu or tool icon.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
11-25-03  0.10  wdh  Created.
05-30-04  0.20  wdh  Fixed SetRadio() to work better.

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
#include <afxwin.h>

CCmdUI::CCmdUI(HWND hwTool)
{
	m_hwTool = hwTool;
	m_hMenu  = NULL;
	m_bContinueRouting = FALSE;
}

CCmdUI::CCmdUI(HMENU hMenu,UINT wMenuItem)
{
	m_hwTool    = NULL;
	m_hMenu     = hMenu;
	m_wMenuItem = wMenuItem;
	m_bContinueRouting = FALSE;
}

void CCmdUI::Enable(BOOL bOn)
{
	if (m_hwTool != NULL) {
		EnableWindow(m_hwTool, bOn);
	} else if (m_hMenu != NULL) {
		::EnableMenuItem(m_hMenu, m_wMenuItem,
			bOn ? MF_BYCOMMAND|MF_ENABLED :
				MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
	}
}

void CCmdUI::SetCheck(int iCheck)
{
	if (m_hwTool != NULL) {
		::SendMessage(m_hwTool, BM_SETCHECK, iCheck, 0);
	} else if (m_hMenu != NULL) {
		::CheckMenuItem(m_hMenu, m_wMenuItem,
			(iCheck == 0) ? MF_BYCOMMAND|MF_UNCHECKED :
				MF_BYCOMMAND|MF_CHECKED);
	}
}

void CCmdUI::SetRadio(BOOL bOn)
{
	if (m_hwTool != NULL) {
		// This should work for "automatic" radio buttons,
		// should we do anything else?
		::SendMessage(m_hwTool, BM_SETCHECK, bOn, 0);
	} else if (m_hMenu != NULL) {
		// %% Uncheck other menu items?
		// %% what if bOn==FALSE?
		::CheckMenuItem(m_hMenu, m_wMenuItem,
			bOn ? MF_BYCOMMAND|MF_CHECKED :
				MF_BYCOMMAND|MF_UNCHECKED);
	}
}

void CCmdUI::SetText(LPCSTR pszText)
{
	if (m_hwTool != NULL) {
		::SetWindowText(m_hwTool, pszText);
	} else if (m_hMenu != NULL) {
		::ModifyMenu(m_hMenu, m_wMenuItem, MF_BYCOMMAND|MF_STRING,
			m_wMenuItem, pszText);
	}
}

void CCmdUI::ContinueRouting()
{
	m_bContinueRouting = TRUE;
}
