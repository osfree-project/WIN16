#include <windows.h>

WORD USER_HeapSel = 0;  /* USER heap selector */

static HINSTANCE gdi_inst;

/***********************************************************************
 *		OldExitWindows (USER.2)
 */
void WINAPI
OldExitWindows(void)
{
        ExitWindows(0,0);
}

/**********************************************************************
 *		DllEntryPoint (USER.374)
 * 
 * We calling this from LibEntry function
 */
BOOL WINAPI DllEntryPoint( DWORD reason, HINSTANCE inst, WORD ds,
                           WORD heap, DWORD reserved1, WORD reserved2 )
{
    if (reason != 1/*DLL_PROCESS_ATTACH*/) return TRUE;
    if (USER_HeapSel) return TRUE;  /* already called */

    USER_HeapSel = ds;
//    register_wow_handlers();
    gdi_inst = LoadLibrary( "gdi.exe" );
    LoadLibrary( "display.drv" );
    LoadLibrary( "keyboard.drv" );
    LoadLibrary( "mouse.drv" );
    LoadLibrary( "user.exe" );  /* make sure it never gets unloaded */
    return TRUE;
}

/* UserSeeUserDo parameters */
#define USUD_LOCALALLOC        0x0001
#define USUD_LOCALFREE         0x0002
#define USUD_LOCALCOMPACT      0x0003
#define USUD_LOCALHEAP         0x0004
#define USUD_FIRSTCLASS        0x0005

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

/* This function sets current DS value */
extern  void          SetDS( unsigned short );
#pragma aux SetDS               = \
        "mov    ds,ax"          \
        parm                   [ax];

/***********************************************************************
 *		UserSeeUserDo (USER.216)
 */
DWORD WINAPI UserSeeUserDo(WORD wReqType, WORD wParam1, WORD wParam2, WORD wParam3)
{
    WORD oldDS = GetDS();
    DWORD ret = (DWORD)-1;

    SetDS(USER_HeapSel);
    switch (wReqType)
    {
    case USUD_LOCALALLOC:
        ret = LocalAlloc(wParam1, wParam3);
        break;
    case USUD_LOCALFREE:
        ret = LocalFree(wParam1);
        break;
    case USUD_LOCALCOMPACT:
        ret = LocalCompact(wParam3);
        break;
    case USUD_LOCALHEAP:
//        ret = USER_HeapSel;
        break;
    case USUD_FIRSTCLASS:
//        FIXME("return a pointer to the first window class.\n");
        break;
    default:
	{}//        WARN("wReqType %04x (unknown)\n", wReqType);
    }
    SetDS(oldDS);
    return ret;
}

/***********************************************************************
 *           OldSetDeskPattern   (USER.279)
 */
BOOL WINAPI OldSetDeskPattern(void)
{
    return SystemParametersInfo( SPI_SETDESKPATTERN, -1, NULL, FALSE );
}

WORD WINAPI LocalCountFree();
WORD WINAPI LocalHeapSize();


/***********************************************************************
 *		GetFreeSystemResources (USER.284)
 */
UINT WINAPI GetFreeSystemResources( UINT resType )
{
    WORD oldDS = GetDS();
    int userPercent, gdiPercent;

    switch(resType)
    {
    case GFSR_USERRESOURCES:
        SetDS(USER_HeapSel);
        userPercent = (int)LocalCountFree() * 100 / LocalHeapSize();
        gdiPercent  = 100;
        SetDS(oldDS);
        break;

    case GFSR_GDIRESOURCES:
        SetDS(gdi_inst);
        gdiPercent  = (int)LocalCountFree() * 100 / LocalHeapSize();
        userPercent = 100;
        SetDS(oldDS);
        break;

    case GFSR_SYSTEMRESOURCES:
        SetDS(USER_HeapSel);
        userPercent = (int)LocalCountFree() * 100 / LocalHeapSize();
        SetDS(gdi_inst);
        gdiPercent  = (int)LocalCountFree() * 100 / LocalHeapSize();
        SetDS(oldDS);
        break;

    default:
        userPercent = gdiPercent = 0;
        break;
    }
//    TRACE("<- userPercent %d, gdiPercent %d\n", userPercent, gdiPercent);
    return (UINT)min( userPercent, gdiPercent );
}
