#include "Shell.h"

/*************************************************************************
 * ShellExecute	[SHELL.20]
 *
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


char *searchpath(const char *filename)
{
	char fullpath[_MAX_PATH];
	_searchenv(filename, "PATH", fullpath);
	if (fullpath[0] == '\0') return NULL;
	return fullpath;
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
static UINT SHELL_FindExecutable(LPCSTR lpPath, LPCSTR lpFile, LPCSTR lpVerb,
                                 LPSTR lpResult, int resultLen)
{
    char *extension = NULL; /* pointer to file extension */
    char classname[256];     /* registry name for this file type */
    LONG  classnamelen = sizeof(classname); /* length of above */
    char command[1024];     /* command from registry */
    char wBuffer[256];      /* Used to GetProfileString */
    UINT  retval = SE_ERR_NOASSOC;
    char *tok;              /* token pointer */
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

	// Search file in paths
	lstrcpy(xlpFile, searchpath(lpFile));
#if 0
    attribs = GetFileAttributes(lpFile);
    if (attribs!=INVALID_FILE_ATTRIBUTES && (attribs&FILE_ATTRIBUTE_DIRECTORY))
    {
       lstrcpy(classname, "Folder");
    }
    else
#endif
    {
        /* Did we get something? Anything? */
        if (xlpFile[0]==0)
        {
//            TRACE("Returning SE_ERR_FNF\n");
            return SE_ERR_FNF;
        }
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
        if (GetProfileString("windows", "programs", "com exe bat pif", wBuffer, sizeof(wBuffer)) > 0)
        {
            AnsiLower(wBuffer);
            tok = wBuffer;
            while (*tok)
            {
                char *p = tok;
                while (*p && *p != ' ' && *p != '\t') p++;
                if (*p)
                {
                    *p++ = 0;
                    while (*p == ' ' || *p == '\t') p++;
                }

                if (strcmp(tok, &extension[1]) == 0) /* have to skip the leading "." */
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
	extension++;
	if (GetProfileString("extensions", extension, "", command, sizeof(command)) > 0)
        {
            if (*command)
            {
                lstrcpy(lpResult, command);
                tok = strchr(lpResult, '^'); /* should be ^.extension? */
                if (tok != NULL)
                {
                    tok[0] = '\0';
                    lstrcat(lpResult, xlpFile); /* what if no dir in xlpFile? */
                    tok = strchr(command, '^'); /* see above */
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
HINSTANCE WINAPI FindExecutable(LPCSTR lpszFile, LPCSTR lpszDir, LPSTR lpResult)
{
    HINSTANCE retval = SE_ERR_NOASSOC;
    char old_dir[1024];
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
        chdir(lpszDir);
    }

    retval = SHELL_FindExecutable(lpszDir, lpszFile, "open", res, MAX_PATH);

    if (retval > 32)
        lstrcpy(lpResult, res);

    if (lpszDir)
        chdir(old_dir);

//    TRACE("returning %s\n", debugstr_w(lpResult));

    return (HINSTANCE)retval;
}

