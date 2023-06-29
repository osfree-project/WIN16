#include <windows.h>
#include <dpmi.h>

#include "win_private.h"

extern void changememstrat(void)
{
__asm
	{
		mov ax,5802h			 ;save umb link state
		int 21h
		xor ah,ah
		mov word ptr [blksize+0],ax
		mov ax,5800h			 ;save memory alloc strategie
		int 21h
		xor ah,ah
		mov word ptr [blksize+2],ax
		mov ax,5803h			 ;set umb link state
		mov bx,0001h
		int 21h
		mov ax,5801h			 ;set "fit best" strategy
		mov bx,0081h			 ;first high, then low
		int 21h
	};
}

extern void restorememstrat(void)
{
__asm
	{
		mov bx,word ptr [blksize+2]
		mov ax,5801h			  ;memory alloc strat restore
		int 21h
		mov bx,word ptr [blksize+0]
		mov ax,5803h			  ;umb link restore
		int 21h
	};
}

extern int DPMI_Switch(int m, void far * switche)
{
__asm
	{
		mov bx, m
		mov ah,48h
		int 21h
		mov es,ax
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

	changememstrat();

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
		}
		if (DPMI_VendorEntry("MS-DOS"))
		{
			printf("MS-DOS API Translation present\r\n");
		} else {
			printf("MS-DOS API Translation not present\r\n");
		};
	}

	__asm
	{
		mov cs:[wKernelDS],ds
	};
	restorememstrat();

	__asm
	{
		mov [TH_TOPPDB],es			;psp
		mov [wCurPSP],es
	}
}
