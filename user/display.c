/*
 * DISPLAY.DRV interface
 *
 * Copyright 2026 Yuri Prokushev
 *
 * This is interface to DISPLAY.DRV. For now it is support only Windows 3.0 things.
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
// Starting from here something another are used
#define OEM_CONFIG_WIN31  21  /* Windows 3.1 - 21 colors (up to COLOR_BTNHIGHLIGHT) */
#define OEM_CONFIG_WIN95  25  /* Windows 95 - 25 colors (up to COLOR_INFOBK) */
#define OEM_CONFIG_WIN98  25  /* Windows 98 - 29 colors (up to COLOR_GRADIENTINACTIVECAPTION) */

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
    /* After this comes a variable-length color array */
} DISPLAY_CONFIG_HEADER;
#pragma pack(pop)

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
#if WINVER >= 0x030a
    "InactiveCaptionText", /* COLOR_INACTIVECAPTIONTEXT = 19 (WINVER >= 0x030a) */
    "BtnHighlight",     /* COLOR_BTNHIGHLIGHT = 20 (WINVER >= 0x030a) */
#endif
#if WINVER >= 0x0400
    "3DDkShadow",       /* COLOR_3DDKSHADOW = 21 (WINVER >= 0x0400) */
    "3DLight",          /* COLOR_3DLIGHT = 22 (WINVER >= 0x0400) */
    "InfoText",         /* COLOR_INFOTEXT = 23 (WINVER >= 0x0400) */
    "InfoBk",           /* COLOR_INFOBK = 24 (WINVER >= 0x0400) */
#endif
#if WINVER >= 0x0410
    "AlternateBtnFace",             /* COLOR_ALTERNATEBTNFACE = 25 (WINVER >= 0x0410) */
    "HotLight",                     /* COLOR_HOTLIGHT = 26 (WINVER >= 0x0410) */
    "GradientActiveCaption",        /* COLOR_GRADIENTACTIVECAPTION = 27 (WINVER >= 0x0410) */
    "GradientInactiveCaption"       /* COLOR_GRADIENTINACTIVECAPTION = 28 (WINVER >= 0x0410) */
#endif
};

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

#if 0
/* Function to update system metrics from config */
static void UpdateSysMetricsFromConfig(DISPLAY_CONFIG_HEADER FAR* pHeader)
{
    /* Update system metrics */
    SysMetricsDef[SM_CYVTHUMB] = pHeader->cyVThumb;         /* Index 9 */
    SysMetricsDef[SM_CXHTHUMB] = pHeader->cxHThumb;         /* Index 10 */
    SysMetricsDef[SM_CYKANJIWINDOW] = pHeader->cyKanjiWindow; /* Index 18 */
    SysMetricsDef[SM_CXBORDER] = pHeader->cxBorder;         /* Index 5 */
    SysMetricsDef[SM_CYBORDER] = pHeader->cyBorder;         /* Index 6 */
    
    /* Apply compression coefficients to icon and cursor sizes */
    if (pHeader->cxIconCompression > 0) {
        SysMetricsDef[SM_CXICON] = 32 / pHeader->cxIconCompression; /* Index 11 */
        SysMetricsDef[SM_CYICON] = 32 / pHeader->cyIconCompression; /* Index 12 */
        TRACE("UpdateSysMetricsFromConfig: icons compressed %dx%d -> %dx%d", 
              32, 32, 
              SysMetricsDef[SM_CXICON], SysMetricsDef[SM_CYICON]);
    }
    
    if (pHeader->cxCursorCompression > 0) {
        SysMetricsDef[SM_CXCURSOR] = 32 / pHeader->cxCursorCompression; /* Index 13 */
        SysMetricsDef[SM_CYCURSOR] = 32 / pHeader->cyCursorCompression; /* Index 14 */
        TRACE("UpdateSysMetricsFromConfig: cursors compressed %dx%d -> %dx%d", 
              32, 32, 
              SysMetricsDef[SM_CXCURSOR], SysMetricsDef[SM_CYCURSOR]);
    }
}

/* Function to update system colors from config */
static void UpdateSysColorsFromConfig(BYTE FAR* pColors, int colorCount)
{
    int i;
    
    TRACE("UpdateSysColorsFromConfig: updating %d colors from config", colorCount);
    TRACE("Current Windows version: %d.%d", HIBYTE(wWinVer), LOBYTE(wWinVer));
    
    /* Update system colors in a loop */
    for (i = 0; i < min(colorCount, NUM_SYS_COLORS); i++) {
        /* Check if this color is supported in the current Windows version */
        BOOL bSupported = TRUE;
        
        if (i >= COLOR_INACTIVECAPTIONTEXT && wWinVer < WINVER_30A) {
            bSupported = FALSE;
            TRACE("  [%2d] %-20s SKIPPED (not supported in Win%d.%d)", 
                  i, SysColorNames[i], HIBYTE(wWinVer), LOBYTE(wWinVer));
            continue;
        } else if (i >= COLOR_3DDKSHADOW && wWinVer < WINVER_400) {
            bSupported = FALSE;
            TRACE("  [%2d] %-20s SKIPPED (not supported in Win%d.%d)", 
                  i, SysColorNames[i], HIBYTE(wWinVer), LOBYTE(wWinVer));
            continue;
        }
        
        if (bSupported) {
            BYTE r = pColors[i * 4 + 0];
            BYTE g = pColors[i * 4 + 1];
            BYTE b = pColors[i * 4 + 2];
            
            SysColors[i] = RGB(r, g, b);
            
            TRACE("  [%2d] %-20s = RGB(%3d, %3d, %3d)", 
                  i, 
                  i < (int)(sizeof(SysColorNames)/sizeof(SysColorNames[0])) ? SysColorNames[i] : "Unknown",
                  r, g, b);
        }
    }
    
    /* If the config has fewer colors than supported by the system, set default values */
    if (colorCount < NUM_SYS_COLORS) {
        TRACE("UpdateSysColorsFromConfig: setting default values for unsupported colors");
        
        /* For Windows 3.0: if the config is old (13 colors), fill in the missing ones */
        if (wWinVer >= WINVER_300 && colorCount == OEM_CONFIG_OLD) {
            /* Windows 3.0 with config from Windows 2.x */
            for (i = colorCount; i < min(NUM_SYS_COLORS, 20); i++) {
                switch (i) {
                    case COLOR_HIGHLIGHT:          /* 13 */
                        SysColors[i] = RGB(166, 202, 240);
                        break;
                    case COLOR_HIGHLIGHTTEXT:      /* 14 */
                        SysColors[i] = RGB(0, 0, 0);
                        break;
                    case COLOR_BTNFACE:            /* 15 */
                        SysColors[i] = RGB(192, 192, 192);
                        break;
                    case COLOR_BTNSHADOW:          /* 16 */
                        SysColors[i] = RGB(128, 128, 128);
                        break;
                    case COLOR_GRAYTEXT:           /* 17 */
                        SysColors[i] = RGB(192, 192, 192);
                        break;
                    case COLOR_BTNTEXT:            /* 18 */
                        SysColors[i] = RGB(0, 0, 0);
                        break;
                    case COLOR_INACTIVECAPTIONTEXT: /* 19 */
                        if (wWinVer >= WINVER_30A) {
                            SysColors[i] = RGB(0, 0, 0);
                        }
                        break;
                    default:
                        SysColors[i] = RGB(0, 0, 0);
                }
                
                if (i < NUM_SYS_COLORS) {
                    TRACE("  [%2d] %-20s = set to default (RGB(%d,%d,%d))", 
                          i, 
                          i < (int)(sizeof(SysColorNames)/sizeof(SysColorNames[0])) ? SysColorNames[i] : "Unknown",
                          GetRValue(SysColors[i]),
                          GetGValue(SysColors[i]),
                          GetBValue(SysColors[i]));
                }
            }
        }
        
        /* For Windows 3.1 with config from Windows 3.0 (19 colors) */
        if (wWinVer >= WINVER_30A && colorCount == OEM_CONFIG_WIN30) {
            /* Add colors 19-20 */
            if (NUM_SYS_COLORS > COLOR_INACTIVECAPTIONTEXT) {
                SysColors[COLOR_INACTIVECAPTIONTEXT] = RGB(0, 0, 0);
                TRACE("  [19] InactiveCaptionText = RGB(0, 0, 0) [default for Win3.1]");
            }
            if (NUM_SYS_COLORS > COLOR_BTNHIGHLIGHT) {
                SysColors[COLOR_BTNHIGHLIGHT] = RGB(255, 255, 255);
                TRACE("  [20] BtnHighlight = RGB(255, 255, 255) [default for Win3.1]");
            }
        }
        
        /* For Windows 95+ with config from Windows 3.1 (21 colors) */
        if (wWinVer >= WINVER_400 && colorCount == OEM_CONFIG_WIN31) {
            /* Add colors 21-24 */
            if (NUM_SYS_COLORS > COLOR_3DDKSHADOW) {
                SysColors[COLOR_3DDKSHADOW] = RGB(0, 0, 0);
                TRACE("  [21] 3DDkShadow = RGB(0, 0, 0) [default for Win95+]");
            }
            if (NUM_SYS_COLORS > COLOR_3DLIGHT) {
                SysColors[COLOR_3DLIGHT] = RGB(192, 192, 192);
                TRACE("  [22] 3DLight = RGB(192, 192, 192) [default for Win95+]");
            }
            if (NUM_SYS_COLORS > COLOR_INFOTEXT) {
                SysColors[COLOR_INFOTEXT] = RGB(0, 0, 0);
                TRACE("  [23] InfoText = RGB(0, 0, 0) [default for Win95+]");
            }
            if (NUM_SYS_COLORS > COLOR_INFOBK) {
                SysColors[COLOR_INFOBK] = RGB(255, 255, 0);
                TRACE("  [24] InfoBk = RGB(255, 255, 0) [default for Win95+]");
            }
        }
    }
}

/* Function to update GDI objects from updated colors */
static void UpdateSysColorObjectsFromColors(void)
{
    int i;
    
    TRACE("UpdateSysColorObjectsFromColors: updating GDI objects");
    
    /* Update brushes and pens for all colors that have objects */
    for (i = 0; i < NUM_SYS_COLORS; i++) {
        /* Skip colors not supported in the current version */
        if (i >= COLOR_INACTIVECAPTIONTEXT && wWinVer < WINVER_30A) {
            continue;
        }
        if (i >= COLOR_3DDKSHADOW && wWinVer < WINVER_400) {
            continue;
        }
        
        SYSCOLOR_SetColor(i, SysColors[i]);
    }
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
	HInstanceDisplay = GetModuleHandle(DISPLAY);
	if (HInstanceDisplay)
	{
		/* Search config.bin (ID 1) */
		lpfnGetDriverResourceID = GetProcAddress(HInstanceDisplay, "GetDriverResourceID");
		if (lpfnGetDriverResourceID)
			hRes = FindResource(HInstanceDisplay, (LPSTR)GetDriverResourceID(1, "oembin")/*MAKEINTRESOURCE(1)*/, "oembin");
		else
			hRes = FindResource(HInstanceDisplay, MAKEINTRESOURCE(1), "oembin");
		if (hRes) 
		{
			hData = LoadResource(HInstanceDisplay, hRes);
			if (hData)
			{
				lpData = (BYTE FAR*)LockResource(hData);
				if (lpData) 
				{
					pHeader = (DISPLAY_CONFIG_HEADER FAR*)lpData;
					pColors = lpData + sizeof(DISPLAY_CONFIG_HEADER);
    
					DISPLAY_DumpDisplayConfig(lpData);
				}
			}
		}
	}    

#if 0    
    if (colorCount > 0) {
        /* Update system metrics */
        TRACE("LW_OEMDependentInit: Updating system metrics from config");
        UpdateSysMetricsFromConfig(pHeader);
        
        /* Update system colors */
        TRACE("LW_OEMDependentInit: Updating system colors from config");
        UpdateSysColorsFromConfig(pColors, colorCount);
        
        /* Update GDI objects */
        TRACE("LW_OEMDependentInit: Updating GDI objects");
        UpdateSysColorObjectsFromColors();
        
        bConfigLoaded = TRUE;
    } else {
        TRACE("LW_OEMDependentInit: Unknown config format");
    }
#endif
    
    /* Free resources */
    UnlockResource(hData);
    FreeResource(hData);
    
   
    /* Get screen resolution */
    hdc = GetDC(0);
    if (hdc) {
        CXScreen = GetDeviceCaps(hdc, HORZRES);
        CYScreen = GetDeviceCaps(hdc, VERTRES);
        ReleaseDC(0, hdc);
        TRACE("LW_OEMDependentInit: Screen resolution: %dx%d", CXScreen, CYScreen);
    } else {
        CXScreen = 640;
        CYScreen = 480;
        TRACE("LW_OEMDependentInit: Failed to get resolution, using %dx%d", CXScreen, CYScreen);
    }
    
    /* Update other system metrics based on screen resolution */
    LW_InitSysMetrics();
    
    FUNCTION_END
}
