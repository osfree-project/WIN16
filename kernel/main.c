
#include <windows.h>
#include <dpmi.h>

#include "win_private.h"

extern void InitKernel();

extern char IsVMM();
#pragma aux IsVMM        = \
	"mov    ax,01600h"       \
	"int    31h" \
	"cmp	al, 3" \
	"jz	exit" \
	"xor	al, al" \
	"exit:" \
	value [al];

extern char GetFPU();
#pragma aux GetFPU        = \
	"int    11h"\
	"and	al,2"\
	"exit:" \
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
	FUNCTIONSTART;
	//@todo WF_PMODE set for non 8086 cpu kernel version
	eWinFlags.wOfs= (
			  (GetFPU()?WF_80x87:0) 
			| (1<<(DPMI_GetCPU()-1)) 
			| WF_PMODE
			| (IsVMM()?WF_ENHANCED:WF_STANDARD)
		) ;
	FUNCTIONEND;
}

#if 0
;*** read command line parameter
;*** will be called for 1. task only
;*** set exec parameter block (int 21,4b)
;*** RC: Carry if error
;*** else: module name in szPgmName
;***	   parameter block in ParmBlk

GetPgmParms proc uses ds
	push ds
	pop es				;es=DGROUP
	@trace_s <"GetPgmParms enter",lf>

	mov ds,[TH_TOPPDB]
	mov si,0080h
	mov di,offset szPgmName
	sub cx,cx
	mov cl,[si] 		;get parameter line
	inc si
	jcxz error
	mov ah,0
nextws:
	lodsb
  if ?32BIT
	cmp al,'-'
	jnz @F
	mov ah,al
	jmp skipcharx
@@:
  endif
	cmp al,' '			;skip spaces
	jnz parmfound
skipcharx:
	loop nextws
error:
	mov ax,offset errstr8	;"filename missing or invalid"
	stc
	ret
parmfound:
	dec si
	mov dl,0
nextchar:
	lodsb
	cmp al,'"'
	jnz @F
	xor ah,1
	jmp skipchar
@@:
	test ah,1
	jnz @F
	cmp al,' '
	jz copydone	   ;copy is done
@@:
	cmp al,'.'
	jnz @F
	inc dl
@@:
	stosb
	cmp al,'/'
	jz @F
	cmp al,'\'
	jnz skipchar
@@:
	mov dl,0
skipchar:
	loop nextchar
copydone:
	test ah,1
	jnz error
	and dl,dl		;file extension supplied? 
	jnz @F
	@trace_s <"'.EXE' added to module name",lf>
	mov ax,'E.'
	stosw
	mov ax,'EX'
	stosw
@@:
	mov al,00
	stosb

;------------------- copy rest of parameter line to psp cmd tail

	push es
	push ds
	pop es
	mov di, 80h
	push di
	mov al,cl
	stosb
	dec si
	inc cl			; copy 0D at least
	rep movsb
	pop si
	pop es
gpp_1:

	mov di,offset ParmBlk
if ?32BIT
	movzx eax,si
	stosd				;cmdline
	mov ax,ds
	stosd
	xor eax,eax 		;fcb1+fcb2
	stosd
	stosd
	stosd
	stosd
else
	xor ax,ax
	stosw				;environment (nur bei 16 Bit)
	mov ax,si
	stosw				;cmdline
	mov ax,ds
	stosw
	xor ax,ax			;fcb1+fcb2
	stosw
	stosw
	stosw
	stosw
endif
	clc
	ret
GetPgmParms endp
#endif

void WINAPI GetPgmParms()
{
	LPSTR cmd=MAKELP(TH_TOPPDB, 0x80);

	FUNCTIONSTART;

	if (cmd[0])
	{
		cmd++;
		while (*cmd==' ') cmd++;
	}
	FUNCTIONEND;
}

void WINAPI StartProgman(void)
{
	char buffer[100];

	FUNCTIONSTART;

	GetPrivateProfileString("boot", "shell", "", buffer, sizeof(buffer), "SYSTEM.INI");
	printf("shell=%s\r\n", buffer);

	FUNCTIONEND;
}

void WINAPI KernelMain(void)
{
	FUNCTIONSTART;
	// Initialize WinFlags
	SetWinFlags();
	// Initialize Kernel Module
	InitKernel();
	// Parse command line
	GetPgmParms();
	// Execute shell
	StartProgman();
	FUNCTIONEND;
}
