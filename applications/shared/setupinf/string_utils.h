#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "common_types.h"
#include <string.h>

#if defined(_WINDOWS) || defined(__WINDOWS__)
  #include <windows.h>
  #define STRLEN(s)       lstrlen(s)
  #define STRCPY(d,s)     lstrcpy(d,s)
  #define STRCMP(s1,s2)   lstrcmp(s1,s2)
  #define STRCMPI(s1,s2)  lstrcmpi(s1,s2)
  #define STRCHR(s,c)     _fstrchr(s,c)
  #define STRRCHR(s,c)    _fstrrchr(s,c)
  #define MEMMOVE(d,s,l)  _fmemmove(d,s,l)
  #define STRDUP(s)       StrDup(s)
#else
  #define STRLEN(s)       _fstrlen(s)
  #define STRCPY(d,s)     _fstrcpy(d,s)
  #define STRCMP(s1,s2)   _fstrcmp(s1,s2)
  #define STRCMPI(s1,s2)  _fstricmp(s1,s2)
  #define STRCHR(s,c)     _fstrchr(s,c)
  #define STRRCHR(s,c)    _fstrrchr(s,c)
  #define MEMMOVE(d,s,l)  _fmemmove(d,s,l)
  #define STRDUP(s)       _fstrdup(s)
#endif

void StrTrimInternal(LPSTR str);
#define STRTRIM(s)        StrTrimInternal(s)

LPSTR StrDup(LPCSTR s);

#endif
