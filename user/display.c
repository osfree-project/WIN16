/*
 * DISPLAY.DRV interface
 *
 * Copyright 2026 Yuri Prokushev
 *
 * This is interface to DISPLAY.DRV. For now it is support only Windows 3.0 things.
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
#include "display.h"

/* An undocumented function called by USER to check is display driver
 * contains a more suitable version of a resource.
 * Exported as ordinal 450.
 */
DWORD WINAPI GetDriverResourceID( WORD wResID, LPSTR lpResType );

/* DISPLAY.DRV configuration versions. This is currently an assumption that requires verification. Definitely for 2.0 and 3.0. The rest haven't been checked.
   Apparently, in versions 3.1 and later, a different scheme is used and such binary resources are only used for compatibility. */

#define OEM_CONFIG_OLD    13  /* Windows 2.x and earlier - 13 colors */
#define OEM_CONFIG_WIN30  19  /* Windows 3.0 - 19 colors (up to COLOR_BTNTEXT) */
// Starting from here something another are used (only win.ini???)
#define OEM_CONFIG_WIN31  21  /* Windows 3.1 - 21 colors (up to COLOR_BTNHIGHLIGHT) */
#define OEM_CONFIG_WIN95  25  /* Windows 95 - 25 colors (up to COLOR_INFOBK) */
#define OEM_CONFIG_WIN98  29  /* Windows 98 - 29 colors (up to COLOR_GRADIENTINACTIVECAPTION) */

/* Basic configuration structure - header with metrics */
#pragma pack(push, 1)
typedef struct {
    WORD cyVThumb;          /* Vertical scrollbar height */
    WORD cxHThumb;          /* Horizontal scrollbar width */
    WORD cxIconCompression; /* Icon compression coefficient horizontally */
    WORD cyIconCompression; /* Icon compression coefficient vertically */
    WORD cxCursorCompression; /* Cursor compression coefficient horizontally */
    WORD cyCursorCompression; /* Cursor compression coefficient vertically */
    WORD cyKanjiWindow;     /* Kanji window height */
    WORD cxBorder;          /* Vertical line thickness */
    WORD cyBorder;          /* Horizontal line thickness */
    /* After this comes a color array */
} DISPLAY_CONFIG_HEADER;
#pragma pack(pop)

#ifdef DEBUG
/* System color names for all versions */
static const char* SysColorNames[] = {
    "Scrollbar",        /* COLOR_SCROLLBAR = 0 */
    "Background",       /* COLOR_BACKGROUND = 1 */
    "ActiveCaption",    /* COLOR_ACTIVECAPTION = 2 */
    "InactiveCaption",  /* COLOR_INACTIVECAPTION = 3 */
    "Menu",             /* COLOR_MENU = 4 */
    "Window",           /* COLOR_WINDOW = 5 */
    "WindowFrame",      /* COLOR_WINDOWFRAME = 6 */
    "MenuText",         /* COLOR_MENUTEXT = 7 */
    "WindowText",       /* COLOR_WINDOWTEXT = 8 */
    "CaptionText",      /* COLOR_CAPTIONTEXT = 9 */
    "ActiveBorder",     /* COLOR_ACTIVEBORDER = 10 */
    "InactiveBorder",   /* COLOR_INACTIVEBORDER = 11 */
    "AppWorkspace",     /* COLOR_APPWORKSPACE = 12 */
    "HiliteBk",         /* COLOR_HIGHLIGHT = 13 */
    "HiliteText",       /* COLOR_HIGHLIGHTTEXT = 14 */
    "BtnFace",          /* COLOR_BTNFACE = 15 */
    "BtnShadow",        /* COLOR_BTNSHADOW = 16 */
    "GrayText",         /* COLOR_GRAYTEXT = 17 */
    "BtnText",          /* COLOR_BTNTEXT = 18 */
#if (WINVER >= 0x030a)
    "InactiveCaptionText", /* COLOR_INACTIVECAPTIONTEXT = 19 (WINVER >= 0x030a) */
    "BtnHighlight",     /* COLOR_BTNHIGHLIGHT = 20 (WINVER >= 0x030a) */
#endif
#if (WINVER >= 0x0400)
    "3DDkShadow",       /* COLOR_3DDKSHADOW = 21 (WINVER >= 0x0400) */
    "3DLight",          /* COLOR_3DLIGHT = 22 (WINVER >= 0x0400) */
    "InfoText",         /* COLOR_INFOTEXT = 23 (WINVER >= 0x0400) */
    "InfoBk",           /* COLOR_INFOBK = 24 (WINVER >= 0x0400) */
#endif
#if (WINVER >= 0x0410)
    "AlternateBtnFace",             /* COLOR_ALTERNATEBTNFACE = 25 (WINVER >= 0x0410) */
    "HotLight",                     /* COLOR_HOTLIGHT = 26 (WINVER >= 0x0410) */
    "GradientActiveCaption",        /* COLOR_GRADIENTACTIVECAPTION = 27 (WINVER >= 0x0410) */
    "GradientInactiveCaption"       /* COLOR_GRADIENTINACTIVECAPTION = 28 (WINVER >= 0x0410) */
#endif
};

#endif

/* Бинарные значения по умолчанию (VGA.DRV) в формате COLORREF */
static COLORREF g_SysColorDefaultsBinary[] = {
    RGB(0, 0, 0),  /* Scrollbar */
    RGB(0, 0, 0),  /* Background */
    RGB(0, 0, 0),     /* ActiveTitle */
    RGB(0, 0, 0),  /* InactiveTitle */
    RGB(0, 0, 0),  /* Menu */
    RGB(0, 0, 0),  /* Window */
    RGB(0, 0, 0),        /* WindowFrame */
    RGB(0, 0, 0),        /* MenuText */
    RGB(0, 0, 0),        /* WindowText */
    RGB(0, 0, 0),  /* TitleText */
    RGB(0, 0, 0),  /* ActiveBorder */
    RGB(0, 0, 0),  /* InactiveBorder */
    RGB(0, 0, 0),  /* AppWorkspace */
    RGB(0, 0, 0),        /* Hilight */
    RGB(0, 0, 0),  /* HilightText */
    RGB(0, 0, 0),  /* ButtonFace */
    RGB(0, 0, 0),  /* ButtonShadow */
    RGB(0, 0, 0),  /* GrayText */
    RGB(0, 0, 0),        /* ButtonText */
#if (WINVER >= 0x030a)
    RGB(0, 0, 0),        /* InactiveTitleText */
    RGB(0, 0, 0)         /* ButtonHilight */
#endif
};

const COLORREF* DISPLAY_GetSysColorDefaultsBinary(void)
{
    return g_SysColorDefaultsBinary;
}

int DISPLAY_GetSysColorCount(void)
{
    return sizeof(g_SysColorDefaultsBinary) / sizeof(COLORREF);
}

#if DEBUG
/* Function to dump configuration */
static void DISPLAY_DumpDisplayConfig(BYTE FAR* pData)
{
	DISPLAY_CONFIG_HEADER FAR* pHeader;
	BYTE FAR* pColors;
	int i, colorCount;
    
	pHeader = (DISPLAY_CONFIG_HEADER FAR*)pData;
    
	TRACE("=== DISPLAY.DRV Configuration Dump ===");
	TRACE("\nMetrics:");
	TRACE("  cyVThumb:          %u", pHeader->cyVThumb);
	TRACE("  cxHThumb:          %u", pHeader->cxHThumb);
	TRACE("  cxIconCompression: %u", pHeader->cxIconCompression);
	TRACE("  cyIconCompression: %u", pHeader->cyIconCompression);
	TRACE("  cxCursorCompression: %u", pHeader->cxCursorCompression);
	TRACE("  cyCursorCompression: %u", pHeader->cyCursorCompression);
	TRACE("  cyKanjiWindow:     %u", pHeader->cyKanjiWindow);
	TRACE("  cxBorder:          %u", pHeader->cxBorder);
	TRACE("  cyBorder:          %u", pHeader->cyBorder);

        pColors = (BYTE FAR*)(pHeader + 1); /* Colors start after the header */
        
        TRACE("\nColors (RGB):");
        for (i = 0; i < (sizeof(SysColorNames)/sizeof(SysColorNames[0])); i++) 
	{
            
                TRACE("  [%2d] %20s %3d,%3d,%3d", 
                      i,
                      SysColorNames[i],
                      pColors[i * 4 + 0],  /* R */
                      pColors[i * 4 + 1],  /* G */
                      pColors[i * 4 + 2]); /* B */
        }
        
    TRACE("=====================================");
}

#endif

VOID DISPLAY_Init()
{
	HDC hdc;
	HRSRC hRes;
	HGLOBAL hData;
	BYTE FAR* lpData;
	DWORD dwSize;
	DISPLAY_CONFIG_HEADER FAR* pHeader;
	BYTE FAR* pColors;
	int colorCount;
	FARPROC lpfnGetDriverResourceID;

	FUNCTION_START
    
	/* Try to get handle to already loaded display driver */
	hInstanceDisplay = GetModuleHandle(DISPLAY);
	if (hInstanceDisplay)
	{
		/* Search config.bin (ID 1) */
		lpfnGetDriverResourceID = GetProcAddress(hInstanceDisplay, "GetDriverResourceID");
		if (lpfnGetDriverResourceID)
			hRes = FindResource(hInstanceDisplay, (LPSTR)GetDriverResourceID(1, "oembin"), "oembin");
		else
			hRes = FindResource(hInstanceDisplay, MAKEINTRESOURCE(1), "oembin");
		if (hRes)
		{
			hData = LoadResource(hInstanceDisplay, hRes);
			if (hData)
			{
				lpData = (BYTE FAR*)LockResource(hData);
				if (lpData) 
				{
					int i;

					pHeader = (DISPLAY_CONFIG_HEADER FAR*)lpData;

					SysMetricsDef[SM_CYVTHUMB] = pHeader->cyVThumb;
					SysMetricsDef[SM_CXHTHUMB] = pHeader->cxHThumb;

// @todo seems this used to compress image resources from DISPLAY.DRV to real size
//	TRACE("  cxIconCompression: %u", pHeader->cxIconCompression);
//	TRACE("  cyIconCompression: %u", pHeader->cyIconCompression);
//	TRACE("  cxCursorCompression: %u", pHeader->cxCursorCompression);
//	TRACE("  cyCursorCompression: %u", pHeader->cyCursorCompression);

					SysMetricsDef[SM_CYKANJIWINDOW] = pHeader->cyKanjiWindow;
					SysMetricsDef[SM_CXBORDER] = pHeader->cxBorder;
					SysMetricsDef[SM_CYBORDER] = pHeader->cyBorder;

					pColors = lpData + sizeof(DISPLAY_CONFIG_HEADER);

					for (i = 0; i < DISPLAY_GetSysColorCount(); i++) 
					{
						g_SysColorDefaultsBinary[i] = RGB(pColors[i * 4 + 0], pColors[i * 4 + 1], pColors[i * 4 + 2]);
					}

					#ifdef DEBUG
					DISPLAY_DumpDisplayConfig(lpData);
					#endif

					UnlockResource(hData);
				}
				FreeResource(hData);
			}
		}
	}    

	/* Get screen resolution */
	hdc = GetDC(0);
	if (hdc) 
	{
		// SysMetricsDef[SM_CXMAXTRACK] = SysMetricsDef[SM_CXMAXIMIZED] = // This is for Win95
		SysMetricsDef[SM_CXFULLSCREEN] = SysMetricsDef[SM_CXSCREEN] = GetDeviceCaps(hdc, HORZRES);

		// SysMetricsDef[SM_CYMAXTRACK] = SysMetricsDef[SM_CYMAXIMIZED] = // This is for Win95
		SysMetricsDef[SM_CYSCREEN] = GetDeviceCaps(hdc, VERTRES);
		SysMetricsDef[SM_CYFULLSCREEN] = SysMetricsDef[SM_CYSCREEN] - SysMetricsDef[SM_CYCAPTION];

		ReleaseDC(0, hdc);
		TRACE("Screen resolution: %dx%d", SysMetricsDef[SM_CXSCREEN], SysMetricsDef[SM_CYSCREEN]);
	}
    
	FUNCTION_END
}
