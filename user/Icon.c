#include <string.h>

#include <user.h>

#include "list.h"

VOID WINAPI FarSetOwner(HANDLE hMem, WORD wOwnerPDB);
WORD WINAPI GetExePtr(HANDLE h);

/**********************************************************************
 * Management of the 16-bit cursors and icons
 */

struct cache_entry
{
    struct list entry;
    HINSTANCE   inst;
    HRSRC       rsrc;
    HRSRC       group;
    HICON       icon;
    int         count;
};

/* Cursors / Icons */

typedef struct tagCURSORICONINFO
{
    POINT   ptHotSpot;
    WORD    nWidth;
    WORD    nHeight;
    WORD    nWidthBytes;
    BYTE    bPlanes;
    BYTE    bBitsPerPixel;
} CURSORICONINFO;

static const WORD ICON_HOTSPOT = 0x4242;

static struct list icon_cache = LIST_INIT( icon_cache );

static HICON alloc_icon_handle( unsigned int size )
{
    HGLOBAL handle = GlobalAlloc( GMEM_MOVEABLE, size + sizeof(DWORD) );
    char far *ptr = GlobalLock( handle );
	FUNCTION_START
    _fmemset( ptr + size, 0, sizeof(DWORD) );
    GlobalUnlock( handle );
    FarSetOwner( handle, 0 );
    return handle;
}

static int free_icon_handle( HICON handle )
{
	FUNCTION_START
    return GlobalFree( handle );
}

static int get_bitmap_width_bytes( int width, int bpp )
{
	FUNCTION_START
    switch(bpp)
    {
    case 1:
        return 2 * ((width+15) / 16);
    case 4:
        return 2 * ((width+3) / 4);
    case 24:
        width *= 3;
        /* fall through */
    case 8:
        return width + (width & 1);
    case 16:
    case 15:
        return width * 2;
    case 32:
        return width * 4;
    default: {}
//        WARN("Unknown depth %d, please report.\n", bpp );
    }
    return -1;
}

static CURSORICONINFO far *get_icon_ptr( HICON handle )
{
	FUNCTION_START
    return (CURSORICONINFO far *)GlobalLock( handle );
}

static void release_icon_ptr( HICON handle, CURSORICONINFO far *ptr )
{
	FUNCTION_START
    GlobalUnlock( handle );
}

static int release_shared_icon( HICON icon )
{
    struct cache_entry *cache;

	FUNCTION_START

    LIST_FOR_EACH_ENTRY( cache, &icon_cache, struct cache_entry, entry )
    {
        if (cache->icon != icon) continue;
        if (!cache->count) return 0;
        return --cache->count;
    }
    return -1;
}

/***********************************************************************
 *		IconSize (USER.86)
 *
 * See "Undocumented Windows". Used by W2.0 paint.exe.
 */
DWORD WINAPI IconSize(void)
{
	FUNCTION_START
  return MAKELONG(GetSystemMetrics(SM_CYICON), GetSystemMetrics(SM_CXICON));
}



/***********************************************************************
 *		CreateCursorIconIndirect (USER.408)
 */
HGLOBAL WINAPI CreateCursorIconIndirect( HINSTANCE hInstance,
                                           CURSORICONINFO far *info,
                                           VOID const far * lpANDbits,
                                           VOID const far * lpXORbits )
{
    HICON handle;
    CURSORICONINFO far *ptr;
    int sizeAnd, sizeXor;

	FUNCTION_START

    hInstance = GetExePtr( hInstance );  /* Make it a module handle */
    if (!lpXORbits || !lpANDbits || info->bPlanes != 1) return 0;
    info->nWidthBytes = get_bitmap_width_bytes(info->nWidth,info->bBitsPerPixel);
    sizeXor = info->nHeight * info->nWidthBytes;
    sizeAnd = info->nHeight * get_bitmap_width_bytes( info->nWidth, 1 );
    if (!(handle = alloc_icon_handle( sizeof(CURSORICONINFO) + sizeXor + sizeAnd )))
        return 0;
    FarSetOwner( handle, hInstance );
    ptr = get_icon_ptr( handle );
    _fmemcpy( ptr, info, sizeof(*info) );
    _fmemcpy( ptr + 1, lpANDbits, sizeAnd );
    _fmemcpy( (char far *)(ptr + 1) + sizeAnd, lpXORbits, sizeXor );
    release_icon_ptr( handle, ptr );
    return handle;
}

/***********************************************************************
 *		CreateCursor (USER.406)
 */
HCURSOR WINAPI CreateCursor(HINSTANCE hInstance,
				int xHotSpot, int yHotSpot,
				int nWidth, int nHeight,
				VOID const far * lpANDbits, VOID const far * lpXORbits)
{
  CURSORICONINFO info;

	FUNCTION_START

  info.ptHotSpot.x = xHotSpot;
  info.ptHotSpot.y = yHotSpot;
  info.nWidth = nWidth;
  info.nHeight = nHeight;
  info.nWidthBytes = 0;
  info.bPlanes = 1;
  info.bBitsPerPixel = 1;

  return CreateCursorIconIndirect(hInstance, &info, lpANDbits, lpXORbits);
}


/***********************************************************************
 *		CreateIcon (USER.407)
 */
HICON WINAPI CreateIcon( HINSTANCE hInstance, int nWidth,
                             int nHeight, BYTE bPlanes, BYTE bBitsPixel,
                             VOID const far * lpANDbits, VOID const far * lpXORbits )
{
    CURSORICONINFO info;

	FUNCTION_START

    info.ptHotSpot.x = ICON_HOTSPOT;
    info.ptHotSpot.y = ICON_HOTSPOT;
    info.nWidth = nWidth;
    info.nHeight = nHeight;
    info.nWidthBytes = 0;
    info.bPlanes = bPlanes;
    info.bBitsPerPixel = bBitsPixel;

    return CreateCursorIconIndirect( hInstance, &info, lpANDbits, lpXORbits );
}

/***********************************************************************
 *		DestroyIcon (USER.457)
 */
BOOL WINAPI DestroyIcon(HICON hIcon)
{
    int count;

	FUNCTION_START

//    TRACE("%04x\n", hIcon );

    count = release_shared_icon( hIcon );
    if (count != -1) return !count;
    /* assume non-shared */
    free_icon_handle( hIcon );
    return TRUE;
}

/***********************************************************************
 *		DestroyCursor (USER.458)
 */
BOOL WINAPI DestroyCursor(HCURSOR hCursor)
{
	FUNCTION_START
    return DestroyIcon( hCursor );
}

/***********************************************************************
 *		CopyIcon (USER.368)
 */
HICON WINAPI CopyIcon( HINSTANCE hInstance, HICON hIcon )
{
    CURSORICONINFO far *info = get_icon_ptr( hIcon );
    void far *and_bits = info + 1;
    void far *xor_bits = (BYTE far *)and_bits + info->nHeight * get_bitmap_width_bytes( info->nWidth, 1 );
    HGLOBAL ret = CreateCursorIconIndirect( hInstance, info, and_bits, xor_bits );
//    HGLOBAL ret = CreateIcon(hInstance, info->nWidth, info->nHeight, info->bPlanes, info->bBitsPerPixel, and_bits, xor_bits);

	FUNCTION_START

    release_icon_ptr( hIcon, info );
    return ret;
}

/***********************************************************************
 *		CopyCursor (USER.369)
 */
HCURSOR WINAPI CopyCursor( HINSTANCE hInstance, HCURSOR hCursor )
{
    CURSORICONINFO far *info = get_icon_ptr( hCursor );
    void far *and_bits = info + 1;
    void far *xor_bits = (BYTE far *)and_bits + info->nHeight * get_bitmap_width_bytes( info->nWidth, 1 );
    HGLOBAL ret = CreateCursorIconIndirect( hInstance, info, and_bits, xor_bits );

	FUNCTION_START

//    HGLOBAL ret = CreateCursor(hInstance, info->ptHotSpot.x, info->ptHotSpot.y, info->nWidth, info->nHeight, and_bits, xor_bits);
    release_icon_ptr( hCursor, info );
    return ret;
}

BOOL  WINAPI
DrawIcon(HDC hDC, int x, int y, HICON hIcon)
{
//    LPTWIN_ICONINFO lpIconInfo;
    CURSORICONINFO far *info = get_icon_ptr( hIcon );
    HDC hCompatDC;
    int cxIcon,cyIcon;
    HBITMAP hOldBitmap, hANDMask, hXORImage;
    COLORREF bg,fg;
    BOOL bRet;
    void far *and_bits = info + 1;
    void far *xor_bits = (BYTE far *)and_bits + info->nHeight * get_bitmap_width_bytes( info->nWidth, 1 );

	FUNCTION_START

//    APISTR((LF_API,"DrawIcon: hDC=%x %d,%d hIcon %x\n",hDC,x,y,hIcon));

    if (!hIcon || !(info))
	return (DWORD)FALSE;

//    if (!lpIconInfo->hXORImage || !lpIconInfo->hANDMask)
//	return (DWORD)FALSE;

    if (!(hCompatDC = CreateCompatibleDC(hDC)))
	return (DWORD)FALSE;

    cxIcon = GetSystemMetrics(SM_CXICON);
    cyIcon = GetSystemMetrics(SM_CYICON);

    bg = SetBkColor(hDC,RGB(255,255,255));
    fg = SetTextColor(hDC,RGB(0,0,0));

    hANDMask = CreateBitmap(info->nWidth, info->nHeight, 1, 1, and_bits);
    hOldBitmap = SelectObject(hCompatDC, hANDMask);
    bRet = BitBlt(hDC,x,y,cxIcon,cyIcon,hCompatDC,0,0,SRCAND);
    DeleteObject(hANDMask);
    if (bRet) {
        hXORImage = CreateBitmap(info->nWidth, info->nHeight, info->bPlanes, info->bBitsPerPixel, xor_bits);
	SelectObject(hCompatDC,hXORImage);
	bRet = BitBlt(hDC,x,y,cxIcon,cyIcon,hCompatDC,0,0,SRCINVERT);
        DeleteObject(hXORImage);
    }

    SelectObject(hCompatDC,hOldBitmap);
    DeleteDC(hCompatDC);

    SetBkColor(hDC,bg);
    SetTextColor(hDC,fg);

    return bRet;
}
