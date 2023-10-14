/************************************************************************
   T h e   O p e n   F o u n d a t i o n   C l a s s e s
 ------------------------------------------------------------------------
   Filename   : CException.h
   Version    : 0.20
   Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file declares the class CException, a few of the more
  common exceptions that do not really belong anywhere else,
  and a few related macros.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
10-05-03  0.10  wdh  Created.
05-30-04  0.20  wdh  Added throws for Memory, NotSupported and User.

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
 Copyright (c) 2000-04 by The Open Foundation Classes
 Copyright (c) 2003-04 by William D. Herndon
/************************************************************************/

#ifndef CEXCEPTION_H
#define CEXCEPTION_H

#define TRY try
#define CATCH(typ,e) catch(typ* e)
#define AND_CATCH(typ,e) catch(typ* e)
#define CATCH_ALL(e) catch(CException* e)
#define AND_CATCH_ALL(e) catch (CException* e)
#define END_CATCH_ALL /**/
#define THROW(e) throw(e)

class CException : public CObject {
	DECLARE_DYNAMIC(CException);
	virtual BOOL GetErrorMessage(LPTSTR pszErr,UINT nMaxErr,
		UINT* pnHelpID = NULL);
};

class CMemoryException : public CException {
	DECLARE_DYNAMIC(CMemoryException);
	CMemoryException() {};
};

class CNotSupportedException : public CException {
	DECLARE_DYNAMIC(CNotSupportedException)
	CNotSupportedException() {};
};

class CUserException : public CException {
	DECLARE_DYNAMIC(CUserException)
	CUserException() {};
};

void AfxThrowMemoryException();
void AfxThrowNotSupportedException();
void AfxThrowUserException();

#endif // CEXCEPTION_H
