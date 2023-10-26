/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CCmdTarget.cpp
  Version    : 0.20
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the implementation of CCmdTarget, a class used to
  pass commands to an appropriate handler. This is the base class
  for the many important classes in OFC and very critical to
  correct functioning.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
11-25-03  0.10  wdh  Created.
05-30-04  0.20  wdh  Fixed routing for ctfOnUpdate, added ctfOnCreate.

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
#include <afxwin.h>

void dbg_printf(LPCSTR pszForm,...);


IMPLEMENT_DYNAMIC(CCmdTarget, CObject)

	// BEGIN_MESSAGE_MAP(CCmdTarget, xxx): we are the last
	// class with a msgMap, so we cannot use the macro.
	OFCMSGMAP CCmdTarget::msgMap = {
		NULL, CCmdTarget::msgEntries };
	OFCMSGENTRY CCmdTarget::msgEntries[] = {
	// end of BEGIN_MESSAGE_MAP(CCmdTarget, xxx)

	END_MESSAGE_MAP()


CCmdTarget::CCmdTarget()
{
}

// Execute the specified message for the given entry.
// For normal Window's messages, iCode is wParam and
// pExtra is usually lParam, but we are also use for
// non-Windows messages.
BOOL ExecMsg(
	CCmdTarget*  pTarg,
	OFCMSGENTRY* pEntry,
	UINT  msg,
	int   iCode,
	void* pExtra
) {
	pmfUnion pmf;
	pmf.AfxPMsg = pEntry->pmf;

	switch (pEntry->eType) {
	case ctfOnCommand:
		(pTarg->*pmf.OnCommand)();
		break;
	case ctfOnUpdate: {
		CCmdUI* pCmdUI = (CCmdUI*)pExtra;
		BOOL    bResult;

		(pTarg->*pmf.OnUpdate)(pCmdUI);
		// Nice little hack: if the update function called
		// ContinueRouting() then m_bContinueRouting was set TRUE,
		// return that as our result and reset the flag.
		// The caller then knows this failed and should try the
		// parent or whatever.
		bResult = pCmdUI->m_bContinueRouting;
		pCmdUI->m_bContinueRouting = FALSE;
		return( bResult );
	}
	case ctfOnCreate:
		return( (pTarg->*pmf.OnCreate)((LPCREATESTRUCT)pExtra) );
	}
	return( TRUE );
}

// The heart of CCmdTarget: the virtual command handler
// This is called *extremely* often: we need some sort of caching
// or something, so that for frequent messages (e.g. WM_MOUSEMOVE),
// the result is found immediately. -wdh
BOOL CCmdTarget::OnCmdMsg(
	UINT  msg,
	int   iCode,
	void* pExtra,
	AFX_CMDHANDLERINFO* pCHI
) {
	OFCMSGMAP* pMap = GetMessageMap();
	OFCMSGENTRY* pEntry;

	// Check this map, then, if that fails, the base map, recursively
	while (pMap != NULL) {
		// Check each entry in the map
		pEntry=pMap->pEntries;
		for ( ; pEntry->eType != ctfEndOfTable ; pEntry++) {
			// If it matches, execute the associated function or,
			// if an info structure has been passed, fill it in.
			if (pEntry->msg == msg && pEntry->iCode == iCode) {
				BOOL bResult;

				if (pCHI == NULL) {
					bResult = ExecMsg(this, pEntry,
						msg, iCode, pExtra);
				} else {
					pCHI->pTarget = this;
					pCHI->pmf = pEntry->pmf;
					bResult = TRUE;
				}
				return( bResult );
			}
		}
		pMap = pMap->pBase;
	}
	return( FALSE );
}
