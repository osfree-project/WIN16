/*
 * file.h – Загрузка и сохранение BMP (C89, Win16)
 */

#ifndef FILE_H
#define FILE_H

#include <windows.h>

BOOL LoadBMP(const char* szFile, HDC hdcMem, int* pWidth, int* pHeight);
BOOL SaveBMP(const char* szFile, HDC hdcMem, int width, int height,
             const COLORREF* pPalette, int nColors);

#endif
