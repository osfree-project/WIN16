
DGROUP group _NULL,_DATA,CONST,DATA,_BSS

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
        public  "C",_cbyte
        public  __no87

__aaltstkovr dw -1              ; alternate stack overflow routine address
_curbrk    dw 0                 ; top of usable memory
_cbyte     dw 0                 ; used by getch, getche
__no87     dw 0                 ; always try to use the 8087
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

_TEXT   ends
        end     LibEntry
