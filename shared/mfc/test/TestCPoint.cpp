/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : TestCPoint.cpp
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

// Test the CPoint class for proper functionality
void TestCPoint()
{
    POINT  pt = { 1, 2 };
    SIZE   siz = { 1, 2 };
    CPoint pt1; // Default initialization
    CPoint pt2(1, 2); // Initialize with int params
    CPoint pt3(pt);   // Initialize with POINT
    CPoint pt4(siz);  // Initialize with SIZE
    CPoint pt5(0x00020001L); // Initialize with DWORD
// This did not compile before the "=" operator for POINT was made explicit.
// It seems to be a compiler incompatibility -wdh
    CPoint pt6 = pt;

    CPoint pt7;
    CPoint pt8;
// The following two lines crashed before the "=" operator was made explicit.
// It should not be necessary, since the class is based on POINT,
// but there seems to be some sort of GCC incompatibility. -wdh
    pt7 = pt;
#ifndef MSVC32 // This does not compile under MS Visual C, though it should
    pt8 = (POINT){ 1, 2 };
#else
    pt8 = pt;
#endif

    BOOL bResult, bAllResult = TRUE, bMsCompat = TRUE;
    outPrintf("\r\n"
        "Testing CPoint functionality\r\n"
        "----------------------------\r\n");

    outPrintf("Initialization / Assignment\r\n");
    bResult = ((pt1.x==0) && (pt1.y==0));
    bAllResult &= bResult;
    outPrintf("CPoint pt1;                        pt1 => (%d, %d) = %s\r\n",
	pt1.x, pt1.y, ResultStr(bResult));
    pt1.x = pt1.y = 0; // MS does not initialize to (0, 0)

    bResult = ((pt2.x==1) && (pt2.y==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CPoint pt2(1, 2);                  pt2 => (%d, %d) = %s\r\n",
	pt2.x, pt2.y, ResultStr(bResult));

    bResult = ((pt3.x==1) && (pt3.y==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CPoint pt3(pt);                    pt3 => (%d, %d) = %s\r\n",
	pt3.x, pt3.y, ResultStr(bResult));

    bResult = ((pt4.x==1) && (pt4.y==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CPoint pt4(siz);                   pt4 => (%d, %d) = %s\r\n",
	pt4.x, pt4.y, ResultStr(bResult));

    bResult = ((pt5.x==1) && (pt5.y==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CPoint pt5(0x00020001L);           pt5 => (%d, %d) = %s\r\n",
	pt5.x, pt5.y, ResultStr(bResult));

    bResult = ((pt6.x==1) && (pt6.y==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CPoint pt6 = pt;                   pt6 => (%d, %d) = %s\r\n",
	pt6.x, pt6.y, ResultStr(bResult));

    bResult = ((pt7.x==1) && (pt7.y==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CPoint pt7; pt7 = pt;              pt7 => (%d, %d) = %s\r\n",
	pt7.x, pt7.y, ResultStr(bResult));

    bResult = ((pt8.x==1) && (pt8.y==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CPoint pt8; pt8 = (POINT){ 1, 2 }; pt8 => (%d, %d) = %s\r\n",
	pt8.x, pt8.y, ResultStr(bResult));

    // Show the Boolean Operators
    BOOL bCompare;
    outPrintf("\r\nBoolean Operators\r\n");

    bCompare = (pt2 == pt1);
    bResult = (bCompare == FALSE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(pt2 == pt1) => %d = %s\r\n",
        bCompare, ResultStr(bResult));

    bCompare = (pt2 == pt3);
    bResult = (bCompare == TRUE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(pt2 == pt3) => %d = %s\r\n",
        bCompare, ResultStr(bResult));

    bCompare = (pt2 != pt1);
    bResult = (bCompare == TRUE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(pt2 != pt1) => %d = %s\r\n",
        bCompare, ResultStr(bResult));

    bCompare = (pt2 != pt3);
    bResult = (bCompare == FALSE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(pt2 != pt3) => %d = %s\r\n",
        bCompare, ResultStr(bResult));

    // Show the Offset() methods
    outPrintf("\r\nOffset() methods\r\n");
    pt2.Offset(1, 2);
    bResult = ((pt2.x == 2) && (pt2.y == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt2.Offset(1, 2) => (%d, %d) = %s\r\n",
        pt2.x, pt2.y, ResultStr(bResult));

    pt3.Offset(pt);
    bResult = ((pt3.x == 2) && (pt3.y == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt3.Offset(pt)   => (%d, %d) = %s\r\n",
        pt3.x, pt3.y, ResultStr(bResult));

    pt4.Offset(siz);
    bResult = ((pt4.x == 2) && (pt4.y == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt4.Offset(siz)  => (%d, %d) = %s\r\n",
        pt4.x, pt4.y, ResultStr(bResult));

    // Show += and -=
    outPrintf("\r\n+= and -= operators\r\n");
    pt1 += pt;
    bResult = ((pt1.x == 1) && (pt1.y == 2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt1 += pt;  => (%d, %d) = %s\r\n",
        pt1.x, pt1.y, ResultStr(bResult));

    pt2 -= pt;
    bResult = ((pt2.x == 1) && (pt2.y == 2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt2 -= pt;  => (%d, %d) = %s\r\n",
        pt2.x, pt2.y, ResultStr(bResult));

    pt3 += siz;
    bResult = ((pt3.x == 3) && (pt3.y == 6));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt3 += siz; => (%d, %d) = %s\r\n",
        pt3.x, pt3.y, ResultStr(bResult));

    pt4 -= siz;
    bResult = ((pt4.x == 1) && (pt4.y == 2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt4 -= siz; => (%d, %d) = %s\r\n",
        pt4.x, pt4.y, ResultStr(bResult));
    
    // Show + and -
    outPrintf("\r\n+ and - operators\r\n");
    pt1 = pt6 + pt;
    bResult = ((pt1.x == 2) && (pt1.y == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt1 = pt6 + pt;   => (%d, %d) = %s\r\n",
        pt1.x, pt1.y, ResultStr(bResult));


    pt2 = pt6 + siz;
    bResult = ((pt2.x == 2) && (pt2.y == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt2 = pt6 + siz;  => (%d, %d) = %s\r\n",
        pt2.x, pt2.y, ResultStr(bResult));

    pt3 = pt6 - pt;
    bResult = ((pt3.x == 0) && (pt3.y == 0));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt3 = pt6 - pt;   => (%d, %d) = %s\r\n",
        pt3.x, pt3.y, ResultStr(bResult));

    pt4 = pt6 - siz;
    bResult = ((pt4.x == 0) && (pt4.y == 0));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt4 = pt6 - siz;  => (%d, %d) = %s\r\n",
        pt4.x, pt4.y, ResultStr(bResult));

    pt5 = -pt6;
    bResult = ((pt5.x == -1) && (pt5.y == -2));
    bAllResult &= bResult;
    bMsCompat &= bResult;
    outPrintf("pt5 = -pt6;       => (%d, %d) = %s\r\n",
        pt5.x, pt5.y, ResultStr(bResult));

    // Show + and - for RECT* to CRect ?
    // !!!! NYI
    
    outPrintf("\r\n"
        "Summary\r\n"
        "-------\r\n"
        "MS Compatibility   = %s\r\n"
        "Full Functionality = %s\r\n",
        ResultStr(bMsCompat),
        ResultStr(bAllResult));
}
