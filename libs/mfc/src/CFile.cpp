/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CFile.cpp
  Version    : 0.30
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file implements CFile and CFileException.
  I have used FILE* in order to be as portable as possible - that way
  it can also be used in GFC. -wdh

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  who   ver   what
10-05-03  wdh   0.10  Created.
11-02-03  wdh   0.20  Fixed error handling in Open(), return value in
                      SeekToBegin() and SeekToEnd().
11-09-03  wdh   0.30  Fixed: errno was not reset.

 --[ How to compile ]----------------------------------------------------

  This file was developed under DevC++ 4.9.8

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
  Copyright (c) 2003 Open Foundation Classes
  Copyright (c) 2003 William D. Herndon
/************************************************************************/

//#include <string>
#include <errno.h>
#include <cstdlib>
#include <afxwin.h>


//************************************************** CFileException

IMPLEMENT_DYNAMIC(CFileException, CException)

int CFileException::ErrnoToException(int nErrno)
{
	switch(nErrno) {
	case EPERM:
	case EACCES:
		return( CFileException::accessDenied );
	case EBADF:
		return( CFileException::invalidFile );
	case EDEADLOCK:
		return( CFileException::sharingViolation );
	case EMFILE:
		return( CFileException::tooManyOpenFiles );
	case ENOENT:
	case ENFILE:
		return( CFileException::fileNotFound );
	case ENOSPC:
		return( CFileException::diskFull );
	case EINVAL:
	case EIO:
		return( CFileException::hardIO );
	default:
		return( CFileException::generic );
	}
}

void CFileException::ThrowErrno(int iErr)
{
	if (iErr == 0)
		return;
	// Create the CFileException
	CFileException* pErr = new CFileException(ErrnoToException(iErr));
	// Set the (passed) errno as extra info and reset the errno
	pErr->m_errno = iErr;
	errno = 0;
	// throw the exception
	throw(pErr);
}


//************************************************** CFile

IMPLEMENT_DYNAMIC(CFile, CObject)

CFile::~CFile()
{
	if (m_pFile != NULL)
		Close();
}

BOOL CFile::Open(
	LPCSTR pszFileName,
	UINT   nOpenFlags,
	CFileException* pError
) {
	char szFlags[10];
	char szFilename[_MAX_PATH];
	if ((nOpenFlags & modeReadWrite) != 0) {
		if ((nOpenFlags & modeCreate) != 0)
			lstrcpy(szFlags, "w+");
		else
			lstrcpy(szFlags, "r+");
	} else if ((nOpenFlags & modeWrite) != 0)
		lstrcpy(szFlags, "w");
	else
		lstrcpy(szFlags, "r");
	// Make it binary - compatible and fewer problems with pointers.
	lstrcat(szFlags, "b");
	lstrcpy(szFilename, pszFileName);
	m_pFile = fopen(szFilename, szFlags);
	if (m_pFile != NULL)
		return( TRUE );
	if (((int)pError) != 0/*NULL*/) {
		pError->m_errno = errno;
		errno = 0;
		pError->m_cause = CFileException::ErrnoToException(pError->m_errno);
	}
	return( FALSE );
}

long CFile::Seek(long lOff,UINT nFrom)
{
	errno = 0;
	int iRet = fseek(m_pFile, lOff, nFrom);
	if (iRet == 0)
		CFileException::ThrowErrno(errno);
	long lRet = ftell(m_pFile);
	if (lRet < 0)
		CFileException::ThrowErrno(errno);
	return( lRet );
}

UINT CFile::ReadHuge(void* pBuf,UINT nCount)
{
	ASSERT(m_pFile != NULL);
	errno = 0;
	UINT nRet = fread(pBuf, sizeof(char), nCount, m_pFile);
	if ( ferror(m_pFile) )
		CFileException::ThrowErrno(errno);
	return( nRet );
}

void CFile::WriteHuge(const void* pBuf,UINT nCount)
{
	ASSERT(m_pFile != NULL);
	errno = 0;
	long lRet = fwrite(pBuf, sizeof(char), nCount, m_pFile);
	if ( ferror(m_pFile) )
		CFileException::ThrowErrno(errno);
	if (lRet != nCount)
		AfxThrowFileException(CFileException::diskFull);
}

UINT CFile::Read(void* pBuf,UINT nCount)
{
	return( ReadHuge(pBuf, nCount) );
}

void CFile::Write(const void* pBuf,UINT nCount)
{
	WriteHuge(pBuf, nCount);
}

void CFile::Flush()
{
	errno = 0;
	int iRet = fflush(m_pFile);
	if (iRet == 0)
		CFileException::ThrowErrno(errno);
}

void CFile::Close()
{
	ASSERT(m_pFile != NULL);
	errno = 0;
	int iRet = fclose(m_pFile);
	m_pFile = NULL;
	if (iRet == 0)
		CFileException::ThrowErrno(errno);
}

// Same as close, but no exception thrown
void CFile::Abort()
{
	fclose(m_pFile);
	m_pFile = NULL;
}

void CFile::SeekToBegin()
{
	Seek(0, CFile::begin);
}

DWORD CFile::SeekToEnd()
{
	return( Seek(0, CFile::end) );
}
