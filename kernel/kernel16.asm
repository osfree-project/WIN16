
;--- implements Win16 kernel emulation
;--- best viewed with TABSIZE 4

	; MacroLib
	include dos.inc
	include dpmi.inc

	; Kernel macros
	include kernel.inc

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

public pascal eWinFlags
public GetExePtr
public GetProcAddress
public GetModuleHandle
public Dos3Call
externdef pascal SetWinFlags: far
;public __AHINCR
;public __AHSHIFT

externdef pascal _lopen:far
externdef pascal _lcreat:far
externdef pascal _lclose:far
externdef pascal _lread:far
externdef pascal _lwrite:far
externdef pascal _llseek:far

; Resource manager
externdef pascal FindResource:far
externdef pascal LoadResource:far
externdef pascal LockResource:far
externdef pascal FreeResource:far
externdef pascal SizeofResource:far
externdef pascal AllocResource:far
externdef pascal AccessResource:far
externdef pascal SetResourceHandler:far
externdef pascal DirectResAlloc:far

; Task related functions
externdef pascal GetCurrentPDB:far
externdef pascal GetCurrentTask:far
externdef pascal GetDOSEnvironment:far
externdef pascal InitTask:far
externdef pascal IsWinOldApTask: far
externdef pascal GetTaskQueueES: far
externdef pascal GetTaskQueueDS: far
externdef pascal GetTaskQueue: far
externdef pascal SetTaskQueue: far
externdef pascal GetNumTasks: far
externdef pascal SetTaskSignalProc: far
externdef pascal GetTaskDS: far
externdef pascal GetCurPID: far

externdef pascal GetHeapSpaces: far

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

externdef pascal IsDBCSLeadByte: far

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
externdef pascal GlobalFlags: far
externdef pascal GlobalLock: far
externdef pascal GlobalUnlock: far
externdef pascal GlobalFix: far
externdef pascal GlobalUnfix: far
externdef pascal GlobalHandle: far
externdef pascal GlobalCompact: far
externdef pascal GlobalFreeAll: far
externdef pascal GlobalMasterHandle: far
externdef pascal GlobalWire: far
externdef pascal GlobalUnWire: far
externdef pascal GlobalNotify: far
externdef pascal LimitEMSPages: far
externdef pascal A20Proc: far

externdef pascal GetFreeSpace: far
externdef pascal GetFreeMemInfo: far

; Local Heap
externdef pascal LocalAlloc: far
externdef pascal LocalReAlloc: far
externdef pascal LocalFree: far
externdef pascal LocalInit: far
externdef pascal LocalLock: far
externdef pascal LocalUnlock: far
externdef pascal LocalSize: far
externdef pascal LocalCompact: far
externdef pascal LocalNotify: far
externdef pascal LocalFlags: far
externdef pascal LocalHandle: far
externdef pascal LocalHandleDelta: far
externdef pascal GetInstanceData: far
externdef pascal LocalHeapSize: far
externdef pascal LocalCountFree: far

externdef pascal Catch: far
externdef pascal Throw: far

externdef pascal IsTask: far
externdef pascal GetExeVersion: far
externdef pascal GetExpWinVer: far
externdef pascal SetPriority: far
externdef pascal LockCurrentTask: far
externdef pascal PostEvent: far
externdef pascal Yield: far
externdef pascal OldYield: far
externdef pascal DirectedYield: far

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
externdef pascal SelectorAccessRights: far
externdef pascal IsBadReadPtr: far
externdef pascal IsBadWritePtr: far
externdef pascal IsBadCodePtr: far
externdef pascal IsBadStringPtr: far
externdef pascal IsBadHugeReadPtr: far
externdef pascal IsBadHugeWritePtr: far
externdef pascal IsSharedSelector: far

externdef pascal GetCodeHandle: far
externdef pascal GetCodeInfo: far

externdef pascal LoadModule: far

externdef pascal LongPtrAdd: far

externdef pascal EnableDOS: far
externdef pascal DisableDOS: far

externdef pascal EnableKernel: far

externdef pascal MakeProcInstance: far
externdef pascal FreeProcInstance: far

externdef pascal KbdRst: far

	include ascii.inc
	include fixups.inc
	include debug.inc
	include debuger.inc
	include version.inc
	include pusha.inc

DGROUP group _TEXT,CCONST,_DATA

	assume CS:DGROUP
	assume DS:DGROUP
	assume SS:NOTHING
	assume ES:DGROUP


if ?32BIT
	.386
endif

_TEXT segment


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
	mov bx,cs:[TH_HEXEHEAD]
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

GetSetKernelDOSProc proc far pascal newproc:dword
	@SetKernelDS
	mov ax, word ptr [newproc+0]
	xchg ax, word ptr [PrevInt21Proc+0]
	mov dx, word ptr [newproc+2]
	xchg dx, word ptr [PrevInt21Proc+2]
GetSetKernelDOSProc endp

externdef doscall: near
NoHookDOSCall proc far pascal
	call doscall
NoHookDOSCall endp

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
	mov [TH_HEXEHEAD],bx
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

_TEXT ends


_DATA segment

KernelNE NEHDR <"EN", 1, KernelEntries - KernelNE, 0, 0, NEHDR.szModPath - 8, 0, AF_DLL or AF_INIT>
        db 79 dup (0)

externdef _end:abs

KernelSeg SEGITEM <0,_end, 0, _end>

KernelEntries label byte
	db 1,1
	ENTRY <1,FatalExit>		;1
	db 1,0
	db 24,1
	ENTRY <1,GetVersion>		;3
	ENTRY <1,LocalInit>		;4
	ENTRY <1,LocalAlloc>		;5
	ENTRY <1,LocalReAlloc>		;6
	ENTRY <1,LocalFree>		;7
	ENTRY <1,LocalLock>		;8
	ENTRY <1,LocalUnlock>		;9
	ENTRY <1,LocalSize>		;10
	ENTRY <1,LocalHandle>		;11
	ENTRY <1,LocalFlags>		;12
	ENTRY <1,LocalCompact>		;13
	ENTRY <1,LocalNotify>		;14
	ENTRY <1,GlobalAlloc>		;15
	ENTRY <1,GlobalReAlloc>		;16
	ENTRY <1,GlobalFree>		;17
	ENTRY <1,GlobalLock>		;18
	ENTRY <1,GlobalUnlock>		;19
	ENTRY <1,GlobalSize>		;20
	ENTRY <1,GlobalHandle>		;21
	ENTRY <1,GlobalFlags>		;22
	ENTRY <1,LockSegment>		;23
	ENTRY <1,UnlockSegment>		;24
	ENTRY <1,GlobalCompact>		;25
	ENTRY <1,GlobalFreeAll>		;26
	db 1,0				;27
	db 11,1
	ENTRY <1,GlobalMasterHandle>	;28
	ENTRY <1,Yield>			;29
	ENTRY <1,WaitEvent>		;30
	ENTRY <1,PostEvent>		;31
	ENTRY <1,SetPriority>		;32
	ENTRY <1,LockCurrentTask>	;33
	ENTRY <1,SetTaskQueue>		;34
	ENTRY <1,GetTaskQueue>		;35
	ENTRY <1,GetCurrentTask>	;36
	ENTRY <1,GetCurrentPDB>		;37
	ENTRY <1,SetTaskSignalProc>	;38
	db 2,0				;39-40
	db 2,1
	ENTRY <1,EnableDOS>		;41
	ENTRY <1,DisableDOS>		;42
	db 2,0				;43-44
	db 1,1
	ENTRY <1,LoadModule>		;45
	db 1,0				;46
	db 6,1
	ENTRY <1,GetModuleHandle>	;47
	ENTRY <1,GetModuleUsage>	;48
	ENTRY <1,GetModuleFileName>	;49
	ENTRY <1,GetProcAddress>	;50
	ENTRY <1,MakeProcInstance>	;51
	ENTRY <1,FreeProcInstance>	;52
	db 1,0				;53
	db 20,1
	ENTRY <1, GetInstanceData>	;54
	ENTRY <1, Catch>		;55
    	ENTRY <1, Throw>		;56
	ENTRY <1, GetProfileInt>			;57
	ENTRY <1, GetProfileString>			;58
	ENTRY <1, WriteProfileString>			;59
	ENTRY <1, FindResource>				;60
	ENTRY <1, LoadResource>				;61
	ENTRY <1, LockResource>				;62
	ENTRY <1, FreeResource>				;63
	ENTRY <1, AccessResource>			;64
	ENTRY <1, SizeofResource>			;65
	ENTRY <1, AllocResource>			;66
	ENTRY <1, SetResourceHandler>			;67
	ENTRY <1, InitAtomTable>			;68
	ENTRY <1, FindAtom>				;69
	ENTRY <1, AddAtom>				;70
	ENTRY <1, DeleteAtom>				;71
	ENTRY <1, GetAtomName>				;72
	ENTRY <1, GetAtomHandle>			;73
	db 3,0						;74-76
	db 15,1
	ENTRY <1,AnsiNext>			; 77
	ENTRY <1,AnsiPrev>			; 78
	ENTRY <1,AnsiUpper>			; 79
	ENTRY <1,AnsiLower>			; 80
	ENTRY <1,_lclose>			;81
	ENTRY <1,_lread>			;82
	ENTRY <1,_lcreat>			;83
	ENTRY <1,_llseek>			;84
	ENTRY <1,_lopen>			;84
	ENTRY <1,_lwrite>			;86
	ENTRY <1,lstrcmp>			;87
	ENTRY <1,lstrcpy>			;88
	ENTRY <1,lstrcat>			;88
	ENTRY <1,lstrlen>			;90
	ENTRY <1,InitTask>			;91
	db 1,0					;92
	db 1,1
	ENTRY <1,GetCodeHandle>			;93
	db 1,0					;94
	db 2,1
	ENTRY <1,LoadLibrary>			;95
	ENTRY <1,FreeLibrary>			;96
	db 4,0					;97-100
	db 5,1
	ENTRY <1,NoHookDOSCall>			;101
	ENTRY <1,Dos3Call>			;102
	ENTRY <1,NetBiosCall>			;103
	ENTRY <1,GetCodeInfo>			;104
	ENTRY <1,GetExeVersion>			;105
	db 1,0					;106
	db 1,1
	ENTRY <1,SetErrorMode>		;107
	db 3,0						;108-110
	db 2,1
	ENTRY <1,GlobalWire>		;111
	ENTRY <1,GlobalUnWire>		;112
	db 2,-2
eSHIFT	ENTRY <1,3>				;113 _AHSHIFT
eINCR	ENTRY <1,8>				;114 _AHINCR
	db 1,1
	ENTRY <1,OutputDebugString>	;115
	db 1,0						;116
	db 4,1
	ENTRY <1,OldYield>		;117
	ENTRY <1,GetTaskQueueDS>	;118
	ENTRY <1,GetTaskQueueES>	;119
	ENTRY <1,UndefDynlink>		;120
	db 1,0						;121
	db 3,1
	ENTRY <1,IsTaskLocked>		;122
	ENTRY <1,KbdRst>		;123
	ENTRY <1,EnableKernel>		;124
	db 3,0						;124-126
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
	ENTRY <1,GetHeapSpaces>		;138
	db 11,0				;139-149
	db 1,1
	ENTRY <1, DirectedYield>	;150
	db 1,0				;151
	db 1,1
	ENTRY <1, GetNumTasks>		;152
	db 1,0				;153
	db 5,1
	ENTRY <1,GlobalNotify>		;154
	ENTRY <1,GetTaskDS>		;155
	ENTRY <1,LimitEMSPages>		;156
	ENTRY <1,GetCurPID>		;157
	ENTRY <1,IsWinOldApTask>	;158
	db 2,0				;159-160
	db 2,1
	ENTRY <1,LocalCountFree>	;161
	ENTRY <1,LocalHeapSize>		;162
	db 2,0				;163-164
	db 1,1
	ENTRY <1,A20Proc>		;165
	db 1,0				;166
	db 3,1
	ENTRY <1,GetExpWinVer>		;167
	ENTRY <1,DirectResAlloc>	;168
	ENTRY <1,GetFreeSpace>		;169
	db 3,1
	ENTRY <1,AllocCSToDSAlias>	;170
	ENTRY <1,AllocDSToCSAlias>	;171
	ENTRY <1,AllocCSToDSAlias>	;172 AllocAlias
	db 2,-2
eROMBIOS ENTRY <1,0>			;173 _ROMBIOS
eA000 ENTRY <1,00h>			;174 _A000H
	db 3,1
	ENTRY <1,AllocSelector>		;175
	ENTRY <1,FreeSelector>		;176
	ENTRY <1,PrestoChangoSelector>	;177
	db 1,-2
eWinFlags ENTRY <1,0>			;178 __WINFLAGS

	db 1,-2
eD000	ENTRY <1,0>			;179
	db 1,1
	ENTRY <1, LongPtrAdd>		;180
	db 3,-2
eB000	ENTRY <1,0>				;181 _B000H
eB800	ENTRY <1,0>				;182 _B800H
e0000	ENTRY <1,0>				;183 _0000H
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
	db 3,1
	ENTRY <1,SelectorAccessRights>		;196
	ENTRY <1,GlobalFix>			;197
	ENTRY <1,GlobalUnfix>		;198
	db 4,0						;199-202
	db 1,1
	ENTRY <1,DebugBreak>		;203
	db 2,0						;204-205
	db 2,1
	ENTRY <1,AllocSelectorArray>	;206
	ENTRY <1,IsDBCSLeadByte>	;207
	db 102,0					;208-309
	db 2,1
	ENTRY <1,LocalHandleDelta>	;310
	ENTRY <1,GetSetKernelDOSProc>	;311
	db 4,0					;312-315
	db 1,1
	ENTRY <1, GetFreeMemInfo>	;316
	db 4,0					;216-319
	db 1,1
	ENTRY <1, IsTask>		; 320
	db 2,0					;321-322
	db 1,1
	ENTRY <1,IsRomModule>		; 323
	db 2,0						;324-325
	db 1,1
	ENTRY <1,IsRomFile>		; 326
	db 7,0				;327-333
	db 4,1
	ENTRY <1,IsBadReadPtr>		;334
	ENTRY <1,IsBadWritePtr>		;335
	ENTRY <1,IsBadCodePtr>		;336
	ENTRY <1,IsBadStringPtr>	;337
	db 7,0				;338-344
	db 3,1
	ENTRY <1,IsSharedSelector>	;345
	ENTRY <1,IsBadHugeReadPtr>	;346
	ENTRY <1,IsBadHugeWritePtr>	;347
	db 5,0				;348-352
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
	NENAME "LOCALHANDLE" ,11
	NENAME "LOCALFLAGS"  ,12
	NENAME "LOCALCOMPACT",13
	NENAME "LOCALNOTIFY", 14
	NENAME "GLOBALALLOC"  ,15
	NENAME "GLOBALREALLOC",16
	NENAME "GLOBALFREE"   ,17
	NENAME "GLOBALLOCK"   ,18
	NENAME "GLOBALUNLOCK" ,19
	NENAME "GLOBALSIZE"   ,20
	NENAME "GLOBALHANDLE" ,21
	NENAME "GLOBALFLAGS", 22
	NENAME "LOCKSEGMENT"  ,23
	NENAME "UNLOCKSEGMENT",24
	NENAME "GLOBALCOMPACT",25
	NENAME "GLOBALFREEALL",26
	NENAME "GLOBALMASTERHANDLE",28
	NENAME "YIELD",29
	NENAME "WAITEVENT"    ,30
	NENAME "POSTEVENT", 31
	NENAME "SETPRIORITY", 32
	NENAME "LOCKCURRENTTASK",33
	NENAME "SETTASKQUEUE" ,34
	NENAME "GETTASKQUEUE"     ,35
	NENAME "GETCURRENTTASK"   ,36
	NENAME "GETCURRENTPDB"    ,37
	NENAME "SETTASKSIGNALPROC"   ,38
	NENAME "ENABLEDOS", 41
	NENAME "DISABLEDOS", 42
	NENAME "LOADMODULE"       ,45
	NENAME "GETMODULEHANDLE"  ,47
	NENAME "GETMODULEUSAGE"   ,48
	NENAME "GETMODULEFILENAME",49
	NENAME "GETPROCADDRESS"   ,50
	NENAME "MAKEPROCINSTANCE", 51
	NENAME "FREEPROCINSTANCE", 52
	NENAME "GETINSTANCEDATA" ,54
	NENAME "CATCH"            ,55
    	NENAME "THROW"            ,56
	NENAME "GETPROFILEINT", 57
	NENAME "GETPROFILESTRING", 58
	NENAME "WRITEPROFILESTRING", 59
	NENAME "FINDRESOURCE", 60
	NENAME "LOADRESOURCE", 61
	NENAME "LOCKRESOURCE", 62
	NENAME "FREERESOURCE", 63
	NENAME "ACCESSRESOURCE", 64
	NENAME "SIZEOFRESOURCE", 65
	NENAME "ALLOCRESOURCE", 66
	NENAME "SETRESOURCEHANDLER", 67
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
	NENAME "GETCODEHANDLE",93
	NENAME "LOADLIBRARY" ,95
	NENAME "FREELIBRARY" ,96
	NENAME "NOHOOKDOSCALL" ,101
	NENAME "DOS3CALL"    ,102
	NENAME "NETBIOSCALL" ,103
	NENAME "GETCODEINFO" ,104
	NENAME "GETEXEVERSION",105
	NENAME "SETERRORMODE",107
 	NENAME "GLOBALWIRE",111
	NENAME "GLOBALUNWIRE",112
	NENAME "__AHSHIFT"   ,113
	NENAME "__AHINCR"    ,114
	NENAME "OUTPUTDEBUGSTRING", 115
	NENAME "OLDYIELD" ,117
	NENAME "GETTASKQUEUEDS", 118
	NENAME "GETTASKQUEUEES", 119
	NENAME "UNDEFDYNLINK",      120
	NENAME "ISTASKLOCKED",      122
	NENAME "KBDRST",               123
	NENAME "ENABLEKERNEL",124
	NENAME "GETPRIVATEPROFILEINT"  ,127
	NENAME "GETPRIVATEPROFILESTRING"  ,128
	NENAME "WRITEPRIVATEPROFILESTRING",129
	NENAME "GETDOSENVIRONMENT", 131
	NENAME "GETWINFLAGS"      , 132
	NENAME "GETEXEPTR"        , 133
	NENAME "GETWINDOWSDIRECTORY"            ,134
	NENAME "GETSYSTEMDIRECTORY"            ,135
	NENAME "FATALAPPEXIT"     , 137
	NENAME "GETHEAPSPACES",138
	NENAME "DIRECTEDYIELD", 150
	NENAME "GETNUMTASKS", 152
	NENAME "GLOBALNOTIFY", 154
	NENAME "GETTASKDS", 155
	NENAME "LIMITEMSPAGES",156
	NENAME "GETCURPID",157
	NENAME "ISWINOLDAPTASK"           ,158
	NENAME "LOCALCOUNTFREE",161
	NENAME "LOCALHEAPSIZE", 162
	NENAME "A20PROC", 165
	NENAME "GETEXPWINVER",167
	NENAME "DIRECTRESALLOC",168
	NENAME "GETFREESPACE"     , 169
	NENAME "ALLOCCSTODSALIAS" , 170
	NENAME "ALLOCDSTOCSALIAS" , 171
	NENAME "ALLOCALIAS", 172
	NENAME "__ROMBIOS", 173
	NENAME "__A000H", 174
	NENAME "ALLOCSELECTOR"       , 175
	NENAME "FREESELECTOR"        , 176
	NENAME "PRESTOCHANGOSELECTOR", 177
	NENAME "__WINFLAGS"          , 178
	NENAME "__D000H", 179
	NENAME "LONGPTRADD", 180
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
	NENAME "SELECTORACCESSRIGHTS", 196
	NENAME "GLOBALFIX"          ,197
	NENAME "GLOBALUNFIX"        ,198
	NENAME "DEBUGBREAK"         ,203
	NENAME "ALLOCSELECTORARRAY" ,206
	NENAME "ISDBCSLEADBYTE",207
	NENAME "LOCALHANDLEDELTA", 310
	NENAME "GETSETKERNELDOSPROC", 311
	NENAME "GETFREEMEMINFO" ,316
	NENAME "ISTASK" ,320
	NENAME "ISROMMODULE" ,323
	NENAME "ISROMFILE" ,326
	NENAME "ISBADREADPTR",334
	NENAME "ISBADWRITEPTR",335
	NENAME "ISBADCODEPTR",336
	NENAME "ISBADSTRINGPTR",337
	NENAME "ISSHAREDSELECTOR",345
	NENAME "ISBADHUGEREADPTR",346
	NENAME "ISBADHUGEWRITEPTR",347
	NENAME "LSTRCPYN" ,353
;    K403                           @403	+ FarSetOwner
;    K404                           @404	+ FarGetOwner
	db 0

EndKernelNE equ $

_DATA ends

	end
