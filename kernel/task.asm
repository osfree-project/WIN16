;
; osFree Windows Kernel - Task related functions
;
; ?REAL	- produce real mode kernel for real CPU mode, else standard/enchanced kernel for protected CPU mode
; ?32BIT - use 80386-specific code (in original loader this mean create 32-bit client. We always 16-bit client)
;
; This code heavely uses Matt Pietrek books
;

	; MacroLib
	include dos.inc
	; Kernel
	include kernel.inc

	include ascii.inc
	include debug.inc

	include tdb.inc
	include pdb.inc

externdef pascal wKernelDS:word
externdef pascal LocalInit: far
GetCurrentPDBReal proto far pascal 

_TEXT	segment

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
	cmp word ptr ds:[0004],5 ; What is this? Why skip if SS equal to 5? Flag it is kernel task??
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
	call LocalInit	;preserves ES
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

;
; GetCurrentTaks
;
; @todo Returns PDB/PSP instead of TDB. Need to fix it
; @todo Also must return dx (undocumented)
;

GetCurrentTask proc far pascal
	GET_PSP			; @todo this is incorrect
	mov ax,bx		; @todo this is incorrect
;	mov ax,cs:[TH_CURTDB]	; @todo Need to use this instead of PDB
	mov dx,cs:[TH_HEADTDB]	; @todo this also not initialized yet
	ret
GetCurrentTask endp

;
; GetDOSEnvironment
;

GetDOSEnvironment proc far pascal uses es
	invoke GetCurrentPDBReal
	mov es,ax
	mov dx,es:[ENVIRON]
	xor ax,ax
	ret
GetDOSEnvironment endp

GetTaskIntoES proc near pascal uses ax bx
	mov bx, sp
	mov ax, ss:[bx+6]
	or ax, ax
	jnz not_zero
	mov es, word ptr cs:[wKernelDS]
	mov ax, es:[TH_CURTDB]
not_zero:
	mov es, ax
	ret
GetTaskIntoES endp

GetTaskIntoES2 proc near pascal uses ax
        mov bx,sp
        mov ax,ss:[bx+8]
        mov bx,ss:[bx+6]
	or ax, ax
	jnz not_zero
	mov es, word ptr cs:[wKernelDS]
	mov ax, es:[TH_CURTDB]
not_zero:
	mov es, ax
	ret
GetTaskIntoES2 endp

;
; SetTaskQueue
;

SetTaskQueue proc far pascal
        call GetTaskIntoES2
        mov ax,bx
        xchg ax,es:[TDB.TDB_HQUEUE]
        ret 4
SetTaskQueue endp

;
; GetTaskQueue
;

GetTaskQueue proc far pascal
	call GetTaskIntoES
	mov ax, es:[TDB.TDB_HQUEUE]
	ret 2
GetTaskQueue endp

;
; IsWinOldApTask
;

IsWinOldApTask proc far pascal uses es
	call GetTaskIntoES
	mov es, es:[TDB.TDB_HPDB]
	mov ax, es:[48H] ;PDB.PDB_UNDOK
	and ax, 1
	ret 2
IsWinOldApTask endp

; @todo For now no task switching...
externdef MTYeld: far
OldYield proc far pascal

	@SetKernelDS
;        cmp     InScheduler, 0
;        jnz     exit
        cmp     TH_CURTDB,0
        jz      exit
        mov     ds,TH_CURTDB
        inc     ds:[TDB.TDB_nEvents]
;        call    MTYield			; call MThread lib functions
        dec     ds:[TDB.TDB_nEvents]
exit:
	ret
OldYield endp

DirectedYield proc far pascal hTask: word
	@SetKernelDS
        mov	ds, TH_CURTDB
	mov	ax, hTask
        mov	ds:[TDB.TDB_hYieldTo], ax
	invoke	OldYield
	ret
DirectedYield endp


_TEXT	ends
	end
