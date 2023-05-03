
#include <win16.h>

extern void InitKernel();

extern char IsVMM();
// 486 by default
#pragma aux IsVMM        = \
        "mov    ax,01600h"       \
	"int    31h"\
	"cmp	al, 3"\
	"jz	exit"\	
	"xor	al, al"\
	"exit:"\
        value [al];

extern char DPMI_GetCPU();
// 486 by default
#pragma aux DPMI_GetCPU        = \
        "mov    ax,0300h"          \
	"int    31h"\
	"jnc	exit"\
	"mov	cl,4"\	
	"exit:"\
        value [cl];

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

extern ENTRY pascal eWinFlags;

void WINAPI SetWinFlags()
{
  //@todo WF_PMODE set for non 8086 cpu kernel version
  eWinFlags.wOfs= (
			  GetFPU()?WF_80x87:0) 
			| (1<<(DPMI_GetCPU()-1)) 
			| (IsVMM()?(WF_PMODE | WF_ENHANCED):(WF_PMODE | WF_STANDARD)
		) ;
}

void WINAPI KernelMain(void)
{
  InitKernel();
}
