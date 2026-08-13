/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : TestCCmdTarget.cpp
  Version    : 0.20
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
11-25-03  0.10  wdh  Created.
05-30-04  0.20  wdh  Added testing for *WaitCursor().

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
  Copyright (c) 2003 The Open Foundation Classes
  Copyright (c) 2003 William D. Herndon
/************************************************************************/
#include "StdAfx.h"
#include "resource.h"

void dbg_printf(LPCSTR pszForm,...);

#define IDM_CMD_TEST1    101
#define IDC_BTN_TEST2    102

class CTestCmdTarget : public CCmdTarget {
    DECLARE_DYNAMIC(CTestCmdTarget)

	//{{AFX_MSG(CTestCmdTarget)
	afx_msg void OnCmdTest1();
	afx_msg void OnBtnTest2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP();
};

IMPLEMENT_DYNAMIC(CTestCmdTarget, CCmdTarget)

#define TARG_CLASS CTestCmdTarget
BEGIN_MESSAGE_MAP(CTestCmdTarget, CCmdTarget)
	//{{AFX_MSG_MAP(CTestCmdTarget)
	ON_COMMAND(IDM_CMD_TEST1, OnCmdTest1)
	ON_BN_CLICKED(IDC_BTN_TEST2, OnBtnTest2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#undef TARG_CLASS

	int  g_iTestNo = 0;

void CTestCmdTarget::OnCmdTest1()
{
	g_iTestNo = 1;
}

void CTestCmdTarget::OnBtnTest2()
{
	g_iTestNo = 2;
}

// Test the CCmdTarget class
void TestCCmdTarget()
{
    CTestCmdTarget* pTestTarg;
    BOOL bResult, bAllResult = TRUE, bMsCompat = TRUE;
    BOOL bRet;

    pTestTarg = new CTestCmdTarget;

    outPrintf("\r\n"
        "Testing CCmdTarget functionality\r\n"
        "--------------------------------\r\n");

    // Try out OnCmdMsg() in different orders
    outPrintf("\r\nTest OnCmdMsg()\r\n");

    bRet = pTestTarg->OnCmdMsg(IDM_CMD_TEST1, BN_CLICKED, NULL, NULL);
    bResult = bRet && (g_iTestNo == 1);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("OnCmdMsg(...IDM_CMD_TEST1...)=>%d last function: %d => %s\r\n",
        bRet, g_iTestNo, ResultStr(bResult));

    bRet = pTestTarg->OnCmdMsg(IDC_BTN_TEST2, BN_CLICKED, NULL, NULL);
    bResult = bRet && (g_iTestNo == 2);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("OnCmdMsg(...IDM_BTN_TEST2...)=>%d last function: %d => %s\r\n",
        bRet, g_iTestNo, ResultStr(bResult));

    bRet = pTestTarg->OnCmdMsg(IDC_BTN_TEST2, BN_CLICKED, NULL, NULL);
    bResult = bRet && (g_iTestNo == 2);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("OnCmdMsg(...IDM_BTN_TEST2...)=>%d last function: %d => %s\r\n",
        bRet, g_iTestNo, ResultStr(bResult));

    bRet = pTestTarg->OnCmdMsg(IDM_CMD_TEST1, BN_CLICKED, NULL, NULL);
    bResult = bRet && (g_iTestNo == 1);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("OnCmdMsg(...IDM_CMD_TEST1...)=>%d last function: %d => %s\r\n",
        bRet, g_iTestNo, ResultStr(bResult));

#ifndef MSVC32 // These ASSERT() when AfxGetApp() fails
    pTestTarg->BeginWaitCursor();
    ::Sleep(1000);
    bResult = (::MessageBox(g_hwMain, "Was the cursor a wait cursor?",
        g_szCaption, MB_ICONQUESTION | MB_YESNO) == IDYES);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("BeginWaitCursor()=>%s\r\n", ResultStr(bResult));

    // Set the cursor to something else, then restore the wait cursor
    ::SetCursor(::LoadCursor(NULL, IDC_CROSS));
    pTestTarg->RestoreWaitCursor();
    ::Sleep(1000);
    bResult = (::MessageBox(g_hwMain, "Was the cursor a wait cursor again?",
        g_szCaption, MB_ICONQUESTION | MB_YESNO) == IDYES);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("RestoreWaitCursor() [while on]=>%s\r\n", ResultStr(bResult));

    //  Make sure we remember the correct depth
    pTestTarg->BeginWaitCursor();
    pTestTarg->EndWaitCursor();
    ::Sleep(1000);
    bResult = (::MessageBox(g_hwMain, "Was the cursor a wait cursor again?",
        g_szCaption, MB_ICONQUESTION | MB_YESNO) == IDYES);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("2x Begin+1x EndWaitCursor()=>%s\r\n", ResultStr(bResult));

    pTestTarg->EndWaitCursor();
    ::Sleep(1000);
    bResult = (::MessageBox(g_hwMain,
        "Was the cursor restored to a normal cursor?",
        g_szCaption, MB_ICONQUESTION | MB_YESNO) == IDYES);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("2x Begin+2x EndWaitCursor()=>%s\r\n", ResultStr(bResult));
#endif

    outPrintf("\r\n"
        "Summary\r\n"
        "-------\r\n"
        "MS Compatibility   = %s\r\n"
        "Full Functionality = %s\r\n",
        ResultStr(bMsCompat),
        ResultStr(bAllResult));
}
