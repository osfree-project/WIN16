
;--- implements Win16 kernel emulation
;--- best viewed with TABSIZE 4

	; MacroLib
	include dos.inc
	include dpmi.inc

	; Kernel macros
	include macros.inc

if ?REAL
		.8086
else
		.286
endif
	option casemap:none
	option proc:private

if ?32BIT
?LARGEALLOC	equ 0	;always 0, not needed for 32-bit
else
?LARGEALLOC	equ 1	;1=allow more than 1 MB with GlobalAlloc/Realloc/Free
endif

public eWinFlags

externdef pascal _hmemset:far
externdef discardmem:near

; Long versions of string functions
externdef pascal lstrcmp: far
externdef pascal lstrcpy:far
externdef pascal lstrcat:far
externdef pascal lstrlen:far
externdef pascal lstrcpyn:far
; Atoms
externdef pascal InitAtomTable:far
externdef pascal FindAtom:far
externdef pascal AddAtom:far
externdef pascal DeleteAtom:far
externdef pascal GetAtomName:far
externdef pascal GetAtomHandle:far
; Profiles
externdef pascal GetProfileInt:far
externdef pascal GetProfileString:far
externdef pascal WriteProfileString:far
externdef pascal GetPrivateProfileInt:far
externdef pascal GetPrivateProfileString:far
externdef pascal WritePrivateProfileString:far
; ANSI
externdef pascal AnsiNext:far
externdef pascal AnsiPrev:far
externdef pascal AnsiUpper:far
externdef pascal AnsiLower:far

externdef pascal IsRomModule: far
externdef pascal IsRomFile: far
externdef pascal GetWindowsDirectory: far
externdef pascal GetSystemDirectory: far

; Global Heap
externdef pascal GlobalSize: far
externdef pascal GlobalDOSAlloc: far
externdef pascal GlobalDOSFree: far
externdef pascal GlobalReAlloc: far
externdef pascal GlobalAlloc: far
externdef pascal GlobalFree: far
externdef pascal GlobalLock: far
externdef pascal GlobalUnlock: far
externdef pascal GlobalFix: far
externdef pascal GlobalUnfix: far
externdef pascal GlobalHandle: far
externdef pascal GlobalCompact: far

externdef pascal GetFreeSpace: far

; Local Heap
externdef pascal LocalAlloc: far
externdef pascal LocalReAlloc: far
externdef pascal LocalFree: far
externdef pascal LocalInit: far
externdef pascal LocalLock: far
externdef pascal LocalUnlock: far
externdef pascal LocalSize: far
externdef pascal LocalCompact: far

externdef pascal Catch: far
externdef pascal Throw: far

externdef pascal IsTask: far

; Selectors
externdef pascal AllocSelector: far
externdef pascal FreeSelector: far
externdef pascal GetSelectorBase: far
externdef pascal SetSelectorBase: far
externdef pascal AllocSelectorArray: far
externdef pascal GetSelectorLimit: far
externdef pascal SetSelectorLimit: far
externdef pascal PrestoChangoSelector: far
externdef pascal AllocDSToCSAlias: far
externdef pascal AllocCSToDSAlias: far


_TEXT segment word public 'CODE'
_TEXT ends
CCONST segment word public 'CODE'
CCONST ends
_DATA segment word public 'DATA'
_DATA ends

	include ascii.inc
	include fixups.inc
	include dpmildr.inc
	include kernel16.inc
	include debug.inc
	include debuger.inc
	include version.inc
	include pusha.inc

_ITEXT segment word public 'DATA'	;use 'DATA' (OPTLINK bug)
_ITEXT ends

DGROUP group _TEXT,CCONST,_DATA,_ITEXT

	assume CS:DGROUP
	assume DS:DGROUP
	assume SS:NOTHING
	assume ES:DGROUP


if ?32BIT
	.386
endif

_TEXT segment


GetDOSEnvironment proc far pascal
	GET_PSP
	mov es,bx
	mov dx,es:[002Ch]
	xor ax,ax
	ret
GetDOSEnvironment endp

UnlockSegment proc far pascal uSegment:word
UnlockSegment endp

LockSegment proc far pascal uSegment:word

	mov ax,uSegment
	ret

LockSegment endp

IsTaskLocked proc far pascal
	xor ax,ax
	ret
IsTaskLocked endp

UndefDynlink proc far pascal
	@strout_err <"Unresolved import called",lf>
	jmp FatalExit
UndefDynlink endp

FatalAppExit proc far pascal
	@loadbx
	@loadparm 2,ds
	@loadparm 0,bx
	@stroutbx
FatalAppExit endp

FatalExit proc far pascal
	@SetKernelDS
	@strout_err <lf,"Fatal exit from application",lf>
	@Exit RC_FATAL
FatalExit endp

GetVersion proc far pascal
	@GetVer
	mov dx,ax
	xchg dh,dl
	mov ax,0A03h		; Windows 3.10
	ret
GetVersion endp

WaitEvent proc far pascal hTask:word
	ret
WaitEvent endp

GetWinFlags proc far pascal
	mov ax,cs:[eWinFlags.wOfs]
	ret
GetWinFlags endp

GetExePtr proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	call checkne
	jnc ismodule
	push si
	mov si,ax
	call Segment2ModuleFirst
	pop si
ismodule:
	@return
GetExePtr endp

if ?32BIT
GetModuleHandle proc far pascal uses ds esi lpszModuleName:far ptr BYTE
else
GetModuleHandle proc far pascal uses ds si lpszModuleName:far ptr BYTE
endif

	mov si,word ptr lpszModuleName+0
	mov ax,word ptr lpszModuleName+2
if ?32BIT
	movzx esi,si	;SearchModule16 will use ESI in 32bit
endif				;but this proc is for NE-Dlls only
	mov bx,cs:[wMDSta]
	push bx			;the first entry should be kernel
	mov ds,ax
	and ax,ax
	jz @F
	call strlen
@@:
	call SearchModule16
	pop dx
	ret

GetModuleHandle endp

;--- GetModuleFileName(hInstance,lpszFileName,maxlen)
;--- hInstance may be a module handle or an instance handle

GetModuleFileName proc far pascal uses ds si di hInst:word, lpszFileName:far ptr BYTE, uMax:word

	push hInst
	call GetExePtr
	and ax, ax
	jz done
	mov ds, ax
	les di, lpszFileName
	mov cx, uMax
	mov si,offset NEHDR.szModPath
	push cx
@@:
	lodsb
	stosb
	and al,al
	loopnz @B
	pop ax
	sub ax,cx
	dec ax
done:
	ret
GetModuleFileName endp

GetModuleUsage proc far pascal
	pop cx
	pop dx
	pop es
	push dx
	push cx
	mov ax,es:[0002]
	@return
GetModuleUsage endp

DebugBreak proc far pascal
	int 3
	ret
DebugBreak endp


;*** OutputDebugString ***

OutputDebugString proc far pascal uses ds si pszString:far ptr BYTE

	lds si,pszString
	mov ax,0012h
	int 41h
	ret
OutputDebugString endp

Dos3Call proc far pascal
	int 21h
	ret
Dos3Call endp

NetBiosCall proc far pascal
	int 5ch
	ret
NetBiosCall endp

SetErrorMode proc far pascal
	pop cx
	pop ax
	pop dx
	push ax
	push cx
	call _SetErrorMode
	mov ax, dx
	@return
SetErrorMode endp

;--- 

LoadModule proc far pascal uses ds lpszModuleName:far ptr byte, lpParameterBlock:far ptr

	@SetKernelDS
	mov [fLoadMod],1	;use a asciiz command line
	lds dx, lpszModuleName
	les bx, lpParameterBlock
if ?32BIT
	movzx ebx,bx
	movzx edx,dx
endif
	@Exec
	@SetKernelDS
	mov [fLoadMod],0
	ret
LoadModule endp

LoadLibrary proc far pascal uses ds lpszLibrary:far ptr byte
	lds dx, lpszLibrary
	xor bx,bx
	mov es,bx
if ?32BIT
	movzx edx, dx
	movzx ebx, bx
endif
	@Exec
	ret
LoadLibrary endp

;void FreeLibrary(hModule);

FreeLibrary proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	call FreeLib16	;C if error
	ret
FreeLibrary endp

;--- GetProcAddress(hInst,lpszProcName)

GetProcAddress proc far pascal uses ds hInst:word, lpszProcName:far ptr byte

	mov bx,hInst
	lds dx,lpszProcName
	xor cx,cx
	mov ax,ds
	and ax,ax
	jnz @F
	inc cx
@@:
	call GetProcAddress16
	ret

GetProcAddress endp

GetCurrentTask proc far pascal
	GET_PSP
	mov ax,bx
	ret
GetCurrentTask endp

GetCurrentPDB proc far pascal
	GET_PSP
	mov ax,bx
	mov dx,cs:[TH_TOPPDB]
	ret
GetCurrentPDB endp

_lclose proc far pascal
	@loadbx
	@loadparm 0,bx
	@ClosFil
	jnc @F
	mov ax,-1
@@:
	@return 2
_lclose endp

_lread proc far pascal
	@loadbx
	push ds
	@loadparm 0,cx
	@loadparm 2,dx
	@loadparm 4,ds
	@loadparm 6,bx
	@Read
	jnc @F
	mov ax,-1
@@:
	pop ds
	@return 8
_lread endp

_lwrite proc far pascal
	@loadbx
	push ds
	@loadparm 0,cx
	@loadparm 2,dx
	@loadparm 4,ds
	@loadparm 6,bx
	@Write
	jnc @F
	mov ax,-1
@@:
	pop ds
	@return 8
_lwrite endp

_llseek proc far pascal
	@loadbx
	@loadparm 0,al
	@loadparm 2,dx
	@loadparm 4,cx
	@loadparm 6,bx
	@MovePtr
	jnc @F
	mov ax,-1
@@:
	@return 8
_llseek endp

_lopen proc far pascal
	@loadbx
	push ds
	@loadparm 0,al
	@loadparm 2,dx
	@loadparm 4,ds
	@OpenFil
	jnc @F
	mov ax,-1
@@:
	pop ds
	@return 6
_lopen endp

_lcreat proc far pascal
	@loadbx
	push ds
	@loadparm 0,al
	@loadparm 2,dx
	@loadparm 4,ds
	@MakFil
	jnc @F
	mov ax,-1
@@:
	pop ds
	@return 6
_lcreat endp

;*** InitTask - this should be called by DPMI16 apps only.
;*** DPMI16 may be splitted to RTM and Win16 compatibles.
;*** this makes the initialization a bit confusing
;*** register values on entry:
;*** BX: Stacksize (16 bit version only)
;*** CX: Heapsize (16 bit version only)
;*** DI: might be Instance handle (== DGROUP)
;*** SI: 
;*** ES: PSP
;*** DS: DGROUP or PSP (if RTM compatible)
;*** SS: DGROUP
;*** SP: top of Stack
;
;From https://devblogs.microsoft.com/oldnewthing/20071203-00/?p=24323
;
;AX	zero (used to contain even geekier information in Windows 2)
;BX	stack size
;CX	heap size
;DX	unused (reserved)
;SI	previous instance handle
;DI	instance handle
;BP	zero (for stack walking)
;DS	application data segment
;ES	selector of program segment prefix
;SS	application data segment (SS=DS)
;SP	top of stack
;
;
;*** Out: CX=stack limit
;*** SI=0 (previous instance)
;*** DI=module Handle
;*** ES=PSP
;*** ES:BX=CmdLine
;
;

InitTask proc far pascal uses ds

	@trace_s <"InitTask enter",lf>
	mov ax,ss		;RTM compatibles may have DS == PSP
	mov ds,ax
	mov ax,sp
	add ax,2*3		;account for DS,IP,CS
	mov dx,ax
	sub dx,bx
	add dx,60h
	cmp word ptr ds:[0004],5 ; What is this? Why skip if SS equal to 5?
	jnz @F
;INSTANCEDATA
	mov ds:[000Ah],dx	;stack bottom
	mov ds:[000Ch],ax
	mov ds:[000Eh],ax	;stack top
@@:
	push dx

	jcxz @F			; No local heap
	push ds			;data segment
	xor ax,ax
	push ax			;start
	push cx			;end
	push cs
	call near ptr LocalInit	;preserves ES
@@:

	call InitDlls
	pop cx		;stack limit
	mov ax,0
	jc error
	mov bx,0081h
	mov dx,1	;cmdshow?
	mov ax,es
	xor si,si	;previous instance
error:
exit:
	@trace_s <"InitTask exit",lf>
	ret

InitTask endp

_TEXT ends

_ITEXT segment

SetWinFlags proc
	@DPMI_GETVERSION			;get CPU
	mov ah,byte ptr [wEquip]
	and ah,2				;FPU?
	shl ah,1
	mov al,1
	dec cl
	cmp cl,3
	jbe @F
	mov cl,3
@@:
	shl al,cl				;processor (2=286,4=386,8=486)
	or al,WF_PMODE or WF_STANDARD		;@todo depends on compilation mode
	mov [eWinFlags.wOfs],ax
	ret
SetWinFlags endp

if 0
SetProcAddress:
	push bx				 ;offset of procedure
	call GetProcAddr16	 ;search entry AX in module ES
	mov bx,cx
	pop ax
	jc @F
	mov es:[bx].ENTRY.wOfs,ax
	clc
@@:
	ret
endif

segments label word
ife ?32BIT
	dw eA000, 0A000h
	dw eB000, 0B000h
	dw eB800, 0B800h
	dw e0000, 00000h
	dw eF000, 0F000h
	dw eROMBIOS, 0F000h
	dw eC000, 0C000h
	dw eD000, 0D000h
	dw eE000, 0E000h
endif
SIZESEGS equ ($ - segments) / 4


InitKernel_ proc public
	@push_a
	mov KernelNE.ne_cseg, 1
	mov KernelNE.ne_segtab, KernelSeg -  KernelNE
	mov KernelNE.ne_restab, KernelNames - KernelNE
	mov KernelSeg.wSel, cs

	call SetWinFlags

if SIZESEGS
	mov si,offset segments
	mov cx,SIZESEGS
nextseg:
	lodsw
	push ax
	lodsw
	xchg bx,ax
	@DPMI_Seg2Desc		;alloc rm selector
	pop bx
	jc @F
	mov [bx].ENTRY.wOfs,ax
@@:
	loop nextseg
endif

	@DPMI_GetIncValue	   ;get AHINC value
	jc @F
	mov [eINCR.wOfs],ax
@@:

if ?MEMFORKERNEL
	xor bx,bx
	mov cx,1000h
	@DPMI_ALLOCMEM
	jc exit
	mov word ptr KernelNE.MEMHDL+0,si
	mov word ptr KernelNE.MEMHDL+2,di
	push bx
	push cx
	@DPMI_AllocDesc
	pop dx
	pop cx
	jc exit
	mov bx,ax
else
	@DPMI_AllocDesc
	jc exit
	push ax
	mov bx,cs
	@DPMI_GetBase
	pop bx
	add dx,offset KernelNE
	adc cx,0
endif
	@DPMI_SetBase
	mov dx,(EndKernelNE - KernelNE) - 1
	or dl,0Fh
	xor cx,cx
	@DPMI_SetLimit
	mov es,bx
	mov [wMDSta],bx
if ?MEMFORKERNEL
	xor di,di
	mov si,offset KernelNE
	mov cx,EndKernelNE - KernelNE
	rep movsb
endif
	clc
exit:
	@pop_a
	ret
InitKernel_ endp

_ITEXT ends


_DATA segment

KernelNE NEHDR <"EN", 1, KernelEntries - KernelNE, 0, 0, NEHDR.szModPath - 8, 0, AF_DLL or AF_INIT>
        db 79 dup (0)

externdef _end:abs

KernelSeg SEGITEM <0,_end, 0, _end>

KernelEntries label byte
	db 1,1
	ENTRY <1,FatalExit>
	db 1,0
	db 8,1
	ENTRY <1,GetVersion>		;3
	ENTRY <1,LocalInit>			;4
	ENTRY <1,LocalAlloc>
	ENTRY <1,LocalReAlloc>
	ENTRY <1,LocalFree>
	ENTRY <1,LocalLock>
	ENTRY <1,LocalUnlock>
	ENTRY <1,LocalSize>			;10
	db 2,0
	db 1,1
	ENTRY <1,LocalCompact>		;13
	db 1,0
	db 7,1
	ENTRY <1,GlobalAlloc>		;15
	ENTRY <1,GlobalReAlloc>
	ENTRY <1,GlobalFree>
	ENTRY <1,GlobalLock>
	ENTRY <1,GlobalUnlock>
	ENTRY <1,GlobalSize>
	ENTRY <1,GlobalHandle>		;21
	db 1,0
	db 3,1
	ENTRY <1,LockSegment>		;23
	ENTRY <1,UnlockSegment>
	ENTRY <1,GlobalCompact>		;25
	db 4,0						;26-29
	db 1,1
	ENTRY <1,WaitEvent>			;30
	db 5,0
	db 2,1
	ENTRY <1,GetCurrentTask>	;36
	ENTRY <1,GetCurrentPDB>		;37
	db 7,0						;38-44
	db 1,1
	ENTRY <1,LoadModule>		;45
	db 1,0						;46
	db 4,1
	ENTRY <1,GetModuleHandle>	;47
	ENTRY <1,GetModuleUsage>
	ENTRY <1,GetModuleFileName>
	ENTRY <1,GetProcAddress>	;50
	db 4,0						;51-54
	db 5,1
	ENTRY <1, Catch>            ;55
    	ENTRY <1, Throw>            ;56
	ENTRY <1, GetProfileInt>			;57
	ENTRY <1, GetProfileString>			;58
	ENTRY <1, WriteProfileString>			;59
	db 8,0						;60-67
	db 6,1
	ENTRY <1, InitAtomTable>			;68
	ENTRY <1, FindAtom>				;69
	ENTRY <1, AddAtom>				;70
	ENTRY <1, DeleteAtom>				;71
	ENTRY <1, GetAtomName>				;72
	ENTRY <1, GetAtomHandle>			;73
	db 3,0						;74-76
	db 15,1
	ENTRY <1,AnsiNext>		; 77
	ENTRY <1,AnsiPrev>		; 78
	ENTRY <1,AnsiUpper>		; 79
	ENTRY <1,AnsiLower>		; 80
	ENTRY <1,_lclose>			;81
	ENTRY <1,_lread>
	ENTRY <1,_lcreat>
	ENTRY <1,_llseek>
	ENTRY <1,_lopen>
	ENTRY <1,_lwrite>			;86
	ENTRY <1,lstrcmp>			;87
	ENTRY <1,lstrcpy>			;88
	ENTRY <1,lstrcat>			;88
	ENTRY <1,lstrlen>			;90
	ENTRY <1,InitTask>			;91
	db 3,0						;92-94
	db 2,1
	ENTRY <1,LoadLibrary>
	ENTRY <1,FreeLibrary>		;96
	db 5,0						;97-101
	db 2,1
	ENTRY <1,Dos3Call>			;102
	ENTRY <1,NetBiosCall>			;103
	db 3,0						;104-106
	db 1,1
	ENTRY <1,SetErrorMode>		;107
	db 5,0						;108-112
	db 2,-2
eSHIFT	ENTRY <1,3>				;113 _AHSHIFT
eINCR	ENTRY <1,8>				;114 _AHINCR
	db 1,1
	ENTRY <1,OutputDebugString>	;115
	db 4,0						;116-119
	db 1,1
	ENTRY <1,UndefDynlink>		;120
	db 1,0						;121
	db 1,1
	ENTRY <1,IsTaskLocked>		;122
	db 4,0						;123-126
	db 3,1
	ENTRY <1,GetPrivateProfileInt>	;127
	ENTRY <1,GetPrivateProfileString>	;128
	ENTRY <1,WritePrivateProfileString>	;129
	db 1,0						;130
	db 3,1
	ENTRY <1,GetDOSEnvironment>	;131
	ENTRY <1,GetWinFlags>		;132
	ENTRY <1,GetExePtr>			;133
	ENTRY <1,GetWindowsDirectory>           ;134
	ENTRY <1,GetSystemDirectory>			;135
	db 1,0						;136
	db 1,1
	ENTRY <1,FatalAppExit>		;137
	db 31,0						;138-168
	db 1,1
	ENTRY <1,GetFreeSpace>		;169
	db 2,1
	ENTRY <1,AllocCSToDSAlias>
	ENTRY <1,AllocDSToCSAlias>
	db 1,0						;172
	db 2,-2
eROMBIOS ENTRY <1,0>				;173 _ROMBIOS
eA000 ENTRY <1,00h>				;174 _A000H
	db 3,1
	ENTRY <1,AllocSelector>		;175
	ENTRY <1,FreeSelector>
	ENTRY <1,PrestoChangoSelector>	;177
	db 1,-2
eWinFlags ENTRY <1,0>			;178 __WINFLAGS

	db 1,-2
eD000 ENTRY <1,0>			;179
	db 1,0				;180
	db 3,-2
eB000 ENTRY <1,0>				;181 _B000H
eB800 ENTRY <1,0>				;182 _B800H
e0000 ENTRY <1,0>				;183 _0000H
	db 6,1
	ENTRY <1,GlobalDOSAlloc>	;184
	ENTRY <1,GlobalDOSFree>		;185
	ENTRY <1,GetSelectorBase>	;186
	ENTRY <1,SetSelectorBase>	;187
	ENTRY <1,GetSelectorLimit>	;188
	ENTRY <1,SetSelectorLimit>	;189
	db 1,-2
eE000 ENTRY <1,0>				;190 _E000H
	db 2,0						;191-192
	db 3,-2
e0040 ENTRY <1,0040h>			;193
eF000 ENTRY <1,0>				;194 _F000H
eC000 ENTRY <1,0>				;195 _C000H
	db 1,0						;196
	db 2,1
	ENTRY <1,GlobalFix>			;197
	ENTRY <1,GlobalUnfix>		;198
	db 4,0						;199-202
	db 1,1
	ENTRY <1,DebugBreak>		;203
	db 2,0						;204-205
	db 1,1
	ENTRY <1,AllocSelectorArray>;206
	db 113,0					;207-319
	db 1,1
	ENTRY <1, IsTask>		; 320
	db 2,0					;321-322
	db 1,1
	ENTRY <1,IsRomModule>		; 323
	db 2,0						;324-325
	db 1,1
	ENTRY <1,IsRomFile>		; 326
	db 26,0						;327-352
	db 1,1
	ENTRY <1,lstrcpyn>		; 353
	db 0

NENAME macro name, export
local x1,x2
	db x2 - x1
x1	equ $
	db name
x2	equ $
	dw export
	endm

KernelNames label byte
	NENAME "KERNEL"    ,0
	NENAME "FATALEXIT" ,1
	NENAME "GETVERSION",3
	NENAME "LOCALINIT"   ,4
	NENAME "LOCALALLOC"  ,5
	NENAME "LOCALREALLOC",6
	NENAME "LOCALFREE"   ,7
	NENAME "LOCALLOCK"   ,8
	NENAME "LOCALUNLOCK" ,9
	NENAME "LOCALSIZE"   ,10
	NENAME "LOCALCOMPACT",13
	NENAME "GLOBALALLOC"  ,15
	NENAME "GLOBALREALLOC",16
	NENAME "GLOBALFREE"   ,17
	NENAME "GLOBALLOCK"   ,18
	NENAME "GLOBALUNLOCK" ,19
	NENAME "GLOBALSIZE"   ,20
	NENAME "GLOBALHANDLE" ,21
	NENAME "LOCKSEGMENT"  ,23
	NENAME "UNLOCKSEGMENT",24
	NENAME "GLOBALCOMPACT",25
	NENAME "WAITEVENT"        ,30
	NENAME "GETCURRENTTASK"   ,36
	NENAME "GETCURRENTPDB"    ,37
	NENAME "LOADMODULE"       ,45
	NENAME "GETMODULEHANDLE"  ,47
	NENAME "GETMODULEUSAGE"   ,48
	NENAME "GETMODULEFILENAME",49
	NENAME "GETPROCADDRESS"   ,50
	NENAME "CATCH"            ,55
    	NENAME "THROW"            ,56
	NENAME "GETPROFILEINT", 57
	NENAME "GETPROFILESTRING", 58
	NENAME "WRITEPROFILESTRING", 59
	NENAME "INITATOMTABLE", 68
	NENAME "FINDATOM", 69
	NENAME "ADDATOM", 70
	NENAME "DELETEATOM", 71
	NENAME "GETATOMNAME", 72
	NENAME "GETATOMHANDLE", 73
	NENAME "ANSINEXT",77
	NENAME "ANSIPREV",78
	NENAME "ANSIUPPER",79
	NENAME "ANSILOWER",80
	NENAME "_LCLOSE",81
	NENAME "_LREAD" ,82
	NENAME "_LCREAT",83
	NENAME "_LLSEEK",84
	NENAME "_LOPEN" ,85
	NENAME "_LWRITE",86
	NENAME "LSTRCMP",87
	NENAME "LSTRCPY",88
	NENAME "LSTRCAT",89
	NENAME "LSTRLEN",90
	NENAME "INITTASK"    ,91
	NENAME "LOADLIBRARY" ,95
	NENAME "FREELIBRARY" ,96
	NENAME "DOS3CALL"    ,102
	NENAME "NETBIOSCALL" ,103
	NENAME "SETERRORMODE",107
	NENAME "__AHSHIFT"   ,113
	NENAME "__AHINCR"    ,114
	NENAME "OUTPUTDEBUGSTRING", 115
	NENAME "UNDEFDYNLINK",      120
	NENAME "ISTASKLOCKED",      122
	NENAME "GETPRIVATEPROFILEINT"  ,127
	NENAME "GETPRIVATEPROFILESTRING"  ,128
	NENAME "WRITEPRIVATEPROFILESTRING",129
	NENAME "GETDOSENVIRONMENT", 131
	NENAME "GETWINFLAGS"      , 132
	NENAME "GETEXEPTR"        , 133
	NENAME "GETWINDOWSDIRECTORY"            ,134
	NENAME "GETSYSTEMDIRECTORY"            ,135
	NENAME "FATALAPPEXIT"     , 137
	NENAME "GETFREESPACE"     , 169
	NENAME "ALLOCCSTODSALIAS" , 170
	NENAME "ALLOCDSTOCSALIAS" , 171
	NENAME "__ROMBIOS", 173
	NENAME "__A000H", 174
	NENAME "ALLOCSELECTOR"       , 175
	NENAME "FREESELECTOR"        , 176
	NENAME "PRESTOCHANGOSELECTOR", 177
	NENAME "__WINFLAGS"          , 178
	NENAME "__D000H", 179
	NENAME "__B000H", 181
	NENAME "__B800H", 182
	NENAME "__0000H", 183
	NENAME "GLOBALDOSALLOC"   ,184
	NENAME "GLOBALDOSFREE"    ,185
	NENAME "GETSELECTORBASE"  ,186
	NENAME "SETSELECTORBASE"  ,187
	NENAME "GETSELECTORLIMIT" ,188
	NENAME "SETSELECTORLIMIT" ,189
	NENAME "__E000H", 190
	NENAME "__0040H"          ,193
	NENAME "__F000H", 194
	NENAME "__C000H", 195
	NENAME "GLOBALFIX"          ,197
	NENAME "GLOBALUNFIX"        ,198
	NENAME "DEBUGBREAK"         ,203
	NENAME "ALLOCSELECTORARRAY" ,206
	NENAME "ISTASK" ,320
	NENAME "ISROMMODULE" ,323
	NENAME "ISROMFILE" ,326
	NENAME "LSTRCPYN" ,353
	db 0

EndKernelNE equ $

_DATA ends

	end
