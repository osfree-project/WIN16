/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : TestCWnd.cpp
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
05-30-03  0.10  wdh  Created. This does not work yet.

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
  Copyright (c) 2004 The Open Foundation Classes
  Copyright (c) 2004 William D. Herndon
/************************************************************************/
#include "StdAfx.h"
#include "resource.h"

void dbg_printf(LPCSTR pszForm,...);

// Test the CWnd class
void TestCWnd()
{
    CWnd  wnd;
    CRect rc;
    BOOL  bResult, bRet, bAllResult = TRUE, bMsCompat = TRUE;

    rc.left = 0;
    rc.top = 0;
    rc.right = 100;
    rc.bottom = 100;
#if 0
    wnd.m_hWnd = NULL; // class has no normal constructor - set to NULL

    bRet = wnd.Create("OfcTest"/*%%*/, "Title", WS_OVERLAPPEDWINDOW, rc,
        NULL/*parent*/, (UINT)-1/*nID*/, NULL/*context*/);
    bResult = (bRet && (wnd.m_hWnd != NULL));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("Create(...)=>%d hwnd=0x%x => %s\r\n",
        bRet, wnd.m_hWnd, ResultStr(bResult));

    if (wnd.m_hWnd != NULL) {
    }
#else
    outPrintf("Under construction: Framework must work first\r\n");
#endif
    outPrintf("\r\n"
        "Summary\r\n"
        "-------\r\n"
        "MS Compatibility   = %s\r\n"
        "Full Functionality = %s\r\n",
        ResultStr(bMsCompat),
        ResultStr(bAllResult));
}
