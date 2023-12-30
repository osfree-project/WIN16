/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CWnd.h
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the inline interface to the CWnd object class.
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
 ************************************************************************/

#ifndef CWND_INLINE_H
#define CWND_INLINE_H

inline	HWND CWnd::GetSafeHwnd() const
	{ return( this==NULL ? NULL : m_hWnd ); };
inline BOOL CWnd::ShowWindow(int nCmdShow)
	{ return( ::ShowWindow(m_hWnd, nCmdShow) ); };
inline void CWnd::UpdateWindow()
	{ ::UpdateWindow(m_hWnd); };

#endif // CWND_INLINE_H
