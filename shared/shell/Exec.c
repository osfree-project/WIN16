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

/*! @file Exec.c
    @brief ShellExecute and FindExecutable.

    Windows version handling (see WINVER in Shell.h):

      - WINVER >= 0x030A (Windows 3.1+): associations live in REG.DAT:
            HKEY_CLASSES_ROOT\\.ext                 -> class name
            HKEY_CLASSES_ROOT\\<class>\\shell\\<verb>\\command
                                                    -> "cmd %1 ..."
        Source: note.txt (REG.DAT layout) and the Windows 3.1 SDK
        documentation of ShellExecute and FindExecutable.

      - WINVER < 0x030A (Windows 3.0): associations live in
        win.ini [extensions]:
            [extensions]
            txt=notepad.exe ^.txt
            bmp=pbrush.exe ^.bmp /p
        where '^' marks the placeholder for the file name, and the
        bytes after "^.ext" are appended unchanged.
        Source: Windows 3.0 SDK, WIN.INI file documentation.

    ShellExecute returns the HINSTANCE of the launched application
    (>32) on success, or a WinExec-style error code (<=32) on failure.
    WinExec error codes 0..21 are part of the SE_ERR_* contract and
    are passed through unchanged. SE_ERR_NOASSOC (31) is returned
    when there is no association for the extension.

    For executable files (.exe / .com / .bat / .pif) the registry and
    win.ini are bypassed, and the file is passed to WinExec()
    directly, concatenated with lpszParams. Source: Windows 3.1 SDK,
    ShellExecute(): "If it is an executable file, this function opens
    it, even if the string 'print' is pointed to by lpszOp."
*/

#include "Shell.h"

/*************************************************************************
 * Internal helpers
 */

/*! @brief Locate a file along %PATH% and copy the resolved path out.

    @param[in]  filename Far string with the file name to search for.
    @param[out] fullpath Near output buffer receiving the resolved path.
    @param[in]  size     Size of @p fullpath in bytes.

    @return TRUE when the file was found and @p fullpath is non-empty.

    @retval TRUE  The file was found.
    @retval FALSE The file was not found, or the buffer is too small.
*/
static BOOL searchpath(LPCSTR filename, char *fullpath, UINT size)
{
    static char searchenv_buf[_MAX_PATH];
    char        nearpath[MAX_PATH];

    if (!fullpath || size < 2) return FALSE;
    fullpath[0] = '\0';

    lstrcpyn(nearpath, filename, sizeof(nearpath));
    _searchenv(nearpath, "PATH", searchenv_buf);

    if (searchenv_buf[0] == '\0') return FALSE;

    lstrcpyn(fullpath, searchenv_buf, size);
    return TRUE;
}

/*! @brief Find the extension (including the leading dot) of a file.

    Only the last '.' after the last backslash is treated as the
    extension separator, so "C:\\a.b\\c" has no extension.

    @param[in] nearfile Near string with the file path.

    @return Pointer to the dot, or NULL if the file has no extension.

    @retval NULL The file has no extension.
*/
static char *FindExtension(char *nearfile)
{
    char *p = NULL;
    char *q;

    for (q = nearfile; *q; q++) {
        if (*q == '\\') p = NULL;
        else if (*q == '.') p = q;
    }
    return p;
}

/*! @brief Extract the first token (the executable path) from a command.

    Handles the two documented forms:
        "C:\\Program Files\\foo.exe" %1
        notepad.exe %1

    @param[in,out] command Command line. On return, the first token is
                           NUL-terminated in place.
    @param[out]    exe     Near buffer receiving the executable path.
    @param[in]     size    Size of @p exe in bytes.

    @return TRUE on success, FALSE if the command is empty or has an
            unbalanced quotation mark.

    @retval FALSE The command is empty or has an unbalanced quotation
                  mark.
*/
static BOOL ExtractFirstToken(char *command, char *exe, int size)
{
    char *p = command;
    char *q;

    while (*p == ' ' || *p == '\t') p++;

    if (*p == '"') {
        p++;
        q = strchr(p, '"');
        if (!q) return FALSE;
        *q = '\0';
    } else {
        q = p;
        while (*q && *q != ' ' && *q != '\t') q++;
        *q = '\0';
    }

    if (!*p) return FALSE;

    lstrcpyn(exe, p, size);
    return TRUE;
}

/*! @brief Copy @p name to @p out, quoting it if it contains a space.

    WinExec parses the command line and honours double quotes, so a path
    like "C:\\My Files\\a.txt" must be quoted.

    @param[out] out   Destination buffer.
    @param[in]  size  Size of @p out in bytes.
    @param[in]  name  Near path to copy.
*/
static void AppendQuoted(char *out, int size, char *name)
{
    BOOL spaces = (lstrchr(name, ' ') != NULL);

    out[0] = '\0';
    if (spaces) lstrcat(out, "\"");
    lstrcat(out, name);
    if (spaces) lstrcat(out, "\"");
    (void)size;
}

/*************************************************************************
 * Windows 3.1+ : REG.DAT lookup
 */

#if WINVER >= 0x030A

/*! @brief Resolve a class name for a file extension from REG.DAT.

    Source: note.txt and the Windows 3.1 registry layout.
    HKEY_CLASSES_ROOT\\.ext has the class name as the default value.

    @param[in]  ext        Far string, the extension including the dot.
    @param[out] classname  Near output buffer.
    @param[in]  size       Size of @p classname in bytes.

    @return TRUE on success, FALSE on failure.

    @retval TRUE  A class name was found.
    @retval FALSE REG.DAT has no association for @p ext.
*/
static BOOL RegLookupClass(LPCSTR ext, char *classname, int size)
{
    LONG cb;

    classname[0] = '\0';
    cb = (LONG)size;

    if (RegQueryValue(HKEY_CLASSES_ROOT, ext, classname, &cb) != ERROR_SUCCESS)
        return FALSE;

    return (classname[0] != '\0');
}

/*! @brief Resolve the command line for a class + verb from REG.DAT.

    Source: note.txt and the Windows 3.1 registry layout.
    HKEY_CLASSES_ROOT\\<class>\\shell\\<verb>\\command has the
    command line as the default value.

    @param[in]  classname Class name resolved by RegLookupClass.
    @param[in]  verb      Verb ("open" or "print").
    @param[out] command   Near output buffer.
    @param[in]  size      Size of @p command in bytes.

    @return TRUE on success, FALSE on failure.

    @retval TRUE  A command line was found.
    @retval FALSE No "command" subkey for this verb.
*/
static BOOL RegLookupCommand(LPCSTR classname, LPCSTR verb,
                             char *command, int size)
{
    char keypath[512];
    LONG cb;

    wsprintf(keypath, "%s\\shell\\%s\\command", classname, verb);

    command[0] = '\0';
    cb = (LONG)size;
    if (RegQueryValue(HKEY_CLASSES_ROOT, keypath, command, &cb) != ERROR_SUCCESS)
        return FALSE;

    return (command[0] != '\0');
}

/*! @brief Substitute %1 in a REG.DAT command with the file name.

    Source: Windows 3.1 SDK documentation of ShellExecute only
    documents %1 as a placeholder. The file name is quoted if it
    contains spaces, because WinExec parses double quotes.

    @param[in]  command  Near command template, NUL-terminated.
    @param[in]  nearfile Near file name.
    @param[out] expanded Near output buffer.
    @param[in]  size     Size of @p expanded in bytes.
*/
static void ExpandCommandRegDat(LPCSTR command, LPCSTR nearfile,
                                char *expanded, int size)
{
    BOOL quoted = (lstrchr(nearfile, ' ') != NULL);
    LPCSTR src = command;
    LPCSTR f;
    int ofs = 0;

    while (*src && ofs < size - MAX_PATH - 4) {
        if (src[0] == '%' && src[1] == '1') {
            if (quoted) expanded[ofs++] = '"';
            for (f = nearfile; *f; f++) expanded[ofs++] = *f;
            if (quoted) expanded[ofs++] = '"';
            src += 2;
        } else {
            expanded[ofs++] = *src++;
        }
    }
    expanded[ofs] = '\0';
}

#endif /* WINVER >= 0x030A */

/*************************************************************************
 * Windows 3.0 : win.ini [extensions] lookup
 */

#if WINVER < 0x030A

/*! @brief Read the win.ini [extensions] entry for an extension.

    Source: Microsoft Windows 3.0 SDK, WIN.INI file documentation.
    The entry looks like:
        [extensions]
        txt=notepad.exe ^.txt
        bmp=pbrush.exe ^.bmp /p
    where '^' marks the placeholder for the file name.

    @param[in]  ext     Far string, the extension including the dot.
    @param[out] command Near output buffer receiving the raw entry.
    @param[in]  size    Size of @p command in bytes.

    @return TRUE on success, FALSE on failure.

    @retval TRUE  A win.ini entry was found.
    @retval FALSE win.ini has no [extensions] entry for @p ext.
*/
static BOOL WinIniLookupCommand(LPCSTR ext, char *command, int size)
{
    char key[32];

    command[0] = '\0';
    lstrcpyn(key, ext + 1, sizeof(key));   /* skip the leading dot */

    LoadString(Globals.hInstance, IDS_EXTENSIONS, szExtensions,
               sizeof(szExtensions));

    if (GetProfileString(szExtensions, key, "", command, size) <= 0)
        return FALSE;

    return (command[0] != '\0');
}

/*! @brief Substitute '^' in a win.ini [extensions] entry.

    The '^' is replaced by the file name. The bytes that follow
    "^.ext" (for example " /p" in "pbrush.exe ^.bmp /p") are
    appended unchanged. Source: Microsoft Windows 3.0 SDK, WIN.INI
    file documentation.

    @param[in,out] command  Near win.ini entry. The '^' is overwritten
                            with NUL on return.
    @param[in]     ext      Extension including the dot.
    @param[in]     nearfile Near file name.
    @param[out]    expanded Near output buffer.
    @param[in]     size     Size of @p expanded in bytes.
*/
static void ExpandCommandWinIni(char *command, LPCSTR ext, LPCSTR nearfile,
                                char *expanded, int size)
{
    char *tok;
    int skip;

    expanded[0] = '\0';

    tok = lstrchr(command, '^');
    if (!tok) {
        lstrcpyn(expanded, command, size);
        return;
    }

    /* "^" + "." + ext is the length of the placeholder. */
    skip = 1 + lstrlen(ext);

    *tok = '\0';   /* command now holds everything before '^' */

    lstrcpyn(expanded, command, size);
    lstrcat(expanded, nearfile);
    lstrcat(expanded, tok + skip);
}

#endif /* WINVER < 0x030A */

/*************************************************************************
 * ShellExecute	[SHELL.20]
 */

/*! @brief Open or print a file using its association.

    Source: Microsoft Windows 3.1 SDK, ShellExecute(). On
    WINVER >= 0x030A associations are resolved through REG.DAT; on
    WINVER < 0x030A through win.ini [extensions].

    Executable files (.exe / .com / .bat / .pif) are passed to
    WinExec() directly, bypassing the association tables. This matches
    the documented behaviour: "If it is an executable file, this
    function opens it, even if the string 'print' is pointed to by
    lpszOp."

    @param[in] hWnd         Parent window handle. In Win 3.1 this was
                            the parent of any DDE error dialogs; since
                            we do not implement DDE, it is informational.
    @param[in] lpOperation  Verb ("open" or "print"), or NULL/empty
                            for the default "open".
    @param[in] lpFile       Path of the file to launch.
    @param[in] lpParameters Parameters passed to the application when
                            @p lpFile is an executable, or NULL.
    @param[in] lpDirectory  Working directory, or NULL.
    @param[in] nShowCmd     Show command passed to WinExec().

    @return HINSTANCE of the launched application (>32) on success, or
            an SE_ERR_* / WinExec error code (<=32) on failure.

    @retval SE_ERR_FNF     @p lpFile was NULL or empty.
    @retval SE_ERR_NOASSOC No association for the file's extension.
    @retval SE_ERR_PNF     Path was not found (passed through from
                           WinExec).
    @retval SE_ERR_OOM     Out of memory (passed through from WinExec).
*/
HINSTANCE WINAPI ShellExecute(HWND hWnd, LPCSTR lpOperation, LPCSTR lpFile,
		LPCSTR lpParameters, LPCSTR lpDirectory, int nShowCmd)
{
    char        nearfile[MAX_PATH];
    char        ext[MAX_PATH];
    char        command[1024];
    char        expanded[4096];
    char        cmdline[2048];
    char        old_dir[MAX_PATH];
    char        nearDir[MAX_PATH];
    LPSTR       p;
    LPCSTR      verb;
    BOOL        need_exe_quote;
    HINSTANCE   hInst;

    (void)hWnd;

    if (!lpFile || !*lpFile)
        return (HINSTANCE)SE_ERR_FNF;

    lstrcpyn(nearfile, lpFile, sizeof(nearfile));

    if (lpDirectory && *lpDirectory)
    {
        getcwd(old_dir, sizeof(old_dir));
        lstrcpyn(nearDir, lpDirectory, sizeof(nearDir));
        chdir(nearDir);
    }

    p = FindExtension(nearfile);
    if (!p)
    {
        if (lpDirectory && *lpDirectory) chdir(old_dir);
        return (HINSTANCE)SE_ERR_NOASSOC;
    }
    lstrcpyn(ext, p, sizeof(ext));

    /* Executable file: run it directly, ignoring association and verb.
       Source: Windows 3.1 SDK, ShellExecute(). */
    if (lstrcmpi(ext, ".exe") == 0 ||
        lstrcmpi(ext, ".com") == 0 ||
        lstrcmpi(ext, ".bat") == 0 ||
        lstrcmpi(ext, ".pif") == 0)
    {
        need_exe_quote = (lstrchr(nearfile, ' ') != NULL);

        if (lpParameters && *lpParameters)
        {
            if (need_exe_quote)
                wsprintf(cmdline, "\"%s\" %s", nearfile, lpParameters);
            else
                wsprintf(cmdline, "%s %s", nearfile, lpParameters);
        }
        else
        {
            if (need_exe_quote)
                wsprintf(cmdline, "\"%s\"", nearfile);
            else
                lstrcpyn(cmdline, nearfile, sizeof(cmdline));
        }

        hInst = WinExec(cmdline, nShowCmd);

        if (lpDirectory && *lpDirectory) chdir(old_dir);

        /* WinExec errors 0..21 are already SE_ERR_* compatible. */
        return hInst;
    }

    verb = (lpOperation && *lpOperation) ? lpOperation : "open";

#if WINVER >= 0x030A
    {
        char classname[MAX_PATH];

        if (!RegLookupClass(ext, classname, sizeof(classname)))
        {
            if (lpDirectory && *lpDirectory) chdir(old_dir);
            return (HINSTANCE)SE_ERR_NOASSOC;
        }

        if (!RegLookupCommand(classname, verb, command, sizeof(command)))
        {
            if (lpDirectory && *lpDirectory) chdir(old_dir);
            return (HINSTANCE)SE_ERR_NOASSOC;
        }

        ExpandCommandRegDat(command, nearfile, expanded, sizeof(expanded));
    }
#else  /* WINVER < 0x030A */
    if (!WinIniLookupCommand(ext, command, sizeof(command)))
    {
        if (lpDirectory && *lpDirectory) chdir(old_dir);
        return (HINSTANCE)SE_ERR_NOASSOC;
    }

    ExpandCommandWinIni(command, ext, nearfile, expanded, sizeof(expanded));
    (void)verb;
#endif

    hInst = WinExec(expanded, nShowCmd);

    if (lpDirectory && *lpDirectory) chdir(old_dir);

    /* WinExec errors 0..21 are already SE_ERR_* compatible. */
    return hInst;
}

/*************************************************************************
 * FindExecutable	[SHELL.21]
 */

/*! @brief Return the executable associated with a file's extension.

    Source: Microsoft Windows 3.1 SDK, FindExecutable(). On
    WINVER >= 0x030A associations are resolved through REG.DAT; on
    WINVER < 0x030A through win.ini [extensions].

    @param[in]  lpszFile Path of the file whose association to resolve.
    @param[in]  lpszDir  Optional working directory.
    @param[out] lpResult Buffer receiving the executable path.

    @return Greater than 32 on success, or an SE_ERR_* value on failure.

    @retval SE_ERR_FNF             @p lpszFile or @p lpResult was NULL,
                                  or the executable was not found.
    @retval SE_ERR_NOASSOC         No association for the extension.
    @retval SE_ERR_ASSOCINCOMPLETE The command line has an unbalanced
                                  quotation mark.
    @retval 33                     Success; greater than 32, as in
                                  Win 3.1.
*/
HINSTANCE WINAPI FindExecutable(LPCSTR lpszFile, LPCSTR lpszDir, LPSTR lpResult)
{
    char        nearfile[MAX_PATH];
    char        ext[MAX_PATH];
    char        command[1024];
    char        exe[MAX_PATH];
    char        fullpath[MAX_PATH];
    char        old_dir[MAX_PATH];
    char        nearDir[MAX_PATH];
    LPSTR       p;
    LPSTR       tok;
    HINSTANCE   hInst = (HINSTANCE)SE_ERR_NOASSOC;

    if (!lpResult) return (HINSTANCE)SE_ERR_FNF;
    lpResult[0] = '\0';
    if (!lpszFile || !*lpszFile) return (HINSTANCE)SE_ERR_FNF;

    lstrcpyn(nearfile, lpszFile, sizeof(nearfile));

    if (lpszDir && *lpszDir)
    {
        getcwd(old_dir, sizeof(old_dir));
        lstrcpyn(nearDir, lpszDir, sizeof(nearDir));
        chdir(nearDir);
    }

    p = FindExtension(nearfile);
    if (!p)
    {
        if (lpszDir && *lpszDir) chdir(old_dir);
        return (HINSTANCE)SE_ERR_NOASSOC;
    }
    lstrcpyn(ext, p, sizeof(ext));

#if WINVER >= 0x030A
    {
        char classname[MAX_PATH];

        if (!RegLookupClass(ext, classname, sizeof(classname)))
        {
            if (lpszDir && *lpszDir) chdir(old_dir);
            return (HINSTANCE)SE_ERR_NOASSOC;
        }

        if (!RegLookupCommand(classname, "open", command, sizeof(command)))
        {
            if (lpszDir && *lpszDir) chdir(old_dir);
            return (HINSTANCE)SE_ERR_NOASSOC;
        }
    }
#else  /* WINVER < 0x030A */
    if (!WinIniLookupCommand(ext, command, sizeof(command)))
    {
        if (lpszDir && *lpszDir) chdir(old_dir);
        return (HINSTANCE)SE_ERR_NOASSOC;
    }

    /* win.ini entry: "pbrush.exe ^.bmp /p". Truncate at '^' so that
       only the executable (plus any fixed parameters before '^')
       remains; the first token is what we need. */
    tok = lstrchr(command, '^');
    if (tok) *tok = '\0';
#endif

    if (!ExtractFirstToken(command, exe, sizeof(exe)))
    {
        if (lpszDir && *lpszDir) chdir(old_dir);
        return (HINSTANCE)SE_ERR_ASSOCINCOMPLETE;
    }

    if (!searchpath(exe, fullpath, sizeof(fullpath)))
    {
        if (lpszDir && *lpszDir) chdir(old_dir);
        return (HINSTANCE)SE_ERR_FNF;
    }

    lstrcpyn(lpResult, fullpath, MAX_PATH);
    hInst = (HINSTANCE)33;

    if (lpszDir && *lpszDir) chdir(old_dir);

    return hInst;
}
