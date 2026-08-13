;--------------------------------------------------------------------
;
;       INFOPLUS.ASM  ->  LOWLEVEL.ASM
;
;       Version 1.57
;
;       Eleven subprograms used by INFOPLUS.PAS:
;
;               CPUID           - identifies host CPU and NDP (if
;                                       any)
;               DISKREAD        - reads absolute sectors from disk
;               LONGCALL        - calls a routine using a CALL FAR
;               ATIINFO         - for accessing ATI VGAWonder cards
;               ALTINTR         - calls interrupts with a true INT call
;               ALTMSDOS        - calls DOS with a true INT call
;               CIRRUSCK        - Cirrus VGA check
;               CTICK           - Chips & Technologies VGA check
;               TSENGCK         - Tseng VGA check
;               ZYMOSCK         - ZyMOS VGA check
;               BUGTST          - Test for 386 POPAD bug
;
;       Originally by:
;       Steve Grant
;       Long Beach, CA
;       January 13, 1989
;
;       mods by Andrew Rossmann (6/26/93)
;
;       Adapted for C calling convention (Open Watcom 1.9, large model)
;--------------------------------------------------------------------

        .model large, C
        .286P
        .8087

;--------------------------------------------------------------------
;                            DATA
;--------------------------------------------------------------------
        .data

; storage for CPUID

; redirected INT 01H vector

old_int01       label   dword
old_int01_ofs   dw      ?
old_int01_seg   dw      ?

; storage for NDPID

; 80x87 control word after initialization, status word after divide by zero

ndp_cw          dw      ?
ndp_sw          dw      ?

; storage for DISKREAD

; DOS 4.0 extended read parameter block
dos4_block                      label   byte
extd_starting_sector_lo         dw      ?
extd_starting_sector_hi         dw      ?
extd_number_of_sectors          dw      ?
extd_bufofs                     dw      ?
extd_bufseg                     dw      ?

; storage for LONGCALL
address         dd      ?
ds_save         dw      ?
si_save         dw      ?

; storage for ALTINTR
save_ds         dw      ?
save_si         dw      ?

;--------------------------------------------------------------------
;                            CODE
;--------------------------------------------------------------------
        .code

;--------------------------------------------------------------------

_CPUID   proc    far

cpu_info        equ     [bx]    ; BX указывает на структуру (первый параметр)

mCPU    equ     byte ptr [bx]
mMSW    equ     word ptr [bx + 1]
mGDT    equ     [bx + 3]
mIDT    equ     [bx + 9]
mchkint equ     byte ptr [bx + 15]
mNDP    equ     byte ptr [bx + 16]
mNDPCW  equ     word ptr [bx + 17]
mWeitek equ     byte ptr [bx + 19]
mtest   equ     byte ptr [bx + 20]

f8088   equ     0
f8086   equ     1
fV20    equ     2
fV30    equ     3
f80188  equ     4
f80186  equ     5
f80286  equ     6
f80386  equ     7
f80486  equ     8
funk    =       0FFH

false   equ     0
true    equ     1

        push    bp
        mov     bp,sp
        push    ds
        push    si
        push    di
        les     bx,dword ptr [bp+6]     ; адрес структуры в ES:BX
        cmp     mtest, 'C'
        jnz     skipcpu
        call    cpu
        call    chkint
skipcpu:
        cmp     mtest, 'N'
        jnz     skipndp
        call    ndp
skipndp:
        cmp     mtest, 'W'
        jnz     skipweitek
        call    weitek
skipweitek:
        pop     di
        pop     si
        pop     ds
        pop     bp
        ret
_CPUID   endp

;--------------------------------------------------------------------

cpu     proc    near

; interrupt of multi-prefix string instruction

        mov     mCPU,funk               ;set CPU type to unknown
        sti
        mov     cx,0FFFFH
rep     lods    byte ptr es:[si]
        jcxz    cpu_02
        call    piq
        cmp     dx,4
        jg      cpu_01
        mov     mCPU,f8088
        jmp     cpu_done
cpu_01:
        cmp     dx,6
        jne     cpu_01a
        mov     mCPU,f8086
cpu_01a:
        jmp     cpu_done
cpu_02:

; number of bits in displacement register used by shift

        mov     al,0FFH
        mov     cl,20H
        shl     al,cl
        or      al,al
        jnz     cpu_04
        call    piq
        cmp     dx,4
        jg      cpu_03
        mov     mCPU,fV20
        jmp     cpu_done
cpu_03:
        cmp     dx,6
        je      cpu_03a
        jmp     cpu_done
cpu_03a:
        mov     mCPU,fV30
        jmp     cpu_done
cpu_04:

; order of write/decrement by PUSH SP

        push    sp
        pop     ax
        cmp     ax,sp
        je      cpu_06
        call    piq
        cmp     dx,4
        jg      cpu_05
        mov     mCPU,f80188
        jmp     cpu_done
cpu_05:
        cmp     dx,6
        jne     cpu_done
        mov     mCPU,f80186
        jmp     cpu_done

; We most likely have a 286, 386 or 486 CPU by now
;First, grab some tables

cpu_06:
        smsw    mMSW
        sgdt    mGDT
        sidt    mIDT

;!!!!!!!
;!!! Original 286/386 detection code (modified 8/10/90)
;!!! Modified by code supplied by John Levine, apparantly from an Intel
;!!! '486 manual.
;!!!!!!!

        pushf                           ;put flags into CX
        pop     cx
        and     cx,0fffh                ;mask off upper 4 bits
        push    cx
        popf
        pushf
        pop     ax
        and     ax,0f000h               ;look only at upper 4 bits
        cmp     ax,0f000h               ;88/86 etc.. turn them on
        jz      badcpu                  ;not 286/386/486!!!
        or      cx,0f000h               ;force upper 4 bits on
        push    cx
        popf
        pushf
        pop     ax
        and     ax,0f000h
        jz      found286                ;bits are zeroed in real mode 286
;
;since we probably have have a 386 or 486 by now, we need to do some 32-bit
;work. Detect the 486 by seeing if the Alignment Check flag is settable. This
;flag only exists on the '486.
;
.386
        and     esp,0FFFFh              ;use only 64K stack
        mov     edx,esp                 ;save current stack position
        and     esp,0FFFCh              ;dword align to avoid traps
        pushfd                          ;push 32 bit flag
        pop     eax
        mov     ecx,eax                 ;save current flags
        xor     eax,40000h              ;flip AC (alignment check) flag
        push    eax
        popfd
        pushfd
        pop     eax
        xor     eax,ecx                 ;eliminate all but AC bit
        push    ecx                     ;restore flags
        popfd
        mov     esp,edx                 ;restore stack position
        test    eax,40000h              ;is bit set?
.286
        jz      found386                ;if not, is a 386
        mov     mCPU,f80486             ;must be a 486!!
        jmp     cpu_done
found286:
        mov     mCPU,f80286
        jmp     cpu_done
found386:
        mov     mCPU,f80386
        jmp     cpu_done
badcpu:
        mov     mCPU,funk               ;how'd an 8088 get this far?????
cpu_done:
        ret
cpu     endp
;--------------------------------------------------------------------

piq     proc    near

;       On exit:
;
;               DX      = length of prefetch instruction queue
;
;       This subroutine uses self-modifying code, but can
;       nevertheless be run repeatedly in the course of the calling
;       program.

count   =       7
opincdx equ     42H                     ; inc dx opcode
opnop   equ     90H                     ; nop opcode

        mov     al,opincdx
        mov     cx,count
        push    cx
        push    cs
        pop     es
        mov     di,offset piq_01 - 1
        push    di
        std
        rep stosb
        mov     al,opnop
        pop     di
        pop     cx
        xor     dx,dx
        cli
        rep stosb
        rept    count
        inc     dx
        endm
piq_01:
        sti
        ret
piq     endp

;--------------------------------------------------------------------

chkint  proc    near

; save old INT 01H vector

        push    bx
        mov     ax,3501H
        int     21H
        mov     word ptr old_int01_ofs,bx
        mov     word ptr old_int01_seg,es
        pop     bx

; redirect INT 01H vector

        push    ds
        mov     ax,2501H
        mov     dx,seg new_int01
        mov     ds,dx
        mov     dx,offset new_int01
        int     21H
        pop     ds

; set TF and change SS -- did we trap on following instruction?

        pushf
        pop     ax
        or      ah,01H                  ; set TF
        push    ax
        popf
        push    ss                      ; CPU may wait one
                                        ; instruction before
                                        ; recognizing single step
                                        ; interrupt
        pop     ss
chkint_01::                             ; shouldn't ever trap here

; restore old INT 01H vector

        push    ds
        mov     ax,2501H
        lds     dx,dword ptr old_int01
        int     21H
        pop     ds
        ret
chkint  endp
;--------------------------------------------------------------------

new_int01       proc    near

;       INT 01H handler (single step)
;
;       On entry:
;
;       SP =>   IP
;               CS
;               flags

        sti
        pop     ax                      ; IP
        cmp     ax,offset chkint_01
        jb      new_int01_03
        je      new_int01_01
        mov     mchkint,false
        jmp     new_int01_02
new_int01_01:
        mov     mchkint,true
new_int01_02:
        pop     cx                      ; CS
        pop     dx                      ; flags
        and     dh,0FEH                 ; turn off TF
        push    dx                      ; flags
        push    cx                      ; CS
new_int01_03:
        push    ax                      ; IP
        iret
new_int01       endp
;--------------------------------------------------------------------

ndp     proc    near

fnone   equ     0
f8087   equ     1
f80287  equ     2
f80387  equ     3
funk    =       0FFH


; The next two 80x87 instructions cannot carry the WAIT prefix,
; because there may not be an 80x87 for which to wait.  The WAIT is
; therefore emulated with a MOV CX,<value>! LOOP $ combination.

.287
        mov     word ptr ndp_cw,0000H
        cli                     ;no interrupts during this test

        fninit                  ;initialize NDP
        mov     cx,2
        loop    $

        fnstcw  word ptr ndp_cw          ;store control word in ndp_cw
        mov     cx,14h
        loop    $

        sti
        mov     ax,word ptr ndp_cw       ;check for valid status word
        cmp     ah,3            ;is NDP present?
        je      ndp_01    ;if 3, must be there
        mov     mNDP,fnone
        jmp     ndp_done

ndp_01:
        cmp     ax,03FFH        ;check if 8087
        jne     ndp_02
        mov     mNDP,f8087
        jmp     ndp_04
ndp_02:

.287

        cmp     ax,037FH        ;check if 286/387/486
        jne     ndp_05    ;must be garbage

;detect 287 or 387

        fld1                    ;Load +1.0 onto NDP stack
        fldz                    ;Load +0.0 onto NDP stack
        fdiv                    ;do +1/0
        fld1                    ;Load +1.0 onto NDP stack
        fchs                    ;Change to -1.0
        fldz                    ;Load +0.0 onto NDP stack
        fdiv                    ;do -1/0
        fcom                    ;compare
        fstsw   word ptr ndp_sw
        mov     ax,word ptr ndp_sw
        and     ah,41H          ; C3, C0
        cmp     ah,40H          ; ST(0) = ST(1)
        jne     ndp_03
        mov     mNDP,f80287
        jmp     ndp_04
ndp_03:
        cmp     ah,01H          ; ST(0) < ST(1)
        jne     ndp_05
        mov     mNDP,f80387
ndp_04:

.8087
        fstcw   mNDPCW          ;save status for INFOPLUS
        ret
ndp_05:
        mov     mNDP,funk
ndp_done:
        ret
ndp     endp

;------------------------------------------------------------------------------
; This checks to see if the BIOS reports a Weitek math coprocessor. This should
; only be called if a 386 or 486 is found.
; NOTE!! This may not work with all computers!!

fnoWeitek       equ     0
fWeitek         equ     1
fWeitek_real    equ     81h

weitek  proc    near
.386
        xor     eax,eax                 ;zero everything
        int     11h                     ;do equipment check
        test    eax,01000000h           ;check bit 24, set if Weitek present
        je      no_weitek
        mov     mWeitek,fWeitek
        test    eax,0800000h            ;check bit 23, set if Weitek can be
        je      weitek_done             ; addressed in real mode
        mov     mWeitek,fWeitek_real
        jmp     weitek_done
no_weitek:
        mov     mWeitek,fnoWeitek
weitek_done:
        ret
.286
weitek  endp

;--------------------------------------------------------------------

_DISKREAD proc    far

;       On entry (C calling convention, large model):
;
;               drive                   word [bp+6]
;               starting_sector (low)   word [bp+8]
;               starting_sector (high)  word [bp+10]
;               number_of_sectors       word [bp+12]
;               buffer (offset)         word [bp+14]
;               buffer (segment)        word [bp+16]
;
;       On exit:
;
;               AX      = function result
;                       00      - function successful
;                       01..FF  - DOS INT 25H error result

        push    bp
        mov     bp,sp
        push    ds
        push    es
        mov     ax,3000h                ;get DOS version
        int     21h
        cmp     al,4                    ;DOS 4?
        jge     read4                   ;We have 4 or newer, so use extended
        cmp     ax,1d04h                ;use old for anything less than 3.30
        jle     read3
;
;Check bit 1 of the device attributes bit. If it's set, then the driver
;supports use of the extended access method
;
        push    es                      ;save regs
        push    ds
        mov     dl,byte ptr [bp+6]      ;get drive number
        inc     dl                      ;func uses 0=dflt, 1=A, etc..
        mov     ah,32h                  ;get driver parameter block
        int     21h
        push    ds                      ;move ds to es
        pop     es
        pop     ds                      ;restore original ds
        les     bx,dword ptr [es:bx + 12h]        ;point ES:BX to device driver
        test    word ptr [es:bx + 4],2  ;test device attributes
        pop     es
        jz      read3                   ;wasn't, so use old method

read4:
        mov     al,byte ptr [bp+6]      ;drive
        mov     bx,word ptr [bp+8]      ;starting_sector (lo)
        mov     word ptr extd_starting_sector_lo,bx
        mov     bx,word ptr [bp+10]     ;starting_sector (hi)
        mov     word ptr extd_starting_sector_hi,bx
        mov     bx,word ptr [bp+12]     ;number_of_sectors
        mov     word ptr extd_number_of_sectors,bx
        les     bx,dword ptr [bp+14]    ;buffer
        mov     word ptr extd_bufofs,bx
        mov     word ptr extd_bufseg,es
        mov     bx,offset dos4_block    ;DS:BX points to block
        mov     cx,-1                   ;-1 means extended read
        push    ds
        jmp     readit

read3:  mov     al,byte ptr [bp+6]
        mov     dx,word ptr [bp+8]      ;starting_sector (lo)
        mov     cx,word ptr [bp+12]     ;number_of_sectors
        push    ds
        lds     bx,dword ptr [bp+14]    ;buffer
readit: int     25H
        inc     sp                      ; fix broken stack
        inc     sp
        pop     ds
        jc      diskread_01
        xor     ax,ax
diskread_01:

        pop     es
        pop     ds
        pop     bp
        ret

_DISKREAD endp

;
;LONGCALL will call a routine using a CALL FAR.
;
;C format: void longcall(unsigned long addr, union REGS far *regs);
;

_LONGCALL        proc    far

        push    bp
        mov     bp,sp
        push    ds
        push    si
        push    di
        mov     ax,word ptr [bp+10]     ; addr (lo)
        mov     word ptr address,ax
        mov     ax,word ptr [bp+12]     ; addr (hi)
        mov     word ptr address+2,ax
        lds     si,dword ptr [bp+6]     ; regs
        mov     ds_save,ds
        mov     si_save,si
        cld
        lodsw
        push    ax
        lodsw
        mov     bx,ax
        lodsw
        mov     cx,ax
        lodsw
        mov     dx,ax
        lodsw
        mov     bp,ax
        lodsw
        push    ax
        lodsw
        mov     di,ax
        lodsw
        push    ax
        lodsw
        mov     es,ax
        lodsw
        and     ax,008D5h
        push    bx
        mov     bx,ax
        pushf
        pop     ax
        and     ax,0F72Ah
        or      ax,bx
        push    ax
        popf
        pop     bx
        pop     ds
        pop     si
        pop     ax
        call    dword ptr address
        pushf
        push    es
        push    di
        mov     es,ds_save
        mov     di,si_save
        cld
        stosw
        mov     ax,bx
        stosw
        mov     ax,cx
        stosw
        mov     ax,dx
        stosw
        mov     ax,bp
        stosw
        mov     ax,si
        stosw
        pop     ax
        stosw
        mov     ax,ds
        stosw
        pop     ax
        stosw
        pop     ax
        stosw
        pop     di
        pop     si
        pop     ds
        pop     bp
        ret

_LONGCALL endp

;
; ATIINFO is used in the Video identification routine to get special
; information from ATI VGA Wonder cards.
;
; C format: unsigned char ATIinfo(unsigned char data_in, unsigned int reg);
;
_ATIINFO         proc    far

        push    bp
        mov     bp,sp
        mov     dx,word ptr [bp+8]      ; reg
        mov     al,byte ptr [bp+6]      ; data_in
        cli
        out     dx,al
        inc     dx
        in      al,dx
        sti
        pop     bp
        ret

_ATIINFO endp

; AltIntr is an alternative to the Intr function.
;
; C format: void AltIntr(unsigned char intno, union REGS far *regs);

AltIntr        proc    far

        push    bp
        mov     bp,sp
        push    ds
        push    si
        push    di
        mov     al,byte ptr [bp+10]     ; intno
        mov     byte ptr cs:intrpt,al
        lds     si,dword ptr [bp+6]     ; regs
        mov     save_ds,ds
        mov     save_si,si
        cld
        lodsw
        push    ax
        lodsw
        mov     bx,ax
        lodsw
        mov     cx,ax
        lodsw
        mov     dx,ax
        lodsw
        mov     bp,ax
        lodsw
        push    ax
        lodsw
        mov     di,ax
        lodsw
        push    ax
        lodsw
        mov     es,ax
        lodsw
        and     ax,008D5h
        push    bx
        mov     bx,ax
        pushf
        pop     ax
        and     ax,0F72Ah
        or      ax,bx
        push    ax
        popf
        pop     bx
        pop     ds
        pop     si
        pop     ax
        db      0cdh
intrpt  db      ?
        pushf
        push    es
        push    di
        mov     es,save_ds
        mov     di,save_si
        cld
        stosw
        mov     ax,bx
        stosw
        mov     ax,cx
        stosw
        mov     ax,dx
        stosw
        mov     ax,bp
        stosw
        mov     ax,si
        stosw
        pop     ax
        stosw
        mov     ax,ds
        stosw
        pop     ax
        stosw
        pop     ax
        stosw
        pop     di
        pop     si
        pop     ds
        pop     bp
        ret

AltIntr        endp

;
; C format: void AltMsDos(union REGS far *regs);
;
AltMsDos       proc    far

        push    bp
        mov     bp,sp
        push    ds
        push    si
        push    di
        lds     si,dword ptr [bp+6]     ; regs
        mov     save_ds,ds
        mov     save_si,si
        cld
        lodsw
        push    ax
        lodsw
        mov     bx,ax
        lodsw
        mov     cx,ax
        lodsw
        mov     dx,ax
        lodsw
        mov     bp,ax
        lodsw
        push    ax
        lodsw
        mov     di,ax
        lodsw
        push    ax
        lodsw
        mov     es,ax
        lodsw
        and     ax,008D5h
        push    bx
        mov     bx,ax
        pushf
        pop     ax
        and     ax,0F72Ah
        or      ax,bx
        push    ax
        popf
        pop     bx
        pop     ds
        pop     si
        pop     ax
        int     21h
        pushf
        push    es
        push    di
        mov     es,save_ds
        mov     di,save_si
        cld
        stosw
        mov     ax,bx
        stosw
        mov     ax,cx
        stosw
        mov     ax,dx
        stosw
        mov     ax,bp
        stosw
        mov     ax,si
        stosw
        pop     ax
        stosw
        mov     ax,ds
        stosw
        pop     ax
        stosw
        pop     ax
        stosw
        pop     di
        pop     si
        pop     ds
        pop     bp
        ret

AltMsDos       endp

_CIRRUSCK        proc    far

;Cirrus VGA detection from 'Advanced Programmer's Guide to Super VGAs'

; Fetch address of CRT controller
        mov     ax,40h
        mov     es,ax
        mov     dx,es:[63h]
; clear Start Address register in CRTC (index 0Ch)
        mov     al,0ch
        out     dx,al
        inc     dx
        mov     ah,al
        in      al,dx
        xchg    ah,al
        push    ax
        push    dx
        xor     al,al
        out     dx,al
        dec     dx
; fetch unlock password
        mov     al,1fh
        out     dx,al
        inc     dx
        in      al,dx
        mov     ah,al
        mov     bh,al
        mov     ch,cl
; enable extended regs
        mov     cl,4
        mov     dx,3c4h
        mov     bl,6
        ror     bh,cl
        mov     ax,bx
        out     dx,al
        inc     dx
        in      al,dx
        or      al,al
        jnz     exit_cirrus

        mov     bh,al
        dec     dx
        mov     al,6
        out     dx,al
        inc     dx
        mov     al,ah
        out     dx,al
        in      al,dx
        cmp     al,1
        jne     exit_cirrus
        ror     bh,cl
        dec     dx
        mov     ax,bx
        out     dx,ax
        inc     dx
        in      al,dx
        cmp     al,1
        jne     exit_cirrus
        pop     dx
        dec     dx
        pop     ax
        out     dx,ax
        mov     ah,0
        mov     al,ch
        jmp     end_cirrusck
exit_cirrus:
        pop     dx
        dec     dx
        pop     ax
        out     dx,ax
        mov     ax,0
end_cirrusck:
        ret

_CIRRUSCK        endp

_CTICK   proc    far

;CTI VGA detection from 'Advanced Programmer's Guide to Super VGAs'

;place VGA in setup mode
        cli
        mov     dx,46e8h
        in      al,dx
        or      al,10h
        out     dx,al
;enable extended register bank
        mov     dx,103h
        in      al,dx
        or      al,80h
        out     dx,al
;read global ID
        mov     dx,104h
        in      al,dx
        mov     ah,al
; place vga in normal mode
        mov     dx,46e8h
        in      al,dx
        and     al,03fh
        out     dx,al
        sti
; read version extended register
        mov     dx,3d6h
        mov     al,0
        out     dx,al
        inc     dx
        in      al,dx
        cmp     ah,5ah
        jne     notcti
        and     al,0f0h
        shr     al,1
        shr     al,1
        shr     al,1
        shr     al,1
        cmp     al,2
        je      notcti
        cmp     al,4
        jge     notcti
        cmp     al,3
        je      end_ctick
        inc     al
        jmp     end_ctick
notcti:
        xor     ax,ax
end_ctick:
        ret

_CTICK   endp

_TSENGCK proc    far

;Tseng VGA detection from 'Advanced Programmer's Guide to Super VGAs'

        mov     dx,3cdh
        in      al,dx
        mov     ah,al
        and     al,0c0h
        or      al,55h
        out     dx,al
        in      al,dx
        cmp     al,55h
        jne     nottseng
        mov     al,0aah
        out     dx,al
        in      al,dx
        cmp     al,0aah
        jne     nottseng
        mov     al,ah
        out     dx,al
        mov     al,1
        jmp     end_tsengck
nottseng:
        mov     al,0
end_tsengck:
        ret

_TSENGCK endp

_ZYMOSCK proc    far

;ZyMOS VGA detection from 'Advanced Programmer's Guide to Super VGAs'

        mov     dx,3c4h
        mov     al,0bh
        out     dx,al
        inc     dx
        in      al,dx
        and     al,0fh
        cmp     al,2
        je      end_zymosck
        mov     al,0
end_zymosck:
        ret
_ZYMOSCK endp

_BUGTST  proc    far
;
; BUGTST.ASM - By: John Lauro
;              Based on bug found by Jeff Prothero
;               Adapted for Infoplus by Andrew Rossmann, 7/20/91.

.386
        mov     eax,12345678
        mov     edx, 0
        mov     edi, 0
        pushad
        popad

; The instruction immediately following popad is the critical
; instruction.  Simple fix, insert a NOP after popad.

        mov     ecx, [edx+edi]

        cmp     eax, 12345678
.286
        mov     al,0
        je      end_bugtst
        mov     al,1
end_bugtst:
        ret
_BUGTST  endp

        end
