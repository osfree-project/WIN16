#include <user.h>


static HINSTANCE gdi_inst;

/***********************************************************************
 *		OldExitWindows (USER.2)
 */
void WINAPI
OldExitWindows(void)
{
	FUNCTION_START
        ExitWindows(0,0);
}

/**********************************************************************
 *		DllEntryPoint (USER.374)
 * 
 * We calling this from LibEntry function @todo not sure yet is is correct declaration...
 */
BOOL WINAPI DllEntryPoint( DWORD reason, HINSTANCE inst, WORD ds,
                           WORD heap, DWORD reserved1, WORD reserved2 )
{
	FUNCTION_START
	
	TRACE("reason=%d", reason);

    if (reason != 1/*DLL_PROCESS_ATTACH*/)
	{
		FUNCTION_END
		return TRUE;
	}
    if (USER_HeapSel) 
	{
		FUNCTION_END
		return TRUE;  /* already called */
	}


    USER_HeapSel = ds;
    gdi_inst = LoadLibrary( "gdi.exe" );

	FUNCTION_END
    return TRUE;
}

/* UserSeeUserDo parameters */
#define USUD_LOCALALLOC        0x0001
#define USUD_LOCALFREE         0x0002
#define USUD_LOCALCOMPACT      0x0003
#define USUD_LOCALHEAP         0x0004
#define USUD_FIRSTCLASS        0x0005


/***********************************************************************
 *		UserSeeUserDo (USER.216)
 */
DWORD WINAPI UserSeeUserDo(WORD wReqType, WORD wParam1, WORD wParam2, WORD wParam3)
{
    WORD oldDS = GetDS();
    DWORD ret = (DWORD)-1;

	FUNCTION_START

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
        ret = USER_HeapSel;
        break;
    case USUD_FIRSTCLASS:
        FIXME("return a pointer to the first window class.\n");
        break;
    default:
		WARN("wReqType %04x (unknown)\n", wReqType);
    }
    SetDS(oldDS);
    return ret;
}

/***********************************************************************
 *		GetFreeSystemResources (USER.284)
 */
UINT WINAPI GetFreeSystemResources( UINT resType )
{
    WORD oldDS = GetDS();
    int userPercent, gdiPercent;

	FUNCTION_START

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
    TRACE("<- userPercent %d, gdiPercent %d\n", userPercent, gdiPercent);
    return (UINT)min( userPercent, gdiPercent );
}
