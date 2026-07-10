#include "string_utils.h"
#include "mem_platf.h"
#include <ctype.h>

void StrTrimInternal(LPSTR str) {
    LPSTR start;
    LPSTR end;
    int len;

    if (!str) return;

    while (*str == ' ' || *str == '\t') str++;
    if (*str == '\0') return;

    end = str + STRLEN(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }

    len = STRLEN(str);
    if (len >= 2 && str[0] == '"' && str[len-1] == '"') {
        MEMMOVE(str, str+1, len-2);
        str[len-2] = '\0';
    }
}

LPSTR StrDup(LPCSTR s) {
    LPSTR d;
    int len;
    if (!s) return NULL;
    len = STRLEN(s);
    d = (LPSTR)ALLOC(len + 1);
    if (d) {
        STRCPY(d, s);
    }
    return d;
}
