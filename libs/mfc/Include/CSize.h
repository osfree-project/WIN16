/************************************************************************
   T h e   O p e n   F o u n d a t i o n   C l a s s e s
 ------------------------------------------------------------------------
   Filename   : CSize.h
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
10-12-03  0.30  wdh  Added operator =.

 ------------------------------------------------------------------------
 Copyright (c) 2000-03 by The Open Foundation Classes
 Copyright (c) 2000 by Carsten Breuer
 ************************************************************************/

#ifndef CSIZE_H
#define CSIZE_H

#include <windows.h>
#include <CPoint.h>

class CPoint;
class CRect;

class CSize : public tagSIZE
{
   public:
      CSize ();
      CSize (int x, int y);
      CSize (POINT pt);
      CSize (SIZE sz);
      CSize (DWORD dwSize);

      // Operators:
      BOOL    operator == (SIZE  Size) const;
      BOOL    operator != (SIZE  Size) const;
      void    operator += (SIZE  Size);
      void    operator -= (SIZE  Size);
      CSize   operator +  (SIZE  Size)  const;
      CPoint  operator +  (POINT Point) const;
      CRect   operator +  (const RECT *pRect) const;
      CSize   operator -  (SIZE  Size)  const;
      CPoint  operator -  (POINT Point) const;
      CRect   operator -  (const RECT *pRect) const;
      CSize   operator - () const;

      // Without these, gcc creates bad code, but does not crash
      // like it does with the corresponding POINT assignment
      const CSize& operator = (SIZE siz);
      const CSize& operator = (POINT pt);

   protected:

   private:
};

#endif // CSIZE_H
