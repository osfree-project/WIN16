/*
 * KERNEL32 thunks and other undocumented stuff
 *
 * Copyright 1996, 1997 Alexandre Julliard
 * Copyright 1997, 1998 Marcus Meissner
 * Copyright 1998       Ulrich Weigand
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

// @todo not checked is correctly registers stored. Need to compare with Windows
// Most probably ip and sp stored incorrectly

#include <windows.h>

#include "win_private.h"

/**********************************************************************
 *	     Catch    (KERNEL.55)
 *
 * Real prototype is:
 *   INT16 WINAPI Catch( LPCATCHBUF lpbuf );
 */
int WINAPI Catch( LPCATCHBUF lpbuf )
{
	FUNCTIONSTART;
    /* Note: we don't save the current ss, as the catch buffer is */
    /* only 9 words long. Hopefully no one will have the silly    */
    /* idea to change the current stack before calling Throw()... */

    /* Windows uses:
     * lpbuf[0] = ip
     * lpbuf[1] = cs
     * lpbuf[2] = sp
     * lpbuf[3] = bp
     * lpbuf[4] = si
     * lpbuf[5] = di
     * lpbuf[6] = ds
     * lpbuf[7] = unused
     * lpbuf[8] = ss
     */

    __asm {
      pop ax			// is this correct?
      push ax
      mov word ptr lpbuf[0], ax
      mov ax, cs
      mov word ptr lpbuf[1], ax
      mov ax, sp
    /* Windows pushes 4 more words before saving sp */
      sub ax, 8                // is this correct?
      mov word ptr lpbuf[3], ax
      mov word ptr lpbuf[3], bp
      mov word ptr lpbuf[4], si
      mov word ptr lpbuf[5], di
      mov ax, ds
      mov word ptr lpbuf[6], ax
      xor ax, ax
      mov word ptr lpbuf[7], ax
      mov ax, ss
      mov word ptr lpbuf[8], ax
    }
	FUNCTIONEND;
    return 0;
}


/**********************************************************************
 *	     Throw    (KERNEL.56)
 *
 * Real prototype is:
 *   INT16 WINAPI Throw( LPCATCHBUF lpbuf, INT16 retval );
 */
void WINAPI Throw(CATCHBUF const far *  lpbuf, int retval)
{
	FUNCTIONSTART;
//    context->Eax = (context->Eax & ~0xffff) | (WORD)retval;


//    context->Eip = lpbuf[0];
//    context->SegCs  = lpbuf[1];
//    context->Esp = lpbuf[2] + 4 * sizeof(WORD) - sizeof(WORD) /*extra arg*/;
//    context->Ebp = lpbuf[3];
//    context->Esi = lpbuf[4];
//    context->Edi = lpbuf[5];
//    context->SegDs  = lpbuf[6];

//    if (lpbuf[8] != context->SegSs)
//        ERR("Switching stack segment with Throw() not supported; expect crash now\n" );
	FUNCTIONEND;
}
