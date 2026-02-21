/*

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<https://www.gnu.org/licenses/>.

*/
#include <stdarg.h>

#include <user.h>

/***********************************************************************
 *           lstrcmp   (USER.430)
 */
int WINAPI lstrcmp( LPCSTR str1, LPCSTR str2 )
{
//	FUNCTION_START

	while (*str1 || *str2)
		{
		if (*str1 < *str2)
		{
			FUNCTION_END
			return -1;
		}
		else if (*str1 > *str2)
		{
			FUNCTION_END
			return 1;
		}
		str1 ++;
		str2 ++;
	}

//	FUNCTION_END
	return 0;
}

/***********************************************************************
 *           AnsiUpper   (USER.431)
 */

LPSTR WINAPI AnsiUpper( LPSTR strOrChar )
{
//	FUNCTION_START
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
//	FUNCTION_START
    /* lowercase only one char if strOrChar < 0x10000 */
    if (HIWORD(strOrChar))
    {
        char far *s = strOrChar;
        while (*s)
        {
            *s = _tolower(*s);
            s++;
        }
        return strOrChar;
    }
    else return (LPSTR)_tolower((char)strOrChar);
}

/***********************************************************************
 *           AnsiNext   (USER.472)
 */
LPSTR WINAPI AnsiNext(LPCSTR lpchCurrentChar)
{
//	FUNCTION_START
    if (!lpchCurrentChar)
	return (LPSTR)0;

    if (*lpchCurrentChar) {
	if ( IsDBCSLeadByte(*lpchCurrentChar) )
	    return (LPSTR)(lpchCurrentChar+2);
	else
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

//	FUNCTION_START

    if (lpchStart == lpchCurrentChar)
	{
		FUNCTION_END
		return (LPSTR)lpchStart;
	}

//    if ( SetCodePage() ) {
//	while ((lpNext = AnsiNext((LPCSTR)lpPrev)) != (LPSTR)lpchCurrentChar)
//	    lpPrev = lpNext;
//	return lpPrev;
//    }

//	FUNCTION_END
	return (LPSTR)(lpchCurrentChar-1);
}

typedef VOID const far *VA_LIST;

#define __VA_ROUNDED(type) \
    ((sizeof(type) + sizeof(WORD) - 1) / sizeof(WORD) * sizeof(WORD))
#define VA_ARG(list,type) \
    (((list) = (VA_LIST)((char far *)(list) + __VA_ROUNDED(type))), \
     *((type far *)(void far *)((char far *)(list) - __VA_ROUNDED(type))))

/***********************************************************************
 * Helper for wsprintf
 *
 * int parse_format( LPCSTR format, WPRINTF_FORMAT *res )
 *
 * format - format-control string
 * res - result flags structure 
 *
 * Returns size of parsed part of format-control string
 *
 * The format-control string contains format specifications that determine 
 * the output format for the arguments following the lpFmt parameter. 
 * Format specifications, discussed below, always begin with a percent 
 * sign (%). If a percent sign is followed by a character that has no 
 * meaning as a format field, the character is not formatted 
 * (for example, %% produces a single percent-sign character).
 *
 * The format-control string is read from left to right. When the first format
 * specification (if any) is encountered, it causes the value of the first 
 * argument after the format-control string to be converted and copied 
 * to the output buffer according to the format specification. The second
 * format specification causes the second argument to be converted and 
 * copied, and so on. If there are more arguments than format specifications,
 * the extra arguments are ignored. If there are not enough arguments for 
 * all of the format specifications, the results are undefined.
 *
 * A format specification has the following form:
 *
 * %[-][#][0][width][.precision]type
 *
 * Each field is a single character or a number signifying a particular 
 * format option. The type characters that appear after the last 
 * optional format field determine whether the associated argument
 * is interpreted as a character, a string, or a number. The simplest
 * format specification contains only the percent sign and a type 
 * character (for example, %s). The optional fields control other 
 * aspects of the formatting. Following are the optional and 
 * required fields and their meanings.
 *
 * +------------+-----------------------------------------------------+
 * ! Field      ! Meaning                                             !
 * +------------+-----------------------------------------------------+
 * ! -          ! Pad the output with blanks or zeros to the right to !
 * !            ! fill the field width, justifying output to the left.!
 * !            ! If this field is omitted, the output is padded to   !
 * !            ! the left, justifying it to the right.               !
 * +------------+-----------------------------------------------------+
 * ! #          ! Prefix hexadecimal values with 0x (lowercase) or 0X !
 * !            ! (uppercase).                                        !
 * +------------+-----------------------------------------------------+
 * ! 0          ! Pad the output value with zeros to fill the field   !
 * !            ! width. If this field is omitted, the output value   !
 * !            ! is padded with blank spaces.                        !
 * +------------+-----------------------------------------------------+
 * ! width      ! Copy the specified minimum number of characters to  !
 * !            ! the output buffer. The width field is a nonnegative !
 * !            ! integer. The width specification never causes a     !
 * !            ! value to be truncated; if the number of characters  !
 * !            ! in the output value is greater than the specified   !
 * !            ! width, or if the width field is not present, all    !
 * !            ! characters of the value are printed, subject to the !
 * !            ! precision specification.                            !
 * +------------+-----------------------------------------------------+
 * ! .precision ! For numbers, copy the specified minimum number of   !
 * !            ! digits to the output buffer. If the number of       !
 * !            ! digits in the argument is less than the specified   !
 * !            ! precision, the output value is padded on the left   !
 * !            ! with zeros. The value is not truncated when the     !
 * !            ! number of digits exceeds the specified precision.   !
 * !            ! If the specified precision is 0 or omitted entirely,!
 * !            ! or if the period (.) appears without a number       !
 * !            ! following it, the precision is set to 1.            !
 * !            ! For strings, copy the specified maximum number of   !
 * !            ! characters to the output buffer.                    !
 * +------------+-----------------------------------------------------+
 * ! type       ! Output the corresponding argument as a character, a !
 * !            ! string, or a number. This field can be any of the   !
 * !            ! following values.                                   !
 * +------------+-----------------------------------------------------+
 *
 * +------------+-----------------------------------------------------+
 * ! Type       ! Meaning                                             !
 * +------------+-----------------------------------------------------+
 * ! c          ! Single character. This value is interpreted as type !
 * !            ! char.                                               !
 * +------------+-----------------------------------------------------+
 * ! C          ! Single character. This value is interpreted as type !
 * !            ! char (for win32 as WCHAR).                          !
 * +------------+-----------------------------------------------------+
 * ! d          ! Signed decimal integer. This value is equivalent to !
 * !            ! i.                                                  !
 * +------------+-----------------------------------------------------+
hc, hC
    Single character. The wsprintf function ignores character arguments with a numeric value of zero. This value is always interpreted as type CHAR, even when the calling application defines Unicode. 
hd
    Signed short integer argument. 
hs, hS
    String. This value is always interpreted as type LPSTR, even when the calling application defines Unicode. 
hu
    Unsigned short integer. 
i
    Signed decimal integer. This value is equivalent to d. 
Ix, IX
    64-bit unsigned hexadecimal integer in lowercase or uppercase on 64-bit platforms, 32-bit unsigned hexadecimal integer in lowercase or uppercase on 32-bit platforms. 
lc, lC
    Single character. The wsprintf function ignores character arguments with a numeric value of zero. This value is always interpreted as type WCHAR, even when the calling application does not define Unicode. 
ld
    Long signed integer. This value is equivalent to li. 
li
    Long signed integer. This value is equivalent to ld. 
ls, lS
    String. This value is always interpreted as type LPWSTR, even when the calling application does not define Unicode. This value is equivalent to ws. 
lu
    Long unsigned integer. 
lx, lX
    Long unsigned hexadecimal integer in lowercase or uppercase. 
p
    Pointer. The address is printed using hexadecimal. 
s
    String. This value is interpreted as type LPWSTR when the calling application defines Unicode and as type LPSTR otherwise. 
S
    String. This value is interpreted as type LPSTR when the calling application defines Unicode and as type LPWSTR otherwise. 
u
    Unsigned integer argument. 
x, X
    Unsigned hexadecimal integer in lowercase or uppercase. 

  */
  

#define WPRINTF_LEFTALIGN   0x0001  /* Align output on the left ('-' prefix) */
#define WPRINTF_PREFIX_HEX  0x0002  /* Prefix hex with 0x ('#' prefix) */
#define WPRINTF_ZEROPAD     0x0004  /* Pad with zeros ('0' prefix) */
#define WPRINTF_LONG        0x0008  /* Long arg ('l' prefix) */
#define WPRINTF_SHORT       0x0010  /* Short arg ('h' prefix) */
#define WPRINTF_UPPER_HEX   0x0020  /* Upper-case hex ('X' specifier) */

typedef enum
{
    WPR_UNKNOWN,
    WPR_CHAR,
    WPR_STRING,
    WPR_SIGNED,
    WPR_UNSIGNED,
    WPR_HEXA
} WPRINTF_TYPE;

typedef struct
{
    UINT         flags;
    UINT         width;
    UINT         precision;
    WPRINTF_TYPE type;
} WPRINTF_FORMAT;

static int parse_format( LPCSTR format, WPRINTF_FORMAT FAR *res )
{
    LPCSTR p = format;

//	FUNCTION_START

    res->flags = 0;
    res->width = 0;
    res->precision = 0;
    if (*p == '-') { res->flags |= WPRINTF_LEFTALIGN; p++; }
    if (*p == '#') { res->flags |= WPRINTF_PREFIX_HEX; p++; }
    if (*p == '0') { res->flags |= WPRINTF_ZEROPAD; p++; }
    while ((*p >= '0') && (*p <= '9'))  /* width field */
    {
        res->width = res->width * 10 + *p - '0';
        p++;
    }
    if (*p == '.')  /* precision field */
    {
        p++;
        while ((*p >= '0') && (*p <= '9'))
        {
            res->precision = res->precision * 10 + *p - '0';
            p++;
        }
    }
    if (*p == 'l') { res->flags |= WPRINTF_LONG; p++; }
    else if (*p == 'h') { res->flags |= WPRINTF_SHORT; p++; }
    switch(*p)
    {
    case 'c':
    case 'C':  /* no Unicode in Win16 */
        res->type = WPR_CHAR;
        break;
    case 's':
    case 'S':
        res->type = WPR_STRING;
        break;
    case 'd':
    case 'i':
        res->type = WPR_SIGNED;
        break;
    case 'u':
        res->type = WPR_UNSIGNED;
        break;
    case 'p':
        res->width = 8;
        res->flags |= WPRINTF_ZEROPAD;
        /* fall through */
    case 'X':
        res->flags |= WPRINTF_UPPER_HEX;
        /* fall through */
    case 'x':
        res->type = WPR_HEXA;
        break;
    default: /* unknown format char */
        res->type = WPR_UNKNOWN;
        p--;  /* print format as normal char */
        break;
    }
	
//	FUNCTION_END
    return (p - format) + 1;
}

/***********************************************************************
 *           wvsprintf   (USER.421)
 */
int WINAPI wvsprintf( LPSTR buffer, LPCSTR spec, VA_LIST args )
{
    WPRINTF_FORMAT format;
    LPSTR p = buffer;
    UINT i, len, sign;
    char number[20];
    char char_view = 0;
    LPCSTR lpcstr_view = NULL;
    int int_view;
    LPCSTR seg_str;

//	FUNCTION_START

    while (*spec)
    {
		if (IsBadWritePtr(buffer, 1))
		{
			OutputDebugString("oops 1\r\n");
		};
		if (IsBadReadPtr(spec, 1))
		{
			OutputDebugString("oops 2\r\n");
		};
        if (*spec != '%') { *p = *spec; p++; spec++; continue; }
        spec++;
        if (*spec == '%') { *p++ = *spec++; continue; }
        spec += parse_format( spec, &format );
        switch(format.type)
        {
        case WPR_CHAR:
            char_view = VA_ARG( args, char );
            len = format.precision = 1;
            break;
        case WPR_STRING:
            seg_str = VA_ARG( args, LPCSTR );
            if (IsBadReadPtr( seg_str, 1 )) lpcstr_view = "";
            else lpcstr_view = seg_str;
            if (!lpcstr_view) lpcstr_view = "(null)";
            for (len = 0; !format.precision || (len < format.precision); len++)
                if (!lpcstr_view[len]) break;
            format.precision = len;
            break;
        case WPR_SIGNED:
            if (format.flags & WPRINTF_LONG) int_view = VA_ARG( args, int );
            else int_view = VA_ARG( args, int );
			lstrcpy(number, itoa(int_view));
            len = lstrlen(number);
            break;
        case WPR_UNSIGNED:
            if (format.flags & WPRINTF_LONG) int_view = VA_ARG( args, UINT );
            else int_view = VA_ARG( args, UINT );
			lstrcpy(number, uitoa(int_view));
			len = lstrlen(number);
            break;
        case WPR_HEXA:
            if (format.flags & WPRINTF_LONG) int_view = VA_ARG( args, UINT );
            else int_view = VA_ARG( args, UINT );
			lstrcpy(number, itox(int_view));
            len = lstrlen(number);//sprintf( number, (format.flags & WPRINTF_UPPER_HEX) ? "%X" : "%x", int_view);
            break;
        case WPR_UNKNOWN:
            continue;
        }
        if (format.precision < len) format.precision = len;
        if (format.flags & WPRINTF_LEFTALIGN) format.flags &= ~WPRINTF_ZEROPAD;
        if ((format.flags & WPRINTF_ZEROPAD) && (format.width > format.precision))
            format.precision = format.width;
        if (format.flags & WPRINTF_PREFIX_HEX) len += 2;

        sign = 0;
        if (!(format.flags & WPRINTF_LEFTALIGN))
            for (i = format.precision; i < format.width; i++) *p++ = ' ';

		
        switch(format.type)
        {
        case WPR_CHAR:
            *p = char_view;
            /* wsprintf ignores null characters */
            if (*p != '\0') p++;
            else if (format.width > 1) *p++ = ' ';
            break;
        case WPR_STRING:
            if (len) _fmemcpy( p, lpcstr_view, len );
            p += len;
            break;
        case WPR_HEXA:
            if (format.flags & WPRINTF_PREFIX_HEX)
            {
                *p++ = '0';
                *p++ = (format.flags & WPRINTF_UPPER_HEX) ? 'X' : 'x';
                len -= 2;
            }
            /* fall through */
        case WPR_SIGNED:
            /* Transfer the sign now, just in case it will be zero-padded*/
            if (number[0] == '-')
            {
                *p++ = '-';
                sign = 1;
            }
            /* fall through */
        case WPR_UNSIGNED:
            for (i = len; i < format.precision; i++) *p++ = '0';
            if (len > sign) _fmemcpy( p, number + sign, len - sign );
            p += len-sign;
            break;
        case WPR_UNKNOWN:
            continue;
        }
        if (format.flags & WPRINTF_LEFTALIGN)
            for (i = format.precision; i < format.width; i++) *p++ = ' ';
    }
    *p = 0;

//	FUNCTION_END
    return p - buffer;
}


/***********************************************************************
 *           wsprintf   (USER.420)
 */
int FAR CDECL wsprintf( LPSTR buffer, LPCSTR spec, ... )
{
	int rc;
	va_list valist;

//	FUNCTION_START

	va_start(valist, spec);
	rc=wvsprintf( buffer, spec, valist );
	va_end(valist);

//	FUNCTION_END
	return rc;
}

/***********************************************************************
 *           LSTRCMPI   (USER.471)
 */
int WINAPI lstrcmpi(LPCSTR lpszString1,LPCSTR lpszString2)
{
	int rc;

//	FUNCTION_START

	if (!lpszString1 || !lpszString2)
	return (lpszString2-lpszString1);

	while (_tolower((unsigned char) *lpszString1) == _tolower((unsigned char) *lpszString2)) {
		if (*lpszString1 == '\0')
			return 0;
		lpszString1++; lpszString2++;
	}

	rc=(int) _tolower((unsigned char) *lpszString1) -
		(int) _tolower((unsigned char) *lpszString2);

//	FUNCTION_END
	return rc;
}

BOOL    WINAPI 
IsCharUpper(char ch)
{
//	FUNCTION_START
//	APISTR((LF_API,"IsCharUpper(char %c)\n",ch));
	return (BOOL) _isupper((int) ch);	
}

BOOL    WINAPI 
IsCharLower(char ch)
{
//	FUNCTION_START
//	APISTR((LF_API,"IsCharLower(char %c)\n",ch));
	return (BOOL) _islower((int) ch);	
}

BOOL    WINAPI 
IsCharAlpha(char ch)
{
//	FUNCTION_START
//	APISTR((LF_API,"IsCharAlpha(char=%c)\n",ch));
	return (BOOL) isalpha((int) ch);
}

BOOL    WINAPI 
IsCharAlphaNumeric(char ch)
{
//	FUNCTION_START
//	APISTR((LF_API,"IsCharAlphaNumeric(char=%c)\n",ch));
	return (BOOL) isalnum((int) ch);
}

UINT    WINAPI 
AnsiUpperBuff(LPSTR lpstr, UINT n)
{
	unsigned long count = n;

//	FUNCTION_START
	
	if(count == 0)
		count = 65536;	
	while(count) {
		*lpstr = toupper(*lpstr);	
		lpstr++;
		count--;
	}
	return n;
}

UINT    WINAPI 
AnsiLowerBuff(LPSTR lpstr, UINT n) 
{
	unsigned long count = n;

//	FUNCTION_START
	
	if(count == 0)
		count = 65536;	
	while(count) {
		*lpstr = _tolower(*lpstr);	
		lpstr++;
		count--;
	}
	return n;
}



int _tolower (int c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));

  return c;
}

int toupper (int c)
{
  if (c >= 'a' && c <= 'z')
    return (c + ('A' - 'a'));

  return c;
}

void far * _fmemset (void far *start, int c, unsigned int len)
{
  char far *p = start;

  while (len -- > 0)
    *p ++ = c;

  return start;
}

void far * _fmemcpy(void far * s1, void const far * s2, unsigned length)
{	
	char far * p;
	char far * q;

//	FUNCTION_START
	//TRACE(s2);
	if(length) {
		p = s1;
		q = (char far *)s2;
		do *p++ = *q++;
		while(--length);
	}

//	FUNCTION_END
	return s1;
}

int _fstrnicmp(char const far *s1, const char far *s2, unsigned int n)
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

int _islower(int c)
{
	if (c >= 'a' && c <= 'z')
		return (1);
	else
		return (0);
}

int _isupper(int c)
{
	if (c >= 'A' && c <= 'Z')
		return (1);
	else
		return (0);
}

int isalpha(int c)
{
	return _isupper(c) || _islower(c);
}

int isdigit(int c)
{
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}


char far *itoa(int i)
{
  static char buf[12]={0};
  char far * p = buf + sizeof(buf) -1;
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

char far *uitoa(unsigned int i)
{
  static char buf[12]={0};
  char far *p = buf + sizeof(buf)-1;
  do {
    *--p = '0' + (i % 10);
    i /= 10;
  } while (i != 0);
  return p;
}

char far * itox(int i) 
{
  char hexdigits[] = "0123456789ABCDEF";
  static char buf[12]={0};
  char far *p = buf + sizeof(buf)-1;
  do {
    *--p = hexdigits[i % 16];
    i /= 16;
  } while (i != 0);
  return p;
}

void WINAPI StringFunc(void)
{
//	FUNCTION_START
}

char far * lstrchr (const char far *s, int c)
{
  do {
    if (*s == c)
      {
	return (char far *)s;
      }
  } while (*s++);
  return (0);
}

int latoi(const char far *h)
{
  char far *s = (char far *)h;
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

  l = lstrlen(s) - 1;

  while (*s) {
    c = _tolower(*s);

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
