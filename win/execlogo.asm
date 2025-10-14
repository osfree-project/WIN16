;/*!
;   @file
;
;   @brief Windows loader
;
;   (c) osFree Project 2002-2023, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;*/

.8086
.model tiny

		; MacroLib
		include bios.inc
		include dos.inc

;WINVER		equ	101		; Windows 1.01
;WINVER		equ	102		; Windows 1.02
;WINVER		equ	103		; Windows 1.03
;WINVER		equ	104		; Windows 1.04
WINVER		equ	300		; Windows 3.00

TRACE		equ	1		; Turn trace on = 1, off = 0

@Trace	MACRO szMsg
if TRACE
	@DispStr offset szMsg
endif
	ENDM
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
	align 1
	org 100h

;.COM-format executables begin running with the following register values:
;	AL = 00h if first FCB has valid drive letter, FFh if not
;	AH = 00h if second FCB has valid drive letter, FFh if not
;	CS,DS,ES,SS = PSP segment
;	SP = offset of last word available in first 64K segment
;	(note: AX is always 0000h under DESQview)

main:
	; Tune stack
	lea	sp, stackstart

	; Prepare execution block
	mov	[seg1s], es
	mov	[seg2s], es
	mov	[seg3s], es

	mov	[seg1], es
	mov	[seg2], es
	mov	[seg3], es

	; Parse command line
	call	ParseCmd

; Show LOGO
	jmp	skip

szTraceFreeMem db 'Free unused memory', 0dh, 0ah, '$'
szTraceCheckMem db 'Check required free memory', 0dh, 0ah, '$'
szTraceSearchKernel db 'Search KERNEL.EXE', 0dh, 0ah, '$'
szTraceSearchKernel286 db 'Search KRNL286.EXE', 0dh, 0ah, '$'
szTraceSearchKernel386 db 'Search KRNL386.EXE', 0dh, 0ah, '$'
szTraceKrnl386 db 'KRNL386.EXE selected', 0dh, 0ah, '$'
szTraceKrnl286 db 'KRNL286.EXE selected', 0dh, 0ah, '$'
szTraceKernel db 'KERNEL.EXE selected', 0dh, 0ah, '$'
szTraceSearchDOSX db 'Search DOSX.EXE', 0dh, 0ah, '$'
szTraceExecDOSX db 'Execute DOSX.EXE', 0dh, 0ah, '$'
szTraceExecKernel db 'Execute KERNEL.EXE', 0dh, 0ah, '$'
szTraceSearchWIN386 db 'Search WIN386.EXE', 0dh, 0ah, '$'
szTraceExecWIN386 db 'Execute WIN386.EXE', 0dh, 0ah, '$'
szTraceDetectCPU db 'Detect CPU', 0dh, 0ah, '$'
szTrace286CPU db '80286 or higher CPU detected', 0dh, 0ah, '$'
szTrace386CPU db '80386 or higher CPU detected', 0dh, 0ah, '$'
szTraceRealMode db 'Real Mode selected', 0dh, 0ah, '$'
szTraceStandardMode db 'Standard Mode selected', 0dh, 0ah, '$'
szTraceEnhancedMode db 'Enhanced Mode selected', 0dh, 0ah, '$'
szTraceShowLogo db 'Show logo', 0dh, 0ah, '$'
szTraceHideLogo db 'Hide logo', 0dh, 0ah, '$'
szTraceNoLogo db 'Logo not found', 0dh, 0ah, '$'

skip:
	@Trace	szTraceShowLogo
	call	ShowLogo		; Returns in AX resident part of LOGO

; free unused memory

	@Trace	szTraceFreeMem
	add	ax, LogoStart        	; End of our code (aligned to 16)
	mov	cl, 4
	shr	ax, cl			; number of used para
	push	cs
	pop	es
	@ModBlok ax			; Modify memory block

; check memory for Windows Kernel

	@Trace	szTraceCheckMem
	@GetBlok 0FFFFH			; Allocate impossible value of memory
	jnc	panic			; Something wrong, it is not possible to have so many memory
	cmp	bx, MIN_CONV_MEM	; kb in para
	jb	NoMem

if WINVER eq 300
	; Detect CPU
	call	DetectCPU

	; Check kernels presense
	call	CheckKernels

	; Check extenders presense
	call	CheckExtenders

	; First, if switch predefined, then select initial mode
	cmp	[opEnhanced], 1
	jz	EnhancedMode
	cmp	[opStandard], 1
	jz	StandardMode
	cmp	[opReal], 1
	jz	RealModeKernel

	; Second, if no switch predefined, use CPU to select initial mode
	cmp	[opCPU], 2		; if 80386, then use enhanced mode
	jz	EnhancedMode
	cmp	[opCPU], 1		; if 80286, then use standard mode
	jz	StandardMode
	jmp	RealModeKernel

EnhancedMode:
	cmp	[opCPU], 2
	jnz	StandardMode		; If no 386 CPU, then downgrade to StandardMode

	cmp	[opWIN386], 1
	jnz	StandardMode		; If no WIN386 extender, then downgrade to StandardMode

	cmp	[opKRNL386], 1
	jnz	StandardMode		; If no KRNL386 kernel, then  downgrade to StandardMode

	@Trace	szTraceKrnl386

	; All exists for Enhanced mode
	@Trace	szTraceEnhancedMode

	; load and execute WIN386.EXE
	@Trace	szTraceExecWIN386
	push	cs
	pop	es
	mov	[tmpSS], ss
	mov	[tmpSP], sp
	@Exec	szWIN386, exeparams		; Execute program
	push	cs
	pop	ds
	mov	ss, [tmpSS]
	mov	sp, [tmpSP]
	jc	ExecErr
	jmp	Exit

StandardMode:
	cmp	[opCPU], 1
	jb	RealModeKernel		; If no 286 or 386 CPU, then downgrade to RealModeKernel
	je	StandardModeKrnl286	; If 286 CPU then check KRNL286

	cmp	[opKRNL386], 1
	jnz	StandardModeKrnl286	; If no KRNL386, then try KRNL286

	; Here we ready to start KRNL386
	@Trace	szTraceKrnl386
	jmp	CheckExt

StandardModeKrnl286:
	cmp	[opKRNL286], 1
	jnz	RealModeKernel		; If no KRNL286, then downgrade to RealModeKernel

	; Here we ready to start KRNL286
	@Trace	szTraceKrnl386

CheckExt:
	cmp	[opDOSX], 1
	jnz	RealModeKernel		; If no DOSX extender, then downgrade to RealModeKernel


	; All exists for Standard mode
	@Trace	szTraceStandardMode

	; load and execute DOSX.EXE
	@Trace	szTraceExecDOSX

	if 0
	push	cs
	pop	es
	mov	[tmpSS], ss
	mov	[tmpSP], sp
	@Exec	szDSWAP, exeparams2		; Execute program
	push	cs
	pop	ds
	mov	ss, [tmpSS]
	mov	sp, [tmpSP]
	jc	ExecErr
	endif

	push	cs
	pop	es
	mov	[tmpSS], ss
	mov	[tmpSP], sp
	@Exec	szDOSX, exeparams		; Execute program
	push	cs
	pop	ds
	mov	ss, [tmpSS]
	mov	sp, [tmpSP]
	jc	ExecErr

	jmp	Exit
RealModeKernel:
endif
	cmp	[opKERNEL], 1
	jnz	NoKernel

	@Trace	szTraceKernel
	@Trace	szTraceRealMode

; load and execute KERNEL.EXE
	@Trace	szTraceExecKernel

KernelFound:
; load and execute KERNEL.EXE
	push	cs
	pop	es
	mov	[tmpSS], ss
	mov	[tmpSP], sp
	@Exec	szKernel, exeparams		; Execute program
	push	cs
	pop	ds
	mov	ss, [tmpSS]
	mov	sp, [tmpSP]
	jc	ExecErr

;
; exit from windows kernel
Exit:
	@Trace	szTraceHideLogo
	call	HideLogo
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
	push	ds			; Store our data segment (restored in NoLogo or LogoRet)

	mov	ax, cs			; calc code segment of LOGO
	lea	bx, LogoStart
	mov	cl, 4
	shr	bx, cl
	add	ax, bx			; Number of segments before logo starts

	mov	ds, ax			; set data segment to LOGO

	xor	ax, ax			; Size of LOGO

	xor	bx, bx			; check for 'LOGO' signature
	cmp	word ptr [bx], 'OL'
	jne	NoLogo
	cmp	word ptr [bx+2], 'OG'
	jne	NoLogo

	lea	bx, LogoRet
	push	cs			; prepare return from ShowLogo
	push	bx

	push	ds			; LOGO code/data segment
	push	dx			; Show logo entry
	retf				; Simulate far jump to LogoStart:0004h

NoLogo:
	pop	ds			; Restore our data segment (stored in ShowLogo)
	@Trace	szTraceNoLogo
	retn

; Return from LOGO
LogoRet:
	pop	ds			; Restore our data segment (stored in ShowLogo)
	retn

DetectCPU:
	@Trace	szTraceDetectCPU

	; test for 80286 -- this CPU executes PUSH SP by first storing SP on
	; stack, then decrementing it.  earlier CPU's decrement, THEN store.
	push	sp			; only 80286 pushes pre-push SP
	pop	ax			; get it back
	cmp	ax,sp			; check for same
	jne	ExitDetectCPU		; they are not, so it a 8086
	inc	[opCPU]			; CPU=80286
	@Trace	szTrace286CPU

	;check if 386+
	;MSW bits 15..4 should be clear
.286
	smsw	ax
.8086
	cmp	ax, 0fff0h
	jae	StandardMode		; It is not 80386+, so only StandardMode allowed
	inc	[opCPU]			; CPU=80386
	@Trace	szTrace386CPU

ExitDetectCPU:
	ret

CheckKernels:
; search KERNEL.EXE
	@Trace	szTraceSearchKernel
	@GetFirst szKernel		; Find first file entry
	jc	ChkKRNL286
	mov	[opKERNEL], 1
; search KRNL286.EXE
ChkKRNL286:
	@Trace	szTraceSearchKernel286
	@GetFirst szKernel286		; Find first file entry
	jc	ChkKRNL386
	mov	[opKRNL286], 1
; search KRNL386.EXE
ChkKRNL386:
	@Trace	szTraceSearchKernel386
	@GetFirst szKernel386		; Find first file entry
	jc	ExitCheckKernels
	mov	[opKRNL386], 1
ExitCheckKernels:
	ret

CheckExtenders:
	@Trace	szTraceSearchWIN386
	@GetFirst szWIN386		; Find first file entry
	jc	CheckDOSX		; No Win386.exe, so only standard mode
	mov	[opWIN386], 1
CheckDOSX:
	@Trace	szTraceSearchDOSX
	@GetFirst szDOSX		; Find first file entry
	jc	ExitCheckExtenders
	mov	[opDOSX], 1
ExitCheckExtenders:
	ret

exeparams label byte
	dw	0
	dw	cmdline
seg1s	dw	?
	dw	5Ch
seg2s	dw	?
	dw	6Ch
seg3s	dw	?

tmpSS	dw	?
tmpSP	dw	?

cmdline db 128 dup (?)

exeparams2 label byte
	dw	0
	dw	cmdline2
seg1	dw	?
	dw	5Ch
seg2	dw	?
	dw	6Ch
seg3	dw	?

cmdline2 db     02h, ' ', 08h, 0dh, 'test', 0dh

opCPU		db	0	; 0-8086, 1-80286, 2-80386
opDOSX		db	0
opWIN386	db	0
opKERNEL	db	0
opKRNL286	db	0
opKRNL386	db	0
opStandard	db	0
opReal		db	0
opEnhanced	db	0
opBootlog	db	0
opNo32Disk	db	0
opNoROM		db	0
opNoVirtualHD	db	0
opEMMExclide	db	0

PanicMsg:
	db	'Panic! Unrecoverable error!',0dh,0ah,'$'
NoMemMsg:
	db	'Windows requires at least 256Kb of RAM',0dh,0ah,'$'
NoKernelMsg:
	db	'Windows kernel not found',0dh,0ah,'$'
NoDOSXMsg:
	db	'DOS Extender not found',0dh,0ah,'$'
ExecErrMsg:
	db	'Can''t execute Windows kernel',0dh,0ah,'$'
HelpMsg:
	db	'osFree Windows subsystem loader',0dh,0ah
if SWITCHES_SUPPORT eq 0
	db	'$'
endif

if SWITCHES_SUPPORT eq 5
	db	'WIN [/R] [/3] [/S] [/B] [/D:[F][S][V][X]]',0dh,0ah,0dh,0ah
	db	'   /? or /H Prints this instruction banner.',0dh,0ah
	db	'   /3  Starts Windows in 386 enhanced mode.',0dh,0ah
	db	'   /S or /2  Starts Windows in standard mode.',0dh,0ah
	db	'   /R  Starts Windows in real mode.',0dh,0ah
	db	'   /B  Creates a file, BOOTLOG.TXT, that records system messages.',0dh,0ah
	db	'       generated during system startup (boot).',0dh,0ah
	db	'   /D  Used for troubleshooting when Windows does not start correctly:',0dh,0ah
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
szDSWAP:
	db	'SYSTEM\DSWAP.EXE', 0
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
	push	dx
	call	HideLogo

	@SetMode		; Switch to video mode 3
	pop	dx

	@DispStr dx		; Print message

	@Exit	0		; die

ParseCmd:
; Equates into command line

;CmdLen	equ	byte ptr es:[80h] ; Command line length
Cmd	equ	byte ptr es:[81h] ; Command line data


; Okay, begin scanning and parsing the command line. Replace handled
; switches by space.

	lea	si, Cmd		; Pointer to command line
	mov	al,[si-1]
	lea	di, cmdline+1	; Pointer to command line
	mov	[cmdline],al
	cld

SkipSpaces:
	lodsb
	stosb
	cmp	al, ' '
	je	SkipSpaces

; EOL?
	cmp	al, 0dh
	je	ExitParseCmd

; Determine if this is a switch
CheckSwitch:
	cmp	al, "/"
	jz	ParseSwitch
	cmp	al, "-"
	jz	ParseSwitch

; If no, copy until space or EOL
FindDelimiter:
	lodsb
	stosb
	cmp	al, 0dh
	je	ExitParseCmd
	cmp	al, ' '
	jne	FindDelimiter
	jmp	CheckSwitch

ParseSwitch:
	lodsb
	stosb

	cmp	al, "?"
	jz	Help
	cmp	al, "2"
	jZ	SetStandard
	cmp	al, "3"
	jz	SetEnhanced
	or	al, 020h		;Convert to lowercase
	cmp	al, "h"
	jz	Help
	cmp	al, "r"
	jz	SetReal
	cmp	al, "s"
	jz	SetStandard
;	cmp	al, "d"
;	jz	SetDisables
	cmp	al, "b"
	jz	SetBootlog

ContinueParse:
	dec	si
	dec	di

	jmp	FindDelimiter

SetBootlog:
	call	TestDelimiter	; a delimiter
	jnz	ContinueParse
	mov	opBootlog, 1
	jmp	ContinueParse

SetReal:
	call	TestDelimiter	; a delimiter
	jnz	ContinueParse
	mov	opReal, 1
	jmp	HideSwitch

SetStandard:
	call	TestDelimiter	; a delimiter
	jnz	ContinueParse
	mov	opStandard, 1
	jmp	HideSwitch

SetEnhanced:
	call	TestDelimiter	; a delimiter
	jnz	ContinueParse
	mov	opEnhanced, 1

HideSwitch:
	mov	word ptr es:[di-3], '  '	; Hide switch

	jmp	ContinueParse

;SetDisables:

ExitParseCmd:

	ret

; The following subroutine sets the zero flag if the character in 
; the AL register is one of DOS' six delimiter characters, 
; otherwise the zero flag is returned clear. This allows us to use 
; the JE/JNE instructions afterwards to test for a delimiter.

TestDelimiter:
	lodsb			;Make sure next char is
	stosb
	cmp	al, " "
	jz	ItsOne
	cmp	al, 0dh
ItsOne:	ret

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

