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

		; MacroLib
		include bios.inc
		include dos.inc

;WINVER		equ	101		; Windows 1.01
;WINVER		equ	102		; Windows 1.02
;WINVER		equ	103		; Windows 1.03
;WINVER		equ	104		; Windows 1.04
WINVER		equ	300		; Windows 3.00

TRACE		equ	1		; Turn trace on = 1, off = 0

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

	mov	[seg1s], cs
	mov	[seg2s], cs
	mov	[seg3s], cs

; Show LOGO
if	TRACE
	xor	ax, ax			; Set Logo size to zero
	jp	skip

szTraceFreeMem db 'Free unused memory', 0dh, 0ah, '$'
szTraceCheckMem db 'Check required free memory', 0dh, 0ah, '$'
szTraceSearchKernel db 'Search KERNEL.EXE', 0dh, 0ah, '$'
szTraceSearchKernel286 db 'Search KRNL286.EXE', 0dh, 0ah, '$'
szTraceSearchKernel386 db 'Search KRNL386.EXE', 0dh, 0ah, '$'
szTraceSearchDOSX db 'Search DOSX.EXE', 0dh, 0ah, '$'
szTraceExecDOSX db 'Execute DOSX.EXE', 0dh, 0ah, '$'
szTraceSearchWIN386 db 'Search WIN386.EXE', 0dh, 0ah, '$'
szTraceExecWIN386 db 'Execute WIN386.EXE', 0dh, 0ah, '$'
szTraceDetectCPU db 'Detect CPU', 0dh, 0ah, '$'
szTrace286CPU db '80286 or higher CPU detected', 0dh, 0ah, '$'
szTrace386CPU db '80386 or higher CPU detected', 0dh, 0ah, '$'

skip:
else
	call	ShowLogo		; Returns in AX resident part of LOGO
endif

; free unused memory

if TRACE
	@DispStr offset szTraceFreeMem
endif
	add	ax, LogoStart        	; End of our code (aligned to 16)
	mov	cl, 4
	shr	ax, cl			; number of used para
	push	cs
	pop	es
	@ModBlok ax			; Modify memory block

; check memory for Windows Kernel

if TRACE
	@DispStr offset szTraceCheckMem
endif
	@GetBlok 0FFFFH			; Allocate impossible value of memory
	jnc	panic			; Something wrong, it is not possible to have so many memory
	cmp	bx, MIN_CONV_MEM	; kb in para
	jb	NoMem

if WINVER eq 300

if TRACE
	@DispStr offset szTraceDetectCPU
endif

	; test for 80286 -- this CPU executes PUSH SP by first storing SP on
	; stack, then decrementing it.  earlier CPU's decrement, THEN store.
	push	sp			; only 80286 pushes pre-push SP
	pop	ax			; get it back
	cmp	ax,sp			; check for same
	jne	RealModeKernel		; they are not, so it a 8086

if TRACE
	@DispStr offset szTrace286CPU
endif

	;check if 386+
	;MSW bits 15..4 should be clear
.286
	smsw	ax
.8086
	cmp	ax, 0fff0h
	jae	StandardMode		; It is not 80386+, so only StandardMode allower

if TRACE
	@DispStr offset szTrace386CPU
endif

; search KRNL386.EXE
if TRACE
	@DispStr offset szTraceSearchKernel386
endif
	@GetFirst szKernel386		; Find first file entry
	jc	StandardMode

EnhancedMode:

if TRACE
	@DispStr offset szTraceSearchWIN386
endif
	@GetFirst szWIN386		; Find first file entry
	jc	StandardMode		; No Win386.exe, so only standard mode

; load and execute WIN386.EXE
if TRACE
	@DispStr offset szTraceExecWIN386
endif
	push	cs
	pop	es
	mov	[tmpSS], ss
	mov	[tmpSP], sp
	@Exec	szWIN386;, exeparams		; Execute program
	push	cs
	pop	ds
	mov	ss, [tmpSS]
	mov	sp, [tmpSP]
	jc	ExecErr
	jmp	Exit

StandardMode:
; Here only Standard mode

; search KRNL286.EXE
if TRACE
	@DispStr offset szTraceSearchKernel286
endif

	@GetFirst szKernel286		; Find first file entry
	jc	NoKernel

if TRACE
	@DispStr offset szTraceSearchDOSX
endif
	@GetFirst szDOSX		; Find first file entry
	jc	NoDOSX

; load and execute DOSX.EXE
if TRACE
	@DispStr offset szTraceExecDOSX
endif
	push	cs
	pop	es
	mov	[tmpSS], ss
	mov	[tmpSP], sp
	@Exec	szDOSX;, exeparams		; Execute program
	push	cs
	pop	ds
	mov	ss, [tmpSS]
	mov	sp, [tmpSP]
	jc	ExecErr
	jmp	Exit
RealModeKernel:
endif

; search KERNEL.EXE
if TRACE
	@DispStr offset szTraceSearchKernel
endif

	@GetFirst szKernel		; Find first file entry
	jc	NoKernel

KernelFound:
; load and execute KERNEL.EXE
	push	cs
	pop	es
	mov	[tmpSS], ss
	mov     [tmpSP], sp
	@Exec	szKernel;, exeparams		; Execute program
	push	cs
	pop	ds
	mov	ss, [tmpSS]
	mov     sp, [tmpSP]
	jc	ExecErr

;
; exit from windows kernel
Exit:
if	not TRACE
	call	HideLogo
endif
	@Exit	0			; die

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
	cmp	word ptr [bx], 'OL'
	jne	LogoRet
	cmp	word ptr [bx+2], 'OG'
	jne	LogoRet

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
NoDOSXMsg:
	db	'DOS Extender not found$'
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
	db	'     :F  Turns off 32-bit disk access. Equivalent to SYSTEM.INI [386enh]',0dh,0ah
	db	'         setting: 32BitDiskAccess=FALSE.',0dh,0ah
	db	'     :S  Specifies that Windows should not use ROM address space between',0dh,0ah
	db	'         F000:0000 and 1 MB for a break point. Equivalent to SYSTEM.INI',0dh,0ah
	db	'         [386enh] setting: SystemROMBreakPoint=FALSE.',0dh,0ah
	db	'     :V  Specifies that the ROM routine handles interrupts from the hard',0dh,0ah
	db	'         drive controller. Equivalent to SYSTEM.INI [386enh] setting:',0dh,0ah
	db	'         VirtualHDIRQ=FALSE.',0dh,0ah
	db	'     :X  Excludes all of the adapter area from the range of memory that',0dh,0ah
	db	'         Windows scans to find unused space. Equivalent to SYSTEM.INI',0dh,0ah
	db	'         [386enh] setting: EMMExclude=A000-FFFF.',0dh,0ah, '$'
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
szKernel286:
	db	'SYSTEM\KRNL286.EXE', 0
szKernel386:
	db	'SYSTEM\KRNL386.EXE', 0
szDOSX:
	db	'SYSTEM\DOSX.EXE', 0
szWIN386:
	db	'SYSTEM\WIN386.EXE', 0
endif

NoDOSX:
	lea	dx, NoDOSXMSG
	jmp	Die

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
if	not TRACE
	push	dx
	call	HideLogo
	@SetMode		; Switch to video mode 3
	pop	dx
endif

	@DispStr dx		; Print message

	@Exit	0		; die

	align 16
; Stack
stackend:
	db	1024 dup (0)
stackstart:
; LOGO will be attached here. Because it must start on para start it must be aligned to 16 bytes
	align 16
LogoStart:
code	ends
	END main

