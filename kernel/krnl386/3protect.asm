;
; osFree Windows Kernel 
;
; - Switching to protect mode 386 CPU version
;
; Matt Pietrek describes Enchanced mode version of Selector functions. It differs from
; Standard mode functions. It seems, standard mode doesn't handle own list of selectors.
;


		; MacroLib
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

DGROUP group _TEXT,CCONST,_DATA

	assume CS:DGROUP
	assume DS:DGROUP
	assume SS:NOTHING
	assume ES:DGROUP

_TEXT segment

externdef	szLF:near
externdef	szDOSstr:near
externdef	errstr2:near
externdef	errstr3:near
externdef	blksize: near

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

;*** SwitchToPMode

SwitchToPMode proc
	call changememstrat

	push cs				; Set data segment to code segment
	pop ds
	
	externdef DumpDPMIInfo_: near
	call DumpDPMIInfo_

	@trace_s <lf,"------------------------------------",lf>
	
	mov cs:[wKernelDS],ds
	@trace_s <lf,"------------------------------------",lf>

	call restorememstrat
	@trace_s <lf,"------------------------------------",lf>

	mov [TH_TOPPDB],es			;psp
	mov [wCurPSP],es
	@trace_s <lf,"------------------------------------",lf>
	ret
SwitchToPMode endp



_TEXT	ends
	end
