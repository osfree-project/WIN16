GetTaskIntoES proc near
	mov bx, sp
	mov ax, ss:[bx+6]
	or ax, ax
	jnz not_zero
	mov es, word ptr cs:MyCSDS
	mov ax, es:CurrTask
not_zero:
	mov es, ax
	ret
GetTaskIntoES endp

GetTaskQueue proc far pascal
	call GetTaskIntoES
	mov ax, es:TDB_HQUEUE
	ret 2
GetTaskQueue endp

GetTaskQueueDS proc far pascal
	push 0
	call GetTaskIntoES
	mov ds, es:TDB_HQUEUE
	ret 2
GetTaskQueueDS endp

GetTaskQueueES proc far pascal
	push 0
	call GetTaskIntoES
	mov es, es:TDB_HQUEUE
	ret 2
GetTaskQueueES endp

IsWinOldApTask proc far pascal
	call GetTaskIntoES
	mov es, es:[TDB_PDB]
	mov ax, es:[PDB_UNDOK] ;48h
	and ax, 1
	ret 2
IsWinOldApTask endp
