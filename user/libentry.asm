
DGROUP group _NULL,_DATA,CONST,DATA,_BSS

;public  pLocalHeap
;public  pAtomTable
;public  pStackTop
;public  pStackMin
;public  pStackBot

;pLocalHeap      equ     0006H
;pAtomTable      equ     0008H
;pStackTop       equ     000AH
;pStackMin       equ     000CH
;pStackBot       equ     000EH

; cloned from windows.inc
GMEM_SHARE      equ     2000h

        .dosseg

FAR_DATA segment byte public 'FAR_DATA'
FAR_DATA ends

_BSS    segment word public 'BSS'
_BSS    ends

DATA    segment word public 'DATA'
DATA    ends

CONST   segment word public 'DATA'
CONST   ends

_NULL   segment para public 'BEGDATA'
__nullarea label word
           dw   0,0
           dw   5
           dw   0               ; pLocalHeap
           dw   0               ; pAtomTable
_STACKLOW  dw   0               ; pStackTop: lowest address in stack
_STACKTOP  dw   0               ; pStackMin:
           dw   0               ; pStackBot: highest address in stack
        public  __nullarea
_NULL   ends

_DATA segment word public 'DATA'
;*
;*** externals we need
;*
assume es:nothing
assume ss:nothing
assume ds:dgroup
assume cs:_TEXT

;        extrn   __AHSHIFT                   : word

        public  "C",_curbrk
;        public  "C",_psp
;        public  "C",_osmajor
;        public  "C",_osminor
;        public  "C",_osmode
;        public  "C",_STACKLOW
;        public  "C",_STACKTOP
        public  "C",_cbyte
;        public  "C",_child
        public  __no87
;        public  "C",_HShift
;        public  __get_ovl_stack
;        public  __restore_ovl_stack
;        public  "C",__FPE_handler
;        public  "C",_LpCmdLine

__aaltstkovr dw -1              ; alternate stack overflow routine address
_curbrk    dw 0                 ; top of usable memory
;_psp       dw 0                 ; segment addr of program segment prefix
;_osmajor   db 0                 ; major DOS version number
;_osminor   db 0                 ; minor DOS version number
;_osmode    db 0                 ; 0 => DOS real mode
;_HShift    db 0                 ; Huge Shift value
_cbyte     dw 0                 ; used by getch, getche
;_child     dw 0                 ; non-zero => a spawned process is running
__no87     dw 0                 ; always try to use the 8087
;__get_ovl_stack dw 0,0          ; get overlay stack pointer
;__restore_ovl_stack dw 0,0      ; restore overlay stack pointer
;__FPE_handler dd 0              ; FPE handler
;_LpCmdLine dw 0,0               ; lpCmdLine (for _argc, _argv processing)
           db 0                 ; slack byte

_DATA ends

;*
;*** the windows extender code lies here
;*
_TEXT segment word public 'CODE'

        extrn   LIBMAIN     : near       ; startup code
        extrn   LOCALINIT   : far       ; Windows heap init routine
        extrn   GLOBALFIX   : far       ; Fix segment
        extrn   GLOBALWIRE  : far       ; Move segment segment
        extrn   GLOBALPAGELOCK  : far       ; Move segment segment

public          _large_code_
_large_code_    equ 0

;****************************************************************************
;***                                                                      ***
;*** LibEntry - 16-bit library entry point                                ***
;***                                                                      ***
;****************************************************************************
LibEntry proc far
        public  LibEntry
__DLLstart_:
        public  __DLLstart_

;       di               ; handle of the module instance
;       ds               ; library data segment
;       cx               ; heap size
;       es               ; command line segment
;       si               ; command line offset
        xor     ax,ax
        jcxz    exit		; If no heap then error exit (USER.EXE needs heap)

        push    ds		; DS (can be 0 also which is same as DS)
        push    ax		; 0
        push    cx		; Heap size
        call    LOCALINIT	; Initialize LocalHeap
        jcxz    exit		; CX=AX for LocalInit. Quit if it failed

	mov	ax, DGROUP:FAR_DATA
	push	ax
	push	ax
	call	GlobalWire
	call	GlobalFix

	; @todo: only for enhanced mode
.386
	push	DGROUP:FAR_DATA
	call	GlobalPageLock
.8086

	push	di
        call	LIBMAIN         ; invoke the 'C' routine (result in AX)

exit:
        ret
LibEntry    endp

;__null_FPE_rtn proc far
;        ret                             ; return
;__null_FPE_rtn endp

;public  __GETDS
;__GETDS proc    near
;        ret                             ; return
;__GETDS endp

_TEXT   ends
        end     LibEntry
