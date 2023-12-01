;--------------------------------------------------------------------
;
;       INFOPLUS.ASM
;
;       Version 1.35
;
;       Six subprograms used by INFOPLUS.PAS:
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
;
;       Originally by:
;       Steve Grant
;       Long Beach, CA
;       January 13, 1989
;
;       mods by Andrew Rossmann (10/7/90)
;--------------------------------------------------------------------

.286P
.8087

        public  CPUID, DISKREAD, LONGCALL, ATIINFO, ALTINTR, ALTMSDOS
        public  CIRRUSCK, CTICK, TSENGCK, ZYMOSCK

CODE    segment byte

;       Conditional jumps are all coded with the SHORT qualifier in
;       order to minimize the size of the .OBJ file output of Turbo
;       Assembler.

;--------------------------------------------------------------------

CPUID   proc    far

assume  cs:CODE, ds:DATA, es:nothing, ss:nothing

;       On entry:
;
;               BP
;       SP =>   near return address
;               offset  of a cpu_info_t record
;               segment "  "     "        "
;       also, the test type byte should be a 'C' or 'N' to execute the
;       CPU or NDP tests.
;
;       On exit, the cpu_info_t record has been filled in as follows:
;
;               byte    = CPU type
;               word    = Machine Status Word
;               6 bytes = Global Descriptor Table
;               6 bytes = Interrupt Descriptor Table
;               boolean = segment register change/interrupt flag
;               byte    = NDP type
;               word    = NDP control word
;               byte    = Weitek presence
;               byte    = test type (C, N, or W)

cpu_info        equ     [bp + 6]

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
        lds     bx,cpu_info
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
        pop     ds
        pop     bp
        ret     4
CPUID   endp

;--------------------------------------------------------------------

cpu     proc    near

; interrupt of multi-prefix string instruction

        mov     mCPU,funk               ;set CPU type to unknown
        sti
        mov     cx,0FFFFH
rep     lods    byte ptr es:[si]
        jcxz    short cpu_02
        call    piq
        cmp     dx,4
        jg      short cpu_01
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
        jnz     short cpu_04
        call    piq
        cmp     dx,4
        jg      short cpu_03
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
        je      short cpu_06
        call    piq
        cmp     dx,4
        jg      short cpu_05
        mov     mCPU,f80188
        jmp     cpu_done
cpu_05:
        cmp     dx,6
        jne     short cpu_done
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
        jmp     short cpu_done
found286:
        mov     mCPU,f80286
        jmp     short cpu_done
found386:
        mov     mCPU,f80386
        jmp     short cpu_done
badcpu:
        mov     mCPU,funk               ;how'd an 8088 get this far?????
CPU_done:
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
        mov     old_int01_ofs,bx
        mov     old_int01_seg,es
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
chkint_01:                              ; shouldn't ever trap here

; restore old INT 01H vector

        push    ds
        mov     ax,2501H
        lds     dx,old_int01
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
        jb      short new_int01_03
        je      short new_int01_01
        mov     mchkint,false
        jmp     short new_int01_02
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

        fnstcw  ndp_cw          ;store control word in ndp_cw
        mov     cx,14h
        loop    $

        sti
        mov     ax,ndp_cw       ;check for valid status word
        cmp     ah,3            ;is NDP present?
        je      short ndp_01    ;if 3, must be there
        mov     mNDP,fnone
        jmp     short ndp_done

ndp_01:
        cmp     ax,03FFH        ;check if 8087
        jne     short ndp_02
        mov     mNDP,f8087
        jmp     short ndp_04
ndp_02:

.287

        cmp     ax,037FH        ;check if 286/387/486
        jne     short ndp_05    ;must be garbage

;detect 287 or 387

        fld1                    ;Load +1.0 onto NDP stack
        fldz                    ;Load +0.0 onto NDP stack
        fdiv                    ;do +1/0
        fld1                    ;Load +1.0 onto NDP stack
        fchs                    ;Change to -1.0
        fldz                    ;Load +0.0 onto NDP stack
        fdiv                    ;do -1/0
        fcom                    ;compare
        fstsw   ndp_sw
        mov     ax,ndp_sw
        and     ah,41H          ; C3, C0
        cmp     ah,40H          ; ST(0) = ST(1)
        jne     short ndp_03
        mov     mNDP,f80287
        jmp     short ndp_04
ndp_03:
        cmp     ah,01H          ; ST(0) < ST(1)
        jne     short ndp_05
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
        jmp     short weitek_done
no_weitek:
        mov     mWeitek,fnoWeitek
weitek_done:
        ret
.286
weitek  endp

;--------------------------------------------------------------------

DISKREAD        proc    far

assume cs:CODE, ds:DATA, es:nothing

;       On entry:
;
;               BP
;       SP =>   near return address
;               offset  of disk buffer
;               segment "   "     "
;               number of sectors to read
;               starting logical sector number
;               drive number (0=A, 1=B, etc.)
;
;       On exit:
;
;               AX      = function result
;                       00      - function successful
;                       01..FF  - DOS INT 25H error result

        drive                   equ     [bp + 16]
        starting_sector         equ     [bp + 12]
        number_of_sectors       equ     [bp + 10]
        buffer                  equ     [bp + 6]

        push    bp
        mov     bp,sp
        mov     ax,3000h                ;get DOS version
        int     21h
        cmp     al,4                    ;DOS 4?
        jge     read4                   ;We have 4 or newer, so use extended
        push    es                      ;save regs
        push    ds
        mov     dl,drive                ;get drive number (0=A,1=B,etc)
        mov     ah,32h                  ;get driver parameter block
        int     21h
        push    ds                      ;move ds to es
        pop     es
        pop     ds                      ;restore original ds
        les     bx,[es:bx + 12h]        ;point ES:BX to device driver
        mov     ax,[es:bx + 4]          ;get device attributes
        pop     es
        test    ax,2                    ;check if bit 1 set
        jz      read3                   ;wasn't, so use old method

read4:
        mov     al,drive
        mov     bx,starting_sector      ;copy info into parameter block
        mov     extd_starting_sector_lo,bx
        mov     bx,starting_sector + 2
        mov     extd_starting_sector_hi,bx
        mov     bx,number_of_sectors
        mov     extd_number_of_sectors,bx
        les     bx,buffer               ;get seg:ofs of buffer in ES:BX
        mov     extd_bufofs,bx          ;put into block
        mov     extd_bufseg,es
        mov     bx,offset dos4_block    ;DS:BX points to block
        mov     cx,-1                   ;-1 means extended read
        push    ds                      ;save DS (not really needed, but lets
                                        ;me share code with DOS 3 read.)
        jmp     short readit

read3:  mov     al,drive
        mov     dx,starting_sector
        mov     cx,number_of_sectors
        push    ds
        lds     bx,buffer               ;get seg:ofs of buffer in DS:BX
readit: int     25H
        inc     sp                      ; fix broken stack
        inc     sp
        pop     ds
        jc      short diskread_01
        xor     ax,ax
diskread_01:

        pop     bp
        ret     10

DISKREAD        endp

;
;LONGCALL will call a routine using a CALL FAR.
;
;Pascal format: procedure longcall(addr: longint; var regs: registers); external;
;

longcall        proc    far
        assume  cs:CODE, ds:DATA, es:nothing

regaddr equ     [bp + 6]
addr    equ     [bp + 10]

        push    bp
        mov     bp,sp
        push    ds
        mov     ax,addr                 ;copy calling address for later use
        mov     word ptr cs:address,ax
        mov     ax,addr+2
        mov     word ptr cs:address+2,ax
        lds     si,regaddr              ;get pointer to regs
        mov     cs:ds_save,ds           ;save needed ones
        mov     cs:si_save,si
        cld                             ;go forward
        lodsw                           ;load AX and hold it
        push    ax
        lodsw                           ;load BX
        mov     bx,ax
        lodsw                           ;load CX
        mov     cx,ax
        lodsw                           ;load DX
        mov     dx,ax
        lodsw                           ;load BP
        mov     bp,ax
        lodsw                           ;load SI and hold it
        push    ax
        lodsw                           ;load DI
        mov     di,ax
        lodsw                           ;load DS and hold it
        push    ax
        lodsw                           ;load ES
        mov     es,ax
        lodsw                           ;load Flags
        push    ax
        popf
        pop     ds                      ;get rest of regs
        pop     si
        pop     ax
        call    dword ptr cs:address    ;make far call
        pushf                           ;save flags and modified regs
        push    es
        push    di
        mov     es,cs:save_ds           ;get regs pointer into ES:DI
        mov     di,cs:save_si
        cld                             ;go forward
        stosw                           ;save AX
        mov     ax,bx
        stosw                           ;save BX
        mov     ax,cx
        stosw                           ;save CX
        mov     ax,dx
        stosw                           ;save DX
        mov     ax,bp
        stosw                           ;save BP
        mov     ax,si
        stosw                           ;save SI
        pop     ax
        stosw                           ;save DI
        mov     ax,ds
        stosw                           ;save DS
        pop     ax
        stosw                           ;save ES
        pop     ax
        stosw                           ;save Flags
        pop     ds                      ;restore regs
        pop     bp
        ret     8

address dd      ?
ds_save dw      ?
si_save dw      ?

longcall endp

;
; ATIINFO is used in the Video identification routine to get special
; information from ATI VGA Wonder cards.
;
; Pascal format: function ATIinfo(data_in: byte; register: word): byte;
;
ATIinfo         proc    far
        assume  cs:CODE, ds:DATA, es:NOTHING

data_in         equ     [bp+8]
register        equ     [bp+6]

        push    bp
        mov     bp,sp
        mov     dx,register             ;get register
        mov     ax,data_in              ;get command word (actually byte)
        cli                             ;no interrupts
        out     dx,al
        inc     dx                      ;next port
        in      al,dx                   ;get result
        sti                             ;restore interrupts
        mov     sp,bp
        pop     bp
        ret     4

ATIinfo endp

; AltIntr is an alternative to the Intr function. The standard Intr function
; does not do a true Interrupt!! Instead, it gets the address of the interrupt
; from the interrupt table, loads all the registers, and then does a RETF!!!
; The address of a return routine has been pushed on the stack so that it
; returns to TP and unloads the registers. This was probably done because
; Intel saw to it that all interrupt numbers must be immediate, and Borland
; didn't want to use self-modifying code.
;   NOTE: The MsDos routine is ALSO affected by this problem. It just stuffs
; a 21h into the stack, and calls Intr!!! So you can use ALTMSDOS instead!
;   Now, normally, the above procedure works perfectly fine, except under 1
; condition. When the CPU is under protected or Virtual 86 mode. When in those
; modes, a program with higher privileges can trap an interrupt and act on it.
; I found this out the hard way by going nuts wondering why I couldn't detect
; DPMI drivers or Windows!! My alternative Interrupt functions identically to
; Borlands, but uses self-modifying code to generate a true interrupt. To
; prevent possible problems with CPU pipelining, the entry point is near the
; end of the code, and then jumps back to continue.
;
; Pascal format: procedure AltIntr(intno: byte; regs: registers); external;

ALTINTRP        proc    far
        assume  cs:CODE, ds:DATA, es:NOTHING

regaddr equ     [bp + 6]
intno   equ     [bp + 10]


altcont:
        lds     si,regaddr              ;point DS:SI to regs
        mov     cs:save_ds,ds           ;save pointer for return
        mov     cs:save_si,si
        cld                             ;go forward
        lodsw                           ;load AX and hold it
        push    ax
        lodsw                           ;load BX
        mov     bx,ax
        lodsw                           ;load CX
        mov     cx,ax
        lodsw                           ;load DX
        mov     dx,ax
        lodsw                           ;load BP
        mov     bp,ax
        lodsw                           ;load SI and hold it
        push    ax
        lodsw                           ;load DI
        mov     di,ax
        lodsw                           ;load DS and hold it
        push    ax
        lodsw                           ;load ES
        mov     es,ax
        lodsw                           ;load Flags
        push    ax
        popf
        pop     ds                      ;get rest of regs
        pop     si
        pop     ax
        db      0cdh                    ;Int opcode
intrpt  db      ?                       ;loaded with real interrupt
        pushf                           ;save flags and modified regs
        push    es
        push    di
        mov     es,cs:save_ds           ;get regs pointer into ES:DI
        mov     di,cs:save_si
        cld                             ;go forward
        stosw                           ;save AX
        mov     ax,bx
        stosw                           ;save BX
        mov     ax,cx
        stosw                           ;save CX
        mov     ax,dx
        stosw                           ;save DX
        mov     ax,bp
        stosw                           ;save BP
        mov     ax,si
        stosw                           ;save SI
        pop     ax
        stosw                           ;save DI
        mov     ax,ds
        stosw                           ;save DS
        pop     ax
        stosw                           ;save ES
        pop     ax
        stosw                           ;save Flags
        pop     ds                      ;restore regs
        pop     bp
        ret     6

altintr:
        push    bp
        mov     bp,sp
        push    ds                      ;save DS, because we screw it up
        mov     al,intno                ;get interrupt number to use
        mov     cs:intrpt,al            ;and modify our code
        jmp     altcont                 ;continue with rest of code

;local storage

save_ds dw      ?
save_si dw      ?

ALTINTRP        endp
;
; Pascal format: procedure AltMsDos(var regs: registers); external;
;
ALTMSDOS        proc    far
        assume  cs:CODE, ds:DATA, es:NOTHING

        pop     si              ;back track a bit so we can stuff
        pop     dx              ;interrupt number in
        pop     cx
        pop     bx
        mov     al,21h          ;push interrupt number
        push    ax
        push    bx
        push    cx              ;restore other info
        push    dx
        push    si
        jmp     ALTINTR         ;do interrupt call

ALTMSDOS        endp

CIRRUSCK        proc    far
        assume  cs:CODE, ds:DATA, es:nothing;

;Cirrus VGA detection from 'Advanced Programmer's Guide to Super VGAs'

; Fetch address of CRT controller
        mov     ax,40h          ;BIOS segment
        mov     es,ax
        mov     dx,es:[63h]     ;get CRTC address
        push    dx              ;save
; clear Start Address register in CRTC (index 0Ch)
        mov     al,0ch          ;index of Start Address reg
        out     dx,al           ;select
        inc     dx
        in      al,dx           ;get current value
        mov     ah,al           ;save
        mov     al,0ch
        push    ax
        xor     al,al
        out     dx,al           ;clear start address reg
        dec     dx
; fetch unlock password
        mov     al,1fh
        out     dx,al           ;select id reg
        inc     dx
        in      al,dx           ;read unlock password
        mov     ah,al           ;save
; enable extended regs
        mov     dx,3c4h         ;address of sequencer
        mov     al,6
        out     dx,al           ;select extension control reg
        inc     dx
        mov     al,ah           ;get unlock password
        out     dx,al           ;enable extended regs
        in      al,dx           ;read back extension reg
        cmp     al,1
        jne     not_cirrus      ;wasn't a cirrus
;disable extended regs
        mov     al,ah           ;get unlock password
        ror     al,1            ;compute lock password
        ror     al,1
        ror     al,1
        ror     al,1
        out     dx,al           ;lock extended regs
        in      al,dx           ;read extended control reg
        or      al,al           ;is it 0
        jne     not_cirrus      ;wasn't cirrus
        pop     ax              ;restore regs
        pop     dx
        out     dx,ax           ;retore registerC
        mov     ax,1
        jmp     short end_altcirrus
not_cirrus:
        pop     ax              ;restore regs
        pop     dx
        out     dx,ax           ;restore registerC
        xor     ax,ax
end_altcirrus:
        ret

CIRRUSCK        endp

CTICK   proc    far
        assume  cs:CODE, ds:DATA, es:nothing;

;CTI VGA detection from 'Advanced Programmer's Guide to Super VGAs'

;place VGA in setup mode
        cli
        mov     dx,46e8h        ;address of setup control reg
        in      al,dx
        or      al,10h          ;turn on setup bit
        out     dx,al           ;go to setup mode
;enable extended register bank
        mov     dx,103h         ;extended reg address
        in      al,dx
        or      al,80h          ;turn enable bit on
        out     dx,al
;read global ID
        mov     dx,104h         ;global ID reg
        in      al,dx
        mov     ah,al           ;save
; place vga in normal mode
        mov     dx,46e8h
        in      al,dx
        and     al,0efh         ;disable setup
        out     dx,al
        sti
; read version extended register
        mov     dx,3d6h
        mov     al,0
        out     dx,al           ;select version register
        inc     dx
        in      al,dx
        cmp     ah,5ah          ;check for CTI ID
        jne     notcti
        and     al,0f0h         ;adjust chip id
        shr     al,1
        shr     al,1
        shr     al,1
        shr     al,1
        cmp     al,2            ;only 0, 1 and 3 are good
        je      notcti
        cmp     al,4
        jge     notcti
        cmp     al,3
        je      end_ctick
        inc     al              ;adjust to match chip number
        jmp     short end_ctick
notcti:
        xor     ax,ax
end_ctick:
        ret

CTICK   endp

TSENGCK proc    far
        assume  cs:CODE, ds:DATA, es:nothing;

;Tseng VGA detection from 'Advanced Programmer's Guide to Super VGAs'

        mov     dx,3cdh         ;page select reg
        in      al,dx
        mov     ah,al           ;save
        and     al,0c0h         ;save some bits
        or      al,55h          ;test value one
        out     dx,al           ;write it
        in      al,dx
        cmp     al,55h          ;same?
        jne     nottseng
        mov     al,0aah         ;test value two
        out     dx,al
        in      al,dx
        cmp     al,0aah         ;same
        jne     nottseng
        mov     al,1
        jmp     short end_tsengck
nottseng:
        mov     al,0
end_tsengck:
        ret

TSENGCK endp

ZYMOSCK proc    far
        assume  cs:CODE, ds:DATA, es:nothing;

;ZyMOS VGA detection from 'Advanced Programmer's Guide to Super VGAs'

        mov     dx,3c4h         ;extended reg bank
        mov     al,0bh          ;version reg
        cli
        out     dx,al
        inc     dx
        in      al,dx           ;get version
        sti
        and     al,0fh
        cmp     al,2
        mov     al,0
        jmp     short end_zymosck
        mov     al,1
end_zymosck:
        ret
ZYMOSCK endp

code    ends

;--------------------------------------------------------------------

DATA    segment byte

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


DATA    ends

        end
