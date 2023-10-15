#include "Shell.h"

/*************************************************************************
 * ExtractIcon	[SHELL.34]
 *
 */
HICON WINAPI
ExtractIcon(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex)
{
//    APISTR((LF_APISTUB,"ExtractIcon(HINSTANCE=%x,LPCSTR=%s,UINT=%x0\n",
//	hInst,lpszExeFileName?lpszExeFileName:"NULL",nIconIndex));
	MessageBox(0, "ExtractIcon", "ExtractIcon", MB_OK);
    return (HICON)0;
}

/*************************************************************************
 *				ExtractAssociatedIcon	[SHELL.36]
 *
 * Return icon for given file (either from file itself or from associated
 * executable) and patch parameters if needed.
 */
HICON WINAPI ExtractAssociatedIcon(HINSTANCE hInst, LPSTR lpIconPath, LPWORD lpiIcon)
{
	MessageBox(0, "ExtractAssociatedIcon", "ExtractAssociatedIcon", MB_OK);
    return 0;
}


static HDC display_dc;

HDC get_display_dc(void)
{
//    EnterCriticalSection( &display_dc_section );
    if (!display_dc)
    {
        HDC dc;

//        LeaveCriticalSection( &display_dc_section );
        dc = CreateDC( "DISPLAY", NULL, NULL, NULL );
//        EnterCriticalSection( &display_dc_section );
        if (display_dc)
            DeleteDC(dc);
        else
            display_dc = dc;
    }
    return display_dc;
}

void release_display_dc( HDC hdc )
{
    //LeaveCriticalSection( &display_dc_section );
}

/***********************************************************************
 *           DIB_GetBitmapInfo
 *
 * Get the info from a bitmap header.
 * Return 1 for INFOHEADER, 0 for COREHEADER, -1 in case of failure.
 */
static int DIB_GetBitmapInfo( const BITMAPINFOHEADER *header, LONG *width,
                              LONG *height, WORD *bpp, DWORD *compr )
{
    if (header->biSize == sizeof(BITMAPCOREHEADER))
    {
        const BITMAPCOREHEADER *core = (const BITMAPCOREHEADER *)header;
        *width  = core->bcWidth;
        *height = core->bcHeight;
        *bpp    = core->bcBitCount;
        *compr  = 0;
        return 0;
    }
    else if (header->biSize == sizeof(BITMAPINFOHEADER) /*||
             header->biSize == sizeof(BITMAPV4HEADER) ||
             header->biSize == sizeof(BITMAPV5HEADER) */)
    {
        *width  = header->biWidth;
        *height = header->biHeight;
        *bpp    = header->biBitCount;
        *compr  = header->biCompression;
        return 1;
    }
//    WARN("unknown/wrong size (%lu) for header\n", header->biSize);
    return -1;
}

/***********************************************************************
 *          bmi_has_alpha
 */
static BOOL bmi_has_alpha( const BITMAPINFO *info, const void *bits )
{
    int i;
    BOOL has_alpha = FALSE;
    const unsigned char *ptr = bits;

    if (info->bmiHeader.biBitCount != 32) return FALSE;
    for (i = 0; i < info->bmiHeader.biWidth * abs(info->bmiHeader.biHeight); i++, ptr += 4)
        if ((has_alpha = (ptr[3] != 0))) break;
    return has_alpha;
}

/***********************************************************************
 *          create_alpha_bitmap
 *
 * Create the alpha bitmap for a 32-bpp icon that has an alpha channel.
 */
static HBITMAP create_alpha_bitmap( HBITMAP color, BITMAPINFO FAR *src_info, const void *color_bits )
{
    HBITMAP alpha = 0;
    BITMAPINFO FAR *info = NULL;
    BITMAP bm;
    HDC hdc;
    void *bits;
    unsigned char *ptr;
    int i;

    if (!GetObject( color, sizeof(bm), &bm )) return 0;
    if (bm.bmBitsPixel != 32) return 0;

    if (!(hdc = CreateCompatibleDC( 0 ))) return 0;
    if (!(info = (BITMAPINFO FAR *)GlobalAllocPtr(GPTR, FIELD_OFFSET( BITMAPINFO, bmiColors[256] )))) goto done;
    info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info->bmiHeader.biWidth = bm.bmWidth;
    info->bmiHeader.biHeight = -bm.bmHeight;
    info->bmiHeader.biPlanes = 1;
    info->bmiHeader.biBitCount = 32;
    info->bmiHeader.biCompression = BI_RGB;
    info->bmiHeader.biSizeImage = bm.bmWidth * bm.bmHeight * 4;
    info->bmiHeader.biXPelsPerMeter = 0;
    info->bmiHeader.biYPelsPerMeter = 0;
    info->bmiHeader.biClrUsed = 0;
    info->bmiHeader.biClrImportant = 0;
    if (!(alpha = CreateDIBSection( hdc, info, DIB_RGB_COLORS, &bits, NULL, 0 ))) goto done;

    if (src_info)
    {
        SelectObject( hdc, alpha );
        StretchDIBits( hdc, 0, 0, bm.bmWidth, bm.bmHeight,
                       0, 0, src_info->bmiHeader.biWidth, src_info->bmiHeader.biHeight,
                       color_bits, src_info, DIB_RGB_COLORS, SRCCOPY );

    }
    else
    {
        GetDIBits( hdc, color, 0, bm.bmHeight, bits, info, DIB_RGB_COLORS );
        if (!bmi_has_alpha( info, bits ))
        {
            DeleteObject( alpha );
            alpha = 0;
            goto done;
        }
    }

    /* pre-multiply by alpha */
    for (i = 0, ptr = bits; i < bm.bmWidth * bm.bmHeight; i++, ptr += 4)
    {
        unsigned int alpha = ptr[3];
        ptr[0] = (ptr[0] * alpha + 127) / 255;
        ptr[1] = (ptr[1] * alpha + 127) / 255;
        ptr[2] = (ptr[2] * alpha + 127) / 255;
    }

done:
    DeleteDC( hdc );
    GlobalFreePtr( info );
    return alpha;
}

/***********************************************************************
 *          is_dib_monochrome
 *
 * Returns whether a DIB can be converted to a monochrome DDB.
 *
 * A DIB can be converted if its color table contains only black and
 * white. Black must be the first color in the color table.
 *
 * Note : If the first color in the color table is white followed by
 *        black, we can't convert it to a monochrome DDB with
 *        SetDIBits, because black and white would be inverted.
 */
static BOOL is_dib_monochrome( const BITMAPINFO* info )
{
    if (info->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
    {
        const RGBTRIPLE *rgb = ((const BITMAPCOREINFO*)info)->bmciColors;

        if (((const BITMAPCOREINFO*)info)->bmciHeader.bcBitCount != 1) return FALSE;

        /* Check if the first color is black */
        if ((rgb->rgbtRed == 0) && (rgb->rgbtGreen == 0) && (rgb->rgbtBlue == 0))
        {
            rgb++;

            /* Check if the second color is white */
            return ((rgb->rgbtRed == 0xff) && (rgb->rgbtGreen == 0xff)
                 && (rgb->rgbtBlue == 0xff));
        }
        else return FALSE;
    }
    else  /* assume BITMAPINFOHEADER */
    {
        const RGBQUAD *rgb = info->bmiColors;

        if (info->bmiHeader.biBitCount != 1) return FALSE;

        /* Check if the first color is black */
        if ((rgb->rgbRed == 0) && (rgb->rgbGreen == 0) &&
            (rgb->rgbBlue == 0) && (rgb->rgbReserved == 0))
        {
            rgb++;

            /* Check if the second color is white */
            return ((rgb->rgbRed == 0xff) && (rgb->rgbGreen == 0xff)
                 && (rgb->rgbBlue == 0xff) && (rgb->rgbReserved == 0));
        }
        else return FALSE;
    }
}


static HBITMAP create_color_bitmap( int width, int height )
{
    HDC hdc = get_display_dc();
    HBITMAP ret = CreateCompatibleBitmap( hdc, width, height );
    release_display_dc( hdc );
    return ret;
}

static void free_icon_frame( struct cursoricon_frame *frame )
{
    if (frame->color) DeleteObject( frame->color );
    if (frame->alpha) DeleteObject( frame->alpha );
    if (frame->mask)  DeleteObject( frame->mask );
}

/***********************************************************************
 *          get_dib_image_size
 *
 * Return the size of a DIB bitmap in bytes.
 */
static int get_dib_image_size( int width, int height, int depth )
{
    return (((width * depth + 31) / 8) & ~3) * abs( height );
}

/***********************************************************************
 *           bitmap_info_size
 *
 * Return the size of the bitmap info structure including color table.
 */
int bitmap_info_size( const BITMAPINFO * info, WORD coloruse )
{
    unsigned int colors, size, masks = 0;

    if (info->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
    {
        const BITMAPCOREHEADER *core = (const BITMAPCOREHEADER *)info;
        colors = (core->bcBitCount <= 8) ? 1 << core->bcBitCount : 0;
        return sizeof(BITMAPCOREHEADER) + colors *
             ((coloruse == DIB_RGB_COLORS) ? sizeof(RGBTRIPLE) : sizeof(WORD));
    }
    else  /* assume BITMAPINFOHEADER */
    {
        colors = info->bmiHeader.biClrUsed;
        if (colors > 256) /* buffer overflow otherwise */
                colors = 256;
        if (!colors && (info->bmiHeader.biBitCount <= 8))
            colors = 1 << info->bmiHeader.biBitCount;
        if (info->bmiHeader.biCompression == BI_BITFIELDS) masks = 3;
        size = max( info->bmiHeader.biSize, sizeof(BITMAPINFOHEADER) + masks * sizeof(DWORD) );
        return size + colors * ((coloruse == DIB_RGB_COLORS) ? sizeof(RGBQUAD) : sizeof(WORD));
    }
}

/*************************************************************************
 *                      ICO_GetIconDirectory
 *
 * Reads .ico file and build phony ICONDIR struct
 */
static BYTE FAR * ICO_GetIconDirectory( LPBYTE peimage, LPicoICONDIR FAR * lplpiID, DWORD FAR *uSize )
{
	CURSORICONFILEDIR FAR *lpcid;	/* icon resource in resource-dir format */
	CURSORICONDIR FAR * lpID;		/* icon resource in resource format */
	int		i;

	//TRACE("%p %p\n", peimage, lplpiID);

	lpcid = (CURSORICONFILEDIR FAR *)peimage;

	if( lpcid->idReserved || (lpcid->idType != 1) || (!lpcid->idCount) )
	  return 0;

	/* allocate the phony ICONDIR structure */
        *uSize = FIELD_OFFSET(CURSORICONDIR, idEntries[lpcid->idCount]);
	if( (lpID = (CURSORICONDIR FAR *)GlobalAllocPtr(GPTR, *uSize) ))
	{
	  /* copy the header */
	  lpID->idReserved = lpcid->idReserved;
	  lpID->idType = lpcid->idType;
	  lpID->idCount = lpcid->idCount;

	  /* copy the entries */
	  for( i=0; i < lpcid->idCount; i++ )
	  {
            lmemcpy(&lpID->idEntries[i], &lpcid->idEntries[i], sizeof(CURSORICONDIRENTRY) - 2);
	    lpID->idEntries[i].wResId = i;
	  }

	  *lplpiID = (LPicoICONDIR)peimage;
	  return (BYTE *)lpID;
	}
	return 0;
}

/*************************************************************************
 *                      ICO_LoadIcon
 */
static BYTE FAR * ICO_LoadIcon( LPBYTE peimage, LPicoICONDIRENTRY lpiIDE, DWORD FAR * uSize)
{
//	TRACE("%p %p\n", peimage, lpiIDE);

	*uSize = lpiIDE->dwBytesInRes;
	return peimage + lpiIDE->dwImageOffset;
}

/*************************************************************************
 *			USER32_LoadResource
 */
static BYTE FAR * USER32_LoadResource( LPBYTE peimage, NE_NAMEINFO FAR * pNInfo, WORD sizeShift, DWORD FAR *uSize)
{
//	TRACE("%p %p 0x%08x\n", peimage, pNInfo, sizeShift);

	*uSize = (DWORD)pNInfo->length << sizeShift;
	return peimage + ((DWORD)pNInfo->offset << sizeShift);
}

/*************************************************************************
 *				USER32_GetResourceTable
 */
static DWORD USER32_GetResourceTable(LPBYTE peimage,DWORD pesize,LPBYTE FAR *retptr)
{
	LPIMAGE_DOS_HEADER	mz_header;

//	TRACE("%p %p\n", peimage, retptr);

	*retptr = NULL;

	mz_header = (LPIMAGE_DOS_HEADER) peimage;

	if (mz_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
	  if (mz_header->e_cblp == 1)	/* .ICO file ? */
	  {
	    *retptr = (LPBYTE)-1;	/* ICONHEADER.idType, must be 1 */
	    return 1;
	  }
	  else
	    return 0; /* failed */
	}
	if (mz_header->e_lfanew >= pesize) {
	    return 0; /* failed, happens with PKZIP DOS Exes for instance. */
	}
	//if (*((DWORD*)(peimage + mz_header->e_lfanew)) == IMAGE_NT_SIGNATURE )
//	  return IMAGE_NT_SIGNATURE;

	if (*((WORD FAR *)(peimage + mz_header->e_lfanew)) == IMAGE_OS2_SIGNATURE )
	{
	  LPIMAGE_OS2_HEADER	ne_header;

	  ne_header = (LPIMAGE_OS2_HEADER)(peimage + mz_header->e_lfanew);

	  if (ne_header->ne_magic != IMAGE_OS2_SIGNATURE)
	    return 0;

	  if( (ne_header->ne_restab - ne_header->ne_rsrctab) <= sizeof(NE_TYPEINFO) )
	    *retptr = (LPBYTE)-1;
	  else
	    *retptr = peimage + mz_header->e_lfanew + ne_header->ne_rsrctab;

	  return IMAGE_OS2_SIGNATURE;
	}
	return 0; /* failed */
}

static BOOL create_icon_frame( const BITMAPINFO FAR *bmi, DWORD maxsize, POINT hotspot, BOOL is_icon,
                               int width, int height, UINT flags, struct cursoricon_frame FAR * frame )
{
    DWORD size, color_size, mask_size, compr;
    const void FAR *color_bits, *mask_bits;
    void FAR *alpha_mask_bits = NULL;
    LONG bmi_width, bmi_height;
    BITMAPINFO FAR *bmi_copy;
    BOOL do_stretch;
    HDC hdc = 0;
    WORD bpp;
    BOOL ret = FALSE;

    memset( frame, 0, sizeof(*frame) );

    /* Check bitmap header */

//    if (bmi->bmiHeader.biSize == PNG_SIGN)
//    {
//        BITMAPINFO *bmi_png;

//        if (!(bmi_png = load_png( (const char *)bmi, &maxsize ))) return FALSE;
//        ret = create_icon_frame( bmi_png, maxsize, hotspot, is_icon, width, height, flags, frame );
//        HeapFree( GetProcessHeap(), 0, bmi_png );
//        return ret;
//    }

    if (maxsize < sizeof(BITMAPCOREHEADER))
    {
//        WARN( "invalid size %lu\n", maxsize );
        return FALSE;
    }
    if (maxsize < bmi->bmiHeader.biSize)
    {
//        WARN( "invalid header size %lu\n", bmi->bmiHeader.biSize );
        return FALSE;
    }
    if ( (bmi->bmiHeader.biSize != sizeof(BITMAPCOREHEADER)) &&
         (bmi->bmiHeader.biSize != sizeof(BITMAPINFOHEADER)  ||
         (bmi->bmiHeader.biCompression != BI_RGB &&
          bmi->bmiHeader.biCompression != BI_BITFIELDS)) )
    {
//        WARN( "invalid bitmap header %lu\n", bmi->bmiHeader.biSize );
        return FALSE;
    }

    size = bitmap_info_size( bmi, DIB_RGB_COLORS );
    DIB_GetBitmapInfo(&bmi->bmiHeader, &bmi_width, &bmi_height, &bpp, &compr);
    color_size = get_dib_image_size( bmi_width, bmi_height / 2,
                                     bpp );
    mask_size = get_dib_image_size( bmi_width, bmi_height / 2, 1 );
    if (size > maxsize || color_size > maxsize - size)
    {
//        WARN( "truncated file %lu < %lu+%lu+%lu\n", maxsize, size, color_size, mask_size );
        return 0;
    }
    if (mask_size > maxsize - size - color_size) mask_size = 0;  /* no mask */

    if (flags & LR_DEFAULTSIZE)
    {
        if (!width) width = GetSystemMetrics( is_icon ? SM_CXICON : SM_CXCURSOR );
        if (!height) height = GetSystemMetrics( is_icon ? SM_CYICON : SM_CYCURSOR );
    }
    else
    {
        if (!width) width = bmi_width;
        if (!height) height = bmi_height/2;
    }
    do_stretch = (bmi_height/2 != height) || (bmi_width != width);

    /* Scale the hotspot */
    if (is_icon)
    {
        hotspot.x = width / 2;
        hotspot.y = height / 2;
    }
    else if (do_stretch)
    {
        hotspot.x = (hotspot.x * width) / bmi_width;
        hotspot.y = (hotspot.y * height) / (bmi_height / 2);
    }

    if (!(bmi_copy = (BITMAPINFO FAR *)GlobalAllocPtr( GPTR, max( size, FIELD_OFFSET( BITMAPINFO, bmiColors[2] )))))
        return 0;
    if (!(hdc = CreateCompatibleDC( 0 ))) goto done;

    memcpy( bmi_copy, bmi, size );
    if (bmi_copy->bmiHeader.biSize != sizeof(BITMAPCOREHEADER))
        bmi_copy->bmiHeader.biHeight /= 2;
    else
        ((BITMAPCOREINFO *)bmi_copy)->bmciHeader.bcHeight /= 2;
    bmi_height /= 2;

    color_bits = (const char*)bmi + size;
    mask_bits = (const char*)color_bits + color_size;

    if (is_dib_monochrome( bmi ))
    {
        if (!(frame->mask = CreateBitmap( width, height * 2, 1, 1, NULL ))) goto done;

        /* copy color data into second half of mask bitmap */
        SelectObject( hdc, frame->mask );
        StretchDIBits( hdc, 0, height, width, height,
                       0, 0, bmi_width, bmi_height,
                       color_bits, bmi_copy, DIB_RGB_COLORS, SRCCOPY );
    }
    else
    {
        if (!(frame->mask = CreateBitmap( width, height, 1, 1, NULL ))) goto done;
        if (!(frame->color = create_color_bitmap( width, height ))) goto done;
        SelectObject( hdc, frame->color );
        StretchDIBits( hdc, 0, 0, width, height,
                       0, 0, bmi_width, bmi_height,
                       color_bits, bmi_copy, DIB_RGB_COLORS, SRCCOPY );

#if 0
        if (bmi_has_alpha( bmi_copy, color_bits ))
        {
            frame->alpha = create_alpha_bitmap( frame->color, bmi_copy, color_bits );
            if (!mask_size)  /* generate mask from alpha */
            {
                LONG x, y, dst_stride = ((bmi_width + 31) / 8) & ~3;

                if ((alpha_mask_bits = heap_calloc( bmi_height, dst_stride )))
                {
                    static const unsigned char masks[] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 };
                    const DWORD *src = color_bits;
                    unsigned char *dst = alpha_mask_bits;

                    for (y = 0; y < bmi_height; y++, src += bmi_width, dst += dst_stride)
                        for (x = 0; x < bmi_width; x++)
                            if (src[x] >> 24 != 0xff) dst[x >> 3] |= masks[x & 7];

                    mask_bits = alpha_mask_bits;
                    mask_size = bmi_height * dst_stride;
                }
            }
        }
#endif
        /* convert info to monochrome to copy the mask */
        if (bmi_copy->bmiHeader.biSize != sizeof(BITMAPCOREHEADER))
        {
            RGBQUAD *rgb = bmi_copy->bmiColors;

            bmi_copy->bmiHeader.biBitCount = 1;
            bmi_copy->bmiHeader.biClrUsed = bmi_copy->bmiHeader.biClrImportant = 2;
            rgb[0].rgbBlue = rgb[0].rgbGreen = rgb[0].rgbRed = 0x00;
            rgb[1].rgbBlue = rgb[1].rgbGreen = rgb[1].rgbRed = 0xff;
            rgb[0].rgbReserved = rgb[1].rgbReserved = 0;
        }
        else
        {
            RGBTRIPLE *rgb = (RGBTRIPLE *)(((BITMAPCOREHEADER *)bmi_copy) + 1);

            ((BITMAPCOREINFO *)bmi_copy)->bmciHeader.bcBitCount = 1;
            rgb[0].rgbtBlue = rgb[0].rgbtGreen = rgb[0].rgbtRed = 0x00;
            rgb[1].rgbtBlue = rgb[1].rgbtGreen = rgb[1].rgbtRed = 0xff;
        }
    }

    if (mask_size)
    {
        SelectObject( hdc, frame->mask );
        StretchDIBits( hdc, 0, 0, width, height,
                       0, 0, bmi_width, bmi_height,
                       mask_bits, bmi_copy, DIB_RGB_COLORS, SRCCOPY );
    }

    frame->width   = width;
    frame->height  = height;
    frame->hotspot = hotspot;
    ret = TRUE;

done:
    if (!ret) free_icon_frame( frame );
    DeleteDC( hdc );
    GlobalFreePtr( bmi_copy );
    GlobalFreePtr( alpha_mask_bits );
    return ret;
}

static HICON create_cursoricon_object( struct cursoricon_desc *desc, BOOL is_icon, HINSTANCE module,
                                       const char *resname, HRSRC rsrc )
{
    char buf[MAX_PATH];
//    UNICODE_STRING module_name = { 0, sizeof(buf), buf };
//    UNICODE_STRING res_str = { 0 };
    HICON handle;
#if 0
    if (!(handle = NtUserCreateCursorIcon( is_icon ))) return 0;

    if (module) LdrGetDllFullName( module, &module_name );

    res_str.Buffer = (char *)resname;
    if (!IS_INTRESOURCE(resname))
    {
        res_str.Length = lstrlen( resname ) * sizeof(WCHAR);
        res_str.MaximumLength = res_str.Length + sizeof(WCHAR);
    }
    desc->rsrc = rsrc; /* FIXME: we should probably avoid storing rsrc */

    if (!NtUserSetCursorIconData( handle, &module_name, &res_str, desc ))
    {
        NtUserDestroyCursor( handle, 0 );
        return 0;
    }
#endif
    return handle;
}

/***********************************************************************
 *          create_icon_from_bmi
 *
 * Create an icon from its BITMAPINFO.
 */
static HICON create_icon_from_bmi( const BITMAPINFO *bmi, DWORD maxsize, HMODULE module, LPCSTR resname,
                                   HRSRC rsrc, POINT hotspot, BOOL bIcon, int width, int height,
                                   UINT flags )
{
    struct cursoricon_frame frame;
    struct cursoricon_desc desc =
    {
        .flags = flags,
        .frames = &frame,
    };
    HICON ret;

    if (!create_icon_frame( bmi, maxsize, hotspot, bIcon, width, height, flags, &frame )) return 0;

    ret = create_cursoricon_object( &desc, bIcon, module, resname, rsrc );
    if (!ret) free_icon_frame( &frame );
    return ret;
}

/**********************************************************************
 *		CreateIconFromResourceEx (USER32.@)
 *
 * FIXME: Convert to mono when cFlag is LR_MONOCHROME.
 */
HICON WINAPI CreateIconFromResourceEx( LPBYTE bits, UINT cbSize,
                                       BOOL bIcon, DWORD dwVersion,
                                       int width, int height,
                                       UINT cFlag )
{
    POINT hotspot;
    const BITMAPINFO *bmi;

//    TRACE_(cursor)("%p (%u bytes), ver %08lx, %ix%i %s %s\n",
//                   bits, cbSize, dwVersion, width, height,
//                   bIcon ? "icon" : "cursor", (cFlag & LR_MONOCHROME) ? "mono" : "" );

    if (!bits) return 0;

    if (dwVersion == 0x00020000)
    {
//        FIXME_(cursor)("\t2.xx resources are not supported\n");
        return 0;
    }

    /* Check if the resource is an animated icon/cursor */
//    if (!memcmp(bits, "RIFF", 4))
//        return CURSORICON_CreateIconFromANI( bits, cbSize, width, height,
//                                             0 /* default depth */, bIcon, cFlag );

    if (bIcon)
    {
        hotspot.x = width / 2;
        hotspot.y = height / 2;
        bmi = (BITMAPINFO FAR *)bits;
    }
    else /* get the hotspot */
    {
        const WORD /*SHORT*/ FAR *pt = (const WORD /*SHORT*/ FAR *)bits;
        hotspot.x = pt[0];
        hotspot.y = pt[1];
        bmi = (const BITMAPINFO FAR *)(pt + 2);
        cbSize -= 2 * sizeof(*pt);
    }

    return create_icon_from_bmi( bmi, cbSize, 0, 0, 0, hotspot, bIcon, width, height, cFlag );
}

/***********************************************************************
 *           PrivateExtractIconsA			[USER32.@]
 */

UINT WINAPI PrivateExtractIconsA (
	LPCSTR lpszExeFileName,
	int nIconIndex,
	UINT cxDesired,
	UINT cyDesired,
	HICON * RetPtr, /* [out] pointer to array of nIcons HICON handles */
	UINT * pIconId,  /* [out] pointer to array of nIcons icon identifiers or NULL */
	UINT nIcons,    /* [in] number of icons to retrieve */
	UINT flags )    /* [in] LR_* flags used by LoadImage */
{
//	TRACE("%s %d %dx%d %p %p %d 0x%08x\n",
//	      debugstr_w(lpwstrFile), nIndex, sizeX, sizeY, phicon, pIconId, nIcons, flags);

//	if ((nIcons & 1) && HIWORD(sizeX) && HIWORD(sizeY))
//	{
//	  WARN("Uneven number %d of icons requested for small and large icons!\n", nIcons);
	//}
	//return ICO_ExtractIconExW(lpwstrFile, phicon, nIndex, nIcons, sizeX, sizeY, pIconId, flags);
/*************************************************************************
 *	ICO_ExtractIconExW		[internal]
 *
 * NOTES
 *  nIcons = 0: returns number of Icons in file
 *
 * returns
 *  invalid file: -1
 *  failure:0;
 *  success: number of icons in file (nIcons = 0) or nr of icons retrieved
 */
//static UINT ICO_ExtractIconExW(
//	LPCSTR lpszExeFileName,
//	HICON * RetPtr,
//	INT nIconIndex,
//	UINT nIcons,
//	UINT cxDesired,
//	UINT cyDesired,
//	UINT *pIconId,
//	UINT flags)
//{
	UINT		ret = 0;
	UINT		cx1, cx2, cy1, cy2;
	LPBYTE		pData;
	DWORD		sig;
	HANDLE		hFile;
	UINT		iconDirCount = 0,iconCount = 0;
	LPBYTE		peimage;
	HANDLE		fmapping;
	DWORD		fsizeh,fsizel;
	char		szExePath[MAX_PATH];
	DWORD		dwSearchReturn;

//	TRACE("%s, %d, %d %p 0x%08x\n", debugstr_w(lpszExeFileName), nIconIndex, nIcons, pIconId, flags);

	// @todo search path
	lstrcpy(szExePath, lpszExeFileName);
//        dwSearchReturn = SearchPath(NULL, lpszExeFileName, NULL, sizeof(szExePath), szExePath, NULL);
//        if ((dwSearchReturn == 0) || (dwSearchReturn > sizeof(szExePath)))
//        {
            //WARN("File %s not found or path too long\n", debugstr_w(lpszExeFileName));
//            return -1;
//        }

	hFile = CreateFile(szExePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) return ret;
	fsizel = GetFileSize(hFile,&fsizeh);

	/* Map the file */
	fmapping = CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_COMMIT, 0, 0, NULL);
	CloseHandle(hFile);
	if (!fmapping)
	{
          //WARN("CreateFileMapping error %ld\n", GetLastError() );
	  return 0xFFFF;
	}

	if (!(peimage = MapViewOfFile(fmapping, FILE_MAP_READ, 0, 0, 0)))
	{
          //WARN("MapViewOfFile error %ld\n", GetLastError() );
	  CloseHandle(fmapping);
	  return 0xFFFF;
	}
	CloseHandle(fmapping);

	cx1 = LOWORD(cxDesired);
	cx2 = HIWORD(cxDesired);
	cy1 = LOWORD(cyDesired);
	cy2 = HIWORD(cyDesired);

	if (pIconId) /* Invalidate first icon identifier */
		*pIconId = 0xFFFF;

	if (!pIconId) /* if no icon identifier array present use the icon handle array as intermediate storage */
	  pIconId = (UINT*)RetPtr;

	sig = USER32_GetResourceTable(peimage, fsizel, &pData);

/* ico file or NE exe/dll*/
	if (sig==IMAGE_OS2_SIGNATURE || sig==1) /* .ICO file */
	{
	  BYTE	FAR	*pCIDir = 0;
	  NE_TYPEINFO	FAR * pTInfo = (NE_TYPEINFO FAR *)(pData + 2);
	  NE_NAMEINFO	FAR *pIconStorage = NULL;
	  NE_NAMEINFO	FAR *pIconDir = NULL;
	  LPicoICONDIR	lpiID = NULL;
	  DWORD		uSize = 0;

          //TRACE("-- OS2/icon Signature (0x%08lx)\n", sig);

	  if (pData == (BYTE FAR *)-1)
	  {
	    pCIDir = ICO_GetIconDirectory(peimage, &lpiID, &uSize);	/* check for .ICO file */
	    if (pCIDir)
	    {
	      iconDirCount = 1; iconCount = lpiID->idCount;
              //TRACE("-- icon found %p 0x%08lx 0x%08x 0x%08x\n", pCIDir, uSize, iconDirCount, iconCount);
	    }
	  }
	  else while (pTInfo->type_id && !(pIconStorage && pIconDir))
	  {
	    if (pTInfo->type_id == NE_RSCTYPE_GROUP_ICON)	/* find icon directory and icon repository */
	    {
	      iconDirCount = pTInfo->count;
	      pIconDir = ((NE_NAMEINFO FAR *)(pTInfo + 1));
	      //TRACE("\tfound directory - %i icon families\n", iconDirCount);
	    }
	    if (pTInfo->type_id == NE_RSCTYPE_ICON)
	    {
	      iconCount = pTInfo->count;
	      pIconStorage = ((NE_NAMEINFO FAR *)(pTInfo + 1));
	      //TRACE("\ttotal icons - %i\n", iconCount);
	    }
	    pTInfo = (NE_TYPEINFO FAR *)((char FAR *)(pTInfo+1)+pTInfo->count*sizeof(NE_NAMEINFO));
	  }

	  if ((pIconStorage && pIconDir) || lpiID)	  /* load resources and create icons */
	  {
	    if (nIcons == 0)
	    {
	      ret = iconDirCount;
              if (lpiID)	/* *.ico file, deallocate heap pointer*/
	        GlobalFreePtr(pCIDir);
	    }
	    else if (nIconIndex < iconDirCount)
	    {
	      UINT   i, icon;
	      if (nIcons > iconDirCount - nIconIndex)
	        nIcons = iconDirCount - nIconIndex;

	      for (i = 0; i < nIcons; i++)
	      {
	        /* .ICO files have only one icon directory */
	        if (lpiID == NULL)	/* not *.ico */
	          pCIDir = USER32_LoadResource(peimage, pIconDir + i + nIconIndex, *(WORD FAR *)pData, &uSize);
	        pIconId[i] = LookupIconIdFromDirectoryEx(pCIDir, TRUE, cx1, cy1, flags);
                if (cx2 && cy2) pIconId[++i] = LookupIconIdFromDirectoryEx(pCIDir, TRUE,  cx2, cy2, flags);
	      }
              if (lpiID)	/* *.ico file, deallocate heap pointer*/
	        GlobalFreePtr(pCIDir);

	      for (icon = 0; icon < nIcons; icon++)
	      {
	        pCIDir = NULL;
	        if (lpiID)
	          pCIDir = ICO_LoadIcon(peimage, lpiID->idEntries + (int)pIconId[icon], &uSize);
	        else
	          for (i = 0; i < iconCount; i++)
	            if (pIconStorage[i].id == ((int)pIconId[icon] | 0x8000) )
	              pCIDir = USER32_LoadResource(peimage, pIconStorage + i, *(WORD FAR *)pData, &uSize);

	        if (pCIDir)
                {
	          RetPtr[icon] = CreateIconFromResourceEx(pCIDir, uSize, TRUE, 0x00030000,
                                                                 cx1, cy1, flags);
                  if (cx2 && cy2)
                      RetPtr[++icon] = CreateIconFromResourceEx(pCIDir, uSize, TRUE, 0x00030000,
                                                                       cx2, cy2, flags);
                }
	        else
	          RetPtr[icon] = 0;
	      }
	      ret = icon;	/* return number of retrieved icons */
	    }
	  }
	}
/* end ico file */

end:
	UnmapViewOfFile(peimage);	/* success */
	return ret;

}

/*************************************************************************
 *			InternalExtractIcon		[SHELL.39]
 *
 * This abortion is called directly by Progman
 */
HGLOBAL WINAPI InternalExtractIcon(HINSTANCE hInstance, LPCSTR lpszExeFileName, UINT nIconIndex, WORD n )
{
    HGLOBAL hRet = 0;
    HICON FAR * RetPtr = NULL;

//	TRACE("(%04x,file %s,start %d,extract %d\n",
//		       hInstance, lpszExeFileName, nIconIndex, n);

	MessageBox(0, "InternalExtractIcon", "InternalExtractIcon", MB_OK);	
	if (!n)
	  return 0;

	hRet = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(*RetPtr) * n);
        RetPtr = (HICON FAR *)GlobalLock(hRet);

	if (nIconIndex == (UINT)-1)  /* get number of icons */
	{
	  RetPtr[0] = PrivateExtractIconsA(lpszExeFileName, 0, 0, 0, NULL, NULL, 0, LR_DEFAULTCOLOR);
	}
	else
	{
	  UINT ret;
	  HICON FAR *icons;

	  icons = (HICON FAR *)GlobalAllocPtr(GPTR, n * sizeof(*icons));
	  ret = PrivateExtractIconsA(lpszExeFileName, nIconIndex,
	                             GetSystemMetrics(SM_CXICON),
	                             GetSystemMetrics(SM_CYICON),
	                             icons, NULL, n, LR_DEFAULTCOLOR);
	  if ((ret != 0xffff) && ret)
	  {
	    int i;
	    for (i = 0; i < n; i++) RetPtr[i] = icons[i];
	  } else
	  {
	    GlobalFree(hRet);
	    hRet = 0;
	  }
	  GlobalFreePtr(icons);
	}
	return hRet;
}
