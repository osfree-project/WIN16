/*
 * Support for system colors
 *
 * Copyright  David W. Metcalfe, 1993
 * Copyright  Alexandre Julliard, 1994
 *
 */

//#include <stdio.h>
//#include <stdlib.h>
#include "user.h"
#include "syscolor.h"
//#include "stddebug.h"
/* #define DEBUG_SYSCOLOR */
//#include "debug.h"

struct SysColorObjects sysColorObjects = { 0, };

static char * DefSysColors[] =
{
    "Scrollbar", "224 224 224",      /* COLOR_SCROLLBAR           */
    "Background", "192 192 192",     /* COLOR_BACKGROUND          */
    "ActiveTitle", "0 64 128",       /* COLOR_ACTIVECAPTION       */
    "InactiveTitle", "255 255 255",  /* COLOR_INACTIVECAPTION     */
    "Menu", "0 255 255",             /* COLOR_MENU                */
    "Window", "255 255 255",         /* COLOR_WINDOW              */
    "WindowFrame", "111 0 0",          /* COLOR_WINDOWFRAME         */
    "MenuText", "0 0 0",             /* COLOR_MENUTEXT            */
    "WindowText", "0 0 0",           /* COLOR_WINDOWTEXT          */
    "TitleText", "255 255 255",      /* COLOR_CAPTIONTEXT         */
    "ActiveBorder", "128 128 128",   /* COLOR_ACTIVEBORDER        */
    "InactiveBorder", "255 255 255", /* COLOR_INACTIVEBORDER      */
    "AppWorkspace", "255 255 232",   /* COLOR_APPWORKSPACE        */
    "Hilight", "166 202 240",        /* COLOR_HIGHLIGHT           */
    "HilightText", "0 0 0",          /* COLOR_HIGHLIGHTTEXT       */
    "ButtonFace", "192 192 192",     /* COLOR_BTNFACE             */
    "ButtonShadow", "128 128 128",   /* COLOR_BTNSHADOW           */
    "GrayText", "192 192 192",       /* COLOR_GRAYTEXT            */
    "ButtonText", "0 0 0",           /* COLOR_BTNTEXT             */
    "InactiveTitleText", "0 0 0",    /* COLOR_INACTIVECAPTIONTEXT */
    "ButtonHilight", "255 255 255"   /* COLOR_BTNHIGHLIGHT        */
};

#define NUM_SYS_COLORS     (COLOR_BTNHIGHLIGHT+1)

static COLORREF SysColors[NUM_SYS_COLORS];

//#define MAKE_SOLID(color) (PALETTEINDEX(GetNearestPaletteIndex(STOCK_DEFAULT_PALETTE,(color))))

#define MAKE_SOLID(color) 0

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
    case COLOR_INACTIVECAPTIONTEXT:
	break;
    case COLOR_BTNHIGHLIGHT:
	DeleteObject( sysColorObjects.hbrushBtnHighlight );
	sysColorObjects.hbrushBtnHighlight = CreateSolidBrush( color );
	break;
    }
}


/*************************************************************************
 *             SYSCOLOR_Init
 */
void SYSCOLOR_Init(void)
{
    int i, r, g, b;
    char **p;
    char buffer[100];
    char far *ptr;
    int color_index;
    int value;

    for (i = 0, p = DefSysColors; i < NUM_SYS_COLORS; i++, p += 2)
    {
        GetProfileString("colors", p[0], p[1], buffer, 100);

	TRACE("buffer=%S", buffer);        

        // Парсинг строки вручную вместо sscanf
        r = g = b = 0;
        color_index = 0;
        value = 0;
        ptr = buffer;
        
        while (*ptr && color_index < 3)
        {
            // Пропускаем пробелы
            while (*ptr == ' ') ptr++;
            
            if (!*ptr) break;
            
            // Читаем число
            value = 0;
            while (*ptr >= '0' && *ptr <= '9')
            {
                value = value * 10 + (*ptr - '0');
                ptr++;
            }
            
            // Сохраняем значение в соответствующую переменную
            if (color_index == 0)
                r = value;
            else if (color_index == 1)
                g = value;
            else if (color_index == 2)
                b = value;
                
            color_index++;
            
            // Пропускаем пробелы после числа
            while (*ptr == ' ') ptr++;
        }
        
        // Если не удалось считать все три значения
        if (color_index < 3)
        {
            r = g = b = 0;
        }
	TRACE("r=%d g=%d b=%d", r, g, b);        
        SYSCOLOR_SetColor(i, RGB(r, g, b));
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

	if ((nIndex < 0) || (nIndex >= sizeof(SysColors)/sizeof(SysColors[0])))
	{
		retVal=RGB(0,0,0);
	} else {
		retVal=SysColors[nIndex];
	}

	PopDS();
	return retVal;
}


/*************************************************************************
 *             SetSysColors          (USER.181)
 */

void WINAPI SetSysColors(int nChanges, const int FAR * lpSysColor, const COLORREF FAR *lpColorValues)
{
	int i;

	PushDS();
	SetDS(USER_HeapSel);

	for (i = 0; i < nChanges; i++)
	{
		SYSCOLOR_SetColor( lpSysColor[i], lpColorValues[i] );
	}

	PopDS();

	/* Send WM_SYSCOLORCHANGE message to all windows */
	SendMessage(HWND_BROADCAST,WM_SYSCOLORCHANGE,0,0);

    /* Repaint affected portions of all visible windows */

    /* ................ */
}
