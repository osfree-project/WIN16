/*!

   (c) ReactOS project
   (c) osFree Project 2002-2026, <https://www.osFree.org>
 
   SPDX-License-Identifier: LGPL-2.1-or-later

*/
/*
 * clipboard.c – Работа с буфером обмена (C89, Win16)
 * Исправлено: ручное копирование HBITMAP вместо CopyBitmap.
 */

#include "clipboard.h"

BOOL Clipboard_Copy(HBITMAP hbm)
{
    if (!hbm) return FALSE;
    if (!OpenClipboard(NULL)) return FALSE;
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hbm);
    CloseClipboard();
    return TRUE;
}

HBITMAP Clipboard_Paste(void)
{
    HBITMAP hbmClip, hbmCopy;
    HDC hdc, hdcSrc, hdcDst;
    BITMAP bm;
    HBITMAP hOldSrc, hOldDst;

    if (!OpenClipboard(NULL)) return NULL;

    hbmClip = (HBITMAP)GetClipboardData(CF_BITMAP);
    if (!hbmClip)
    {
        CloseClipboard();
        return NULL;
    }

    /* Получаем размеры исходного битмапа */
    GetObject(hbmClip, sizeof(BITMAP), &bm);

    /* Создаём копию */
    hdc = GetDC(NULL);
    hdcSrc = CreateCompatibleDC(hdc);
    hdcDst = CreateCompatibleDC(hdc);
    hbmCopy = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);

    hOldSrc = SelectObject(hdcSrc, hbmClip);
    hOldDst = SelectObject(hdcDst, hbmCopy);
    BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
    SelectObject(hdcSrc, hOldSrc);
    SelectObject(hdcDst, hOldDst);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    ReleaseDC(NULL, hdc);

    CloseClipboard();
    return hbmCopy;
}
