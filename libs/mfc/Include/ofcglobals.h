/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : ofcglobals.h
  Version    : 0.20
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
11-09-03  0.10  wdh  Created.
05-30-04  0.20  wdh  Added ASSERT() and ASSERT_VALID(), CWinApp,
    g_hOfcInstanceHandle, g_pWinApp, and AfxGet/SetInstanceHandle().

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
/************************************************************************/

#ifndef OFCGLOBALS_H
#define OFCGLOBALS_H

#ifdef _DEBUG
#define ASSERT(x)       if ( !(x) ) { _asm int 3 }
#define ASSERT_VALID(x) if ( IsBadWritePtr((x), sizeof(*(x))) ) { _asm int 3 }
#else
#define ASSERT(x)       /**/
#define ASSERT_VALID(x) /**/
#endif

class CWinApp;

extern HINSTANCE g_hOfcResourceHandle;
extern HINSTANCE g_hOfcInstanceHandle;
extern CWinApp*  g_pWinApp;

inline void AfxSetResourceHandle(HINSTANCE hInstance) {
	g_hOfcResourceHandle = hInstance;
};
inline HINSTANCE AfxGetResourceHandle() {
	return( g_hOfcResourceHandle );
};

inline void AfxSetInstanceHandle(HINSTANCE hInstance) {
	g_hOfcInstanceHandle = hInstance;
};
inline HINSTANCE AfxGetInstanceHandle() {
	return( g_hOfcInstanceHandle );
};

#endif // OFCGLOBALS_H
