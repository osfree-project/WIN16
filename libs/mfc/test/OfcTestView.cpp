/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : OfcTestView.cpp
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
05-30-04  0.10  wdh  Created using VC 6.0 and adapted for OFC use.

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
  Copyright (c) 2000-04 The Open Foundation Classes
  Copyright (c) 2004 William D. Herndon
/************************************************************************/

#include "stdafx.h"
#include "OfcTest.h"

#include "OfcTestDoc.h"
#include "OfcTestView.h"

// Stuff originally from OfcTest.cpp -wdh
#define ID_EDIT_RESULT  50

    char      g_szCaption[] = "OFC Test";
    HINSTANCE g_hInstance = NULL; // Our instance
    HWND      g_hwMain = NULL;    // Our main window

#define MAX_EDIT      65535
#define N_EDIT_BUFFER 16384

// outPrintf - Functions like printf, but output is to the
// edit control in the main window.
void outPrintf(LPCSTR pszForm,...)
{
    HWND   hwEdit = GetDlgItem(g_hwMain, ID_EDIT_RESULT);
    LPSTR  pszEdit, pszEnd;
    va_list arglist;

    if (hwEdit == NULL) {
        ::MessageBox(g_hwMain, "Output Edit Control not found!",
            g_szCaption, MB_OK|MB_ICONEXCLAMATION);
    }
    // Allocate a buffer and get the current contents of the edit control
    pszEdit = (LPSTR)malloc(MAX_EDIT);
    ::GetWindowText(hwEdit, pszEdit, MAX_EDIT);

    // If the contents are getting too long, shorten them
    if (strlen(pszEdit) > MAX_EDIT-N_EDIT_BUFFER) {
        strcpy(pszEdit, pszEdit+N_EDIT_BUFFER);
    }

    // Output the message to the end of the string and
    // put the text back in the edit control
    pszEnd = pszEdit + strlen(pszEdit);
    va_start(arglist, pszForm);
    ::wvsprintf(pszEnd, pszForm, arglist);
    va_end(arglist);
    ::SetWindowText(hwEdit, pszEdit);

    // Set the cursor to the end and scroll it into view
    // Note that despite MS documentation, using -1 to indicate
    // the end of the string does not work.
    int iLen = strlen(pszEdit);
    ::SendMessage(hwEdit, EM_SETSEL, (WPARAM)iLen, (LPARAM)iLen);
    ::SendMessage(hwEdit, EM_SCROLLCARET, 0, 0);
    free(pszEdit);
}

LPCSTR ResultStr(BOOL bResult)
{
    if ( bResult )
        return( "OK" );
    else
        return( "Fail!" );
}

// End of stuff originally from OfcTest.cpp

/////////////////////////////////////////////////////////////////////////////
// COfcTestView

IMPLEMENT_DYNCREATE(COfcTestView, CView)

BEGIN_MESSAGE_MAP(COfcTestView, CView)
	//{{AFX_MSG_MAP(COfcTestView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COfcTestView construction/destruction

COfcTestView::COfcTestView()
{
	// TODO: add construction code here

}

COfcTestView::~COfcTestView()
{
}

BOOL COfcTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// COfcTestView drawing

void COfcTestView::OnDraw(CDC* pDC)
{
	COfcTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// COfcTestView diagnostics

#ifdef _DEBUG
void COfcTestView::AssertValid() const
{
	CView::AssertValid();
}

void COfcTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

COfcTestDoc* COfcTestView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COfcTestDoc)));
	return (COfcTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COfcTestView message handlers

LRESULT COfcTestView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	HWND hwnd = m_hWnd;

	// Switch from the main window of the original OfcTest
	switch (message) {
	case WM_CREATE: {
		HWND hwEdit;
		g_hwMain = hwnd;
		hwEdit = ::CreateWindow("EDIT", "Result Window\r\n",
			WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_LEFT|ES_MULTILINE,
			0, 0, 100, 100, hwnd, (HMENU)ID_EDIT_RESULT,
			g_hInstance, NULL);
		::SendMessage(hwEdit, WM_SETFONT,
			(WPARAM)::GetStockObject(ANSI_FIXED_FONT), 0);
	}    break;
		// MFC deactivates the menus if we do not do them
		// over the standard message-maps - so they are
		// handled in MainFrame (the main window), while
		// we only handle the edit window for results here.
	case WM_SIZE: {
		RECT rcClient;
		HWND hwEdit = ::GetDlgItem(hwnd, ID_EDIT_RESULT);
		if (hwEdit == NULL)
			break;
		::GetClientRect(hwnd, &rcClient);
		::SetWindowPos(hwEdit, NULL, rcClient.left, rcClient.top,
			rcClient.right-rcClient.left,
			rcClient.bottom-rcClient.top, 0);
	}   break;
	}
	// End of switch from the original OfcTest
	
	return CView::WindowProc(message, wParam, lParam);
}
