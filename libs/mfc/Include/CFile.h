/************************************************************************
   T h e   O p e n   F o u n d a t i o n   C l a s s e s
 ------------------------------------------------------------------------
   Filename   : CFile.h
   Author(s)  : William D. Herndon

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
10-05-03  0.10  wdh  Created.
11-02-03  0.20  wdh  Fixed Rename and Remove to throw correct exception,
                     corrected return value for SeekToBegin() and SeekToEnd().

 ------------------------------------------------------------------------
 Copyright (c) 2003 by The Open Foundation Classes
 Copyright (c) 2003 by William D. Herndon
/************************************************************************/

#ifndef CFILE_H
#define CFILE_H

#include <stdio.h>

// CFileException - the exceptions that CFile throws.
class CFileException : public CException {
	DECLARE_DYNAMIC(CFileException);

	enum {
		none,
		generic,
		fileNotFound,
		badPath,
		tooManyOpenFiles,
		accessDenied,
		invalidFile,
		removeCurrentDir,
		directoryFull,
		badSeek,
		hardIO,
		sharingViolation,
		lockViolation,
		diskFull,
		endOfFile
	};

	int     m_cause;
	int     m_errno;
	CString m_strFileName;

	CFileException() { m_cause = generic; };
	CFileException(int eType) { m_cause = eType; };
static	int ErrnoToException(int nErrno);
static	void ThrowErrno(int nErrno);
};


inline void AfxThrowFileException(int eExceptType)
	{ throw(new CFileException(eExceptType)); };


extern int dosrename(LPCSTR szold, LPCSTR sznew);
#pragma aux dosrename = \
	"mov ah, 56h" \
	"int 21h" \
	parm [ds dx es di] \
	value [ax] \
	modify [ds dx es di];

extern int dosremove(LPCSTR sz);
#pragma aux dosremove = \
	"mov ah, 41h" \
	"int 21h" \
	parm [ds dx] \
	value [ax] \
	modify [ds dx];

// CFile - standard file function in the form of a class.
// Especially useful when you want to do things like write
// alternately to a file or to memory.
class CFile : public CObject {
	DECLARE_DYNAMIC(CFile)
public:
// Flag values
	enum OpenFlags {
		modeRead =       0x0000,
		modeWrite =      0x0001,
		modeReadWrite =  0x0002,
		shareCompat =    0x0000,
		shareExclusive = 0x0010,
		shareDenyWrite = 0x0020,
		shareDenyRead =  0x0030,
		shareDenyNone =  0x0040,
		modeNoInherit =  0x0080,
		modeCreate =     0x1000,
		typeText =       0x4000,
		typeBinary =(int)0x8000
	};
	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
	};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	FILE*  m_pFile;

	CFile() { m_pFile = NULL; };
	CFile(LPCSTR pszFileName,UINT nOpenFlags)
		{ Open(pszFileName, nOpenFlags, NULL); };


static	void Rename(LPCSTR pszOldName,LPCSTR pszNewName) {
		if (dosrename(pszOldName, pszNewName) != 0) ;
			// @todo CFileException::ThrowErrno(errno);
	};
static	void Remove(LPCSTR pszFileName) {
		if (dosremove(pszFileName) != 0) ;
			// @todo CFileException::ThrowErrno(errno);
	};

	// Almost everything must be virtual, so that
	// we can do memory files and other fun stuff.
	virtual BOOL Open(LPCSTR pszFileName,UINT nOpenFlags,
		CFileException* pError = NULL);
	virtual long Seek(long lOff,UINT nFrom);
	virtual void SeekToBegin();
	virtual DWORD SeekToEnd();
	virtual UINT ReadHuge(void* pBuf,UINT nCount);
	virtual UINT Read(void* pBuf,UINT nCount);
	virtual void WriteHuge(const void* pBuf,UINT nCount);
	virtual void Write(const void* pBuf,UINT nCount);
	virtual void Flush();
	virtual void Close();
	virtual void Abort();
	virtual ~CFile();
};

#endif // CFILE_H
