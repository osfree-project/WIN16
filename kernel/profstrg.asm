        ; MacroLib
	include dos.inc

	include kernel.inc
	include debug.inc

	include pusha.inc
if ?REAL
		.8086
else
		.286
endif

cr	equ 13
lf	equ 10

O_RDONLY	equ 0
O_RDWR		equ 2

_TEXT	segment; word public 'CODE'

check:
        push    si
check_0:
        mov     al,[si]
        cmp     al,'a'
        jnc     @F
        or      al,20h
@@:
        mov     ah,es:[di]
        cmp     ah,'a'
        jnc     @F
        or      ah,20h
@@:
        inc     si
        inc     di
        cmp     al,ah
        jnz     @F
        loop    check_0
@@:
        mov     al,[si-1]
        pop     si
        ret

getallkeys:
        xor     al,al
getallkeys_1:
        cmp     al,'='
        jz      getallkeys_5
        lodsb
        stosb
        dec     dx
        jz      getallkeys_4
        loop    getallkeys_1
getallkeys_4:
        mov     byte ptr es:[di-1],0
        ret
getallkeys_5:
        mov     byte ptr es:[di-1],0
getallkeys_3:
        cmp     al,lf
        jz      getallkeys_2
        lodsb
        loop    getallkeys_3
        ret
getallkeys_2:
        mov     al,[si]
        cmp     al,';'
        jz      getallkeys_3
        cmp     al,' '
        jz      getallkeys_3
        cmp     al,'['
        jnz     getallkeys_1
        ret


searchsection:
        xor     di,di
        cld
        jmp     @F
searchsec_0:
        mov     al,lf
        repnz   scasb
@@:
        jcxz    searchsec_er            ;fertig, eintrag nicht gefunden
        mov     al,'['
        scasb
        jnz     searchsec_0       ;--> naechste zeile
        call    check
        jcxz    searchsec_er            ;file zuende
        cmp     byte ptr es:[di-1],']'
        jnz     searchsec_0       ;--> falsche section
        cmp     al,0
        jnz     searchsec_0       ;--> falsche section
        @trace_s <"section found",lf>
        clc
        ret
searchsec_er:
        stc
        ret

searchentry:
searchentry_0:                          ;<----
        mov     al,lf
        repnz   scasb
        jcxz    searchentry_er2         ;file zuende
        cmp     byte ptr es:[di],'['
        jz      searchentry_er1
        mov     ax,ds
        and     ax,ax
        jz      searchentry_ex
        call    check
        jcxz    searchentry_er
        cmp     byte ptr es:[di-1],'='
        jnz     searchentry_0           ;--> naechsten eintrag suchen
        cmp     al,0
        jnz     searchentry_0           ;--> naechsten eintrag suchen
searchentry_ex:
        ret
searchentry_er2:
searchentry_er1:
searchentry_er:
        stc
        ret

GetPrivateProfileString proc far pascal uses ds es lpszSection:far ptr byte,
		lpszEntry:far ptr byte, lpszDefault:far ptr byte,
		retbuff:far ptr byte, bufsize:word,
		lpszFilename:far ptr byte

local	rc:word
local	sel:word

        @push_a
        xor     ax,ax
        mov     rc,ax
	@GetBlok 1000h			;64k allokieren als puffer
        jc      getpps_ex               ;fehler: kein freier speicher
        @trace_s <"allokierung 64k ok",lf>
        mov     sel,ax
        lds     dx,lpszFilename
	@OpenFil ,O_RDONLY
        jc      getpps_ex1              ;fehler: file not found
        @trace_s <"open ok",lf>
	@Read	0, 0FFF0h, ax, sel
        jc      getpps_ex2             ;fehler: read error
        @trace_s <"read ok",lf>
        mov     cx,ax
        mov     es,sel
        lds     si,lpszSection              ;section ueberpruefen
        call    searchsection
        jc      getpps_ex2
        lds     si,lpszEntry
        call    searchentry
        jc      getpps_ex2
        mov     ax,ds
        and     ax,ax
        jnz     @F
        push    es
        pop     ds
        mov     si,di
        mov     dx,bufsize
        les     di,retbuff
        call    getallkeys
        sub     dx,bufsize
        neg     dx
        jmp     getpps_6
@@:
        @trace_s <"entry found",lf>
        mov     ax,bufsize
        cmp     ax,cx
        jnc     @F
        mov     cx,ax
@@:
        jcxz    getpps_ex2              ;puffersize = 0!
        @trace_s <"copy entry value",lf>
        push    es
        pop     ds
        mov     si,di
        les     di,retbuff
        xor     dx,dx
getpps_5:
        lodsb
        cmp     al,cr
        jz      getpps_6
        stosb
        inc     dx
        loop    getpps_5
getpps_6:
        xor     al,al
        stosb
        mov     rc,dx
getpps_ex2:
	@SetKernelDS
        @trace_s <"close file",lf>
        @ClosFil                  ;close file
getpps_ex1:
        @trace_s <"free buffer",lf>
	@FreeBlok sel
getpps_ex:
        @trace_s <"exit",lf>
        @pop_a
        mov     ax,rc
        ret

GetPrivateProfileString endp

WritePrivateProfileString proc far pascal uses ds es lpszSection:far ptr byte,
        lpszEntry:far ptr byte, lpszString:far ptr byte, lpszFilename:far ptr byte

local	rc:word
local	sel:word
local	lbuf:word

        @push_a
        xor     ax,ax
        mov     rc,ax
	@GetBlok 1000h			;64k allokieren als puffer
        jc      writepps_ex             ;fehler: kein freier speicher
        @trace_s <"allokierung 64k ok",lf>
        mov     sel,ax
        lds     dx,lpszFilename
	@OpenFil ,O_RDWR
        jc      writepps_1              ;file nicht da
        @trace_s <"open ok",lf>
	@Read	0, 0FFF0h, ax, sel
        jc      writepps_ex2              ;fehler: read error
        @trace_s <"read ok",lf>
        mov     cx,ax
        mov     es,sel
        mov     di,ax
        mov     byte ptr es:[di],0
        lds     si,lpszSection              ;section ueberpruefen
        call    searchsection
        jc      writepps_2
        lds     si,lpszEntry
        call    searchentry
        pushf
        jcxz    @F
        @MovePtr ,0,di,0
@@:
        popf
        jc      writepps_3              ;eintrag nicht gefunden
        call    writevalue
        call    skipline
        jmp     writepps_4
writepps_1:                             ;create file
	@MakFil 
        jc      writepps_ex1
        mov     bx,ax
        @trace_s <"create ok",lf>
writepps_2:                             ;section not found
        call    writeseckap
writepps_3:                             ;entry not found
        call    writeentry
        call    writevalue
writepps_4:
        call    writerest
writepps_ex2:
        @ClosFil                  	;close file
writepps_ex1:
	@FreeBlok sel
writepps_ex:
        @pop_a
        mov     ax,rc
        ret

writerest:
        @trace_s <"write rest",lf>
        mov     dx,di
        push    es
        pop     ds
        call    getstrlen
        jcxz    @F
	@Write
	xor	cx,cx
@@:
	@Write		;write with CX=0 will truncate file
        retn
writeseckap:
        @trace_s <"write section capital",lf>
        mov     al,'['
        call    writechar
        lds     dx,lpszSection
        call    getstrlen
        jcxz    @F
	@Write
@@:
        mov     al,']'
        call    writechar
        call    writecrlf
        retn
writeentry:
        @trace_s <"write entry",lf>
        lds     dx,lpszEntry
        call    getstrlen
        jcxz    @F
	@Write
@@:
        mov     al,'='
        call    writechar
        retn
writevalue:
        @trace_s <"write value",lf>
        lds     dx,lpszString
        call    getstrlen
	@Write
        call    writecrlf
        retn
writecrlf:
        mov     al,cr
        call    writechar
        mov     al,lf
        call    writechar
        retn
writechar:
        push    ds
        push    ss
        pop     ds
        mov     byte ptr lbuf,al
        lea     dx,lbuf
        mov     cx,1
	@Write
        pop     ds
        retn
getstrlen:
	xor	cx,cx
        push    bx
        mov     bx,dx
@@:
        mov     al,[bx]
        inc     cx
        inc     bx
        cmp     al,0
        jnz     @B
        dec     cx
        pop     bx
        retn
skipline:
@@:
        mov     al,es:[di]
        cmp     al,00
        jz      @F
        inc     di
        cmp     al,lf
        jnz     @B
@@:
        retn
        
WritePrivateProfileString endp

_TEXT	ends

        end

