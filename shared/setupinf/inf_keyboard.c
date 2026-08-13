#include "setupinf_internal.h"

BOOL InfParseKeyboardLine(LPCSTR line, KEYBOARD_ENTRY FAR *entry)
{
    LPSTR  p, q;
    LPSTR  dllEnd, colon, fileStart;
    int    len, diskLen;

    if (!line || !entry) {
        return FALSE;
    }

    MEMSET(entry, 0, sizeof(KEYBOARD_ENTRY));

    p = (LPSTR)line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == ';' || *p == '\0') return FALSE;

    q = STRCHR(p, '=');
    if (!q) return FALSE;

    /* 1. Код */
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

    /* 2. DLL */
    p = q + 1;
    while (*p == ' ' || *p == '\t') p++;

    q = STRCHR(p, ',');
    if (!q) q = p + STRLEN(p);

    dllEnd = q;
    while (dllEnd > p && (*(dllEnd-1) == ' ' || *(dllEnd-1) == '\t'))
        dllEnd--;
    while (p < dllEnd && (*p == ' ' || *p == '\t'))
        p++;

    if (p < dllEnd) {
        colon = STRCHR(p, ':');
        if (colon && colon < dllEnd) {
            diskLen = (int)(colon - p);
            if (diskLen == 1 && *p >= '1' && *p <= '9')
                entry->disk = *p - '0';
            else if (diskLen == 1 && *p >= 'A' && *p <= 'Z')
                entry->disk = 10 + (*p - 'A');
            else
                entry->disk = 0;
            fileStart = colon + 1;
            while (fileStart < dllEnd && (*fileStart == ' ' || *fileStart == '\t'))
                fileStart++;
            len = (int)(dllEnd - fileStart);
            entry->file = (LPSTR)ALLOC(len + 1);
            if (!entry->file) goto error;
            if (len > 0) MEMMOVE(entry->file, fileStart, len);
            entry->file[len] = '\0';
        } else {
            entry->disk = 0;
            len = (int)(dllEnd - p);
            entry->file = (LPSTR)ALLOC(len + 1);
            if (!entry->file) goto error;
            if (len > 0) MEMMOVE(entry->file, p, len);
            entry->file[len] = '\0';
        }
    } else {
        entry->disk = 0;
        entry->file = (LPSTR)ALLOC(1);
        if (!entry->file) goto error;
        entry->file[0] = '\0';
    }

    /* 3. Description */
    if (*q == ',') {
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
            len = STRLEN(p);
            entry->description = (LPSTR)ALLOC(len + 1);
            if (!entry->description) goto error;
            if (len > 0) MEMMOVE(entry->description, p, len);
            entry->description[len] = '\0';
        }
    } else {
        entry->description = NULL;
    }
    return TRUE;

error:
    InfFreeKeyboardEntry(entry);
    return FALSE;
}

void InfFreeKeyboardEntry(KEYBOARD_ENTRY FAR *entry)
{
    if (!entry) return;
    if (entry->code)        { FREE(entry->code);        entry->code        = NULL; }
    if (entry->file)        { FREE(entry->file);        entry->file        = NULL; }
    if (entry->description) { FREE(entry->description); entry->description = NULL; }
    MEMSET(entry, 0, sizeof(KEYBOARD_ENTRY));
}
