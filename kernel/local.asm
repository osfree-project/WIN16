	include kernel.inc

if ?REAL
		.8086
else
		.286
endif

public LocalInit
public LocalAlloc
public LocalReAlloc
public LocalCompact
public LocalSize
public LocalLock
public LocalUnlock
public LocalNotify

externdef pascal GlobalReAlloc: far

_TEXT segment

;*****************************
;*** Local Heap functions  ***
;*****************************


;*** increase local heap segment 
;*** inp: DS=segment, AX=new size (0=64k)
;*** out: C=Fehler, AX=new size
;*** BX,CX,DX,SI,DI not modified

__incseg proc uses bx cx dx

	push ax
	xor cx,cx				  ;0000 -> 10000
	cmp ax,1
	adc cx,cx
	push ds					  ;selector
	push cx
	push ax					  ;CX:AX bytes request  
if ?REAL
	mov ax, 0
	push ax
else
	push 0					  ;flags (means: FIXED)
endif
	call far ptr GlobalReAlloc
	and ax,ax
	jz error
	pop ax
	clc
	jmp exit
error:
	pop dx
	stc
exit:
	ret
__incseg endp

;*** jump to the end of the local heap
;*** the end is marked with a (near) pointer to itself
;*** inp: DS=heap segment, BX=heap pointer ***
;*** out: BX,AX=^end of heap ***

__findlast proc
@@:
	mov AX,[BX]
	cmp AX,BX
	mov BX,AX
	jnz @B
	ret
__findlast endp

;*** grow heap segment
;*** inp: DS=heapsegm,CX=size,BX->heapdesc,DX=size last free segment

__growseg proc uses cx di

	call __findlast
	cmp ax,0FFF0h
	jnb growseg_err
	mov bx,ax
	sub cx,dx
	cmp cx,2000h
	jbe @F
	inc cx
	add ax,cx
	jc growseg_err
	inc ax
	jmp growseg_1
@@:
	add ax,2002h
	jnc growseg_1
	mov ax,0000 		;set AX to 64k (maximum) 
growseg_1:
	push ax
	push bx
	call __incseg
	pop bx
	pop ax
	jnc done	;jump if ok
growseg_err:
	stc 		;error
	jmp exit
done:			;grow has worked
	dec ax
	mov [bx],AX
	push bx
	and al,0FEh
	mov bx,ax
	mov [bx],ax
	pop bx
exit:
	ret
__growseg endp

;--- sets Win16 Local Heap start at DS:[0006]

LocalInit proc far pascal uSegment:word, uStart:word, uEnd:word

	mov ax,uSegment
	mov cx,uStart
	mov dx,uEnd
	and ax,ax
	jnz @F
	mov ax,ds
@@:
	cmp dx,4
	jb LocalInit_err
if ?REAL
	xor bx, bx		; Emulate access rights
else
	lar bx,ax
endif
	jnz LocalInit_err
	jcxz LocalInit_1
	cmp dx,cx
	jb LocalInit_err
	jmp LocalInit_2
LocalInit_1:
if ?REAL
	mov bx, 0FFFFH		; Emulate segment limit
else
	lsl bx,ax
endif
	inc bx
	mov cx,dx
	mov dx,bx
	sub bx,cx
	mov cx,bx
LocalInit_2:
	push ds
	mov ds,ax
	mov ds:[0006],cx
	add word ptr ds:[0006],2
	mov bx,cx
	sub dx,2
	mov [bx],dx
	or byte ptr [bx],1
	mov bx,dx
	mov [bx],dx
	mov ax,1
	pop ds
	jmp LocalInit_ex
LocalInit_err:
	xor ax,ax
LocalInit_ex:
	ret
LocalInit endp

;--- called by LocalAlloc
;*** get a free memory block in local heap
;*** inp: DS=heapsegm, BX= ^heapdesc, CX=size

__searchseg proc
	inc CX
	and CL,0FEh
__searchseg3:
	mov ax,[bx]
	cmp ax,bx
	jbe __searchseg5		;error, end of heap reached
	xor dx,dx
	test al,1				;free?
	jz __searchseg1			;if not -> go on 
	mov dx,ax
	sub dx,bx				;get size
	sub dx,3				;correct it
	cmp cx,dx
	jbe __searchseg2		;block is large enough
	push si 				;if next block is free as well
	and al,0FEh
	mov si,ax
	mov ax,[si]
	test al,1
	jz @F
	mov [bx],ax
	mov si,bx
@@:
	mov ax,si
	pop si
__searchseg1:
	mov bx,ax
	jmp __searchseg3
__searchseg5:
	stc
	ret
__searchseg2:				;found a free item
	mov dx,ax
	lea ax,[bx+2]
	jz __searchseg4			;size matches as well
	add cx,ax
	mov [bx],cx 			;save new pointer
	push bx
	mov bx,cx
	mov [bx],dx 			;save it here as well
	pop bx
__searchseg4:
	and byte ptr [bx],0FEh
	ret
__searchseg endp

LocalAlloc proc far pascal uses si di uFlags:word, uBytes:word
	mov cx,uBytes
	cmp CX,0FFE8h
	ja LocalAlloc1
	mov BX,ds:[0006h]
	and bx,bx
	jz LocalAlloc1
	sub bx,2
	call __searchseg
	jnc LocalAlloc2			;ok, free item found
	call __growseg
	jc LocalAlloc1			;heap cannot grow, error
	mov BX,ds:[0006h]
	sub bx,2
	call __searchseg
	jc LocalAlloc1			;was growing sufficient?
	mov cx,uFlags
	test cl,40h
	jz LocalAlloc2
	push ax
	push di
	mov cx,uBytes
	mov di,ax
	push ds
	pop es
	xor ax,ax
	shr cx,1
	cld
	rep stosw
	adc cl,0
	rep stosb
	pop di
	pop ax
	jmp LocalAlloc2
LocalAlloc1:
	xor AX,AX
LocalAlloc2:
	ret
LocalAlloc endp

LocalFree proc far pascal handle:WORD

	mov ax,handle
	mov bx,ds:[0006]
	and bx,bx
	jz LocalFree_err
	sub bx,2
	sub ax,2
LocalFree_2:
	cmp ax,bx
	jz LocalFree_3
	mov cx,[bx]
	and cl,0FEh
	cmp cx,bx
	mov bx,cx
	jnz LocalFree_2
LocalFree_err:
	xor ax,ax
	jmp LocalFree_ex
LocalFree_3:
	test byte ptr [bx],1
	jnz LocalFree_err
	or byte ptr [bx],1
LocalFree_ex:
	ret
LocalFree endp

LocalReAlloc proc far pascal
	xor ax,ax
	@return 6
LocalReAlloc endp

LocalUnlock proc far pascal
LocalUnlock endp

LocalLock proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	retf
LocalLock endp

LocalSize proc far pascal
	@loadbx
	@loadparm 0,ax
	and ax,ax
	jz localsize_ex
	mov bx,ax
	mov ax,[bx-2]
	sub ax,bx
localsize_ex:
	@return 2
LocalSize endp

LocalCompact proc far pascal
	mov ax,ds:[0006]
	and ax,ax
	jz localcompact_ex
	sub ax,2
	mov bx,ax
	xor cx,cx
localcompact_3:
	mov ax,[bx]
	cmp ax,bx
	jz localcompact_2
	test al,1
	jz localcompact_1
	and al,0FEh
	cmp ax,cx
	jc localcompact_1
	mov cx,ax
localcompact_1:
	mov bx,ax
	jmp localcompact_3
localcompact_2:
	mov ax,cx
localcompact_ex:
	@return 2
LocalCompact endp

LocalNotify proc far pascal
	mov bx, sp
	mov ax, ss:[bx+4]	; lpNotifyFunc
	mov dx, ss:[bx+6]	; lpNotifyFunc+2
	mov bx, ds:[6]		; plocalheap Schulman error here. Was dx.
if ?32BIT
	xchg ax, [bx+1eh]	; 1eh for krnl386
	xchg dx, [bx+1eh+2]	; 1eh for krnl386
else
	xchg ax, [bx+18h]	; 18h for krnl286
	xchg dx, [bx+18h+2]	; 18h for krnl286
endif
	retf 4
LocalNotify endp

_TEXT ends

	end
