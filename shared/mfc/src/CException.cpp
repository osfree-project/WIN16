/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CException.cpp
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file implements CException and some of the more common
  exceptions and exception functions (AfxThrow*) that really do
  not belong anywhere else. -wdh

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
10-05-03  0.10  wdh  Created.

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
 ************************************************************************/

#include <afxwin.h>
//#include <errno.h>

	CException g_e;
	CMemoryException g_eMemory;
	CNotSupportedException g_eNotSupported;
	CUserException g_eUserException;

IMPLEMENT_DYNAMIC(CException, CObject)

IMPLEMENT_DYNAMIC(CMemoryException, CException)

IMPLEMENT_DYNAMIC(CNotSupportedException, CException)

IMPLEMENT_DYNAMIC(CUserException, CException)

void AfxThrowMemoryException()
{
	THROW(&g_eMemory);
}

void AfxThrowNotSupportedException()
{
	THROW(&g_eNotSupported);
}

void AfxThrowUserException()
{
	THROW(&g_eUserException);
}

BOOL CException::GetErrorMessage(LPTSTR pszErr,UINT nMaxErr,UINT* pnHelpID)
{
	return( FALSE );
}
