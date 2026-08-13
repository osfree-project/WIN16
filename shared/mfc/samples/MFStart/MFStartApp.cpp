#include <afxwin.h>
#include "MFStartApp.h"
#include "MFStartWindow.h"

CMFStartApp MFStartApp;

BOOL CMFStartApp::InitInstance()
{
	m_pMainWnd = new CMFStartWindow();
	m_pMainWnd -> ShowWindow(m_nCmdShow);
	m_pMainWnd -> UpdateWindow();
	return TRUE;
}
