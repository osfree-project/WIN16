/*
 * Cursor and icon support
 *
 * Copyright 1995 Alexandre Julliard
 * Copyright 1996 Martin Von Loewis
 * Copyright 1997 Alex Korobka
 * Copyright 1998 Turchanov Sergey
 * Copyright 2007 Henri Verbeet
 * Copyright 2009 Vincent Povirk for CodeWeavers
 * Copyright 2016 Dmitry Timoshkov
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

#include <user.h>

#include <string.h>


/**********************************************************************
 *     LoadString   (USER.176)
 */
int WINAPI LoadString( HINSTANCE instance, UINT resource_id, LPSTR buffer, int buflen )
{
    HGLOBAL hmem;
    HRSRC hrsrc;
    unsigned char far *p;
    int string_num;
    int ret;

	FUNCTION_START
	TRACE("inst=%x id=%x buff=%x:%x len=%d", instance, resource_id, buffer, buflen);

	hrsrc = FindResource( instance, MAKEINTRESOURCE((resource_id>>4)+1), RT_STRING );
	if (!hrsrc) 
	{
		TRACE("Resource not found");
		FUNCTION_END
		return 0;
	}

	hmem = LoadResource( instance, hrsrc );
	if (!hmem) 
	{
		TRACE("Error loading resource");
		FUNCTION_END
		return 0;
	}

	p = LockResource(hmem);
	string_num = resource_id & 0x000f;
	while (string_num--) p += *p + 1;

	if (buffer == NULL) ret = *p;
	else
	{
		if (buflen - 1 < *p)
			ret = buflen - 1;
		else
			ret = *p;
			
		if (ret > 0)
		{
			_fmemcpy(buffer, p + 1, ret);
			buffer[ret] = '\0';
		}
		else if (buflen > 1)
		{
			buffer[0] = '\0';
			ret = 0;
		}
        TRACE("%S loaded", buffer);
	}
	FreeResource( hmem );

	FUNCTION_END
	return ret;
}

/**********************************************************************
 *              LoadAccelerators  (USER.177)
 */
HACCEL WINAPI LoadAccelerators(HINSTANCE hInstance, LPCSTR lpTableName)
{
	HANDLE hResInfo;
	HACCEL rc;

/*	FUNCTION_START*/
	TRACE("LoadAccelerators(HINSTANCE=%x,LPCSTR=%x)",
		hInstance,lpTableName);

	hResInfo = FindResource(hInstance, lpTableName, RT_ACCELERATOR);

	if(hResInfo == 0) {
    		TRACE("LoadAccelerators: returns HACCEL 0");
		return 0;
	}

	rc =  LoadResource(hInstance,hResInfo);
    	TRACE("LoadAccelerators: returns HACCEL %x",rc);
	return rc;
}



int get_display_bpp(void)
{
	HDC hdc = GetDC(0);
	int ret = GetDeviceCaps( hdc, BITSPIXEL );
	ReleaseDC(0, hdc);
	return ret;
}

/*
 *  The following macro functions account for the irregularities of
 *   accessing cursor and icon resources in files and resource entries.
 */
typedef BOOL (*fnGetCIEntry)( LPVOID dir, DWORD size, int n,
                              int FAR *width, int FAR *height, int FAR *bits );

/**********************************************************************
 *	    CURSORICON_FindBestCursor
 *
 * Find the cursor closest to the requested size.
 *
 * FIXME: parameter 'color' ignored.
 */
int CURSORICON_FindBestCursor(LPVOID dir, DWORD size, fnGetCIEntry get_entry,
                                      int width, int height, int depth, UINT loadflags )
{
	int i, maxwidth, maxheight, maxbits, cx, cy, bits, bestEntry = -1;
	FUNCTION_START

    TRACE("CURSORICON_FindBestCursor: dir=%p, size=%u, width=%d, height=%d, depth=%d, loadflags=%08X",
          dir, size, width, height, depth, loadflags);

    if (loadflags & LR_DEFAULTSIZE)
    {
        if (!width) width = GetSystemMetrics( SM_CXCURSOR );
        if (!height) height = GetSystemMetrics( SM_CYCURSOR );
        TRACE("LR_DEFAULTSIZE set: width=%d, height=%d", width, height);
    }
    else if (!width && !height)
    {
        /* use the first entry */
        if (!get_entry( dir, size, 0, &width, &height, &bits )) 
        {
            TRACE("Failed to get first entry");
            return -1;
        }
        TRACE("Using first entry: %dx%d, %d bpp", width, height, bits);
        return 0;
    }

    /* First find the largest one smaller than or equal to the requested size*/

    maxwidth = 0;
    maxheight = 0;
    maxbits = 0;
    for ( i = 0; get_entry( dir, size, i, &cx, &cy, &bits ); i++ )
    {
        TRACE("Entry %d: %dx%d, %d bpp", i, cx, cy, bits);
        if (cx > width || cy > height) 
        {
            TRACE("  Too big, skipping");
            continue;
        }
        if (cx < maxwidth || cy < maxheight) 
        {
            TRACE("  Smaller than current best (%dx%d), skipping", maxwidth, maxheight);
            continue;
        }
        if (cx == maxwidth && cy == maxheight)
        {
            if (loadflags & LR_MONOCHROME)
            {
                if (maxbits && bits >= maxbits) 
                {
                    TRACE("  Same size but not better bpp (monochrome), skipping");
                    continue;
                }
            }
            else if (bits <= maxbits) 
            {
                TRACE("  Same size but not better bpp, skipping");
                continue;
            }
        }
        bestEntry = i;
        maxwidth  = cx;
        maxheight = cy;
        maxbits = bits;
        TRACE("  New best entry: %d (%dx%d, %d bpp)", bestEntry, maxwidth, maxheight, maxbits);
    }
    if (bestEntry != -1) 
    {
        TRACE("Found best entry (fits): %d", bestEntry);
        return bestEntry;
    }

    TRACE("No entry fits, looking for smallest larger entry");

    /* Now find the smallest one larger than the requested size */

    maxwidth = 255;
    maxheight = 255;
    for ( i = 0; get_entry( dir, size, i, &cx, &cy, &bits ); i++ )
    {
        TRACE("Entry %d: %dx%d, %d bpp", i, cx, cy, bits);
        if (cx > maxwidth || cy > maxheight) 
        {
            TRACE("  Larger than current smallest (%dx%d), skipping", maxwidth, maxheight);
            continue;
        }
        if (cx == maxwidth && cy == maxheight)
        {
            if (loadflags & LR_MONOCHROME)
            {
                if (maxbits && bits >= maxbits) 
                {
                    TRACE("  Same size but not better bpp (monochrome), skipping");
                    continue;
                }
            }
            else if (bits <= maxbits) 
            {
                TRACE("  Same size but not better bpp, skipping");
                continue;
            }
        }
        bestEntry = i;
        maxwidth  = cx;
        maxheight = cy;
        maxbits = bits;
        TRACE("  New best entry: %d (%dx%d, %d bpp)", bestEntry, maxwidth, maxheight, maxbits);
    }
    if (bestEntry == -1) 
    {
        bestEntry = 0;
        TRACE("No suitable entry found, using first entry: %d", bestEntry);
    }
    else
    {
        TRACE("Found best entry (larger): %d", bestEntry);
    }

	FUNCTION_END
	return bestEntry;
}

static BOOL CURSORICON_GetResIconEntry( LPVOID dir, DWORD size, int n,
                                        int FAR *width, int FAR *height, int FAR *bits )
{
	CURSORICONDIR FAR * resdir = dir;
	ICONRESDIR FAR * icon;

/*	FUNCTION_START*/
    TRACE("CURSORICON_GetResIconEntry: dir=%p, size=%u, n=%d", dir, size, n);

	if ( resdir->idCount <= n )
	{
        TRACE("  Index %d out of range (idCount=%d)", n, resdir->idCount);
/*		FUNCTION_END*/
	        return FALSE;
	}
	if ((const char FAR *)&resdir->idEntries[n + 1] - (const char FAR *)dir > size)
	{
        TRACE("  Entry %d exceeds buffer size", n);
/*		FUNCTION_END*/
	        return FALSE;
	}
    icon = &resdir->idEntries[n].ResInfo.icon;
    *width = icon->bWidth;
    *height = icon->bHeight;
    *bits = resdir->idEntries[n].wBitCount;
    if (!*width && !*height) 
    {
        *width = 256;
        *height = 256;
        TRACE("  Width/height 0, using 256");
    }

    TRACE("  Entry %d: %dx%d, %d bpp", n, *width, *height, *bits);
/*	FUNCTION_END*/

    return TRUE;
}


int abs(int i)
{
  if (i < 0)
    return -i;
  else
    return i;
}

/**********************************************************************
 *	    CURSORICON_FindBestIcon
 *
 * Find the icon closest to the requested size and bit depth.
 */
int CURSORICON_FindBestIcon( LPVOID dir, DWORD size, fnGetCIEntry get_entry,
                                    int width, int height, int depth, UINT loadflags )
{
    int i, cx, cy, bits, bestEntry = -1;
    UINT iTotalDiff, iXDiff = 0, iYDiff = 0, iColorDiff;
    UINT iTempXDiff, iTempYDiff, iTempColorDiff;

/*	FUNCTION_START*/
    TRACE("CURSORICON_FindBestIcon: dir=%p, size=%u, width=%d, height=%d, depth=%d, loadflags=%08X",
          dir, size, width, height, depth, loadflags);

    /* Find Best Fit */
    iTotalDiff = 0xFFFFFFFF;
    iColorDiff = 0xFFFFFFFF;

    if (loadflags & LR_DEFAULTSIZE)
    {
        if (!width) width = GetSystemMetrics( SM_CXICON );
        if (!height) height = GetSystemMetrics( SM_CYICON );
        TRACE("LR_DEFAULTSIZE set: width=%d, height=%d", width, height);
    }
    else if (!width && !height)
    {
        /* use the size of the first entry */
        if (!get_entry( dir, size, 0, &width, &height, &bits )) 
        {
            TRACE("Failed to get first entry");
            return -1;
        }
        iTotalDiff = 0;
        TRACE("Using first entry size: %dx%d, %d bpp (iTotalDiff=0)", width, height, bits);
    }

    TRACE("Starting search for best icon...");
    for ( i = 0; iTotalDiff && get_entry( dir, size, i, &cx, &cy, &bits ); i++ )
    {
        TRACE("Entry %d: %dx%d, %d bpp", i, cx, cy, bits);
        iTempXDiff = abs(width - cx);
        iTempYDiff = abs(height - cy);

        if(iTotalDiff > (iTempXDiff + iTempYDiff))
        {
            iXDiff = iTempXDiff;
            iYDiff = iTempYDiff;
            iTotalDiff = iXDiff + iYDiff;
            TRACE("  New best size diff: iXDiff=%u, iYDiff=%u, iTotalDiff=%u", 
                  iXDiff, iYDiff, iTotalDiff);
        }
    }

    TRACE("Best size diff found: iTotalDiff=%u, iXDiff=%u, iYDiff=%u", iTotalDiff, iXDiff, iYDiff);
    
    /* Find Best Colors for Best Fit */
    TRACE("Looking for best color match with size diff %ux%u...", iXDiff, iYDiff);
    for ( i = 0; get_entry( dir, size, i, &cx, &cy, &bits ); i++ )
    {
        TRACE("Entry %d: %dx%d, %d bpp", i, cx, cy, bits);

        if(abs(width - cx) == iXDiff && abs(height - cy) == iYDiff)
        {
            iTempColorDiff = abs(depth - bits);
            TRACE("  Size matches: colorDiff=%u (current best=%u)", iTempColorDiff, iColorDiff);
            if(iColorDiff > iTempColorDiff)
            {
                bestEntry = i;
                iColorDiff = iTempColorDiff;
                TRACE("  New best entry: %d (colorDiff=%u)", bestEntry, iColorDiff);
            }
        }
    }

    TRACE("Best icon entry: %d", bestEntry);
/*	FUNCTION_END*/
    return bestEntry;
}

CURSORICONDIRENTRY FAR * CURSORICON_FindBestIconRes( CURSORICONDIR FAR * dir, DWORD size,
                                                             int width, int height, int depth,
                                                             UINT loadflags )
{
    int n;
    
    TRACE("CURSORICON_FindBestIconRes: dir=%p, size=%u, %dx%d, depth=%d, loadflags=%08X",
          dir, size, width, height, depth, loadflags);

    n = CURSORICON_FindBestIcon( dir, size, CURSORICON_GetResIconEntry,
                                 width, height, depth, loadflags );
    if ( n < 0 )
    {
        TRACE("CURSORICON_FindBestIconRes: no suitable icon found");
        return NULL;
    }
    
    TRACE("CURSORICON_FindBestIconRes: best entry index=%d", n);
    return &dir->idEntries[n];
}

BOOL CURSORICON_GetResCursorEntry( LPVOID dir, DWORD size, int n,
                                          int FAR *width, int FAR *height, int FAR *bits )
{
    CURSORICONDIR FAR *resdir = dir;
    CURSORDIR FAR *cursor;

/*	FUNCTION_START*/
    TRACE("CURSORICON_GetResCursorEntry: dir=%p, size=%u, n=%d", dir, size, n);

	if ( resdir->idCount <= n )
	{
        TRACE("  Index %d out of range (idCount=%d)", n, resdir->idCount);
/*		FUNCTION_END*/
        	return FALSE;
	}
    if ((char FAR *)&resdir->idEntries[n + 1] - (char FAR *)dir > size)
    {
        TRACE("  Entry %d exceeds buffer size", n);
/*		FUNCTION_END*/
        return FALSE;
    }
    cursor = &resdir->idEntries[n].ResInfo.cursor;
    *width = cursor->wWidth;
    *height = cursor->wHeight;
    *bits = resdir->idEntries[n].wBitCount;
    if (*height == *width * 2) 
    {
        TRACE("  Adjusting height: was %d (width*2), now %d", *height, *height/2);
        *height /= 2;
    }

    TRACE("  Entry %d: %dx%d, %d bpp", n, *width, *height, *bits);
/*	FUNCTION_END*/

    return TRUE;
}

CURSORICONDIRENTRY FAR * CURSORICON_FindBestCursorRes(CURSORICONDIR FAR *dir, DWORD size,
                                                               int width, int height, int depth,
                                                               UINT loadflags )
{
	int n;

/*	FUNCTION_START*/
    TRACE("CURSORICON_FindBestCursorRes: dir=%p, size=%u, %dx%d, depth=%d, loadflags=%08X",
          dir, size, width, height, depth, loadflags);

	n = CURSORICON_FindBestCursor( dir, size, CURSORICON_GetResCursorEntry,
                                       width, height, depth, loadflags );
	
    TRACE("CURSORICON_FindBestCursorRes: best entry index=%d", n);
/*	FUNCTION_END*/
	if ( n < 0 ) 
    {
        TRACE("CURSORICON_FindBestCursorRes: no suitable cursor found");
        return NULL;
    }
	return &dir->idEntries[n];
}

void dumpcursoricondir(CURSORICONDIR FAR * dir)
{
/*typedef struct
{
    WORD                idReserved;
    WORD                idType;
    WORD                idCount;
    CURSORICONDIRENTRY  idEntries[1];
} CURSORICONDIR;
	TRACE("idReserver=%d idType=%d idCount=%d\n\r", dir->idReserved, dir->idType, dir->idCount);*/
}

/**********************************************************************
 *		LookupIconIdFromDirectoryEx
 */
int WINAPI LookupIconIdFromDirectoryEx( LPBYTE xdir, BOOL bIcon,
             int width, int height, UINT cFlag )
{
	CURSORICONDIR FAR * dir = (CURSORICONDIR FAR *)xdir;
	UINT retVal = 0;

	FUNCTION_START

	TRACE("LookupIconIdFromDirectoryEx: bIcon=%d, width=%d, height=%d, cFlag=%08X", 
          bIcon, width, height, cFlag);
	TRACE("Directory at %p, idReserved=%d, idType=%d, idCount=%d", 
          dir, dir->idReserved, dir->idType, dir->idCount);

	dumpcursoricondir(dir);

	if(dir && !dir->idReserved && (dir->idType & 3))
	{
		CURSORICONDIRENTRY FAR * entry;
		int depth = (cFlag & LR_MONOCHROME) ? 1 : get_display_bpp();
		TRACE("depth=%d", depth);

		if(bIcon)
		{
			TRACE("Looking for best icon");
			entry = CURSORICON_FindBestIconRes( dir, ~0u, width, height, depth, LR_DEFAULTSIZE );
		}
		else
		{
			TRACE("Looking for best cursor");
			entry = CURSORICON_FindBestCursorRes( dir, ~0u, width, height, depth, LR_DEFAULTSIZE );
		}

		if (entry) 
		{
			retVal = entry->wResId;
			TRACE("Found entry: wResId=%d, wBitCount=%d", retVal, entry->wBitCount);
			if (bIcon)
			{
				TRACE("Icon: bWidth=%d, bHeight=%d", 
                      entry->ResInfo.icon.bWidth, entry->ResInfo.icon.bHeight);
			}
			else
			{
				TRACE("Cursor: wWidth=%d, wHeight=%d", 
                      entry->ResInfo.cursor.wWidth, entry->ResInfo.cursor.wHeight);
			}
		}
		else 
		{
			TRACE("No suitable entry found!");
		}
	} 
	else 
	{
		TRACE("invalid resource directory: dir=%p, idReserved=%d, idType=%d", 
              dir, dir ? dir->idReserved : -1, dir ? dir->idType : -1);
	}

	TRACE("ret=%d", retVal);
	FUNCTION_END
	return retVal;
}

static int get_bitmap_width_bytes( int width, int bpp )
{
/*	FUNCTION_START*/
    TRACE("get_bitmap_width_bytes: width=%d, bpp=%d", width, bpp);
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
    default:
        TRACE("Unknown depth %d, please report.\n", bpp );
    }
    return -1;
}

/*void dumpbits(LPBYTE bits)
{
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;
    LPBITMAPINFO bmi=(LPBITMAPINFO)(bits);
    TRACE("biSize=%d", bmi->bmiHeader.biSize);
    TRACE("biWidth=%d", bmi->bmiHeader.biWidth);
    TRACE("biHeight=%d", bmi->bmiHeader.biHeight);
    TRACE("biPlanes=%d", bmi->bmiHeader.biPlanes);
    TRACE("biBitCount=%d", bmi->bmiHeader.biBitCount);
    TRACE("biCompression=%d", bmi->bmiHeader.biCompression);
    TRACE("biSizeImage=%d", bmi->bmiHeader.biSizeImage);
    TRACE("biXPelsPerMeter=%d", bmi->bmiHeader.biXPelsPerMeter);
    TRACE("biXPelsPerMeter=%d", bmi->bmiHeader.biYPelsPerMeter);
    TRACE("biClrUsed=%d", bmi->bmiHeader.biClrUsed);
    TRACE("biClrImportant=%d", bmi->bmiHeader.biClrImportant);
}*/

HICON WINAPI CreateIconFromResourceEx(HINSTANCE hinst, LPBYTE bits, UINT cbSize,
                                       BOOL bIcon, DWORD dwVersion,
                                       int width, int height,
                                       UINT cFlag )
{
    POINT hotspot;
    BITMAPINFO FAR *bmi;
    HCURSOR result;
    LPBYTE lpXOR, lpAND;
    LPBYTE lpXOR_flipped, lpAND_flipped;
    LPBYTE lpXOR_scaled, lpAND_scaled;
    int xorRowSize, andRowSize;
    int scaledXorRowSize, scaledAndRowSize;
    int y, x, i;
    HGLOBAL hMemXOR, hMemAND;
    HGLOBAL hMemXOR_scaled, hMemAND_scaled;
    DWORD xorOffset, andOffset;
    DWORD expectedXORSize, expectedANDSize;
    DWORD andOffsetFromXOR;
    BOOL needFlip;
    BOOL needScale;
    int dibWidth, dibHeight;
    int srcY, srcX, srcByte, srcBit, dstByte, dstBit;
    int srcByteIndex, srcNibble, dstByteIndex, dstNibble;
    BYTE srcPixel, dstPixel;
    BYTE FAR *srcXOR_line, FAR *srcAND_line, FAR *dstXOR_line, FAR *dstAND_line;
    int realHeight;

    FUNCTION_START
    
    TRACE("CreateIconFromResourceEx called:");
    TRACE("  hinst=%08X", hinst);
    TRACE("  bits=%p", bits);
    TRACE("  cbSize=%u", cbSize);
    TRACE("  bIcon=%d (1=icon, 0=cursor)", bIcon);
    TRACE("  dwVersion=%08X", dwVersion);
    TRACE("  width=%d, height=%d", width, height);
    TRACE("  cFlag=%08X", cFlag);

    /* Инициализация */
    lpXOR_scaled = NULL;
    lpAND_scaled = NULL;
    hMemXOR = 0;
    hMemAND = 0;
    hMemXOR_scaled = 0;
    hMemAND_scaled = 0;
    needFlip = FALSE;
    needScale = FALSE;
    lpXOR_flipped = NULL;
    lpAND_flipped = NULL;
    result = 0;
    dibWidth = 0;
    dibHeight = 0;

    if (!bits) 
    {
        TRACE("Error: bits is NULL!");
        FUNCTION_END
        return 0;
    }

    if (dwVersion == 0x00020000)
    {
        TRACE("\t2.xx resources are not supported\n");
        FUNCTION_END
        return 0;
    }

    if (bIcon)
    {
        /* Для иконки hotspot в центре */
        hotspot.x = width / 2;
        hotspot.y = height / 2;
        bmi = (BITMAPINFO FAR *)bits;
        TRACE("Icon: hotspot at (%d, %d)", hotspot.x, hotspot.y);
    }
    else
    {
        /* Для курсора первые 4 байта - hotspot */
        short FAR *pt = (short FAR *)bits;
        hotspot.x = pt[0];
        hotspot.y = pt[1];
        bmi = (LPBITMAPINFO)(bits + 4);
        TRACE("Cursor: hotspot at (%d, %d)", hotspot.x, hotspot.y);
    }

    /* Получаем размеры из заголовка DIB */
    dibWidth = bmi->bmiHeader.biWidth;
    
    TRACE("DIB Header:");
    TRACE("  biSize=%d", bmi->bmiHeader.biSize);
    TRACE("  biWidth=%d", bmi->bmiHeader.biWidth);
    TRACE("  biHeight=%d", bmi->bmiHeader.biHeight);
    TRACE("  biPlanes=%d", bmi->bmiHeader.biPlanes);
    TRACE("  biBitCount=%d", bmi->bmiHeader.biBitCount);
    TRACE("  biCompression=%d", bmi->bmiHeader.biCompression);
    TRACE("  biSizeImage=%d", bmi->bmiHeader.biSizeImage);
    
    /* Высота в DIB может быть положительной (снизу вверх) или отрицательной (сверху вниз) */
    if (bmi->bmiHeader.biHeight > 0) 
    {
        /* Положительная высота - данные снизу вверх, нужно переворачивать */
        dibHeight = bmi->bmiHeader.biHeight;
        needFlip = TRUE;
        TRACE("DIB height positive (%d), needFlip=TRUE", dibHeight);
    }
    else 
    {
        /* Отрицательная высота - данные сверху вниз, не нужно переворачивать */
        dibHeight = -bmi->bmiHeader.biHeight;
        needFlip = FALSE;
        TRACE("DIB height negative (%d), needFlip=FALSE", dibHeight);
    }

    /* ВНИМАНИЕ: Для курсоров и иконок высота в DIB - это высота XOR+AND масок! */
    /* Поэтому реальная высота изображения = dibHeight / 2 */
    if (dibHeight % 2 != 0)
    {
        /* На всякий случай проверяем, что высота четная */
        TRACE("Warning: DIB height is odd: %d", dibHeight);
    }
    
    /* Реальная высота иконки/курсора */
    realHeight = dibHeight / 2;
    TRACE("realHeight (dibHeight/2) = %d", realHeight);

    /* Определяем, нужно ли масштабирование */
    if (width > 0 && height > 0 && (width != dibWidth || height != realHeight))
    {
        needScale = TRUE;
        TRACE("Need scale: target=%dx%d, source=%dx%d", width, height, dibWidth, realHeight);
    }
    else
    {
        /* Используем оригинальные размеры */
        width = dibWidth;
        height = realHeight;
        TRACE("No scale needed: using source size %dx%d", width, height);
    }

    /* Вычисляем размеры строк для оригинала */
    xorRowSize = get_bitmap_width_bytes(dibWidth, bmi->bmiHeader.biBitCount);
    andRowSize = get_bitmap_width_bytes(dibWidth, 1);
    TRACE("Row sizes: xorRowSize=%d, andRowSize=%d", xorRowSize, andRowSize);

    /* Вычисляем смещение до данных XOR маски */
    xorOffset = bmi->bmiHeader.biSize;
    TRACE("Initial xorOffset (biSize) = %u", xorOffset);
    
    if (bmi->bmiHeader.biClrUsed > 0)
    {
        xorOffset += bmi->bmiHeader.biClrUsed * sizeof(RGBQUAD);
        TRACE("After adding colors: xorOffset = %u (biClrUsed=%d)", 
              xorOffset, bmi->bmiHeader.biClrUsed);
    }
    else if (bmi->bmiHeader.biBitCount <= 8)
    {
        DWORD colorTableSize = (1 << bmi->bmiHeader.biBitCount) * sizeof(RGBQUAD);
        xorOffset += colorTableSize;
        TRACE("After adding default color table: xorOffset = %u (bpp=%d, tableSize=%u)", 
              xorOffset, bmi->bmiHeader.biBitCount, colorTableSize);
    }
    
    /* Выравнивание до 4 байт */
    if (xorOffset % 4 != 0)
    {
        DWORD oldOffset = xorOffset;
        xorOffset += 4 - (xorOffset % 4);
        TRACE("Alignment: %u -> %u", oldOffset, xorOffset);
    }
    
    lpXOR = (LPBYTE)bmi + xorOffset;
    TRACE("XOR data starts at %p (offset %u)", lpXOR, xorOffset);
    
    /* XOR маска занимает realHeight строк (половина от dibHeight) */
    expectedXORSize = xorRowSize * realHeight;
    expectedANDSize = andRowSize * realHeight;
    TRACE("Expected sizes: XOR=%u, AND=%u, total=%u", 
          expectedXORSize, expectedANDSize, expectedXORSize + expectedANDSize);
    
    /* Проверяем, что данные помещаются в буфер */
    {
        DWORD totalNeeded = (DWORD)(lpXOR - bits) + expectedXORSize + expectedANDSize;
        TRACE("Total needed: %u, cbSize: %u", totalNeeded, cbSize);
        
        if (totalNeeded > cbSize)
        {
            TRACE("ERROR: Resource data too small for icon/cursor!");
            TRACE("  Needed at least: %u bytes", totalNeeded);
            TRACE("  Available: %u bytes", cbSize);
            FUNCTION_END
            return 0;
        }
    }
    
    /* AND маска начинается сразу после XOR маски */
    lpAND = lpXOR + expectedXORSize;
    
    /* Для иконок и курсоров одинаково: выравнивание AND маски до 4 байт */
    andOffsetFromXOR = expectedXORSize;
    if (andOffsetFromXOR % 4 != 0)
    {
        andOffsetFromXOR += 4 - (andOffsetFromXOR % 4);
        lpAND = lpXOR + andOffsetFromXOR;
        TRACE("AND data aligned to %u bytes from XOR start", andOffsetFromXOR);
    }
    
    TRACE("XOR data: %p-%p", lpXOR, lpXOR + expectedXORSize - 1);
    TRACE("AND data: %p-%p", lpAND, lpAND + expectedANDSize - 1);

    /* Выделяем память для перевернутых масок */
    TRACE("Allocating memory for flipped masks...");
    hMemXOR = GlobalAlloc(GMEM_MOVEABLE, expectedXORSize);
    hMemAND = GlobalAlloc(GMEM_MOVEABLE, andRowSize * realHeight);
    
    if (!hMemXOR || !hMemAND)
    {
        TRACE("ERROR: Memory allocation failed!");
        TRACE("  hMemXOR=%p, hMemAND=%p", hMemXOR, hMemAND);
        if (hMemXOR) GlobalFree(hMemXOR);
        if (hMemAND) GlobalFree(hMemAND);
        FUNCTION_END
        return 0;
    }
    
    lpXOR_flipped = (LPBYTE)GlobalLock(hMemXOR);
    lpAND_flipped = (LPBYTE)GlobalLock(hMemAND);
    
    if (!lpXOR_flipped || !lpAND_flipped)
    {
        TRACE("ERROR: Failed to lock memory!");
        if (hMemXOR) 
        {
            GlobalUnlock(hMemXOR); 
            GlobalFree(hMemXOR);
        }
        if (hMemAND) 
        {
            GlobalUnlock(hMemAND); 
            GlobalFree(hMemAND);
        }
        FUNCTION_END
        return 0;
    }
    
    TRACE("Memory allocated and locked successfully");
    
    /* Копируем XOR маску (первые realHeight строк) */
    TRACE("Copying XOR mask (needFlip=%d)...", needFlip);
    if (needFlip)
    {
        /* Данные снизу вверх - переворачиваем */
        TRACE("Flipping XOR mask (bottom-up to top-down)...");
        for (y = 0; y < realHeight; y++)
        {
            _fmemcpy(lpXOR_flipped + (realHeight - 1 - y) * xorRowSize,
                     lpXOR + y * xorRowSize,
                     xorRowSize);
        }
    }
    else
    {
        /* Данные сверху вниз - просто копируем */
        TRACE("Copying XOR mask (top-down, no flip)...");
        for (y = 0; y < realHeight; y++)
        {
            _fmemcpy(lpXOR_flipped + y * xorRowSize,
                     lpXOR + y * xorRowSize,
                     xorRowSize);
        }
    }
    
    /* Копируем AND маску (следующие realHeight строк после XOR) */
    TRACE("Copying AND mask (needFlip=%d)...", needFlip);
    if (needFlip)
    {
        /* Данные снизу вверх - переворачиваем */
        TRACE("Flipping AND mask (bottom-up to top-down)...");
        for (y = 0; y < realHeight; y++)
        {
            _fmemcpy(lpAND_flipped + (realHeight - 1 - y) * andRowSize,
                     lpAND + y * andRowSize,
                     andRowSize);
        }
    }
    else
    {
        /* Данные сверху вниз - просто копируем */
        TRACE("Copying AND mask (top-down, no flip)...");
        for (y = 0; y < realHeight; y++)
        {
            _fmemcpy(lpAND_flipped + y * andRowSize,
                     lpAND + y * andRowSize,
                     andRowSize);
        }
    }

    TRACE("Masks copied successfully");

    /* Масштабирование, если нужно */
    if (needScale && (bmi->bmiHeader.biBitCount == 1 || bmi->bmiHeader.biBitCount == 4 || bmi->bmiHeader.biBitCount == 8))
    {
        TRACE("Scaling needed for %d bpp image", bmi->bmiHeader.biBitCount);
        
        /* Вычисляем размеры строк для масштабированного изображения */
        scaledXorRowSize = get_bitmap_width_bytes(width, bmi->bmiHeader.biBitCount);
        scaledAndRowSize = get_bitmap_width_bytes(width, 1);
        TRACE("Scaled row sizes: xor=%d, and=%d", scaledXorRowSize, scaledAndRowSize);

        /* Выделяем память для масштабированных масок */
        hMemXOR_scaled = GlobalAlloc(GMEM_MOVEABLE, scaledXorRowSize * height);
        hMemAND_scaled = GlobalAlloc(GMEM_MOVEABLE, scaledAndRowSize * height);
        
        if (!hMemXOR_scaled || !hMemAND_scaled)
        {
            TRACE("ERROR: Failed to allocate memory for scaled masks!");
            if (hMemXOR_scaled) GlobalFree(hMemXOR_scaled);
            if (hMemAND_scaled) GlobalFree(hMemAND_scaled);
            GlobalUnlock(hMemXOR);
            GlobalUnlock(hMemAND);
            GlobalFree(hMemXOR);
            GlobalFree(hMemAND);
            FUNCTION_END
            return 0;
        }
        
        lpXOR_scaled = (LPBYTE)GlobalLock(hMemXOR_scaled);
        lpAND_scaled = (LPBYTE)GlobalLock(hMemAND_scaled);
        
        if (!lpXOR_scaled || !lpAND_scaled)
        {
            TRACE("ERROR: Failed to lock scaled mask memory!");
            if (hMemXOR_scaled) 
            {
                GlobalUnlock(hMemXOR_scaled); 
                GlobalFree(hMemXOR_scaled);
            }
            if (hMemAND_scaled) 
            {
                GlobalUnlock(hMemAND_scaled); 
                GlobalFree(hMemAND_scaled);
            }
            GlobalUnlock(hMemXOR);
            GlobalUnlock(hMemAND);
            GlobalFree(hMemXOR);
            GlobalFree(hMemAND);
            FUNCTION_END
            return 0;
        }
        
        TRACE("Scaled mask memory allocated and locked");
        
        /* Обнуляем масштабированные буферы */
        for (i = 0; i < scaledXorRowSize * height; i++) 
        {
            lpXOR_scaled[i] = 0;
        }
        for (i = 0; i < scaledAndRowSize * height; i++) 
        {
            lpAND_scaled[i] = 0;
        }
        
        /* Простое масштабирование методом ближайшего соседа */
        TRACE("Starting nearest-neighbor scaling...");
        for (y = 0; y < height; y++)
        {
            /* Вычисляем исходную строку */
            srcY = (y * realHeight) / height;
            if (srcY >= realHeight) srcY = realHeight - 1;
            
            srcXOR_line = lpXOR_flipped + srcY * xorRowSize;
            srcAND_line = lpAND_flipped + srcY * andRowSize;
            dstXOR_line = lpXOR_scaled + y * scaledXorRowSize;
            dstAND_line = lpAND_scaled + y * scaledAndRowSize;
            
            /* Масштабируем XOR маску */
            if (bmi->bmiHeader.biBitCount == 1)
            {
                /* Монохромное изображение */
                for (x = 0; x < width; x++)
                {
                    srcX = (x * dibWidth) / width;
                    if (srcX >= dibWidth) srcX = dibWidth - 1;
                    
                    srcByte = srcX / 8;
                    srcBit = 7 - (srcX % 8);
                    dstByte = x / 8;
                    dstBit = 7 - (x % 8);
                    
                    srcPixel = (srcXOR_line[srcByte] >> srcBit) & 1;
                    
                    if (srcPixel)
                        dstXOR_line[dstByte] |= (1 << dstBit);
                    else
                        dstXOR_line[dstByte] &= ~(1 << dstBit);
                }
            }
            else if (bmi->bmiHeader.biBitCount == 4)
            {
                /* 16-цветное изображение */
                for (x = 0; x < width; x++)
                {
                    srcX = (x * dibWidth) / width;
                    if (srcX >= dibWidth) srcX = dibWidth - 1;
                    
                    srcByteIndex = srcX / 2;
                    srcNibble = (srcX % 2) ? 0 : 4;
                    dstByteIndex = x / 2;
                    dstNibble = (x % 2) ? 0 : 4;
                    
                    srcPixel = (srcXOR_line[srcByteIndex] >> srcNibble) & 0x0F;
                    
                    dstXOR_line[dstByteIndex] = (dstXOR_line[dstByteIndex] & (0xF0 >> dstNibble)) | 
                                              (srcPixel << dstNibble);
                }
            }
            else if (bmi->bmiHeader.biBitCount == 8)
            {
                /* 256-цветное изображение */
                for (x = 0; x < width; x++)
                {
                    srcX = (x * dibWidth) / width;
                    if (srcX >= dibWidth) srcX = dibWidth - 1;
                    
                    dstXOR_line[x] = srcXOR_line[srcX];
                }
            }
            
            /* Масштабируем AND маску */
            for (x = 0; x < width; x++)
            {
                srcX = (x * dibWidth) / width;
                if (srcX >= dibWidth) srcX = dibWidth - 1;
                
                srcByte = srcX / 8;
                srcBit = 7 - (srcX % 8);
                dstByte = x / 8;
                dstBit = 7 - (x % 8);
                
                srcPixel = (srcAND_line[srcByte] >> srcBit) & 1;
                
                if (srcPixel)
                    dstAND_line[dstByte] |= (1 << dstBit);
                else
                    dstAND_line[dstByte] &= ~(1 << dstBit);
            }
        }
        
        TRACE("Scaling completed successfully");
        
        GlobalUnlock(hMemXOR_scaled);
        GlobalUnlock(hMemAND_scaled);
    }
    else if (needScale)
    {
        TRACE("WARNING: Scaling not implemented for %d bpp images", bmi->bmiHeader.biBitCount);
    }
    
    GlobalUnlock(hMemXOR);
    GlobalUnlock(hMemAND);
    
    /* Создаем иконку/курсор */
    TRACE("Creating %s...", bIcon ? "icon" : "cursor");
    if (bIcon)
    {
        if (needScale && (bmi->bmiHeader.biBitCount == 1 || bmi->bmiHeader.biBitCount == 4 || bmi->bmiHeader.biBitCount == 8))
        {
            TRACE("Creating scaled icon...");
            result = CreateIcon(hinst, width, height,
                               bmi->bmiHeader.biPlanes,
                               bmi->bmiHeader.biBitCount,
                               lpAND_scaled,
                               lpXOR_scaled);
        }
        else
        {
            TRACE("Creating icon from original/flipped data...");
            result = CreateIcon(hinst, width, height,
                               bmi->bmiHeader.biPlanes,
                               bmi->bmiHeader.biBitCount,
                               lpAND_flipped,
                               lpXOR_flipped);
        }
    }
    else
    {
        if (needScale && (bmi->bmiHeader.biBitCount == 1 || bmi->bmiHeader.biBitCount == 4 || bmi->bmiHeader.biBitCount == 8))
        {
            /* Масштабируем hotspot пропорционально */
            hotspot.x = (hotspot.x * width) / dibWidth;
            hotspot.y = (hotspot.y * height) / realHeight;
            TRACE("Creating scaled cursor with hotspot (%d, %d)...", hotspot.x, hotspot.y);
            result = CreateCursor(hinst, hotspot.x, hotspot.y,
                                 width, height,
                                 lpAND_scaled,
                                 lpXOR_scaled);
        }
        else
        {
            TRACE("Creating cursor from original/flipped data with hotspot (%d, %d)...", 
                  hotspot.x, hotspot.y);
            result = CreateCursor(hinst, hotspot.x, hotspot.y,
                                 width, height,
                                 lpAND_flipped,
                                 lpXOR_flipped);
        }
    }
    
    TRACE("%s created: %p", bIcon ? "Icon" : "Cursor", result);
    
    /* Очистка памяти */
    if (hMemXOR) 
    {
        GlobalFree(hMemXOR);
        TRACE("Freed hMemXOR");
    }
    if (hMemAND) 
    {
        GlobalFree(hMemAND);
        TRACE("Freed hMemAND");
    }
    if (hMemXOR_scaled) 
    {
        GlobalFree(hMemXOR_scaled);
        TRACE("Freed hMemXOR_scaled");
    }
    if (hMemAND_scaled) 
    {
        GlobalFree(hMemAND_scaled);
        TRACE("Freed hMemAND_scaled");
    }
    
    FUNCTION_END
    return result;
}

#pragma code_seg( "FIXED_TEXT" );

/***********************************************************************
 *		LoadImage (USER.389)
 */
HANDLE WINAPI LoadImage(HINSTANCE hinst, LPCSTR name, UINT type, int cx, int cy, UINT flags)
{
	HGLOBAL handle;
	HRSRC hRsrc, hGroupRsrc;
	DWORD size;
	LPVOID data;

	FUNCTION_START
	
	TRACE("LoadImage called:");
	TRACE("  hinst=%08X", hinst);
	if (((DWORD)name) < 0x10000)
		TRACE("Resource name: #%u", (UINT)name)
	else
		TRACE("Resource name: %S", name);

	if (type == IMAGE_BITMAP)
		TRACE("  type=%d (BITMAP)", type)
	else if (type == IMAGE_ICON)
		TRACE("  type=%d (ICON)", type)
	else if (type == IMAGE_CURSOR)
		TRACE("  type=%d (CURSOR)", type)
	else
		TRACE("  type=%d (UNKNOWN)", type);
	TRACE("  cx=%d, cy=%d", cx, cy);
	TRACE("  flags=%08X", flags);

/* @TODO
    if (!hinst || (flags & LR_LOADFROMFILE))
    {
        if (type == IMAGE_BITMAP)
            return HBITMAP_16( LoadImageA( 0, name, type, cx, cy, flags ));
        else
            return get_icon_16( LoadImageA( 0, name, type, cx, cy, flags ));
    }*/

	PushDS();
	SetDS(USER_HeapSel);	
	if (!hinst) 
	{
		hinst = HInstanceDisplay;  /* Load OEM cursor/icon */
		TRACE("Using HInstanceDisplay: %08X", hinst);
	}

	hinst = GetExePtr(hinst);
	TRACE("GetExePtr returned: %08X", hinst);

    if (flags & LR_DEFAULTSIZE)
    {
        if (type == IMAGE_ICON)
        {
            int sysCx = GetSystemMetrics(SM_CXICON);
            int sysCy = GetSystemMetrics(SM_CYICON);
            if (!cx) cx = sysCx;
            if (!cy) cy = sysCy;
            TRACE("LR_DEFAULTSIZE for ICON: using %dx%d (system: %dx%d)", cx, cy, sysCx, sysCy);
        }
        else if (type == IMAGE_CURSOR)
        {
            int sysCx = GetSystemMetrics(SM_CXCURSOR);
            int sysCy = GetSystemMetrics(SM_CYCURSOR);
            if (!cx) cx = sysCx;
            if (!cy) cy = sysCy;
            TRACE("LR_DEFAULTSIZE for CURSOR: using %dx%d (system: %dx%d)", cx, cy, sysCx, sysCy);
        }
    }

	switch (type)
	{
	case IMAGE_BITMAP:
	{
		HBITMAP hBitmap = 0;
		BITMAPINFO FAR *bmi;
		BYTE FAR *bits;
		HDC hdc;
		int width, height;
		DWORD colors;
		DWORD headerSize;
		DWORD FAR *dwData;
		int i;
		WORD wSig;
		BITMAPINFOHEADER winHeader;
		BYTE FAR *palette;
		BYTE FAR *pixelData;
		HGLOBAL hTempInfo = 0;
		BITMAPINFO FAR *tempInfo = NULL;
		int tempInfoSize;
		BOOL isOS2Format = FALSE;
		BOOL isMonochrome = FALSE;
		int rowSize;
		int imageSize;
		HANDLE hScaledBmp;
		HDC hdcSrc;
		HDC hdcDst;
		HANDLE hOldSrc;
		HANDLE hOldDst;
    
		/* Загружаем ресурс */
		TRACE("Loading BITMAP resource...");
		hRsrc = FindResource(hinst, name, (LPCSTR)RT_BITMAP);
		if (!hRsrc) 
		{
			TRACE("Bitmap resource not found");
			return 0;
		}
		TRACE("Bitmap resource found: %p", hRsrc);
    
		handle = LoadResource(hinst, hRsrc);
		if (!handle) 
		{
			TRACE("Error loading bitmap resource");
			return 0;
		}
		TRACE("Bitmap resource loaded: %p", handle);
    
		data = LockResource(handle);
		if (!data)
		{
			FreeResource(handle);
			return 0;
		}
		TRACE("Bitmap resource locked at %p", data);
    
		size = SizeofResource(hinst, hRsrc);
		TRACE("Bitmap resource size: %u", size);
    
		/* Проверяем сигнатуру 'BM' (BITMAPFILEHEADER) */
		wSig = *((WORD FAR *)data);
		TRACE("Bitmap signature: %04X", wSig);
		if (wSig == 0x4D42) /* 'BM' в little-endian */
		{
			data = ((BYTE FAR *)data) + 14; /* Пропускаем BITMAPFILEHEADER */
			TRACE("Skipping BITMAPFILEHEADER (14 bytes)");
		}
    
		/* Получаем размер заголовка */
		dwData = (DWORD FAR *)data;
		headerSize = dwData[0];
		TRACE("Header size: %u", headerSize);
    
		/* Поддерживаем как 12-байтный (OS/2), так и 40-байтный (Windows) формат */
		if (headerSize == 12) /* BITMAPCOREHEADER (OS/2) */
		{
			BITMAPCOREHEADER FAR *coreHeader = (BITMAPCOREHEADER FAR *)data;
			TRACE("OS/2 (BITMAPCOREHEADER) format detected");
        
			/* Конвертируем OS/2 заголовок в Windows заголовок */
			winHeader.biSize = sizeof(BITMAPINFOHEADER);
			winHeader.biWidth = (LONG)coreHeader->bcWidth;
			winHeader.biHeight = (LONG)coreHeader->bcHeight;
			winHeader.biPlanes = coreHeader->bcPlanes;
			winHeader.biBitCount = coreHeader->bcBitCount;
			winHeader.biCompression = 0; /* BI_RGB */
			winHeader.biSizeImage = 0;
			winHeader.biXPelsPerMeter = 0;
			winHeader.biYPelsPerMeter = 0;
			winHeader.biClrUsed = 0;
			winHeader.biClrImportant = 0;
        
			width = winHeader.biWidth;
			height = winHeader.biHeight;
			isOS2Format = TRUE;
        
			TRACE("OS/2 bitmap: %dx%d, %d planes, %d bpp", 
				  width, height, winHeader.biPlanes, winHeader.biBitCount);
        
			/* Смещение до палитры */
			palette = (BYTE FAR *)data + sizeof(BITMAPCOREHEADER);
        
			/* Рассчитываем размер палитры для OS/2 формата */
			if (winHeader.biBitCount <= 8)
			{
				if (winHeader.biBitCount == 1)
				{
					colors = 2;
					isMonochrome = TRUE;
				}
				else if (winHeader.biBitCount == 4)
					colors = 16;
				else /* 8 бит */
					colors = 256;
            
				TRACE("Color palette: %d entries", colors);
            
				/* Смещение до пиксельных данных */
				pixelData = palette + colors * 3; /* OS/2 использует RGBTRIPLE (3 байта) */
			}
			else
			{
				colors = 0;
				pixelData = palette; /* Нет палитры */
				TRACE("No palette (true color)");
			}
        
			/* Выравнивание по DWORD */
			if (((DWORD)pixelData - (DWORD)data) % 4 != 0)
			{
				pixelData += 4 - (((DWORD)pixelData - (DWORD)data) % 4);
				TRACE("Pixel data aligned to DWORD boundary");
			}
        
			TRACE("Pixel data at offset %u", (DWORD)pixelData - (DWORD)data);
        
			/* Создаем временную структуру BITMAPINFO с Windows заголовком */
			tempInfoSize = sizeof(BITMAPINFOHEADER) + colors * sizeof(RGBQUAD);
			hTempInfo = GlobalAlloc(GMEM_FIXED, tempInfoSize);
        
			if (!hTempInfo)
			{
				TRACE("Failed to allocate memory for tempInfo");
				FreeResource(handle);
				return 0;
			}
        
			tempInfo = (BITMAPINFO FAR *)GlobalLock(hTempInfo);
        
			/* Копируем заголовок */
			tempInfo->bmiHeader = winHeader;
        
			/* Конвертируем палитру из OS/2 (RGBTRIPLE) в Windows (RGBQUAD) */
			if (colors > 0)
			{
				RGBTRIPLE FAR *os2pal = (RGBTRIPLE FAR *)palette;
				for (i = 0; i < (int)colors; i++)
				{
					tempInfo->bmiColors[i].rgbRed = os2pal[i].rgbtRed;
					tempInfo->bmiColors[i].rgbGreen = os2pal[i].rgbtGreen;
					tempInfo->bmiColors[i].rgbBlue = os2pal[i].rgbtBlue;
					tempInfo->bmiColors[i].rgbReserved = 0;
				}
				TRACE("Palette converted from OS/2 to Windows format");
			}
        
			/* Создаем DDB из DIB */
			hdc = GetDC(0);
			if (hdc)
			{
				TRACE("Creating DDB from DIB...");
				/* Для монохромных битмапов (1 bpp) - используем CreateBitmap */
				if (isMonochrome)
				{
					rowSize = ((width + 31) / 32) * 4;
					imageSize = rowSize * height;
					TRACE("Monochrome bitmap: rowSize=%d, imageSize=%d", rowSize, imageSize);
                
					hBitmap = CreateBitmap(width, height, 1, 1, NULL);
					if (hBitmap && pixelData)
					{
						TRACE("Setting bitmap bits...");
						SetBitmapBits(hBitmap, imageSize, pixelData);
						TRACE("Bitmap bits set");
					}
				}
				else
				{
					/* Для цветных битмапов используем CreateDIBitmap */
					TRACE("Creating DIBitmap...");
					hBitmap = CreateDIBitmap(hdc, 
										   &winHeader,
										   CBM_INIT,
										   pixelData,
										   tempInfo,
										   DIB_RGB_COLORS);
					TRACE("CreateDIBitmap returned: %p", hBitmap);
				}
            
				ReleaseDC(0, hdc);
			}
			else
			{
				TRACE("Failed to get DC");
			}
        
			if (hTempInfo)
			{
				GlobalUnlock(hTempInfo);
				GlobalFree(hTempInfo);
				TRACE("TempInfo freed");
			}
		}
		else if (headerSize == 40) /* BITMAPINFOHEADER (Windows) */
		{
			bmi = (BITMAPINFO FAR *)data;
			TRACE("Windows (BITMAPINFOHEADER) format detected");
        
			/* Проверяем корректность заголовка */
			if (bmi->bmiHeader.biSize != 40)
			{
				TRACE("Invalid header size: %d (expected 40)", bmi->bmiHeader.biSize);
				FreeResource(handle);
				return 0;
			}
        
			/* Windows 3.0 поддерживает только BI_RGB */
			if (bmi->bmiHeader.biCompression != 0)
			{
				TRACE("Unsupported compression: %d (only BI_RGB=0 supported)", 
					  bmi->bmiHeader.biCompression);
				FreeResource(handle);
				return 0;
			}
        
			/* Получаем размеры */
			width = bmi->bmiHeader.biWidth;
			height = bmi->bmiHeader.biHeight;
			if (height < 0) height = -height;
        
			TRACE("Windows bitmap: %dx%d, %d planes, %d bpp", 
				  width, height, bmi->bmiHeader.biPlanes, bmi->bmiHeader.biBitCount);
        
			/* Вычисляем смещение до пиксельных данных */
			bits = (BYTE FAR *)data + bmi->bmiHeader.biSize;
        
			/* Обрабатываем палитру */
			if (bmi->bmiHeader.biBitCount <= 8)
			{
				if (bmi->bmiHeader.biClrUsed > 0)
					colors = bmi->bmiHeader.biClrUsed;
				else
					colors = 1 << bmi->bmiHeader.biBitCount;
            
				TRACE("Color palette: %d entries", colors);
				bits += colors * sizeof(RGBQUAD);
			}
			else
			{
				colors = 0;
				TRACE("No palette (true color)");
			}
        
			/* Выравнивание по DWORD */
			if (((DWORD)bits - (DWORD)data) % 4 != 0)
			{
				bits += 4 - (((DWORD)bits - (DWORD)data) % 4);
				TRACE("Pixel data aligned to DWORD boundary");
			}
        
			TRACE("Pixel data at offset %u", (DWORD)bits - (DWORD)data);
        
			/* Создаем DDB из DIB */
			hdc = GetDC(0);
			if (hdc)
			{
				TRACE("Creating DDB from DIB...");
				/* Для монохромных битмапов (1 bpp) - используем CreateBitmap */
				if (bmi->bmiHeader.biBitCount == 1)
				{
					rowSize = ((width + 31) / 32) * 4;
					imageSize = rowSize * height;
					TRACE("Monochrome bitmap: rowSize=%d, imageSize=%d", rowSize, imageSize);
                
					hBitmap = CreateBitmap(width, height, 1, 1, NULL);
					if (hBitmap && bits)
					{
						TRACE("Setting bitmap bits...");
						SetBitmapBits(hBitmap, imageSize, bits);
						TRACE("Bitmap bits set");
					}
				}
				else
				{
					/* Для цветных битмапов используем CreateDIBitmap */
					TRACE("Creating DIBitmap...");
					hBitmap = CreateDIBitmap(hdc, 
										   (BITMAPINFOHEADER FAR *)&bmi->bmiHeader,
										   CBM_INIT,
										   bits,
										   bmi,
										   DIB_RGB_COLORS);
					TRACE("CreateDIBitmap returned: %p", hBitmap);
				}
            
				ReleaseDC(0, hdc);
			}
			else
			{
				TRACE("Failed to get DC");
			}
		}
		else
		{
			TRACE("Unsupported header size: %d", headerSize);
			FreeResource(handle);
			return 0;
		}
    
		TRACE("Bitmap created: %p", hBitmap);
		FreeResource(handle);
		TRACE("Resource freed");
    
		/* Масштабирование при необходимости */
		if (hBitmap && (cx != 0 && cy != 0) && (cx != width || cy != height))
		{
			TRACE("Scaling bitmap from %dx%d to %dx%d", width, height, cx, cy);
			hScaledBmp = 0;
			hdcSrc = CreateCompatibleDC(0);
			hdcDst = CreateCompatibleDC(0);
        
			if (hdcSrc && hdcDst)
			{
				TRACE("DCs created for scaling");
				hOldSrc = SelectObject(hdcSrc, hBitmap);
            
				hScaledBmp = CreateCompatibleBitmap(hdcSrc, cx, cy);
				if (hScaledBmp)
				{
					TRACE("Scaled bitmap created: %p", hScaledBmp);
					hOldDst = SelectObject(hdcDst, hScaledBmp);
                
					TRACE("Stretching bitmap...");
					StretchBlt(hdcDst, 0, 0, cx, cy,
							  hdcSrc, 0, 0, width, height,
							  SRCCOPY);
					TRACE("StretchBlt completed");
                
					SelectObject(hdcDst, hOldDst);
				}
				else
				{
					TRACE("Failed to create scaled bitmap");
				}
            
				SelectObject(hdcSrc, hOldSrc);
			}
			else
			{
				TRACE("Failed to create DCs for scaling");
			}
        
			if (hdcSrc) DeleteDC(hdcSrc);
			if (hdcDst) DeleteDC(hdcDst);
        
			if (hScaledBmp)
			{
				TRACE("Replacing original bitmap with scaled one");
				DeleteObject(hBitmap);
				hBitmap = hScaledBmp;
			}
		}
    
		TRACE("LoadImage for BITMAP returning: %p", hBitmap);
		return hBitmap;
	}
	case IMAGE_ICON:
	case IMAGE_CURSOR:
	{
		HICON hIcon = 0;
		BYTE FAR *dir, FAR *bits;
		int id = 0;
        
		TRACE("Loading %s resource...", type == IMAGE_ICON ? "ICON" : "CURSOR");
if (((DWORD)name) < 0x10000)
    TRACE("Resource name: #%u", (UINT)name)
else
    TRACE("Resource name: %s", name);

		hRsrc = FindResource(hinst, name, (LPCSTR)(type == IMAGE_ICON ? RT_GROUP_ICON : RT_GROUP_CURSOR));
		if (!hRsrc) 
		{
			TRACE("ERROR: Group resource not found!");
			FUNCTION_END
			return 0;
		}
		hGroupRsrc = hRsrc;
		TRACE("Group resource found: %p", hRsrc);
        
		if (!(handle = LoadResource( hinst, hRsrc ))) 
		{
			TRACE("ERROR: Failed to load group resource!");
			FUNCTION_END
			return 0;
		}
		TRACE("Group resource loaded: %p", handle);
        
		if ((dir = LockResource( handle ))) 
		{
			TRACE("Group resource locked at %p", dir);
			id = LookupIconIdFromDirectoryEx( dir, type == IMAGE_ICON, 0, 0, (type == IMAGE_ICON) ? 0 : LR_MONOCHROME);
			TRACE("LookupIconIdFromDirectoryEx returned id=%d", id);
		}
		FreeResource( handle );
		TRACE("Group resource freed");
        
		if (!id) 
		{
			TRACE("ERROR: No valid icon/cursor ID found!");
			FUNCTION_END
			return 0;
		}

		TRACE("Looking for specific resource with ID=%d...", id);
		if (!(hRsrc = FindResource( hinst, MAKEINTRESOURCE(id),
									(LPCSTR)(type == IMAGE_ICON ? RT_ICON : RT_CURSOR) ))) 
		{
			TRACE("ERROR: Specific icon/cursor resource not found!");
			FUNCTION_END
			return 0;
		}
		TRACE("Specific resource found: %p", hRsrc);

/* @todo port shared icon
		if ((flags & LR_SHARED) && (hIcon = find_shared_icon( hinst, hRsrc ) ) != 0) return hIcon;*/

		if (!(handle = LoadResource(hinst, hRsrc))) 
		{
			TRACE("ERROR: Failed to load specific resource!");
			FUNCTION_END
			return 0;
		}
		TRACE("Specific resource loaded: %p", handle);
        
		bits = LockResource(handle);
		size = SizeofResource(hinst, hRsrc);
		TRACE("Resource locked at %p, size=%u", bits, size);

		TRACE("Calling CreateIconFromResourceEx...");
		hIcon = CreateIconFromResourceEx(hinst, bits, size, type == IMAGE_ICON, 0x00030000, cx, cy, flags);
		TRACE("CreateIconFromResourceEx returned: %p", hIcon);

		FreeResource( handle );
		TRACE("Specific resource freed");

/* @todo port shared icons code from wine
		if (hIcon && (flags & LR_SHARED)) add_shared_icon( hinst, hRsrc, hGroupRsrc, hIcon );*/
	
		FUNCTION_END
		return hIcon;
	}
	default:
		TRACE("ERROR: Unknown image type: %d", type);
		return 0;
	}

	FUNCTION_END
	return 0;
}

/***********************************************************************
 *		LoadCursor (USER.173)
 */
HCURSOR WINAPI LoadCursor(HINSTANCE hInstance, LPCSTR name)
{
    HCURSOR cursor;
	if (((DWORD)name) < 0x10000)
		TRACE("LoadCursor: hInstance=%08X, name=#%u", hInstance, name)
	else
		TRACE("LoadCursor: hInstance=%08X, name=%S", hInstance, name);
    cursor = LoadImage(hInstance, name, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    TRACE("LoadCursor returning: %p", cursor);
    return cursor;
}

/***********************************************************************
 *		LoadIcon (USER.174)
 */
HICON WINAPI LoadIcon(HINSTANCE hInstance, LPCSTR name)
{
    HICON icon;
	if (((DWORD)name) < 0x10000)
		TRACE("LoadIcon: hInstance=%08X, name=#%u", hInstance, name)
	else
		TRACE("LoadIcon: hInstance=%08X, name=%S", hInstance, name);
    icon = LoadImage(hInstance, name, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    TRACE("LoadIcon returning: %p", icon);
    return icon;
}

/**********************************************************************
 *		LoadBitmap (USER.175)
 */
HBITMAP WINAPI LoadBitmap(HINSTANCE hInstance, LPCSTR name)
{
	HBITMAP bitmap;
	if (((DWORD)name) < 0x10000)
		TRACE("LoadBitmap: hInstance=%08X, name=#%u", hInstance, name)
	else
		TRACE("LoadBitmap: hInstance=%08X, name=%S", hInstance, name);

    bitmap = LoadImage(hInstance, name, IMAGE_BITMAP, 0, 0, 0);
    TRACE("LoadBitmap returning: %p", bitmap);
    return bitmap;
}

#pragma code_seg();
