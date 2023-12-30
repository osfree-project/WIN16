/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CWinApp.cpp
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the implementation for the class CWinApp.
  The CWinApp is the global application class. It is never instantiated
  directly, but is used as the basis for the actual application class.
  We have WinMain here (at least in the static library) - the
  application itself does not have it, and this is the logical place
  to put it.

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

BOOL intRegisterClasses(HINSTANCE hInstance); // From CWnd.cpp

	CWinApp* g_pWinApp = NULL;

IMPLEMENT_DYNCREATE(CWinApp, CCmdTarget)


	BEGIN_MESSAGE_MAP(CWinApp, CCmdTarget)
	END_MESSAGE_MAP()

CWinApp::CWinApp()
{
	// If this assert is thrown, you have
	// two or more instances of CWinApp.
	ASSERT(g_pWinApp == NULL);
	g_pWinApp   = this;
	m_pTemplate = NULL;
	m_pMainWnd  = NULL;
	m_nCmdShow  = 0;
}

BOOL CWinApp::InitInstance()
{
	return( TRUE );
}

int CWinApp::ExitInstance()
{
	return( 0 );
}

void CWinApp::AddDocTemplate(CDocTemplate* pTemplate)
{
	// Add this to a list of document templates...
	// %% Currently we support one, until we get CArray implemented.
	m_pTemplate = pTemplate;
}

void CWinApp::ParseCommandLine(CCommandLineInfo& rCmdInfo)
{
  // Parse the global command line (in our instance)
  // and store the result in rCmdInfo
  // NYI
}

BOOL CWinApp::ProcessShellCommand(CCommandLineInfo& rCmdInfo)
{
	// Without a template (AddDocTemplate()),
	// we cannot create the windows we need to.
	ASSERT(m_pTemplate != NULL);

	// Take the command in rCmdInfo and execute it

	// This generally creates the main frame, so we do that here.
	// This should probably really occur somewhere else.
	m_pMainWnd = (CWnd*)m_pTemplate->m_pFrameClass->CreateObject();
	ASSERT(m_pMainWnd);
	ASSERT(m_pMainWnd->IsKindOf(RUNTIME_CLASS(CWnd)));

	// Create the create context
	CCreateContext ctx(m_pTemplate->m_pViewClass,
		NULL/*doc*/, m_pTemplate);

	// If this is a frame window (it should be), then use LoadFrame(),
	// otherwise just do a CreateEx().
	if ( m_pMainWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)) ) {
		CFrameWnd* pMainFrame = (CFrameWnd*)m_pMainWnd;
		if ( !pMainFrame->LoadFrame(m_pTemplate->m_nIDResource,
			WS_OVERLAPPEDWINDOW, NULL/*parent*/, &ctx) )
		{
			return( FALSE );
		}
	} else {
		if ( !m_pMainWnd->CreateEx(0/*exstyle*/, NULL/*class*/,
			""/*title*/, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL/*parent*/, NULL/*menu*/, &ctx) )
		{
			return( FALSE );

		}
	}

	return( TRUE );
}

	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-170#domessagebox
int CWinApp::DoMessageBox(LPCSTR lpszPrompt, UINT nType, UINT nIDPrompt)
{
	if ((nType & MB_ICONMASK)==0)
	{
		switch (nType & MB_TYPEMASK)
		{
		case MB_OK:
		case MB_OKCANCEL:
			nType|=MB_ICONEXCLAMATION;
			break;

		case MB_YESNO:
		case MB_YESNOCANCEL:
			nType|=MB_ICONQUESTION;
			break;
		}
	}

	// @todo Need to add application title
	return ::MessageBox(0, lpszPrompt, NULL, nType);
}

int WINAPI WinMain(
    HINSTANCE hThisInstance,
    HINSTANCE hPrevInstance,
    LPSTR     pszArgs,
    int       nShowWindow
) {
	CWinApp* pApp = AfxGetApp();

	pApp->m_nCmdShow = nShowWindow;

	if ( intRegisterClasses(hThisInstance) ) {
		if ( pApp->InitInstance() ) {
			MSG  msg;
			while ( ::GetMessage(&msg, NULL, 0, 0) ) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}
	return( pApp->ExitInstance() );
}

int AfxMessageBox(LPCSTR lpszText, UINT nType, UINT nIDHelp)
{
	return AfxGetApp()->DoMessageBox(lpszText, nType, nIDHelp);
}

int AfxMessageBox(UINT nIDPrompt, UINT nType, UINT nIDHelp)
{
	CString str;
	
	str.LoadString(nIDPrompt);
	if (nIDHelp==(UINT)-1) nIDHelp=nIDPrompt;
	return AfxGetApp()->DoMessageBox(str, nType, nIDHelp);
}
