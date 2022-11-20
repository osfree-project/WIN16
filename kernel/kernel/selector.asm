;
; Real Mode Selector functions doesn't actually have selectors, but segments.
; So, we just emulate selectors via segments.
;
; @todo emulate it
;

	; MacroLib
	include dos.inc

DGROUP	group _TEXT,_DATA

_TEXT	segment word public 'CODE'

	; Kernel
	include macros.inc
	include dpmildr.inc


;--- WORD AllocSelectorArray(WORD)

AllocSelectorArray proc far pascal
	pop dx			; Get far return address
	pop ax
	pop cx			; Get Number of selectors
	push ax			; Restore far return address
	push dx
;	@DPMI_AllocDesc cx	; Allocate descriptors
	ret
AllocSelectorArray endp

;--- WORD AllocSelector(WORD)
;--- returns 0 if an error occured
;
; UINT WINAPI AllocSelector(UINT uSelector);
;
; Create descriptor copy in LDT using uSelector as template.
; If uSelector is 0 then create new empty descriptor.
;
; Note: Original Andreas Grech implementation doesn't take into account 
; huge selector. Matt Pietrek describe how it must work.
;
; @todo: implement huge selector

AllocSelector proc far pascal
	pop cx
	pop dx
	pop bx
	push dx
	push cx
;	@DPMI_AllocDesc
	jc error
	and bx,bx
	jz @F
	push ds
	@SetKernelDS
;	call CopyDescriptor	;copy BX -> AX
	pop ds
@@:
	ret
error:
	xor ax,ax
	ret
AllocSelector endp

;--- WORD FreeSelector(WORD)
;--- returns 0 if successful, else the selector!
;
; Note: Original Andreas Grech implementation doesn't take in account 
; huge selector. Matt Pietrek describe how it must work.
;
; @todo: implement huge selector

FreeSelector proc far pascal
	pop cx
	pop dx
	pop bx
	push dx
	push cx
;        @DPMI_FreeDesc bx
	mov ax,0000
	jnc @F
	mov ax,bx
@@:
	ret
FreeSelector endp

;--- DWORD GetSelectorBase(WORD)

GetSelectorBase proc far pascal
	pop dx
	pop cx
	pop bx
	push cx
	push dx
;        @DPMI_GetBase
	jc @F
	mov ax,dx
	mov dx,cx
	ret
@@:
	xor ax,ax
	xor dx,dx
	ret

GetSelectorBase endp

;--- WORD SetSelectorBase(WORD)
;--- returns 0 if an error occured, else the selector value

SetSelectorBase proc far pascal
	@loadbx
	@loadparm 0,dx
	@loadparm 2,cx
	@loadparm 4,bx
;        @DPMI_SetBase
	mov ax,0000
	jc @F
	mov ax,bx
@@:
	@return 6
SetSelectorBase endp

;--- DWORD GetSelectorLimit(WORD)

GetSelectorLimit proc far pascal

	pop dx
	pop cx
	pop bx
	push cx
	push dx

if ?32BIT
	lsl eax,ebx
	jnz @F
	xor eax,eax
@@:
	push eax
	pop ax
	pop dx
	ret
else
	push di
	sub sp,8
	mov di,sp
	push ss
	pop es
;	@DPMI_GetDescriptor
	jc error
	mov ax,es:[di+0]
	mov dl,es:[di+6]
	and dx,000Fh
exit:
	add sp,8
	pop di
	ret
error:
	xor ax,ax
	xor dx,dx
	jmp exit
endif

GetSelectorLimit endp

;--- SetSelectorLimit(WORD);
;--- returns always 0

SetSelectorLimit proc far pascal
	@loadbx
	@loadparm 0,dx
	@loadparm 2,cx
	@loadparm 4,bx
;	@DPMI_SetLimit
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
;	@DPMI_CreateCSAlias
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
;	@DPMI_AllocDesc
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

_TEXT ends

	end