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