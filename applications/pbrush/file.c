/*
 * file.c – Загрузка и сохранение BMP (C89, Win16)
 * Исправлены предупреждения GlobalFree и логика копирования.
 */

#include "file.h"
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

BOOL LoadBMP(const char *szFile, HDC hdcMem, int *pWidth, int *pHeight)
{
    OFSTRUCT of;
    HFILE hf;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    int w, h, bitCount, colors;
    int stride, imageSize;
    LPBYTE pBits;
    HDC hdc;
    HBITMAP hbm;
    HDC hdcMemTmp;
    int x, y;
    RGBQUAD pal[256];
    COLORREF cr;
    BYTE byte, idx;

    hf = OpenFile(szFile, &of, OF_READ);
    if (hf == HFILE_ERROR) return FALSE;

    if (_lread(hf, (LPSTR)&bfh, sizeof(bfh)) != sizeof(bfh)) { _lclose(hf); return FALSE; }
    if (_lread(hf, (LPSTR)&bih, sizeof(bih)) != sizeof(bih)) { _lclose(hf); return FALSE; }
    if (bfh.bfType != 0x4D42) { _lclose(hf); return FALSE; }

    w = bih.biWidth;
    h = bih.biHeight;
    if (w <= 0 || h <= 0 || w > 2048 || h > 2048) { _lclose(hf); return FALSE; }
    bitCount = bih.biBitCount;
    if (bitCount != 1 && bitCount != 4 && bitCount != 8 && bitCount != 24) { _lclose(hf); return FALSE; }

    colors = (bitCount <= 8) ? (1 << bitCount) : 0;
    if (colors > 0)
    {
        if (_lread(hf, (LPSTR)pal, colors * sizeof(RGBQUAD)) != (UINT)(colors * sizeof(RGBQUAD)))
        {
            _lclose(hf);
            return FALSE;
        }
    }

    stride = ((w * bitCount + 31) / 32) * 4;
    imageSize = stride * h;
    pBits = (LPBYTE)GlobalAlloc(GPTR, imageSize);
    if (!pBits) { _lclose(hf); return FALSE; }
    if (_lread(hf, (LPSTR)pBits, imageSize) != imageSize)
    {
        GlobalFree((HGLOBAL)pBits);
        _lclose(hf);
        return FALSE;
    }
    _lclose(hf);

    hdc = GetDC(NULL);
    hbm = CreateCompatibleBitmap(hdc, w, h);
    hdcMemTmp = CreateCompatibleDC(hdc);
    SelectObject(hdcMemTmp, hbm);

    for (y = 0; y < h; y++)
    {
        LPBYTE pRow = pBits + (h - 1 - y) * stride;
        for (x = 0; x < w; x++)
        {
            if (bitCount == 24)
            {
                cr = RGB(pRow[x * 3 + 2], pRow[x * 3 + 1], pRow[x * 3]);
                SetPixel(hdcMemTmp, x, y, cr);
            }
            else if (bitCount == 8)
            {
                idx = pRow[x];
                cr = RGB(pal[idx].rgbRed, pal[idx].rgbGreen, pal[idx].rgbBlue);
                SetPixel(hdcMemTmp, x, y, cr);
            }
            else if (bitCount == 4)
            {
                byte = pRow[x >> 1];
                idx = (x & 1) ? (byte & 0x0F) : ((byte >> 4) & 0x0F);
                cr = RGB(pal[idx].rgbRed, pal[idx].rgbGreen, pal[idx].rgbBlue);
                SetPixel(hdcMemTmp, x, y, cr);
            }
            else /* bitCount == 1 */
            {
                byte = pRow[x >> 3];
                idx = (byte >> (7 - (x & 7))) & 1;
                cr = RGB(pal[idx].rgbRed, pal[idx].rgbGreen, pal[idx].rgbBlue);
                SetPixel(hdcMemTmp, x, y, cr);
            }
        }
    }

    /* Копируем загруженное изображение в буфер холста */
    BitBlt(hdcMem, 0, 0, w, h, hdcMemTmp, 0, 0, SRCCOPY);

    DeleteDC(hdcMemTmp);
    ReleaseDC(NULL, hdc);
    GlobalFree((HGLOBAL)pBits);

    *pWidth = w;
    *pHeight = h;
    return TRUE;
}

BOOL SaveBMP(const char *szFile, HDC hdcMem, int width, int height,
             const COLORREF *pPalette, int nColors)
{
    int bitCount, colors;
    int stride, imageSize;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    RGBQUAD *pal;
    LPBYTE pBits;
    int i, x, y;
    COLORREF cr;
    long dr, dg, db, dist, mindist;
    int idx;
    BYTE byte;
    OFSTRUCT of;
    HFILE hf;

    if (nColors <= 2)
        bitCount = 1;
    else if (nColors <= 16)
        bitCount = 4;
    else
        bitCount = 8;

    colors = nColors;
    stride = ((width * bitCount + 31) / 32) * 4;
    imageSize = stride * height;

    memset(&bfh, 0, sizeof(bfh));
    bfh.bfType = 0x4D42;
    bfh.bfSize = sizeof(bfh) + sizeof(bih) + colors * sizeof(RGBQUAD) + imageSize;
    bfh.bfOffBits = sizeof(bfh) + sizeof(bih) + colors * sizeof(RGBQUAD);

    memset(&bih, 0, sizeof(bih));
    bih.biSize = sizeof(bih);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = bitCount;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = imageSize;
    bih.biClrUsed = colors;
    bih.biClrImportant = colors;

    pal = (RGBQUAD *)GlobalAlloc(GPTR, colors * sizeof(RGBQUAD));
    if (!pal) return FALSE;
    for (i = 0; i < colors; i++)
    {
        pal[i].rgbRed   = GetRValue(pPalette[i]);
        pal[i].rgbGreen = GetGValue(pPalette[i]);
        pal[i].rgbBlue  = GetBValue(pPalette[i]);
    }

    pBits = (LPBYTE)GlobalAlloc(GPTR, imageSize);
    if (!pBits) { GlobalFree((HGLOBAL)pal); return FALSE; }

    for (y = 0; y < height; y++)
    {
        LPBYTE pRow = pBits + (height - 1 - y) * stride;
        for (x = 0; x < width; x++)
        {
            cr = GetPixel(hdcMem, x, y);
            mindist = 0x7FFFFFFF;
            idx = 0;
            for (i = 0; i < colors; i++)
            {
                dr = (long)GetRValue(cr) - (long)GetRValue(pPalette[i]);
                dg = (long)GetGValue(cr) - (long)GetGValue(pPalette[i]);
                db = (long)GetBValue(cr) - (long)GetBValue(pPalette[i]);
                dist = dr * dr + dg * dg + db * db;
                if (dist < mindist)
                {
                    mindist = dist;
                    idx = i;
                }
            }
            if (bitCount == 8)
            {
                pRow[x] = (BYTE)idx;
            }
            else if (bitCount == 4)
            {
                byte = pRow[x >> 1];
                if (x & 1)
                    byte = (byte & 0xF0) | (idx & 0x0F);
                else
                    byte = (byte & 0x0F) | ((idx << 4) & 0xF0);
                pRow[x >> 1] = byte;
            }
        }
    }

    hf = OpenFile(szFile, &of, OF_CREATE | OF_WRITE);
    if (hf == HFILE_ERROR)
    {
        GlobalFree((HGLOBAL)pBits);
        GlobalFree((HGLOBAL)pal);
        return FALSE;
    }
    _lwrite(hf, (LPSTR)&bfh, sizeof(bfh));
    _lwrite(hf, (LPSTR)&bih, sizeof(bih));
    _lwrite(hf, (LPSTR)pal, colors * sizeof(RGBQUAD));
    _lwrite(hf, (LPSTR)pBits, imageSize);
    _lclose(hf);

    GlobalFree((HGLOBAL)pBits);
    GlobalFree((HGLOBAL)pal);
    return TRUE;
}
