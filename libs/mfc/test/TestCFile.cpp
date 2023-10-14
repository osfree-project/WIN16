/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : TestCFile.cpp
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
10-26-03  0.10  wdh  Created.
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

void dbg_printf(LPCSTR pszForm,...);

LPCSTR CFileExceptionStr(int cause)
{
	switch (cause) {
	case CFileException::none:
		return( "none" );
	case CFileException::generic:
		return( "generic" );
	case CFileException::fileNotFound:
		return( "fileNotFound" );
	case CFileException::badPath:
		return( "badPath" );
	case CFileException::tooManyOpenFiles:
		return( "tooManyOpenFiles" );
	case CFileException::accessDenied:
		return( "accessDenied" );
	case CFileException::invalidFile:
		return( "invalidFile" );
	case CFileException::removeCurrentDir:
		return( "removeCurrentDir" );
	case CFileException::directoryFull:
		return( "directoryFull" );
	case CFileException::badSeek:
		return( "badSeek" );
	case CFileException::hardIO:
		return( "hardIO" );
	case CFileException::sharingViolation:
		return( "sharingViolation" );
	case CFileException::lockViolation:
		return( "lockViolation" );
	case CFileException::diskFull:
		return( "diskFull" );
	case CFileException::endOfFile:
		return( "endOfFile" );
	}
	return( "!!Unknown!!" );
}

// Test the CFile class for proper functionality
void TestCFile()
{
    BOOL bResult, bAllResult = TRUE, bMsCompat = TRUE;
    outPrintf("\r\n"
        "Testing CFile functionality\r\n"
        "---------------------------\r\n");

    CFile fil1;
    CFileException filExc;
    char  szBuf[80];
    LPSTR pszName1 = "CreateFile.Test";
    LPSTR pszName2 = "RenameFile.Test";
    LPSTR pszBuf = "This is a test!";
    long  lOffs;
    BOOL  bOpen;

    outPrintf("LPSTR pszName1 = \"%s\"\r\n", pszName1);
    outPrintf("LPSTR pszName2 = \"%s\"\r\n", pszName2);
    outPrintf("LPSTR pszBuf = \"%s\"\r\n", pszBuf);

    outPrintf("Try basic stuff: open a file with modeReadWrite|modeCreate,\r\n"
        "write to it, seek, read from it and close\r\n");
    try {
        bOpen = fil1.Open(pszName1,
            CFile::modeReadWrite|CFile::modeCreate|CFile::typeBinary);
        bResult = (bOpen == TRUE);
        bAllResult &= bResult;
        bMsCompat  &= bResult;
        outPrintf("fil1.Open(pszName1,\r\n"
            "    modeReadWrite|modeCreate|typeBinary) = %d = %s\r\n",
            bOpen, ResultStr(bResult));
    } catch (...) {
        outPrintf("Unknown exception in fil1.Open()!\r\n");
        bAllResult = FALSE;
        bMsCompat = FALSE;
    }
    if ( bOpen ) try {
        fil1.Write(pszBuf, strlen(pszBuf));
        outPrintf("fil1.Write(pszBuf, strlen(pszBuf));\r\n");
        lOffs = fil1.Seek(-5, CFile::end);
        bResult = (lOffs == 10);
        bAllResult &= bResult;
        bMsCompat  &= bResult;
        outPrintf("fil1.Seek(-5, CFile::end) = %d\r\n",
            lOffs, ResultStr(bResult));
        szBuf[4] = '\0';
        fil1.Read(szBuf, 4);
        bResult = (strcmp(szBuf, "test") == 0);
        bAllResult &= bResult;
        bMsCompat  &= bResult;
        outPrintf("fil1.Read(szBuf, 4); => \"%s\" = %s\r\n",
            (LPCSTR)szBuf, ResultStr(bResult));
        lOffs = fil1.SeekToEnd();
        bResult = (lOffs == (long)strlen(pszBuf));
        bAllResult &= bResult;
        bMsCompat  &= bResult;
        outPrintf("fil1.SeekToEnd(); => %d = %s\r\n",
            lOffs, ResultStr(bResult));
        fil1.Write("\r\n", 2);
        outPrintf("fil1.Write(\"\\r\\n\", 2);\r\n");
    } catch (...) {
        outPrintf("Unknown exception!\r\n");
        bAllResult = FALSE;
        bMsCompat = FALSE;
    }
    if ( bOpen ) try {
        fil1.Close();
        outPrintf("fil1.Close();\r\n");
    } catch (...) {
        outPrintf("Unknown exception in fil1.Close()!\r\n");
        bAllResult = FALSE;
        bMsCompat = FALSE;
    }

    try {
        outPrintf("fil1.Rename(pszName1, pszName2);\r\n");
        fil1.Rename(pszName1, pszName2);
    } catch (...) {
        outPrintf("Unknown exception in fil1.Rename()!\r\n");
        bAllResult = FALSE;
        bMsCompat = FALSE;
    }

    outPrintf("Try reopening the file with modeRead,\r\n"
        "do various seek and reads and close it\r\n");
    try {
        bOpen = fil1.Open(pszName2, CFile::modeRead|CFile::typeBinary);
        bResult = bOpen;
        bAllResult &= bResult;
        bMsCompat  &= bResult;
        outPrintf("fil1.Open(pszName2,\r\n"
            "    modeRead|typeBinary) = %d = %s\r\n",
            bOpen, ResultStr(bResult));
        if ( bOpen ) {
            lOffs = fil1.Seek(5, CFile::begin);
            bResult = (lOffs == 5);
            bAllResult &= bResult;
            bMsCompat  &= bResult;
            outPrintf("fil1.Seek(5, CFile::begin); => %d = %s\r\n",
                lOffs, ResultStr(bResult));
            szBuf[2] = '\0';
            fil1.Read(szBuf, 2);
            bResult = (strcmp(szBuf, "is") == 0);
            bAllResult &= bResult;
            bMsCompat  &= bResult;
            outPrintf("fil1.Read(szBuf, 2); => \"%s\" = %s\r\n",
                (LPCSTR)szBuf, ResultStr(bResult));

            fil1.SeekToBegin();
            outPrintf("fil1.SeekToBegin(); (void)\r\n");

            szBuf[4] = '\0';
            fil1.Read(szBuf, 4);
            bResult = (strcmp(szBuf, "This") == 0);
            bAllResult &= bResult;
            bMsCompat  &= bResult;
            outPrintf("fil1.Read(szBuf, 2); => \"%s\" = %s\r\n",
                (LPCSTR)szBuf, ResultStr(bResult));

            lOffs = fil1.Seek(4, CFile::current);
            bResult = (lOffs == 8);
            bAllResult &= bResult;
            bMsCompat  &= bResult;
            outPrintf("fil1.Seek(4, CFile::current); => %d = %s\r\n",
                lOffs, ResultStr(bResult));

            szBuf[7] = '\0';
            fil1.Read(szBuf, 7);
            bResult = (strcmp(szBuf, "a test!") == 0);
            bAllResult &= bResult;
            bMsCompat  &= bResult;
            outPrintf("fil1.Read(szBuf, 7); => \"%s\" = %s\r\n",
                (LPCSTR)szBuf, ResultStr(bResult));

            szBuf[2] = '\0';
            fil1.Read(szBuf, 2);
            bResult = (strcmp(szBuf, "\r\n") == 0);
            bAllResult &= bResult;
            bMsCompat  &= bResult;
            outPrintf("fil1.Read(szBuf, 2); => \"\\x%02x\\x%02x\" = %s\r\n",
                szBuf[0], szBuf[1], ResultStr(bResult));

            fil1.Close();
            outPrintf("fil1.Close();\r\n");
        }
    } catch (...) {
        outPrintf("Unknown exception!\r\n");
        bAllResult = FALSE;
        bMsCompat = FALSE;
    }

    outPrintf("Try removing the file\r\n");
    try {
        CFile::Remove(pszName2);
        outPrintf("fil1.Remove(pszName2)\r\n");
    } catch (...) {
        outPrintf("Unknown exception during Remove()!\r\n");
        bAllResult = FALSE;
        bMsCompat = FALSE;
    }

    outPrintf("Try opening the file we removed w/ modeRead (should fail)\r\n");
    try {
        bOpen = fil1.Open(pszName2,
            CFile::modeRead|CFile::typeBinary, &filExc);
        bResult = (bOpen == FALSE) &&
            (filExc.m_cause == CFileException::fileNotFound);
        bAllResult &= bResult;
        bMsCompat  &= bResult;
        outPrintf("fil1.Open(pszName2,\r\n"
            "    modeRead|typeBinary, &filExc) = %d / filExc=%d = %s\r\n",
            bOpen, (int)filExc.m_cause, ResultStr(bResult));
        if ( bOpen ) {
            fil1.Close();
            outPrintf("fil1.Close();\r\n");
        }
    } catch (...) {
        outPrintf("Unknown exception!\r\n");
        bAllResult = FALSE;
        bMsCompat = FALSE;
    }

    int  iCause = 0;
    BOOL bExcept = FALSE;

    outPrintf("Try provoking some exceptions\r\n");
    try {
        fil1.Rename(pszName1, pszName2);
    } catch (CFileException* pE) {
        iCause = pE->m_cause;
        bExcept = TRUE;
    } catch (...) {
        bExcept = TRUE;
    }
    bResult = (iCause == CFileException::fileNotFound);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("fil1.Rename(pszName1, pszName2);\r\n"
        "Except=%d, cause=%s => %s\r\n", bExcept,
	CFileExceptionStr(iCause), ResultStr(bResult));

    try {
        fil1.Remove(pszName2);
    } catch (CFileException* pE) {
        iCause = pE->m_cause;
        bExcept = TRUE;
    } catch (...) {
        bExcept = TRUE;
    }
    bResult = (iCause == CFileException::fileNotFound);
    bAllResult &= bResult;
    bMsCompat  &= bResult;
    outPrintf("fil1.Remove(pszName2);\r\n"
        "Except=%d, cause=%s => %s\r\n", bExcept,
	CFileExceptionStr(iCause), ResultStr(bResult));


/*
 Yet to test:
	CFile(LPCSTR pszFileName,UINT nOpenFlags)
	void Flush();
	void Abort();
exceptions of various kinds
*/

    outPrintf("\r\n"
        "Summary\r\n"
        "-------\r\n"
        "MS Compatibility   = %s\r\n"
        "Full Functionality = %s\r\n",
        ResultStr(bMsCompat),
        ResultStr(bAllResult));
}
