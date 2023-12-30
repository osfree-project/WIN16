/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CSize.CPP
  Version    : 0.50
  Author(s)  : Carsten Breuer
 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  Implementation of the class CSize.

 --[ Developers ]--------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

 --[ History ] ----------------------------------------------------------

  0.10: Original file by Carsten Breuer. First beta version.
  0.20: Finished by Carsten Breuer

mm-dd-yy  who  ver   What
10-05-03  wdh  0.30  Changed to use windef.h instead of OfcTypes.h. Changed
    CSize() to init to (0,0) (like CPoint), fixed bug in CSize(dwSize).
10-12-03  wdh  0.40  Fixed bug in operator + and operator -. Added operator =
05-30-04  wdh  0.50  Added return( *this ) to operator =(CPoint).
    Exchanged many includes for one afxwin.h. Changed history to be like
    other source files.

 --[ How to compile ]----------------------------------------------------

  This file was developed under DJGPP and Rhide. If you are familiar with
  Borland C++ 3.1, you will feel like at home ;-)).
  Both tools are free software. Downloads at: http://www.delorie.com/djgpp


 --[ Where to get help/information ]-------------------------------------

  The author              : Carsten.Breuer@breuer-software.de

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
  Copyright (c) 2000 Carsten Breuer
 ************************************************************************/

#include <afxwin.h>


CSize::CSize ()
{
   cx = 0;
   cy = 0;
}

CSize::CSize (int x, int y)
{
   cx = x;
   cy = y;
}

CSize::CSize (POINT pt)
{
   cx = pt.x;
   cy = pt.y;
}
CSize::CSize (SIZE sz)
{
   cx = sz.cx;
   cy = sz.cy;
}

CSize::CSize (DWORD dwSize)
{
   cx = dwSize & 0x0000FFFF;
   cy = (dwSize >> 16) & 0x0000FFFF;
}

// overloaded operators
BOOL CSize::operator == (SIZE Size) const
{
   if ((cx == Size.cx) && (cy == Size.cy))
   {
      return TRUE;
   }
   return FALSE;
}

BOOL CSize::operator != (SIZE Size) const
{
   if ((cx != Size.cx) && (cy != Size.cy))
   {
      return TRUE;
   }
   return FALSE;
}

void CSize::operator += (SIZE Size)
{
   cx += Size.cx;
   cy += Size.cy;
}

void CSize::operator -= (SIZE Size)
{
  cx -= Size.cx;
  cy -= Size.cy;
}

CSize CSize::operator + (SIZE  Size)  const
{
   CSize rSize (*this);
   rSize += Size;
   return rSize;
}

CPoint CSize::operator + (POINT Point) const
{
   CPoint rPoint (*this);
   rPoint.x += Point.x;
   rPoint.y += Point.y;
   return rPoint;
}

CRect CSize::operator + (const RECT *pRect) const
{
   CRect Rect;
   Rect.CopyRect ((RECT*)pRect); // CopyRect checks the pointer
   Rect += *this;
   return Rect;
}

CSize CSize::operator -  (SIZE  Size) const
{
   CSize rSize (*((SIZE*)this));
   rSize -= Size;
   return rSize;
}

CPoint CSize::operator - (POINT Point) const
{
   CPoint rPoint (*this);
   rPoint.x -= Point.x;
   rPoint.y -= Point.y;
   return rPoint;
}

CRect CSize::operator - (const RECT *pRect) const
{
   CRect Rect;
   Rect.CopyRect ((RECT*)pRect);
   Rect -= *this;
   return Rect;
}

CSize CSize::operator - () const
{
   CSize Size (*this);
   Size.cx = 0 - Size.cx;
   Size.cy = 0 - Size.cy;
   return Size;
}

// Without this, gcc creates bad code on SIZE assignment
const CSize& CSize::operator = (SIZE siz)
{
   cx = siz.cx;
   cy = siz.cy;
   return( *this );
}

// Without this, gcc creates bad code on SIZE assignment
const CSize& CSize::operator = (POINT pt)
{
   cx = pt.x;
   cy = pt.y;
   return( *this );
}
