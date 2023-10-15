#include "Shell.h"

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
 * DoEnvironmentSubst      [SHELL.37]
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

	MessageBox(0, "DoEnvironmentSubst", "DoEnvironmentSubst", MB_OK);
  CharToOem(str,str);

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

  OemToChar(str,str);
  GlobalFreePtr(lpBuffer);

  return (DWORD)MAKELONG(retLength, retStatus);
}
