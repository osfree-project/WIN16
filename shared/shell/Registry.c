/*
 * 				Shell Library Functions
 *
 * Copyright 1993, 1994, 1995 Alexandre Julliard
 * Copyright 1997 Willows Software, Inc. 
 * Copyright 1998 Marcus Meissner
 * Copyright 2000 Juergen Schmied
 * Copyright 2002 Eric Pouech
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

/*! @file Registry.c
    @brief Windows 3.1 registry (in-memory tree, setup.reg loader).

    @todo This module uses its own global atom table. See:
          https://devblogs.microsoft.com/oldnewthing/20080117-00/?p=23783
          https://devblogs.microsoft.com/oldnewthing/20120521-00/?p=7573

    @todo Registry is in-memory only. See note.txt for the REG.DAT format.
*/

#include "Shell.h"

/*! @brief Find (and optionally create) a key by its backslash-separated path.

    Defined below.

    @param[in] lpKeyStruct Starting key.
    @param[in] lpszSubKey  Path of subkeys, or NULL/empty for @p lpKeyStruct.
    @param[in] wFlag       IFK_FIND (no create) or IFK_CREATE (create missing).

    @return The target key, or NULL if not found and @p wFlag is IFK_FIND.

    @retval NULL The key was not found.
*/
static LPKEYSTRUCT InternalFindKey(LPKEYSTRUCT,LPCSTR,WORD);

/*! @brief Allocate a new key and attach it as the last child of @p lpKeyStruct.

    Defined below.

    @param[in] lpKeyStruct Parent key.
    @param[in] atomSubKey  Atom for the new key's name.

    @return Pointer to the newly created KEYSTRUCT.
*/
static LPKEYSTRUCT InternalCreateKey(LPKEYSTRUCT,ATOM);

/******************************************************************************/

/*! @brief Read setup.reg and populate the in-memory registry.

    Parses HKEY_CLASSES_ROOT\\<path> = <value> lines from setup.reg in
    the Windows directory (or the current directory if the former is
    unavailable) and creates the corresponding keys.
*/
static void ReadSetupReg()
{
    char buf[MAX_PATH];
    HFILE hf;
    LPSTR lpBuffer,ptr;
    char lpTmp[0x100];
    DWORD dwFileSize;
    int i = 0,j;
    LPKEYSTRUCT lpKeyStruct;
    HKEY hk;

	if (!GetWindowsDirectory(buf,MAX_PATH)) getcwd(buf,MAX_PATH);
	if (buf[lstrlen(buf)-1]!='\\') lstrcat(buf, "\\");
	lstrcat(buf,"setup.reg");

	if ((hf = _lopen(buf,READ)) == HFILE_ERROR) return;
	dwFileSize = _llseek(hf,0,2);
	lpBuffer = (LPSTR) GlobalAllocPtr(GPTR, dwFileSize+2);
	_llseek(hf,0,0);
	_lread(hf,lpBuffer,dwFileSize);
	_lclose(hf);

	ptr = lpBuffer;
    while (i < dwFileSize) {
	while ((lpBuffer[i] != '\n') && (i < dwFileSize))
	    i++;
	if (lpBuffer[i] == '\n')
	    lpBuffer[i] = '\0';
	else
	    lpBuffer[i+1] ='\0';
	if (lpBuffer[i-1] == 0xd)	/* strip ^M */
	    lpBuffer[i-1] = 0;
	lstrcpyn(lpTmp,ptr,17);
	lpTmp[17] = 0;
	if (!lstrcmpi(lpTmp,"HKEY_CLASSES_ROOT")) { /* <key> <value> pair */
	    lstrcpy(lpTmp,ptr);
	    lpKeyStruct = &RootKey;
	    while (*ptr++ != '\\');
	    j = 0;
	    while (ptr[j] != ' ') j++;
	    ptr[j] = 0;
	    if (RegCreateKey((HKEY)lpKeyStruct,ptr,&hk) != ERROR_SUCCESS)
		break;
	    ptr += j+3;
	    if (RegSetValue(hk,NULL,REG_SZ,ptr,lstrlen(ptr)) != ERROR_SUCCESS)
		break;
	}
	ptr = &lpBuffer[i+1];
    }
    GlobalFreePtr(lpBuffer);
}

/*! @brief Map legacy HKEY values 0 and 1 to HKEY_CLASSES_ROOT.

    0 and 1 are valid rootkeys in Win16 shell.dll and are used by some
    programs. Do not remove those cases. -MM

    @param[in,out] hkey Pointer to the HKEY to normalise in place.
*/
static inline void fix_win16_hkey( HKEY *hkey )
{
    if (*hkey == 0 || *hkey == (HKEY)1) *hkey = HKEY_CLASSES_ROOT;
}

/*! @brief Compute the REG.DAT hash bucket for a string.

    Uses the Windows 3.1 formula: sum(toupper(c)) % hashsize.
    Confirmed against a real REG.DAT: ".classes" -> bucket 0x11 with
    hashsize 37.

    @param[in] str String to hash.

    @return Bucket index in [0, hashsize).

    @retval 0 The hash table is uninitialised (@p hashsize is zero).
*/
static WORD StringHash(LPCSTR str)
{
	WORD i, hash = 0;
	LPDATHEADER lpDatHeader;

	lpDatHeader=(LPDATHEADER)GlobalLock(Globals.Registry);

	if (!lpDatHeader || !lpDatHeader->hashsize) {
		GlobalUnlock(Globals.Registry);
		return 0;
	}

	for (i = 0; i < lstrlen(str); i++) hash += (WORD)AnsiUpperChar(str[i]);
	hash=hash % lpDatHeader->hashsize;
	GlobalUnlock(Globals.Registry);
	return hash;
}

/*! @brief Add a string to the REG.DAT text block (stub).

    @todo This function is a work-in-progress. See note.txt, section
          "Adding value to Registry", for the intended algorithm.

    @param[in] str String to add.
*/
static void AddString(LPCSTR str)
{
	LPDATHEADER lpDatHeader;
	LPDATNAVIGATION lpDatNavigation;
	WORD hash=StringHash(str);

	(void)str;

	lpDatHeader=(LPDATHEADER)GlobalLock(Globals.Registry);
	lpDatNavigation=(LPDATNAVIGATION)GlobalLock(Globals.EntryTable);

	if (lpDatNavigation && lpDatHeader &&
	    lpDatNavigation[hash+1].Next == hash+1)	/* Empty bucket */
	{
		lpDatHeader->dwEntries++;
		GlobalUnlock(Globals.EntryTable);
		GlobalReAlloc(Globals.EntryTable,
		              lpDatHeader->dwEntries*sizeof(DATNAVIGATION),
		              GMEM_MOVEABLE);
		lpDatNavigation=(LPDATNAVIGATION)GlobalLock(Globals.EntryTable);
		/* TODO: fill in the new string entry. */
	}

	GlobalUnlock(Globals.EntryTable);
	GlobalUnlock(Globals.Registry);
}

/*! @brief Initialise the in-memory registry tree.

    Creates the ".classes" root atom and loads setup.reg.

    @return TRUE on success.

    @retval TRUE Always returned by this implementation.
*/
BOOL InitReg()
{
    RootKey.hParentKey = (HKEY)0L;
    if (!RootKey.atomKey)
	RootKey.atomKey = AddAtomEx(&AtomTable,".classes");
    RootKey.fOpen = TRUE;
    fRegInitialized = TRUE;
    ReadSetupReg();

    return TRUE;
}

/******************************************************************************
 *           RegOpenKey   [SHELL.1]
 */

/*! @brief Open an existing registry key.

    @param[in]  hKey       Parent key handle (HKEY_CLASSES_ROOT, 0, or 1).
    @param[in]  lpszSubKey Subkey path, or NULL/empty for @p hKey itself.
    @param[out] phkResult  Receives the opened key handle.

    @return ERROR_SUCCESS or ERROR_BADKEY.
*/
LONG WINAPI
RegOpenKey(HKEY hKey, LPCSTR lpszSubKey, PHKEY phkResult)
{
    LPKEYSTRUCT lpKeyStruct;
    LPKEYSTRUCT lpSubKey;
    LONG rc;

    fix_win16_hkey( &hKey );

    if (!fRegInitialized)
	InitReg();

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else
	lpKeyStruct = (LPKEYSTRUCT)hKey;

    lpSubKey = InternalFindKey(lpKeyStruct,lpszSubKey,IFK_FIND);

    if ((*phkResult = (HKEY)lpSubKey))
	rc = ERROR_SUCCESS;
    else
	rc = ERROR_BADKEY;

    return rc;
}

/******************************************************************************
 *           RegCreateKey   [SHELL.2]
 */

/*! @brief Create or open a registry key.

    @param[in]  hKey       Parent key handle.
    @param[in]  lpszSubKey Subkey path to create.
    @param[out] phkResult  Receives the new key handle.

    @return ERROR_SUCCESS on success, ERROR_BADKEY on failure.

    @retval ERROR_SUCCESS The key was created or opened.
    @retval ERROR_BADKEY  The parent key was invalid or the child could not be created.
*/
LONG WINAPI
RegCreateKey(HKEY hKey, LPCSTR lpszSubKey, PHKEY phkResult)
{
    LPKEYSTRUCT lpKeyStruct;
    LPKEYSTRUCT lpSubKey;

    fix_win16_hkey( &hKey );

    if (!fRegInitialized)
	InitReg();

    if (hKey == HKEY_CLASSES_ROOT) 
	lpKeyStruct = &RootKey;
    else  {
	lpKeyStruct = (LPKEYSTRUCT)hKey;
	if (!lpKeyStruct || !lpKeyStruct->fOpen) {
	    return ERROR_BADKEY;
	}
    }
    lpSubKey = InternalFindKey(lpKeyStruct,lpszSubKey,IFK_CREATE);

    if ((*phkResult = (HKEY)lpSubKey)) {
	return (LONG)ERROR_SUCCESS;
    }

    return ERROR_BADKEY;
}

/******************************************************************************
 *           RegCloseKey   [SHELL.3]
 */

/*! @brief Close a registry key handle.

    @param[in] hKey Key handle to close.

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The key was closed.
    @retval ERROR_BADKEY  The key handle was invalid.
*/
LONG WINAPI
RegCloseKey(HKEY hKey)
{
    LPKEYSTRUCT lpKeyStruct;

    fix_win16_hkey( &hKey );

    if (!fRegInitialized)
	InitReg();

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else
	lpKeyStruct = (LPKEYSTRUCT)hKey;

    if (!lpKeyStruct) {
	return ERROR_BADKEY;
    }

    lpKeyStruct->fOpen = 0;

    return (LONG)ERROR_SUCCESS;
}

/******************************************************************************
 *           RegDeleteKey   [SHELL.4]
 */

/*! @brief Delete a registry key and its subkeys.

    @param[in] hKey       Parent key handle.
    @param[in] lpszSubKey Subkey path to delete.

    @return ERROR_SUCCESS, ERROR_BADKEY, or ERROR_ACCESS_DENIED.

    @retval ERROR_SUCCESS       The key was deleted.
    @retval ERROR_BADKEY        The key handle was invalid or the key not found.
    @retval ERROR_ACCESS_DENIED Attempt to delete the root key itself.
*/
LONG WINAPI
RegDeleteKey(HKEY hKey, LPCSTR lpszSubKey)
{
    LPKEYSTRUCT lpKeyStruct,lpKeyParent,lpKeyTmp;

    fix_win16_hkey( &hKey );

    if (!fRegInitialized)
	InitReg();

    lpKeyStruct = InternalFindKey((LPKEYSTRUCT)hKey,lpszSubKey,IFK_FIND);

    if (!lpKeyStruct || !lpKeyStruct->fOpen) {
	return ERROR_BADKEY;
    }

    if (lpKeyStruct == &RootKey) {
	return ERROR_ACCESS_DENIED;
    }

    if (lpKeyStruct->lpszValue)
	GlobalFreePtr(lpKeyStruct->lpszValue);

    while (lpKeyStruct->hSubKey) 
	RegDeleteKey(lpKeyStruct->hSubKey,"");

    lpKeyParent = (LPKEYSTRUCT)lpKeyStruct->hParentKey;

    if (lpKeyParent->hSubKey == (HKEY)lpKeyStruct)
	lpKeyParent->hSubKey = lpKeyStruct->hNext;
    else {
	for (lpKeyTmp = (LPKEYSTRUCT)lpKeyParent->hSubKey;
			lpKeyTmp;
			lpKeyTmp = (LPKEYSTRUCT)lpKeyTmp->hNext) 
	    if (lpKeyTmp->hNext == (HKEY)lpKeyStruct)
		lpKeyTmp->hNext = lpKeyStruct->hNext;
    }

    DeleteAtomEx(&AtomTable,lpKeyStruct->atomKey);

	GlobalFreePtr((LPSTR)lpKeyStruct);

    return ERROR_SUCCESS;
}

/******************************************************************************
 *           RegSetValue   [SHELL.5]
 */

/*! @brief Set the default string value of a registry key.

    @param[in] hKey       Parent key handle.
    @param[in] lpszSubKey Subkey path, or NULL for @p hKey itself.
    @param[in] fdwType    Value type; must be REG_SZ.
    @param[in] lpszValue  Value string, or NULL/empty to clear.
    @param[in] cb         Size of @p lpszValue (ignored).

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The value was set.
    @retval ERROR_BADKEY  The key was invalid or not found.
*/
LONG WINAPI
RegSetValue(HKEY hKey, LPCSTR lpszSubKey,
		DWORD fdwType, LPCSTR lpszValue, DWORD cb)
{
    LPKEYSTRUCT lpKeyStruct;
    LPKEYSTRUCT lpSubKeyStruct;

    fix_win16_hkey( &hKey );

    if (!fRegInitialized)
	InitReg();

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else
	lpKeyStruct = (LPKEYSTRUCT)hKey;

    if (!lpKeyStruct || !(lpKeyStruct->fOpen)) {
	return ERROR_BADKEY;
    }

    lpSubKeyStruct = InternalFindKey(lpKeyStruct,lpszSubKey,IFK_FIND);

    if (!lpSubKeyStruct) {
	return ERROR_BADKEY;
    }

    if (lpSubKeyStruct->lpszValue)
	GlobalFreePtr(lpSubKeyStruct->lpszValue);

    if (lpszValue && (*lpszValue != 0)) {
	lpSubKeyStruct->lpszValue = (LPSTR) GlobalAllocPtr(GPTR, lstrlen(lpszValue)+1);
	lstrcpy(lpSubKeyStruct->lpszValue,lpszValue);
    }

    return ERROR_SUCCESS;
}

/******************************************************************************
 *           RegQueryValue   [SHELL.6]
 *
 * NOTES
 *    Is this HACK still applicable?
 *
 * HACK
 *    The 16bit RegQueryValue doesn't handle selectorblocks anyway, so we just
 *    mask out the high 16 bit.  This (not so much incidentally) hopefully fixes
 *    Aldus FH4)
 */

/*! @brief Retrieve the default string value of a registry key.

    @param[in]  hKey       Parent key handle.
    @param[in]  lpszSubKey Subkey path, or NULL for @p hKey itself.
    @param[out] lpszValue  Buffer receiving the value string.
    @param[in,out] lpcb    Size of @p lpszValue; receives copied length.

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The value was retrieved.
    @retval ERROR_BADKEY  The key was invalid or not found.
*/
LONG WINAPI
RegQueryValue(HKEY hKey, LPCSTR lpszSubKey, LPSTR lpszValue, LPLONG lpcb)
{
    LPKEYSTRUCT lpKeyStruct;
    LPKEYSTRUCT lpSubKeyStruct;

    if (lpcb) *lpcb &= 0xffff;

    fix_win16_hkey( &hKey );

    if (!fRegInitialized)
	InitReg();

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else 
	lpKeyStruct = (LPKEYSTRUCT)hKey;

    if (!lpKeyStruct) {
	return ERROR_BADKEY;
    }

#ifdef LATER
    if (!(lpKeyStruct->fOpen)) {
	return ERROR_BADKEY;
    }
#endif


    lpSubKeyStruct = InternalFindKey(lpKeyStruct,lpszSubKey,IFK_FIND);

    if (!lpSubKeyStruct) {
	return ERROR_BADKEY;
    }

    if (lpSubKeyStruct->lpszValue && (*lpSubKeyStruct->lpszValue != 0)) {
	lstrcpyn(lpszValue,lpSubKeyStruct->lpszValue,min(*lpcb,
		(int)lstrlen(lpSubKeyStruct->lpszValue)+1));
	*lpcb = lstrlen(lpszValue) + 1;
    }
    else {
	lpszValue[0] = '\0';
	*lpcb = 0;
    }

    return ERROR_SUCCESS;
}

/******************************************************************************
 *           RegEnumKey   [SHELL.7]
 */

/*! @brief Enumerate the subkeys of a registry key.

    @param[in]  hKey       Parent key handle.
    @param[in]  iSubKey    Zero-based index of the subkey to retrieve.
    @param[out] lpszBuffer Buffer receiving the subkey name.
    @param[in]  cbBuffer   Size of @p lpszBuffer.

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The subkey name was retrieved.
    @retval ERROR_BADKEY  The key handle or index was invalid.
*/
LONG WINAPI
RegEnumKey(HKEY hKey, DWORD iSubKey, LPSTR lpszBuffer, DWORD cbBuffer)
{
    LPKEYSTRUCT lpKeyStruct;
    char buf[128];
    int i;

    fix_win16_hkey( &hKey );

    if (!fRegInitialized)
	InitReg();

    if (hKey == HKEY_CLASSES_ROOT)
	lpKeyStruct = &RootKey;
    else
	lpKeyStruct = (LPKEYSTRUCT)hKey;

    if (!lpKeyStruct) {
	return ERROR_BADKEY;
    }

    for (i=0,lpKeyStruct = (LPKEYSTRUCT)lpKeyStruct->hSubKey; 
		i< iSubKey && lpKeyStruct;
		i++,lpKeyStruct = (LPKEYSTRUCT)lpKeyStruct->hNext);

    if ((i != iSubKey) || (lpKeyStruct == NULL)) {
	return ERROR_BADKEY;
    }

    if (GetAtomNameEx(&AtomTable,lpKeyStruct->atomKey,buf,sizeof(buf)))
	lstrcpyn(lpszBuffer,buf,min(cbBuffer,lstrlen(buf)+1));
    else
	lpszBuffer[0] = '\0';

    return ERROR_SUCCESS;
}

#if 0
LONG WINAPI
RegOpenKeyEx
(
  HKEY  hKey,           /* handle of open key */
  LPCTSTR  lpSubKey,    /* address of name of subkey to open */
  DWORD  ulOptions,       /* reserved */
  REGSAM  samDesired,   /* security access mask */
  PHKEY  phkResult        /* address of handle of open key */
)
{
  return( RegOpenKey( hKey,     lpSubKey,       phkResult ) );
}

LONG
WINAPI
RegQueryValueEx
(
  HKEY hKey,
  LPCSTR lpValueName,
  LPDWORD lpReserved,
  LPDWORD lpType,
  LPBYTE lpData,
  LPDWORD lpcbData
)
{
  /* Success is 0, so return arbitrary error. */
  return(1);
}

#endif

/******************************************************************************
 *           InternalFindKey [Internal]
 */

/*! @brief Find (and optionally create) a key by its backslash-separated path.

    @param[in] lpKeyStruct Starting key.
    @param[in] lpszSubKey  Path of subkeys, or NULL/empty for @p lpKeyStruct.
    @param[in] wFlag       IFK_FIND (no create) or IFK_CREATE (create missing).

    @return The target key, or NULL if not found and @p wFlag is IFK_FIND.

    @retval NULL The key was not found.
*/
static LPKEYSTRUCT
InternalFindKey(LPKEYSTRUCT lpKeyStruct, LPCSTR lpszSubKey, WORD wFlag)
{
    LPKEYSTRUCT lpKeyTmp;
    char buf[80];
    LPSTR ptr,lpTmp;
    ATOM atomSubKey;

    if (lpszSubKey)
	lstrcpy(buf,lpszSubKey);
    if (lpszSubKey && (lstrlen(lpszSubKey) != 0)) {
	ptr = buf;
	while (1) {
	    lpTmp = lstrchr(ptr,'\\');
	    if (lpTmp)
		*lpTmp = '\0';
	    atomSubKey = FindAtomEx(&AtomTable,ptr);
	    if (atomSubKey == 0) 
		if (wFlag == IFK_CREATE)
		    atomSubKey = AddAtomEx(&AtomTable,ptr);
		else
		    return NULL;
	    for (lpKeyTmp = (LPKEYSTRUCT)lpKeyStruct->hSubKey;
			lpKeyTmp;
			lpKeyTmp = (LPKEYSTRUCT)lpKeyTmp->hNext) {
		if (lpKeyTmp->atomKey == atomSubKey)
		    break;
	    }
	    if (lpKeyTmp)
		lpKeyStruct = lpKeyTmp;
	    else 
		if (wFlag == IFK_CREATE)
		    lpKeyStruct = InternalCreateKey(lpKeyStruct,atomSubKey);
		else
		    return NULL;
	    lpKeyStruct->fOpen = TRUE;
	    if (lpTmp)
		ptr = lpTmp + 1;
	    else
		break;
	}
    }
    return lpKeyStruct;
}

/******************************************************************************
 *           InternalCreateKey [Internal]
 */

/*! @brief Allocate a new key and attach it as the last child of @p lpKeyStruct.

    @param[in] lpKeyStruct Parent key.
    @param[in] atomSubKey  Atom for the new key's name.

    @return Pointer to the newly created KEYSTRUCT.
*/
static LPKEYSTRUCT
InternalCreateKey(LPKEYSTRUCT lpKeyStruct, ATOM atomSubKey)
{
    LPKEYSTRUCT lpSubKey;
    LPKEYSTRUCT lpKeyTmp;

    lpSubKey = (LPKEYSTRUCT)GlobalAllocPtr(GPTR, sizeof(KEYSTRUCT));

    if (!lpKeyStruct->hSubKey)
	lpKeyStruct->hSubKey = (HKEY)lpSubKey;
    else {
	for (lpKeyTmp = (LPKEYSTRUCT)lpKeyStruct->hSubKey;
		lpKeyTmp->hNext;
		lpKeyTmp = (LPKEYSTRUCT)lpKeyTmp->hNext);
	lpKeyTmp->hNext = (HKEY)lpSubKey;
    }
    lpSubKey->hParentKey = (HKEY)lpKeyStruct;
    lpSubKey->atomKey = atomSubKey;
    return lpSubKey;
}
