/*
 * Support for system colors
 *
 * Copyright  David W. Metcalfe, 1993
 * Copyright  Alexandre Julliard, 1994
 * Copyright  Yuri Prokushev, 2026
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "user.h"
#include "syscolor.h"

#define MAKE_SOLID(color) (PALETTEINDEX(GetNearestPaletteIndex(GetStockObject(DEFAULT_PALETTE), (color))))

#include <windows.h>


/*************************************************************************
 *             SYSCOLOR_SetColor
 */
static void FAR SYSCOLOR_SetColor( int index, COLORREF color )
{
    SysColors[index] = color;
    switch(index)
    {
    case COLOR_SCROLLBAR:
	DeleteObject( sysColorObjects.hbrushScrollbar );
	sysColorObjects.hbrushScrollbar = CreateSolidBrush( color );
	break;
    case COLOR_BACKGROUND:
	break;
    case COLOR_ACTIVECAPTION:
	DeleteObject( sysColorObjects.hbrushActiveCaption );
	sysColorObjects.hbrushActiveCaption = CreateSolidBrush( color );
	break;
    case COLOR_INACTIVECAPTION:
	DeleteObject( sysColorObjects.hbrushInactiveCaption );
	sysColorObjects.hbrushInactiveCaption = CreateSolidBrush( color );
	break;
    case COLOR_MENU:
	DeleteObject( sysColorObjects.hbrushMenu );
	sysColorObjects.hbrushMenu = CreateSolidBrush( MAKE_SOLID(color) );
	break;
    case COLOR_WINDOW:
	DeleteObject( sysColorObjects.hbrushWindow );
	sysColorObjects.hbrushWindow = CreateSolidBrush( color );
	break;
    case COLOR_WINDOWFRAME:
	DeleteObject( sysColorObjects.hpenWindowFrame );
	sysColorObjects.hpenWindowFrame = CreatePen( PS_SOLID, 1, color );
	break;
    case COLOR_MENUTEXT:
	break;
    case COLOR_WINDOWTEXT:
	DeleteObject( sysColorObjects.hpenWindowText );
	sysColorObjects.hpenWindowText = CreatePen( PS_DOT, 1, color );
	break;
    case COLOR_CAPTIONTEXT:
	break;
    case COLOR_ACTIVEBORDER:
	DeleteObject( sysColorObjects.hbrushActiveBorder );
	sysColorObjects.hbrushActiveBorder = CreateSolidBrush( color );
	break;
    case COLOR_INACTIVEBORDER:
	DeleteObject( sysColorObjects.hbrushInactiveBorder );
	sysColorObjects.hbrushInactiveBorder = CreateSolidBrush( color );
	break;
    case COLOR_APPWORKSPACE:
	break;
    case COLOR_HIGHLIGHT:
	DeleteObject( sysColorObjects.hbrushHighlight );
	sysColorObjects.hbrushHighlight = CreateSolidBrush( MAKE_SOLID(color));
	break;
    case COLOR_HIGHLIGHTTEXT:
	break;
    case COLOR_BTNFACE:
	DeleteObject( sysColorObjects.hbrushBtnFace );
	sysColorObjects.hbrushBtnFace = CreateSolidBrush( color );
	break;
    case COLOR_BTNSHADOW:
	DeleteObject( sysColorObjects.hbrushBtnShadow );
	sysColorObjects.hbrushBtnShadow = CreateSolidBrush( color );
	break;
    case COLOR_GRAYTEXT:
    case COLOR_BTNTEXT:
#if WINVER >= 0x030a
    case COLOR_INACTIVECAPTIONTEXT:
#endif
	break;
#if WINVER >= 0x030a
    case COLOR_BTNHIGHLIGHT:
	DeleteObject( sysColorObjects.hbrushBtnHighlight );
	sysColorObjects.hbrushBtnHighlight = CreateSolidBrush( color );
	break;
#endif
    }
}

#pragma code_seg( "INIT_TEXT" );

/* Вспомогательная функция: преобразует шестнадцатеричный символ в число (0-15) или возвращает -1 */
static int hex_char_to_int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

/*************************************************************************
 *             ParseColorString
 *
 * Parse color string in any supported WIN.INI format:
 * 1. Decimal RGB: "red green blue"           (0-255 each)
 * 2. Hexadecimal full: "#RRGGBB"             (0-ffffff)
 * 3. Hexadecimal triplet: "#R #G #B"         (0-ff each)
 */
static COLORREF ParseColorString(const char far *str)
{
    int r = 0, g = 0, b = 0;
    const char far *p = str;
    int i;

    if (!str || !*str)
        return RGB(0, 0, 0);

    /* Пропускаем ведущие пробелы */
    while (*p == ' ') p++;

    /* Проверяем, не начинается ли строка с '#' – значит шестнадцатеричный формат */
    if (*p == '#')
    {
        const char far *start = p;   /* Запоминаем начало для повторной попытки */

        /* ---- Попытка разобрать полный формат "#RRGGBB" (ровно 6 цифр) ---- */
        int digits[6];
        int count = 0;

        p++;  /* пропускаем первый '#' */
        while (count < 6 && *p) {
            int val = hex_char_to_int(*p);
            if (val == -1) break;
            digits[count++] = val;
            p++;
        }

        /* Если прочитали 6 цифр и следующий символ не является шестнадцатеричной цифрой и не '#' */
        if (count == 6 && hex_char_to_int(*p) == -1 && *p != '#') {
            r = (digits[0] << 4) | digits[1];
            g = (digits[2] << 4) | digits[3];
            b = (digits[4] << 4) | digits[5];
            return RGB(r, g, b);
        }

        /* ---- Если не подошло, пробуем разобрать как триплет "#R #G #B" ---- */
        p = start;  /* возвращаемся к началу строки */

        for (i = 0; i < 3; i++) {
            int comp = 0;
            int digit_count = 0;
            int val;

            /* Пропускаем пробелы перед '#' */
            while (*p == ' ') p++;

            /* Должен быть '#' */
            if (*p != '#')
                return RGB(0, 0, 0);
            p++;  /* пропускаем '#' */

            /* Читаем первую цифру */
            val = hex_char_to_int(*p);
            if (val == -1)   /* после '#' нет ни одной цифры */
                return RGB(0, 0, 0);
            comp = val;
            digit_count = 1;
            p++;

            /* Пробуем прочитать вторую цифру */
            val = hex_char_to_int(*p);
            if (val != -1) {
                comp = (comp << 4) | val;
                digit_count = 2;
                p++;
            } else {
                /* Если была только одна цифра, дублируем её (например, #F -> 0xFF) */
                comp = (comp << 4) | comp;
            }

            /* Убеждаемся, что после компоненты нет лишней hex-цифры */
            if (hex_char_to_int(*p) != -1)
                return RGB(0, 0, 0);

            /* Сохраняем компоненту */
            if (i == 0) r = comp;
            else if (i == 1) g = comp;
            else b = comp;

            /* Пропускаем пробелы перед следующей компонентой */
            while (*p == ' ') p++;
        }

        /* Успешно разобрали три компоненты */
        return RGB(r, g, b);
    }

    /* ---- Десятичный формат RGB: "red green blue" ---- */
    for (i = 0; i < 3; i++) {
        int component = 0;

        /* Пропускаем пробелы */
        while (*p == ' ') p++;

        /* Если строка закончилась до того, как прочитали три числа, возвращаем чёрный */
        if (!(*p >= '0' && *p <= '9')) {
            return RGB(0, 0, 0);
        }

        while (*p >= '0' && *p <= '9') {
            component = component * 10 + (*p - '0');
            p++;
        }

        if (i == 0) r = component;
        else if (i == 1) g = component;
        else b = component;
    }

    /* Ограничиваем значения диапазоном 0-255 */
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;

    return RGB(r, g, b);
}

VOID FAR SYSCOLOR_Init(VOID)
{
    int i;
    char buffer[100];
    int len;
    char szColors[0x14];
    char szColor[0x14];
    
    /* Имена цветов для WIN.INI (нужны только для GetProfileString) */
    static const WORD SysColorIds[] = {
	IDS_SCROLLBAR,
	IDS_BACKGROUND,
	IDS_ACTIVETITLE,
	IDS_INACTIVETITLE,
	IDS_MENU,
	IDS_WINDOW,
	IDS_WINDOWFRAME,
	IDS_MENUTEXT,
	IDS_WINDOWTEXT,
	IDS_TITLETEXT,
	IDS_ACTIVEBORDER,
	IDS_INACTIVEBORDER,
	IDS_APPWORKSPACE,
	IDS_HILIGHT,
	IDS_HILIGHTTEXT,
	IDS_BUTTONFACE,
	IDS_BUTTONSHADOW,
	IDS_GRAYTEXT,
	IDS_BUTTONTEXT,
#if WINVER >= 0x030a
        IDS_INACTIVETITLETEXT,
	IDS_BUTTONHILIGHT
#endif
    };
    
    /* Получаем бинарные значения по умолчанию из драйвера */
    const COLORREF* defaultColors = DISPLAY_GetSysColorDefaultsBinary();
    int colorCount = min(DISPLAY_GetSysColorCount(), NUM_SYS_COLORS);

	LoadString(USER_HeapSel, IDS_COLORS, szColors, sizeof(szColors));
    for (i = 0; i < colorCount; i++)
    {
        COLORREF finalColor;
        
	LoadString(USER_HeapSel, SysColorIds[i], szColor, sizeof(szColor));

        len = GetProfileString(szColors, (LPSTR)szColor, "", buffer, 100);
        
        if (len > 0)
        {
            buffer[len] = '\0'; /* Гарантируем завершение строки */
            finalColor = ParseColorString(buffer);
//            TRACE("Color %s: from INI='%S'", SysColorNames[i], buffer);
        }
        else
        {
            finalColor = defaultColors[i];
            TRACE("Color %S: using driver default", szColor);
        }
        
        SYSCOLOR_SetColor(i, finalColor);
    }
}

#pragma code_seg();

/*************************************************************************
 *             GetSysColor           (USER.180)
 */

COLORREF WINAPI GetSysColor(int nIndex)
{
	COLORREF retVal;

	PushDS();
	SetDS(USER_HeapSel);
	FUNCTION_START

	if ((nIndex < 0) || (nIndex >= NUM_SYS_COLORS/*sizeof(SysColors)/sizeof(SysColors[0])*/))
	{
		retVal=RGB(0,0,0);
	} else {
		retVal=SysColors[nIndex];
	}

	FUNCTION_END
	PopDS();
	return retVal;
}


/*************************************************************************
 *             SetSysColors          (USER.181)
 */

VOID WINAPI SetSysColors(int nChanges, const int FAR * lpSysColor, const COLORREF FAR *lpColorValues)
{
	int i;

	PushDS();
	SetDS(USER_HeapSel);
	FUNCTION_START

	for (i = 0; i < nChanges; i++)
	{
		SYSCOLOR_SetColor( lpSysColor[i], lpColorValues[i] );
	}

	/* Send WM_SYSCOLORCHANGE message to all windows */
	SendMessage(HWND_BROADCAST,WM_SYSCOLORCHANGE,0,0);

	/* Repaint affected portions of all visible windows */

	/* ................ */

	FUNCTION_END
	PopDS();
}
