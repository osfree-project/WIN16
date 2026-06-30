/*
 * clipboard.h – Работа с буфером обмена (C89, Win16)
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <windows.h>

BOOL Clipboard_Copy(HBITMAP hbm);
HBITMAP Clipboard_Paste(void);

#endif
