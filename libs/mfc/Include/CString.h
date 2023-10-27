/************************************************************************
   T h e   O p e n   F o u n d a t i o n   C l a s s e s
 ------------------------------------------------------------------------
   Filename   : CString.h
   Version    : 0.40
   Author(s)  : Ivan Deras

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
??-??-00  0.10  id   Created.
10-05-03  0.20  wdh  Added this history, changed to use windef.h
                     instead of OfcTypes.h.
10-26-03  0.30  wdh  Now uses <windows.h>, implemented collate functions,
                     removed 'implemented' commentary, implemented SysString,
                     methods, LoadString() and ANSI/OEM methods.
05-30-04  0.40  wdh  Moved ASSERT() to ofcglobals.h, added MSVC32 check for
    AfxStrICompare().

 ------------------------------------------------------------------------
 Copyright (c) 2000-04 by The Open Foundation Classes
 Copyright (c) 2000 by Carsten Breuer
/************************************************************************/

#ifndef CSTRING_H
#define CSTRING_H

#include <windows.h>

//So we can change the alloc-free functions
#ifndef mem_alloc
#define mem_alloc(size) std::malloc(size)
#define mem_free(obj) std::free(obj)
#else
#ifndef mem_free
#error "You must redefine mem_free if you had redefined mem_alloc"
#endif
#endif

//DELTA is the block size for strings
#define DELTA 32

#define afxEmptyString AfxGetEmpyString()

//This string functions could change depending on the compiler
//#ifndef MSVC32
//#define AfxStrCompare(a, b) a.Compare(b)
//#else
#define AfxStrCompare(a, b) lstrcmp( (a), (b) )
//#endif
//#ifndef MSVC32
//#define AfxStrICompare(a, b) strcasecmp( (a), (b) )
//#else
#define AfxStrICompare(a, b) lstrcmpi( (a), (b) )
//#endif
#define AfxStrNCompare(a, b, n) _fstrncmp( (a), (b), (n) )
#define AfxStrStr(a, b) _fstrstr( (a), (b) )
#define AfxStrLen(a) lstrlen( (a) )
#define AfxStrChr(a, b) lstrchr((a), (b))

class CMemoryException;

typedef char TCHAR;
typedef const wchar_t* LPCWSTR;
//typedef const char* LPCSTR;
//typedef char* LPSTR;
typedef LPCSTR LPCTSTR;
typedef LPSTR LPTSTR;

struct CStringData
{   long nRefs;
    int nDataLength;
    int nAllocLength;

    TCHAR* data() { return (TCHAR*)(&this[1]); };
    BOOL IsShared() { return nRefs > 1; }; //ofc enhancement
    BOOL IsEmpty() { return nDataLength == 0; }; //ofc enhancement
    BOOL IsLocked() { return (nRefs == -1 && nDataLength != 0); }  //ofc enhancement
    void Lock() { nRefs = -1; } //ofc enhancement
    void UnLock() { if ( IsLocked() ) nRefs = 1; } //ofc enhancement
    long IncRef() { if (!IsEmpty()) return nRefs++; else return nRefs; }; //ofc enhancement
    long DecRef() { if (!IsEmpty()) return nRefs--; else return nRefs; }; //ofc enhancement
};

typedef char FAR * BSTR;

class CString
{
public:

	//Constructors
	CString();
	CString( const CString& str );
	CString( TCHAR ch, int nCount = 1 );
	CString( LPCTSTR pstr, int nLen );
	CString( const unsigned char* pstr );
	CString( LPCWSTR pwstr ); //Not Implemented
	CString( LPCSTR pstr );
	~CString( );

	//Operators
	TCHAR operator []( int nIndex ) const;
	operator LPCTSTR ( ) const;

	//Assigment operator
	const CString& operator =( const CString& rhs_str );
	const CString& operator =( TCHAR ch );
	const CString& operator =( const unsigned char* pstr );
	const CString& operator =( LPCWSTR pwstr ); //Not implemented
	const CString& operator =( LPCSTR pstr );

	//Add operator
	friend CString operator +( const CString& cstr1, const CString& cstr2 );
	friend CString operator +( const CString& cstr, TCHAR ch );
	friend CString operator +( TCHAR ch, const CString& cstr );
	friend CString operator +( const CString& cstr, LPCTSTR pstr );
	friend CString operator +( LPCTSTR pstr, const CString& str );

	const CString& operator +=( const CString& cstr );
	const CString& operator +=( TCHAR ch );
	const CString& operator +=( LPCTSTR pstr );

	//Member functions
	int GetLength( ) const;
	BOOL IsEmpty( ) const;
	void Empty( );
	TCHAR GetAt( int nIndex ) const;
	void SetAt( int nIndex, TCHAR ch );
	void Fill( TCHAR ch, int nCount ); //ofc enhancement

	int Compare( LPCTSTR pstr ) const;
	int CompareNoCase( LPCTSTR pstr ) const;
	int Collate( LPCTSTR pstr ) const;
	int CollateNoCase( LPCTSTR pstr ) const;

	CString Mid( int nPos ) const;
	CString Mid( int nPos, int nCount ) const;
	CString Left( int nCount ) const;
	CString Right( int nCount ) const;
	CString SpanIncluding( LPCTSTR pstr ) const;
	CString SpanExcluding( LPCTSTR pstr ) const;

	void MakeUpper( );
	void MakeLower( );
	void MakeReverse( );
	int Replace( TCHAR oldChar, TCHAR newChar );
	int Replace( LPCTSTR poldStr, LPCTSTR pnewStr );
	int Remove( TCHAR ch );
	int Insert( int nIndex, TCHAR ch );
	int Insert( int nIndex, LPCTSTR pstr );
	int Delete( int nIndex, int nCount = 1 );

	/*Not implemented yet
	void Format( LPCTSTR pstrFmt, ... );
	void Format( UINT nFormatID, ... );
	void FormatV( LPCTSTR pstrFormat, va_list argList );
	void FormatMessage( LPCTSTR pstrFormat, ... );
	void FormatMessage( UINT nFormatID, ... );*/

	void TrimLeft( );
	void TrimLeft( TCHAR ch );
	void TrimLeft( LPCTSTR pstr );
	void TrimRight( );
	void TrimRight( TCHAR ch );
	void TrimRight( LPCTSTR pstr );

	int Find( TCHAR ch, int nPos = -1 ) const;
	int Find( LPCTSTR pstr, int nPos = -1 ) const;
	int ReverseFind( TCHAR ch ) const;
	int FindOneOf( LPCTSTR lpszCharSet ) const;

	/*friend CArchive& operator <<( CArchive& ar, const CString& string ); throw( CArchiveException );
	friend CArchive& operator >>( CArchive& ar, CString& string ); throw( CArchiveException );
	friend CDumpContext& operator <<( CDumpContext& dc, const CString& string );
	friend CArchive& operator <<( CArchive& ar, const CString& string ); throw( CArchiveException );
	friend CArchive& operator >>( CArchive& ar, CString& string ); throw( CArchiveException );
	friend CDumpContext& operator <<( CDumpContext& dc, const CString& string );*/

	LPTSTR GetBuffer( int nLen );
	LPTSTR GetBufferSetLength( int nLen );
	void ReleaseBuffer( int nLen = -1 );
	void FreeExtra( );
	LPTSTR LockBuffer( );
	void UnlockBuffer( );

	BSTR AllocSysString ( ) const;
	BSTR SetSysString( BSTR* pbstr ) const;
	BOOL LoadString( UINT nID );
	void AnsiToOem( );
	void OemToAnsi( );

 protected: //Helper functions
	LPTSTR m_pchData;

	void AllocBuffer( int nLen );
	void AllocCopy(CString& dest, int nLen, int nIndex, int nLen2 = 0) const;
	void AllocBeforeWrite(int nLen);

	void SetData( int nLen, LPCTSTR pstr ); //ofc enhancement
	void AssignCopy(int nLen, LPCTSTR pstr);
	void ConcatCopy(int nLen1, LPCTSTR pstr1, int nLen2, LPCTSTR pstr2);
	void ConcatInPlace(int nLen, LPCTSTR pstr);
	void CopyBeforeWrite();

	void Init();
	void Release();
	CStringData* GetData() const;

	static void Release(CStringData* pData);
	static int SafeStrlen(LPCTSTR pstr);
	static void FreeData(CStringData* pData);
};

//Comparison operators
BOOL operator ==( const CString& cstr1, const CString& cstr2 );
BOOL operator ==( const CString& cstr1, LPCTSTR pstr2 ); 
BOOL operator ==( LPCTSTR pstr1, const CString& cstr2 ); 
BOOL operator !=( const CString& cstr1, const CString& cstr2 ); 
BOOL operator !=( const CString& cstr1, LPCTSTR pstr2 ); 
BOOL operator !=( LPCTSTR pstr1, const CString& cstr2 ); 
BOOL operator <( const CString& cstr1, const CString& cstr2 ); 
BOOL operator <( const CString& cstr1, LPCTSTR pstr2 ); 
BOOL operator <( LPCTSTR pstr, const CString& cstr ); 
BOOL operator >( const CString& cstr1, const CString& cstr2 ); 
BOOL operator >( const CString& cstr1, LPCTSTR pstr2 ); 
BOOL operator >( LPCTSTR pstr1, const CString& cstr2 ); 
BOOL operator <=( const CString& cstr1, const CString& cstr2 ); 
BOOL operator <=( const CString& cstr1, LPCTSTR pstr2 ); 
BOOL operator <=( LPCTSTR pstr1, const CString& cstr2 ); 
BOOL operator >=( const CString& cstr1, const CString& cstr2 ); 
BOOL operator >=( const CString& cstr1, LPCTSTR pstr2 ); 
BOOL operator >=( LPCTSTR pstr1, const CString& cstr2 ); 

#endif // CSTRING_H
