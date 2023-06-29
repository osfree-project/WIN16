
#include <windows.h>
#include <dpmi.h>

#include "win_private.h"

extern void InitKernel();

extern char IsVMM();
#pragma aux IsVMM        = \
        "mov    ax,01600h"       \
	"int    31h"\
	"cmp	al, 3"\
	"jz	exit"\	
	"xor	al, al"\
	"exit:"\
        value [al];

extern char GetFPU();
// 486 by default
#pragma aux GetFPU        = \
	"int    11h"\
	"and	al,2"\
	"exit:"\
        value [al];

#pragma pack (1)
typedef struct tagENTRY
{
	BYTE	bSegm;
	WORD	wOfs;
} ENTRY, * PENTRY, FAR * LPENTRY;

extern ENTRY pascal near eWinFlags;

void WINAPI Copyright()
{
	printf("\r\nosFree Windows Kernel version 0.1\r\n" 
		   "Copyright (C) 2022-23 osFree\r\n"
		   "Based on HX DPMI loader, Wine and TWIN\r\n"
		   "Copyright (C) 1993-2022 Japheth\r\n"
		   "Copyright (C) 1993-2022 the Wine project authors\r\n"
		   "Copyright (C) 1997 Willows Software, Inc.\r\n"
		   "\r\n");
}

/***********************************************************************
 *          GetWinFlags   (KERNEL.132)
 */
DWORD WINAPI GetWinFlags(void)
{
  return eWinFlags.wOfs;
}

void WINAPI SetWinFlags()
{
  printf("enter SetWinFlags\r\n");
  //@todo WF_PMODE set for non 8086 cpu kernel version
  eWinFlags.wOfs= (
			  (GetFPU()?WF_80x87:0) 
			| (1<<(DPMI_GetCPU()-1)) 
			| WF_PMODE
			| (IsVMM()?WF_ENHANCED:WF_STANDARD)
		) ;
  printf("exit SetWinFlags\r\n");
}

void WINAPI KernelMain(void)
{
	printf("enter KernelMain\r\n");
	// Initialize WinFlags
	SetWinFlags();
	// Initialize Kernel Module
	InitKernel();
	printf("exit KernelMain\r\n");
}
