
DGROUP group _NULL,_DATA,CONST,_BSS

        .dosseg

_BSS    segment word public 'BSS'
_BSS    ends

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
assume es:nothing
assume ss:nothing
assume ds:dgroup
assume cs:_TEXT

_DATA ends

FIXED_TEXT segment word public 'CODE'
FIXED_TEXT ends

; Global Atoms implementation
ATOM_GROUP group ATOM_TEXT
ATOM_TEXT segment word public 'CODE'
ATOM_TEXT ends

; DCE implementation
DCE_TEXT segment word public 'CODE'
DCE_TEXT ends

; Network implementation
NETWORK_GROUP group NETWORK_TEXT
NETWORK_TEXT segment word public 'CODE'
NETWORK_TEXT ends

; Dialog implementation
DLG_GROUP group DLG_TEXT
DLG_TEXT segment word public 'CODE'
DLG_TEXT ends

DEBUG_TEXT segment word public 'CODE'
DEBUG_TEXT ends

; Initialization functions
INIT_TEXT segment word public 'CODE'
INIT_TEXT ends

INIT_TEXT segment word public 'CODE'

        extrn   LIBMAIN     : near       ; startup code


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
;       ds               ; segment/selector of the module instance
;       cx               ; heap size
;       es               ; command line segment
;       si               ; command line offset

	push	ds
	push	di
	push	cx
        call	LIBMAIN         ; invoke the 'C' routine (result in AX)

        ret
LibEntry    endp

INIT_TEXT   ends
        end     LibEntry
