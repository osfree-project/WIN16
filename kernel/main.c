
#include <windows.h>

#include <dpmi.h>


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

typedef struct tagENTRY
{
	BYTE	bSegm;
	WORD	wOfs;
} ENTRY, * PENTRY, FAR * LPENTRY;

extern ENTRY pascal near eWinFlags;

/***********************************************************************
 *          GetWinFlags   (KERNEL.132)
 */
DWORD WINAPI __loadds GetWinFlags(void)
{
  return eWinFlags.wOfs;
}

void WINAPI __loadds SetWinFlags()
{
  //@todo WF_PMODE set for non 8086 cpu kernel version
  eWinFlags.wOfs= (
			  (GetFPU()?WF_80x87:0) 
			| (1<<(DPMI_GetCPU()-1)) 
			| WF_PMODE
			| (IsVMM()?WF_ENHANCED:WF_STANDARD)
		) ;
}

void WINAPI KernelMain(void)
{
	// Initialize WinFlags
	SetWinFlags();
	// Initialize Kernel Module
	InitKernel();
}
