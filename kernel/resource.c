#include <win16.h>
#include <win_private.h>

/*
 * Resource table structures.
 */
typedef struct
{
    WORD     offset;
    WORD     length;
    WORD     flags;
    WORD     id;
    HANDLE   handle;
    WORD     usage;
} NE_NAMEINFO;

typedef struct
{
    WORD        type_id;   /* Type identifier */
    WORD        count;     /* Number of resources of this type */
    FARPROC   resloader; /* SetResourceHandler() */
    /*
     * Name info array.
     */
} NE_TYPEINFO;

#define NE_SEGFLAGS_LOADED      0x0004

// @todo use exported variables
#define __AHSHIFT  3  /* don't change! */
#define __AHINCR   (1 << __AHSHIFT)

#define VALID_HANDLE(handle) (((handle)>>__AHSHIFT)<globalArenaSize)
#define GET_ARENA_PTR(handle)  (pGlobalArena + ((handle) >> __AHSHIFT))

HMODULE WINAPI GetExePtr( HANDLE handle );

// @todo Implement this as fast as pGlobalArena implemented
/***********************************************************************
 *           FarSetOwner   (KERNEL.403)
 */
void WINAPI FarSetOwner( HGLOBAL handle, HANDLE hOwner )
{
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to FarSetOwner!\n",handle);
	return;
//    }
//    GET_ARENA_PTR(handle)->hOwner = hOwner;
}

// @todo Implement this as fast as pGlobalArena implemented
/***********************************************************************
 *           FarGetOwner   (KERNEL.404)
 */
HANDLE WINAPI FarGetOwner( HGLOBAL handle )
{
//    if (!VALID_HANDLE(handle)) {
//	WARN("Invalid handle 0x%04x passed to FarGetOwner!\n",handle);
	return 0;
//    }
//    return GET_ARENA_PTR(handle)->hOwner;
}


/***********************************************************************
 *           NE_GetPtr
 */
NE_MODULE *NE_GetPtr( HMODULE hModule )
{
    return (NE_MODULE *)GlobalLock( GetExePtr(hModule) );
}

static inline NE_MODULE *get_module( HMODULE mod )
{
    if (!mod) mod = *((HMODULE far *)MAKELP(GetCurrentTask(), 0x1e));
    return NE_GetPtr( mod );
}

/**********************************************************************
 *          next_typeinfo
 */
static inline NE_TYPEINFO *next_typeinfo( NE_TYPEINFO *info )
{
    return (NE_TYPEINFO *)((char*)(info + 1) + info->count * sizeof(NE_NAMEINFO));
}


/**********************************************************************
 *          get_default_res_handler
 */
static inline FARPROC get_default_res_handler(void)
{
    static FARPROC handler;

    if (!handler) handler = GetProcAddress( GetModuleHandle("KERNEL"), "DefResourceHandler" );
    return handler;
}

typedef unsigned long ULONG_PTR, *PULONG_PTR;

#define MAKEINTRESOURCEA(i) (LPSTR)((ULONG_PTR)((WORD)(i)))

/**********************************************************************
 *          get_res_name
 *
 * Convert a resource name from '#xxx' form to numerical id.
 */
static inline LPCSTR get_res_name( LPCSTR name )
{
    if (HIWORD(name) && name[0] == '#') name = MAKEINTRESOURCEA( atoi( name + 1 ) );
    return name;
}

/***********************************************************************
 *           NE_FindNameTableId
 *
 * Find the type and resource id from their names.
 * Return value is MAKELONG( typeId, resId ), or 0 if not found.
 */
static DWORD NE_FindNameTableId(HMODULE hModule, NE_MODULE *pModule, LPCSTR typeId, LPCSTR resId )
{
    NE_TYPEINFO *pTypeInfo = (NE_TYPEINFO *)((char *)pModule + pModule->ne_rsrctab + 2);
    NE_NAMEINFO *pNameInfo;
    HGLOBAL handle;
    LPWORD p;
    DWORD ret = 0;
    int count;

    for (; pTypeInfo->type_id != 0;
           pTypeInfo = (NE_TYPEINFO *)((char*)(pTypeInfo+1) +
                                        pTypeInfo->count * sizeof(NE_NAMEINFO)))
    {
        if (pTypeInfo->type_id != 0x800f) continue;
        pNameInfo = (NE_NAMEINFO *)(pTypeInfo + 1);
        for (count = pTypeInfo->count; count > 0; count--, pNameInfo++)
        {
//            TRACE("NameTable entry: type=%04x id=%04x\n",
//                              pTypeInfo->type_id, pNameInfo->id );
            handle = LoadResource( hModule,
                                     (HRSRC)((char *)pNameInfo - (char *)pModule) );
            for(p = (LPWORD)LockResource(handle); p && *p; p = (WORD *)((char *)p+*p))
            {
//                TRACE("  type=%04x '%s' id=%04x '%s'\n",
//                                  p[1], (char *)(p+3), p[2],
//                                  (char *)(p+3)+strlen((char *)(p+3))+1 );
                /* Check for correct type */

                if (p[1] & 0x8000)
                {
                    if (!HIWORD(typeId)) continue;
                    if (stricmp( typeId, (char *)(p + 3) )) continue;
                }
                else if (HIWORD(typeId) || (((DWORD)typeId & ~0x8000)!= p[1]))
                  continue;

                /* Now check for the id */

                if (p[2] & 0x8000)
                {
                    if (!HIWORD(resId)) continue;
                    if (stricmp( resId, (char*)(p+3)+strlen((char*)(p+3))+1 )) continue;

                }
                else if (HIWORD(resId) || ((LOWORD(resId) & ~0x8000) != p[2]))
                  continue;

                /* If we get here, we've found the entry */

//                TRACE("  Found!\n" );
                ret = MAKELONG( p[1], p[2] );
                break;
            }
            FreeResource( handle );
            if (ret) return ret;
        }
    }
    return 0;
}

/***********************************************************************
 *           NE_FindTypeSection
 *
 * Find header struct for a particular resource type.
 */
static NE_TYPEINFO *NE_FindTypeSection( LPBYTE pResTab, NE_TYPEINFO *pTypeInfo, LPCSTR typeId )
{
    /* start from pTypeInfo */

    if (HIWORD(typeId) != 0)  /* Named type */
    {
        LPCSTR str = typeId;
        BYTE len = lstrlen( str );
        while (pTypeInfo->type_id)
        {
            if (!(pTypeInfo->type_id & 0x8000))
            {
                LPBYTE p = pResTab + pTypeInfo->type_id;
                if ((*p == len) && !strnicmp( (char*)p+1, str, len ))
                {
//                    TRACE("  Found type '%s'\n", str );
                    return pTypeInfo;
                }
            }
//            TRACE("  Skipping type %04x\n", pTypeInfo->type_id );
            pTypeInfo = next_typeinfo(pTypeInfo);
        }
    }
    else  /* Numeric type id */
    {
        WORD id = LOWORD(typeId) | 0x8000;
        while (pTypeInfo->type_id)
        {
            if (pTypeInfo->type_id == id)
            {
//                TRACE("  Found type %04x\n", id );
                return pTypeInfo;
            }
//            TRACE("  Skipping type %04x\n", pTypeInfo->type_id );
            pTypeInfo = next_typeinfo(pTypeInfo);
        }
    }
    return NULL;
}

/***********************************************************************
 *           NE_FindResourceFromType
 *
 * Find a resource once the type info structure has been found.
 */
static NE_NAMEINFO *NE_FindResourceFromType( LPBYTE pResTab, NE_TYPEINFO *pTypeInfo, LPCSTR resId )
{
    LPBYTE p;
    int count;
    NE_NAMEINFO *pNameInfo = (NE_NAMEINFO *)(pTypeInfo + 1);

    if (HIWORD(resId) != 0)  /* Named resource */
    {
        LPCSTR str = resId;
        BYTE len = strlen( str );
        for (count = pTypeInfo->count; count > 0; count--, pNameInfo++)
        {
            if (pNameInfo->id & 0x8000) continue;
            p = pResTab + pNameInfo->id;
            if ((*p == len) && !strnicmp( (char*)p+1, str, len ))
                return pNameInfo;
        }
    }
    else  /* Numeric resource id */
    {
        WORD id = LOWORD(resId) | 0x8000;
        for (count = pTypeInfo->count; count > 0; count--, pNameInfo++)
            if (pNameInfo->id == id) return pNameInfo;
    }
    return NULL;
}

/**********************************************************************
 *	    AllocResource    (KERNEL.66)
 */
HGLOBAL WINAPI AllocResource( HMODULE hModule, HRSRC hRsrc, DWORD size)
{
    NE_NAMEINFO *pNameInfo=NULL;
    WORD sizeShift;
    HGLOBAL ret;

    NE_MODULE *pModule = NE_GetPtr( hModule );
    if (!pModule || !pModule->ne_rsrctab || !hRsrc) return 0;

//    TRACE("module=%04x res=%04x size=%ld\n", hModule, hRsrc, size );

    sizeShift = *(WORD *)((char *)pModule + pModule->ne_rsrctab);
    pNameInfo = (NE_NAMEINFO*)((char*)pModule + hRsrc);
    if (size < (DWORD)pNameInfo->length << sizeShift)
        size = (DWORD)pNameInfo->length << sizeShift;
    ret = GlobalAlloc( GMEM_FIXED, size );
    if (ret) FarSetOwner( ret, hModule );
    return ret;
}

/**********************************************************************
 *	SetResourceHandler	(KERNEL.67)
 */
FARPROC WINAPI SetResourceHandler( HMODULE hModule, LPCSTR typeId, FARPROC resourceHandler )
{
    LPBYTE pResTab;
    NE_TYPEINFO *pTypeInfo;
    FARPROC prevHandler = NULL;
    NE_MODULE *pModule = NE_GetPtr( hModule );

    if (!pModule || !pModule->ne_rsrctab) return NULL;

    pResTab = (LPBYTE)pModule + pModule->ne_rsrctab;
    pTypeInfo = (NE_TYPEINFO *)(pResTab + 2);

//    TRACE("module=%04x type=%s\n", hModule, debugstr_a(typeId) );

    for (;;)
    {
        if (!(pTypeInfo = NE_FindTypeSection( pResTab, pTypeInfo, typeId )))
            break;
        prevHandler = pTypeInfo->resloader;
        pTypeInfo->resloader = resourceHandler;
        pTypeInfo = next_typeinfo(pTypeInfo);
    }
    if (!prevHandler) prevHandler = get_default_res_handler();
    return prevHandler;
}

/**********************************************************************
 *          SizeofResource   (KERNEL.65)
 */
DWORD WINAPI SizeofResource( HMODULE hModule, HRSRC hRsrc )
{
    NE_MODULE *pModule;

//    TRACE("(%x, %x)\n", hModule, hRsrc );

    if (!hRsrc) return 0;
    if (!(pModule = get_module( hModule ))) return 0;
    if (pModule->ne_rsrctab)
    {
        WORD sizeShift = *(WORD *)((char *)pModule + pModule->ne_rsrctab);
        NE_NAMEINFO *pNameInfo = (NE_NAMEINFO*)((char*)pModule + hRsrc);
        return (DWORD)pNameInfo->length << sizeShift;
    }
    return 0;
}

/**********************************************************************
 *          AccessResource (KERNEL.64)
 */
int WINAPI AccessResource( HINSTANCE hModule, HRSRC hRsrc )
{
    HFILE fd;
    NE_MODULE *pModule = NE_GetPtr( hModule );

    if (!pModule || !pModule->ne_rsrctab || !hRsrc) return -1;

//    TRACE("module=%04x res=%04x\n", pModule->self, hRsrc );

    if ((fd = _lopen( NE_MODULE_NAME(pModule), OF_READ )) != HFILE_ERROR)
    {
        WORD sizeShift = *(WORD *)((char *)pModule + pModule->ne_rsrctab);
        NE_NAMEINFO *pNameInfo = (NE_NAMEINFO*)((char*)pModule + hRsrc);
        _llseek( fd, (int)pNameInfo->offset << sizeShift, SEEK_SET );
    }
    return fd;
}

/**********************************************************************
 *          FreeResource     (KERNEL.63)
 */
BOOL WINAPI FreeResource( HGLOBAL handle )
{
    FARPROC proc;
    HMODULE user;
    NE_MODULE *pModule = NE_GetPtr( FarGetOwner( handle ) );

//    TRACE("(%04x)\n", handle );

    /* Try NE resource first */

    if (pModule && pModule->ne_rsrctab)
    {
        NE_TYPEINFO *pTypeInfo = (NE_TYPEINFO *)((char *)pModule + pModule->ne_rsrctab + 2);
        while (pTypeInfo->type_id)
        {
            WORD count;
            NE_NAMEINFO *pNameInfo = (NE_NAMEINFO *)(pTypeInfo + 1);
            for (count = pTypeInfo->count; count > 0; count--)
            {
                if (pNameInfo->handle == handle)
                {
                    if (pNameInfo->usage > 0) pNameInfo->usage--;
                    if (pNameInfo->usage == 0)
                    {
                        GlobalFree( pNameInfo->handle );
                        pNameInfo->handle = 0;
                        pNameInfo->flags &= ~NE_SEGFLAGS_LOADED;
                    }
                    return FALSE;
                }
                pNameInfo++;
            }
            pTypeInfo = (NE_TYPEINFO *)pNameInfo;
        }
    }

    /* If this failed, call USER.DestroyIcon32; this will check
       whether it is a shared cursor/icon; if not it will call
       GlobalFree16() */
/*    user = GetModuleHandle( "user" );
    if (user && (proc = GetProcAddress( user, "DestroyIcon32" )))
    {
        WORD args[2];
        DWORD result;

        args[1] = handle;
        args[0] = 1;  // CID_RESOURCE 
        WOWCallback16Ex( (SEGPTR)proc, WCB16_PASCAL, sizeof(args), args, &result );
        return LOWORD(result);
    }
    else */
        return GlobalFree( handle );
}

/**********************************************************************
 *          LockResource   (KERNEL.62)
 */
LPSTR WINAPI LockResource(HGLOBAL handle)
{
//    TRACE("(%04x)\n", handle );
    /* May need to reload the resource if discarded */
    return GlobalLock(handle);
}

/***********************************************************************
 *           DefResourceHandler (KERNEL.456)
 *
 * This is the default LoadProc() function.
 */
HGLOBAL WINAPI DefResourceHandler( HGLOBAL hMemObj, HMODULE hModule,
                                        HRSRC hRsrc )
{
    HGLOBAL handle;
    WORD sizeShift;
    NE_NAMEINFO* pNameInfo;
    NE_MODULE* pModule = NE_GetPtr( hModule );

    if (!pModule) return 0;

    sizeShift = *(WORD *)((char *)pModule + pModule->ne_rsrctab);
    pNameInfo = (NE_NAMEINFO *)((char *)pModule + hRsrc);

    if ( hMemObj )
        handle = GlobalReAlloc( hMemObj, pNameInfo->length << sizeShift, 0 );
    else
        handle = AllocResource( hModule, hRsrc, 0 );

    if (handle)
    {
// @todo make real read here, no Wine extension
//        if (!NE_READ_DATA( pModule, GlobalLock( handle ),
//                           (int)pNameInfo->offset << sizeShift,
//                           (int)pNameInfo->length << sizeShift ))
        {
            GlobalFree( handle );
            handle = 0;
        }
    }
    return handle;
}

/**********************************************************************
 *          LoadResource     (KERNEL.61)
 */
HGLOBAL WINAPI LoadResource(HINSTANCE hModule, HRSRC hRsrc)
{
    NE_TYPEINFO *pTypeInfo;
    NE_NAMEINFO *pNameInfo = NULL;
    NE_MODULE *pModule = get_module( hModule );
    int d;

    if (!hRsrc || !pModule) return 0;


    /* first, verify hRsrc (just an offset from pModule to the needed pNameInfo) */

    d = pModule->ne_rsrctab + 2;
    pTypeInfo = (NE_TYPEINFO *)((char *)pModule + d);
    while( hRsrc > d )
    {
        if (pTypeInfo->type_id == 0) break; /* terminal entry */
        d += sizeof(NE_TYPEINFO) + pTypeInfo->count * sizeof(NE_NAMEINFO);
        if (hRsrc < d)
        {
            if( ((d - hRsrc)%sizeof(NE_NAMEINFO)) == 0 )
            {
                pNameInfo = (NE_NAMEINFO *)((char *)pModule + hRsrc);
                break;
            }
            else break; /* NE_NAMEINFO boundary mismatch */
        }
        pTypeInfo = (NE_TYPEINFO *)((char *)pModule + d);
    }

    if (pNameInfo)
    {
        if (pNameInfo->handle && !(GlobalFlags(pNameInfo->handle) & GMEM_DISCARDED))
        {
            pNameInfo->usage++;
//            TRACE("  Already loaded, new count=%d\n", pNameInfo->usage );
	        }
        else
        {
            FARPROC resloader = pTypeInfo->resloader;
            if (resloader && resloader != get_default_res_handler())
            {
                WORD args[3];
                DWORD ret;

                args[2] = pNameInfo->handle;
//                args[1] = pModule->self;
                args[0] = hRsrc;
//@todo call resloader here
//                WOWCallback16Ex( (DWORD)resloader, WCB16_PASCAL, sizeof(args), args, &ret );
                pNameInfo->handle = LOWORD(ret);
            }
            else
                pNameInfo->handle = DefResourceHandler( pNameInfo->handle, hModule, hRsrc );

            if (pNameInfo->handle)
            {
                pNameInfo->usage++;
                pNameInfo->flags |= NE_SEGFLAGS_LOADED;
            }
        }
        return pNameInfo->handle;
    }
    return 0;
}

/**********************************************************************
 *          FindResource     (KERNEL.60)
 */
HRSRC WINAPI FindResource(HMODULE hModule, LPCSTR name, LPCSTR type)
{
    NE_TYPEINFO *pTypeInfo;
    NE_NAMEINFO *pNameInfo;
    LPBYTE pResTab;
    NE_MODULE *pModule = get_module( hModule );

    if (!pModule) return 0;

//    TRACE("module=%04x name=%s type=%s\n", pModule->self, debugstr_a(name), debugstr_a(type) );

    if (!pModule->ne_rsrctab) return 0;

    type = get_res_name( type );
    name = get_res_name( name );

    if (HIWORD(type) || HIWORD(name))
    {
        DWORD id = NE_FindNameTableId(hModule, pModule, type, name );
        if (id)  /* found */
        {
            type = (LPCSTR)(ULONG_PTR)LOWORD(id);
            name = (LPCSTR)(ULONG_PTR)HIWORD(id);
        }
    }
    pResTab = (LPBYTE)pModule + pModule->ne_rsrctab;
    pTypeInfo = (NE_TYPEINFO *)( pResTab + 2 );

    for (;;)
    {
        if (!(pTypeInfo = NE_FindTypeSection( pResTab, pTypeInfo, type ))) break;
        if ((pNameInfo = NE_FindResourceFromType( pResTab, pTypeInfo, name )))
        {
//            TRACE("    Found id %p\n", name );
            return (HRSRC)( (char *)pNameInfo - (char *)pModule );
        }
        pTypeInfo = next_typeinfo(pTypeInfo);
    }
    return 0;
}

/**********************************************************************
 *	    GetExpWinVer    (KERNEL.167)
 */
WORD WINAPI GetExpWinVer( HMODULE hModule )
{
    NE_MODULE *pModule = NE_GetPtr( hModule );
    if ( !pModule ) return 0;
    return pModule->ne_expver;
}

/**********************************************************************
 *      DirectResAlloc    (KERNEL.168)
 *
 * Check Schulman, p. 232 for details
 */
HGLOBAL WINAPI DirectResAlloc( HINSTANCE hInstance, WORD wType,
                                 UINT wSize )
{
    HGLOBAL ret;
//    TRACE("(%04x,%04x,%04x)\n", hInstance, wType, wSize );
    if (!(hInstance = GetExePtr( hInstance ))) return 0;
    if(wType != 0x10)	/* 0x10 is the only observed value, passed from
                           CreateCursorIndirect. */
{
//        TRACE("(wType=%x)\n", wType);
}
    ret = GlobalAlloc( GMEM_MOVEABLE, wSize );
    if (ret) FarSetOwner( ret, hInstance );
    return ret;
}
