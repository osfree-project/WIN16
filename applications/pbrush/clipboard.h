/*!

   (c) ReactOS project
   (c) osFree Project 2002-2026, <https://www.osFree.org>
 
   SPDX-License-Identifier: LGPL-2.1-or-later

*/
/*
 * clipboard.h – Работа с буфером обмена (C89, Win16)
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <windows.h>

BOOL Clipboard_Copy(HBITMAP hbm);
HBITMAP Clipboard_Paste(void);

#endif
