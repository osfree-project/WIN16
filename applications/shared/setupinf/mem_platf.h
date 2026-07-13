#ifndef MEM_PLATF_H
#define MEM_PLATF_H

#if defined(_WINDOWS) || defined(__WINDOWS__)
  #include <windows.h>
  #define ALLOC(size)       ((LPSTR)GlobalLock( GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, (DWORD)(size))))
  #define FREE(ptr)         (GlobalUnlock((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(ptr)))),(BOOL)GlobalFree((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(ptr)))))
  #define REALLOC(ptr,sz)   (GlobalUnlock(GlobalHandle(SELECTOROF(ptr))), (LPSTR)GlobalLock(GlobalReAlloc(GlobalHandle(SELECTOROF(ptr)), (DWORD)(sz), GMEM_MOVEABLE)))
#else
  #include <malloc.h>
  #define ALLOC(size)       _fmalloc(size)
  #define FREE(ptr)         _ffree(ptr)
  #define REALLOC(ptr,sz)   _frealloc(ptr, sz)
#endif

#endif
