/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : Debug.cpp
  Version    : 0.10
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  Provide some debugging functions (DevCpp is not yet great on debugging)

 --[ Developers ]--------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

 --[ History ] ----------------------------------------------------------

mm-dd-yy  ver   who
10-05-03  0.10  wdh
  Created.

 --[ To Do ]-------------------------------------------------------------

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
  Copyright (c) 2000 Open Foundation Classes
  Copyright (c) 2003 William D. Herndon
 ************************************************************************/
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <windows.h>

void dbg_vprintf(LPCSTR pszForm,va_list arglist)
{
	char szMsg[512];
	wvsprintf(szMsg, pszForm, arglist);
        ::MessageBox(GetActiveWindow(), szMsg, "Debug", MB_OK);
}

void dbg_printf(LPCSTR pszForm,...)
{
	va_list arglist;
	va_start(arglist, pszForm);
	dbg_vprintf(pszForm, arglist);
	va_end(arglist);
}
