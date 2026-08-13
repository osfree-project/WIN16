#include "setupinf_internal.h"

BOOL InfParseLanguageLine(LPCSTR line, LANGUAGE_ENTRY FAR *entry)
{
    LPSTR  p, q;
    LPSTR  dllStr;
    int    len;

    if (!line || !entry) return FALSE;

    MEMSET(entry, 0, sizeof(LANGUAGE_ENTRY));

    /* 1. Код языка */
    p = (LPSTR)line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == ';' || *p == '\0') return FALSE;

    q = STRCHR(p, '=');
    if (!q) return FALSE;

    /* Длина кода */
    {
        LPSTR codeEnd = q;
        while (codeEnd > p && (*(codeEnd-1) == ' ' || *(codeEnd-1) == '\t'))
            codeEnd--;
        len = (int)(codeEnd - p);
        entry->code = (LPSTR)ALLOC(len + 1);
        if (!entry->code) goto error;
        if (len > 0) MEMMOVE(entry->code, p, len);
        entry->code[len] = '\0';
    }

    /* 2. DLL (между '=' и ',') */
    p = q + 1;
    while (*p == ' ' || *p == '\t') p++;

    q = STRCHR(p, ',');
    if (q) {
        len = (int)(q - p);
    } else {
        len = STRLEN(p);
    }

    /* Удаляем пробелы в конце DLL */
    while (len > 0 && (p[len-1] == ' ' || p[len-1] == '\t'))
        len--;
    /* Удаляем пробелы в начале */
    while (len > 0 && (*p == ' ' || *p == '\t')) {
        p++;
        len--;
    }

    if (len > 0) {
        dllStr = (LPSTR)ALLOC(len + 1);
        if (!dllStr) goto error;
        MEMMOVE(dllStr, p, len);
        dllStr[len] = '\0';

        /* Разделяем диск и файл */
        {
            LPSTR colon = STRCHR(dllStr, ':');
            if (colon) {
                *colon = '\0';
                /* Диск */
                if (STRLEN(dllStr) == 1 && dllStr[0] >= '1' && dllStr[0] <= '9')
                    entry->disk = dllStr[0] - '0';
                else if (STRLEN(dllStr) == 1 && dllStr[0] >= 'A' && dllStr[0] <= 'Z')
                    entry->disk = 10 + (dllStr[0] - 'A');
                else
                    entry->disk = 0;

                /* Файл */
                {
                    LPSTR filePart = colon + 1;
                    while (*filePart == ' ' || *filePart == '\t') filePart++;
                    len = STRLEN(filePart);
                    entry->file = (LPSTR)ALLOC(len + 1);
                    if (!entry->file) {
                        FREE(dllStr);
                        goto error;
                    }
                    if (len > 0) MEMMOVE(entry->file, filePart, len);
                    entry->file[len] = '\0';
                }
            } else {
                /* Нет двоеточия – нет диска */
                entry->disk = 0;
                entry->file = (LPSTR)ALLOC(len + 1);
                if (!entry->file) {
                    FREE(dllStr);
                    goto error;
                }
                MEMMOVE(entry->file, dllStr, len);
                entry->file[len] = '\0';
            }
        }
        FREE(dllStr);
    } else {
        /* DLL отсутствует – пустая строка */
        entry->disk = 0;
        entry->file = (LPSTR)ALLOC(1);
        if (!entry->file) goto error;
        entry->file[0] = '\0';
    }

    /* 3. Описание (после запятой) */
    if (q) {
        p = q + 1;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\"') {
            p++;
            q = STRCHR(p, '\"');
            if (q) {
                len = (int)(q - p);
            } else {
                len = STRLEN(p);
            }
            entry->description = (LPSTR)ALLOC(len + 1);
            if (!entry->description) goto error;
            if (len > 0) MEMMOVE(entry->description, p, len);
            entry->description[len] = '\0';
        } else {
            entry->description = NULL;
        }
    } else {
        entry->description = NULL;
    }

    return TRUE;

error:
    InfFreeLanguageEntry(entry);
    return FALSE;
}

void InfFreeLanguageEntry(LANGUAGE_ENTRY FAR *entry)
{
    if (!entry) return;

    if (entry->code)        { FREE(entry->code);        entry->code        = NULL; }
    if (entry->file)        { FREE(entry->file);        entry->file        = NULL; }
    if (entry->description) { FREE(entry->description); entry->description = NULL; }

    MEMSET(entry, 0, sizeof(LANGUAGE_ENTRY));
}
