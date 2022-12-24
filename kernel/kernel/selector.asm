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