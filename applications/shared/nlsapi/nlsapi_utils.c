/*
 * nlsapi_utils.c – общие функции для работы с SETUP.INF
 */
#include "nlsapi_internal.h"
#include <ctype.h>

/* ------------------------------------------------------------ */
HFILE OpenSetupInf(void)
{
    static char szPath[144];
    OFSTRUCT of;
    GetSystemDirectory(szPath, sizeof(szPath) - 20);
    lstrcat(szPath, "\\SETUP.INF");
    return OpenFile(szPath, &of, OF_READ);
}

#if 0
/* ------------------------------------------------------------ */
BOOL EnumSetupInfSection(const char *sectionName,
                         SETUPINF_CALLBACK callback,
                         DWORD lParam)
{
    HFILE hFile;
    static char szReadBuf[512];
    static char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;
    BOOL bInSection = FALSE;
    BOOL bContinue = TRUE;
    char *p;
    char FAR *q;

    if (!callback) return FALSE;

    hFile = OpenSetupInf();
    if (hFile == HFILE_ERROR) return FALSE;

    while (!bEOF && bContinue) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) {
                szLine[nLinePos] = '\0';
                goto parse_line;
            }
            break;
        }

        for (i = 0; i < nRead && bContinue; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
parse_line:
                if (nLinePos > 0) {
                    p = szLine;
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p == '[') {
                        q = (char FAR *)_fstrchr(p + 1, ']');
                        if (q) {
                            *q = '\0';
                            bInSection = (lstrcmpi(p + 1, sectionName) == 0);
                        }
                    } else if (bInSection && *p != '\0' && *p != ';') {
                        if (!callback(p, lParam)) {
                            bContinue = FALSE;
                            break;
                        }
                    }
                }
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') i++;
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1)
                    szLine[nLinePos++] = c;
            }
        }
    }

    _lclose(hFile);
    return TRUE;
}

#endif

/* ------------------------------------------------------------ */
BOOL ParseCountryLine(LPCSTR line, LPSTR name, int nameSize, int FAR * lpCode, LPSTR lang, int langSize, LPSTR params, int paramsSize)
{
    const char FAR *p, FAR *q;
    int len;

    /* 1. Имя страны */
    p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '\"') return FALSE;
    p++;
    q = p;
    while (*q && *q != '\"') q++;
    len = (int)(q - p);
    if (len >= nameSize) len = nameSize - 1;
    if (len > 0) { int k; for (k = 0; k < len; k++) name[k] = p[k]; }
    name[len] = '\0';
    if (*q != '\"') return FALSE;
    q++;  /* за кавычку */

    /* 2. Параметры */
    while (*q == ' ' || *q == '\t' || *q == ',') q++;
    if (*q != '\"') return FALSE;
    q++;
    p = q;
    while (*q && *q != '\"') q++;
    len = (int)(q - p);
    if (len >= paramsSize) len = paramsSize - 1;
    if (len > 0) { int k; for (k = 0; k < len; k++) params[k] = p[k]; }
    params[len] = '\0';

    /* 3. Код страны (первое число до '!') */
    {
        int code = 0;
        const char FAR *pp = p;
        while (pp < q && *pp >= '0' && *pp <= '9') {
            code = code * 10 + (*pp - '0');
            pp++;
        }
        *lpCode = code;
    }


    /* 4. Код языка (последнее поле после последнего '!') */
    {
        const char FAR *lastExcl = NULL;
        const char FAR *pp;
        for (pp = q - 1; pp >= p; pp--) {
            if (*pp == '!') { lastExcl = pp; break; }
        }
        if (lastExcl && (lastExcl + 1) < q) {
            const char FAR *langStart = lastExcl + 1;
            int langLen = (int)(q - langStart);
            int i;
            if (langLen >= langSize) langLen = langSize - 1;
            for (i = 0; i < langLen; i++) lang[i] = langStart[i];
            lang[langLen] = '\0';
        } else {
            lang[0] = 'e'; lang[1] = 'n'; lang[2] = 'g'; lang[3] = '\0';
        }
    }

    return TRUE;
}

/* ------------------------------------------------------------ */
int AtoiFar(const char FAR *s)
{
    int result = 0;
    int sign = 1;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return sign * result;
}

void StringCopyN(LPSTR dest, LPCSTR src, int n)
{
    int i;
    if (n <= 0) return;
    for (i = 0; i < n - 1 && src[i] != '\0'; i++)
        dest[i] = src[i];
    dest[i] = '\0';
}

/* Проверка, состоит ли строка только из цифр */
BOOL IsDigitsOnly(LPCSTR str, int len)
{
    int i;
    for (i = 0; i < len; i++)
        if (!isdigit((unsigned char)str[i])) return FALSE;
    return TRUE;
}
