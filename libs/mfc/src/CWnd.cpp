/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CWnd.cpp
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the implementation of CWnd, a class used to wrap
  window handles. This is pretty much the heart of MS Windows
  and hence also to OFC.

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
#include <afxwin.h>

//#define _DEBUG
#ifdef _DEBUG
void dbg_printf(LPCSTR pszForm,...);
#endif


//************************************************** global variables

	// Object that supports FromHandle() of CWnd.
	CObjFromHandle g_ofhWnd(RUNTIME_CLASS(CWnd));

	CWnd* g_pWndInit = NULL;
	HHOOK g_hPrevCreateHook = NULL;

	MSG g_ofcCurMsg;

	char g_szOfcWnd[] = "OFCWnd";


IMPLEMENT_DYNCREATE(CWnd, CCmdTarget)

#define TARG_CLASS CWnd
	BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	END_MESSAGE_MAP()
#undef TARG_CLASS

CWnd::CWnd()
{
	m_hWnd = NULL;
	m_fpOldWndProc = NULL;
	m_bAutoDelete = FALSE;
}

CWnd::~CWnd()
{
	if (m_hWnd != NULL)
		Detach();
}

// Support functions for FromHandle()
HANDLE CWnd::GetObjHandle() const
{
	return( m_hWnd );
}

BOOL CWnd::Attach(HANDLE hObj)
{
	if (hObj == NULL)
		return( FALSE );
	if (m_hWnd != NULL)
		Detach();
	m_hWnd = (HWND)hObj;
	g_ofhWnd.AddEntry(this);
	return( TRUE );
}

HWND CWnd::Detach()
{
	HWND hRet = m_hWnd;
	g_ofhWnd.RemoveEntry(this);
	m_hWnd = NULL;
	return( hRet );
}

/*static*/ CWnd* CWnd::FromHandle(HWND hwnd)
{
	return( (CWnd*)g_ofhWnd.FromHandle(hwnd) );
}

// Internal version of AfxWndProc() handling:
// in case we already have the window pointer.
LRESULT intOfcCallWndProc(
	CWnd*  pWnd,
	HWND   hwnd,
	UINT   msg,
	WPARAM wParam,
	LPARAM lParam
) {
	LRESULT lResult;
	// Backup previous message (this may be a message in a message)
	MSG     prevMsg = g_ofcCurMsg;

	// Save the current message globally
	g_ofcCurMsg.hwnd    = hwnd;
	g_ofcCurMsg.message = msg;
	g_ofcCurMsg.wParam  = wParam;
	g_ofcCurMsg.lParam  = lParam;

	// 
	TRY {
		lResult = pWnd->WindowProc(msg, wParam, lParam);
	} CATCH_ALL(e) {
		lResult = -1;
		// %% Better error handling?
	} END_CATCH_ALL

	// restore the previous message
	g_ofcCurMsg = prevMsg;
	return( lResult );
}

// This is the actual Window Procedure, set in the window procedure.
// As an export, we name it as MFC does.
LRESULT CALLBACK AfxWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	CWnd* pWnd = CWnd::FromHandle(hWnd);
	return( intOfcCallWndProc(pWnd, hWnd, msg, wParam, lParam) );
}

// Register all the Window classes we will need
// Currently only one, but this will expand.
// Called by WinMain() in CWinApp.cpp
BOOL intRegisterClasses(HINSTANCE hInstance)
{
	WNDCLASS wc;

        wc.style         = 0;
	wc.lpfnWndProc   = AfxWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szOfcWnd;

	if ( !::RegisterClass(&wc) )
		return( FALSE );
	return( TRUE );
}

// This is a Windows hook set with SetWindowsHookEx() before window
// creation we use it to catch the window creation before WM_CREATE,
// replace the windows procedure with our own AfxWndProc()
// and attach the window handle to the class.
LRESULT CALLBACK intOfcHookCreate(int code,WPARAM wParam,LPARAM lParam)
{
	// If this is not window creation, we are not interested
	if (code != HCBT_CREATEWND) {
		return( CallNextHookEx(g_hPrevCreateHook,
			code, wParam, lParam) );
	}

	LPCREATESTRUCT pcs = ((LPCBT_CREATEWND)lParam)->lpcs;
	CWnd* pWndInit = g_pWndInit;
	LRESULT lResult;

	// Note: g_pWndInit is set in before window creation so
	// we can get a pointer to the CWnd instance we need.
	if (pWndInit != NULL) {
		HWND hWnd = (HWND)wParam;
		WNDPROC fpOldWndProc;

		// PreSubclassWindow() may want to create windows too...
		// we have the value from g_pWndInit, so reset it now.
		g_pWndInit = NULL;

		pWndInit->Attach(hWnd);
		pWndInit->PreSubclassWindow();
		// %% CTL3D support?
		fpOldWndProc = (WNDPROC)SetWindowLong(hWnd,
			GWL_WNDPROC, (DWORD)AfxWndProc);
		// If the original window procedure *is* AfxWndProc,
		// do not set it: we get infinite recursion!
		if (fpOldWndProc != AfxWndProc)
			pWndInit->m_fpOldWndProc = fpOldWndProc;
		else
			pWndInit->m_fpOldWndProc = NULL;
	}

	// Call the next hook in the chain
	return( CallNextHookEx(g_hPrevCreateHook, code, wParam, lParam) );
}

// This is used to set our window creation hook before a window is created
void intOfcHookWindowCreate(CWnd* pWnd)
{
	ASSERT(g_pWndInit == NULL);

	// We may still be hooked from recursive creates
	// (e.g. creating main window, which creates toolbars, etc.
	// in the creation process itself)
	// If so, skip hooking and just set the next window.
	#if 0 
	//@todo
	if (g_hPrevCreateHook == NULL) {
		g_hPrevCreateHook = ::SetWindowsHookEx(WH_CBT,
			intOfcHookCreate, NULL, 0::GetCurrentThreadId()	);
		if (g_hPrevCreateHook == NULL)
			AfxThrowMemoryException();
	}
	#endif

	g_pWndInit = pWnd;
}

// This is used to remove the window creation hook.
BOOL intOfcUnhookWindowCreate()
{
	// If we have had recursive creates, the hook
	// may already be gone, do not worry about it.
	if (g_hPrevCreateHook != NULL) {
		::UnhookWindowsHookEx(g_hPrevCreateHook);
		g_hPrevCreateHook = NULL;
	}
	// If g_pWndInit is not gone, then the hooking failed: return fail.
	if (g_pWndInit != NULL) {
		g_pWndInit = NULL;
		return( FALSE );
	}
	return( TRUE );
}

BOOL CWnd::CreateEx(
	DWORD   dwExStyle,
	LPCTSTR pszClass,
	LPCTSTR pszTitle,
	DWORD   dwStyle,
	int     x,
	int     y,
	int     nWidth,
	int     nHeight,
	HWND    hwParent,
	HMENU   hMenu,
	LPVOID  pParam
) {
	CREATESTRUCT cs;
	HWND hWnd;

	// Let the user-routine change the parameters if needed
	cs.dwExStyle  = dwExStyle;
	cs.lpszClass  = pszClass;
	cs.lpszName   = pszTitle;
	cs.style      = dwStyle;
	cs.x          = x;
	cs.y          = y;
	cs.cx         = nWidth;
	cs.cy         = nHeight;
	cs.hwndParent = hwParent;
	cs.hMenu      = hMenu;
	cs.hInstance  = AfxGetInstanceHandle();
	cs.lpCreateParams = pParam;
	if ( !PreCreateWindow(cs) ) {
		PostNcDestroy();
		return( FALSE );
	}

#if 0
	dbg_printf("CreateWindowEx(exs=0x%x, cls=%s, nam=%s, sty=0x%x\r\n"
		"..rc.., par=0x%x, mnu=0x%x, ins=0x%x, crp=0x%x)\r\n",
		cs.dwExStyle, cs.lpszClass ? cs.lpszClass : "NULL",
		cs.lpszName ? cs.lpszName : "NULL", cs.style, cs.hwndParent,
		cs.hMenu, cs.hInstance, cs.lpCreateParams);
#endif
	intOfcHookWindowCreate(this);
	hWnd = ::CreateWindowEx(cs.dwExStyle, cs.lpszClass,
		cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy,
		cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);
	if (hWnd == NULL) {
		char szMsg[256];
		//DWORD dwErr = GetLastError();
		//sprintf(szMsg, "Error in CreateWindowEx(): 0x%x", dwErr);
#if 0 // The hook has not been released: m_hWnd would get the alert window
//%%		AfxMessageBox(szMsg);
#endif
	}
	if (!intOfcUnhookWindowCreate()) {
		PostNcDestroy();
	}

	return( hWnd != NULL );
}

BOOL CWnd::Create(
	LPCSTR pszClass,
	LPCSTR pszTitle,
	DWORD  dwStyle,
	const RECT& rc,
	CWnd*  pParentWnd,
	UINT   nID,
	CCreateContext* pContext
) {
	return( CreateEx(0, pszClass, pszTitle, dwStyle | WS_CHILD,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		pParentWnd->GetSafeHwnd(), (HMENU)nID, (LPVOID)pContext) );
}

BOOL CWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (cs.lpszClass == NULL) {
		cs.lpszClass = g_szOfcWnd;
	}
	return( TRUE );
}

void CWnd::PreSubclassWindow()
{
	// Virtual for super-classing...
}

// PostNcDestroy - Do last minute "clean up"
void CWnd::PostNcDestroy()
{
}

OFCMSGENTRY* intOfcFindMsgEntry(
	OFCMSGENTRY* pEntry,
	UINT nCode,
	UINT nMsg
) {
	while (pEntry->eType != ctfEndOfTable) {
		 // %% msg/iCode correct or switched?
		if (pEntry->iCode == nCode && pEntry->msg == nMsg)
			return( pEntry );
		pEntry++;
	}
	return( NULL );
}

// OnWndMsg - This is both one of the most complex calls and the
// most often called in all of OFC.
// %% It is important that this is optimized for speed.
BOOL CWnd::OnWndMsg(UINT msg,WPARAM wParam,LPARAM lParam,LRESULT* plRet)
{
	OFCMSGENTRY* pEntry;
	OFCMSGMAP*  pMsgMap;
	UINT    nCode, nMsg;
	UINT    iHash;
	LRESULT lRet = 0;

	// Special case WM_COMMAND
	if (msg == WM_COMMAND) {
		// Check if the command handler will handle it.
		if ( OnCommand(wParam, lParam) )
			return( TRUE );
		// Otherwise pass to the message map, as usual.
		nCode = HIWORD(wParam); // Normally BN_CLICKED=0
		nMsg  = LOWORD(wParam);
	} else {
		nCode = msg;
		nMsg  = 0;
	}

	// Searching message maps is very time consuming,
	// so we speed things up by caching "recent hits"
	// and checking them before we do a conventional search.
	pMsgMap = GetMessageMap();

	// Search the message map
	for ( ; pMsgMap != NULL; pMsgMap = pMsgMap->pBase) {
		if (pMsgMap == pMsgMap->pBase) {
			// Catch infinite loop:
			// Hey user! Do not declare the base class
			// in your message map as the class itself!
			ASSERT(0);
			goto fail;
		}

		pEntry = intOfcFindMsgEntry(pMsgMap->pEntries, nCode, nMsg);
		if (pEntry != NULL)
			goto found;
	}

fail:
	return( FALSE );

found:
	pmfUnion pmf;
	pmf.AfxPMsg = pEntry->pmf;

	switch (pEntry->eType) {
	case ctfOnCommand:
		(this->*pmf.OnCommand)();
		break;
	case ctfOnCreate:
		//%% result of this?
		(this->*pmf.OnCreate)((LPCREATESTRUCT)lParam);
		break;
	case ctfOnSize:
		(this->*pmf.OnSize)(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	default:
		ASSERT(0);
		break;
	}
done:
	if (plRet != NULL)
		*plRet = lRet;
	return( TRUE );
}

LRESULT CWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	LRESULT lResult = 0;
	if ( !OnWndMsg(msg, wParam, lParam, &lResult) )
		lResult = DefWindowProc(msg, wParam, lParam);
	return( lResult );
}

LRESULT CWnd::DefWindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	// Call the original window procedure
	if (m_fpOldWndProc != NULL) {
		return( ::CallWindowProc((FARPROC)m_fpOldWndProc,
			m_hWnd, msg, wParam, lParam) );
	}
	return( ::DefWindowProc(m_hWnd, msg, wParam, lParam) );
}

BOOL CWnd::OnCommand(WPARAM wParam,LPARAM lParam)
{
	// OnWndMsg() calls us, if we return FALSE,
	// the message is passed to the message map.
	// Since this is a virtual function, the function may return TRUE.
	return( FALSE );
}

afx_msg void CWnd::OnPaint()
{
}


afx_msg void CWnd::OnDestroy()
{
	// So our HWND is removed from the global hash table
	// and our local value is set to NULL.
	if (m_hWnd != NULL)
		Detach();

	// Delete ourselves
	if ( m_bAutoDelete )
		delete this;
}
