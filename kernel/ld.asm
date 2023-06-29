	; MacroLib
	include dos.inc
	; Kernel
	include kernel.inc


_TEXT segment
externdef pascal wKernelDS:word
public LoadModule

LoadModule proc far pascal uses ds lpszModuleName:far ptr byte, lpParameterBlock:far ptr
	@SetKernelDS
	mov [fLoadMod],1	;use a asciiz command line
	lds dx, lpszModuleName
	les bx, lpParameterBlock
	@Exec
	@SetKernelDS
	mov [fLoadMod],0
	ret
LoadModule endp

_TEXT ends
	end

