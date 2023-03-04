#include <stdio.h>
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

typedef VOID const far *VA_LIST;

#define __VA_ROUNDED(type) \
    ((sizeof(type) + sizeof(WORD) - 1) / sizeof(WORD) * sizeof(WORD))
#define VA_ARG(list,type) \
    (((list) = (VA_LIST)((char far *)(list) + __VA_ROUNDED(type))), \
     *((type far *)(void far *)((char far *)(list) - __VA_ROUNDED(type))))

/***********************************************************************
 * Helper for wsprintf16
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

static int parse_format( LPCSTR format, WPRINTF_FORMAT *res )
{
    LPCSTR p = format;

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
    return (int)(p - format) + 1;
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

    while (*spec)
    {
        if (*spec != '%') { *p++ = *spec++; continue; }
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
            len = sprintf( number, "%d", int_view );
            break;
        case WPR_UNSIGNED:
            if (format.flags & WPRINTF_LONG) int_view = VA_ARG( args, UINT );
            else int_view = VA_ARG( args, UINT );
            len = sprintf( number, "%u", int_view );
            break;
        case WPR_HEXA:
            if (format.flags & WPRINTF_LONG) int_view = VA_ARG( args, UINT );
            else int_view = VA_ARG( args, UINT );
            len = sprintf( number, (format.flags & WPRINTF_UPPER_HEX) ? "%X" : "%x", int_view);
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
            /* wsprintf16 ignores null characters */
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
    return p - buffer;
}


/***********************************************************************
 *           _wsprintf   (USER.420)
 */
int WINAPI _wsprintf( LPSTR buffer, LPCSTR spec, VA_LIST valist )
{
    return wvsprintf( buffer, spec, valist );
}

/***********************************************************************
 *           LSTRCMPI   (USER.471)
 */
int WINAPI lstrcmpi(LPCSTR lpszString1,LPCSTR lpszString2)
{
    if (!lpszString1 || !lpszString2)
	return (lpszString2-lpszString1);

//    LOGSTR((LF_API,"lstrcmpi: %s %s\n",lpszString1,lpszString2));

    return _fstricmp(lpszString1,lpszString2);
}
