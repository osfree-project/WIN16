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

/*! @file Environment.c
    @brief DOS environment lookup and %KEYWORD% substitution.
*/

#include "Shell.h"

/*************************************************************************
 *				FindEnvironmentString	[SHELL.38]
 *
 * Returns a pointer into the DOS environment... Ugh.
 */

/*! @brief Return a pointer into the DOS environment for the given key.

    @param[in] lpEnv Environment block (double-NUL terminated).
    @param[in] entry Variable name to look for.

    @return Pointer past '=' (or past the name for empty entries), or NULL.

    @retval NULL Variable not found.
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

/*! @brief Return a pointer into the DOS environment for the given key.

    @param[in] str Variable name to look up.

    @return Pointer to the value string, or NULL if not found.

    @retval 0 Variable not found.
*/
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

/*! @brief Substitute %KEYWORD% tokens in a string from the DOS environment.

    Replaces %KEYWORD% in @p str with the value of the variable KEYWORD
    from the "DOS" environment. If the variable is not found, the
    %KEYWORD% token is left intact. If the buffer is too small, @p str is
    not modified.

    @param[in,out] str    '\0'-terminated string with %keyword% tokens;
                          replaced in-place on success.
    @param[in]     length Size of @p str in bytes.

    @return LOWORD is the resulting length, HIWORD is 1 on success,
            0 on failure.
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
  AnsiToOem(str,str);

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

  OemToAnsi(str,str);
  GlobalFreePtr(lpBuffer);

  return (DWORD)MAKELONG(retLength, retStatus);
}
