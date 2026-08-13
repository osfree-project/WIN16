/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : TestCGdiObject.cpp
  Version    : 0.40
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
10-26-03  0.10  wdh  Created.
11-09-03  0.20  wdh  Added test cases for bitmaps.
11-25-03  0.30  wdh  Fixes to testing, added testing for mapping functions.
05-30-04  0.40  wdh  Changed OfcTest.h to StdAfx.h

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
  Copyright (c) 2003-04 William D. Herndon
/************************************************************************/
#include "StdAfx.h"
#include "resource.h"

void dbg_printf(LPCSTR pszForm,...);

    int  g_iTestPhase = 0;

BOOL NextGdiTest(HWND hwDlg,BOOL bReset = FALSE)
{
    if ( bReset )
        g_iTestPhase = 0;
    else
        g_iTestPhase++;
    switch (g_iTestPhase) {
    case 0:
        outPrintf("This test uses: CDC::Attach(),\r\n"
            "CBrush::CreateStockObject(), CDC::FillRect()\r\n"
            "CDC::Detach(), and implicit destructors calling \r\n"
            "DeleteDC() and DeleteObject()\r\n");
        SetDlgItemText(hwDlg, IDC_INSTRUCTIONS,
            "The result should be a plain white rectangle without border.\r\n"
            "Click [Success] if this is what you see,\r\n"
            "otherwise click [Fail].");
        break;
    case 1:
        outPrintf("This test uses: CBrush::CreateSolidBrush(),\r\n"
            "CPen::CreatePen(wide), CFont:CreateFontIndirect(),\r\n"
            "CDC::SelectObject(CBrush*), CDC::SelectObject(CPen*),\r\n"
            "CDC::SelectObject(CFont*), CDC::GetTextExtent(CString),\r\n"
            "CDC::SetTextColor(), CDC::SetBkMode(TRANSPARENT),\r\n"
            "CDC::Ellipse(), CDC::TextOut(int,int,CString),\r\n"
            "and implicit destructors calling DeleteDC() and \r\n"
            "DeleteObject()\r\n");
        SetDlgItemText(hwDlg, IDC_INSTRUCTIONS,
            "The result should be a red circle with a green border "
            "3 pixels wide and the text 'Test Text' in TimesNewRoman blue "
            "centered with transparent background.\r\n"
            "Click [Success] if this is what you see,\r\n"
            "otherwise click [Fail].");
        break;
    case 2:
        SetDlgItemText(hwDlg, IDC_INSTRUCTIONS,
            "The result should be an orange quarter circle pie slice (made "
            "using a Bezier), with a dashed brown 1 pixel wide border, "
            "with green background color (background opaque).\r\n"
            "Click [Success] if this is what you see,\r\n"
            "otherwise click [Fail].");
        outPrintf("This test uses: CBrush::CreateSolidBrush(),\r\n"
            "CPen::CreatePen(dash), CDC::SaveDC(), \r\n"
            "CDC::SelectObject(CBrush*), CDC::SelectObject(CPen*),\r\n"
            "CDC::SetBkMode(OPAQUE), CDC::SetBkColor(),\r\n"
            "CDC::BeginPath(), CDC::MoveTo(int,int), CDC::LineTo(int,int)\r\n"
            "CDC::PolyBezierTo(), CDC::LineTo(POINT), CDC:EndPath()\r\n"
            "CDC::StrokeAndFillPath(), CDC::RestoreDC(iSaveDC),\r\n"
            "and implicit destructors calling DeleteDC() and \r\n"
            "DeleteObject()\r\n");
        break;
    case 3:
        outPrintf("This test uses: CDC::CreateCompatibleDC(),\r\n"
            "CBitmap::CreateCompatibleBitmap(), CDC::SaveDC()\r\n"
            "CDC::SelectObject(CBitmap*), [Test 1 to bitmap DC],\r\n"
            "CDC::BitBlt(), CDC::RestoreDC(-1), and implicit \r\n"
            "destructors calling DeleteDC() and DeleteObject()\r\n");
        SetDlgItemText(hwDlg, IDC_INSTRUCTIONS,
            "The result should (again) be a red circle with a green border "
            "3 pixels wide and the text 'Test Text' in TimesNewRoman blue "
            "centered with transparent background.\r\n"
            "Click [Success] if this is what you see,\r\n"
            "otherwise click [Fail].");
        break;
    case 4:
        SetDlgItemText(hwDlg, IDC_INSTRUCTIONS,
            "Check the GDI resources. Are all resources freed?\r\n"
            "Click [Success] if they are,\r\n"
            "otherwise click [Fail].");
        outPrintf("Test: GDI resource usage.\r\n");
        break;
    default:
        return( FALSE );
    }
    InvalidateRect(GetDlgItem(hwDlg, IDC_OWNDRAW), NULL, TRUE);
    return( TRUE );
}

void TestEllipseAndText(CDC* pDC,LPCRECT prc)
{
    LOGFONT  lf;
    CBrush   brRed;
    CBrush*  pbrOld;
    CPen     penGreen;
    CPen*    ppenOld;
    CFont    fntBlue;
    CFont*   pfntOld;
    CString  strText = "Test Text";
    COLORREF crOldText;
    SIZE     sizText;
    POINT    ptText;
    int      iOldBkMode;

    // Create our GDI objects: brush, pen, font
    brRed.CreateSolidBrush(0x0000FF);
    penGreen.CreatePen(PS_SOLID, 3, 0x00FF00);
    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = 20;
    lf.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
    lf.lfCharSet = ANSI_CHARSET;
    strcpy(lf.lfFaceName, "TimesNewRoman");
    fntBlue.CreateFontIndirect(&lf);

    // Select our GDI objects and attributes
    pbrOld = pDC->SelectObject(&brRed);
    ppenOld = pDC->SelectObject(&penGreen);
    pfntOld = pDC->SelectObject(&fntBlue);
    crOldText = pDC->SetTextColor(0xFF0000);
    iOldBkMode = pDC->SetBkMode(TRANSPARENT);

    // Calculate the offsets to center the text
    sizText = pDC->GetTextExtent(strText);
    ptText.x = (prc->left + prc->right) / 2 - sizText.cx / 2;
    ptText.y = (prc->top + prc->bottom) / 2 - sizText.cy / 2;

    // Draw the ellipse and the text
    pDC->Ellipse(prc);
    pDC->TextOut(ptText.x, ptText.y, strText);

    // Restore all the attributes
    pDC->SetBkMode(iOldBkMode);
    pDC->SetTextColor(crOldText);
    pDC->SelectObject(pfntOld);
    pDC->SelectObject(ppenOld);
    pDC->SelectObject(pbrOld);

    // The default destructors take care of the rest
}

void TestPathFunctions(CDC* pDC,LPCRECT prc)
{
    CBrush brOrange;
    CPen   penBrown;
    POINT  pts[4];
    int    iSaveDC;
    // Create our GDI objects: brush, pen, font
    brOrange.CreateSolidBrush(0x0080FF);
    penBrown.CreatePen(PS_DASH, 1, 0x004080);

    // Save the DC, then select our GDI objects and attributes
    iSaveDC = pDC->SaveDC();
    pDC->SelectObject(&brOrange);
    pDC->SelectObject(&penBrown);
    pDC->SetBkMode(OPAQUE);
    pDC->SetBkColor(0x00FF00);

    pDC->BeginPath();
    pDC->MoveTo(5, 5);
    pDC->LineTo(prc->right-5, 5);
    pts[0].x = prc->right-5;
    pts[0].y = (prc->top+prc->bottom)/2;
    pts[1].x = (prc->left+prc->right)/2;
    pts[1].y = prc->bottom - 5;
    pts[2].x = 5;
    pts[2].y = pts[1].y;
    pDC->PolyBezierTo(pts, 3);
    pts[0].x = pts[0].y = 5;
    pDC->LineTo(pts[0]);
    pDC->EndPath();
    pDC->StrokeAndFillPath();

    // Restore the previous DC values
    pDC->RestoreDC(iSaveDC);

    // The default destructors take care of the rest
}

void TestBitmaps(CDC* pDC,LPCRECT prc)
{
    CDC     dcBmp;
    CBitmap bmp;
    CRect   rcBmp(0, 0, prc->right-prc->left, prc->bottom-prc->top);

    // Create a compatible device context with selected bitmap
    dcBmp.CreateCompatibleDC(pDC);
    dcBmp.SaveDC();
    bmp.CreateCompatibleBitmap(pDC, rcBmp.right, rcBmp.bottom);
    dcBmp.SelectObject(&bmp);

    // Blank out the bitmap
    CBrush brWhite;
    brWhite.CreateStockObject(WHITE_BRUSH);
    dcBmp.FillRect(&rcBmp, &brWhite);

    // Draw in the bitmap context
    TestEllipseAndText(&dcBmp, &rcBmp);

    // Not bit blt it to the destination
    pDC->BitBlt(prc->left, prc->top, rcBmp.right, rcBmp.bottom, 
        &dcBmp, 0, 0, SRCCOPY);

    // Restore the previous status of the bitmap DC
    dcBmp.RestoreDC(-1);

    // The default destructors take care of the rest
}

BOOL CALLBACK TestGdiDlgProc(
    HWND   hwDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
) {
static BOOL bAllResult, bMsCompat;
    switch (message) {
    case WM_INITDIALOG:
        bAllResult = TRUE;
        bMsCompat = TRUE;
        NextGdiTest(hwDlg, TRUE);
        g_iTestPhase = 0;
        SetFocus(GetDlgItem(hwDlg, IDOK));
        break;
    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
        case IDNO: {
            BOOL bResult;
            bResult = (wParam == IDOK);
            bAllResult &= bResult;
            bMsCompat  &= bResult;
            outPrintf("Test result: %s\r\n", ResultStr(bResult));
            if ( !NextGdiTest(hwDlg) ) {

                outPrintf("\r\n"
                    "Summary\r\n"
                    "-------\r\n"
                    "MS Compatibility   = %s\r\n"
                    "Full Functionality = %s\r\n",
                    ResultStr(bMsCompat),
                    ResultStr(bAllResult));

                EndDialog(hwDlg, TRUE);
            }
        }   break;
        case IDCANCEL:
            EndDialog(hwDlg, FALSE);
            break;
        }
        break;
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
        RECT rcItem = pDIS->rcItem;
        CDC  dc;
        CBrush brWhite;
        dc.Attach(pDIS->hDC);
        // Always fill the background with white -
        // the first test is only this.
        brWhite.CreateStockObject(WHITE_BRUSH);
        dc.FillRect(&rcItem, &brWhite);
        switch (g_iTestPhase) {
        case 0: // Test only white background
            break;
        case 1:
            TestEllipseAndText(&dc, &rcItem);
            break;
        case 2:
            TestPathFunctions(&dc, &rcItem);
            break;
	case 3:
	    TestBitmaps(&dc, &rcItem);
	    break;
        }
        dc.Detach();
    }   break;
    }
    return( 0 );
}

// Test the CGdiObject classes and CDC for proper functionality
void TestCGdiObject()
{
    MessageBox(g_hwMain, "Start tracking GDI resources now:\r"
        "under Win95/98/ME use a 'memory' program\r"
        "under WinNT/2000/XP open the Process Manager and add appropriate "
        "GDI resource columns to the view.\r"
        "This test may not be valid under Win95/98/ME if other programs "
        "that are not careful with resources are running.",
        "OFC Test", MB_OK);
    // To test GDI objects we need a CDC, which needs a window
    // or printer for output. In this case we will use a user-draw
    // button in a dialog.
    if ( DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_GDITEST),
        g_hwMain, TestGdiDlgProc) )
    {
        outPrintf("Testing complete.\r\n");
    } else {
        outPrintf("Testing cancelled by user.\r\n");
    }
}
