
DGROUP group _DATA,CONST,DATA,XIB,XI,XIE,YIB,YI,YIE,_BSS


        .dosseg

_BSS    segment word public 'BSS'
_BSS    ends

DATA    segment word public 'DATA'
DATA    ends

CONST   segment word public 'DATA'
CONST   ends

XIB     segment word public 'DATA'
XIB     ends
XI      segment word public 'DATA'
XI      ends
XIE     segment word public 'DATA'
XIE     ends

YIB     segment word public 'DATA'
YIB     ends
YI      segment word public 'DATA'
YI      ends
YIE     segment word public 'DATA'
YIE     ends


_DATA segment word public 'DATA'
;*
;*** externals we need
;*
assume es:nothing
assume ss:nothing
assume ds:dgroup
assume cs:_TEXT

        extrn   __AHSHIFT                   : word
;        public  "C",_HShift
_HShift    db 0                 ; Huge Shift value

_DATA ends

;*
;*** the windows extender code lies here
;*
_TEXT segment word public 'CODE'

public _small_code_
_small_code_  dd 0

        extrn   LIBMAIN     : far       ; startup code


;****************************************************************************
;***                                                                      ***
;*** LibEntry - 16-bit library entry point                                ***
;***                                                                      ***
;****************************************************************************
LibEntry proc far
        public  LibEntry
__DLLstart_:
        public  __DLLstart_

        mov     ax,ds            ; prologue
        nop
        inc     bp
        push    bp
        mov     bp,sp
        push    ds
        mov     ds,ax

        push    di               ; handle of the module instance
        push    ds               ; library data segment
        push    cx               ; heap size
        push    es               ; command line segment
        push    si               ; command line offset

        mov     ax,offset __AHSHIFT ; get huge shift value
        mov     _HShift,al       ; ...
        call    LIBMAIN         ; invoke the 'C' routine (result in AX)

        lea     sp,-2H[bp]
        pop     ds
        pop     bp
        dec     bp
        ret
LibEntry    endp

public __PIA
__PIA   proc    far
        add     ax,bx           ; add offsets
        adc     cx,0            ; calculate overflow
        mov     bx,cx           ; shuffle overflow info bx
        mov     cl,_HShift      ; get huge shift value
        shl     bx,cl           ; adjust the overflow by huge shift value
        add     dx,bx           ; and add into selector value
        ret                     ; ...
__PIA   endp

_TEXT   ends
        end     LibEntry
