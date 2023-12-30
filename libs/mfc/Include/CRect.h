/************************************************************************
   T h e   O p e n   F o u n d a t i o n   C l a s s e s
 ------------------------------------------------------------------------
   Filename   : CRect.h
   Author(s)  : Carsten Breuer

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
??-??-00  0.10  cb   Created.
10-05-03  0.20  wdh  Added this history, changed to use windef.h
                     instead of OfcTypes.h.
10-12-03  0.30  wdh  Added const to some declarations that needed it.

 ------------------------------------------------------------------------
 Copyright (c) 2000-03 by The Open Foundation Classes
 Copyright (c) 2000 by Carsten Breuer
 ************************************************************************/

#ifndef CRECT_H
#define CRECT_H

#include <windows.h>

class CSize;
class CPoint;
class CRect;

typedef const RECT FAR* LPCRECT;       // far pointer to read/only RECT

class CRect : public tagRECT
{
   public:
   CRect ();
   CRect (long Left, long Top, long Right, long Bottom);
   CRect (const RECT& rRect);
   CRect (CRect *pRect);
   CRect (POINT pt, SIZE sz);
   CRect (POINT TopLeft, POINT BottomRight);

   int     Width       ();
   int     Height      ();
   CSize   Size        ();
   CPoint  TopLeft     ();
   CPoint  BottomRight ();
   CPoint  CenterPoint ();
   BOOL    IsRectEmpty ();
   BOOL    IsRectNull  ();
   BOOL    PtInRect    (POINT Pt);
   //BOOL    RectInRect  (CRect& Rect);       // This is a OFC enhancement
   //BOOL    RectInRect  (const RECT* pRect); // This is a OFC enhancement
   void    SetRect     (int left, int top, int right, int bottom);
   void    SetRect     (const RECT* pRect); // This is a OFC enhancement
   void    SetRectEmpty(void);
   void    CopyRect    (const RECT* pRect);
   BOOL    EqualRect   (const RECT* pRect);
   void    InflateRect (int x, int y);
   void    InflateRect (SIZE Size);
   void    InflateRect (RECT *pRect);
   void    InflateRect (int left, int top, int right, int bottom);
   void    DeflateRect (int x, int y);
   void    DeflateRect (SIZE Size);
   void    DeflateRect (RECT *pRect);
   void    DeflateRect (int left, int top, int right, int bottom);
   void    NormalizeRect ();
           operator LPCRECT ();
           operator LPRECT  ();
   void    operator =  (const RECT& Rect);
   BOOL    operator == (const RECT& Rect);
   BOOL    operator != (const RECT& Rect);
   void    operator += (POINT Pt);
   void    operator += (SIZE Size);
   void    operator += (RECT *pRect);
   void    operator -= (POINT Pt);
   void    operator -= (SIZE Size);
   void    operator -= (RECT *pRect);
   void    operator &= (const RECT& Rect);
   void    operator |= (const RECT& Rect);
   CRect   operator +  (POINT point) const;
   CRect   operator +  (SIZE  Size ) const;
   CRect   operator +  (RECT *pRect) const;
   CRect   operator -  (POINT point) const;
   CRect   operator -  (SIZE  Size ) const;
   CRect   operator -  (RECT *pRect) const;
   CRect   operator &  (RECT&  Rect) const;
   CRect   operator |  (RECT&  Rect) const;
   protected:
   private:
};

#endif // CRECT_H
