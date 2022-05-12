;
; This is function to execute windows logo file
;
.8086

code segment
	org 100h

main:

; Show LOGO
	call	ShowLogo

; check memory for KERNEL.EXE
	mov	ah, 48h			; Allocate memory
	mov	bx, 0ffffh		; Impossible value of memory
	int	21h
	jnc	panic			; Something wrong, not possible to have such many memory
	cmp	bx, 4000h		; 256 kb in para
	jb	NoMem

; search KERNEL.EXE
	mov	ah, 4eh			; Find first file entry
	mov	dx, szKernel		; Filename
	xor	cx, cx
	int	21h
	jc	NoKernel

; load and execute KERNEL.EXE
	push	ds
	pop	es
	mov	dx, szKernel
	mov	bx, exeparams
	mov	ax, 4b00h		; Execute program
	int	21h
	jc	execErr
; exit from windows kernel

	call	HideLogo
	int	20h			; die

ShowLogo:
	push	cs			; prepare return from ShowLogo
	mov	ax, offset LogoRet
	push	ax
; check for signature
	mov	ax, cs
	mov	bx, LogoStart
	mov	cl, 4
	shr	bx, cl
	add	ax, bx			; Number of segments before logo starts
	push	ax
	mov	ax, 4
	push	ax
	retf				; Simulate far jump to LogoStart:0004h
LogoRet:
; free unneeded memory (part of LOGO code/data)
	retn

HideLogo:
	push	cs			; prepare return from ShowLogo
	mov	ax, offset HideLogoRet
	push	ax
	mov	ax, cs
	mov	bx, LogoStart
	mov	cl, 4
	shr	bx, cl
	add	ax, bx			; Number of segments before logo starts
	push	ax
	mov	ax, 7
	push	ax
	retf				; Simulate far jump to LogoStart:0007h
HideLogoRet:
	retn

exeparams label byte
	dw	0
	dw	80h
seg1s	dw	?
	dw	5Ch
seg2s	dw	?
	dw	6Ch
seg3s	dw	?

PanicMsg:
	db	'Panic! Unrecoverable error!$'
NoMemMsg:
	db	'Windows requires at least 256Kb of RAM$'
NoKernelMsg:
	db	'Windows kernel not found$'
ExecErrMsg:
	db	'Can''t execute Windows kernel$'
szKernel:
	db	'SYSTEM\KERNEL.EXE', 0

ExecErr:
	lea	dx, ExecErrMsg
	jp	Die
NoKernel:
	lea	dx, NoKernelMsg
	jp	Die
NoMem:	lea	dx, NoMemMsg
	jp	Die
Panic:
	lea	dx, PanicMsg
Die:	mov	ax, 0300h
	int	10h		; Switch to video mode 3
	mov	ah, 09h
	int	21h		; Print message
	call	HideLogo
	int	20h		; Die

	align 16
LogoStart:
code	ends
	END main

