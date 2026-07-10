/* setupinf.c */
#include "setupinf_internal.h"

#if defined(__WINDOWS__) || defined(_WINDOWS)
int InfReadLine(INF_FILE_HANDLE f, LPSTR buf, int size) {
    static char szReadBuf[512];
    static int  nBufPos = 0, nBufSize = 0;
    int i = 0;
    char c;

    while (i < size - 1) {
        if (nBufPos >= nBufSize) {
            nBufSize = _lread(f, szReadBuf, sizeof(szReadBuf));
            if (nBufSize <= 0) break;
            nBufPos = 0;
        }
        c = szReadBuf[nBufPos++];
        if (c == '\r') {
            if (nBufPos < nBufSize && szReadBuf[nBufPos] == '\n')
                nBufPos++;
            break;
        } else if (c == '\n') {
            break;
        } else {
            buf[i++] = c;
        }
    }
    buf[i] = '\0';
    return (i > 0 || nBufSize > 0) ? 1 : 0;
}
#endif

HINF InfOpen(LPCSTR filename) {
    HINF hInf;
    LPSTR nameCopy;

    if (!filename) {
        return NULL;
    }

    hInf = (HINF)INF_ALLOC(sizeof(INF_FILE));
    if (!hInf) {
        return NULL;
    }

    nameCopy = (LPSTR)INF_ALLOC(STRLEN(filename) + 1);
    if (!nameCopy) {
        INF_FREE(hInf);
        return NULL;
    }
    STRCPY(nameCopy, filename);

    hInf->filename = nameCopy;
    hInf->firstSection = NULL;
    return hInf;
}

void InfClose(HINF hInf) {
    if (!hInf) return;
    InfClearAllCache(hInf);
    if (hInf->filename) INF_FREE(hInf->filename);
    INF_FREE(hInf);
}

void InfClearAllCache(HINF hInf) {
    LPINF_SECTION sec, next;
    int i;

    if (!hInf) return;

    sec = hInf->firstSection;
    while (sec) {
        next = sec->next;
        if (sec->name) INF_FREE(sec->name);
        if (sec->lines) {
            for (i = 0; i < sec->lineCount; i++) {
                if (sec->lines[i]) INF_FREE(sec->lines[i]);
            }
            INF_FREE(sec->lines);
        }
        INF_FREE(sec);
        sec = next;
    }
    hInf->firstSection = NULL;
}

void InfFreeSection(HINF hInf, LPCSTR sectionName) {
    LPINF_SECTION prev = NULL, sec;
    int i;

    if (!hInf || !sectionName) return;

    sec = hInf->firstSection;
    while (sec) {
        if (STRCMPI(sec->name, sectionName) == 0) {
            if (prev)
                prev->next = sec->next;
            else
                hInf->firstSection = sec->next;

            if (sec->name) INF_FREE(sec->name);
            if (sec->lines) {
                for (i = 0; i < sec->lineCount; i++)
                    if (sec->lines[i]) INF_FREE(sec->lines[i]);
                INF_FREE(sec->lines);
            }
            INF_FREE(sec);
            return;
        }
        prev = sec;
        sec = sec->next;
    }
}

LPINF_SECTION InfFindSection(HINF hInf, LPCSTR sectionName) {
    LPINF_SECTION sec;
    INF_FILE_HANDLE file;
    LPSTR         lineBuf;
    int           inTarget = 0;
    LPINF_SECTION newSec = NULL;

    if (!hInf || !sectionName) {
        return NULL;
    }

    sec = hInf->firstSection;
    while (sec) {
        if (STRCMPI(sec->name, sectionName) == 0) {
            return sec;
        }
        sec = sec->next;
    }

    file = INF_FILE_OPEN(hInf->filename);
    if (file == HFILE_ERROR) {
        return NULL;
    }

    lineBuf = (LPSTR)INF_ALLOC(256);
    if (!lineBuf) {
        INF_FILE_CLOSE(file);
        return NULL;
    }

    while (INF_READ_LINE(file, lineBuf, 256)) {
        if (lineBuf[0] == '[') {
            LPSTR pEnd = STRCHR(lineBuf, ']');
            if (pEnd) {
                *pEnd = '\0';
                if (STRCMPI(lineBuf + 1, sectionName) == 0) {
                    inTarget = 1;
                } else {
                    inTarget = 0;
                }
            }
        } else if (inTarget && lineBuf[0] != ';' && lineBuf[0] != '\0') {
            LPSTR lineCopy;

            if (!newSec) {
                newSec = (LPINF_SECTION)INF_ALLOC(sizeof(INF_SECTION));
                if (!newSec) break;
                newSec->name = (LPSTR)INF_ALLOC(STRLEN(sectionName) + 1);
                STRCPY(newSec->name, sectionName);
                newSec->lines = NULL;
                newSec->lineCount = 0;
                newSec->linesAllocated = 0;
                newSec->next = NULL;
            }
            if (newSec->lineCount >= newSec->linesAllocated) {
                int newAlloc;
                LPINF_LINE newLines;

                newAlloc = newSec->linesAllocated ? newSec->linesAllocated + 16 : 16;
		newLines = (newSec->linesAllocated ? ((LPINF_LINE)INF_REALLOC(newSec->lines, newAlloc * sizeof(INF_LINE))) : ((LPINF_LINE)INF_ALLOC(newAlloc * sizeof(INF_LINE))));
                if (!newLines) break;
                newSec->lines = newLines;
                newSec->linesAllocated = newAlloc;
            }
            lineCopy = (LPSTR)INF_ALLOC(STRLEN(lineBuf) + 1);
            if (!lineCopy) break;
            STRCPY(lineCopy, lineBuf);
            newSec->lines[newSec->lineCount++] = lineCopy;
        } else if (inTarget && lineBuf[0] == '[') {
            break;
        }
    }

    INF_FREE(lineBuf);
    INF_FILE_CLOSE(file);

    if (newSec) {
        newSec->next = hInf->firstSection;
        hInf->firstSection = newSec;
     
        return newSec;
    }
    return NULL;
}

int InfGetLineCount(LPINF_SECTION hSection) {
    return hSection ? hSection->lineCount : 0;
}

LPCSTR InfGetLine(LPINF_SECTION hSection, int index) {
    if (!hSection || index < 0 || index >= hSection->lineCount)
        return NULL;
    return hSection->lines[index];
}
