/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : TestCRect.cpp
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
10-26-03  0.10  wdh  Split off from OfcTest.cpp
05-30-04  0.20  wdh  Changed OfcTest.h to StdAfx.h

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

// Test the CRect class for proper functionality
void TestCRect()
{
    POINT pt = { 1, 2 };
    SIZE  siz = { 3, 4 };
    RECT  rc = { 1, 2, 3, 4 };
    CRect rc1; // Default initialization
    CRect rc2(1, 2, 3, 4); // Initialize with int params
    CRect rc3(rc);   // Initialize with RECT
    CRect rc4(&rc3);  // Initialize with CRect*
    CRect rc5(pt, siz); // Initialize with POINT, SIZE
    CRect rc6(pt, pt); // Initialize with POINT, POINT
// This did not compile before the "=" operator for POINT was made explicit.
// It seems to be a compiler incompatibility -wdh
    CRect rc7 = rc;

    CRect rc8;
    CRect rc9;
// The following line crashed before the "=" operator was made explicit.
// It should not be necessary, since the class is based on POINT,
// but there seems to be some sort of GCC incompatibility. -wdh
    rc8 = rc;

    BOOL bResult, bAllResult = TRUE, bMsCompat = TRUE;
    outPrintf("\r\n"
        "Testing CRect functionality\r\n"
        "---------------------------\r\n");

    // Show all the different kinds of initialization
    outPrintf("\r\nInitialization / Assignment\r\n");

    bResult = ((rc1.left == 0) && (rc1.top == 0) &&
        (rc1.right == 0) && (rc1.bottom == 0));
    bAllResult &= bResult;
    outPrintf("CRect rc1;                    rc1 => (%d, %d, %d, %d) = %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));
    // MS does not initialize to (0, 0)
    if ( !bResult ) {
        rc1.left = rc1.top = rc1.right = rc1.bottom = 0;
        outPrintf("Setting rc1 to (0, 0, 0, 0)\r\n");
    }

    bResult = ((rc2.left == 1) && (rc2.top == 2) &&
        (rc2.right == 3) && (rc2.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CRect rc2(1, 2, 3, 4);        rc2 => (%d, %d, %d, %d) = %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    bResult = ((rc3.left == 1) && (rc3.top == 2) &&
        (rc3.right == 3) && (rc3.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CRect rc3(rc);                rc3 => (%d, %d, %d, %d) = %s\r\n",
	rc3.left, rc3.top, rc3.right, rc3.bottom, ResultStr(bResult));

    bResult = ((rc4.left == 1) && (rc4.top == 2) &&
        (rc4.right == 3) && (rc4.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CRect rc4(&rc3);              rc4 => (%d, %d, %d, %d) = %s\r\n",
	rc4.left, rc4.top, rc4.right, rc4.bottom, ResultStr(bResult));

    bResult = ((rc5.left == 1) && (rc5.top == 2) &&
        (rc5.right == 4) && (rc5.bottom == 6));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CRect rc5(pt, siz);           rc5 => (%d, %d, %d, %d) = %s\r\n",
	rc5.left, rc5.top, rc5.right, rc5.bottom, ResultStr(bResult));

    bResult = ((rc6.left == 1) && (rc6.top == 2) &&
        (rc6.right == 1) && (rc6.bottom == 2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CRect rc6(pt, pt);            rc6 => (%d, %d, %d, %d) = %s\r\n",
	rc6.left, rc6.top, rc6.right, rc6.bottom, ResultStr(bResult));

    bResult = ((rc7.left == 1) && (rc7.top == 2) &&
        (rc7.right == 3) && (rc7.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CRect rc7 = rc;               rc7 => (%d, %d, %d, %d) = %s\r\n",
	rc7.left, rc7.top, rc7.right, rc7.bottom, ResultStr(bResult));

    bResult = ((rc8.left == 1) && (rc8.top == 2) &&
        (rc8.right == 3) && (rc8.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CRect rc8; rc8 = rc;          rc8 => (%d, %d, %d, %d) = %s\r\n",
	rc8.left, rc8.top, rc8.right, rc8.bottom, ResultStr(bResult));

    outPrintf("\r\nAttribute methods\r\n");
    int iWidth = rc5.Width(),
        iHeight = rc5.Height();
    bResult = (iWidth == 3) && (iHeight == 4);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc5.Width() = %d, rc5.Height() = %d => %s\r\n",
	iWidth, iHeight, ResultStr(bResult));

    siz = rc5.Size();
    bResult = (siz.cx == 3) && (siz.cy == 4);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc5.Size() = (%d, %d)               => %s\r\n",
	siz.cx, siz.cy, ResultStr(bResult));

    pt = rc5.TopLeft();
    bResult = (pt.x == 1) && (pt.y == 2);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc5.TopLeft() = (%d, %d)            => %s\r\n",
	pt.x, pt.y, ResultStr(bResult));

    pt = rc5.BottomRight();
    bResult = (pt.x == 4) && (pt.y == 6);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc5.BottomRight() = (%d, %d)        => %s\r\n",
	pt.x, pt.y, ResultStr(bResult));

    pt = rc5.CenterPoint();
    bResult = (pt.x == 2) && (pt.y == 4);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc5.CenterPoint() = (%d, %d)        => %s\r\n",
	pt.x, pt.y, ResultStr(bResult));
    pt.x = 1; pt.y = 2; // Put value back the way it was

    BOOL bTest1, bTest2;
    bTest1 = rc1.IsRectEmpty();
    bTest2 = rc1.IsRectNull();
    bResult = (bTest1 == TRUE) && (bTest2 == TRUE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.IsRectEmpty() = %d, rc1.IsRectNull() = %d   => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

    bTest1 = rc2.IsRectEmpty();
    bTest2 = rc2.IsRectNull();
    bResult = (bTest1 == FALSE) && (bTest2 == FALSE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2.IsRectEmpty() = %d, rc2.IsRectNull() = %d   => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

    bTest1 = rc6.IsRectEmpty();
    bTest2 = rc6.IsRectNull();
    bResult = (bTest1 == TRUE) && (bTest2 == FALSE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc6.IsRectEmpty() = %d, rc6.IsRectNull() = %d   => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

    rc6.SetRect(3, 4, 1, 2);
    bTest1 = rc6.IsRectEmpty();
    bTest2 = rc6.IsRectNull();
    bResult = (bTest1 == TRUE) && (bTest2 == FALSE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc6.SetRect(3, 4, 1, 2);\r\n"
        "rc6.IsRectEmpty() = %d, rc6.IsRectNull() = %d   => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

    bTest1 = rc1.PtInRect(pt);
    bTest2 = rc2.PtInRect(pt);
    bResult = ((bTest1 == FALSE) && (bTest2 == TRUE));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.PtInRect(pt)  = %d, rc2.ptInRect(pt)  = %d  => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

#ifndef MSVC32 // OFC extension
    bTest1 = rc1.RectInRect(rc1);
    bTest2 = rc1.RectInRect(&rc);
    bResult = ((bTest1 == TRUE) && (bTest2 == FALSE));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.RectInRect(rc1) = %d, rc2.RectInRect(&rc) = %d  => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

    bTest1 = rc2.RectInRect(rc1);
    bTest2 = rc2.RectInRect(&rc);
    bResult = ((bTest1 == FALSE) && (bTest2 == TRUE));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2.RectInRect(rc1) = %d, rc2.RectInRect(&rc) = %d  => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));
#else
    bAllResult = FALSE;
    outPrintf("CRect::RectInRect(CRect&) is an OFC extension.\r\n");
    outPrintf("CRect::RectInRect(LPRECT) is an OFC extension.\r\n");
#endif
    rc2.SetRectEmpty();
    bResult = ((rc2.left == 0) && (rc2.top == 0) &&
        (rc2.right == 0) && (rc2.bottom == 0));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2.SetRectEmpty();      rc2 => (%d, %d, %d, %d) = %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc1.SetRect(5, 6, 7, 8);
    bResult = ((rc1.left == 5) && (rc1.top == 6) &&
        (rc1.right == 7) && (rc1.bottom == 8));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.SetRect(5, 6, 7, 8); rc1 => (%d, %d, %d, %d) = %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

#ifndef MSVC32 // OFC extension
    rc2.SetRect(&rc);
    bResult = ((rc2.left == 1) && (rc2.top == 2) &&
        (rc2.right == 3) && (rc2.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2.SetRect(&rc);        rc2 => (%d, %d, %d, %d) = %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));
#else
    bAllResult = FALSE;
    outPrintf("CRect::SetRect(LPCRECT) is an OFC extension.\r\n");
#endif

    rc1.CopyRect(&rc);
    bResult = ((rc1.left == 1) && (rc1.top == 2) &&
        (rc1.right == 3) && (rc1.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.CopyRect(&rc);       rc1 => (%d, %d, %d, %d) = %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    bTest1 = rc1.EqualRect(&rc);
    bTest2 = rc5.EqualRect(&rc);
    bResult = ((bTest1 == TRUE) && (bTest2 == FALSE));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.EqualRect(&rc) = %d rc5.EqualRect(&rc) = %d => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

    rc1.InflateRect(2, 1);
    bResult = ((rc1.left == -1) && (rc1.top == 1) &&
        (rc1.right == 5) && (rc1.bottom == 5));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.InflateRect(2, 1);    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.InflateRect(siz);
    bResult = ((rc1.left == -4) && (rc1.top == -3) &&
        (rc1.right == 8) && (rc1.bottom == 9));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.InflateRect(siz);    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.InflateRect(&rc);
    bResult = ((rc1.left == -5) && (rc1.top == -5) &&
        (rc1.right == 11) && (rc1.bottom == 13));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.InflateRect(&rc);    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.InflateRect(1, 2, 3, 4);
    bResult = ((rc1.left == -6) && (rc1.top == -7) &&
        (rc1.right == 14) && (rc1.bottom == 17));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.InflateRect(1, 2, 3, 4); rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.DeflateRect(2, 1);
    bResult = ((rc1.left == -4) && (rc1.top == -6) &&
        (rc1.right == 12) && (rc1.bottom == 16));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.DeflateRect(2, 1);    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.DeflateRect(siz);
    bResult = ((rc1.left == -1) && (rc1.top == -2) &&
        (rc1.right == 9) && (rc1.bottom == 12));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.DeflateRect(siz);    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.DeflateRect(&rc);
    bResult = ((rc1.left == 0) && (rc1.top == 0) &&
        (rc1.right == 6) && (rc1.bottom == 8));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.DeflateRect(&rc);    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.DeflateRect(1, 2, 3, 4);
    bResult = ((rc1.left == 1) && (rc1.top == 2) &&
        (rc1.right == 3) && (rc1.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.DeflateRect(&rc);    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.NormalizeRect();
    bResult = ((rc1.left == 1) && (rc1.top == 2) &&
        (rc1.right == 3) && (rc1.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.NormalizeRect();    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc1.SetRect(4, 3, 2, 1);
    rc1.NormalizeRect();
    bResult = ((rc1.left == 2) && (rc1.top == 1) &&
        (rc1.right == 4) && (rc1.bottom == 3));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1.SetRect(4, 3, 2, 1);\r\n"
        "rc1.NormalizeRect();    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc = *(LPCRECT)rc1;
    bResult = ((rc.left == 2) && (rc.top == 1) &&
        (rc.right == 4) && (rc.bottom == 3));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc = *(LPCRECT)rc1;    rc => (%d, %d, %d, %d) => %s\r\n",
	rc.left, rc.top, rc.right, rc.bottom, ResultStr(bResult));

    *((LPRECT)rc2) = rc;
    bResult = ((rc2.left == 2) && (rc2.top == 1) &&
        (rc2.right == 4) && (rc2.bottom == 3));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("*((LPRECT)rc2) = rc;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    bTest1 = (rc2 == rc);
    bTest2 = (rc5 == rc);
    bResult = ((bTest1 == TRUE) && (bTest2 == FALSE));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(rc2 == rc) = %d (rc5 == rc) = %d => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

    bTest1 = (rc2 != rc);
    bTest2 = (rc5 != rc);
    bResult = ((bTest1 == FALSE) && (bTest2 == TRUE));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(rc2 != rc) = %d (rc5 != rc) = %d => %s\r\n",
	bTest1, bTest2, ResultStr(bResult));

    rc2 += pt;
    bResult = ((rc2.left == 3) && (rc2.top == 3) &&
        (rc2.right == 5) && (rc2.bottom == 5));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 += pt;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc1 += siz;
    bResult = ((rc1.left == 5) && (rc1.top == 5) &&
        (rc1.right == 7) && (rc1.bottom == 7));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1 += siz;    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc3 += &rc;
    bResult = ((rc3.left == -1) && (rc3.top == 1) &&
        (rc3.right == 7) && (rc3.bottom == 7));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc3 += rc;     rc3 => (%d, %d, %d, %d) => %s\r\n",
	rc3.left, rc3.top, rc3.right, rc3.bottom, ResultStr(bResult));

    rc2 -= pt;
    bResult = ((rc2.left == 2) && (rc2.top == 1) &&
        (rc2.right == 4) && (rc2.bottom == 3));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 -= pt;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc1 -= siz;
    bResult = ((rc1.left == 2) && (rc1.top == 1) &&
        (rc1.right == 4) && (rc1.bottom == 3));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc1 -= siz;    rc1 => (%d, %d, %d, %d) => %s\r\n",
	rc1.left, rc1.top, rc1.right, rc1.bottom, ResultStr(bResult));

    rc3 -= &rc;
    bResult = ((rc3.left == 1) && (rc3.top == 2) &&
        (rc3.right == 3) && (rc3.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc3 -= rc;     rc3 => (%d, %d, %d, %d) => %s\r\n",
	rc3.left, rc3.top, rc3.right, rc3.bottom, ResultStr(bResult));

    rc3 &= rc;
    bResult = ((rc3.left == 2) && (rc3.top == 2) &&
        (rc3.right == 3) && (rc3.bottom == 3));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc3 &= rc;     rc3 => (%d, %d, %d, %d) => %s\r\n",
	rc3.left, rc3.top, rc3.right, rc3.bottom, ResultStr(bResult));

    rc4 |= rc;
    bResult = ((rc4.left == 1) && (rc4.top == 1) &&
        (rc4.right == 4) && (rc4.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc4 |= rc;     rc4 => (%d, %d, %d, %d) => %s\r\n",
	rc4.left, rc4.top, rc4.right, rc4.bottom, ResultStr(bResult));

    rc2 = rc1 + pt;
    bResult = ((rc2.left == 3) && (rc2.top == 3) &&
        (rc2.right == 5) && (rc2.bottom == 5));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 = rc1 + pt;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc2 = rc1 + siz;
    bResult = ((rc2.left == 5) && (rc2.top == 5) &&
        (rc2.right == 7) && (rc2.bottom == 7));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 = rc1 + siz;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc2 = rc1 + &rc;
    bResult = ((rc2.left == 0) && (rc2.top == 0) &&
        (rc2.right == 8) && (rc2.bottom == 6));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 = rc1 + &rc;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc2 = rc1 - pt;
    bResult = ((rc2.left == 1) && (rc2.top == -1) &&
        (rc2.right == 3) && (rc2.bottom == 1));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 = rc1 - pt;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc2 = rc1 - siz;
    bResult = ((rc2.left == -1) && (rc2.top == -3) &&
        (rc2.right == 1) && (rc2.bottom == -1));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 = rc1 - siz;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc2 = rc1 - &rc;
    bResult = ((rc2.left == 4) && (rc2.top == 2) &&
        (rc2.right == 0) && (rc2.bottom == 0));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 = rc1 - &rc;    rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc5.SetRect(1, 2, 3, 4);
    rc2 = rc5 & rc;
    bResult = ((rc2.left == 2) && (rc2.top == 2) &&
        (rc2.right == 3) && (rc2.bottom == 3));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc5.SetRect(1, 2, 3, 4)\r\n"
        "rc2 = rc5 & rc;     rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    rc2 = rc5 | rc;
    bResult = ((rc2.left == 1) && (rc2.top == 1) &&
        (rc2.right == 4) && (rc2.bottom == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("rc2 = rc5 | rc;     rc2 => (%d, %d, %d, %d) => %s\r\n",
	rc2.left, rc2.top, rc2.right, rc2.bottom, ResultStr(bResult));

    outPrintf("\r\n"
        "Summary\r\n"
        "-------\r\n"
        "MS Compatibility   = %s\r\n"
        "Full Functionality = %s\r\n",
        ResultStr(bMsCompat),
        ResultStr(bAllResult));
}
