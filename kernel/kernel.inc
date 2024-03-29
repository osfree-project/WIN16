		; MacroLib
		include dpmi.inc

@use16	textequ <>

_TEXT	segment dword @use16 public 'CODE'
_TEXT	ends
CCONST	segment word @use16 public 'CODE'
CCONST	ends
_DATA	segment word @use16 public 'DATA'
_DATA	ends
_BSS	segment word @use16 public 'BSS'
_BSS	ends
STACK	segment para @use16 stack  'STACK'
STACK	ends

DGROUP	group _TEXT,CCONST,_DATA,_BSS,STACK

	assume CS:DGROUP
	assume DS:DGROUP
	assume SS:NOTHING
	assume ES:DGROUP


;********** global equates **********

?MULTPSP		= 1		;1, create a PSP for every task


ifndef _FILEHANDLES_
_FILEHANDLES_	= 20	;20 initial no of file handles for a task
endif
?INT24RES		= 1		;1, restore int 24 when task exits
?INT23RES		= 1		;1, restore int 23
?INT41SUPPORT	= 1		;1, support Int 41h debugging messages
?SETDTA			= 1		;1, 1=for ?MULTPSP: init DTA to PSP:80 on start
?ZEROHIWORDEBP  = 1		;1, 1=if ?32BIT clear hiword(ebp) before 16-bit proc is called.
                        ;   this is essentially for ENTER/LEAVE instructions!
?32RTMBUG		= 0		;0, 1=repair 32RTM.exe bugs (doesn't work since
						;   32RTM doesn't know int 21h, ah=00, and I won't
                        ;   implement this function by hand)
?MEMFORKERNEL   = 0     ;0 1=alloc extra memory for KERNEL

if ?32BIT                        
?DOS4G			= 1		;1, 1=support DOS4G                        
?DOS4GMEM		= 0		;0, 1=support DOS4G data segment in GS
else
?DOS4G			= 0
endif



?MAXPATH	equ 65
?MAXNAME	equ 13		;8+3 + dot + terminating 0

if ?DEBUG
_TRACE_ 		= 1
else
_TRACE_			= 0
endif

HBSEM_NOOPENFILEBOX	equ 80h	;high byte of wErrMode (=SEM_NOOPENFILEBOX)

;*** flags from environment variable DPMILDR

ENVFL_BREAKATENTRY  equ 01h	;debug break at program entry
ENVFL_DONTLOADHIGH  equ 02h	;dont move in extended memory
ENVFL_DISABLENE32   equ 04h	;disable support for loading NE32 apps
ENVFL_LOAD1APPONLY  equ 08h	;disable loader after first pm app has started
ENVFL_IGNNOOPENERR	equ 40h	;64   ignore SEM_NOOPENFILEERRORBOX flag
ENVFL2_LOADDBGOUT	equ 01h	;256  try to load debugout.dll /debugo32.dll

;--- fMode flags

FMODE_DISABLED	equ 02h	;bit 1: 1=loader is disabled
FMODE_DOSEMU	equ 04h	;bit 2: 1=running on DOSEMU
FMODE_NOERRDISP	equ 20h	;bit 5: 1=temporarily suppress dpmi error messages

;*** exits:
;*** 4CFB: error in loader Init (real mode)
;*** 4CFC: error in loader init (prot mode)
;*** 4CFD: error in exception 0B (segment load)
;*** 4CFE: error in app Init (prot mode)
;*** 4CFF: fatal exit
;***

RC_INITRM  equ 0FBh	;init loader real mode
RC_INITPM  equ 0FCh	;init loader protected mode
RC_EXC0B   equ 0FDh	;error in exc 0b handler (usually 16bit only)
RC_INITAPP equ 0FEh	;error in init application
RC_FATAL   equ 0FFh	;FatalExit/FatalAppExit called (16bit only)



;********** publics/externals **********

_DATA segment
externdef	pascal wKernelDS:word
externdef	wTDStk:word
externdef	starttaskstk:byte
externdef	TH_HEXEHEAD:word
externdef	TH_HEADTDB:word
externdef	TH_CURTDB:word
externdef	wLastRC:word
externdef	bEnvFlgs:byte
externdef	bEnvFlgs2:byte
externdef	wEnvFlgs:word
externdef	wErrMode:word
externdef	szPath:byte
externdef	segtable:byte
externdef	fMode:byte
externdef	TH_TOPPDB:word
externdef	pascal wCurPSP:word
if ?32BIT
externdef	dStktop:dword
else
externdef	wStktop:word
endif
externdef KernelFlags: WORD
externdef PMouseTermProc: DWORD
externdef PKeyboardTermProc: DWORD
externdef PSystemTermProc: DWORD
externdef PrevInt21Proc: DWORD
_DATA ends

_TEXT segment
SearchNEExport	proto near
checkne			proto near
strlen			proto near
Segment2Module	proto near
SearchModule16	proto near
;GetModuleHandle proto far pascal :far ptr byte
GetModuleHandle16 proto near
_SetErrorMode	proto near
SetCmdLine		proto near
_mycrout		proto near
CreatePsp		proto near
openfile        proto near
saveint2x		proto near
printchar		proto near
stroutstk		proto near
doscall			proto near
_dwordout		proto near		;used if trace on
_wordout		proto near		;used if trace on
if ?32BIT
externdef DWORDOUT:near
endif
externdef Segment2ModuleFirst:near
externdef GetProcAddress16:near
CallAllLibEntries proto near
FreeLib16       proto near
CreateAlias     proto near
CopyDescriptor  proto near
stroutBX        proto near
InitDlls        proto near
externdef stroutstk_err:near
externdef pascal LoadModule:far
_TEXT ends

;********** constants and structures ***********

;*** module database (MD)
;*** bytes 0x00-0x3F are copied from file NE-Header

NEHDR   struct
ne_magic	dw	?		;+00: contains "NE"
ne_count	dw	?		;+02: module count
ne_enttab	dw	?		;+04: ^ Entries
ne_cbenttab dw	?		;+06: length entry table (will be overwritten)
DGROFS		dw	?		;+08: offset in segment table for DGROUP
FINFO		dw	?		;+0A: FileInfo (not used)
PGMFLGS		db	?		;+0C: program flags
APPFLGS		db	?		;+0D: application flags
ne_autodata	dw	?		;+0E: no of DGROUP segment (needn't exist!)
ne_heap		dw	?		;+10: heap space (bytes/pages)
ne_stack	dw	?		;+12: extra stack space (bytes/pages)
ne_csip		dd	?		;+14: CS:IP
ne_sssp		dd	?		;+18: SS:SP
ne_cseg 	dw	?		;+1C: number of segments
ne_cmod 	dw	?		;+1E: number of referenced modules
NRESLEN		dw	?		;+20: length of table nonres names
ne_segtab	dw	?		;+22: ^ segment table
ne_rsrctab	dw	?		;+24: ^ resource table
ne_restab   dw	?		;+26: ^ resident names table
ne_modtab	dw	?		;+28: ^ module table (=module references)
ne_imptab	dw	?		;+2A: ^ imported names table
NRESADR		dd	?		;+2C: address nonresident names (DWORD)
ne_cmovent	dw	?		;+30: moveable entry points in entry table
SEGSHFT		dw	?		;+32: shift faktor
ne_cres		dw	?		;+34: number of resource table entries
ne_exetyp	db	?		;+36: operating system
ne_flagsothers	db	?	;+37: exe flags (long filenames, prop font)
ne_pretthunks	dw	?	;+38: start of gangload area?
ne_psegrefbytes	dw	?	;+3A: length of gangload area?
ne_swaparea	dw	?		;+3C: codeswap
ne_expver	dw	?		;+3E: expected windows version
NXTFREE		dw	?		;+40: offset free space in this MD
NRESNAM		dw	?		;+42: ^ nonresident names
ne_hFile	dw	?		;+44: file handle of .EXE image
MEMHDL		dd	?		;+46: dpmi memory handle of MD (DWORD)
szModPath   db	?		;+4A: offset to binary full path
NEHDR   ends

NXTMOD  equ <ne_cbenttab>
MDCONST equ <szModPath> ;size of MD without binary full path

;--- program flags

;PF_NONE	equ 00h		;bits 0+1: 00 no DGROUP
;PF_SINGLE	equ 01h		;bits 0+1: 01 single DGROUP
PF_MULTIPLE	equ 02h		;bits 0+1: 10 multiple DGROUPs
;PF_GLOBALINIT equ 04h	;bit 2: global initialization
;PF_PROTMODE equ 08h	;bit 3: protected mode only
;PF_8086				;bit 4: 8086 instructions
;PF_80286				;bit 5: 80286 instructions
;PF_80386				;bit 6: 80386 instructions
;PF_80X87				;bit 7: 80x87 instructions

;--- application flags

;--- bits 0-2:
;--- 000=fullscreen
;--- 001=windows aware
;--- 010=uses windows API

;AF_WINAPI	equ 02h

;AF_FAMILY	equ 08h		;bit 3: OS2: family app; Win16: self-loading
;AF_      	equ 10h		;bit 4: ???
;AF_ERROR	equ 20h		;bit 5: error in image
AF_INIT		equ 40h		;bit 6: dlls: LibEntry done
AF_DLL		equ 80h		;bit 7: module is dll

;--- ne_exetyp

ET_UNKNOWN  equ 0   
ET_OS2		equ 1	
ET_WINDOWS	equ 2
ET_DOS4 	equ 3	
ET_WIN386	equ 4
ET_DPMI16	equ 5
ET_DPMI32	equ 6		;this is known by HX only

;----------------------------------------------------

;*** segment table entry ***
;*** first 4 words are in segment table of file as well

SEGITEM struct
filepos dw  ?       ;segment position in file (without shift factor)
filesiz dw  ?       ;segment size in file
flags   dw  ?       ;segment flags (see below)
memsiz  dw  ?       ;size in memory
wSel    dw  ?       ;standard selector
dwHdl   dd  ?       ;DPMI memory handle (0x0501)
wDosSel dw  ?       ;selector for DOS memory alloc (0x0100)
SEGITEM ends

SGTSIZE equ SIZE SEGITEM

;--- segment flags

SF_DATA   equ 0001h		;1=DATA, 0=CODE
SF_LOADED equ 0002h		;1=segment is loaded in memory
SF_RES1   equ 0004h
SF_ITERED equ 0008h		;1=ITERATED (OS/2 only)
SF_MOVABL equ 0010h		;1=MOVEABLE, 0=FIXED
SF_SHARED equ 0020h		;1=SHARED, 0=NONSHARED
SF_PRELOD equ 0040h		;1=PRELOAD, 0=LOADONCALL
SF_RDONLY equ 0080h		;1=READONLY, 0=R/W
SF_RELOCS equ 0100h		;1=relocations exist for this segment
SF_CONFRM equ 0200h		;1=conforming segment (OS/2 only)
SF_RES2   equ 0C00h		;00,01,02,03=priviledge level (OS/2 only)
SF_DISCRD equ 1000h		;1=DISCARDABLE, 0=NONDISCARDABLE
SF_32BIT  equ 2000h		;1=MIXED3216
SF_RES4   equ 4000h		;1=huge segment (OS/2 only)
SF_RES5   equ 8000h
SF_ALIAS  equ 8000h

;*** task structure ***
;--- missing: wErrorMode ***

TASK struct

if ?32BIT
dwESP   dd ?        ;SS:ESP of previous task
else
wSP     dw ?        ;SS:SP of previous task
endif
wSS     dw ?

if ?32BIT
union
dwModul dd ?        ;module selector/linear address
struct
wModul	dw ?
		dw ?
ends
ends
else
wModul  dw ?        ;module selector
endif

if ?MULTPSP
wPSP    dw ?        ;PSP
if ?SETDTA
if ?32BIT
dta		df ?		;dos disk transfer area
else
dta		dd ?
endif
endif
endif

if ?INT23RES
if ?32BIT
dfI23   df ?        ;previous int 23 vector
else
dwI23   dd ?        ;previous int 23 vector
endif
endif

if ?INT24RES
if ?32BIT
dfI24   df ?        ;previous int 24 vector
else
dwI24   dd ?        ;previous int 24 vector
endif
endif

if ?32BIT
wFlags	dw ?
endif

TASK ends

_TRACEUSED_ = 0   ;internal variable

;********** Macros **********

;-- define string if not already done
;-- then push string address onto stack

;!!!!!!!! ax is bad!!!
@pushString macro String, bIsDefined
local NewStr
  ifnb <bIsDefined>
if ?REAL
	push bx
	push bx
	push bp
	mov bp ,sp
	mov bx, offset String
	xchg bx,[bp+4]
	pop bp
	pop bx
else  
    push offset String
endif
  else
CCONST   segment
NewStr  db String
        db 00
CCONST   ends
if ?REAL
	push bx
	push bx
	push bp
	mov bp, sp
	mov bx, offset NewStr
	xchg bx,[bp+4]
	pop bp
	pop bx
else  
    push offset NewStr
endif
  endif
	endm

;-- display a string (define it if not already done)

@strout macro String,bIsDefined
	@pushString <String>, bIsDefined
	call stroutstk
	endm

;--- display string in BX

@stroutbx macro
	call stroutBX
	endm

;--- display an error (string as constant or in BX)

@strout_err macro String,bIsDefined
	ifnb <String>
		@pushString <String>,bIsDefined
		call stroutstk_err
	else
		call strout_err
	endif
	endm

;--- display a new line

@cr_out macro
	call _mycrout		;;std routine doesnt work with 32bit stack
	endm


@swap macro x,y
	push x
	push y
	pop x
	pop y
	endm

;--- @saveregs, @restoreregs

@saveregs_exec macro
if ?32BIT
	push gs
	push es
	push ds
	pushad
?SIZE_SAVEREGS equ 8*4+3*2
else
	push es
	push ds
	@push_a
?SIZE_SAVEREGS equ 8*2+2*2
endif
	endm

@restoreregs_exec macro
if ?32BIT
	popad	;get register from application stack
	pop ds
	pop es
	pop gs
else
	@pop_a
	pop ds
	pop es
endif
	endm

;--- @iret, @retf, @pusha, @popa

@iret macro
if ?32BIT
	iretd
else
	iret
endif
	endm

@retf macro
if ?32BIT
	db 66h
endif
	retf
	endm

include pusha.inc

@pusha macro
if ?32BIT
	pushad
else
	@push_a
endif
	endm

@popa macro
if ?32BIT
	popad
else
	@pop_a
endif
	endm

;-------------------------

@entercriticalsection macro
if 1
	@DPMI_DisableInt
	push ax
else
	push bp
	mov bp,sp
	push ax
	in al,21h
	push ax
	mov al,0FFh
	out 21h,al
	mov ax,[bp-2]
endif
	endm

@exitcriticalsection macro
local l1
if 1
;	pop bx
;	test bh,2
;	jz l1
;	sti
	mov cx,ax
	pop ax
	int 31h
	mov ax,cx
l1:
else
	mov [bp-2],ax
	pop ax
	out 21h,al
	pop ax
endif
	endm


if ?32BIT
	.386
endif

@return  macro xx
	retf xx
	endm

@loadbx macro
if ?32BIT
	mov ebx,esp
else
	mov bx,sp
endif
	endm

@loadparm macro ofs,xx
if ?32BIT
	mov xx,ss:[ebx+4+ofs]
else
	mov xx,ss:[bx+4+ofs]
endif
	endm

ENTRY struct
bSegm	db ?
wOfs	dw ?
ENTRY ends

WF_PMODE	equ 1
WF_CPU286	equ 2
WF_CPU386	equ 4
WF_CPU486	equ 8
WF_STANDARD	equ 10h
WF_ENHANCED	equ 20h
WF_80x87	equ 0400h
WF_PAGING	equ 0800h

ENVIRON equ 2Ch

@SetKernelDS	macro segm
		ifnb <segm>
			mov segm,cs:[wKernelDS]
			assume segm:DGROUP
		else
			mov ds,cs:[wKernelDS]
			assume ds:DGROUP
		endif
		endm


_DATA segment
externdef KernelNE:NEHDR
_DATA ends

_TEXT segment

InitKernel proto near

_TEXT ends
