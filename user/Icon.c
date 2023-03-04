#include <string.h>
#include <windows.h>

VOID WINAPI FarSetOwner(HANDLE hMem, WORD wOwnerPDB);
WORD WINAPI GetExePtr(HANDLE h);

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

static HICON alloc_icon_handle( unsigned int size )
{
    HGLOBAL handle = GlobalAlloc( GMEM_MOVEABLE, size + sizeof(DWORD) );
    char far *ptr = GlobalLock( handle );
    _fmemset( ptr + size, 0, sizeof(DWORD) );
    GlobalUnlock( handle );
    FarSetOwner( handle, 0 );
    return handle;
}

static int get_bitmap_width_bytes( int width, int bpp )
{
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
    return (CURSORICONINFO far *)GlobalLock( handle );
}

static void release_icon_ptr( HICON handle, CURSORICONINFO far *ptr )
{
    GlobalUnlock( handle );
}

/***********************************************************************
 *		IconSize (USER.86)
 *
 * See "Undocumented Windows". Used by W2.0 paint.exe.
 */
DWORD WINAPI IconSize(void)
{
  return MAKELONG(GetSystemMetrics(SM_CYICON), GetSystemMetrics(SM_CXICON));
}



/***********************************************************************
 *		CopyIcon (USER.368)
 */
HICON WINAPI CopyIcon( HINSTANCE hInstance, HICON hIcon )
{
    CURSORICONINFO far *info = get_icon_ptr( hIcon );
    void far *and_bits = info + 1;
    void far *xor_bits = (BYTE far *)and_bits + info->nHeight * get_bitmap_width_bytes( info->nWidth, 1 );
//    HGLOBAL ret = CreateCursorIconIndirect( hInstance, info, and_bits, xor_bits );
    HGLOBAL ret = CreateIcon(hInstance, info->nWidth, info->nHeight, info->bPlanes, info->bBitsPerPixel, and_bits, xor_bits);

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
//    HGLOBAL16 ret = CreateCursorIconIndirect16( hInstance, info, and_bits, xor_bits );
    HGLOBAL ret = CreateCursor(hInstance, info->ptHotSpot.x, info->ptHotSpot.y, info->nWidth, info->nHeight, and_bits, xor_bits);
    release_icon_ptr( hCursor, info );
    return ret;
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

    info.ptHotSpot.x = ICON_HOTSPOT;
    info.ptHotSpot.y = ICON_HOTSPOT;
    info.nWidth = nWidth;
    info.nHeight = nHeight;
    info.nWidthBytes = 0;
    info.bPlanes = bPlanes;
    info.bBitsPerPixel = bBitsPixel;

    return CreateCursorIconIndirect( hInstance, &info, lpANDbits, lpXORbits );
}
