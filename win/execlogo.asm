;/*!
;   @file
;
;   @brief Windows loader
;
;   (c) osFree Project 2002-2022, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;*/

.8086

WINVER		equ	101		; Windows 1.01
;WINVER		equ	102		; Windows 1.02
;WINVER		equ	103		; Windows 1.03
;WINVER		equ	104		; Windows 1.04
;WINVER		equ	300		; Windows 3.00

;----------------------------------------------------------------
; Memory check configuration
;
; Constants here is smaller of official documentation because
; WIN.COM and LOGO eats some space too.
;----------------------------------------------------------------

if WINVER eq 101
MIN_CONV_MEM		equ 3400h		; 208 kb in para
endif

IF WINVER eq 300
MIN_CONV_MEM		equ 6000h		; 384 kb in para
ENDIF

;----------------------------------------------------------------
; Switches configuration
;
; Sets type of supported switches
;----------------------------------------------------------------

if WINVER eq 101
SWITCHES_SUPPORT	equ 0			; No switches support
endif
if WINVER eq 102
SWITCHES_SUPPORT	equ 0			; No switches support
endif
if WINVER eq 103
SWITCHES_SUPPORT	equ 0			; No switches support
endif
if WINVER eq 104
SWITCHES_SUPPORT	equ 0			; No switches support
endif
if WINVER eq 300
SWITCHES_SUPPORT	equ 5			; Full Windows 3.11 switches support
endif

code segment
	org 100h

main:
	lea	sp, stackstart
; Show LOGO
	call	ShowLogo		; Returns in AX resident part of LOGO

; free unused memory

	add	ax, LogoStart        	; End of our code (aligned to 16)
	mov	cl, 4
	shr	ax, cl
	mov	bx, ax			; number of used para
	push	cs
	pop	es
	mov	ah, 4Ah
	int	21h

; check memory for Windows Kernel
	mov	ah, 48h			; Allocate memory
	mov	bx, 0ffffh		; Impossible value of memory
	int	21h
	jnc	panic			; Something wrong, it is not possible to have so many memory
	cmp	bx, MIN_CONV_MEM	; kb in para
	jb	NoMem

; search KERNEL.EXE
	mov	ah, 4eh			; Find first file entry
	lea	dx, szKernel		; Filename
	xor	cx, cx
	int	21h
	jc	NoKernel

; load and execute KERNEL.EXE
	push	ds
	pop	es
	lea	dx, szKernel
	lea	bx, exeparams
	mov	[seg1s], cs
	mov	[seg2s], cs
	mov	[seg3s], cs
	mov	[tmpSS], ss
	mov     [tmpSP], sp
	mov	ax, 4b00h		; Execute program
	int	21h
	mov	ss, [tmpSS]
	mov     sp, [tmpSP]
	push	cs
	pop	ds
	jc	ExecErr

; exit from windows kernel
	call	HideLogo
	mov	ax, 4c00h			; die
	int	21h

; Call HideLogo
HideLogo:
	mov	dx, 7
	jmp	CallLogo

; Call ShowLogo
ShowLogo:
	mov	dx, 4

; Execute LOGO entry at DX
CallLogo:
	push	ds                      ; Store our data segment

	mov	ax, cs			; calc code segment of LOGO
	lea	bx, LogoStart
	mov	cl, 4
	shr	bx, cl
	add	ax, bx			; Number of segments before logo starts

	mov	ds, ax			; set data segment to LOGO

	xor	ax, ax			; Size of LOGO

	xor	bx, bx			; check for 'LOGO' signature
	cmp     word ptr [bx], 'OL'
	jne     LogoRet
	cmp     word ptr [bx+2], 'OG'
	jne     LogoRet
	

	lea	bx, LogoRet
	push	cs			; prepare return from ShowLogo
	push	bx

	push	ds			; LOGO code segment
	push	dx			; Show logo entry
	retf				; Simulate far jump to LogoStart:0004h

; Return from LOGO
LogoRet:
	pop	ds			; Restore our data segment (stored in ShowLogo)
	retn


exeparams label byte
	dw	0
	dw	80h
seg1s	dw	?
	dw	5Ch
seg2s	dw	?
	dw	6Ch
seg3s	dw	?

tmpSS	dw	?
tmpSP	dw	?

PanicMsg:
	db	'Panic! Unrecoverable error!$'
NoMemMsg:
	db	'Windows requires at least 256Kb of RAM$'
NoKernelMsg:
	db	'Windows kernel not found$'
ExecErrMsg:
	db	'Can''t execute Windows kernel$'
HelpMsg:
	db	'osFree Windows loader',0dh,0ah,0dh,0ah
if SWITCHES_SUPPORT eq 0
	db	'$'
endif

if SWITCHES_SUPPORT eq 5
	db	'WIN [/R] [/3] [/S] [/B] [/D:[F][S][V][X]]',0dh,0ah,0dh,0ah
	db	'   /?  Prints this instruction banner.',0dh,0ah
	db	'   /h  Synonym for the /? switch.',0dh,0ah
	db	'   /3  Starts Windows in 386 enhanced mode.',0dh,0ah
	db	'   /S  Starts Windows in standard mode.',0dh,0ah
	db	'   /2  Synonym for the /S switch.',0dh,0ah
	db	'   /R  Starts Windows in real mode.',0dh,0ah
	db	'   /B  Creates a file, BOOTLOG.TXT, that records system messages.',0dh,0ah
	db	'       generated during system startup (boot).',0dh,0ah
	db	'   /D  Used for troubleshooting when Windows does not start',0dh,0ah
	db	'       correctly.',0dh,0ah
	db	'   :F  Turns off 32-bit disk access. Equivalent to SYSTEM.INI [386enh]',0dh,0ah
	db	'       setting: 32BitDiskAccess=FALSE.',0dh,0ah
	db	'   :S  Specifies that Windows should not use ROM address space between',0dh,0ah
	db	'       F000:0000 and 1 MB for a break point. Equivalent to SYSTEM.INI',0dh,0ah
	db	'       [386enh] setting: SystemROMBreakPoint=FALSE.',0dh,0ah
	db	'   :V  Specifies that the ROM routine handles interrupts from the hard',0dh,0ah
	db	'       drive controller. Equivalent to SYSTEM.INI [386enh] setting:',0dh,0ah
	db	'       VirtualHDIRQ=FALSE.',0dh,0ah
	db	'   :X  Excludes all of the adapter area from the range of memory that',0dh,0ah
	db	'       Windows scans to find unused space. Equivalent to SYSTEM.INI',0dh,0ah
	db	'       [386enh] setting: EMMExclude=A000-FFFF.',0dh,0ah, '$'
endif

szKernel:
if WINVER eq 101
	db	'WIN100.BIN', 0
endif
if WINVER eq 102
	db	'WIN100.BIN', 0
endif
if WINVER eq 103
	db	'WIN100.BIN', 0
endif
if WINVER eq 104
	db	'WIN100.BIN', 0
endif
if WINVER eq 300
	db	'SYSTEM\KERNEL.EXE', 0
endif

Help:
	lea	dx, HelpMsg
	jmp	Die

ExecErr:
	lea	dx, ExecErrMsg
	jmp	Die

NoKernel:
	lea	dx, NoKernelMsg
	jmp	Die

NoMem:	lea	dx, NoMemMsg
	jmp	Die

Panic:
	lea	dx, PanicMsg

; Disable logo, set standard video mode and exit to DOS
Die:	
	push	dx
	call	HideLogo

	mov	ax, 0003h
	int	10h		; Switch to video mode 3

	pop	dx

	mov	ah, 09h
	int	21h		; Print message

	mov	ax, 4c00h			; die
	int	21h

; Stack
stackend:
	db	200h dup (0)
stackstart:
; LOGO will be attached here. Because it must start on para start it must be aligned to 16 bytes
	align 16
LogoStart:
code	ends
	END main

