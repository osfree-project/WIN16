/*
 * nlsapi_currency.c – реализация GetCurrencyFormatA для Win16 NLS API
 */
#include "nlsapi_internal.h"
#include <ctype.h>

/* Безопасное копирование строки с ограничением длины */
static void safe_strcpy(LPSTR dest, LPCSTR src, int destSize)
{
    int i;
    if (destSize <= 0) return;
    for (i = 0; i < destSize - 1 && src[i] != '\0'; i++)
        dest[i] = src[i];
    dest[i] = '\0';
}

/* Проверка, состоит ли строка только из цифр */
static BOOL IsDigitsOnly(LPCSTR str, int len)
{
    int i;
    for (i = 0; i < len; i++)
        if (!isdigit((unsigned char)str[i])) return FALSE;
    return TRUE;
}

/* Форматирование отрицательной валюты согласно NegativeOrder */
static int FormatCurrencyNegative(LPSTR dest, int destSize, LPCSTR numberStr,
                                  LPCSTR symbol, UINT negOrder)
{
    switch (negOrder)
    {
    case 0:  /* ($1.1) */
        return wsprintf(dest, "(%s%s)", (LPSTR)symbol, (LPSTR)numberStr);
    case 1:  /* -$1.1 */
        return wsprintf(dest, "-%s%s", (LPSTR)symbol, (LPSTR)numberStr);
    case 2:  /* $-1.1 */
        return wsprintf(dest, "%s-%s", (LPSTR)symbol, (LPSTR)numberStr);
    case 3:  /* $1.1- */
        return wsprintf(dest, "%s%s-", (LPSTR)symbol, (LPSTR)numberStr);
    case 4:  /* (1.1$) */
        return wsprintf(dest, "(%s%s)", (LPSTR)numberStr, (LPSTR)symbol);
    case 5:  /* -$ 1.1 */
        return wsprintf(dest, "-%s %s", (LPSTR)symbol, (LPSTR)numberStr);
    case 6:  /* $- 1.1 */
        return wsprintf(dest, "%s- %s", (LPSTR)symbol, (LPSTR)numberStr);
    case 7:  /* 1.1 $- */
        return wsprintf(dest, "%s %s-", (LPSTR)numberStr, (LPSTR)symbol);
    case 8:  /* - $1.1 */
        return wsprintf(dest, "- %s%s", (LPSTR)symbol, (LPSTR)numberStr);
    case 9:  /* $ -1.1 */
        return wsprintf(dest, "%s -%s", (LPSTR)symbol, (LPSTR)numberStr);
    case 10: /* 1.1 $ - */
        return wsprintf(dest, "%s %s -", (LPSTR)numberStr, (LPSTR)symbol);
    case 11: /* $ 1.1- */
        return wsprintf(dest, "%s %s-", (LPSTR)symbol, (LPSTR)numberStr);
    case 12: /* 1.1- $ */
        return wsprintf(dest, "%s- %s", (LPSTR)numberStr, (LPSTR)symbol);
    case 13: /* -$ 1.1 */
        return wsprintf(dest, "-%s %s", (LPSTR)symbol, (LPSTR)numberStr);
    case 14: /* 1.1$ - */
        return wsprintf(dest, "%s%s -", (LPSTR)numberStr, (LPSTR)symbol);
    case 15: /* - 1.1$ */
        return wsprintf(dest, "- %s%s", (LPSTR)numberStr, (LPSTR)symbol);
    default:
        return wsprintf(dest, "-%s%s", (LPSTR)symbol, (LPSTR)numberStr);
    }
}

/* Форматирование положительной валюты согласно PositiveOrder */
static int FormatCurrencyPositive(LPSTR dest, LPCSTR numberStr,
                                  LPCSTR symbol, UINT posOrder)
{
    switch (posOrder)
    {
    case 0:  /* $1.1 */
        return wsprintf(dest, "%s%s", (LPSTR)symbol, (LPSTR)numberStr);
    case 1:  /* 1.1$ */
        return wsprintf(dest, "%s%s", (LPSTR)numberStr, (LPSTR)symbol);
    case 2:  /* $ 1.1 */
        return wsprintf(dest, "%s %s", (LPSTR)symbol, (LPSTR)numberStr);
    case 3:  /* 1.1 $ */
        return wsprintf(dest, "%s %s", (LPSTR)numberStr, (LPSTR)symbol);
    default:
        return wsprintf(dest, "%s%s", (LPSTR)symbol, (LPSTR)numberStr);
    }
}

/*
 *  GetCurrencyFormatA – Win16-совместимая реализация
 */
int WINAPI DECLSPEC GetCurrencyFormatA(
    LCID     Locale,
    DWORD    dwFlags,
    LPCSTR   lpValue,
    const CURRENCYFMTA FAR *lpFormat,
    LPSTR    lpCurrencyStr,
    int      cchCurrency)
{
    static char szInt[40], szDec[40], szOut[120];
    int  iInt, iDec, cDec;
    int  lenFormatted;
    BOOL bNegative;
    UINT numDigits, leadingZero, grouping, negOrder, posOrder;
    static char decSep[8], thouSep[8], symbol[16];
    int  j, k, n;

    /* ---------- 1. Проверка параметров ---------- */
    if (!lpValue) return 0;
    if (cchCurrency < 0) return 0;
    if (dwFlags != 0 && !(dwFlags & LOCALE_NOUSEROVERRIDE)) return 0;

    /* ---------- 2. Извлечение настроек ---------- */
    if (lpFormat)
    {
        numDigits   = lpFormat->NumDigits;
        leadingZero = lpFormat->LeadingZero;
        grouping    = lpFormat->Grouping;
        negOrder    = lpFormat->NegativeOrder;
        posOrder    = lpFormat->PositiveOrder;

        safe_strcpy(decSep, lpFormat->lpDecimalSep ? lpFormat->lpDecimalSep : ".", sizeof(decSep));
        safe_strcpy(thouSep, lpFormat->lpThousandSep ? lpFormat->lpThousandSep : ",", sizeof(thouSep));
        safe_strcpy(symbol, lpFormat->lpCurrencySymbol ? lpFormat->lpCurrencySymbol : "$", sizeof(symbol));
    }
    else
    {
        numDigits   = 2;
        leadingZero = 0;
        grouping    = 0x30;
        negOrder    = 0;
        posOrder    = 0;
        lstrcpy(decSep, ".");
        lstrcpy(thouSep, ",");
        lstrcpy(symbol, "$");

        GetLocaleInfo(Locale, LOCALE_ICURRDIGITS | LOCALE_RETURN_NUMBER, (LPSTR)&numDigits, sizeof(UINT));
        GetLocaleInfo(Locale, LOCALE_ILZERO | LOCALE_RETURN_NUMBER, (LPSTR)&leadingZero, sizeof(UINT));
        GetLocaleInfo(Locale, LOCALE_INEGCURR | LOCALE_RETURN_NUMBER, (LPSTR)&negOrder, sizeof(UINT));
        GetLocaleInfo(Locale, LOCALE_IPOSITIVECURRENCY | LOCALE_RETURN_NUMBER, (LPSTR)&posOrder, sizeof(UINT));
        GetLocaleInfo(Locale, LOCALE_SCURRENCY, symbol, sizeof(symbol));
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

        iInt = 0;
        while (*p >= '0' && *p <= '9' && iInt < (int)sizeof(szInt)-1)
            szInt[iInt++] = *p++;
        szInt[iInt] = '\0';

        if (iInt == 0 && *p != '.') return 0;

        iDec = 0;
        if (*p == '.')
        {
            p++;
            while (*p >= '0' && *p <= '9' && iDec < (int)sizeof(szDec)-1)
                szDec[iDec++] = *p++;
            szDec[iDec] = '\0';
        }
        if (*p != '\0') return 0;
        if (iInt == 0 && iDec == 0) return 0;
    }

    /* ---------- 4. Округление ---------- */
    cDec = (int)numDigits;
    if (cDec < 0) cDec = 0;
    if (cDec > 9) cDec = 9;
    if (iDec > cDec && cDec < (int)sizeof(szDec)-1)
    {
        if (szDec[cDec] >= '5')
        {
            int carry = 1;
            for (j = iInt-1; j >= 0 && carry; j--)
            {
                if (szInt[j] < '9') { szInt[j]++; carry = 0; }
                else { szInt[j] = '0'; carry = 1; }
            }
            if (carry)
            {
                memmove(szInt+1, szInt, iInt+1);
                szInt[0] = '1';
                iInt++;
            }
        }
        iDec = cDec;
        szDec[cDec] = '\0';
    }
    while (iDec < cDec)
    {
        szDec[iDec++] = '0';
        szDec[iDec]   = '\0';
    }

    /* ---------- 5. Группировка тысяч ---------- */
    {
        int groupSize = (grouping >> 8) & 0xFF;
        int lenThou = lstrlen(thouSep);
        int src = iInt;
        int dst = sizeof(szOut)-1;
        if (groupSize <= 0 || groupSize > 9) groupSize = 3;
        szOut[dst--] = '\0';
        while (src > 0)
        {
            for (k = 0; k < groupSize && src > 0; k++)
                szOut[dst--] = szInt[--src];
            if (src > 0)
            {
                for (n = lenThou-1; n >= 0; n--)
                    szOut[dst--] = thouSep[n];
            }
        }
        lstrcpy(szInt, szOut + dst + 1);
        iInt = lstrlen(szInt);
    }

    /* ---------- 6. Сборка числовой части и вставка символа валюты ---------- */
    {
        char numberPart[80];
        if (iInt == 0 && !leadingZero)
            wsprintf(numberPart, "%s%s", (LPSTR)decSep, (LPSTR)szDec);
        else if (iDec > 0)
            wsprintf(numberPart, "%s%s%s", (LPSTR)szInt, (LPSTR)decSep, (LPSTR)szDec);
        else
            wsprintf(numberPart, "%s", (LPSTR)szInt);

        if (bNegative)
            lenFormatted = FormatCurrencyNegative(szOut, sizeof(szOut),
                                                  numberPart, symbol, negOrder);
        else
            lenFormatted = FormatCurrencyPositive(szOut, numberPart, symbol, posOrder);
    }

    /* ---------- 7. Возврат результата ---------- */
    if (cchCurrency == 0)
        return lenFormatted + 1;
    if (lenFormatted + 1 > cchCurrency)
        return 0;

    lstrcpy(lpCurrencyStr, szOut);
    return lenFormatted;
}
