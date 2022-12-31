;
; osFree Windows Kernel 
;
; - Switching to protect mode 286 CPU version
; - Selector functions
;
; Matt Pietrek describes Enchanced mode version of Selector functions. It differs from
; Standard mode functions. It seems, standard mode doesn't handle own list of selectors.
;


		; MacroLib
;		include bios.inc
		include dos.inc
		include dpmi.inc

		; Kernel macros
		include ascii.inc
		include kernel.inc
		include debug.inc


.286
_DATA segment

externdef	wKernelDS:word
externdef	wCurPSP:word
externdef	TH_TOPPDB:word

_DATA ends

_ITEXT segment word public 'DATA'	;use 'DATA' (OPTLINK bug)
_ITEXT ends

DGROUP group _TEXT,CCONST,_DATA,_ITEXT

	assume CS:DGROUP
	assume DS:DGROUP
	assume SS:NOTHING
	assume ES:DGROUP

_TEXT segment

externdef	szNoDPMI:near
externdef	szLF:near
externdef	szDOSstr:near
externdef	errstr2:near
externdef	errstr3:near
externdef	blksize: near

externdef DumpDPMIInfo_: near

changememstrat proc
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
	ret
changememstrat endp

restorememstrat proc
	mov bx,word ptr [blksize+2]
	mov ax,5801h			  ;memory alloc strat restore
	int 21h
	mov bx,word ptr [blksize+0]
	mov ax,5803h			  ;umb link restore
	int 21h
	ret
restorememstrat endp

;wDPMIFlg	dw ?			;DPMI init call CX flags (CL=CPU[2,3,4],CH=??)
;wDPMIVer	dw ?			;DPMI init call DX Flags (DPMI version)

;*** SwitchToPMode
;--- returns C on errors, bx->error msg 

SwitchToPMode proc
	call changememstrat

	@trace_s <lf,"------------------------------------",lf>
	@trace_s <"KERNEL now in real mode, PDB=">
	@trace_w es
	@trace_s <",CS=">
	@trace_w cs
	@trace_s <",SS=">
	@trace_w ss
	@trace_s <",DS=">
	@trace_w ds
	@trace_s <lf>

	push cs				; Set data segment to code segment
	pop ds
	call DumpDPMIInfo_
	mov cs:[wKernelDS],ds

	mov bp,offset cs:szNoDPMI  ;message "no dpmi server"

	call restorememstrat

	mov [TH_TOPPDB],es			;psp
if 1;?USE1PSP
	mov [wCurPSP],es
endif
	@trace_s <lf,"------------------------------------",lf>
	@trace_s <"KERNEL now in protected mode, PDB=">
	@trace_w es
	@trace_s <",CS=">
	@trace_w cs
	@trace_s <",SS=">
	@trace_w ss
	@trace_s <",DS=">
	@trace_w ds
	@trace_s <lf>

	ret
SwitchToPMode endp



_TEXT	ends
	end
