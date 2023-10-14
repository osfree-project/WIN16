/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : OfcTest.cpp
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  Test program for OFC.

  Defines the class behaviors for the application.


 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
mm-dd-yy  ver   who  what
10-01-03  0.10  wdh  First beta version.
10-26-03  0.20  wdh  Split off test routines into separate files.
                     Added AfxSetResourceHandle() call for true MFC.
11-02-03  0.30  wdh  Added testing for CFile and CGdiObject.
11-09-03  0.40  wdh  OFC now supports AfxSetResourceHandle() too.
11-25-03  0.50  wdh  Added TestCCmdTarget().
05-30-03  0.60  wdh  Recreated using VC 6.0 - almost all of the content
    has wandered off to other modules.

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
  Copyright (c) 2000-2004 The Open Foundation Classes
  Copyright (c) 2003-2004 William D. Herndon
/************************************************************************/

#include "stdafx.h"
#include "OfcTest.h"

#include "MainFrm.h"
#include "OfcTestDoc.h"
#include "OfcTestView.h"

/////////////////////////////////////////////////////////////////////////////
// COfcTestApp

BEGIN_MESSAGE_MAP(COfcTestApp, CWinApp)
	//{{AFX_MSG_MAP(COfcTestApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COfcTestApp construction

COfcTestApp::COfcTestApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only COfcTestApp object

COfcTestApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COfcTestApp initialization

BOOL COfcTestApp::InitInstance()
{
#if 0 // NYI in OFC
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("ShadowDog"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
#endif

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(COfcTestDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(COfcTestView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	// OFC Note: This initializes the view window, so we cannot live without it
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// COfcTestApp message handlers


