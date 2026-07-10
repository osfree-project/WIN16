#include "nlsapi_internal.h"

/*

int WINAPI DECLSPEC GetKeyboardLayoutList(int nBuff, LPSTR lpList)
{
    HFILE hFile;
    static char szReadBuf[512];
    static char szLine[256];
    int nLinePos = 0;
    int nRead, i;
    BOOL bEOF = FALSE;
    BOOL bInKbd = FALSE;
    char FAR *p, FAR *q, FAR *p1, FAR *p2;
    static char szName[64];
    int len;
    static char szPrevNames[64][64];
    int nPrev = 0, nCount = 0;
    LPSTR lpDest = lpList;
    int nRemain = (lpList && nBuff > 0) ? nBuff : 0;
    int j;
    BOOL bDup;

    hFile = OpenSetupInf();
    if (hFile == HFILE_ERROR) return 0;

    while (!bEOF) {
        nRead = _lread(hFile, szReadBuf, sizeof(szReadBuf));
        if (nRead == HFILE_ERROR || nRead == 0) {
            bEOF = TRUE;
            if (nLinePos > 0) { szLine[nLinePos] = '\0'; goto parse_kbd; }
            break;
        }
        for (i = 0; i < nRead; i++) {
            char c = szReadBuf[i];
            if (c == '\r' || c == '\n') {
                szLine[nLinePos] = '\0';
parse_kbd:
                if (nLinePos > 0) {
                    p = szLine; while (*p == ' ' || *p == '\t') p++;
                    if (*p == '[') {
                        q = (char FAR *)_fstrchr(p + 1, ']');
                        if (q) { *q = '\0'; bInKbd = (lstrcmpi(p + 1, "keyboard.tables") == 0); }
                    } else if (bInKbd && *p != '\0' && *p != ';') {
                        p1 = (char FAR *)_fstrchr(p, '\"');
                        if (p1) {
                            p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                            if (p2) {
                                len = 0;
                                { char FAR *tmp = p1 + 1; while (tmp < p2 && len < 63) szName[len++] = *tmp++; }
                                szName[len] = '\0';
                                bDup = FALSE;
                                for (j = 0; j < nPrev; j++) {
                                    if (lstrcmpi(szPrevNames[j], szName) == 0) { bDup = TRUE; break; }
                                }
                                if (!bDup) {
                                    if (nPrev < 64) { lstrcpy(szPrevNames[nPrev], szName); nPrev++; }
                                }
                            }
                        }
                    }
                }
                nLinePos = 0;
                if (c == '\r' && i + 1 < nRead && szReadBuf[i + 1] == '\n') i++;
            } else {
                if (nLinePos < (int)sizeof(szLine) - 1) szLine[nLinePos++] = c;
            }
        }
    }
    _lclose(hFile);

    nCount = nPrev;
    if (!lpList || nBuff <= 0) return nCount;
    for (j = 0; j < nCount; j++) {
        int cb = lstrlen(szPrevNames[j]) + 1;
        if (cb <= nRemain) { lstrcpy(lpDest, szPrevNames[j]); lpDest += cb; nRemain -= cb; }
        else break;
    }
    if (nRemain >= 1) *lpDest = '\0';
    return nCount;
}

*/

int WINAPI DECLSPEC GetKeyboardLayoutList(int nBuff, LPSTR lpList)
{
    LPINF_SECTION sec;
    int i, count;
    LPCSTR line;
    static char szName[64];
    static char szPrevNames[64][64];
    int nPrev = 0, nCount = 0;
    LPSTR lpDest = lpList;
    int nRemain = (lpList && nBuff > 0) ? nBuff : 0;
    int j;
    BOOL bDup;

    if (!g_hInf) return 0;

    sec = InfFindSection(g_hInf, "keyboard.tables");
    if (!sec) return 0;

    count = InfGetLineCount(sec);
    for (i = 0; i < count; i++)
    {
        line = InfGetLine(sec, i);
        if (!line) continue;
        while (*line == ' ' || *line == '\t') line++;
        if (*line == ';' || *line == '\0') continue;

        {
            char FAR *p1 = (char FAR *)_fstrchr(line, '\"');
            if (p1)
            {
                char FAR *p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                if (p2)
                {
                    int len = 0;
                    char FAR *tmp = p1 + 1;
                    while (tmp < p2 && len < 63)
                        szName[len++] = *tmp++;
                    szName[len] = '\0';

                    bDup = FALSE;
                    for (j = 0; j < nPrev; j++)
                    {
                        if (lstrcmpi(szPrevNames[j], szName) == 0)
                        {
                            bDup = TRUE;
                            break;
                        }
                    }
                    if (!bDup)
                    {
                        if (nPrev < 64)
                        {
                            lstrcpy(szPrevNames[nPrev], szName);
                            nPrev++;
                        }
                    }
                }
            }
        }
    }

    nCount = nPrev;
    if (!lpList || nBuff <= 0) return nCount;
    for (j = 0; j < nCount; j++)
    {
        int cb = lstrlen(szPrevNames[j]) + 1;
        if (cb <= nRemain)
        {
            lstrcpy(lpDest, szPrevNames[j]);
            lpDest += cb;
            nRemain -= cb;
        }
        else break;
    }
    if (nRemain >= 1) *lpDest = '\0';
    return nCount;
}
