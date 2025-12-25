/*
 * DOS API functions
 *
 * Copyright 2025 Yuri Prokushev
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
 *
 */

#include <windows.h>
#include <win_private.h>

/***********************************************************************
 *           DOS3Call         (KERNEL.102)
 */
void __declspec(naked) __pascal __far Dos3Call()
{
    _asm
    {
        int 21h
        ret
    }
}

/***********************************************************************
 *           NetBIOSCall      (KERNEL.103)
 */
void __declspec(naked) __pascal __far NetBiosCall()
{
    _asm
    {
        int 5ch
        ret
    }
}

/***********************************************************************
 *		EnableDos (KERNEL.41)
 */
void WINAPI EnableDOS(void)
{
    FUNCTIONSTART;
    FUNCTIONEND;
    return ;
}

/***********************************************************************
 *		DisableDos (KERNEL.42)
 */
void WINAPI DisableDOS(void)
{
    FUNCTIONSTART;
    FUNCTIONEND;
    return ;
}
