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


/******************************************************************************
 * Internal helpers                                                            *
 ******************************************************************************/

/*! @brief Load REG.DAT from disk into the three global blocks.

    Reads the DATHEADER, validates the signature and field ranges, then
    reads the navigation table and text block into Globals.EntryTable
    and Globals.StringTable respectively.

    GMEM_LOWER is kept for Windows 3.0 compatibility (it is ignored by
    Windows 3.1). GMEM_DISCARDABLE is intentionally not set: the
    registry must never be silently reclaimed by the system.

    @param[in] path Path to REG.DAT.

    @return TRUE on success, FALSE on any read or validation error.

    @retval TRUE  REG.DAT was read successfully into the three global blocks.
    @retval FALSE File missing, malformed, or allocation failed.
*/
static BOOL RegLoadFile(LPCSTR path)
{
	DATHEADER hdr;
	HFILE hf;
	DWORD dwEntriesSize;
	DWORD dwTextSize;
	LPSTR lp;

	hf = _lopen(path, 0);
	if (hf == HFILE_ERROR)
		return FALSE;

	if (_lread(hf, (LPSTR)&hdr, sizeof(hdr)) != sizeof(hdr) ||
	    memcmp(hdr.szSignature, DATMAGIC, sizeof(hdr.szSignature)) != 0 ||
	    hdr.dwEntries == 0 || hdr.sizeDataBlock == 0 || hdr.hashsize == 0)
	{
		_lclose(hf);
		return FALSE;
	}

	dwEntriesSize = hdr.dwEntries * sizeof(DATNAVIGATION);
	dwTextSize    = hdr.sizeDataBlock;

	Globals.Registry    = GlobalAlloc(GMEM_MOVEABLE | GMEM_LOWER, sizeof(DATHEADER));
	Globals.EntryTable  = GlobalAlloc(GMEM_MOVEABLE | GMEM_LOWER, dwEntriesSize);
	Globals.StringTable = GlobalAlloc(GMEM_MOVEABLE | GMEM_LOWER, dwTextSize);

	if (!Globals.Registry || !Globals.EntryTable || !Globals.StringTable)
		goto fail;

	lp = GlobalLock(Globals.Registry);
	if (!lp) goto fail;
	lmemcpy(lp, &hdr, sizeof(DATHEADER));
	GlobalUnlock(Globals.Registry);

	_llseek(hf, hdr.unk1, 0);
	lp = GlobalLock(Globals.EntryTable);
	if (!lp) goto fail;
	_lread(hf, lp, dwEntriesSize);
	GlobalUnlock(Globals.EntryTable);

	_llseek(hf, hdr.offsetDataBlock, 0);
	lp = GlobalLock(Globals.StringTable);
	if (!lp) goto fail;
	_lread(hf, lp, dwTextSize);
	GlobalUnlock(Globals.StringTable);

	_lclose(hf);
	return TRUE;

fail:
	if (Globals.Registry)    { GlobalFree(Globals.Registry);    Globals.Registry    = 0; }
	if (Globals.EntryTable)  { GlobalFree(Globals.EntryTable);  Globals.EntryTable  = 0; }
	if (Globals.StringTable) { GlobalFree(Globals.StringTable); Globals.StringTable = 0; }
	_lclose(hf);
	return FALSE;
}

/*! @brief Write the embedded resource REG.DAT image to disk.

    @param[in] path Destination file path.

    @return TRUE on success, FALSE otherwise.

    @retval FALSE Resource missing, empty, or write failed.
*/
static BOOL CreateRegDatFromResource(LPCSTR path)
{
	HRSRC hrsrc;
	HGLOBAL hres;
	DWORD size;
	LPSTR lp;
	HFILE hf;
	BOOL result = FALSE;

	hrsrc = FindResource(Globals.hInstance, MAKEINTRESOURCE(100), MAKEINTRESOURCE(100));
	if (!hrsrc)
		return FALSE;

	hres = LoadResource(Globals.hInstance, hrsrc);
	if (!hres)
		return FALSE;

	size = SizeofResource(Globals.hInstance, hrsrc);
	if (!size)
	{
		FreeResource(hres);
		return FALSE;
	}

	lp = LockResource(hrsrc);
	if (!lp)
	{
		FreeResource(hres);
		return FALSE;
	}

	hf = _lcreat(path, 0);
	if (hf != HFILE_ERROR)
	{
		if (_lwrite(hf, lp, size) != HFILE_ERROR)
			result = TRUE;
		_lclose(hf);
	}

	UnlockResource(hres);
	FreeResource(hres);
	return result;
}


#pragma off (unreferenced);

/*! @brief Library initialisation: load or create REG.DAT.

    Loads an existing REG.DAT into the three global blocks. If the file
    is missing or malformed, it is recreated from the embedded resource
    (type 100, id 100) and then loaded. Finally InitReg() is called to
    ensure the root dirent exists and to load setup.reg.

    @param[in] hInstance     Instance handle of the library.
    @param[in] wDataSegment  Data segment selector.
    @param[in] wHeapSize     Local heap size.
    @param[in] lpszCmdLine   Command line.

    @return TRUE on success, FALSE otherwise.

    @retval TRUE  The library initialised successfully, or was already
                  initialised.
    @retval FALSE Initialisation failed (missing strings, missing
                  resource, or failed allocation).
*/
BOOL WINAPI LibMain( HINSTANCE hInstance, WORD wDataSegment, WORD wHeapSize, LPSTR lpszCmdLine )
#pragma on (unreferenced);
{
	char szPath[MAX_PATH];
	char szFilename[MAX_PATH];

	if (Globals.hInstance)
		return TRUE;

	Globals.hInstance = hInstance;

	if (!GetWindowsDirectory(szPath, sizeof(szPath)) ||
	    !LoadString(Globals.hInstance, IDS_REGISTRY, szFilename, sizeof(szFilename)))
		return FALSE;

	if (szPath[lstrlen(szPath)-1] != '\\') lstrcat(szPath, "\\");
	lstrcat(szPath, szFilename);

	if (!RegLoadFile(szPath))
	{
		if (!CreateRegDatFromResource(szPath))
			return FALSE;
		if (!RegLoadFile(szPath))
			return FALSE;
	}

	InitReg();
	return TRUE;
}

#pragma off (unreferenced);

/*! @brief Library termination: flush the in-memory registry to REG.DAT.

    Writes exactly sizeof(DATHEADER) bytes of the header,
    hdr.dwEntries * sizeof(DATNAVIGATION) bytes of the entry table, and
    hdr.sizeDataBlock bytes of the text block. This keeps the on-disk
    layout in sync with the header even after RegAllocSlot/RegAddString
    have grown the in-memory blocks via GlobalReAlloc (GlobalSize may
    be larger than the logical sizes due to heap rounding).

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
	DATHEADER hdr;
	DWORD dwEntriesSize;
	DWORD dwTextSize;

	if (!GetWindowsDirectory(szPath, sizeof(szPath)) ||
	    !LoadString(Globals.hInstance, IDS_REGISTRY, szFilename, sizeof(szFilename)))
		return 1;

	if (szPath[lstrlen(szPath)-1] != '\\') lstrcat(szPath, "\\");
	lstrcat(szPath, szFilename);

	if (!Globals.Registry || !Globals.EntryTable || !Globals.StringTable)
		return 1;

	lpRegistry = GlobalLock(Globals.Registry);
	if (!lpRegistry)
		return 1;
	lmemcpy(&hdr, lpRegistry, sizeof(DATHEADER));
	GlobalUnlock(Globals.Registry);

	dwEntriesSize = hdr.dwEntries * sizeof(DATNAVIGATION);
	dwTextSize    = hdr.sizeDataBlock;

	hfRegistry = _lcreat(szPath, 0);
	if (hfRegistry == HFILE_ERROR)
		return 1;

	_lwrite(hfRegistry, (LPSTR)&hdr, sizeof(DATHEADER));

	if (dwEntriesSize)
	{
		lpRegistry = GlobalLock(Globals.EntryTable);
		if (lpRegistry)
		{
			_lwrite(hfRegistry, lpRegistry, dwEntriesSize);
			GlobalUnlock(Globals.EntryTable);
		}
	}

	if (dwTextSize)
	{
		lpRegistry = GlobalLock(Globals.StringTable);
		if (lpRegistry)
		{
			_lwrite(hfRegistry, lpRegistry, dwTextSize);
			GlobalUnlock(Globals.StringTable);
		}
	}

	_lclose(hfRegistry);
	return 1;
}
