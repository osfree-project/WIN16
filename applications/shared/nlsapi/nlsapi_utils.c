/*
 * nlsapi_utils.c – общие функции для работы с SETUP.INF
 */
#include "nlsapi_internal.h"

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
//                        /* Покажем raw-строку для отладки */
//                        {
//                            char szMsg[300];
//                            wsprintf(szMsg, "Raw line: %s", (LPSTR)p);
//                            MessageBox(0, szMsg, "EnumSystemLocales: raw line", MB_OK);
//                        }

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

//                                {
//                                    char szMsg[128];
//                                    wsprintf(szMsg, "Country name: '%s'", (LPSTR)name);
//                                    MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                }

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

                                        /* Трассировка параметров */
//                                        {
//                                            char szMsg[300];
//                                            wsprintf(szMsg, "Params: '%s'", (LPSTR)params);
//                                            MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                        }

    /* 3. Код страны (первое число до '!') */
    {
        int code = 0;
        const char FAR *pp = p;
        while (pp < q && *pp >= '0' && *pp <= '9') {
            code = code * 10 + (*pp - '0');
            pp++;
        }
        *lpCode = code;

//                                        {
//                                            char szMsg[128];
//                                            wsprintf(szMsg, "code=%d", *lpCode);
//                                            MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                        }

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
                                        /* Трассировка: код страны и язык */
//                                        {
//                                            char szMsg[128];
//                                            wsprintf(szMsg, "lang='%s'", (LPSTR)lang);
//                                            MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                        }

    return TRUE;
}


#if 0 //!!!!!
                        p1 = (char FAR *)_fstrchr(p, '\"');
                        if (p1) {
                            p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                            if (p2) {
                                /* ----- Извлекаем имя страны (первая пара кавычек) ----- */
                                nameLen = 0;
                                {
                                    char FAR *tmp = p1 + 1;
                                    while (tmp < p2 && nameLen < 63) {
                                        nameLen++;
                                        tmp++;
                                    }
                                }
                                {
                                    int k;
                                    for (k = 0; k < nameLen; k++) {
                                        szCountryName[k] = (char)(*(p1 + 1 + k));
                                    }
                                    szCountryName[nameLen] = '\0';
                                }

                                /* Покажем имя страны и его длину */
//                                {
//                                    char szMsg[128];
//                                    wsprintf(szMsg, "Country name length: %d", nameLen);
//                                    MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                }
//                                {
//                                    char szMsg[128];
//                                    wsprintf(szMsg, "Country name: '%s'", (LPSTR)szCountryName);
//                                    MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                }

                                /* ----- Ищем параметры (вторая пара кавычек) ----- */
                                p1 = (char FAR *)_fstrchr(p2 + 1, '\"');
                                if (p1) {
//                                    MessageBox(0, "EnumSystemLocales: found start of params", "Trace", MB_OK);
                                    p2 = (char FAR *)_fstrchr(p1 + 1, '\"');
                                    if (p2) {
//                                        MessageBox(0, "EnumSystemLocales: found end of params", "Trace", MB_OK);

                                        /* Безопасное копирование параметров через индекс */
                                        len = 0;
                                        {
                                            const char FAR *src = p1 + 1;
                                            while (src + len < p2 && len < 255) {
                                                len++;
                                            }
                                        }
                                        {
                                            int k;
                                            for (k = 0; k < len; k++) {
                                                szParams[k] = (char)(*(p1 + 1 + k));
                                            }
                                        }
                                        szParams[len] = '\0';

                                        /* Трассировка параметров */
//                                        {
//                                            char szMsg[300];
//                                            wsprintf(szMsg, "Params: '%s'", (LPSTR)szParams);
//                                            MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                        }

                                        /* извлекаем код страны (первое поле) */
                                        pos = 0;
                                        while (pos < len && szParams[pos] != '!' && pos < 15)
                                        {
                                            szCode[pos] = szParams[pos];
                                            pos++;
                                        }
                                        szCode[pos] = '\0';
//                                        MessageBox(0, (LPSTR)szCode, "EnumSystemLocales", MB_OK);
                                        /* Трассировка извлечённого кода */
//                                        {
//                                            char szMsg[128];
//                                            wsprintf(szMsg, "Code substring: '%s', pos=%d", (LPSTR)szCode, pos);
//                                            MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                        }

                                        code = AtoiFar(szCode);

//                                        {
//                                            char szMsg[128];
//                                            wsprintf(szMsg, "code=%d", code);
//                                            MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                        }
                                        /* ===================================================== */

                                        /* извлекаем код языка (последнее поле после последнего '!') */
                                        totalLen = lstrlen((LPSTR)szParams);
                                        pos = totalLen - 1;
                                        while (pos >= 0 && szParams[pos] != '!') {
                                            pos--;
                                        }
                                        if (pos >= 0 && pos < totalLen - 1) {
                                            langLen = 0;
                                            pos++;
                                            while (pos < totalLen && langLen < 7) {
                                                szLang[langLen++] = szParams[pos++];
                                            }
                                            szLang[langLen] = '\0';
                                        } else {
                                            lstrcpy(szLang, "eng");
                                        }

                                        /* Трассировка: код страны и язык */
//                                        {
//                                            char szMsg[128];
//                                            wsprintf(szMsg, "code=%d, lang='%s'", code, (LPSTR)szLang);
//                                            MessageBox(0, szMsg, "EnumSystemLocales", MB_OK);
//                                        }

#endif //!!!!!
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
