/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CRect.CPP
  Version    : 0.70
  Author(s)  : Carsten Breuer

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  Implementation of the class CRect.

 --[ Developers ]--------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

 --[ History ] ----------------------------------------------------------

  0.10: Original file by Carsten Breuer. First beta version.
  0.20: Changes & enhancements by Geurt Vos.
  0.30: Finished. C. Breuer

mm-dd-yy  who  ver   What
10-05-03  wdh  0.40  Changed to use windef.h instead of OfcTypes.h.
10-12-03  wdh  0.50  Changed some things to be const. Fixed PtInRect() to be
    MS compatible. Fixed infinite recursion in RectInRect().
10-26-03  wdh  0.60  Fixed IsRectEmpty(), and operators "&=" and "|="
05-30-04  wdh  0.70  Exchanged many includes for afxwin.h. Changed history to
    be like other source files.

 --[ To Do ]-------------------------------------------------------------

  Some methods should be inline.

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
  Copyright (c) 2000-03 Open Foundation Classes
  Copyright (c) 2000 Carsten Breuer
 ************************************************************************/

#include <afxwin.h>

CRect::CRect ()
{
   // This init would not be done in MFC
   left   = 0;
   top    = 0;
   right  = 0;
   bottom = 0;
}

CRect::CRect (long Left, long Top, long Right, long Bottom)
{
   left   = Left;
   top    = Top;
   right  = Right;
   bottom = Bottom;
}

CRect::CRect (const RECT& Rect)
{
   CopyRect (&Rect);
}

CRect::CRect (CRect *pRect)
{
   if (pRect == NULL)
   {
      left = 0;
      top = 0;
      right = 0;
      bottom = 0;
   }
   else
   {
      CopyRect (pRect);
   }
}

CRect::CRect (POINT pt, SIZE sz)
{
   left   = pt.x;
   top    = pt.y;
   right  = left + sz.cx;
   bottom = top  + sz.cy;
}

CRect::CRect (POINT TopLeft, POINT BottomRight)
{
   left   = TopLeft.x;
   top    = TopLeft.y;
   right  = BottomRight.x;
   bottom = BottomRight.y;
}

int CRect::Width  ()
{
   return (right - left);
}

int CRect::Height ()
{
   return (bottom - top);
}

CSize CRect::Size ()
{
   return CSize (right - left,bottom - top);
}

CPoint CRect::TopLeft ()
{
   return CPoint (left,top);
}
CPoint CRect::BottomRight ()
{
   return CPoint (right,bottom);
}
CPoint CRect::CenterPoint ()
{
   return CPoint (left + ((right  - left) / 2),top  + ((bottom - top ) / 2));
} 

BOOL CRect::IsRectEmpty ()
{
   if ((left >= right) && (top >= bottom))
   {
      return TRUE;
   }
   return FALSE;
}

BOOL CRect::IsRectNull    ()
{
   if ( (left   == 0) &&
        (top    == 0) &&
        (right  == 0) &&
        (bottom == 0) )
   {
      return TRUE;
   }
   return FALSE;
}

BOOL CRect::PtInRect (POINT Pt)
{
   // Note: for MS compatibility, right/bottom must be strictly less -wdh
   if ( (Pt.x >= left  ) &&
        (Pt.x < right ) &&
        (Pt.y >= top   ) &&
        (Pt.y < bottom) )
   {
      return TRUE;
   }
   return FALSE;
}

#if 0
// Is it better to put all ofc enhancemnts in a seperated
// class (e. g. CExRect)? We have to check it.
 // This is a OFC enhancement
BOOL CRect::RectInRect  (const RECT* pRect)
{
   // we are not ms, so we have to check a pointer.
   // the rule is: a pointer is valid or NULL!
   if (pRect == NULL)
      return FALSE;
   // Fixed: previously did endless recursion here. -wdh
   return( (pRect->left >= left) && (pRect->top >= top) &&
      (pRect->right <= right) && (pRect->bottom <= bottom) );
}

BOOL CRect::RectInRect (CRect& Rect)
{
   // Fixed: previously this did a check for TopLeft() and BottomRight()
   // being in the rectangle. This is not correct, because MS defines
   // "in" as being greater than or equal to top/left, but *strictly* less
   // than bottom/right. For rectangles we drop the strictly:
   // equal rectangles should contain each other.
   return( RectInRect((const RECT*)Rect) );
}
#endif

void CRect::SetRect (int nLeft, int nTop, int nRight, int nBottom)
{
   left   = nLeft;
   top    = nTop;
   right  = nRight;
   bottom = nBottom;
}
void CRect::SetRect (const RECT* pRect) // This is a OFC enhancement
{
   CopyRect (pRect); // CopyRect checks, if the pointer is NULL
}
void CRect::SetRectEmpty(void)
{
   SetRect (0,0,0,0);
}
void CRect::CopyRect (const RECT *pRect)
{
   if (pRect == NULL)
   {
      return;
   }
   left   = pRect->left;
   top    = pRect->top;
   right  = pRect->right;
   bottom = pRect->bottom;
}

BOOL CRect::EqualRect (const RECT* pRect)
{
   if (pRect == NULL)
   {
      return FALSE;
   }
   if ((left   == pRect->left  ) &&
       (top    == pRect->top   ) &&
       (right  == pRect->right ) &&
       (bottom == pRect->bottom) )
   {
      return TRUE;
   }
   return FALSE;
}

void CRect::InflateRect (int x, int y)
{
   left   -= x;
   right  += x;
   top    -= y;
   bottom += y;
}
void CRect::InflateRect (SIZE Size)
{
   InflateRect (Size.cx, Size.cy);
}
void CRect::InflateRect (RECT *pRect)
{
   if (pRect == NULL)
   {
      return;
   }
   InflateRect (pRect->left, pRect->top, pRect->right, pRect->bottom);
}
void CRect::InflateRect (int nLeft, int nTop, int nRight, int nBottom)
{
   left   -= nLeft;
   right  += nRight;
   top    -= nTop;
   bottom += nBottom;
}
void CRect::DeflateRect (int x, int y)
{
   left   += x;
   right  -= x;
   top    += y;
   bottom -= y;
}
void CRect::DeflateRect (SIZE Size)
{
   DeflateRect (Size.cx, Size.cy);
}
void CRect::DeflateRect (RECT *pRect)
{
   if (pRect == NULL)
   {
      return;
   }
   DeflateRect (pRect->left, pRect->top, pRect->right, pRect->bottom);
}
void CRect::DeflateRect (int nLeft, int nTop, int nRight, int nBottom)
{
   left   += nLeft;
   right  -= nRight;
   top    += nTop;
   bottom -= nBottom;
}
void CRect::NormalizeRect ()
{
   long temp;
   if (left > right)
   {
       temp  = left;
       left  = right;
       right = temp;
   }
   if (top > bottom)
   {
       temp   = top;
       top    = bottom;
       bottom = temp;
   }
}

void CRect::operator = (const RECT& Rect)
{
   CopyRect ((RECT *)&Rect);
}


BOOL CRect::operator == (const RECT& Rect)
{
   return EqualRect ((RECT*)&Rect);
}
BOOL CRect::operator != (const RECT& Rect)
{
   return EqualRect ((RECT*)&Rect)?FALSE:TRUE;
}

void CRect::operator += (POINT Pt)
{
   left   += Pt.x;
   top    += Pt.y;
   right  += Pt.x;
   bottom += Pt.y;
}

void CRect::operator += (SIZE Size)
{
   left   += Size.cx;
   top    += Size.cy;
   right  += Size.cx;
   bottom += Size.cy;
}
void CRect::operator += (RECT *pRect)
{
   InflateRect (pRect); // InvlateRect checks the pointer for NULL!
}

void CRect::operator -= (POINT Pt)
{
   left   -= Pt.x;
   top    -= Pt.y;
   right  -= Pt.x;
   bottom -= Pt.y;
}


void CRect::operator -= (SIZE Size)
{
   left   -= Size.cx;
   top    -= Size.cy;
   right  -= Size.cx;
   bottom -= Size.cy;
}
void CRect::operator -= (RECT *pRect)
{
   DeflateRect (pRect); // DelateRect checks the pointer for NULL!
}
void CRect::operator &= (const RECT& Rect)
{
   // Much simplified and corrected -wdh
   // Take the innermost dimension
   if (left < Rect.left)
      left = Rect.left;
   if (top < Rect.top)
      top = Rect.top;
   if (right > Rect.right)
      right = Rect.right;
   if (bottom > Rect.bottom)
      bottom = Rect.bottom;
   // Test for an empty rectangle: if so, set to empty (MS compatible)
   if (left >= right || top >= bottom)
      SetRectEmpty();
}

void CRect::operator |= (const RECT& Rect)
{
   // Much simplified and corrected -wdh
   CRect rcCopy(Rect);
   // To be MS compatible, empty rectangles must be ignored
   if ( rcCopy.IsRectEmpty() )
      return;
   if ( IsRectEmpty() ) {
      CopyRect(&Rect);
      return;
   }
   // Take the outermost dimension
   if (left > Rect.left)
      left = Rect.left;
   if (top > Rect.top)
      top = Rect.top;
   if (right < Rect.right)
      right = Rect.right;
   if (bottom < Rect.bottom)
      bottom = Rect.bottom;
}

CRect CRect::operator +  (POINT point) const
{
   CRect Rect ((CRect*)this);
   Rect += point;
   return Rect;
}

CRect CRect::operator +  (SIZE Size  ) const
{
   CRect Rect ((CRect*)this);
   Rect += Size;
   return Rect;
}

CRect CRect::operator +  (RECT *pRect) const
{
   CRect Rect ((CRect*)this);
   Rect.InflateRect (pRect); // InvlateRect checks the pointer
   return Rect;
}

CRect CRect::operator -  (POINT point) const
{
   CRect Rect ((CRect*)this);
   Rect -= point;
   return Rect;
}

CRect CRect::operator -  (SIZE Size  ) const
{
   CRect Rect ((CRect*)this);
   Rect -= Size;
   return Rect;
}

CRect CRect::operator -  (RECT *pRect) const
{
   CRect Rect ((CRect*)this);
   Rect.DeflateRect (pRect); // DevlateRect checks the pointer
   return Rect;
}
CRect CRect::operator &  (RECT&  Rect) const
{
   CRect nRect ((CRect*)this);
   nRect &= Rect;
   return nRect;
}
CRect CRect::operator | (RECT&  Rect) const
{
   CRect nRect ((CRect*)this);
   nRect |= Rect;
   return nRect;
}

CRect::operator LPCRECT()
{
   return this;
}

CRect::operator LPRECT  ()
{
   return this;
}

