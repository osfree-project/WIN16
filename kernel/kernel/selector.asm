;
; Real Mode Selector functions doesn't actually have selectors, but segments.
; So, we just emulate selectors via segments.
;
; @todo emulate it
;

	; MacroLib
	include dos.inc

	; Kernel
	include kernel.inc


_TEXT	segment


;--- DWORD GetSelectorBase(WORD)
;
; Just multiple on 16 (paragraph size)
;
GetSelectorBase proc far pascal
	pop dx
	pop cx
	pop ax
	push cx
	push dx
	xor dx,dx
	mov cx, 4
mult:
	shl ax, 1
	rcl dx, 1
	loop mult
	ret
GetSelectorBase endp

;--- WORD SetSelectorBase(WORD)
;--- returns 0 if an error occured, else the segment value
;
; Just delete base on 16 (para size)
;

SetSelectorBase proc far pascal
	@loadbx
	@loadparm 0,ax	; base
	@loadparm 2,dx
	@loadparm 4,bx	; selector
	mov cx, 4
divi:
	shr dx, 1
	rcr ax, 1
	loop divi
	or dx,dx
	jz @F
	xor ax,ax
@@:
	@return 6
SetSelectorBase endp

;--- DWORD GetSelectorLimit(WORD)
; Segment limit always 64k

GetSelectorLimit proc far pascal
	mov ax, 0ffffh
	xor dx,dx
	ret 2
GetSelectorLimit endp

;--- SetSelectorLimit(WORD);
;--- returns always 0

SetSelectorLimit proc far pascal
	xor ax,ax
	@return 6
SetSelectorLimit endp

AllocCSToDSAlias proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	@return
AllocCSToDSAlias endp

AllocDSToCSAlias proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	@return
AllocDSToCSAlias endp

;@todo finish it

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