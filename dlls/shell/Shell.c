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
BOOL WINAPI DllEntryPoint(DWORD Reason, HINSTANCE hInst,
				WORD ds, WORD HeapSize, DWORD res1, WORD res2)
{
    return TRUE;
}


#pragma off (unreferenced);
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
			    (Globals.StringTable=GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE | GMEM_LOWER, 0)))
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
int WINAPI WEP( int bSystemExit )
#pragma on (unreferenced);
{
	// Save regfile here
	return( 1 );
}

