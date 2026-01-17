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

//#ifndef BI_BITFIELDS
//#define BI_BITFIELDS     3
//#endif

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

//	FUNCTION_START
//	TRACE("inst=%x id=%x buff=%x:%x len=%d", instance, resource_id, buffer, buflen);

	hrsrc = FindResource( instance, MAKEINTRESOURCE((resource_id>>4)+1), RT_STRING );
	if (!hrsrc) 
	{
		TRACE("Resource not found");
//		FUNCTION_END
		return 0;
	}

	hmem = LoadResource( instance, hrsrc );
	if (!hmem) 
	{
		TRACE("Error loading resource");
//		FUNCTION_END
		return 0;
	}

	p = LockResource(hmem);
	string_num = resource_id & 0x000f;
	while (string_num--) p += *p + 1;

	if (buffer == NULL) ret = *p;
	else
	{
		ret = min(buflen - 1, *p);
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
//        TRACE("%S loaded", buffer);
	}
	FreeResource( hmem );

//	FUNCTION_END
	return ret;
}

/**********************************************************************
 *              LoadAccelerators  (USER.177)
 */
HACCEL WINAPI LoadAccelerators(HINSTANCE hInstance, LPCSTR lpTableName)
{
	HANDLE hResInfo;
	HACCEL rc;

//	FUNCTION_START
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
                              int *width, int *height, int *bits );

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
//	FUNCTION_START

    if (loadflags & LR_DEFAULTSIZE)
    {
        if (!width) width = GetSystemMetrics( SM_CXCURSOR );
        if (!height) height = GetSystemMetrics( SM_CYCURSOR );
    }
    else if (!width && !height)
    {
        /* use the first entry */
        if (!get_entry( dir, size, 0, &width, &height, &bits )) return -1;
//	FUNCTION_END
        return 0;
    }

    /* First find the largest one smaller than or equal to the requested size*/

    maxwidth = maxheight = maxbits = 0;
    for ( i = 0; get_entry( dir, size, i, &cx, &cy, &bits ); i++ )
    {
        if (cx > width || cy > height) continue;
        if (cx < maxwidth || cy < maxheight) continue;
        if (cx == maxwidth && cy == maxheight)
        {
            if (loadflags & LR_MONOCHROME)
            {
                if (maxbits && bits >= maxbits) continue;
            }
            else if (bits <= maxbits) continue;
        }
        bestEntry = i;
        maxwidth  = cx;
        maxheight = cy;
        maxbits = bits;
    }
    if (bestEntry != -1) return bestEntry;

    /* Now find the smallest one larger than the requested size */

    maxwidth = maxheight = 255;
    for ( i = 0; get_entry( dir, size, i, &cx, &cy, &bits ); i++ )
    {
        if (cx > maxwidth || cy > maxheight) continue;
        if (cx == maxwidth && cy == maxheight)
        {
            if (loadflags & LR_MONOCHROME)
            {
                if (maxbits && bits >= maxbits) continue;
            }
            else if (bits <= maxbits) continue;
        }
        bestEntry = i;
        maxwidth  = cx;
        maxheight = cy;
        maxbits = bits;
    }
    if (bestEntry == -1) bestEntry = 0;

//	FUNCTION_END
	return bestEntry;
}

static BOOL CURSORICON_GetResIconEntry( LPVOID dir, DWORD size, int n,
                                        int *width, int *height, int *bits )
{
	CURSORICONDIR FAR * resdir = dir;
	ICONRESDIR FAR * icon;

//	FUNCTION_START

	if ( resdir->idCount <= n )
	{
//		FUNCTION_END
	        return FALSE;
	}
	if ((const char FAR *)&resdir->idEntries[n + 1] - (const char FAR *)dir > size)
	{
//		FUNCTION_END
	        return FALSE;
	}
    icon = &resdir->idEntries[n].ResInfo.icon;
    *width = icon->bWidth;
    *height = icon->bHeight;
    *bits = resdir->idEntries[n].wBitCount;
    if (!*width && !*height) *width = *height = 256;

//	FUNCTION_END

    return TRUE;
}


int abs(int i)
{
  return i < 0 ? -i : i;
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
    UINT iTotalDiff, iXDiff=0, iYDiff=0, iColorDiff;
    UINT iTempXDiff, iTempYDiff, iTempColorDiff;

//	FUNCTION_START

    /* Find Best Fit */
    iTotalDiff = 0xFFFFFFFF;
    iColorDiff = 0xFFFFFFFF;

    if (loadflags & LR_DEFAULTSIZE)
    {
        if (!width) width = GetSystemMetrics( SM_CXICON );
        if (!height) height = GetSystemMetrics( SM_CYICON );
    }
    else if (!width && !height)
    {
        /* use the size of the first entry */
        if (!get_entry( dir, size, 0, &width, &height, &bits )) return -1;
        iTotalDiff = 0;
    }

    for ( i = 0; iTotalDiff && get_entry( dir, size, i, &cx, &cy, &bits ); i++ )
    {
        iTempXDiff = abs(width - cx);
        iTempYDiff = abs(height - cy);

        if(iTotalDiff > (iTempXDiff + iTempYDiff))
        {
            iXDiff = iTempXDiff;
            iYDiff = iTempYDiff;
            iTotalDiff = iXDiff + iYDiff;
        }
    }

    /* Find Best Colors for Best Fit */
    for ( i = 0; get_entry( dir, size, i, &cx, &cy, &bits ); i++ )
    {
//        TRACE("entry %d: %d x %d, %d bpp\n", i, cx, cy, bits);

        if(abs(width - cx) == iXDiff && abs(height - cy) == iYDiff)
        {
            iTempColorDiff = abs(depth - bits);
            if(iColorDiff > iTempColorDiff)
            {
                bestEntry = i;
                iColorDiff = iTempColorDiff;
            }
        }
    }

//	FUNCTION_END
    return bestEntry;
}

CURSORICONDIRENTRY FAR * CURSORICON_FindBestIconRes( CURSORICONDIR FAR * dir, DWORD size,
                                                             int width, int height, int depth,
                                                             UINT loadflags )
{
    int n;

    n = CURSORICON_FindBestIcon( dir, size, CURSORICON_GetResIconEntry,
                                 width, height, depth, loadflags );
    if ( n < 0 )
        return NULL;
    return &dir->idEntries[n];
}

BOOL CURSORICON_GetResCursorEntry( LPVOID dir, DWORD size, int n,
                                          int *width, int *height, int *bits )
{
    CURSORICONDIR FAR *resdir = dir;
    CURSORDIR FAR *cursor;

//	FUNCTION_START

	if ( resdir->idCount <= n )
	{
//		FUNCTION_END
        	return FALSE;
	}
    if ((char FAR *)&resdir->idEntries[n + 1] - (char FAR *)dir > size)
{
//		FUNCTION_END
        return FALSE;
}
    cursor = &resdir->idEntries[n].ResInfo.cursor;
    *width = cursor->wWidth;
    *height = cursor->wHeight;
    *bits = resdir->idEntries[n].wBitCount;
    if (*height == *width * 2) *height /= 2;

//	FUNCTION_END

    return TRUE;
}

CURSORICONDIRENTRY FAR * CURSORICON_FindBestCursorRes(CURSORICONDIR FAR *dir, DWORD size,
                                                               int width, int height, int depth,
                                                               UINT loadflags )
{
	int n;

//	FUNCTION_START

	n = CURSORICON_FindBestCursor( dir, size, CURSORICON_GetResCursorEntry,
                                       width, height, depth, loadflags );
	
//	FUNCTION_END
	if ( n < 0 ) return NULL;
	return &dir->idEntries[n];
}

void dumpcursoricondir(CURSORICONDIR FAR * dir)
{
//typedef struct
//{
//    WORD                idReserved;
//    WORD                idType;
//    WORD                idCount;
//    CURSORICONDIRENTRY  idEntries[1];
//} CURSORICONDIR;
//	TRACE("idReserver=%d idType=%d idCount=%d\n\r", dir->idReserved, dir->idType, dir->idCount);
}
/**********************************************************************
 *		LookupIconIdFromDirectoryEx (USER32.@)
 */
int WINAPI LookupIconIdFromDirectoryEx( LPBYTE xdir, BOOL bIcon,
             int width, int height, UINT cFlag )
{
	CURSORICONDIR FAR * dir = (CURSORICONDIR FAR *)xdir;
	UINT retVal = 0;

//	FUNCTION_START

//	dumpcursoricondir(dir);

	if(dir && !dir->idReserved && (dir->idType & 3))
	{
		CURSORICONDIRENTRY FAR * entry;
		int depth = (cFlag & LR_MONOCHROME) ? 1 : get_display_bpp();

		if(bIcon)
			entry = CURSORICON_FindBestIconRes( dir, ~0u, width, height, depth, LR_DEFAULTSIZE );
		else
			entry = CURSORICON_FindBestCursorRes( dir, ~0u, width, height, depth, LR_DEFAULTSIZE );

		if (entry) retVal = entry->wResId;
	} else TRACE("invalid resource directory\n");

//	TRACE("ret=%d", retVal);
//	FUNCTION_END
	return retVal;
}

/**********************************************************************
 *              LookupIconIdFromDirectory (USER32.@)
 */
int WINAPI LookupIconIdFromDirectory( LPBYTE dir, BOOL bIcon )
{
    return LookupIconIdFromDirectoryEx( dir, bIcon, 0, 0, bIcon ? 0 : LR_MONOCHROME );
}


static int get_bitmap_width_bytes( int width, int bpp )
{
//	FUNCTION_START
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

//void dumpbits(LPBYTE bits)
//{
//typedef struct tagBITMAPINFO {
//    BITMAPINFOHEADER    bmiHeader;
//    RGBQUAD             bmiColors[1];
//} BITMAPINFO;
//    LPBITMAPINFO bmi=(LPBITMAPINFO)(bits);
//    TRACE("biSize=%d", bmi->bmiHeader.biSize);
//    TRACE("biWidth=%d", bmi->bmiHeader.biWidth);
//    TRACE("biHeight=%d", bmi->bmiHeader.biHeight);
//    TRACE("biPlanes=%d", bmi->bmiHeader.biPlanes);
//    TRACE("biBitCount=%d", bmi->bmiHeader.biBitCount);
//    TRACE("biCompression=%d", bmi->bmiHeader.biCompression);
//    TRACE("biSizeImage=%d", bmi->bmiHeader.biSizeImage);
//    TRACE("biXPelsPerMeter=%d", bmi->bmiHeader.biXPelsPerMeter);
//    TRACE("biXPelsPerMeter=%d", bmi->bmiHeader.biYPelsPerMeter);
//    TRACE("biClrUsed=%d", bmi->bmiHeader.biClrUsed);
//    TRACE("biClrImportant=%d", bmi->bmiHeader.biClrImportant);
//}

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
    BYTE FAR *srcXOR_line, *srcAND_line, *dstXOR_line, *dstAND_line;
    int realHeight;

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

    if (!bits) return 0;

    if (dwVersion == 0x00020000)
    {
        TRACE("\t2.xx resources are not supported\n");
        return 0;
    }

    if (bIcon)
    {
        /* Для иконки hotspot в центре */
        hotspot.x = width / 2;
        hotspot.y = height / 2;
        bmi = (BITMAPINFO FAR *)bits;
    }
    else
    {
        /* Для курсора первые 4 байта - hotspot */
        short FAR *pt = (short FAR *)bits;
        hotspot.x = pt[0];
        hotspot.y = pt[1];
        bmi = (LPBITMAPINFO)(bits + 4);
    }

    /* Получаем размеры из заголовка DIB */
    dibWidth = bmi->bmiHeader.biWidth;
    
    /* Высота в DIB может быть положительной (снизу вверх) или отрицательной (сверху вниз) */
    if (bmi->bmiHeader.biHeight > 0) 
    {
        /* Положительная высота - данные снизу вверх, нужно переворачивать */
        dibHeight = bmi->bmiHeader.biHeight;
        needFlip = TRUE;
    }
    else 
    {
        /* Отрицательная высота - данные сверху вниз, не нужно переворачивать */
        dibHeight = -bmi->bmiHeader.biHeight;
        needFlip = FALSE;
    }

    /* ВНИМАНИЕ: Для курсоров и иконок высота в DIB - это высота XOR+AND масок! */
    /* Поэтому реальная высота изображения = dibHeight / 2 */
    if (dibHeight % 2 != 0)
    {
        /* На всякий случай проверяем, что высота четная */
        TRACE("Warning: DIB height is odd: %d\n", dibHeight);
    }
    
    /* Реальная высота иконки/курсора */
    realHeight = dibHeight / 2;

    /* Определяем, нужно ли масштабирование */
    if (width > 0 && height > 0 && (width != dibWidth || height != realHeight))
    {
        needScale = TRUE;
    }
    else
    {
        /* Используем оригинальные размеры */
        width = dibWidth;
        height = realHeight;
    }

    /* Вычисляем размеры строк для оригинала */
    xorRowSize = get_bitmap_width_bytes(dibWidth, bmi->bmiHeader.biBitCount);
    andRowSize = get_bitmap_width_bytes(dibWidth, 1);

    /* Вычисляем смещение до данных XOR маски */
    xorOffset = bmi->bmiHeader.biSize;
    
    if (bmi->bmiHeader.biClrUsed > 0)
    {
        xorOffset += bmi->bmiHeader.biClrUsed * sizeof(RGBQUAD);
    }
    else if (bmi->bmiHeader.biBitCount <= 8)
    {
        xorOffset += (1 << bmi->bmiHeader.biBitCount) * sizeof(RGBQUAD);
    }
    
    /* Выравнивание до 4 байт */
    if (xorOffset % 4 != 0)
    {
        xorOffset += 4 - (xorOffset % 4);
    }

    lpXOR = (LPBYTE)bmi + xorOffset;
    
    /* XOR маска занимает realHeight строк (половина от dibHeight) */
    expectedXORSize = xorRowSize * realHeight;
    
    /* Проверяем, что данные помещаются в буфер */
    if ((DWORD)(lpXOR - bits) + expectedXORSize + (andRowSize * realHeight) > cbSize)
    {
        TRACE("Error: Resource data too small for icon/cursor\n");
        return 0;
    }
    
    /* AND маска начинается сразу после XOR маски */
    lpAND = lpXOR + expectedXORSize;
    
    /* Для иконок и курсоров одинаково: выравнивание AND маски до 4 байт */
    andOffsetFromXOR = expectedXORSize;
    if (andOffsetFromXOR % 4 != 0)
    {
        andOffsetFromXOR += 4 - (andOffsetFromXOR % 4);
        lpAND = lpXOR + andOffsetFromXOR;
    }

    /* Выделяем память для перевернутых масок */
    hMemXOR = GlobalAlloc(GMEM_MOVEABLE, expectedXORSize);
    hMemAND = GlobalAlloc(GMEM_MOVEABLE, andRowSize * realHeight);
    
    if (!hMemXOR || !hMemAND)
    {
        if (hMemXOR) GlobalFree(hMemXOR);
        if (hMemAND) GlobalFree(hMemAND);
        return 0;
    }
    
    lpXOR_flipped = (LPBYTE)GlobalLock(hMemXOR);
    lpAND_flipped = (LPBYTE)GlobalLock(hMemAND);
    
    if (!lpXOR_flipped || !lpAND_flipped)
    {
        if (hMemXOR) { GlobalUnlock(hMemXOR); GlobalFree(hMemXOR); }
        if (hMemAND) { GlobalUnlock(hMemAND); GlobalFree(hMemAND); }
        return 0;
    }
    
    /* Копируем XOR маску (первые realHeight строк) */
    if (needFlip)
    {
        /* Данные снизу вверх - переворачиваем */
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
        for (y = 0; y < realHeight; y++)
        {
            _fmemcpy(lpXOR_flipped + y * xorRowSize,
                     lpXOR + y * xorRowSize,
                     xorRowSize);
        }
    }
    
    /* Копируем AND маску (следующие realHeight строк после XOR) */
    if (needFlip)
    {
        /* Данные снизу вверх - переворачиваем */
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
        for (y = 0; y < realHeight; y++)
        {
            _fmemcpy(lpAND_flipped + y * andRowSize,
                     lpAND + y * andRowSize,
                     andRowSize);
        }
    }

    /* Масштабирование, если нужно */
    if (needScale && (bmi->bmiHeader.biBitCount == 1 || bmi->bmiHeader.biBitCount == 4 || bmi->bmiHeader.biBitCount == 8))
    {
        /* Вычисляем размеры строк для масштабированного изображения */
        scaledXorRowSize = get_bitmap_width_bytes(width, bmi->bmiHeader.biBitCount);
        scaledAndRowSize = get_bitmap_width_bytes(width, 1);

        /* Выделяем память для масштабированных масок */
        hMemXOR_scaled = GlobalAlloc(GMEM_MOVEABLE, scaledXorRowSize * height);
        hMemAND_scaled = GlobalAlloc(GMEM_MOVEABLE, scaledAndRowSize * height);
        
        if (!hMemXOR_scaled || !hMemAND_scaled)
        {
            if (hMemXOR_scaled) GlobalFree(hMemXOR_scaled);
            if (hMemAND_scaled) GlobalFree(hMemAND_scaled);
            GlobalUnlock(hMemXOR);
            GlobalUnlock(hMemAND);
            GlobalFree(hMemXOR);
            GlobalFree(hMemAND);
            return 0;
        }
        
        lpXOR_scaled = (LPBYTE)GlobalLock(hMemXOR_scaled);
        lpAND_scaled = (LPBYTE)GlobalLock(hMemAND_scaled);
        
        if (!lpXOR_scaled || !lpAND_scaled)
        {
            if (hMemXOR_scaled) { GlobalUnlock(hMemXOR_scaled); GlobalFree(hMemXOR_scaled); }
            if (hMemAND_scaled) { GlobalUnlock(hMemAND_scaled); GlobalFree(hMemAND_scaled); }
            GlobalUnlock(hMemXOR);
            GlobalUnlock(hMemAND);
            GlobalFree(hMemXOR);
            GlobalFree(hMemAND);
            return 0;
        }
        
        /* Обнуляем масштабированные буферы */
        for (i = 0; i < scaledXorRowSize * height; i++) {
            lpXOR_scaled[i] = 0;
        }
        for (i = 0; i < scaledAndRowSize * height; i++) {
            lpAND_scaled[i] = 0;
        }
        
        /* Простое масштабирование методом ближайшего соседа */
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
        
        GlobalUnlock(hMemXOR_scaled);
        GlobalUnlock(hMemAND_scaled);
    }
    
    GlobalUnlock(hMemXOR);
    GlobalUnlock(hMemAND);
    
    /* Создаем иконку/курсор */
    if (bIcon)
    {
        if (needScale && (bmi->bmiHeader.biBitCount == 1 || bmi->bmiHeader.biBitCount == 4 || bmi->bmiHeader.biBitCount == 8))
        {
            result = CreateIcon(hinst, width, height,
                               bmi->bmiHeader.biPlanes,
                               bmi->bmiHeader.biBitCount,
                               lpAND_scaled,
                               lpXOR_scaled);
        }
        else
        {
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
            result = CreateCursor(hinst, hotspot.x, hotspot.y,
                                 width, height,
                                 lpAND_scaled,
                                 lpXOR_scaled);
        }
        else
        {
            result = CreateCursor(hinst, hotspot.x, hotspot.y,
                                 width, height,
                                 lpAND_flipped,
                                 lpXOR_flipped);
        }
    }
    
    /* Очистка памяти */
    if (hMemXOR) GlobalFree(hMemXOR);
    if (hMemAND) GlobalFree(hMemAND);
    if (hMemXOR_scaled) GlobalFree(hMemXOR_scaled);
    if (hMemAND_scaled) GlobalFree(hMemAND_scaled);
    
    return result;
}

/***********************************************************************
 *		LoadImage (USER.389)
 */
HANDLE WINAPI LoadImage(HINSTANCE hinst, LPCSTR name, UINT type, int cx, int cy, UINT flags)
{
	HGLOBAL handle;
	HRSRC hRsrc, hGroupRsrc;
	DWORD size;
	LPVOID data;

//	FUNCTION_START
// @TODO
/*
    if (!hinst || (flags & LR_LOADFROMFILE))
    {
        if (type == IMAGE_BITMAP)
            return HBITMAP_16( LoadImageA( 0, name, type, cx, cy, flags ));
        else
            return get_icon_16( LoadImageA( 0, name, type, cx, cy, flags ));
    }
*/
	if (!hinst) hinst = HInstanceDisplay;  /* Load OEM cursor/icon */

	hinst = GetExePtr(hinst);

    if (flags & LR_DEFAULTSIZE)
    {
        if (type == IMAGE_ICON)
        {
            if (!cx) cx = GetSystemMetrics(SM_CXICON);
            if (!cy) cy = GetSystemMetrics(SM_CYICON);
        }
        else if (type == IMAGE_CURSOR)
        {
            if (!cx) cx = GetSystemMetrics(SM_CXCURSOR);
            if (!cy) cy = GetSystemMetrics(SM_CYCURSOR);
        }
    }

	switch (type)
	{
#if 0
// Huh... Save and load bitmap is not good idea... Better rewrite to create from memory
    case IMAGE_BITMAP:
    {
        HBITMAP ret = 0;
        char *ptr;
        static const char prefixW[] = {'b','m','p',0};
        BITMAPFILEHEADER header;
        char path[MAX_PATH], filename[MAX_PATH];
        HANDLE file;

        filename[0] = 0;
        if (!(hRsrc = FindResource( hinst, name, (LPCSTR)RT_BITMAP ))) return 0;
        if (!(handle = LoadResource( hinst, hRsrc ))) return 0;
        if (!(ptr = LockResource( handle ))) goto done;
        size = SizeofResource( hinst, hRsrc );

        header.bfType = 0x4d42; /* 'BM' */
        header.bfReserved1 = 0;
        header.bfReserved2 = 0;
        header.bfSize = sizeof(header) + size;
        header.bfOffBits = 0;  /* not used by the 32-bit loading code */

        if (!GetTempPath( MAX_PATH, path )) goto done;
        if (!GetTempFileName( path, prefixW, 0, filename )) goto done;

        file = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
        if (file != INVALID_HANDLE_VALUE)
        {
            DWORD written;
            BOOL ok;
            ok = WriteFile( file, &header, sizeof(header), &written, NULL ) && (written == sizeof(header));
            if (ok) ok = WriteFile( file, ptr, size, &written, NULL ) && (written == size);
            CloseHandle( file );
            if (ok) ret = LoadImage( 0, filename, IMAGE_BITMAP, cx, cy, flags | LR_LOADFROMFILE );
        }
    done:
        if (filename[0]) DeleteFile( filename );
        FreeResource( handle );
        return ret;
    }
#endif
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
    hRsrc = FindResource(hinst, name, (LPCSTR)RT_BITMAP);
    if (!hRsrc) 
    {
        TRACE("Bitmap resource not found");
        return 0;
    }
    
    handle = LoadResource(hinst, hRsrc);
    if (!handle) 
    {
        TRACE("Error loading bitmap resource");
        return 0;
    }
    
    data = LockResource(handle);
    if (!data)
    {
        FreeResource(handle);
        return 0;
    }
    
    size = SizeofResource(hinst, hRsrc);
    
    /* Проверяем сигнатуру 'BM' (BITMAPFILEHEADER) */
    wSig = *((WORD FAR *)data);
    if (wSig == 0x4D42) /* 'BM' в little-endian */
    {
        data = ((BYTE FAR *)data) + 14; /* Пропускаем BITMAPFILEHEADER */
    }
    
    /* Получаем размер заголовка */
    dwData = (DWORD FAR *)data;
    headerSize = dwData[0];
    
    /* Поддерживаем как 12-байтный (OS/2), так и 40-байтный (Windows) формат */
    if (headerSize == 12) /* BITMAPCOREHEADER (OS/2) */
    {
        BITMAPCOREHEADER FAR *coreHeader = (BITMAPCOREHEADER FAR *)data;
        
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
            
            /* Смещение до пиксельных данных */
            pixelData = palette + colors * 3; /* OS/2 использует RGBTRIPLE (3 байта) */
        }
        else
        {
            colors = 0;
            pixelData = palette; /* Нет палитры */
        }
        
        /* Выравнивание по DWORD */
        if (((DWORD)pixelData - (DWORD)data) % 4 != 0)
        {
            pixelData += 4 - (((DWORD)pixelData - (DWORD)data) % 4);
        }
        
        /* Создаем временную структуру BITMAPINFO с Windows заголовком */
        tempInfoSize = sizeof(BITMAPINFOHEADER) + colors * sizeof(RGBQUAD);
        hTempInfo = GlobalAlloc(GMEM_FIXED, tempInfoSize);
        
        if (!hTempInfo)
        {
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
        }
        
        /* Создаем DDB из DIB */
        hdc = GetDC(0);
        if (hdc)
        {
            /* Для монохромных битмапов (1 bpp) - используем CreateBitmap */
            if (isMonochrome)
            {
                rowSize = ((width + 31) / 32) * 4;
                imageSize = rowSize * height;
                
                hBitmap = CreateBitmap(width, height, 1, 1, NULL);
                if (hBitmap && pixelData)
                {
                    SetBitmapBits(hBitmap, imageSize, pixelData);
                }
            }
            else
            {
                /* Для цветных битмапов используем CreateDIBitmap */
                hBitmap = CreateDIBitmap(hdc, 
                                       &winHeader,
                                       CBM_INIT,
                                       pixelData,
                                       tempInfo,
                                       DIB_RGB_COLORS);
            }
            
            ReleaseDC(0, hdc);
        }
        
        if (hTempInfo)
        {
            GlobalUnlock(hTempInfo);
            GlobalFree(hTempInfo);
        }
    }
    else if (headerSize == 40) /* BITMAPINFOHEADER (Windows) */
    {
        bmi = (BITMAPINFO FAR *)data;
        
        /* Проверяем корректность заголовка */
        if (bmi->bmiHeader.biSize != 40)
        {
            FreeResource(handle);
            return 0;
        }
        
        /* Windows 3.0 поддерживает только BI_RGB */
        if (bmi->bmiHeader.biCompression != 0)
        {
            FreeResource(handle);
            return 0;
        }
        
        /* Получаем размеры */
        width = bmi->bmiHeader.biWidth;
        height = bmi->bmiHeader.biHeight;
        if (height < 0) height = -height;
        
        /* Вычисляем смещение до пиксельных данных */
        bits = (BYTE FAR *)data + bmi->bmiHeader.biSize;
        
        /* Обрабатываем палитру */
        if (bmi->bmiHeader.biBitCount <= 8)
        {
            if (bmi->bmiHeader.biClrUsed > 0)
                colors = bmi->bmiHeader.biClrUsed;
            else
                colors = 1 << bmi->bmiHeader.biBitCount;
            
            bits += colors * sizeof(RGBQUAD);
        }
        else
        {
            colors = 0;
        }
        
        /* Выравнивание по DWORD */
        if (((DWORD)bits - (DWORD)data) % 4 != 0)
        {
            bits += 4 - (((DWORD)bits - (DWORD)data) % 4);
        }
        
        /* Создаем DDB из DIB */
        hdc = GetDC(0);
        if (hdc)
        {
            /* Для монохромных битмапов (1 bpp) - используем CreateBitmap */
            if (bmi->bmiHeader.biBitCount == 1)
            {
                rowSize = ((width + 31) / 32) * 4;
                imageSize = rowSize * height;
                
                hBitmap = CreateBitmap(width, height, 1, 1, NULL);
                if (hBitmap && bits)
                {
                    SetBitmapBits(hBitmap, imageSize, bits);
                }
            }
            else
            {
                /* Для цветных битмапов используем CreateDIBitmap */
                hBitmap = CreateDIBitmap(hdc, 
                                       (BITMAPINFOHEADER FAR *)&bmi->bmiHeader,
                                       CBM_INIT,
                                       bits,
                                       bmi,
                                       DIB_RGB_COLORS);
            }
            
            ReleaseDC(0, hdc);
        }
    }
    else
    {
        FreeResource(handle);
        return 0;
    }
    
    FreeResource(handle);
    
    /* Масштабирование при необходимости */
    if (hBitmap && (cx != 0 && cy != 0) && (cx != width || cy != height))
    {
        hScaledBmp = 0;
        hdcSrc = CreateCompatibleDC(0);
        hdcDst = CreateCompatibleDC(0);
        
        if (hdcSrc && hdcDst)
        {
            hOldSrc = SelectObject(hdcSrc, hBitmap);
            
            hScaledBmp = CreateCompatibleBitmap(hdcSrc, cx, cy);
            if (hScaledBmp)
            {
                hOldDst = SelectObject(hdcDst, hScaledBmp);
                
                StretchBlt(hdcDst, 0, 0, cx, cy,
                          hdcSrc, 0, 0, width, height,
                          SRCCOPY);
                
                SelectObject(hdcDst, hOldDst);
            }
            
            SelectObject(hdcSrc, hOldSrc);
        }
        
        if (hdcSrc) DeleteDC(hdcSrc);
        if (hdcDst) DeleteDC(hdcDst);
        
        if (hScaledBmp)
        {
            DeleteObject(hBitmap);
            hBitmap = hScaledBmp;
        }
    }
    
    return hBitmap;
}
    case IMAGE_ICON:
    case IMAGE_CURSOR:
    {
        HICON hIcon = 0;
        BYTE FAR *dir, FAR *bits;
        int id = 0;
                          //TRACE("%x\n\r", name);
	hRsrc = FindResource(hinst, name, (LPCSTR)(type == IMAGE_ICON ? RT_GROUP_ICON : RT_GROUP_CURSOR));
	if (!hRsrc) 
	{
		TRACE("Resource not found");
//		FUNCTION_END
		return 0;
	}
	hGroupRsrc = hRsrc;
        if (!(handle = LoadResource( hinst, hRsrc ))) return 0;
        if ((dir = LockResource( handle ))) id = LookupIconIdFromDirectory( dir, type == IMAGE_ICON );
        FreeResource( handle );
        if (!id) return 0;

        if (!(hRsrc = FindResource( hinst, MAKEINTRESOURCE(id),
                                      (LPCSTR)(type == IMAGE_ICON ? RT_ICON : RT_CURSOR) ))) return 0;

// @todo port shared icon
//        if ((flags & LR_SHARED) && (hIcon = find_shared_icon( hinst, hRsrc ) ) != 0) return hIcon;

        if (!(handle = LoadResource(hinst, hRsrc))) return 0;
        bits = LockResource(handle );
        size = SizeofResource(hinst, hRsrc);
//	TRACE("Create icon/cursor");

        hIcon = CreateIconFromResourceEx(hinst, bits, size, type == IMAGE_ICON, 0x00030000, cx, cy, flags );

        FreeResource( handle );

/* @todo port shared icons code from wine */
//        if (hIcon && (flags & LR_SHARED)) add_shared_icon( hinst, hRsrc, hGroupRsrc, hIcon );
//	FUNCTION_END
        return hIcon;
    }
    default:
        return 0;
    }

//	FUNCTION_END
	return 0;
}

/***********************************************************************
 *		LoadCursor (USER.173)
 */
HCURSOR WINAPI LoadCursor(HINSTANCE hInstance, LPCSTR name)
{
	HCURSOR res;
//	FUNCTION_START
//	TRACE("name=%x:%x", name);
    	res=LoadImage(hInstance, name, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
//	FUNCTION_END
	return res;
}

/***********************************************************************
 *		LoadIcon (USER.174)
 */
HICON WINAPI LoadIcon(HINSTANCE hInstance, LPCSTR name)
{
	HICON res;
//	FUNCTION_START
	res=LoadImage(hInstance, name, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
//	FUNCTION_END
	return res;
}

/**********************************************************************
 *		LoadBitmap (USER.175)
 */
HBITMAP WINAPI LoadBitmap(HINSTANCE hInstance, LPCSTR name)
{
	HBITMAP res;
//	FUNCTION_START
	res=LoadImage(hInstance, name, IMAGE_BITMAP, 0, 0, 0);
//	FUNCTION_END
	return res;
}
