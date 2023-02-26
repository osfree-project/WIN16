#include <windows.h>

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

/***********************************************************************
 *		IconSize (USER.86)
 *
 * See "Undocumented Windows". Used by W2.0 paint.exe.
 */
DWORD WINAPI IconSize(void)
{
  return MAKELONG(GetSystemMetrics(SM_CYICON), GetSystemMetrics(SM_CXICON));
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
