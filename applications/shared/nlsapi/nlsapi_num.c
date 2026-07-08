/*
 * nlsapi_num.c – реализация GetNumberFormatA для Win16 NLS API
 */
#include "nlsapi_internal.h"
#include <ctype.h>

/* Вспомогательная функция: проверка, состоит ли строка только из цифр */
static BOOL IsDigitsOnly(LPCSTR str, int len)
{
    int i;
    for (i = 0; i < len; i++)
        if (!isdigit((unsigned char)str[i])) return FALSE;
    return TRUE;
}

/* Формирование отрицательного числа по шаблону NegativeOrder */
static int FormatNegative(LPSTR dest, int destSize, LPCSTR numberStr, UINT negOrder)
{
    switch (negOrder)
    {
    case NEGATIVE_LEFT_PAREN:   /* (1.1) */
        return wsprintf(dest, "(%s)", (LPSTR)numberStr);
    case NEGATIVE_SIGN_LEFT:    /* -1.1 */
        return wsprintf(dest, "-%s", (LPSTR)numberStr);
    case NEGATIVE_SIGN_SPACE:   /* - 1.1 */
        return wsprintf(dest, "- %s", (LPSTR)numberStr);
    case NEGATIVE_SIGN_RIGHT:   /* 1.1- */
        return wsprintf(dest, "%s-", (LPSTR)numberStr);
    case NEGATIVE_SPACE_RIGHT:  /* 1.1 - */
        return wsprintf(dest, "%s -", (LPSTR)numberStr);
    default:
        return wsprintf(dest, "-%s", (LPSTR)numberStr); /* fallback */
    }
}

/*
 *  GetNumberFormatA – Win16-совместимая реализация
 */
int WINAPI DECLSPEC GetNumberFormatA(LCID Locale, DWORD dwFlags, LPCSTR lpValue,
                            const NUMBERFMTA FAR *lpFormat,
                            LPSTR lpNumberStr, int cchNumber)
{
    static char        szInt[40], szDec[40], szOut[120];
    int         iInt, iDec, cDec, cInt, cGroups, groupDigits, firstGroupLen;
    int         lenFormatted;
    BOOL        bNegative;
    UINT        numDigits, leadingZero, grouping, negOrder;
    static char        decSep[8], thouSep[8];
    int         i, j, k, n;

    /* ---------- 1. Проверка параметров ---------- */
    if (!lpValue) return 0;
    if (cchNumber < 0) return 0;
    if (dwFlags != 0 && !(dwFlags & LOCALE_NOUSEROVERRIDE)) return 0;


    /* ---------- 2. Извлечение настроек ---------- */
    if (lpFormat)
    {
        numDigits   = lpFormat->NumDigits;
        leadingZero = lpFormat->LeadingZero;
        grouping    = lpFormat->Grouping;
        negOrder    = lpFormat->NegativeOrder;
        if (lpFormat->lpDecimalSep && *lpFormat->lpDecimalSep)
            lstrcpy(decSep, lpFormat->lpDecimalSep);
        else
            lstrcpy(decSep, ".");
        if (lpFormat->lpThousandSep && *lpFormat->lpThousandSep)
            lstrcpy(thouSep, lpFormat->lpThousandSep);
        else
            lstrcpy(thouSep, ",");
    }
    else
    {
        char buf[10];
        /* получаем системные настройки для локали */
        numDigits   = 2;
        leadingZero = 0;
        negOrder    = 0;
        grouping    = 0x30;  /* "3;0" */
        lstrcpy(decSep, ".");
        lstrcpy(thouSep, ",");

        if (dwFlags & LOCALE_NOUSEROVERRIDE)
        {
            /* @todo В Win16 нет различий между системной и пользовательской локалью */
        }

        GetLocaleInfo(Locale, LOCALE_IDIGITS | LOCALE_RETURN_NUMBER, (LPSTR)&numDigits, sizeof(UINT));
        GetLocaleInfo(Locale, LOCALE_ILZERO | LOCALE_RETURN_NUMBER, (LPSTR)&leadingZero, sizeof(UINT));
        GetLocaleInfo(Locale, LOCALE_INEGNUMBER | LOCALE_RETURN_NUMBER, (LPSTR)&negOrder, sizeof(UINT));
        GetLocaleInfo(Locale, LOCALE_SDECIMAL, decSep, sizeof(decSep));
        GetLocaleInfo(Locale, LOCALE_STHOUSAND, thouSep, sizeof(thouSep));
        {
            char grp[10];
            if (GetLocaleInfo(Locale, LOCALE_SGROUPING, grp, sizeof(grp)))
            {
                int g = AtoiFar(grp);
                if (g <= 0 || g > 9) g = 3;
                grouping = (UINT)((g << 8) | 0x30);
            }
        }               
    }

    /* ---------- 3. Разбор входной строки ---------- */
    {
        LPSTR p = (LPSTR)lpValue;
        bNegative = FALSE;
        if (*p == '-') { bNegative = TRUE; p++; }

        /* целая часть */
        iInt = 0;
        while (*p >= '0' && *p <= '9' && iInt < (int)sizeof(szInt)-1)
            szInt[iInt++] = *p++;
        szInt[iInt] = '\0';

        if (iInt == 0 && *p != '.') return 0;  /* нет цифр и нет точки */

        /* дробная часть */
        iDec = 0;
        if (*p == '.')
        {
            p++;
            while (*p >= '0' && *p <= '9' && iDec < (int)sizeof(szDec)-1)
                szDec[iDec++] = *p++;
            szDec[iDec] = '\0';
        }
        if (*p != '\0') return 0;   /* посторонние символы */

        /* если нет цифр вообще, ошибка (только знак или точка) */
        if (iInt == 0 && iDec == 0) return 0;
    }


    /* ---------- 4. Округление дробной части до numDigits ---------- */
    cDec = (int)numDigits;
    if (cDec < 0) cDec = 0;
    if (cDec > 9) cDec = 9;   /* разумный предел */

    if (iDec > cDec && cDec < (int)sizeof(szDec)-1)
    {
        /* округление */
        if (szDec[cDec] >= '5')
        {
            /* прибавляем 1 к целой части, если необходимо */
            int carry = 1;
            for (j = iInt-1; j >= 0 && carry; j--)
            {
                if (szInt[j] < '9') { szInt[j]++; carry = 0; }
                else { szInt[j] = '0'; carry = 1; }
            }
            if (carry)
            {
                /* сдвигаем целую часть вправо */
                memmove(szInt+1, szInt, iInt+1); /* +1 для '\0' */
                szInt[0] = '1';
                iInt++;
            }
        }
        /* обрезаем дробную часть */
        iDec = cDec;
        szDec[cDec] = '\0';
    }
    /* дополняем нулями справа, если короче */
    while (iDec < cDec)
    {
        szDec[iDec++] = '0';
        szDec[iDec]   = '\0';
    }

    /* ---------- 5. Вставка разделителей тысяч ---------- */
    /* разбираем grouping: младший байт = последняя группа (обычно '0'),
       старший байт = размер первой группы (обычно 3) */
    groupDigits = (grouping >> 8) & 0xFF;   /* размер первой группы */
    /* вторая группа (последняя) = grouping & 0xFF; это 0, значит остаток не группируется */
    if (groupDigits <= 0 || groupDigits > 9) groupDigits = 3;

    /* формируем строку с разделителями */
    {
        int lenThou = lstrlen(thouSep);
        int src = iInt;
        int dst = sizeof(szOut)-1;
        szOut[dst--] = '\0';

        /* обрабатываем целую часть с группировкой */
        while (src > 0)
        {
            for (k = 0; k < groupDigits && src > 0; k++)
            {
                szOut[dst--] = szInt[--src];
            }
            if (src > 0)
            {
                /* вставляем разделитель */
                for (n = lenThou-1; n >= 0; n--)
                    szOut[dst--] = thouSep[n];
            }
            /* после первой группы используем остальные правила: если вторая группа 0, то оставшиеся цифры не группируются */
        }
        /* копируем результат в szInt */
        lstrcpy(szInt, szOut + dst + 1);
        iInt = lstrlen(szInt);
    }

    /* ---------- 6. Сборка итоговой строки ---------- */
    {
        char numberPart[80];
        /* ведущий ноль */
        if (iInt == 0 && !leadingZero)
        {
            /* только дробная часть, ноль не выводим */
            wsprintf(numberPart, "%s%s", decSep, szDec);
        }
        else
        {
            if (iDec > 0)
            {
                wsprintf(numberPart, "%s%s%s", (LPSTR)szInt, (LPSTR)decSep, (LPSTR)szDec);
            } else {
                wsprintf(numberPart, "%s", (LPSTR)szInt);
           }
        }

        if (bNegative)
        {
            lenFormatted = FormatNegative((LPSTR)szOut, sizeof(szOut), (LPSTR)numberPart, negOrder);
        }
        else
        {
            lstrcpy(szOut, numberPart);
            lenFormatted = lstrlen(szOut);
        }
    }

    /* ---------- 7. Возврат размера или копирование ---------- */
    if (cchNumber == 0)
        return lenFormatted + 1;   /* включая завершающий нуль, как в Win32 */

    if (lenFormatted + 1 > cchNumber)
        return 0;   /* буфер недостаточен */

    lstrcpy(lpNumberStr, szOut);
    return lenFormatted;
}
