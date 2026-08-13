/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CString_inline.h
  Version    : 0.1
  Author(s)  : Ivan Deras, William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
10-05-03  0.10  wdh  Split inline functions out of CString.cpp, creating
                     this file: the compiler cannot make them inline if
                     they are not included. -wdh

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
  Copyright (c) 2000-03 Open Foundation Classes
  Copyright (c) 2000 Ivan Deras
 ************************************************************************/


inline int CString::GetLength() const
{
  return GetData()->nDataLength;
}

inline CStringData* CString::GetData() const
{
    return &(((CStringData*)m_pchData)[-1]);
}

inline BOOL CString::IsEmpty( ) const
{
    return GetData()->IsEmpty();
}

//Comparison operators
inline BOOL operator ==( const CString& cstr1, const CString& cstr2 )
{
    return cstr1.Compare( cstr2 ) == 0;
}

inline BOOL operator ==( const CString& cstr1, LPCTSTR pstr2 )
{
    return cstr1.Compare( pstr2 ) == 0;
}

inline BOOL operator ==( LPCTSTR pstr1, const CString& cstr2 )
{
    return AfxStrCompare( cstr2, pstr1 ) == 0;
}

inline BOOL operator !=( const CString& cstr1, const CString& cstr2 )
{
    return cstr1.Compare( cstr2 ) != 0;
}

inline BOOL operator !=( const CString& cstr1, LPCTSTR pstr2 )
{
    return AfxStrCompare( cstr1, pstr2 ) != 0;
}

inline BOOL operator != ( LPCTSTR pstr1, const CString& cstr2 )
{
    return AfxStrCompare( cstr2, pstr1 ) != 0;
}

inline BOOL operator < ( const CString& cstr1, const CString& cstr2 )
{
    return cstr1.Compare( cstr2 ) < 0;
}

inline BOOL operator < ( const CString& cstr1, LPCTSTR pstr2 )
{
    return AfxStrCompare( cstr1, pstr2 ) < 0;
}

inline BOOL operator < ( LPCTSTR pstr1, const CString& cstr2 )
{
    return AfxStrCompare( cstr2, pstr1 ) > 0;
}

inline BOOL operator > ( const CString& cstr1, const CString& cstr2 )
{
    return cstr1.Compare( cstr2 ) > 0;
}

inline BOOL operator > ( const CString& cstr1, LPCTSTR pstr2 )
{
    return AfxStrCompare( cstr1, pstr2 ) > 0;
}

inline BOOL operator >( LPCTSTR pstr1, const CString& cstr2 )
{
    return AfxStrCompare( cstr2, pstr1 ) < 0;
}

inline BOOL operator <=( const CString& cstr1, const CString& cstr2 )
{
    return cstr1.Compare( cstr2 ) <= 0;
}

inline BOOL operator <=( const CString& cstr1, LPCTSTR pstr2 )
{
    return AfxStrCompare( cstr1, pstr2 ) <= 0;
}

inline BOOL operator <=( LPCTSTR pstr1, const CString& cstr2 )
{
    return AfxStrCompare( cstr2, pstr1 ) >= 0;
}

inline BOOL operator >=( const CString& cstr1, const CString& cstr2 )
{
    return cstr1.Compare( cstr2 ) >= 0;
}

inline BOOL operator >=( const CString& cstr1, LPCTSTR pstr2 )
{
    return AfxStrCompare( cstr1, pstr2 ) >= 0;
}

inline BOOL operator >=( LPCTSTR pstr1, const CString& cstr2 )
{
    return AfxStrCompare( cstr2,pstr1 ) <= 0;
}
