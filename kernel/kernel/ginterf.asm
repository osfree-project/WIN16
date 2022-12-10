;/*!
;   @file
;
;   @brief osFree Windows Real Mode Kernel global heap management
;
;   (c) osFree Project 2002-2022, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;   Documentation: http://osfree.org/doku/en:docs:win16
;
;*/

	.8086

	; MacroLib
	include dos.inc
	include dpmi.inc

	include ascii.inc


	include kernel.inc
	include debug.inc
	include pusha.inc
	include ascii.inc


	.8086

externdef pascal _hmemset:far
externdef discardmem:near
externdef eWinFlags:near
externdef TH_HGLOBALHEAP:word
externdef TH_PGLOBALHEAP:word

?LARGEALLOC	equ 1	;1=allow more than 1 MB with GlobalAlloc/Realloc/Free

_TEXT segment

GlobalSize proc far pascal
	pop bx
	pop cx
	pop ax
	push cx
	push bx
	xor dx,dx
if ?REAL
	xor ax, ax
	mov ax, 0FFFFH		; Emulate segment limit
else
	lsl ax,ax
endif
	jnz @F
	add ax,1
	adc dx,0
	jmp exit
@@:
	xor ax,ax
	cwd
exit:
	@return
GlobalSize endp

;--- DWORD GlobalDOSAlloc(DWORD size)
;--- returns selector in ax, segment in dx

GlobalDOSAlloc proc far pascal
	pop bx			; Get return address
	pop cx
	pop ax			; Get size into DX:AX
	pop dx
	push cx			; Restore return address
	push bx
	mov cl,al
	push cx
        mov cl, 4
	shr ax, cl
        mov cl, 12
	shl dx, cl
	pop cx
	or ax,dx
	test cl,0Fh
	jz @F
	inc ax
@@:
;	@DPMI_DOSALLOC ax	; Alloc dos memory
	xchg ax,dx
	jnc @F
	xor ax,ax
@@:
	@return
GlobalDOSAlloc endp

GlobalDOSFree proc far pascal
	pop bx
	pop cx
	pop dx
	push cx
	push bx
	@DPMI_DOSFREE dx		;free dos memory
	mov ax,dx
	jc @F
	xor ax,ax			;return 0 on success
@@:
	@return
GlobalDOSFree endp

GlobalLock proc far pascal
	pop cx
	pop bx
	pop dx
	push bx
	push cx
	xor ax,ax
if ?REAL
				; Emulate readable (ZF=1)
else
	verr dx
endif
	jnz @F
	retf
@@:
	xor dx,dx
	retf
GlobalLock endp

GlobalUnlock proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	@return
GlobalUnlock endp

;--- GlobalAlloc(WORD flags, DWORD dwSize);
;--- according to win31 docs max size is 16 MB - 64 kB on a 80386
;--- and 1 MB - 80 bytes on a 80286

parm1	equ <bp+6>
parm2	equ <bp+6+4>

GlobalAlloc proc far pascal

	push bp
	mov bp,sp
	mov ax,[parm1+0]
	mov dx,[parm1+2]
	mov bx,dx
	mov cx,ax
	test dx,0FFF0h
  if ?LARGEALLOC
	jnz largealloc	 ;maximum is 1 MB - 16 in 16 bit version
  else
	jnz error
  endif
	push cx
	mov cl, 4
	shr ax, cl
	mov cl, 12
	shl dx, cl
	pop cx
	or ax,dx
	test cl,0Fh
	jz @F
	inc ax
  if ?LARGEALLOC
	jz largealloc
  else
	jz error
  endif
@@:
	cmp ax,-1				;bx=FFFF might not work
  if ?LARGEALLOC
	jz largealloc
  else
	jz error
  endif
	xchg bx,ax
	
	@GetBlok				;alloc with DOS call, so we need no
						;handle management (ebx paras)
	jc error
allocok:
	mov cx,[parm2]
	test cl,40h				;GMEM_ZEROINIT?
	jz exit
	push ax

;--- _hmemset(FAR16 dst, WORD value, DWORD cnt), requires __AHINCR
public __AHINCR
__AHINCR equ 8

	push ax		;selector
	xor ax,ax
	push ax		;offset
	push ax		;value
	mov ax,[parm1+0]
	mov dx,[parm1+2]
	push dx		;HiWord(cnt)
	push ax		;LoWord(cnt)
	push cs
	call near ptr _hmemset
	pop ax
exit:
	pop bp
	@return 6
error:
	@trace_s <"GlobalAlloc failed",lf>
	xor ax,ax
	jmp exit
if ?LARGEALLOC

;--- size in BX:CX

largealloc:
	test byte ptr cs:[eWinFlags.ENTRY.wOfs], WF_CPU286
	jnz error
	@push_a
	mov bp,sp
	xor ax,ax
	push ax
	push bx
	push cx
	add cx,8h		;add 8 bytes as prefix
	adc bx,0
	@DPMI_ALLOCMEM
	jc failed
	push bx			;save linear address
	push cx
	mov cx,[bp-4]	;get no of 64 k blocks
	cmp word ptr [bp-6],0
	jz @F
	inc cx
@@:
	mov [bp-2],cx
	@DPMI_AllocDesc
	jc failed2
	mov bx,ax
	pop dx
	pop cx
	@DPMI_SetBase		;set base
	pop dx
	pop cx
	push cx
	sub dx,1		;calc limit
	sbb cx,0
	@DPMI_SetLimit		;set limit
	mov es,bx
	mov es:[0],di	;save DPMI handle
	mov es:[2],si
	mov es:[4],bx	;save selector
	mov si,[bp-2]
	mov es:[6],si	;save no of selectors
	@DPMI_GetBase
	add dx,8
	adc cx,0
	@DPMI_SetBase
	push cx
	mov di,dx
nextdesc:
	dec si
	jz done
	add bx,8
	pop cx
	inc cx
	push cx
	mov dx,di
	@DPMI_SetBase
	mov dx,-1
	cmp si,1
	jnz @F
	mov cx,es
if ?REAL
	mov dx, 0FFFFH		; Emulate segment limit
else
	lsl dx,cx
endif
@@:
	mov cx,0
	@DPMI_SetLimit
	jmp nextdesc
done:
	mov sp,bp
	mov [bp+0Eh],es
	@pop_a
	jmp allocok
failed2:
	@DPMI_FREEMEM
failed:
	mov sp,bp
	@pop_a
	jmp error
endif
GlobalAlloc endp

;--- GlobalFree(WORD handle);
;--- rc: ax=0 if successful, else ax=handle

GlobalFree proc far pascal
	pop cx
	pop dx
	pop bx		;get handle
	push dx
	push cx
if ?LARGEALLOC
	test byte ptr cs:[eWinFlags.ENTRY.wOfs], WF_CPU286
	jnz @F
	.386
	lsl eax,ebx
	jnz error
;	test eax,0FFF00000h	;limit >= 100000h
;	jnz largefree
	cmp eax,0FFFEFh			;largest block for DOS int
	jnc largefree
	.8086
@@:
endif
	push es
	@FreeBlok bx
	pop ax
if ?REAL
			; no access check
else
	verr ax
	jnz done
endif
	mov es,ax
done:
	xor ax,ax
exit:
	ret
error:
	mov ax,bx
	jmp exit
if ?LARGEALLOC
failed: 
	add dx,8
	adc cx,0
	@DPMI_SetBase
	jmp error
largefree:
	@DPMI_GetBase		;get base
	jc error
	sub dx,8
	sbb cx,0
	@DPMI_SetBase		;set base
	mov es,bx
	cmp bx,es:[4]
	jnz failed
	mov ax,es:[6]
	and ax,ax
	jz failed
	@push_a
	mov cx,ax
	mov di,es:[0]
	mov si,es:[2]
	@DPMI_FREEMEM
@@:
	@DPMI_FreeDesc
	add bx,8
	loop @B
	@pop_a
	jmp done
endif
GlobalFree endp

;--- resize a module segment in DOS memory
;--- DX:AX = new size
;--- ES:BX-> segment descriptor

resizedosblock proc
	test dx,0FFF0h					;size > 1 MB is impossible
	jnz error
	mov cl,al
if ?REAL
	shr ax,1
	shr ax,1
	shr ax,1
	shr ax,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
	shl dx,1
else
	shr ax,4
	shl dx,12
endif
	or ax,dx
	test cl,0Fh
	jz @F
	inc ax
	jz error
@@:
;	mov dx,
	xchg ax,bx
	@DPMI_DOSSIZE es:[bx].SEGITEM.wDosSel	; @todo ???
	jc error
	mov ax,dx
	ret
error:
	xor ax,ax
	stc
	ret
resizedosblock endp

;--- resize a module segment in extended memory
;--- DX:AX = new size
;--- ES:BX-> segment descriptor

resizeextmemblock proc
	push dx
	push ax
	push cx				;selector 1
	xor cx,cx
	test word ptr es:[bx.SEGITEM.flags],SF_ALIAS
	jz @F
	mov bx,word ptr es:[bx].SEGITEM.dwHdl+0
	mov cx,es:[bx].SEGITEM.wSel
@@:
	push cx				;selector 2
	push bx
	mov si,word ptr es:[bx].SEGITEM.dwHdl+2
	mov di,word ptr es:[bx].SEGITEM.dwHdl+0
	mov cx,ax
	mov bx,dx
	@DPMI_SIZE			;resize dpmi memory block
	mov dx,cx			;base address -> cx:dx
	mov cx,bx
	pop bx
	pop ax				;selector 2
	jc error0
	mov word ptr es:[bx].SEGITEM.dwHdl+2,si
	mov word ptr es:[bx].SEGITEM.dwHdl+0,di
	and ax,ax
	jz @F
	mov bx,ax
	@DPMI_SetBase		;set segment base address
@@:
	pop bx				;selector 1
	@DPMI_SetBase		;set segment base address
	jc error1
	pop dx
	pop cx
	sub dx,1
	sbb cx,0
	@DPMI_SetLimit		;set limit
	jc error2
	mov ax,bx
exit:
	clc
	ret
error0:
	pop ax
error1:
	pop dx
	pop cx
error2:
	xor ax,ax
	stc
	ret
resizeextmemblock endp

;*** called by GlobalRealloc(): the block to be resized is a module segment
;*** (E)SI is saved already
;--- ES:BX-> segment descriptor
;--- DX:AX=new size
;--- CX=Selector

;*** segment to resize might be DGROUP (SS == DGROUP)!
;*** there is a problem: the segment's linear base address
;*** may change. If SS (or CS?) are using this block
;*** the segment descriptor cache has to be reloaded.
;*** fix: the loader's stack is used during resizing.

resizemodulesegm proc uses di
	mov di, ax
	@entercriticalsection	;this routine is not reentrant
	mov ax, di
	mov di, ss			;switch to loader stack
	mov si,sp
	@SetKernelDS ss
	mov sp,cs:[wStktop]
	push di
	push si
	cmp es:[bx].SEGITEM.wDosSel,0	;conventional memory?
	jz @F
	call resizedosblock
	jmp resizemodseg_1
@@:
	call resizeextmemblock
resizemodseg_1:
	pop si
	pop ss
	mov sp,si
	@exitcriticalsection
	ret

resizemodulesegm endp

;--- GlobalReAlloc(WORD hMem, DWORD dwSize, WORD flags);
;--- todo: if block increases and GMEM_ZEROINIT is set, additional memory
;--- should be zeroed.

GlobalReAlloc proc far pascal uses si hMem:WORD, dwNewsize:DWORD, uiMode:WORD 

	mov si, hMem
	mov dx, word ptr dwNewsize+2
	mov ax, word ptr dwNewsize+0

	push si
	push ax
	push dx
	call Segment2ModuleFirst		;is it a module segment?
	and ax,ax
	pop dx
	pop ax
	pop cx
	jz @F
	call resizemodulesegm
	jmp exit
@@:
	mov es,cx
if ?32BIT
	mov cl,al
	push dx
	push ax
	pop eax
	shr eax,4
	test cl,0Fh
	jz @F
	inc eax
@@:
	mov ebx,eax
else
	test dx,0FFF0h
	jnz globalreallocerr		;for 16-bit 1 MB - 10h is maximum
	mov cl,al
	push cx
	mov cl, 4
	shr ax,cl
	mov cl, 12
	shl dx, cl
	pop cx
	or ax,dx
	test cl,0Fh					;since no D bit exists
	jz @F
	inc ax
	jz globalreallocerr
@@:
	xchg bx,ax
endif
	push es
	@ModBlok
	pop ax
	jnc exit

globalreallocerr:
	xor ax,ax
exit:
	ret

GlobalReAlloc endp

GlobalUnfix proc far pascal
GlobalUnfix endp
GlobalFix proc far pascal
	@return 2
GlobalFix endp

GlobalHandle proc far pascal
	pop cx
	pop dx
	pop ax
	push dx
	push cx
	mov dx,ax
	@return
GlobalHandle endp

;--- DWORD GlobalCompact(DWORD);
;--- returns the largest free memory object if dwMinFree != 0

GlobalCompact proc far pascal dwMinFree:DWORD

	mov ax,word ptr dwMinFree+0
	mov dx,word ptr dwMinFree+2
	mov cx,ax
	and cx,dx
	inc cx
	jnz @F
	push ds
	@SetKernelDS
	call discardmem
	pop ds
@@:
	xor ax,ax
	push ax
	call GetFreeSpace
	ret
GlobalCompact endp

GetFreeSpace proc far pascal
	push es
	push di
	sub sp,48
	mov di,sp
	push ss
	pop es
	@DPMI_GETFREEMEMINFO
	pop ax		;get the first dword in DX:AX
	pop dx
	add sp,48-4
	pop di
	pop es
	retf 2
GetFreeSpace endp

GetFreeMemInfo proc far pascal uses ds
local buf[30h]: BYTE
	@SetKernelDS
	mov ax, -1
	mov dx, -1
	test word ptr eWinFlags, WF_PAGING
	jz exit
	lea di, buf
	push ss
	pop es
	@DPMI_GETFREEMEMINFO
	jc exit
	mov ax, es:[di][10h]
	mov bx, es:[di][14h]
exit:
	ret
GetFreeMemInfo endp

GlobalMasterHandle proc far pascal uses ds
	@SetKernelDS
	mov	ax,TH_HGLOBALHEAP
	mov	dx,TH_PGLOBALHEAP
	ret
GlobalMasterHandle endp

_TEXT ends

	end
