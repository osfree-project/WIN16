	; MacroLib
	include dos.inc
	; Kernel
	include kernel.inc

EW_REBOOTSYSTEM equ 43h


_TEXT segment

DisableKernel proc far pascal
	@SetKernelDS
	or	KernelFlags[2], 02h		; Windows Exit Flag
	cmp	WORD PTR [PrevInt21Proc+2], 0
	je	nodos
	call	InternalDisableDOS
nodos:
	mov	cx,word ptr ds:[PrevInt3FProc+2]
	mov	dx,word ptr ds:[PrevInt3FProc+0]
	@DPMI_SetExcVec 0bh	

	mov	cx,word ptr ds:[PrevInt0CProc+2]
	mov	dx,word ptr ds:[PrevInt0CProc+0]
	@DPMI_SetExcVec 0ch	

	mov	cx,word ptr ds:[PrevInt0DProc+2]
	mov	dx,word ptr ds:[PrevInt0DProc+0]
	@DPMI_SetExcVec 0dh	

	mov	cx,word ptr ds:[PrevInt06Proc+2]
	mov	dx,word ptr ds:[PrevInt06Proc+0]
	@DPMI_SetExcVec 06h	

	mov	cx,word ptr ds:[PrevInt0EProc+2]
	mov	dx,word ptr ds:[PrevInt0EProc+0]
	@DPMI_SetExcVec 0Eh	

	mov	ax, ds:[TH_HEADPDB]
term_pdb:
	mov	ds,ax
	cmp	ax, ds:[TH_TOPPDB]
	je	skip_toppdb

	call	TerminatePDB			; Need more info

skip_toppdb:
	mov	ax,ds:[PDB_NEXTPDBSEL]
	or	ax,ax
	jnz	term_pdb

	mov	bx,[TH_TOPPDB]
	mov	ah,50h
	int	21h
	and	KernelFlags[2],NOT 02h	; Disable kernel exit flag

	mov	ds,[TH_TOPPDB]
	mov	cx,ds:[PDB_NBFILES]
nextfile:
	mov	bx,cx
	dec	bx
	cmp	bx,5
	jb	skipstd
	@CloseFil
skipistd:
	loop	nextfile

; restore SFT here
;@todo not finished yet
exit:
	ret
DisableKernel endp

ExitKernel proc far pascal rc: word
	@SetKernelDS
	or	ds:KernelFlags[2], 02h		; Windows Exit Flag
	
	call KillLibraries
	
	mov     ax,word ptr [PMouseTermProc]
	or      ax,word ptr [PMouseTermProc+2]
	jz      exit1
	call	[PMouseTermProc]
	@SetKernelDS

exit1:
	mov     ax,word ptr [PKeyboardTermProc]
	or      ax,word ptr [PKeyboardTermProc+2]
	jz      exit2
	call	[PKeyboardTermProc]
	@SetKernelDS

exit2:
	mov     ax,word ptr [PSystemTermProc]
	or      ax,word ptr [PSystemTermProc+2]
	jz      exit3
	call	[PSystemTermProc]
	@SetKernelDS

exit3:
	call WriteOutProfiles

	mov fProfileMayBeStale, 1

	call Enter_Gmove_Stack

	invoke DisableKernel

	cmp rc, EW_REBOOTSYSTEM
	jne exit_via_DOS

ife ?REAL	
	mov	ax, 1600h
	int	2fh

	test	al, 7fh
	jz	exit_via_INT_19

	cmp	al, 1
	je	exit_via_DOS

	cmp al, 0ffh
	je	exit_via_DOS

	mov	ax, 1684h
	mov	bx, 9
	mov	di, 0
	mov	es, di
	int	2fh

	mov	ax, es
	or	ax,di
	jz	exit_via_DOS

	mov	ax, 100h
	call	[es:di]

	jmp exit_via_DOS

exit_via_INT_19:
endif
	RESET_DISK

;INT 2F - NORTON UTILITIES 5.0+ TSRs - FLUSH BUFFERS
;
;	AX = FE03h
;	DI = 4E55h ("NU")
;	SI = TSR identifier (see #03140)
;Return: SI = TSR reply (lowercase version of entry SI, i.e. SI OR 2020h)
;	AX = status
;	    0006h successful???
;Notes:	only supported by DISKMON, FILESAVE, and NCACHE-x
;	useful for flushing NCACHE before rebooting
;SeeAlso: AX=FE00h,AX=FE10h

	mov	ax, 0fe03h
	mov	di, 4e55h		; "NU"
	mov	si, 4346h		; "CF"
	stc
	int	2fh

; reboot

	int	19h

exit_via_DOS:
	@Exit rc

ExitKernel endp
	
_TEXT ends
	end
