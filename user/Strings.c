#include <string.h>
#include <ctype.h>

#include <windows.h>

/**********************************************************************
 *     LoadString   (USER.176)
 */
int WINAPI LoadString( HINSTANCE instance, UINT resource_id, LPSTR buffer, int buflen )
{
    HGLOBAL hmem;
    HRSRC hrsrc;
    unsigned char far *p;
    int string_num;
    int ret;

//    TRACE("inst=%04x id=%04x buff=%p len=%d\n", instance, resource_id, buffer, buflen);

    hrsrc = FindResource( instance, MAKEINTRESOURCE((resource_id>>4)+1), (LPSTR)RT_STRING );
    if (!hrsrc) return 0;
    hmem = LoadResource( instance, hrsrc );
    if (!hmem) return 0;

    p = LockResource(hmem);
    string_num = resource_id & 0x000f;
    while (string_num--) p += *p + 1;

    if (buffer == NULL) ret = *p;
    else
    {
        ret = min(buflen - 1, *p);
        if (ret > 0)
        {
            _fmemcpy(buffer, p + 1, ret);
            buffer[ret] = '\0';
        }
        else if (buflen > 1)
        {
	    buffer[0] = '\0';
	    ret = 0;
	}
//        TRACE( "%s loaded\n", debugstr_a(buffer));
    }
    FreeResource( hmem );
    return ret;
}

/***********************************************************************
 *           lstrcmp   (USER.430)
 */
int WINAPI lstrcmp( LPCSTR str1, LPCSTR str2 )
{
  while (*str1 || *str2)
    {
      if (*str1 < *str2)
        return -1;
      else if (*str1 > *str2)
        return 1;
      str1 ++;
      str2 ++;
    }

  return 0;
}

/***********************************************************************
 *           AnsiUpper   (USER.431)
 */

LPSTR WINAPI AnsiUpper( LPSTR strOrChar )
{
    /* uppercase only one char if strOrChar < 0x10000 */
    if (HIWORD(strOrChar))
    {
        char far *s = strOrChar;
        while (*s)
        {
            *s = toupper(*s);
            s++;
        }
        return strOrChar;
    }
    else return (LPSTR)toupper((char)strOrChar);
}

/***********************************************************************
 *           AnsiLower   (USER.432)
 */
LPSTR WINAPI AnsiLower( LPSTR strOrChar )
{
    /* lowercase only one char if strOrChar < 0x10000 */
    if (HIWORD(strOrChar))
    {
        char far *s = strOrChar;
        while (*s)
        {
            *s = tolower(*s);
            s++;
        }
        return strOrChar;
    }
    else return (LPSTR)tolower((char)strOrChar);
}

/***********************************************************************
 *           AnsiNext   (USER.472)
 */
LPSTR WINAPI AnsiNext(LPCSTR lpchCurrentChar)
{
    if (!lpchCurrentChar)
	return (LPSTR)0;

    if (*lpchCurrentChar) {
//	if ( IsDBCSLeadByte(*lpchCurrentChar) )
//	    return (LPSTR)(lpchCurrentChar+2);
//	else
	    return (LPSTR)(lpchCurrentChar+1);
    }
    else
	return (LPSTR)lpchCurrentChar;
}

/***********************************************************************
 *           AnsiPrev   (USER.473)
 */
LPSTR WINAPI AnsiPrev(LPCSTR lpchStart, LPCSTR lpchCurrentChar)
{
    LPSTR lpPrev = (LPSTR)lpchStart;
    LPSTR lpNext;

    if (lpchStart == lpchCurrentChar)
	return (LPSTR)lpchStart;

//    if ( SetCodePage() ) {
//	while ((lpNext = AnsiNext((LPCSTR)lpPrev)) != (LPSTR)lpchCurrentChar)
//	    lpPrev = lpNext;
//	return lpPrev;
//    }

    return (LPSTR)(lpchCurrentChar-1);
}
