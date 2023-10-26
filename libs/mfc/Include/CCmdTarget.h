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
05-30-04  0.20  wdh  Added handling for WM_CREATE, SIZE, DESTROY and PAINT.

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

#ifndef CCMDTARGET_H
#define CCMDTARGET_H

#define afx_msg /**/

class CCmdTarget;

typedef void (afx_msg CCmdTarget::*AFX_PMSG)(void);

enum enumCmdTargetFunc {
	ctfEndOfTable,
	ctfOnCommand,
	ctfOnUpdate,
	ctfOnCreate,
	ctfOnSize
};

typedef int (afx_msg CCmdTarget::*OFC_ONCREATE)(LPCREATESTRUCT);
typedef void (afx_msg CCmdTarget::*OFC_ONSIZE)(UINT nType,int cx,int cy);

typedef union {
	AFX_PMSG AfxPMsg;
	void (CCmdTarget::*OnCommand)(void);
	void (CCmdTarget::*OnUpdate)(CCmdUI* pCmdUI);
	OFC_ONCREATE OnCreate;
	OFC_ONSIZE   OnSize;
} pmfUnion;

class CCmdTarget;

typedef struct tagOFCMSGENTRY {
	int      iCode;
	UINT     msg;
	enumCmdTargetFunc eType;
	AFX_PMSG pmf;
} OFCMSGENTRY;

typedef struct tagOFCMSGMAP {
	struct tagOFCMSGMAP* pBase;
	OFCMSGENTRY* pEntries;
} OFCMSGMAP;

#define DECLARE_MESSAGE_MAP() \
	static OFCMSGMAP msgMap; \
	static OFCMSGENTRY msgEntries[]; \
	virtual OFCMSGMAP* GetMessageMap() { return( &msgMap ); };

#define BEGIN_MESSAGE_MAP(thisClass, baseClass) \
	OFCMSGMAP thisClass::msgMap = { \
		&baseClass::msgMap, thisClass::msgEntries }; \
	OFCMSGENTRY thisClass::msgEntries[] = {

#define END_MESSAGE_MAP() \
		{ 0, 0, ctfEndOfTable, NULL } \
	};

	// It looks like MS hacks the table, making the cmd do double duty.
	// WM_COMMAND/BN_CLICKED is one code, so the control-id can be
	// passed too. It looks like the control ID is 0 except for WM_COMMAND.
#define ON_CONTROL(cod, id, fn) \
	{ cod, id, ctfOnCommand, (AFX_PMSG)&TARG_CLASS::fn },
#define ON_BN_CLICKED(id, fn)   ON_CONTROL(BN_CLICKED, id, fn)
#define ON_COMMAND(cmd, fn)     ON_BN_CLICKED(cmd, fn)

#define ON_WM_CREATE() \
	{ WM_CREATE, 0, ctfOnCreate, \
		(AFX_PMSG)(OFC_ONCREATE)&TARG_CLASS::OnCreate },

#define ON_WM_SIZE() \
	{ WM_SIZE, 0, ctfOnSize, (AFX_PMSG)(OFC_ONSIZE)&TARG_CLASS::OnSize },

#define ON_WM_DESTROY() \
	{ WM_DESTROY, 0, ctfOnCommand, (AFX_PMSG)&TARG_CLASS::OnDestroy },

#define ON_WM_PAINT() \
	{ WM_PAINT, 0, ctfOnCommand, (AFX_PMSG)&TARG_CLASS::OnPaint },

// This struct appears to be hidden: we may have issues of
// compatibility with IDispatch if we do not make this right.
struct AFX_CMDHANDLERINFO;

class CCmdTarget : public CObject {
	DECLARE_DYNAMIC(CCmdTarget)
public:
	// Constructor
	CCmdTarget();

	// The heart of this: the virtual command handler
	virtual BOOL OnCmdMsg(UINT wCmd,int iCode,void* pExtra,
		AFX_CMDHANDLERINFO* pCHI);

	// Utilities: Wait cursor stuff
	void BeginWaitCursor();
	void EndWaitCursor();
	void RestoreWaitCursor();
protected:
	int m_iWaitDepth;
	HCURSOR m_hOldCursor;

	DECLARE_MESSAGE_MAP();
};

struct AFX_CMDHANDLERINFO {
	CCmdTarget* pTarget;
	AFX_PMSG pmf;
};

#endif // CCMDTARGET_H
