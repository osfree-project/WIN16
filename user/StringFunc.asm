.8086

extern AnsiNext: far
extern AnsiPrev: far
extern AnsiUpper: far
extern AnsiLower: far

public pascal StringFunc

_TEXT SEGMENT 'CODE' PUBLIC USE16
oAnsiNext label word
	dw offset AnsiNext
	dw offset AnsiPrev
	dw offset AnsiUpper
	dw offset AnsiLower
StringFunc proc far pascal
	mov bx, cx
	dec bx
	shl bx,1
	jmp oAnsiNext[bx]
StringFunc	endp
_TEXT ENDS

	end