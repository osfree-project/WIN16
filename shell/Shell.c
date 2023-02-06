/*    
	Shell.c	2.13
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

/*
 * 				Shell Library Functions
 *
 * Copyright 1998 Marcus Meissner
 * Copyright 2000 Juergen Schmied
 * Copyright 2002 Eric Pouech
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


#include <string.h>

#include "win16.h"
//#include "windowsx.h"
/*
#include "shellapi.h"
*/
//#define __SHELLAPI_H__
//#include "Willows.h"

//#include "Log.h"
//#include "kerndef.h"
//#include "KrnAtoms.h"

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

void lmemcpy(void far * s1, void far * s2, unsigned length);
int lstrnicmp(char far *s1, const char far *s2, int n);

HICON WINAPI
ExtractIcon(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex)
{
//    APISTR((LF_APISTUB,"ExtractIcon(HINSTANCE=%x,LPCSTR=%s,UINT=%x0\n",
//	hInst,lpszExeFileName?lpszExeFileName:"NULL",nIconIndex));
    return (HICON)0;
}

HINSTANCE WINAPI
ShellExecute(HWND hWnd, LPCSTR lpOperation, LPCSTR lpFile,
		LPCSTR lpParameters, LPCSTR lpDirectory, int nShowCmd)
{
    HINSTANCE hInst;
    //APISTR((LF_APICALL,
      //"ShellExecute(HWND=%x,LPCSTR=%x,LPCSTR=%s,LPCSTR=%x,LPCSTR=%s,int=%d)\n",
	//hWnd,lpOperation,lpFile,lpParameters,lpDirectory,nShowCmd));
    hInst = WinExec(lpFile,nShowCmd);
    //APISTR((LF_APIRET,"ShellExecute: returns HINSTANCE %x\n",hInst));
    return hInst;
}

HINSTANCE WINAPI
FindExecutable(LPCSTR lpszFile, LPCSTR lpszDir, LPSTR lpResult)
{
    //APISTR((LF_APISTUB,"FindExecutable(LPCSTR=%s,LPCSTR=%s,LPSTR=%x)\n",
	//lpszFile,lpszDir,lpResult));
   /* later, add a search for the application to match file if not .exe */
    lstrcpy(lpResult,lpszFile);
    return (HINSTANCE) 33;
}

void WINAPI
RegisterShellHook(HWND hWnd, int foo)
{
    //APISTR((LF_APISTUB,"RegisterShellHook(HWND=%x,int=%d)\n",hWnd,foo));
}

/* (WIN32) File Security *************************************************** */
#if 0   
BOOL    WINAPI
GetFileSecurity(LPCSTR lpszFile,
        SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd,
        DWORD dw, LPDWORD lp)
{
        //APISTR((LF_APISTUB, "GetFileSecurity(LPCSTR=%s,...)\n", lpszFile));
        return (TRUE);
}       
   
BOOL    WINAPI
SetFileSecurity(LPCSTR lpszFile,
        SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd)
{   
        //APISTR((LF_APISTUB, "SetFileSecurity(LPCSTR=%s,...)\n", lpszFile));
        return (TRUE);
}
#endif

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

/*************************************************************************
 *				FindEnvironmentString	[SHELL.38]
 *
 * Returns a pointer into the DOS environment... Ugh.
 */
static LPSTR SHELL_FindString(LPSTR lpEnv, LPCSTR entry)
{ UINT l;

//  TRACE("\n");

  l = lstrlen(entry);
  for( ; *lpEnv ; lpEnv+=lstrlen(lpEnv)+1 )
  { if( lstrnicmp(lpEnv, entry, l) )
      continue;
	if( !*(lpEnv+l) )
	    return (lpEnv + l); 		/* empty entry */
	else if ( *(lpEnv+l)== '=' )
	    return (lpEnv + l + 1);
    }
    return NULL;
}

/**********************************************************************/

LPSTR WINAPI FindEnvironmentString(LPCSTR str)
{ LPSTR  spEnv;
  LPSTR lpEnv,lpString;
  //TRACE("\n");

  lpEnv = GetDOSEnvironment();

  lpString = (lpEnv)?SHELL_FindString(lpEnv, str):NULL;

    if( lpString )
	return lpString;

    return 0;
}

/*************************************************************************
 *              		DoEnvironmentSubst      [SHELL.37]
 *
 * Replace %KEYWORD% in the str with the value of variable KEYWORD
 * from "DOS" environment. If it is not found the %KEYWORD% is left
 * intact. If the buffer is too small, str is not modified.
 *
 * PARAMS
 *  str        [I] '\0' terminated string with %keyword%.
 *             [O] '\0' terminated string with %keyword% substituted.
 *  length     [I] size of str.
 *
 * RETURNS
 *  str length in the LOWORD and 1 in HIWORD if subst was successful.
 */
DWORD WINAPI DoEnvironmentSubst(LPSTR str,WORD length)
{
  LPSTR   lpEnv = GetDOSEnvironment();
  LPSTR   lpstr = str;
  LPSTR   lpend;
  LPSTR   lpBuffer = GlobalAllocPtr( GPTR, length);
  WORD    bufCnt = 0;
  WORD    envKeyLen;
  LPSTR   lpKey;
  WORD    retStatus = 0;
  WORD    retLength = length;

//  CharToOemA(str,str);

//  TRACE("accept %s\n", str);

  while( *lpstr && bufCnt <= length - 1 ) {
     if ( *lpstr != '%' ) {
        lpBuffer[bufCnt++] = *lpstr++;
        continue;
     }

     for( lpend = lpstr + 1; *lpend && *lpend != '%'; lpend++) /**/;

     envKeyLen = lpend - lpstr - 1;
     if( *lpend != '%' || envKeyLen == 0)
        goto err; /* "%\0" or "%%" found; back off and whine */

     *lpend = '\0';
     lpKey = SHELL_FindString(lpEnv, lpstr+1);
     *lpend = '%';
     if( lpKey ) {
         int l = lstrlen(lpKey);

         if( bufCnt + l > length - 1 )
                goto err;

        lmemcpy(lpBuffer + bufCnt, lpKey, l);
        bufCnt += l;
     } else { /* Keyword not found; Leave the %KEYWORD% intact */
        if( bufCnt + envKeyLen + 2 > length - 1 )
            goto err;

         lmemcpy(lpBuffer + bufCnt, lpstr, envKeyLen + 2);
        bufCnt += envKeyLen + 2;
     }

     lpstr = lpend + 1;
  }

  if (!*lpstr && bufCnt <= length - 1) {
      lmemcpy(str,lpBuffer, bufCnt);
      str[bufCnt] = '\0';
      retLength = bufCnt + 1;
      retStatus = 1;
  }

  err:
//  if (!retStatus)
//      WARN("-- Env subst aborted - string too short or invalid input\n");
//  TRACE("-- return %s\n", str);

//  OemToCharA(str,str);
  GlobalFreePtr(lpBuffer);

  return (DWORD)MAKELONG(retLength, retStatus);
}

#pragma off (unreferenced);
BOOL FAR PASCAL LibMain( HINSTANCE hInstance, WORD wDataSegment,
                         WORD wHeapSize, LPSTR lpszCmdLine )
#pragma on (unreferenced);
{
  short i;
  char buf[128];

  //BreakPoint();
//  i = GetSS();

//  sprintf( buf, "DLL16 Started");
//  MessageBox( NULL, buf, "LZEXPAND", MB_OK | MB_TASKMODAL );

  return( 1 );
}
