/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CWinApp.h
  Version    : 0.10
  Author(s)  : William D. Herndon
 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the include for the class CWinApp.
  The CWinApp is the global application class. It is never instantiated
  directly, but is used as the basis for the actual application class.

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
  Copyright (c) 2000-2004 Open Foundation Classes
  Copyright (c) 2004 William D. Herndon
/************************************************************************/
#ifndef CWINAPP_H
#define CWINAPP_H

class CWinApp : public CCmdTarget {
	DECLARE_DYNCREATE(CWinApp)
public:
	CWinApp();

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void AddDocTemplate(CDocTemplate* pTemplate);

	void ParseCommandLine(CCommandLineInfo& rCmdInfo);
	BOOL ProcessShellCommand(CCommandLineInfo& rCmdInfo);

	CDocTemplate* m_pTemplate; // %% We should have an array: CArray...

	CWnd* m_pMainWnd;

	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-170#domessagebox
	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
protected:
	DECLARE_MESSAGE_MAP();
};

CWinApp* AfxGetApp();

//https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display?view=msvc-170#afxmessagebox

int AfxMessageBox(LPCSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0);
int AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT) -1);

#endif // CWINAPP_H
