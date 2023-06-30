/*
 * Protect mode functions
 *
 * Copyright 2023 osFree
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
#include <dpmi.h>

#include "win_private.h"

extern unsigned _dos_getumblink(void);
#pragma aux _dos_getumblink = \
		"mov ax,5802h" \
		"int 21h " \
		"xor ah,ah" \
		value	[ax];

extern unsigned _dos_getmemstrategy(void);
#pragma aux _dos_getmemstrategy = \
		"mov ax,5800h" \
		"int 21h" \
		"xor ah,ah" \
		value	[ax];

extern void _dos_setumblink(unsigned);
#pragma aux _dos_setumblink = \
		"mov ax,5803h" \
		"int 21h" \
		parm	[bx];

extern void _dos_setmemstrategy(unsigned);
#pragma aux _dos_setmemstrategy = \
		"mov ax,5801h" \
		"int 21h" \
		parm	[bx];

extern unsigned _dos_allocmem(unsigned size);
#pragma aux _dos_allocmem = \
		"mov ah,48h" \
		"int 21h" \
		value	[ax]	\
		parm	[bx];

extern int DPMI_Switch(int m, void far * switche)
{
	SetES(_dos_allocmem(m));

	__asm
	{
		xor ax,ax
		stc
		call switche
		sbb ax,ax
	};
}

void SwitchToPMode()
{
	init_info ii;
	unsigned int mem;
	void(far * switchentry)(void);
	unsigned umblink;
	unsigned strat;
	
	FUNCTIONSTART;

	strat=_dos_getmemstrategy();
	umblink=_dos_getumblink();

	_dos_setumblink(1);
	_dos_setmemstrategy(0x81);	// set "fit best" strategy, first high, then low

	__asm
	{
		push cs				; Set data segment to code segment
		pop ds
	};

	printf("DPMI Installation check\r\n");

	switchentry=DPMI_Init(&ii);
	if (!switchentry)
	{
		printf("DPMI not found\r\n");
	} else {
		printf("DPMI found\r\n");
		printf("DPMI version:\t\t%d.%d\r\n", ii.major_version, ii.minor_version);
		printf("CPU type: \t\t%d (", ii.processor_type);
		if(ii.processor_type == 2) printf("80286");
		else if(ii.processor_type == 3) printf("80386");
		else if(ii.processor_type == 4) printf("80486");
		else if(ii.processor_type == 5) printf("Pentium");
		else if(ii.processor_type == 6) printf("Pentium Pro");
		printf(")\r\n32-bit client support:\t%s\r\n", (ii.flags && 1)?"Present":"Not present");
		printf("DPMI Host memory:\t%d bytes\r\n", ii.host_mem*16);
		printf("Switch to Protected mode entry: %x:%x\r\n", SELECTOROF(ii.switchentry), OFFSETOF(ii.switchentry));
		if (DPMI_Switch(ii.host_mem, ii.switchentry))
		{
			printf("Failed to switch to Protected mode\r\n");
		} else {
			printf("KERNEL in Protected mode\r\n");
			if (DPMI_VendorEntry("MS-DOS"))
			{
				printf("MS-DOS API Translation present\r\n");
			} else {
				printf("MS-DOS API Translation not present\r\n");
			};
			wKernelDS=GetDS();
			TH_TOPPDB=GetES();
			wCurPSP=GetES();
		}
	}

	_dos_setumblink(strat);
	_dos_setmemstrategy(umblink);
	FUNCTIONEND;
}
