/*
 * DISPLAY.DRV interface
 *
 * Copyright 2026 Yuri Prokushev
 *
 * This is interface to DISPLAY.DRV. For now it is support only Windows 3.0 things.
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


#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <windows.h>

VOID DISPLAY_Init();

/* Получить бинарные данные цветов по умолчанию из драйвера */
const COLORREF* DISPLAY_GetSysColorDefaultsBinary(void);

/* Получить количество цветов */
int DISPLAY_GetSysColorCount(void);

#endif
