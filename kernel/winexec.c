#include <windows.h>

#include "win_private.h"

// This is same paramblock as for int 21h ax=4b00h, but instead of FCB1 pointer to
// nCmdShow, and FCB2=NULL.

typedef
struct tagLOADPARMS  {
    WORD      segEnv;         /* child environment  */
    LPSTR     lpszCmdLine;    /* child command tail */
    WORD FAR* lpShow;         /* how to show child  */
    UINT FAR* lpReserved;     /* must be NULL       */
} LOADPARMS;


/***********************************************************************
 *           WinExec     (KERNEL.166)
 */
HINSTANCE WINAPI WinExec(LPCSTR lpCmdLine, UINT nCmdShow)
{
    LPCSTR p, args = NULL;
    LPCSTR name_beg, name_end;
    LPSTR name, cmdline;
    int arglen;
    HINSTANCE ret;
    char buffer[255]; // MAX_PATH
    LOADPARMS params;
    WORD showCmd[2];

    if (*lpCmdLine == '"') /* has to be only one and only at beginning ! */
    {
        name_beg = lpCmdLine+1;
        p = strchr ( lpCmdLine+1, '"' );
        if (p)
        {
            name_end = p;
            args = strchr ( p, ' ' );
        }
        else /* yes, even valid with trailing '"' missing */
            name_end = lpCmdLine+lstrlen(lpCmdLine);
    }
    else
    {
        name_beg = lpCmdLine;
        args = strchr( lpCmdLine, ' ' );
        name_end = args ? args : lpCmdLine+lstrlen(lpCmdLine);
    }

    if ((name_beg == lpCmdLine) && (!args))
    { /* just use the original cmdline string as file name */
        name = (LPSTR)lpCmdLine;
    }
    else
    {
        if (!(name = GlobalAllocPtr(GPTR, name_end - name_beg + 1 ))) ;
            //return ERROR_NOT_ENOUGH_MEMORY;
        memcpy(name, (LPSTR)name_beg, name_end - name_beg );
        name[name_end - name_beg] = '\0';
    }

    if (args)
    {
        args++;
        arglen = lstrlen(args);
        cmdline = GlobalAllocPtr( GPTR, 2 + arglen );
        cmdline[0] = (BYTE)arglen;
        lstrcpy( cmdline + 1, args );
    }
    else
    {
        cmdline = GlobalAllocPtr( GPTR, 2 );
        cmdline[0] = cmdline[1] = 0;
    }

    //TRACE("name: '%s', cmdline: '%.*s'\n", name, cmdline[0], &cmdline[1]);

    showCmd[0] = 2;
    showCmd[1] = nCmdShow;

    params.segEnv = 0;
    params.lpszCmdLine = cmdline;
    params.lpShow = &showCmd;
    params.lpReserved = 0;

    ret = LoadModule(buffer, &params);

    GlobalFreePtr(cmdline);

    if (name != lpCmdLine) GlobalFreePtr(name);

    return ret;
}

