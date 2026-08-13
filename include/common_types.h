#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#if defined(_WINDOWS) || defined(__WINDOWS__)
  #include <windows.h>
#else
  typedef unsigned long  DWORD;
  typedef unsigned int   UINT;
  typedef int            BOOL;
  typedef char far       *LPSTR;
  typedef const char far *LPCSTR;
  #define FAR            far
  #define CALLBACK        __far __pascal
#endif

#endif
