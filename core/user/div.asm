
include struct.inc

_TEXT   segment byte public 'CODE'  use16

public __U4M

__U4M:
        assume cs:_TEXT

        xchg    ax,bx           ; swap low(M1) and low(M2)
        push    ax              ; save low(M2)
        xchg    ax,dx           ; exchange low(M2) and high(M1)
        or      ax,ax           ; if high(M1) non-zero
        _if     ne              ; then
          mul   dx              ; - low(M2) * high(M1)
        _endif                  ; endif
        xchg    ax,cx           ; save that in cx, get high(M2)
        or      ax,ax           ; if high(M2) non-zero
        _if     ne              ; then
          mul   bx              ; - high(M2) * low(M1)
          add   cx,ax           ; - add to total
        _endif                  ; endif
        pop     ax              ; restore low(M2)
        mul     bx              ; low(M2) * low(M1)
        add     dx,cx           ; add previously computed high part

        ret                     ; and return!!!

_TEXT   ends

        end
