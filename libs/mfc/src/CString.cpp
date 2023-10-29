/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CString.cpp
  Version    : 0.50
  Author(s)  : Ivan Deras

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  Implementation of the class CString.

 --[ Developers ]--------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

 --[ History ] ----------------------------------------------------------

  0.10: Original file by Ivan Deras.

mm-dd-yy  ver   who
10-05-03  0.20  wdh
  Changed to use windef.h instead of OfcTypes.h. Split off inline
  functions to CString_inline.h. Removed default value for nPos in Find(),
  which caused a compiler error (it should be and is in the include file).
10-26-03  0.30  wdh
  Fixed operator =(TCHAR ch): did not set length. Fixed bug in ConcatInPlace()
  that caused crashes. Added missing IncRef() to operator =(CString&),
  fixed operator =(TCHAR ch), implemented collate functions (the MFC function
  does not seem to work right, but at least I think we are compatible),
  implemented Mid(nPos), fixes to Mid(nPos, nLen), fixed TrimLeft/TrimRight
  ("\r" forgotten), fixed LockBuffer(), implemented AllocSysString(),
  SetSysString(), LoadString(), AnsiToOem() and OemToAnsi().
11-25-03  0.40  wdh
  Fixed Remove(), needed to assign null at end, and GetBuffer(), length
  for SetData() was the wrong.

mm-dd-yy  who  ver   What
05-30-04  wdh  0.50  Added ifndef MSVC32, fixed Remove() ('\0' assignment
    at end was missing), fixed both range check for Find()s, fixed data length
    in SetData() for GetBuffer() (intermittent crash), fixed LoadString() to
    return a value.

 --[ To Do ]-------------------------------------------------------------
  Format functions, operators >> and <<, Unicode and MBCS support.

 --[ How to compile ]----------------------------------------------------

  This file was developed under DevC++ 4

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
  Copyright (c) 2000-04 Open Foundation Classes
  Copyright (c) 2000 Carsten Breuer
/************************************************************************/
#include <afxwin.h>
#include <ole2.h>
#include <dispatch.h>
#include <cstdlib>
#include <cstring>
#ifdef _DEBUG
void dbg_printf(LPCSTR pszForm,...);
#endif


// Helper string functions (far version)
void FAR * lmemset (void FAR * dest, int val, int len)
{
  char FAR *ptr = (char FAR *)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

void lmemcpy(LPCSTR s1, LPCSTR s2, unsigned length)
{	char FAR * p;
	char FAR * q;

	if(length) {
		p = (LPSTR)s1;
		q = (LPSTR)s2;
		do *p++ = *q++;
		while(--length);
	}
}

char FAR *lstrchr(const char FAR *s, int c)
{
	const char ch = c;

	for ( ; *s != ch; s++)
		if (*s == '\0')
			return 0;
	return (char FAR *)s;
}

int lstrncmp(LPCSTR s1, LPCSTR s2, int n)
{
	if (n == 0)
		return (0);
	do {
		if (*s1 != *s2++)
			return (*(unsigned char *)s1 - *(unsigned char *)--s2);
		if (*s1++ == 0)
			break;
	} while (--n != 0);
	return (0);
}

LPSTR lmemmove (LPSTR dest, LPCSTR src, size_t len)
{
  LPSTR d = dest;
  LPCSTR s = src;
  if (d < s)
    while (len--)
      *d++ = *s++;
  else
    {
      LPCSTR lasts = s + (len-1);
      LPSTR lastd = d + (len-1);
      while (len--)
        *lastd-- = *lasts--;
    }
  return dest;
}

LPSTR lstrstr(LPCSTR s1, LPCSTR s2)
{
	char c1, c2;
	int len;
	if ((c1=*s2++)!=0) {
		len=lstrlen(s2);
		do {
			do {
				if ((c2=*s1++)==0)
					return (NULL);
			} while (c2!=c1);
		} while (lstrncmp(s1, s2, len)!=0);
		s1--;
	}
	return ((LPSTR)s1);
}

#define MAX_LOAD_STRING 32768

static const struct {
    CStringData data;
    TCHAR ch;
} str_empty = {{-1, 0, 0}, 0};

LPCTSTR _afxPchNil = (LPCTSTR)&str_empty.ch;
static CStringData* _afxDataNil = (CStringData*)&str_empty.data;

//This function must only be available in DLL version of OFC
const CString& AfxGetEmpyString()
{ return *(CString*)&_afxPchNil; }

void CString::AllocBuffer( int nLen )
{   //We will allocate the string data in blocks of size DELTA
    int size = ((nLen+1) / DELTA) * DELTA + DELTA;
    CStringData* pData = (CStringData*)mem_alloc(sizeof(CStringData) + size*sizeof(TCHAR));
    pData->nRefs = 1;
    pData->nDataLength = nLen;
    pData->nAllocLength = size;
    m_pchData = pData->data();
    m_pchData[nLen] = 0;
}


void CString::AllocCopy(CString& str, int nLen1, int nIndex, int nLen2) const
{
	int nLen = nLen1 + nLen2;
    if (nLen == 0)
        str.Init();
    else {
        str.AllocBuffer(nLen);
		lmemcpy(str.m_pchData, m_pchData + nIndex, nLen*sizeof(TCHAR));
    }
}

void CString::AllocBeforeWrite(int nLen)
{
  CStringData* pData = GetData();
  if ( pData->IsShared() || pData->IsEmpty() || nLen > pData->nAllocLength ) {
    //If the buffer is shared we can't change it, so we allocate a new one.
    Release(); //Doesn't have effect on empty strings
    AllocBuffer(nLen);
  }
}

void CString::Init()
{
    m_pchData = afxEmptyString.m_pchData;
}

void CString::SetData(int nLen, LPCTSTR pstr)
{   CStringData* pData = GetData();

    //The buffer must be sufficiently large to allow the data
    ASSERT( pData->nAllocLength >= nLen );

    lmemcpy( m_pchData, pstr, nLen*sizeof(TCHAR) );
    pData->nDataLength = nLen;
    m_pchData[nLen] = 0;
}

void CString::AssignCopy(int nLen, LPCTSTR pstr)
{   AllocBeforeWrite( nLen );
    SetData ( nLen, pstr );
}

void CString::ConcatCopy(int nLen1, LPCTSTR pstr1, int nLen2, LPCTSTR pstr2)
{   int nLen = nLen1 + nLen2;

    if ( nLen != 0 )
    {   AllocBeforeWrite( nLen );
        SetData( nLen1, pstr1 );
        ConcatInPlace( nLen2, pstr2 );
    }
}

void CString::ConcatInPlace(int nLen, LPCTSTR pstr)
{
    if (nLen != 0)
    {
        // 10-19-03 wdh: Move CopyBeforeWrite here, if we do it after we
        // have determined that the string is big enough, we can fail horribly
        // because it may get reallocated to the minimum size.
        // This may cause double copying, but it is better than failing.
        CopyBeforeWrite();
        CStringData* pData = GetData();
        int nNewLen = nLen + pData->nDataLength;
        if ( pData->nAllocLength < nNewLen )
        {
            AllocBuffer( nNewLen );
            lmemcpy( m_pchData, pData->data(), pData->nDataLength * sizeof(TCHAR) );
            lmemcpy( m_pchData + pData->nDataLength, pstr, nLen * sizeof(TCHAR) );
            CString::Release( pData );
        }
        else //The buffer is sufficiently big to fit the data
        {
//            CopyBeforeWrite();
            lmemcpy( m_pchData + pData->nDataLength, pstr, nLen * sizeof(TCHAR) );
            pData->nDataLength = nNewLen;
        }
        m_pchData[nNewLen] = 0;
    }
}

//This function insure that the data pointed by m_pchData is owned inclusively by this string
void CString::CopyBeforeWrite()
{
    CStringData* pData = GetData();
    if ( pData->IsShared() )
    {
        pData->DecRef();
        AllocBuffer( pData->nDataLength );
        SetData( pData->nDataLength, pData->data() );
    }
}

void CString::Release()
{   CStringData *pData = GetData();

    ASSERT( pData->nRefs != 0 );

    if ( !pData->IsEmpty() )
    {   if (pData->DecRef() <= 0) //<=0 Comparison made possible to delete locked strings
        {   mem_free( pData );
            Init();
        }
    }
}

//static CString members
void CString::Release(CStringData* pData)
{   if ( !pData->IsEmpty() )
    {   if (pData->DecRef() <= 0)
            mem_free( pData );
    }
}

int CString::SafeStrlen(LPCTSTR pstr)
{
    return pstr==NULL? 0: AfxStrLen(pstr);
}

void CString::FreeData(CStringData* pData)
{
    mem_free( pData );
}
//End static members

//Constructors implementation
CString::CString()
{
    Init();
}

CString::CString( const CString& str )
{   if ( str.GetData()->IsLocked() )
        AssignCopy( str.GetData()->nDataLength, str.m_pchData);
    else
    {   m_pchData = str.m_pchData;
        GetData()->IncRef(); //Sum 1 to nRef is CStringData is not Empty
    }
}

CString::CString( TCHAR ch, int nCount )
{
    Init();
    Fill( ch, nCount );
}

CString::CString( LPCTSTR pstr, int nLen )
{
    ASSERT( nLen > 0 );
    AllocBuffer( nLen );
    SetData( nLen, pstr );
}

CString::CString( const unsigned char* pstr )
{   int nLen = CString::SafeStrlen( (LPCTSTR)pstr );
    ASSERT( nLen > 0 );
    AllocBuffer( nLen );
    SetData( nLen, (LPTSTR)pstr );
}

//Destructor
CString::~CString()
{
  Release();
}

//Not implemented yet
CString::CString( LPCWSTR pwstr )
{
    Init();
}

CString::CString( LPCSTR pstr )
{   int nLen = CString::SafeStrlen( pstr );
    if (nLen == 0)
        Init();
    else
    {   AllocBuffer( nLen );
        SetData( nLen, pstr );
    }
}

TCHAR CString::operator []( int nIndex ) const
{
    return GetAt( nIndex );
}

CString::operator LPCTSTR ( ) const
{
    return m_pchData;
}

//Assigment operator
const CString& CString::operator =( const CString& rhs_str )
{
    CStringData* pData = GetData();
    BOOL bLocked = pData->IsLocked();

    //If right hand side string or this string is locked we need to clone
    //the rhs string
    if ( rhs_str.GetData()->IsLocked() || bLocked )
    {   AssignCopy( rhs_str.GetData()->nDataLength, rhs_str.m_pchData );
        if ( bLocked ) GetData()->Lock();
    }
    else //We don't need to copy the rhs string, just reference it
    {   m_pchData = rhs_str.m_pchData;
        GetData()->IncRef();
        CString::Release( pData );
    }

    return *this;
}

const CString& CString::operator =( TCHAR ch )
{
    CStringData* pData = GetData();

    AllocBeforeWrite( 1 );
    GetData()->nDataLength = 1;
    m_pchData[0] = ch;
    m_pchData[1] = 0;

    return *this;
}

const CString& CString::operator =( const unsigned char* pstr )
{   CStringData* pData = GetData();

    int nLen = AfxStrLen( (LPCTSTR)pstr );
    if (nLen == 0)
        Empty();
    else
        AssignCopy( nLen, (LPCTSTR)pstr );

    return *this;
}

//Dummy implementation
const CString& CString::operator =( LPCWSTR pwstr )
{
    Init();

    return *this;
}

const CString& CString::operator =( LPCSTR pstr )
{   CStringData* pData = GetData();

    int nLen = CString::SafeStrlen( pstr );
    if (nLen == 0)
        Empty();
    else
        AssignCopy( nLen, pstr );

    return *this;
}

//Add operator
CString operator +( const CString& cstr1, const CString& cstr2 )
{   CString str;
    str = cstr1;
    str.ConcatInPlace( cstr2.GetData()->nDataLength, cstr2.m_pchData );

    return str;
}

CString operator +( const CString& cstr, TCHAR ch )
{   CString str;

    str = cstr;
    str.ConcatInPlace( 1, &ch );

    return str;
}

CString operator +( TCHAR ch, const CString& cstr )
{
    CString str;
    str = ch;
    str.ConcatInPlace( cstr.GetData()->nDataLength, cstr.m_pchData );
    return str;
}

CString operator +( const CString& cstr, LPCTSTR pstr )
{   CString str;

    str = cstr;
    str.ConcatInPlace( CString::SafeStrlen(pstr), pstr );

    return str;
}

CString operator +( LPCTSTR pstr, const CString& cstr )
{   CString str;

    str = pstr;
    str.ConcatInPlace( cstr.GetData()->nDataLength, cstr.m_pchData );

    return str;
}

const CString& CString::operator +=( const CString& cstr )
{   ConcatInPlace( cstr.GetData()->nDataLength, cstr.m_pchData );

    return *this;
}

const CString& CString::operator +=( TCHAR ch )
{   ConcatInPlace( 1, &ch );

    return *this;
}

const CString& CString::operator +=( LPCTSTR pstr )
{
    ConcatInPlace( CString::SafeStrlen(pstr), pstr );

    return *this;
}

void CString::Empty( )
{   if (! IsEmpty() )
    {   Release();
        Init();
    }
}

TCHAR CString::GetAt( int nIndex ) const
{
    ASSERT( nIndex >= 0 && nIndex < GetData()->nDataLength);
    return m_pchData[nIndex];
}

void CString::SetAt( int nIndex, TCHAR ch )
{   ASSERT( nIndex>=0 && nIndex < GetData()->nDataLength);
    //We need to ensure that we aren't modifiying a shared string
    CopyBeforeWrite();
    m_pchData[nIndex] = ch;
}

void CString::Fill( TCHAR ch, int nCount )
{   ASSERT( nCount > 0 );
    AllocBeforeWrite( nCount );
    lmemset( m_pchData, ch, nCount * sizeof(TCHAR) );
}

int CString::Compare( LPCTSTR pstr ) const
{
    return lstrcmp( m_pchData, pstr );//AfxStrCompare( m_pchData, pstr );
}

int CString::CompareNoCase( LPCTSTR pstr ) const
{
    return lstrcmpi( m_pchData, pstr );//AfxStrICompare( m_pchData, pstr );
}

int CString::Collate( LPCTSTR pstr ) const
{
    // Same as strcmp() except for locale specific special characters
	// @todo
    return lstrcmp(m_pchData, pstr);//( strcoll(m_pchData, pstr) );
}

int CString::CollateNoCase( LPCTSTR pstr ) const
{
    // Same as stricmp() except for locale specific special characters
//#ifndef MSVC32
    //return( stricoll(m_pchData, pstr) );
//#else
    return( lstrcmpi(m_pchData, pstr) );
//#endif
}

CString CString::Mid( int nPos ) const
{
    CStringData* pData = GetData();
    if (nPos < 0)
        nPos = 0;
    if (nPos > pData->nDataLength)
        nPos = pData->nDataLength;
    int nLen = pData->nDataLength - nPos;
    return Mid(nPos, nLen);
}

CString CString::Mid( int nPos, int nLen ) const
{
    CStringData* pData = GetData();
    if (nPos < 0)
        nPos = 0;
    if (nPos > pData->nDataLength)
        nPos = pData->nDataLength;
    if (nLen < 0 || nLen > pData->nDataLength )
        nLen = pData->nDataLength - nPos;

    if (nLen == pData->nDataLength )
        return *this;

    CString str;
    AllocCopy( str, nLen, nPos );

    return str;
}

CString CString::Left( int nLen ) const
{   CStringData* pData = GetData();
    if (nLen > pData->nDataLength )
        nLen = pData->nDataLength;

    if (nLen == pData->nDataLength )
        return *this;

    CString str;
    AllocCopy( str, nLen, 0);

    return str;
}

CString CString::Right( int nLen ) const
{   CStringData* pData = GetData();
    if (nLen > pData->nDataLength )
        nLen = pData->nDataLength;

    if (nLen == pData->nDataLength )
        return *this;

    int nPos = pData->nDataLength - nLen;

    CString str;
    AllocCopy( str, nLen, nPos);

    return str;
}

CString CString::SpanIncluding( LPCTSTR pstr ) const
{   CString str;
    CStringData* pData = GetData();

    for (int i=0; lstrchr/*AfxStrChr*/(pstr, m_pchData[i]); i++)
            str += m_pchData[i];

    return str;
}

CString CString::SpanExcluding( LPCTSTR pstr ) const
{   CString str;
    CStringData* pData = GetData();

    for (int i=0; !lstrchr/*AfxStrChr*/(pstr, m_pchData[i]); i++)
            str += m_pchData[i];

    return str;
}

#define AnsiUpperChar(c) ((char)AnsiUpper((LPSTR)(unsigned char)(c)))
#define AnsiLowerChar(c) ((char)AnsiLower((LPSTR)(unsigned char)(c)))

void CString::MakeUpper( )
{   CopyBeforeWrite();
    CStringData* pData = GetData();
    for (int i=0; i<pData->nDataLength; i++)
        m_pchData[i] = AnsiUpperChar( m_pchData[i] );
}

void CString::MakeLower( )
{   CopyBeforeWrite();
    CStringData* pData = GetData();
    for (int i=0; i<pData->nDataLength; i++)
        m_pchData[i] = AnsiLowerChar( m_pchData[i] );
}

void CString::MakeReverse( )
{   TCHAR tch;

    CopyBeforeWrite();
    CStringData* pData = GetData();
    int nCount = pData->nDataLength /2;
    for (int i=0; i < nCount; i++)
    {   tch = m_pchData[pData->nDataLength - (i + 1)];
        m_pchData[pData->nDataLength - (i + 1)] = m_pchData[i];
        m_pchData[i] = tch;
    }
}

int CString::Replace( TCHAR oldChar, TCHAR newChar )
{
    CopyBeforeWrite();
    CStringData* pData = GetData();
    int nCount = 0;

    for (int i=0; i<pData->nDataLength; i++)
    {   if (m_pchData[i] == oldChar)
        {   m_pchData[i] = newChar;
            nCount ++;
        }
    }

    return nCount;
}

int CString::Replace( LPCTSTR poldStr, LPCTSTR pnewStr )
{   int nLen1, nLen2, nCount;

    CopyBeforeWrite();
    nLen1 = CString::SafeStrlen(poldStr);
    nLen2 = CString::SafeStrlen(pnewStr);

    ASSERT(nLen1 != 0);

    nCount = 0;
    if (nLen1==nLen2 && nLen2==1) //One character string
        nCount = Replace( (TCHAR)poldStr[0], (TCHAR)pnewStr[0] );
    else if ( nLen1 == nLen2 ) //the two strings has the same length
    {   int i;
        CStringData* pData = GetData();
        TCHAR FAR*pstr = m_pchData;
        for ( i = 0; i + nLen1 < pData->nDataLength; )
        {   if (AfxStrNCompare( pstr, poldStr, nLen1 ) == 0)
            {   lmemcpy(pstr, pnewStr, nLen2 * sizeof(TCHAR) );
                nCount ++;
                i += nLen1;
                pstr += nLen1;
            }
            else
                pstr ++, i++;
        }
    }
    else
    {   CString strTemp;
        TCHAR FAR *pCurrent = m_pchData;
        TCHAR FAR *pstr;

        while ( *pCurrent != 0 )
        {   pstr = AfxStrStr(pCurrent, poldStr);
            if ( pstr == NULL )
            {   if ( nCount == 0 )
                    return 0;

              strTemp += pCurrent;
              break;
            }
            else
            {   strTemp.ConcatInPlace(pstr - pCurrent, pCurrent);
                strTemp.ConcatInPlace(nLen2, pnewStr);
                pCurrent = pstr + nLen1;
                nCount++;
            }
        }

        *this = strTemp;
    }
    return nCount;
}

int CString::Insert( int nIndex, TCHAR ch )
{   TCHAR pStr[2];
    pStr[0] = ch;
    pStr[1] = 0;
    return Insert( nIndex, (LPCTSTR)pStr );
}

int CString::Insert( int nIndex, LPCTSTR pnewStr )
{   CStringData* pData = GetData();

    ASSERT( nIndex>=0 && nIndex<=pData->nDataLength );
    if ( !pData->IsEmpty() )
    {   int nLen = CString::SafeStrlen( pnewStr );
        int newLen = pData->nDataLength + nLen;
        if ( newLen > pData->nAllocLength )
        {   AllocBuffer( newLen );
            lmemcpy(m_pchData, pData->data(), nIndex * sizeof (TCHAR ) );
            lmemcpy(m_pchData + nIndex, pnewStr, nLen * sizeof( TCHAR ) );
            if (nIndex < pData->nDataLength)
                lmemcpy(m_pchData + nIndex + nLen, pData->data() + nIndex, (pData->nDataLength - nIndex) * sizeof(TCHAR));

            CString::Release( pData );

            return GetData()->nDataLength;
        }
        else //Buffer is sufficiently big to fit the data
        {   CopyBeforeWrite();
            LPTSTR pStr = m_pchData + nIndex;
            if ( nIndex < pData->nDataLength )
              lmemmove(pStr + nLen, pStr, pData->nDataLength - nIndex);
            lmemcpy(pStr, pnewStr, nLen * sizeof(TCHAR) );
            GetData()->nDataLength = newLen;
            m_pchData[newLen] = 0;
            return newLen;
        }
    }

    return 0;
}

int CString::Delete( int nIndex, int nCount)
{   CString str;
    CStringData* pData = GetData();

    ASSERT( nIndex >= 0 && nIndex < pData->nDataLength );
    CopyBeforeWrite();
    if (nIndex == 0 && nCount == pData->nDataLength) //Optimize
        Empty();
    else
    {   lmemmove( m_pchData + nIndex, m_pchData + nIndex + nCount, pData->nDataLength - (nCount + nIndex));
        pData->nDataLength -= nCount;
        m_pchData[pData->nDataLength] = 0;
        FreeExtra();
    }

    return GetData()->nDataLength;
}

int CString::Remove(TCHAR ch)
{   int i, nCount = 0, nLen = 0;
    CStringData* pData = GetData();
    LPTSTR pStr = pData->data();

    AllocBuffer( pData->nDataLength );
    for (i=0; i<pData->nDataLength; i++)
    {   if ( ch == pStr[i] )
            nCount ++;
        else
            m_pchData[nLen++] = pStr[i];
    }
    GetData()->nDataLength = nLen;
    m_pchData[nLen] = '\0';
    CString::Release( pData );

    return nCount;
}

void CString::TrimLeft( )
{
    TCHAR pstr[] = {' ', '\r', '\n', '\t', 0};
    TrimLeft( pstr );
}

void CString::TrimLeft( TCHAR ch )
{
    TCHAR pstr[2];
    pstr[0] = ch;
    pstr[1] = '\0';
    TrimLeft( pstr );
}

void CString::TrimLeft( LPCTSTR pstr )
{
    int i = 0;
    CStringData* pData = GetData();
    while ( AfxStrChr(pstr, m_pchData[i]) ) i++;
    if ( i > 0)
    {   lmemmove(m_pchData, m_pchData + i, (pData->nDataLength - i)*sizeof(TCHAR));
        pData->nDataLength -= i;
        m_pchData[pData->nDataLength] = 0;
        FreeExtra(); //Free extra space if were posible
    }
}

void CString::TrimRight( )
{
    TCHAR pstr[] = {' ', '\r', '\n', '\t', 0};
    TrimRight( pstr );
}

void CString::TrimRight( TCHAR ch )
{
    TCHAR pstr[2];
    pstr[0] = ch;
    pstr[1] = '\0';
    TrimRight( pstr );
}

void CString::TrimRight( LPCTSTR pstr )
{
    CStringData* pData = GetData();
    int i = pData->nDataLength - 1;
    while ( AfxStrChr(pstr, m_pchData[i]) ) i--;

    if ( i < pData->nDataLength - 1)
    {   m_pchData[i+1] = 0;
        pData->nDataLength = AfxStrLen( m_pchData );
        FreeExtra(); //Free extra space if where posible
    }
}

int CString::Find( TCHAR ch, int nPos) const
{   TCHAR FAR * pstr;
    if (nPos < 0 || nPos >= GetData()->nDataLength)
        nPos = 0;
    pstr = AfxStrChr( &m_pchData[nPos+1], ch );
    if ( pstr == NULL )
        return -1;

    return (pstr - m_pchData);
}

int CString::Find( LPCTSTR pstr, int nPos) const
{   TCHAR FAR * pstr_t;
    if (nPos < 0 || nPos >= GetData()->nDataLength)
        nPos = 0;
    pstr_t = AfxStrStr( &m_pchData[nPos+1], pstr );
    if ( pstr_t == NULL )
        return -1;

    return (pstr_t - m_pchData);
}

int CString::ReverseFind( TCHAR ch ) const
{   int i = GetData()->nDataLength - 1;

    while ( m_pchData[i] != ch )
    {   i--;
        if (i < 0 ) break;
    }

    return i;
}

int CString::FindOneOf( LPCTSTR pstr ) const
{   int i = 0, nLen = GetData()->nDataLength;

    while ( !AfxStrChr( pstr, m_pchData[i] ) && i<nLen) i++;

    return i==nLen? -1 : i;
}


LPTSTR CString::GetBuffer( int nLen )
{
    CStringData* pData = GetData();

    if ( pData->IsShared() || pData->IsEmpty() )
    {
        if ( nLen < pData->nAllocLength )
            nLen = pData->nAllocLength;

        AllocBeforeWrite( nLen );
        if ( !pData->IsEmpty() )
            SetData( pData->nDataLength, pData->data() ); 
    }
    else if ( nLen > pData->nAllocLength )
    {
        AllocBuffer( nLen );
        SetData( pData->nDataLength, pData->data() );
        CString::Release( pData );
    }

    return m_pchData;
}

LPTSTR CString::GetBufferSetLength( int nLen )
{
    CStringData* pData = GetData();

    AllocBuffer( nLen );
    if ( !pData->IsEmpty() )
    {
        SetData( nLen, pData->data() );
        CString::Release( pData );
    }

    return m_pchData;
}

void CString::ReleaseBuffer( int nLen )
{
    CStringData* pData = GetData();

    if ( nLen == -1 )
        nLen = CString::SafeStrlen( m_pchData );

    ASSERT( nLen <= pData->nAllocLength );
    CopyBeforeWrite();
    pData->nDataLength = nLen;
    m_pchData[nLen] = 0;
}

void CString::FreeExtra()
{   CStringData* pData = GetData();
    int nExtraSize = pData->nAllocLength - pData->nDataLength;

    if ( nExtraSize >= DELTA )
    {   AllocBuffer( pData->nDataLength );
        SetData( pData->nDataLength, pData->data() );
        CString::Release( pData );
    }
}

LPTSTR CString::LockBuffer( )
{
    CStringData* pData = GetData();

    if ( pData->IsLocked() )
        return m_pchData;

    if ( pData->IsShared() )
    {
        CopyBeforeWrite();
        GetData()->Lock();
        pData->DecRef();
    }
    else
        pData->Lock();

    return m_pchData;
}

void CString::UnlockBuffer( )
{
    CStringData* pData = GetData();

    if ( pData->IsLocked() )
        pData->UnLock();
}

BSTR CString::AllocSysString ( ) const
{
    CStringData* pData = GetData();
    BSTR bstr;

    bstr = ::SysAllocStringLen(NULL, pData->nDataLength);
    //@todo mbstowcs(bstr, m_pchData, pData->nDataLength);

    return bstr;
}

BSTR CString::SetSysString( BSTR* pbstr ) const
{
    CStringData* pData = GetData();

    ::SysReAllocStringLen(pbstr, NULL, pData->nDataLength);
    //@todo mbstowcs(*pbstr, m_pchData, pData->nDataLength);

    return *pbstr;
}

BOOL CString::LoadString( UINT nID )
{
    int iRet;
    // Get a big buffer, load the string,
    // then release the extra buffer using ReleaseBuffer()
    GetBuffer(MAX_LOAD_STRING);
    iRet = ::LoadString(NULL, nID, m_pchData, MAX_LOAD_STRING);
    ReleaseBuffer();
    return( iRet != 0 );
}

void CString::AnsiToOem( )
{
    CStringData* pData = GetData();
    AnsiToOemBuff(m_pchData, m_pchData, pData->nDataLength);
}

void CString::OemToAnsi( )
{
    CStringData* pData = GetData();
    OemToAnsiBuff(m_pchData, m_pchData, pData->nDataLength);
}
