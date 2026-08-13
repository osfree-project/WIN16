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
 ************************************************************************/
#ifndef CWINAPP_H
#define CWINAPP_H

class CWinApp : public CCmdTarget {
	DECLARE_DYNCREATE(CWinApp)
public:
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#cwinapp
	CWinApp(LPCTSTR lpszAppName = NULL);	

	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#adddoctemplate
	virtual void AddDocTemplate(CDocTemplate* pTemplate);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#addtorecentfilelist	
	//virtual void AddToRecentFileList(LPCTSTR lpszPathName);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#applicationrecoverycallback
	//virtual DWORD ApplicationRecoveryCallback(LPVOID lpvParam);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#closealldocuments
	//void CloseAllDocuments(BOOL bEndSession);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#createprinterdc
	//BOOL CreatePrinterDC(CDC& dc);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#delregtree
	//LONG DelRegTree(HKEY hParentKey, const CString& strKeyName);
	//LONG DelRegTree(HKEY hParentKey, const CString& strKeyName, CAtlTransactionManager* pTM = NULL);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#domessagebox
	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#dowaitcursor
	//virtual void DoWaitCursor(int nCode);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#enabled2dsupport
	//BOOL EnableD2DSupport(D2D1_FACTORY_TYPE d2dFactoryType = D2D1_FACTORY_TYPE_SINGLE_THREADED, DWRITE_FACTORY_TYPE writeFactoryType = DWRITE_FACTORY_TYPE_SHARED);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#enablehtmlhelp
	//void EnableHtmlHelp();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#enabletaskbarinteraction
	//BOOL EnableTaskbarInteraction(BOOL bEnable = TRUE);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#exitinstance
	virtual int ExitInstance();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getapplicationrecoveryparameter
	//virtual LPVOID GetApplicationRecoveryParameter();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getapplicationrecoverypinginterval
	//virtual DWORD GetApplicationRecoveryPingInterval();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getapplicationrestartflags
	//virtual DWORD GetApplicationRestartFlags();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getappregistrykey
	//HKEY GetAppRegistryKey(CAtlTransactionManager* pTM = NULL);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getdatarecoveryhandler
	//virtual CDataRecoveryHandler *GetDataRecoveryHandler();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getfirstdoctemplateposition
	//POSITION GetFirstDocTemplatePosition() const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#gethelpmode
	//AFX_HELP_TYPE GetHelpMode();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getnextdoctemplate
	//CDocTemplate* GetNextDocTemplate(POSITION& pos) const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getprinterdevicedefaults
	//BOOL GetPrinterDeviceDefaults(struct tagPDA* pPrintDlg);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getprofilebinary
	//BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getprofileint
	//UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getprofilestring
	//CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#getsectionkey
	//HKEY GetSectionKey(LPCTSTR lpszSection, CAtlTransactionManager* pTM = NULL);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#hideapplication
	//void HideApplication();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#htmlhelp
	//virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#initinstance
	virtual BOOL InitInstance();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#istaskbarinteractionenabled
	//virtual BOOL IsTaskbarInteractionEnabled();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#loadcursor
	//HCURSOR LoadCursor(LPCTSTR lpszResourceName) const;
	//HCURSOR LoadCursor(UINT nIDResource) const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#loadicon
	//HICON LoadIcon(LPCTSTR lpszResourceName) const;
	//HICON LoadIcon(UINT nIDResource) const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#loadoemcursor
	//HCURSOR LoadOEMCursor(UINT nIDCursor) const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#loadoemicon
	//HICON LoadOEMIcon(UINT nIDIcon) const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#loadstandardcursor
	//HCURSOR LoadStandardCursor(LPCTSTR lpszCursorName) const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#loadstandardicon
	//HICON LoadStandardIcon(LPCTSTR lpszIconName) const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#onddecommand
	//virtual BOOL OnDDECommand(LPTSTR lpszCommand);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#onidle
	//virtual BOOL OnIdle(LONG lCount);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#opendocumentfile
	//virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName, BOOL bAddToMRU = TRUE);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#parsecommandline
	void ParseCommandLine(CCommandLineInfo& rCmdInfo);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#pretranslatemessage
	//virtual BOOL PreTranslateMessage(MSG* pMsg);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#processmessagefilter
	//virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#processshellcommand
	BOOL ProcessShellCommand(CCommandLineInfo& rCmdInfo);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#processwndprocexception
	//virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#register
	//virtual BOOL Register();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#registerwithrestartmanager
	// CWinApp::RegisterWithRestartManager
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#reopenpreviousfilesatrestart
	//virtual BOOL ReopenPreviousFilesAtRestart() const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#restartinstance
	//virtual BOOL CWinApp::RestartInstance();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#restoreautosavedfilesatrestart
	//virtual BOOL RestoreAutosavedFilesAtRestart() const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#run
	virtual int Run();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#runautomated
	//BOOL RunAutomated();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#runembedded
	//BOOL RunEmbedded();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#saveallmodified
	//virtual BOOL SaveAllModified();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#selectprinter
	//void SelectPrinter(HANDLE hDevNames, HANDLE hDevMode, BOOL bFreeOld = TRUE);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#sethelpmode
	//void SetHelpMode(AFX_HELP_TYPE eHelpType);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#supportsapplicationrecovery
	//virtual BOOL SupportsApplicationRecovery() const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#supportsautosaveatinterval
	//virtual BOOL SupportsAutosaveAtInterval() const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#supportsautosaveatrestart
	//virtual BOOL SupportsAutosaveAtRestart() const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#supportsrestartmanager
	//virtual BOOL SupportsRestartManager() const;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#unregister
	//virtual BOOL Unregister();
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#winhelp
	//virtual void WinHelp(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#writeprofilebinary
	//BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#writeprofileint
	//BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#writeprofilestring
	//BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
protected:

public:
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_bhelpmode
	//BOOL m_bHelpMode;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_ehelptype
	//AFX_HELP_TYPE m_eHelpType;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_hinstance
	//HINSTANCE m_hInstance;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_lpcmdline
	//LPTSTR m_lpCmdLine;
	//https://learn.microsoft.com/ru-ru/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_ncmdshow
	int m_nCmdShow;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_pactivewnd
	//?????
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_pszappid
	//LPCTSTR m_pszAppID;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_pszappname
	//LPCTSTR m_pszAppName;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_pszexename
	//LPCTSTR m_pszExeName;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_pszhelpfilepath
	//LPCTSTR m_pszHelpFilePath;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_pszprofilename
	//LPCTSTR m_pszProfileName;
	//https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class?view=msvc-140#m_pszregistrykey
	//LPCTSTR m_pszRegistryKey;

	CDocTemplate* m_pTemplate; // %% We should have an array: CArray...

	CWnd* m_pMainWnd;
protected:
	

protected:
	DECLARE_MESSAGE_MAP();
};

CWinApp* AfxGetApp();

//https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display?view=msvc-140#afxmessagebox
int AfxMessageBox(LPCSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0);
int AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT) -1);

#endif // CWINAPP_H
