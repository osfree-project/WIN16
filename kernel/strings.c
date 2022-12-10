#include <win16.h>

/***********************************************************************
 *		Reserved5 (KERNEL.87)
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

LPSTR WINAPI lstrcat( LPSTR dst, LPCSTR t )
{
    LPSTR s;
    s = dst;
    while( *s != '\0' )
        ++s;
    while( *s++ = *t++ )
        ;
    return( dst );
}

LPSTR WINAPI lstrcpyn( LPSTR dst, LPCSTR src, int n )
{
    LPSTR d = dst;
    LPCSTR s = src;
    UINT count = n;

    while ((count > 1) && *s)
    {
        count--;
        *d++ = *s++;
    }
    if (count) *d = 0;

    return dst;
}

int WINAPI lstrlen( LPCSTR s )
{
    LPCSTR p;

    p = s;
    while( *p != '\0' )
        ++p;
    return( p - s );

}

LPSTR WINAPI lstrcpy( LPSTR s, LPCSTR t )
{
    LPSTR dst;
    dst = s;
    while( *dst++ = *t++ )
        ;
    return( s );
}

int strlen (const char *str)
{
   int len = 0;

  while (*str++)
    len++;

  return len;
}

int toupper (int c)
{
  if (c >= 'a' && c <= 'z')
    return (c + ('A' - 'a'));

  return c;
}

int tolower (int c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));

  return c;
}

int strnicmp(char far *s1, const char far *s2, int n)
{

    if (n == 0)
	return 0;
    do {
	if (toupper(*s1) != toupper(*s2++))
	    return toupper(*(unsigned char *) s1) -
		toupper(*(unsigned char *) --s2);
	if (*s1++ == 0)
	    break;
    } while (--n != 0);
    return 0;
}

int stricmp(const char* s1, const char* s2) 
{
  while (tolower((unsigned char) *s1) == tolower((unsigned char) *s2)) {
    if (*s1 == '\0')
      return 0;
    s1++; s2++;
  }

  return (int) tolower((unsigned char) *s1) -
    (int) tolower((unsigned char) *s2);
}

/*
 * 16-bit kernel initialization code
 *
 * Copyright 2000 Alexandre Julliard
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

#include <i86.h>
#include <win16.h>

#include <win_private.h>

int tolower (int c);
int toupper (int c);

/***********************************************************************
 *		Reserved1 (KERNEL.77)
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
 *		Reserved2(KERNEL.78)
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

/***********************************************************************
 *		Reserved3 (KERNEL.79)
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
 *		Reserved4 (KERNEL.80)
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

int atoi(char *h)
{
  char *s = h;
  int  i = 0;
  int  j, k, l;
  char c;
  int  base;

  if (s[0] == '0' && s[1] == 'x') {
    base = 16;
    s += 2; // Delete "0x"
  } else {
    base = 10;
  }

  l = strlen(s) - 1;

  while (*s) {
    c = tolower(*s);

    if ('a' <= c && c <= 'f') {
      if (base == 16) {
        c = c - 'a' + 10;
      } else {
        return 0;
      }
    } else if ('0' <= c && c <= '9') {
      c -= '0';
    } else {
      return 0;
    }

    for (j = 0, k = c; j < l; j++)
      k *= base;

    i += k;
    s++;
    l--;
  }

  return i;
}
