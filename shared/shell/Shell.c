/*
 * 				Shell Library Functions
 *
 * Copyright 1993, 1994, 1995 Alexandre Julliard
 * Copyright 1996 Martin Von Loewis
 * Copyright 1997 Willows Software, Inc. 
 * Copyright 1997 Alex Korobka
 * Copyright 1998 Marcus Meissner
 * Copyright 1998 Turchanov Sergey
 * Copyright 2000 Juergen Schmied
 * Copyright 2002 Eric Pouech
 * Copyright 2007 Henri Verbeet
 * Copyright 2009 Vincent Povirk for CodeWeavers
 * Copyright 2016 Dmitry Timoshkov
 * Copyright 2023 Yuri Prokushev
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*! @file Shell.c
    @brief DLL entry point, LibMain() and WEP() for SHELL.DLL.

    All REG.DAT file I/O lives in Registry.c. Shell.c only tracks the
    DLL lifecycle; registry loading is lazy (first RegXxx call) and
    registry saving happens when the last handle is closed or from WEP.
    See Raymond Chen, "How did registry keys work in 16-bit Windows?",
    The Old New Thing, 2008-01-17.
*/

#include "Shell.h"


/***********************************************************************
 * DllEntryPoint [SHELL.101]
 *
 * Initialization code for shell.dll. Automatically loads the
 * 32-bit shell32.dll to allow thunking up to 32-bit code.
 *
 * RETURNS
 *  Success: TRUE. Initialization completed successfully.
 *  Failure: FALSE.
 */

/*! @brief DLL entry point for SHELL.DLL.

    @param[in] Reason    Reason code for the call.
    @param[in] hInst     Instance handle of the library.
    @param[in] ds        Data segment selector.
    @param[in] HeapSize  Local heap size.
    @param[in] res1      Reserved.
    @param[in] res2      Reserved.

    @return TRUE on success, FALSE on failure.

    @retval TRUE Always returned by this stub implementation.
*/
BOOL WINAPI DllEntryPoint(DWORD Reason, HINSTANCE hInst,
				WORD ds, WORD HeapSize, DWORD res1, WORD res2)
{
    return TRUE;
}


#pragma off (unreferenced);

/*! @brief Library initialisation.

    Source: Raymond Chen, 2008-01-17 ("The first program to call
    RegOpenKey caused the registry to be loaded"). LibMain therefore
    does not load REG.DAT; it only remembers the instance handle. The
    first RegXxx call triggers InitReg, which calls EnsureLoaded in
    Registry.c.

    @param[in] hInstance     Instance handle of the library.
    @param[in] wDataSegment  Data segment selector.
    @param[in] wHeapSize     Local heap size.
    @param[in] lpszCmdLine   Command line.

    @return TRUE on success, FALSE otherwise.

    @retval TRUE  The library initialised successfully, or was already
                  initialised.
    @retval FALSE Initialisation failed.
*/
BOOL WINAPI LibMain( HINSTANCE hInstance, WORD wDataSegment, WORD wHeapSize, LPSTR lpszCmdLine )
#pragma on (unreferenced);
{
	if (Globals.hInstance)
		return TRUE;

	Globals.hInstance = hInstance;
	return TRUE;
}

#pragma off (unreferenced);

/*! @brief Library termination: flush any remaining registry state.

    Source: Raymond Chen, 2008-01-17. RegSaveAndUnload normally runs
    when the open-key counter reaches zero; WEP runs it again as a
    safety net in case the application leaked open keys.

    @param[in] bSystemExit Non-zero when the system is shutting down.

    @return Always 1.

    @retval 1 Always returned.
*/
int WINAPI WEP( int bSystemExit )
#pragma on (unreferenced);
{
	RegSaveAndUnload();
	return 1;
}
