/*
 * File handling functions
 *
 * Copyright 1993 John Burton
 * Copyright 1996 Alexandre Julliard
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

#include <win16.h>


/***********************************************************************
 *           GetWindowsDirectory   (KERNEL.134)
 */
UINT WINAPI GetWindowsDirectory( LPSTR path, UINT count )
{
    lstrcpyn(path, "C:\\WINDOWS", count);
    return lstrlen(path);
}


/***********************************************************************
 *           GetSystemDirectory   (KERNEL.135)
 */
UINT WINAPI GetSystemDirectory(LPSTR path, UINT count)
{
    static const char system16[] = "\\SYSTEM";
    char windir[255]; //MAX_PATH
    UINT len;

    len = GetWindowsDirectory(windir, sizeof(windir) - sizeof(system16) + 1) + sizeof(system16);
    if (count >= len)
    {
        lstrcpy(path, windir);
        lstrcat(path, system16);
        len--;  /* space for the terminating zero is not included on success */
    }
    return len;
}

#if 0
_lopen proc far pascal
	@loadbx
	push ds
	@loadparm 0,al
	@loadparm 2,dx
	@loadparm 4,ds
	@OpenFil
	jnc @F
	mov ax,-1
@@:
	pop ds
	@return 6
_lopen endp
#endif

void WINAPI Dos3Call();

#define MOV_BX(x) __asm {mov dx, word ptr x}
#define MOV_DX(x) __asm {mov dx, word ptr x}
#define MOV_DS(x) __asm {mov ds, word ptr x}
#define MOV_AL(x) __asm {mov al, byte ptr x}
#define MOV_AH_CONST(x) __asm {mov ah, x}

HFILE WINAPI _lopen(LPCSTR f, int a)
{
	MOV_AL(a);
	MOV_DX(f);
	MOV_DS(f+2);
	MOV_AH_CONST(0);
    Dos3Call();
	__asm {
	jnc exit
	mov ax,-1
exit:
	}
}

#if 0
_lcreat proc far pascal
	@loadbx
	push ds
	@loadparm 0,al
	@loadparm 2,dx
	@loadparm 4,ds
	@MakFil
	jnc @F
	mov ax,-1
@@:
	pop ds
	@return 6
_lcreat endp
#endif


HFILE WINAPI _lcreat(LPCSTR f, int a)
{
	MOV_AL(a);
	MOV_DX(f);
	MOV_DS(f+2);
	MOV_AH_CONST(0);
    Dos3Call();
	__asm {
	jnc exit
	mov ax,-1
exit:
	}
}

#if 0
_lclose proc far pascal
	@loadbx
	@loadparm 0,bx
	@ClosFil
	jnc @F
	mov ax,-1
@@:
	@return 2
_lclose endp
#endif

HFILE WINAPI _lclose(HFILE h)
{
	MOV_BX(h);
	MOV_AH_CONST(0);
    Dos3Call();
	__asm {
	jnc exit
	mov ax,-1
exit:
	}
}
