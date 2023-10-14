/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : TestCSize.cpp
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

// Test the CSize class for proper functionality
void TestCSize()
{
    POINT pt = { 1, 2 };
    SIZE  siz = { 1, 2 };
    CSize siz1; // Default initialization
    CSize siz2(1, 2); // Initialize with int params
    CSize siz3(pt);   // Initialize with POINT
    CSize siz4(siz);  // Initialize with SIZE
    CSize siz5(0x00020001L); // Initialize with DWORD
// This did not compile before the "=" operator for POINT was made explicit.
// It seems to be a compiler incompatibility -wdh
    CSize siz6 = siz;

// The following two assignments crashed before the "=" operator was made
// explicit. It should not be necessary, since the class is based on POINT,
// but there seems to be some sort of GCC incompatibility. -wdh
    CSize siz7;
    CSize siz8;

    siz7 = siz;
#ifndef MSVC32 // This does not compile under MS Visual C
    siz8 = (SIZE){ 1, 2 };
#else
    siz8 = siz;
#endif

    BOOL bResult, bAllResult = TRUE, bMsCompat = TRUE;
    outPrintf("\r\n"
        "Testing CSize functionality\r\n"
        "---------------------------\r\n");

    // Show all the different kinds of initialization
    outPrintf("Initialization / Assignment\r\n");

    bResult = ((siz1.cx==0) && (siz1.cy==0));
    bAllResult &= bResult;
    outPrintf("CSize siz1;                        siz1 => (%d, %d) = %s\r\n",
	siz1.cx, siz1.cy, ResultStr(bResult));
    siz1.cx = siz1.cy = 0; // MS does not initialize to (0, 0)

    bResult = ((siz2.cx==1) && (siz2.cy==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CSize siz2(1, 2);                  siz2 => (%d, %d) = %s\r\n",
	siz2.cx, siz2.cy, ResultStr(bResult));

    bResult = ((siz3.cx==1) && (siz3.cy==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CSize siz3(pt);                    siz3 => (%d, %d) = %s\r\n",
	siz3.cx, siz3.cy, ResultStr(bResult));

    bResult = ((siz4.cx==1) && (siz4.cy==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CSize siz4(siz);                   siz4 => (%d, %d) = %s\r\n",
	siz4.cx, siz4.cy, ResultStr(bResult));

    bResult = ((siz5.cx==1) && (siz5.cy==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CSize siz5(0x00020001L);           siz5 => (%d, %d) = %s\r\n",
	siz5.cx, siz5.cy, ResultStr(bResult));

    bResult = ((siz6.cx==1) && (siz6.cy==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CSize siz6 = siz;                  siz6 => (%d, %d) = %s\r\n",
	siz6.cx, siz6.cy, ResultStr(bResult));

    bResult = ((siz7.cx==1) && (siz7.cy==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CSize siz7; siz7 = siz;            siz7 => (%d, %d) = %s\r\n",
	siz7.cx, siz7.cy, ResultStr(bResult));

    bResult = ((siz8.cx==1) && (siz8.cy==2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("CSize siz8; siz8 = (SIZE){ 1, 2 }; siz8 => (%d, %d) = %s\r\n",
	siz8.cx, siz8.cy, ResultStr(bResult));

    // Show the Boolean Operators
    BOOL bCompare;
    outPrintf("\r\nBoolean Operators\r\n");

    bCompare = (siz2 == siz1);
    bResult = (bCompare == FALSE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(siz2 == siz1) => %d = %s\r\n",
        bCompare, ResultStr(bResult));

    bCompare = (siz2 == siz3);
    bResult = (bCompare == TRUE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(siz2 == siz3) => %d = %s\r\n",
        bCompare, ResultStr(bResult));

    bCompare = (siz2 != siz1);
    bResult = (bCompare == TRUE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(siz2 != siz1) => %d = %s\r\n",
        bCompare, ResultStr(bResult));

    bCompare = (siz2 != siz3);
    bResult = (bCompare == FALSE);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("(siz2 != siz3) => %d = %s\r\n",
        bCompare, ResultStr(bResult));

    // Show += and -=
    // Note that CSize does not support += and -= with CPoint
    // although CPoint supports += and -= with CSize.
//    siz1 += pt;
//    siz2 -= pt;
    outPrintf("\r\n+= and -= operators\r\n");
    siz3 += siz;
    bResult = ((siz3.cx == 2) && (siz3.cy == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("siz3 += siz; => (%d, %d) = %s\r\n",
        siz3.cx, siz3.cy, ResultStr(bResult));

    siz4 -= siz;
    bResult = ((siz4.cx == 0) && (siz4.cy == 0));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("siz4 -= siz; => (%d, %d) = %s\r\n",
        siz4.cx, siz4.cy, ResultStr(bResult));

    // Show + and -
    CPoint pt1, pt3;
    outPrintf("\r\n+ and - operators\r\n");
    pt1 = siz6 + pt;
    bResult = ((pt1.x == 2) && (pt1.y == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt1 = siz6 + pt;   => (%d, %d) = %s\r\n",
        pt1.x, pt1.y, ResultStr(bResult));


    siz2 = siz6 + siz;
    bResult = ((siz2.cx == 2) && (siz2.cy == 4));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("siz2 = siz6 + siz;  => (%d, %d) = %s\r\n",
        siz2.cx, siz2.cy, ResultStr(bResult));

    pt3 = siz6 - pt;
    bResult = ((pt3.x == 0) && (pt3.y == 0));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("pt3 = siz6 - pt;   => (%d, %d) = %s\r\n",
        pt3.x, pt3.y, ResultStr(bResult));

    siz4 = siz6 - siz;
    bResult = ((siz4.cx == 0) && (siz4.cy == 0));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("siz4 = siz6 - siz;  => (%d, %d) = %s\r\n",
        siz4.cx, siz4.cy, ResultStr(bResult));

    siz5 = -siz6;
    bResult = ((siz5.cx == -1) && (siz5.cy == -2));
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("siz5 = -siz6;       => (%d, %d) = %s\r\n",
        siz5.cx, siz5.cy, ResultStr(bResult));

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
