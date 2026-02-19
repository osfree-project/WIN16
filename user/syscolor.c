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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */


#include "user.h"
#include "syscolor.h"


#if (WINVER >= 0x0300) && (WINVER < 0x030a)
#define NUM_SYS_COLORS     (COLOR_BTNTEXT+1)
#endif

#if WINVER >= 0x030a
#define NUM_SYS_COLORS     (COLOR_BTNHIGHLIGHT+1)
#endif

static COLORREF SysColors[NUM_SYS_COLORS];

#define MAKE_SOLID(color) (PALETTEINDEX(GetNearestPaletteIndex(GetStockObject(DEFAULT_PALETTE), (color))))

/*************************************************************************
 *             ParseColorString
 *
 * Parse color string in any supported WIN.INI format:
 * 1. Decimal RGB: "red green blue"           (0-255 each)
 * 2. Hexadecimal full: "#RRGGBB"             (0-ffffff)
 * 3. Hexadecimal triplet: "#R #G #B"         (0-ff each, osFree Janus)
 * 4. Single number: "gray"                   (0-255, gray scale)
 */
static COLORREF ParseColorString(const char far *str)
{
    int r = 0, g = 0, b = 0;
    const char far *p = str;
    int num_count = 0;       /* Для подсчёта чисел в строке */
    const char far *temp;        /* Для проверки формата */
    int i;                   /* Для циклов */
    int component;           /* Для разбора чисел */
    
    if (!str || !*str) 
        return RGB(0, 0, 0);
    
    /* Skip leading spaces */
    while (*p == ' ') p++;
    
    /* Count numbers in the string */
    temp = p;
    while (*temp) {
        /* Skip to next number */
        while (*temp == ' ') temp++;
        
        if (*temp >= '0' && *temp <= '9') {
            num_count++;
            while (*temp >= '0' && *temp <= '9') temp++;
        } else {
            break;
        }
    }
    
    /* If only one number, treat it as grayscale: R=G=B */
    if (num_count == 1) {
        component = 0;
        while (*p >= '0' && *p <= '9') {
            component = component * 10 + (*p - '0');
            p++;
        }
        if (component < 0) component = 0; 
        else if (component > 255) component = 255;
        return RGB(component, component, component);
    }
    
    /* Otherwise parse three numbers or hex formats */
    
    /* Check for hexadecimal format */
    if (*p == '#')
    {
        /* Hexadecimal format - пропускаем для простоты, 
           можно добавить обработку позже */
        /* For now, return black */
        return RGB(0, 0, 0);
    }
    
    /* Decimal RGB format: "red green blue" */
    for (i = 0; i < 3; i++)
    {
        /* Skip spaces */
        while (*p == ' ') p++;
        
        component = 0;
        while (*p >= '0' && *p <= '9')
        {
            component = component * 10 + (*p - '0');
            p++;
        }
        
        if (i == 0) r = component;
        else if (i == 1) g = component;
        else b = component;
        
        /* Skip spaces before next number */
        while (*p == ' ') p++;
    }
    
    /* Clamp values to 0-255 range */
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;
    
    return RGB(r, g, b);
}

/*************************************************************************
 *             SYSCOLOR_SetColor
 */
static void SYSCOLOR_SetColor( int index, COLORREF color )
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

void SYSCOLOR_Init(void)
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

/*************************************************************************
 *             GetSysColor           (USER.180)
 */

COLORREF WINAPI GetSysColor(int nIndex)
{
	COLORREF retVal;

	PushDS();
	SetDS(USER_HeapSel);
	FUNCTION_START

	if ((nIndex < 0) || (nIndex >= sizeof(SysColors)/sizeof(SysColors[0])))
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
