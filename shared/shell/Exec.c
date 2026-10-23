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
    @brief ShellExecute, FindExecutable and executable search helpers.
*/

#include "Shell.h"

/*************************************************************************
 * ShellExecute	[SHELL.20]
 *
 */

/*! @brief Launch an executable via WinExec().

    @param[in] hWnd        Parent window handle (unused).
    @param[in] lpOperation Operation verb (unused; WinExec only opens).
    @param[in] lpFile      Path of the executable to launch.
    @param[in] lpParameters Command-line parameters (unused).
    @param[in] lpDirectory Working directory (unused).
    @param[in] nShowCmd    Show command passed to WinExec().

    @return Instance handle of the launched application, as returned by
            WinExec(); an SE_ERR_* value less than 32 on failure.
*/
HINSTANCE WINAPI ShellExecute(HWND hWnd, LPCSTR lpOperation, LPCSTR lpFile,
		LPCSTR lpParameters, LPCSTR lpDirectory, int nShowCmd)
{
    HINSTANCE hInst;
    //APISTR((LF_APICALL,
      //"ShellExecute(HWND=%x,LPCSTR=%x,LPCSTR=%s,LPCSTR=%x,LPCSTR=%s,int=%d)\n",
	//hWnd,lpOperation,lpFile,lpParameters,lpDirectory,nShowCmd));
	MessageBox(0, "ShellExecute", "ShellExecute", MB_OK);

    hInst = WinExec(lpFile,nShowCmd);
    //APISTR((LF_APIRET,"ShellExecute: returns HINSTANCE %x\n",hInst));
    return hInst;
}


/*! @brief Locate a file along %PATH% and copy the resolved path out.

    @p filename is far (LPCSTR) while _searchenv() expects a near string;
    the name is copied into a near buffer first. The result is copied
    into @p fullpath with truncation, so a small buffer is safe.

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
    char        nearpath[_MAX_PATH];

    if (!fullpath || size < 2) return FALSE;
    fullpath[0] = '\0';

    lstrcpyn(nearpath, filename, sizeof(nearpath));
    _searchenv(nearpath, "PATH", searchenv_buf);

    if (searchenv_buf[0] == '\0') return FALSE;

    lstrcpyn(fullpath, searchenv_buf, size);
    return TRUE;
}


/*************************************************************************
 *	SHELL_FindExecutable [Internal]
 *
 * Utility for code sharing between FindExecutable and ShellExecute
 * in:
 *      lpFile the name of a file
 *      lpVerb the operation on it (open)
 * out:
 *      lpResult a buffer, big enough :-(, to store the command to do the
 *              operation on the file
 *      key a buffer, big enough, to get the key name to do actually the
 *              command (it'll be used afterwards for more information
 *              on the operation)
 *
 * 1. Filename is empty?
 * 2. File name is executable?
 * 3. Executable found?
 * 4. Find association
 * 5. Find assicoated executable
 */

/*! @brief Find the executable associated with a file by extension.

    Checks win.ini [windows]/programs and [extensions] for the file's
    extension and writes the resulting command line into @p lpResult.

    @param[in]  lpPath    Current working directory (unused).
    @param[in]  lpFile    Path of the file whose association to resolve.
    @param[in]  lpVerb    Operation verb (e.g. "open"; unused here).
    @param[out] lpResult  Buffer receiving the resolved command or path.
    @param[in]  resultLen Size of @p lpResult (currently unused).

    @return An SE_ERR_* value, 33 on success, or ERROR_INVALID_PARAMETER.

    @retval ERROR_INVALID_PARAMETER @p lpResult was NULL.
    @retval SE_ERR_FNF              The file (or its association) was not found.
    @retval SE_ERR_NOASSOC          The file has no usable extension or association.
    @retval 33                      Success; greater than 32 by convention.
*/
static UINT SHELL_FindExecutable(LPCSTR lpPath, LPCSTR lpFile, LPCSTR lpVerb,
                                 LPSTR lpResult, int resultLen)
{
    LPSTR extension = NULL; /* pointer to file extension */
    char classname[256];     /* registry name for this file type */
    LONG  classnamelen = sizeof(classname); /* length of above */
    char command[1024];     /* command from registry */
    char wBuffer[256];      /* Used to GetProfileString */
    UINT  retval = SE_ERR_NOASSOC;
    LPSTR tok;              /* token pointer */
    char xlpFile[256];      /* result of SearchPath */
    DWORD attribs;           /* file attributes */

//    TRACE("%s\n", debugstr_w(lpFile));

    if (!lpResult)
        return ERROR_INVALID_PARAMETER;

    xlpFile[0] = '\0';
    lpResult[0] = '\0'; /* Start off with an empty return string */

    /* trap NULL parameters on entry */
    if (!lpFile)
    {
//        WARN("(lpFile=%s,lpResult=%s): NULL parameter\n",
//             debugstr_w(lpFile), debugstr_w(lpResult));
        return SE_ERR_FNF; /* File not found. */
    }

	/* Search file in paths */
	if (!searchpath(lpFile, xlpFile, sizeof(xlpFile)))
	    return SE_ERR_FNF;
#if 0
    attribs = GetFileAttributes(lpFile);
    if (attribs!=INVALID_FILE_ATTRIBUTES && (attribs&FILE_ATTRIBUTE_DIRECTORY))
    {
       lstrcpy(classname, "Folder");
    }
    else
#endif
    {
        /* First thing we need is the file's extension */
        extension = strrchr(xlpFile, '.'); /* Assume last "." is the one; */
        /* File->Run in progman uses */
        /* .\FILE.EXE :( */
//        TRACE("xlpFile=%s,extension=%s\n", debugstr_w(xlpFile), debugstr_w(extension));

        if (extension == NULL || extension[1]==0)
        {
//            WARN("Returning SE_ERR_NOASSOC\n");
            return SE_ERR_NOASSOC;
        }

        /* Three places to check: */
        /* 1. win.ini, [windows], programs (NB no leading '.') */
        /* 2. Registry, HKEY_CLASS_ROOT\<classname>\shell\open\command */
        /* 3. win.ini, [extensions], extension (NB no leading '.' */
        /* All I know of the order is that registry is checked before */
        /* extensions; however, it'd make sense to check the programs */
        /* section first, so that's what happens here. */

        /* See if it's a program - if GetProfileString fails, we skip this
         * section. Actually, if GetProfileString fails, we've probably
         * got a lot more to worry about than running a program... */
	LoadString(Globals.hInstance, IDS_WINDOWS, szWindows, sizeof(szWindows));
	LoadString(Globals.hInstance, IDS_PROGRAMS, szPrograms, sizeof(szPrograms));
	LoadString(Globals.hInstance, IDS_PROGRAMS_VALUE, szProgramsValue, sizeof(szProgramsValue));

        if (GetProfileString(szWindows, szPrograms, szProgramsValue, wBuffer, sizeof(wBuffer)) > 0)
        {
            AnsiLower(wBuffer);
            tok = wBuffer;
            while (*tok)
            {
                LPSTR p = tok;
                while (*p && *p != ' ' && *p != '\t') p++;
                if (*p)
                {
                    *p++ = 0;
                    while (*p == ' ' || *p == '\t') p++;
                }

                if (lstrcmp(tok, &extension[1]) == 0) /* have to skip the leading "." */
                {
                    lstrcpy(lpResult, xlpFile);
                    /* Need to perhaps check that the file has a path
                     * attached */
//                    TRACE("found %s\n", debugstr_w(lpResult));
                    return 33;
                    /* Greater than 32 to indicate success */
                }
                tok = p;
            }
        }
#if 0
        /* Check registry */
        if (RegQueryValue(HKEY_CLASSES_ROOT, extension, classname,
                           &classnamelen) == ERROR_SUCCESS)
        {
            classnamelen /= sizeof(char);
            if (classnamelen == ARRAY_SIZE(classname))
		classnamelen--;
            classname[classnamelen] = '\0';
//            TRACE("File type: %s\n", debugstr_w(classname));
        }
        else
#endif
        {
            *classname = '\0';
        }
    }

#if 0
    if (*classname)
    {
        /* pass the verb string to SHELL_FindExecutableByVerb() */
        retval = SHELL_FindExecutableByVerb(lpVerb, key, classname, command, sizeof(command));

	if (retval > 32)
	{
	    DWORD finishedLen;
	    SHELL_ArgifyW(lpResult, resultLen, command, xlpFile, pidl, args, &finishedLen);
	    if (finishedLen > resultLen)
		ERR("Argify buffer not large enough.. truncated\n");

	    /* Remove double quotation marks and command line arguments */
	    if (*lpResult == '"')
	    {
		WCHAR *p = lpResult;
		while (*(p + 1) != '"')
		{
		    *p = *(p + 1);
		    p++;
		}
		*p = '\0';
	    }
            else
            {
                /* Truncate on first space */
		char FAR *p = lpResult;
		while (*p != ' ' && *p != '\0')
                    p++;
                *p='\0';
            }
	}
    }
    else /* Check win.ini */
#endif
    {
	/* Toss the leading dot */
	LoadString(Globals.hInstance, IDS_EXTENSIONS, szExtensions, sizeof(szExtensions));
	extension++;
	if (GetProfileString(szExtensions, extension, "", command, sizeof(command)) > 0)
        {
            if (*command)
            {
                lstrcpy(lpResult, command);
                tok = lstrchr(lpResult, '^'); /* should be ^.extension? */
                if (tok != NULL)
                {
                    tok[0] = '\0';
                    lstrcat(lpResult, xlpFile); /* what if no dir in xlpFile? */
                    tok = lstrchr(command, '^'); /* see above */
                    if ((tok != NULL) && (lstrlen(tok)>5))
                    {
                        lstrcat(lpResult, &tok[5]);
                    }
                }
                retval = 33; /* FIXME - see above */
            }
        }
    }

//    TRACE("returning %s\n", debugstr_w(lpResult));
    return retval;
}

/*************************************************************************
 * FindExecutable	[SHELL.21]
 *
 * This function returns the executable associated with the specified file
 * for the default verb.
 *
 * PARAMS
 *  lpFile   [I] The file to find the association for. This must refer to
 *               an existing file otherwise FindExecutable fails and returns
 *               SE_ERR_FNF.
 *  lpResult [O] Points to a buffer into which the executable path is
 *               copied. This parameter must not be NULL otherwise
 *               FindExecutable() segfaults. The buffer must be of size at
 *               least MAX_PATH characters.
 *
 * RETURNS
 *  A value greater than 32 on success, less than or equal to 32 otherwise.
 *  See the SE_ERR_* constants.
 *
 * NOTES
 *  On Windows XP and 2003, FindExecutable() seems to first convert the
 *  filename into 8.3 format, thus taking into account only the first three
 *  characters of the extension, and expects to find an association for those.
 *  However other Windows versions behave sanely.
 */

/*! @brief Return the executable associated with the specified file.

    @param[in]  lpszFile Path of the file whose association to resolve.
    @param[in]  lpszDir  Optional working directory to switch to during
                         the lookup.
    @param[out] lpResult Buffer receiving the executable path.

    @return An HINSTANCE-style value: greater than 32 on success, or an
            SE_ERR_* value on failure.

    @retval SE_ERR_FNF @p lpszFile was NULL.
*/
HINSTANCE WINAPI FindExecutable(LPCSTR lpszFile, LPCSTR lpszDir, LPSTR lpResult)
{
    HINSTANCE retval = SE_ERR_NOASSOC;
    char old_dir[MAX_PATH];
    char nearDir[MAX_PATH];
    char res[MAX_PATH];

    //APISTR((LF_APISTUB,"FindExecutable(LPCSTR=%s,LPCSTR=%s,LPSTR=%x)\n",
	//lpszFile,lpszDir,lpResult));
	MessageBox(0, "FindExecutable", "FindExecutable", MB_OK);
   /* later, add a search for the application to match file if not .exe */

//    TRACE("File %s, Dir %s\n", debugstr_w(lpFile), debugstr_w(lpDirectory));

    lpResult[0] = '\0'; /* Start off with an empty return string */
    if (lpszFile == NULL)
	return (HINSTANCE)SE_ERR_FNF;

    if (lpszDir)
    {
        getcwd(old_dir, sizeof(old_dir));
        /* far -> near copy: chdir() only accepts near strings */
        lstrcpyn(nearDir, lpszDir, sizeof(nearDir));
        chdir(nearDir);
    }

	LoadString(Globals.hInstance, IDS_OPEN, szOpen, sizeof(szOpen));
    retval = SHELL_FindExecutable(lpszDir, lpszFile, szOpen, res, MAX_PATH);

    if (retval > 32)
        lstrcpy(lpResult, res);

    if (lpszDir)
        chdir(old_dir);

//    TRACE("returning %s\n", debugstr_w(lpResult));

    return (HINSTANCE)retval;
}
