/* inf_country.c – окончательная реализация InfParseCountryLine */
#include "setupinf.h"
#include "string_utils.h"
#include "mem_platf.h"
#include "setupinf_internal.h"

static int AtoiFarLocal(LPCSTR s)
{
    int result = 0;
    int sign = 1;

    if (!s) return 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;

    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return sign * result;
}

BOOL InfParseCountryLine(LPCSTR line, COUNTRY_ENTRY FAR *entry)
{
    LPCSTR p, q, end;
    LPSTR  tmp;
    int    len;
    int    tokIdx = 0;
    LPCSTR tokStart;

    if (!line || !entry) {
        return FALSE;
    }

    MEMSET(entry, 0, sizeof(COUNTRY_ENTRY));

    /* 1. Название страны */
    p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '\"') {
        return FALSE;
    }
    p++;
    q = STRCHR(p, '\"');
    if (!q) {
        return FALSE;
    }
    len = (int)(q - p);
    tmp = (LPSTR)ALLOC(len + 1);
    if (!tmp) {
        return FALSE;
    }
    MEMMOVE(tmp, p, len);
    tmp[len] = '\0';
    entry->name = tmp;

    /* 2. Переход к параметрам */
    p = q + 1;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != ',') {
        goto error;
    }
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '\"') {
        goto error;
    }
    p++;
    q = STRCHR(p, '\"');
    if (!q) {
        goto error;
    }
    end = q;

    /* 3. Разбор 21 токена (индексы 0..20) */
    tokStart = p;
    while (tokStart <= end && tokIdx < 21)
    {
        LPCSTR tokEnd = STRCHR(tokStart, '!');
        if (!tokEnd || tokEnd > end) tokEnd = end;

        len = (int)(tokEnd - tokStart);

        tmp = (LPSTR)ALLOC(len + 1);
        if (!tmp) {
            goto error;
        }
        MEMMOVE(tmp, tokStart, len);
        tmp[len] = '\0';

        switch (tokIdx)
        {
        case 0:  entry->ICOUNTRY    = AtoiFarLocal(tmp); FREE(tmp); break;
        case 1:  entry->ICURRDIGITS = AtoiFarLocal(tmp); FREE(tmp); break;
        case 2:  entry->ICURRENCY   = AtoiFarLocal(tmp); FREE(tmp); break;
        case 3:  entry->IDATE       = AtoiFarLocal(tmp); FREE(tmp); break;
        case 4:  entry->IMEASURE    = AtoiFarLocal(tmp); FREE(tmp); break;
        case 5:  entry->INEGCURR    = AtoiFarLocal(tmp); FREE(tmp); break;
        case 6:  entry->ITIME       = AtoiFarLocal(tmp); FREE(tmp); break;
        case 7:  entry->ITLZERO     = AtoiFarLocal(tmp); FREE(tmp); break;
        case 8:  entry->ILZERO      = AtoiFarLocal(tmp); FREE(tmp); break;
        case 9:  entry->IDIGITS     = AtoiFarLocal(tmp); FREE(tmp); break;
        case 10: entry->S1159       = tmp; break;
        case 11: entry->S2359       = tmp; break;
        case 12: entry->SCURRENCY   = tmp; break;
        case 13: entry->STHOUSAND   = tmp; break;
        case 14: entry->SDECIMAL    = tmp; break;
        case 15: entry->SDATE       = tmp; break;
        case 16: entry->STIME       = tmp; break;
        case 17: entry->SLIST       = tmp; break;
        case 18: entry->SSHORTDATE  = tmp; break;
        case 19: entry->SLONGDATE   = tmp; break;
        case 20: entry->lang        = tmp; break;
        }

        if (tokEnd >= end) break;
        tokStart = tokEnd + 1;
        tokIdx++;
    }

    return TRUE;

error:
    InfFreeCountryEntry(entry);
    return FALSE;
}

void InfFreeCountryEntry(COUNTRY_ENTRY FAR *entry)
{
    if (!entry) return;


    if (entry->name) {
        FREE(entry->name);
        entry->name = NULL;
    }
    if (entry->lang) {
        FREE(entry->lang);
        entry->lang = NULL;
    }
    if (entry->S1159) {
        FREE(entry->S1159);
        entry->S1159 = NULL;
    }
    if (entry->S2359) {
        FREE(entry->S2359);
        entry->S2359 = NULL;
    }
    if (entry->SCURRENCY) {
        FREE(entry->SCURRENCY);
        entry->SCURRENCY = NULL;
    }
    if (entry->STHOUSAND) {
        FREE(entry->STHOUSAND);
        entry->STHOUSAND = NULL;
    }
    if (entry->SDECIMAL) {
        FREE(entry->SDECIMAL);
        entry->SDECIMAL = NULL;
    }
    if (entry->SDATE) {
        FREE(entry->SDATE);
        entry->SDATE = NULL;
    }
    if (entry->STIME) {
        FREE(entry->STIME);
        entry->STIME = NULL;
    }
    if (entry->SLIST) {
        FREE(entry->SLIST);
        entry->SLIST = NULL;
    }
    if (entry->SSHORTDATE) {
        FREE(entry->SSHORTDATE);
        entry->SSHORTDATE = NULL;
    }
    if (entry->SLONGDATE) {
        FREE(entry->SLONGDATE);
        entry->SLONGDATE = NULL;
    }

    MEMSET(entry, 0, sizeof(COUNTRY_ENTRY));
}
