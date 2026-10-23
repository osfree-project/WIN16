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
    @brief Windows 3.1 registry backed directly by REG.DAT structures.

    Keys and strings live in Globals.EntryTable (DATNAVIGATION and
    DATTEXT records) and Globals.StringTable (text block). There is no
    runtime KEYSTRUCT tree: an HKEY is the index of a dirent in the
    entry table. HKEY_CLASSES_ROOT is represented by 0 or 1, exactly as
    in Win16; both values are accepted interchangeably.
*/

#include "Shell.h"

/*! @def IFK_FIND
    @brief Path resolution mode: only look up existing keys. */
#define IFK_FIND   0

/*! @def IFK_CREATE
    @brief Path resolution mode: create missing keys. */
#define IFK_CREATE 1

/*! @def REG_TEXT_NUL
    @brief Number of bytes to reserve after each string in the text block. */
#define REG_TEXT_NUL 1

/* ------------------------------------------------------------------ */
/* Forward declarations                                                */
/* ------------------------------------------------------------------ */

/*! @brief Allocate a new dirent and attach it to a parent.

    Defined below.

    @param[in] parentHkey Parent key (0/1 for root, or dirent index).
    @param[in] nameAtom   DATTEXT index for the new key's name.

    @return New dirent's HKEY, or 0 on failure.
*/
static HKEY InternalCreateKey(HKEY parentHkey, ATOM nameAtom);

/* ------------------------------------------------------------------ */
/* Lock helpers                                                        */
/* ------------------------------------------------------------------ */

/*! @brief Lock the registry header.

    @return Pointer to DATHEADER, or NULL on failure.
*/
static LPDATHEADER RegLockHdr(void)
{
    return (LPDATHEADER)GlobalLock(Globals.Registry);
}

/*! @brief Unlock the registry header. */
static void RegUnlockHdr(void)
{
    GlobalUnlock(Globals.Registry);
}

/*! @brief Lock the entry table.

    @return Pointer to the entry table, or NULL on failure.
*/
static LPDATNAVIGATION RegLockTab(void)
{
    return (LPDATNAVIGATION)GlobalLock(Globals.EntryTable);
}

/*! @brief Unlock the entry table. */
static void RegUnlockTab(void)
{
    GlobalUnlock(Globals.EntryTable);
}

/*! @brief Lock the text block.

    @return Pointer to the text block, or NULL on failure.
*/
static LPSTR RegLockText(void)
{
    return (LPSTR)GlobalLock(Globals.StringTable);
}

/*! @brief Unlock the text block. */
static void RegUnlockText(void)
{
    GlobalUnlock(Globals.StringTable);
}

/* ------------------------------------------------------------------ */
/* HKEY helpers                                                        */
/* ------------------------------------------------------------------ */

/*! @brief Check whether @p hkey denotes HKEY_CLASSES_ROOT.

    In Win16, 0 and 1 both identify HKEY_CLASSES_ROOT. This helper
    accepts both spellings.

    @param[in] hkey Key handle to test.

    @return Non-zero if @p hkey is HKEY_CLASSES_ROOT, 0 otherwise.
*/
static int IsRootKey(HKEY hkey)
{
    return (hkey == 0 || hkey == (HKEY)1);
}

/*! @brief Return the dirent index of the root key.

    The root dirent is stored in tab[0].Sub. Returns 0 if the root
    dirent is not yet created.

    @param[in] tab Locked entry table.

    @return Root dirent index, or 0 if absent.
*/
static WORD GetRootDirent(LPDATNAVIGATION tab)
{
    return tab[0].Sub;
}

/* ------------------------------------------------------------------ */
/* Slot management                                                     */
/* ------------------------------------------------------------------ */

/*! @brief Allocate a slot in the entry table.

    @return Index of the new slot, or 0 on failure.

    @retval 0 Out of memory or GlobalReAlloc failed.
*/
static WORD RegAllocSlot(void)
{
    LPDATHEADER hdr;
    LPDATNAVIGATION tab;
    WORD idx;
    DWORD newDwEntries;

    hdr = RegLockHdr();
    if (!hdr) return 0;

    if (hdr->freeidx != 0 && hdr->freeidx != 0xFFFF) {
        idx = hdr->freeidx;
        tab = RegLockTab();
        if (!tab) { RegUnlockHdr(); return 0; }
        hdr->freeidx = tab[idx].Next;
        RegUnlockTab();
        RegUnlockHdr();
        return idx;
    }

    idx = (WORD)hdr->dwEntries;
    newDwEntries = hdr->dwEntries + 1;
    hdr->dwEntries = newDwEntries;
    hdr->offsetDataBlock = hdr->unk1 + newDwEntries * sizeof(DATNAVIGATION);
    RegUnlockHdr();

    if (!GlobalReAlloc(Globals.EntryTable,
                       newDwEntries * sizeof(DATNAVIGATION),
                       GMEM_MOVEABLE)) {
        hdr = RegLockHdr();
        if (hdr) {
            hdr->dwEntries = newDwEntries - 1;
            hdr->offsetDataBlock =
                hdr->unk1 + (newDwEntries - 1) * sizeof(DATNAVIGATION);
            RegUnlockHdr();
        }
        return 0;
    }
    return idx;
}

/*! @brief Return a slot to the free list.

    @param[in] idx Slot index.
*/
static void RegFreeSlot(WORD idx)
{
    LPDATHEADER hdr;
    LPDATNAVIGATION tab;

    if (idx == 0) return;

    hdr = RegLockHdr();
    if (!hdr) return;
    tab = RegLockTab();
    if (tab) {
        tab[idx].Next = hdr->freeidx;
        hdr->freeidx = idx;
        RegUnlockTab();
    }
    RegUnlockHdr();
}

/* ------------------------------------------------------------------ */
/* String storage                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Find a string in the REG.DAT text block.

    @param[in] str NUL-terminated string.

    @return DATTEXT index on success, 0 if not found.

    @retval 0 The string was not found, or the string was too long.
*/
static ATOM RegFindString(LPCSTR str)
{
    LPDATHEADER hdr;
    LPDATNAVIGATION tab;
    LPSTR text;
    WORD hash, bucket, cur, len;
    ATOM result = 0;

    if (!str) return 0;
    len = (WORD)lstrlen(str);
    if (len > MAX_ATOM_LEN) return 0;

    hdr = RegLockHdr();
    if (!hdr || !hdr->hashsize) { RegUnlockHdr(); return 0; }
    hash = 0;
    { LPCSTR s = str; while (*s) hash += (WORD)AnsiUpperChar(*s++); }
    hash = (WORD)(hash % hdr->hashsize);
    RegUnlockHdr();

    bucket = (WORD)(1 + hash);

    tab = RegLockTab();
    if (!tab) return 0;
    text = RegLockText();

    cur = tab[bucket].Next;
    while (cur != 0 && cur != bucket) {
        LPDATTEXT dt = (LPDATTEXT)&tab[cur];
        if (dt->Length == len && text &&
            lstrnicmp(text + dt->offset, str, len) == 0) {
            result = cur;
            break;
        }
        cur = dt->Index;
    }

    if (text) RegUnlockText();
    RegUnlockTab();
    return result;
}

/*! @brief Add a string to the REG.DAT text block, or bump RefCount.

    @param[in] str NUL-terminated string.

    @return DATTEXT index on success, 0 on failure.

    @retval 0 Out of memory or the string was too long.
*/
static ATOM RegAddString(LPCSTR str)
{
    ATOM existing;
    LPDATHEADER hdr;
    LPDATNAVIGATION tab;
    LPSTR text;
    WORD hash, bucket, new_idx, len, cur;
    DWORD old_size, new_size;

    if (!str) return 0;
    len = (WORD)lstrlen(str);
    if (len > MAX_ATOM_LEN) return 0;

    existing = RegFindString(str);
    if (existing) {
        tab = RegLockTab();
        if (tab) { ((LPDATTEXT)&tab[existing])->RefCount++; RegUnlockTab(); }
        return existing;
    }

    hdr = RegLockHdr();
    if (!hdr || !hdr->hashsize) { RegUnlockHdr(); return 0; }
    hash = 0;
    { LPCSTR s = str; while (*s) hash += (WORD)AnsiUpperChar(*s++); }
    hash = (WORD)(hash % hdr->hashsize);
    RegUnlockHdr();
    bucket = (WORD)(1 + hash);

    new_idx = RegAllocSlot();
    if (!new_idx) return 0;

    hdr = RegLockHdr();
    if (!hdr) { RegFreeSlot(new_idx); return 0; }
    old_size = hdr->sizeDataBlock;
    new_size = old_size + 2 + len + REG_TEXT_NUL;
    RegUnlockHdr();

    if (!GlobalReAlloc(Globals.StringTable, new_size, GMEM_MOVEABLE)) {
        RegFreeSlot(new_idx);
        return 0;
    }

    hdr = RegLockHdr();
    if (!hdr) { RegFreeSlot(new_idx); return 0; }
    text = RegLockText();
    if (!text) { RegUnlockHdr(); RegFreeSlot(new_idx); return 0; }

    *((LPWORD)(text + old_size)) = (WORD)(new_idx * 2 + 1);
    lstrcpy(text + old_size + 2, str);
    text[old_size + 2 + len] = 0;
    hdr->sizeDataBlock = new_size;

    tab = RegLockTab();
    if (!tab) {
        RegUnlockText(); RegUnlockHdr();
        RegFreeSlot(new_idx);
        return 0;
    }

    cur = tab[bucket].Next;
    if (cur == 0) cur = bucket;

    ((LPDATTEXT)&tab[new_idx])->Index    = cur;
    ((LPDATTEXT)&tab[new_idx])->RefCount = 1;
    ((LPDATTEXT)&tab[new_idx])->Length   = len;
    ((LPDATTEXT)&tab[new_idx])->offset   = (WORD)(old_size + 2);

    tab[bucket].Next = new_idx;

    RegUnlockTab();
    RegUnlockText();
    RegUnlockHdr();
    return new_idx;
}

/*! @brief Copy a string from the text block into @p buf.

    @param[in]  atom  DATTEXT index.
    @param[out] buf   Output buffer (NUL-terminated).
    @param[in]  count Size of @p buf in bytes.

    @return Length of the copied string, excluding NUL.

    @retval 0 Failure, or @p atom was invalid.
*/
static UINT RegGetString(ATOM atom, LPSTR buf, int count)
{
    LPDATHEADER hdr;
    LPDATNAVIGATION tab;
    LPDATTEXT dt;
    LPSTR text;
    WORD len;

    if (!atom || !buf || count <= 0) return 0;

    hdr = RegLockHdr();
    if (!hdr || (DWORD)atom >= hdr->dwEntries) {
        RegUnlockHdr();
        return 0;
    }
    RegUnlockHdr();

    tab = RegLockTab();
    if (!tab) return 0;

    dt = (LPDATTEXT)&tab[atom];
    len = dt->Length;

    text = RegLockText();
    if (!text) { RegUnlockTab(); return 0; }

    if (len >= count) len = (WORD)(count - 1);
    lmemcpy(buf, text + dt->offset, len);
    buf[len] = '\0';

    RegUnlockText();
    RegUnlockTab();
    return len;
}

/*! @brief Release one reference to a DATTEXT record.

    If RefCount drops to zero, the slot is removed from its hash chain
    and returned to the free list.

    @param[in] atom DATTEXT index.
*/
static void RegDelString(ATOM atom)
{
    LPDATHEADER hdr;
    LPDATNAVIGATION tab;
    LPDATTEXT dt;
    LPSTR text;
    WORD hash, bucket, cur, prev_idx, len;
    LPSTR s;

    if (!atom) return;

    hdr = RegLockHdr();
    if (!hdr || (DWORD)atom >= hdr->dwEntries) {
        RegUnlockHdr();
        return;
    }

    tab = RegLockTab();
    if (!tab) { RegUnlockHdr(); return; }

    dt = (LPDATTEXT)&tab[atom];
    if (dt->RefCount > 1) {
        dt->RefCount--;
        RegUnlockTab();
        RegUnlockHdr();
        return;
    }

    text = RegLockText();
    if (!text) { RegUnlockTab(); RegUnlockHdr(); return; }

    hash = 0;
    s = text + dt->offset;
    len = dt->Length;
    while (len--) hash += (WORD)AnsiUpperChar(*s++);
    hash = (WORD)(hash % hdr->hashsize);
    bucket = (WORD)(1 + hash);

    prev_idx = bucket;
    cur = tab[bucket].Next;
    while (cur != 0 && cur != bucket && cur != atom) {
        prev_idx = cur;
        cur = ((LPDATTEXT)&tab[cur])->Index;
    }

    if (cur == atom) {
        tab[prev_idx].Next = dt->Index;
        if (tab[prev_idx].Next == 0) tab[prev_idx].Next = bucket;

        dt->Index    = hdr->freeidx;
        dt->RefCount = 0;
        hdr->freeidx = atom;
    }

    RegUnlockText();
    RegUnlockTab();
    RegUnlockHdr();
}

/* ------------------------------------------------------------------ */
/* Path resolution                                                     */
/* ------------------------------------------------------------------ */

/*! @brief Resolve a path of subkeys, optionally creating missing ones.

    @param[in] parentHkey Starting key: 0 or 1 for HKEY_CLASSES_ROOT,
                          otherwise a dirent index.
    @param[in] lpszSubKey Backslash-separated path, or NULL/empty.
    @param[in] wFlag      IFK_FIND (no create) or IFK_CREATE.

    @return Final HKEY (dirent index, or 0/1 for root), or 0 if not
            found and @p wFlag is IFK_FIND.

    @retval 0 The path was not found or could not be created.
*/
static HKEY InternalFindKey(HKEY parentHkey, LPCSTR lpszSubKey, WORD wFlag)
{
    HKEY cur;
    char buf[256];
    LPSTR ptr, lpTmp;
    ATOM nameAtom;
    LPDATNAVIGATION tab;
    WORD childIdx;

    if (!lpszSubKey || !*lpszSubKey) return parentHkey;
    if (lstrlen(lpszSubKey) >= sizeof(buf)) return 0;
    lstrcpy(buf, lpszSubKey);

    cur = parentHkey;
    ptr = buf;
    while (1) {
        HKEY found = 0;

        lpTmp = lstrchr(ptr, '\\');
        if (lpTmp) *lpTmp = 0;

        nameAtom = RegFindString(ptr);
        if (nameAtom == 0) {
            if (wFlag == IFK_CREATE) nameAtom = RegAddString(ptr);
            if (nameAtom == 0) return 0;
        }

        tab = RegLockTab();
        if (!tab) return 0;
        if (IsRootKey(cur)) {
            WORD rootDirent = GetRootDirent(tab);
            childIdx = (rootDirent != 0) ? tab[rootDirent].Sub : 0;
        } else {
            childIdx = tab[cur].Sub;
        }
        while (childIdx != 0) {
            if (tab[childIdx].Name == nameAtom) { found = (HKEY)childIdx; break; }
            childIdx = tab[childIdx].Next;
        }
        RegUnlockTab();

        if (found == 0) {
            if (wFlag == IFK_CREATE) {
                found = InternalCreateKey(cur, nameAtom);
                if (found == 0) return 0;
            } else return 0;
        }
        cur = found;

        if (lpTmp) ptr = lpTmp + 1;
        else break;
    }
    return cur;
}

/*! @brief Allocate a new dirent and attach it to a parent.

    @param[in] parentHkey Parent key: 0 or 1 for HKEY_CLASSES_ROOT,
                          otherwise a dirent index.
    @param[in] nameAtom   DATTEXT index for the new key's name.

    @return New dirent's HKEY, or 0 on failure.

    @retval 0 Allocation failed or the parent's root dirent is missing.
*/
static HKEY InternalCreateKey(HKEY parentHkey, ATOM nameAtom)
{
    LPDATNAVIGATION tab;
    WORD parentIdx, newIdx, cur;

    newIdx = RegAllocSlot();
    if (!newIdx) return 0;

    tab = RegLockTab();
    if (!tab) { RegFreeSlot(newIdx); return 0; }

    tab[newIdx].Next  = 0;
    tab[newIdx].Sub   = 0;
    tab[newIdx].Name  = nameAtom;
    tab[newIdx].Value = 0;

    if (IsRootKey(parentHkey)) {
        parentIdx = GetRootDirent(tab);
        if (parentIdx == 0) {
            RegUnlockTab();
            RegFreeSlot(newIdx);
            return 0;
        }
    } else {
        parentIdx = (WORD)parentHkey;
    }

    if (tab[parentIdx].Sub == 0) {
        tab[parentIdx].Sub = newIdx;
    } else {
        cur = tab[parentIdx].Sub;
        while (tab[cur].Next != 0) cur = tab[cur].Next;
        tab[cur].Next = newIdx;
    }

    RegUnlockTab();
    return (HKEY)newIdx;
}

/* ------------------------------------------------------------------ */
/* InitReg and ReadSetupReg                                            */
/* ------------------------------------------------------------------ */

/*! @brief Read setup.reg and populate the in-memory registry. */
static void ReadSetupReg(void)
{
    char buf[MAX_PATH];
    HFILE hf;
    LPSTR lpBuffer, ptr;
    char lpTmp[0x100];
    DWORD dwFileSize;
    int i = 0, j;
    HKEY hk;

    if (!GetWindowsDirectory(buf, MAX_PATH)) getcwd(buf, MAX_PATH);
    if (buf[lstrlen(buf)-1] != '\\') lstrcat(buf, "\\");
    lstrcat(buf, "setup.reg");

    hf = _lopen(buf, READ);
    if (hf == HFILE_ERROR) return;
    dwFileSize = _llseek(hf, 0, 2);
    lpBuffer = (LPSTR)GlobalAllocPtr(GPTR, dwFileSize + 2);
    if (!lpBuffer) { _lclose(hf); return; }
    _llseek(hf, 0, 0);
    _lread(hf, lpBuffer, dwFileSize);
    _lclose(hf);

    ptr = lpBuffer;
    while (i < (int)dwFileSize) {
        while ((lpBuffer[i] != '\n') && (i < (int)dwFileSize)) i++;
        if (lpBuffer[i] == '\n') lpBuffer[i] = '\0';
        else lpBuffer[i+1] = '\0';
        if (i > 0 && lpBuffer[i-1] == 0x0d) lpBuffer[i-1] = 0;

        lstrcpyn(lpTmp, ptr, 17);
        lpTmp[17] = 0;
        if (!lstrcmpi(lpTmp, "HKEY_CLASSES_ROOT")) {
            while (*ptr++ != '\\');
            j = 0;
            while (ptr[j] != ' ') j++;
            ptr[j] = 0;
            if (RegCreateKey(0, ptr, &hk) == ERROR_SUCCESS) {
                ptr += j + 3;
                RegSetValue(hk, NULL, REG_SZ, ptr, lstrlen(ptr));
            }
        }
        ptr = &lpBuffer[i+1];
    }
    GlobalFreePtr(lpBuffer);
}

/*! @brief Initialise the in-memory registry.

    Ensures the root dirent (".classes") exists, then loads setup.reg.
    Safe to call multiple times.

    @return TRUE on success, FALSE on failure.

    @retval TRUE  Registry is ready (or was already initialised).
    @retval FALSE Allocation of the root dirent or of ".classes" failed.
*/
BOOL InitReg(void)
{
    LPDATHEADER hdr;
    LPDATNAVIGATION tab;
    ATOM nameAtom;
    WORD rootIdx;

    if (fRegInitialized) return TRUE;
    fRegInitialized = TRUE;

    if (!Globals.Registry || !Globals.EntryTable || !Globals.StringTable)
        return TRUE;

    hdr = RegLockHdr();
    if (!hdr) return FALSE;
    tab = RegLockTab();
    if (!tab) { RegUnlockHdr(); return FALSE; }

    if (tab[0].Sub != 0) {
        RegUnlockTab();
        RegUnlockHdr();
        ReadSetupReg();
        return TRUE;
    }
    RegUnlockTab();
    RegUnlockHdr();

    nameAtom = RegAddString(".classes");
    if (!nameAtom) return FALSE;
    rootIdx = RegAllocSlot();
    if (!rootIdx) return FALSE;

    tab = RegLockTab();
    if (!tab) return FALSE;
    tab[rootIdx].Next  = 0;
    tab[rootIdx].Sub   = 0;
    tab[rootIdx].Name  = nameAtom;
    tab[rootIdx].Value = 0;
    tab[0].Sub = rootIdx;
    RegUnlockTab();

    hdr = RegLockHdr();
    if (hdr) {
        hdr->unk2 = hdr->unk1;
        RegUnlockHdr();
    }

    ReadSetupReg();
    return TRUE;
}

/******************************************************************************
 *           RegOpenKey   [SHELL.1]
 */

/*! @brief Open an existing registry key.

    @param[in]  hKey       Parent key handle.
    @param[in]  lpszSubKey Subkey path, or NULL/empty for @p hKey itself.
    @param[out] phkResult  Receives the opened key handle.

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The key was opened.
    @retval ERROR_BADKEY  The key handle or path was invalid.
*/
LONG WINAPI
RegOpenKey(HKEY hKey, LPCSTR lpszSubKey, PHKEY phkResult)
{
    HKEY found;

    if (!phkResult) return ERROR_BADKEY;
    if (!fRegInitialized) InitReg();

    found = InternalFindKey(hKey, lpszSubKey, IFK_FIND);
    if (found) { *phkResult = found; return ERROR_SUCCESS; }
    *phkResult = 0;
    return ERROR_BADKEY;
}

/******************************************************************************
 *           RegCreateKey   [SHELL.2]
 */

/*! @brief Create or open a registry key.

    @param[in]  hKey       Parent key handle.
    @param[in]  lpszSubKey Subkey path to create.
    @param[out] phkResult  Receives the new key handle.

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The key was created or opened.
    @retval ERROR_BADKEY  The key could not be created.
*/
LONG WINAPI
RegCreateKey(HKEY hKey, LPCSTR lpszSubKey, PHKEY phkResult)
{
    HKEY found;

    if (!phkResult) return ERROR_BADKEY;
    if (!fRegInitialized) InitReg();

    found = InternalFindKey(hKey, lpszSubKey, IFK_CREATE);
    if (found) { *phkResult = found; return ERROR_SUCCESS; }
    *phkResult = 0;
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
    LPDATHEADER hdr;

    if (!fRegInitialized) InitReg();
    if (IsRootKey(hKey)) return ERROR_SUCCESS;

    hdr = RegLockHdr();
    if (!hdr) return ERROR_BADKEY;
    if ((DWORD)hKey >= hdr->dwEntries || (WORD)hKey <= hdr->hashsize) {
        RegUnlockHdr();
        return ERROR_BADKEY;
    }
    RegUnlockHdr();
    return ERROR_SUCCESS;
}

/******************************************************************************
 *           RegDeleteKey   [SHELL.4]
 */

/*! @brief Delete a registry key and its subkeys.

    @param[in] hKey       Parent key handle.
    @param[in] lpszSubKey Subkey path to delete.

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The key was deleted.
    @retval ERROR_BADKEY  The key could not be deleted.
*/
LONG WINAPI
RegDeleteKey(HKEY hKey, LPCSTR lpszSubKey)
{
    HKEY target;
    LPDATNAVIGATION tab;
    WORD parentIdx, targetIdx, cur, prev, nameAtom, valueAtom;

    if (!fRegInitialized) InitReg();

    target = InternalFindKey(hKey, lpszSubKey, IFK_FIND);
    if (target == 0 || IsRootKey(target)) return ERROR_BADKEY;
    targetIdx = (WORD)target;

    for (;;) {
        WORD child;
        tab = RegLockTab();
        if (!tab) return ERROR_BADKEY;
        child = tab[targetIdx].Sub;
        RegUnlockTab();
        if (child == 0) break;
        RegDeleteKey((HKEY)child, "");
    }

    if (IsRootKey(hKey)) {
        tab = RegLockTab();
        if (!tab) return ERROR_BADKEY;
        parentIdx = GetRootDirent(tab);
        RegUnlockTab();
        if (parentIdx == 0) return ERROR_BADKEY;
    } else {
        parentIdx = (WORD)hKey;
    }

    tab = RegLockTab();
    if (!tab) return ERROR_BADKEY;
    nameAtom  = tab[targetIdx].Name;
    valueAtom = tab[targetIdx].Value;

    if (tab[parentIdx].Sub == targetIdx) {
        tab[parentIdx].Sub = tab[targetIdx].Next;
    } else {
        prev = tab[parentIdx].Sub;
        while (tab[prev].Next != 0 && tab[prev].Next != targetIdx)
            prev = tab[prev].Next;
        if (tab[prev].Next == targetIdx)
            tab[prev].Next = tab[targetIdx].Next;
    }
    RegUnlockTab();

    if (valueAtom) RegDelString(valueAtom);
    if (nameAtom)  RegDelString(nameAtom);
    RegFreeSlot(targetIdx);

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
    @retval ERROR_BADKEY  The key was invalid or the value could not be stored.
*/
LONG WINAPI
RegSetValue(HKEY hKey, LPCSTR lpszSubKey,
            DWORD fdwType, LPCSTR lpszValue, DWORD cb)
{
    HKEY target;
    LPDATNAVIGATION tab;
    WORD targetIdx, oldValue;
    ATOM newValue;

    if (fdwType != REG_SZ) return ERROR_BADKEY;

    if (!fRegInitialized) InitReg();

    target = InternalFindKey(hKey, lpszSubKey, IFK_FIND);
    if (target == 0) return ERROR_BADKEY;
    targetIdx = (WORD)target;

    tab = RegLockTab();
    if (!tab) return ERROR_BADKEY;
    oldValue = tab[targetIdx].Value;
    RegUnlockTab();

    if (lpszValue && *lpszValue) {
        newValue = RegAddString(lpszValue);
        if (!newValue) return ERROR_BADKEY;
    } else {
        newValue = 0;
    }

    tab = RegLockTab();
    if (!tab) {
        if (newValue) RegDelString(newValue);
        return ERROR_BADKEY;
    }
    tab[targetIdx].Value = newValue;
    RegUnlockTab();

    if (oldValue && oldValue != newValue) RegDelString(oldValue);

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

    Win16 callers may pass an LPLONG whose high 16 bits contain selector
    garbage; the high word is masked off before use (see the HACK note
    above).

    @param[in]     hKey       Parent key handle.
    @param[in]     lpszSubKey Subkey path, or NULL for @p hKey itself.
    @param[out]    lpszValue  Buffer receiving the value string.
    @param[in,out] lpcb       Size of @p lpszValue; receives copied length.

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The value was retrieved.
    @retval ERROR_BADKEY  The key was invalid, not found, or the buffer too small.
*/
LONG WINAPI
RegQueryValue(HKEY hKey, LPCSTR lpszSubKey, LPSTR lpszValue, LPLONG lpcb)
{
    HKEY target;
    LPDATNAVIGATION tab;
    WORD targetIdx, valueAtom, len;
    DWORD required;

    if (!lpcb || !lpszValue) return ERROR_BADKEY;
    *lpcb &= 0xffff;

    if (!fRegInitialized) InitReg();

    target = InternalFindKey(hKey, lpszSubKey, IFK_FIND);
    if (target == 0) return ERROR_BADKEY;
    targetIdx = (WORD)target;

    tab = RegLockTab();
    if (!tab) return ERROR_BADKEY;
    valueAtom = tab[targetIdx].Value;
    RegUnlockTab();

    if (valueAtom == 0) {
        lpszValue[0] = 0;
        *lpcb = 0;
        return ERROR_SUCCESS;
    }

    tab = RegLockTab();
    if (!tab) return ERROR_BADKEY;
    len = ((LPDATTEXT)&tab[valueAtom])->Length;
    RegUnlockTab();

    required = (DWORD)len + 1;

    if ((DWORD)*lpcb < required) {
        *lpcb = (LONG)required;
        return ERROR_BADKEY;
    }

    RegGetString(valueAtom, lpszValue, (int)*lpcb);
    *lpcb = (LONG)required;
    return ERROR_SUCCESS;
}

/******************************************************************************
 *           RegEnumKey   [SHELL.7]
 */

/*! @brief Enumerate the subkeys of a registry key.

    @param[in]  hKey       Parent key handle.
    @param[in]  iSubKey    Zero-based index of the subkey.
    @param[out] lpszBuffer Buffer receiving the subkey name.
    @param[in]  cbBuffer   Size of @p lpszBuffer.

    @return ERROR_SUCCESS or ERROR_BADKEY.

    @retval ERROR_SUCCESS The subkey name was retrieved.
    @retval ERROR_BADKEY  The key or index was invalid.
*/
LONG WINAPI
RegEnumKey(HKEY hKey, DWORD iSubKey, LPSTR lpszBuffer, DWORD cbBuffer)
{
    LPDATNAVIGATION tab;
    WORD parentIdx, childIdx, nameAtom, nameLen;
    DWORD i;

    if (!lpszBuffer || !cbBuffer) return ERROR_BADKEY;

    if (!fRegInitialized) InitReg();

    tab = RegLockTab();
    if (!tab) return ERROR_BADKEY;

    if (IsRootKey(hKey)) parentIdx = GetRootDirent(tab);
    else parentIdx = (WORD)hKey;

    if (parentIdx == 0) { RegUnlockTab(); return ERROR_BADKEY; }

    childIdx = tab[parentIdx].Sub;
    for (i = 0; i < iSubKey && childIdx != 0; i++)
        childIdx = tab[childIdx].Next;

    if (childIdx == 0) { RegUnlockTab(); return ERROR_BADKEY; }

    nameAtom = tab[childIdx].Name;
    nameLen  = ((LPDATTEXT)&tab[nameAtom])->Length;
    RegUnlockTab();

    if ((DWORD)(nameLen + 1) > cbBuffer) return ERROR_BADKEY;

    RegGetString(nameAtom, lpszBuffer, (int)cbBuffer);
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
