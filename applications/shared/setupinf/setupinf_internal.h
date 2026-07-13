/* setupinf_internal.h */
#ifndef SETUPINF_INTERNAL_H
#define SETUPINF_INTERNAL_H

#include "setupinf.h"
#include "string_utils.h"
#include "mem_platf.h"

#if defined(_WINDOWS) || defined(__WINDOWS__)
  #include <windows.h>

  typedef HFILE INF_FILE_HANDLE;

  int InfReadLine(INF_FILE_HANDLE f, LPSTR buf, int size);
  #define INF_READ_LINE(f,b,s) InfReadLine(f,b,s)
  #define INF_FILE_OPEN(n)    _lopen((LPSTR)(n), READ)
  #define INF_FILE_CLOSE(h)   _lclose(h)
#else
  #include <stdio.h>

  typedef FILE* INF_FILE_HANDLE;
  #define INF_READ_LINE(f,b,s)  (fgets((b),(s),(f)) ? 1 : 0)
  #define INF_FILE_OPEN(n)    fopen((n), "rt")
  #define INF_FILE_CLOSE(h)   fclose(h)
  #define HFILE_ERROR NULL
#endif

typedef LPSTR INF_LINE;
typedef INF_LINE FAR * LPINF_LINE;

typedef struct INF_SECTION_TAG {
    LPSTR          name;
    LPINF_LINE     lines;
    int            lineCount;
    int            linesAllocated;
    LPINF_SECTION  next;
} INF_SECTION;

typedef struct INF_FILE_TAG {
    LPSTR           filename;
    LPINF_SECTION   firstSection;
} INF_FILE;

#endif
