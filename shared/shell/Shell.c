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

/*! @brief Library initialisation: create/load REG.DAT.

    Allocates the registry global blocks and either loads REG.DAT from
    the Windows directory or recreates it from the embedded resource
    (type 100, id 100).

    @param[in] hInstance     Instance handle of the library.
    @param[in] wDataSegment  Data segment selector.
    @param[in] wHeapSize     Local heap size.
    @param[in] lpszCmdLine   Command line.

    @return TRUE on success, FALSE otherwise.

    @retval TRUE  The library initialised successfully, or was already
                  initialised.
    @retval FALSE Initialisation failed (missing strings, failed
                  allocation, or failed file/resource access).
*/
BOOL WINAPI LibMain( HINSTANCE hInstance, WORD wDataSegment, WORD wHeapSize, LPSTR lpszCmdLine )
#pragma on (unreferenced);
{
	if (!Globals.hInstance)
	{
		char szPath[MAX_PATH];
		char szFilename[MAX_PATH];
		HFILE hfRegistry;
		HRSRC hrsrcRegistry;
		HGLOBAL hRegistry;
		DWORD dwRegistrySize;
		LPSTR lpRegistry;

		Globals.hInstance=hInstance;

		if ((GetWindowsDirectory(szPath, sizeof(szPath))) &
		    (LoadString(Globals.hInstance, IDS_REGISTRY, szFilename, sizeof(szFilename))))
		{
			if (szPath[lstrlen(szPath)-1]!='\\') lstrcat(szPath, "\\");
			lstrcat(szPath, szFilename);

			if ((Globals.EntryTable=GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE | GMEM_LOWER, 0)) &
			    (Globals.StringTable=GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE | GMEM_LOWER, 0)) &
			    (Globals.Registry=GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE | GMEM_LOWER, 0)))
			{
				if ((hfRegistry=_lopen(szPath, 0))==HFILE_ERROR)
				{
					// Custom resource type 100 contains empty REG.DAT
					if (hrsrcRegistry=FindResource(Globals.hInstance, MAKEINTRESOURCE(100), MAKEINTRESOURCE(100)))
					{
						if (hRegistry=LoadResource(Globals.hInstance, hrsrcRegistry))
						{
							if (dwRegistrySize=SizeofResource(Globals.hInstance, hrsrcRegistry))
							{
								if (lpRegistry=LockResource(hrsrcRegistry))
								{
									if ((hfRegistry=_lcreat(szPath, 0))!=HFILE_ERROR)
									{
										if (_lwrite(hfRegistry, lpRegistry, dwRegistrySize)!=HFILE_ERROR)
										{
											_lclose(hfRegistry);
											UnlockResource(hRegistry);
											FreeResource(hRegistry);
											InitReg;
											return TRUE;
										}
										_lclose(hfRegistry);
									}
									UnlockResource(hRegistry);
								}
							}
							FreeResource(hRegistry);
						}
					}
				} else {
					_lclose(hfRegistry);
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	return TRUE;
}

#pragma off (unreferenced);

/*! @brief Library termination: flush the in-memory registry to REG.DAT.

    @param[in] bSystemExit Non-zero when the system is shutting down.

    @return Always 1.

    @retval 1 Always returned.
*/
int WINAPI WEP( int bSystemExit )
#pragma on (unreferenced);
{
	char szPath[MAX_PATH];
	char szFilename[MAX_PATH];
	HFILE hfRegistry;
	LPSTR lpRegistry;

	// Save regfile here
	if ((GetWindowsDirectory(szPath, sizeof(szPath))) &
	    (LoadString(Globals.hInstance, IDS_REGISTRY, szFilename, sizeof(szFilename))))
	{
		if (szPath[lstrlen(szPath)-1]!='\\') lstrcat(szPath, "\\");
		lstrcat(szPath, szFilename);
		
		// save header
		lpRegistry=GlobalLock(Globals.Registry);
		if ((hfRegistry=_lcreat(szPath, 0))!=HFILE_ERROR)
		{
			if (_lwrite(hfRegistry, lpRegistry, GlobalSize(Globals.Registry))!=HFILE_ERROR)
			{
				_lclose(hfRegistry);
			}
			_lclose(hfRegistry);
		}
		GlobalUnlock(Globals.Registry);
		// save table
		lpRegistry=GlobalLock(Globals.EntryTable);
		if ((hfRegistry=_lcreat(szPath, 0))!=HFILE_ERROR)
		{
			if (_lwrite(hfRegistry, lpRegistry, GlobalSize(Globals.EntryTable))!=HFILE_ERROR)
			{
				_lclose(hfRegistry);
			}
			_lclose(hfRegistry);
		}
		GlobalUnlock(Globals.EntryTable);
		// save strings
		lpRegistry=GlobalLock(Globals.StringTable);
		if ((hfRegistry=_lcreat(szPath, 0))!=HFILE_ERROR)
		{
			if (_lwrite(hfRegistry, lpRegistry, GlobalSize(Globals.StringTable))!=HFILE_ERROR)
			{
				_lclose(hfRegistry);
			}
			_lclose(hfRegistry);
		}
		GlobalUnlock(Globals.StringTable);
	}
	return( 1 );
}
