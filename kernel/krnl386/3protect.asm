;
; osFree Windows Kernel
;
; - Switching to protect mode 386 CPU version
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
; Public procedures
GetSelectorLimit proto far pascal ulSelector: WORD
; Local procedures
Get_Physical_Address proto near pascal theSelector: WORD

_DATA segment

externdef	wKernelDS:word
externdef	wCurPSP:word
externdef	TH_TOPPDB:word
externdef	GDTDsc:word

_DATA ends



_TEXT segment

externdef	szNoDPMI:near
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

;wDPMIFlg	dw ?			;DPMI init call CX flags (CL=CPU[2,3,4],CH=??)
;wDPMIVer	dw ?			;DPMI init call DX Flags (DPMI version)

;*** SwitchToPMode
;--- returns C on errors, bx->error msg 

SwitchToPMode proc
	call changememstrat
JumpToPM_1:
	push cx
	@DPMI_SwitchEntry		;get address of PM entry in ES:DI
	mov bp,offset szNoDPMI  ;message "no dpmi server"

IF  @Cpu AND 00001000B			; 80386+
	cmp cl, 3			; 80386
	jb  JumpToPM_2			; Error if CPU not supported
ELSE
	IF  @Cpu AND 00000100B		; 80286+
	cmp cl, 2			; 80286
	jb  JumpToPM_2			; Error if CPU not supported
	ELSE
					; 8086 supported by any CPU, no check here
	ENDIF
ENDIF

	mov bx, ax
	cmp cl, 3
	mov ax, WF_CPU386
	je @f
	mov ax, WF_CPU486
@@:
;	mov [eWinFlags.wOfs],ax

	and bx,bx
	jz JumpToPM_3			;ok, DPMI host found

JumpToPM_2:
	pop cx
	mov ax,bp
	jmp ERROR0

JumpToPM_3:
	pop cx
	push es
	push di
	test si,si
	jz @F
; Allocate real mode buffer for DPMI host
	mov bx,si
	@GetBlok				  ;alloc real-mode mem block
	jc ERROR1
	mov es,ax
@@:
	call restorememstrat

	xor ax,ax				; We are 16-bit DPMI client

	mov bp,sp
	call dword ptr [bp]
	mov ax,offset errstr3		;cannot switch to prot-mode
	jc ERROR3

	@int3 _INT03JMPPM_
	mov [wKernelDS],ds
;	mov [wDPMIFlg],cx			;DPMI Flags
;	mov [wDPMIVer],dx			;dito
	mov [TH_TOPPDB],es			;psp
if 1;?USE1PSP
	mov [wCurPSP],es
endif
	@trace_s <lf,"------------------------------------",lf>
	@trace_s <"KERNEL now in protected mode, PSP=">
	@trace_w es
	@trace_s <",CS=">
	@trace_w cs
	@trace_s <",SS=">
	@trace_w ss
	@trace_s <",DS=">
	@trace_w ds
	@trace_s <lf>

; @todo May be add as extension loadable DOS Translation layer if DPMI-host doesn't support it?
;
; Check is MS-DOS extensions is present

	@DPMI_VendorEntry szDOSstr
	cmp al,0
	jz @F
	@trace_s <"fatal: no DOS API translation",lf>
	@Exit RC_INITPM	;just exit, dont display anything
@@:
	@trace_s <"DOS API translation initiated",lf>

	add sp,4
	ret
ERROR1:
	mov ax,offset errstr2	;insufficient DOS memory
ERROR3:
	add sp,4
ERROR0:
	push ax
	call restorememstrat
	pop bx
	stc
	ret
SwitchToPMode endp

;--- DWORD GetSelectorBase(WORD)
;
; Function rewrittend to conform Pietrek book
;


Get_Physical_Address proc near pascal theSelector: WORD
	and theSelector, not 7
	@DPMI_GetBase theSelector
	jc @F
	mov ax,dx
	mov dx,cx
	ret
@@:
	xor ax,ax
	xor dx,dx
	ret
Get_Physical_Address endp


;--- DWORD GetSelectorLimit(WORD)
GetSelectorLimit proc far pascal ulSelector: WORD
	mov bx, ulSelector
	push di
	sub sp,8
	mov di,sp
	push ss
	pop es
	@DPMI_GetDescriptor
	jc error
	mov ax,es:[di+0]
	mov dl,es:[di+6]
	and dx,000Fh
exit:
	add sp,8
	pop di
	ret 2
error:
	xor ax,ax
	xor dx,dx
	jmp exit
GetSelectorLimit endp

;--- SetSelectorLimit(WORD);
;--- returns always 0

SetSelectorLimit proc far pascal
	@loadbx
	@loadparm 0,dx
	@loadparm 2,cx
	@loadparm 4,bx
	@DPMI_SetLimit
	mov ax,0000
if 0
	jc @F
	mov ax,bx
@@:
endif
	@return 6
SetSelectorLimit endp

AllocCSToDSAlias proc far pascal
	pop cx
	pop dx
	pop bx
	push dx
	push cx
	@DPMI_CreateCSAlias
	jnc @F
	xor ax,ax
@@:
	@return
AllocCSToDSAlias endp

AllocDSToCSAlias proc far pascal
	pop dx
	pop cx
	pop bx
	push cx
	push dx
	@DPMI_AllocDesc
	jc @F
	call CreateAlias
	jnc exit
@@:
	xor ax,ax
exit:
	@return

AllocDSToCSAlias endp

PrestoChangoSelector proc far pascal
	pop cx
	pop dx
	pop ax
	pop bx
	push dx
	push cx
	call CreateAlias	 ;BX -> AX
	@return
PrestoChangoSelector endp

LDT_Init proc far pascal
	cmp GDTDsc, 0
	ret
LDT_Init endp

_TEXT	ends
	end
