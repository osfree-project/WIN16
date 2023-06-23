;
; This is stub for Windows 1.01 logo
; Supported up to Win 3.xx
;
; Logo API
;
; All code compiled as raw binary with offset 0.
;
; First 4 bytes must contain 'LOGO' signature
; Entry table consist of 2 entry points:
;  ShowLogo - Switch display to graphics mode, show logo and return last required byte of logo code in AX
;  HideLogo - Switch display back to original mode
;

.8086
			; MacroLib
			include	bios.inc
			include	dos.inc

GRAPH			equ	1		; Set to 1 for 320x200 VGA mode with logo

code			segment
			org	0h

strt:
			db	"LOGO"
			jmp	near ptr Init
;			jmp	near ptr Done
Done:			@SetMode [CurrentVideoMode]
			retf

CurrentVideoMode	db	?

;--------------------------------------------------------
; All bellow code will be deleted after return from Init
;--------------------------------------------------------
DeadSpace:		
if			GRAPH
ErrorMsg		db 'Error loading LOGO.BMP', 13, 10,'$'
filename db 'logo.bmp',0
filehandle dw ?
Header db 54 dup (0)
Palette db 256*4 dup (0)
ScrLine db 320 dup (0)
else
Hello			db	10,13,10,13
			db	"                             ,.=:^!^!t3Z3z.,             ",10,13
			db	"                            :tt:::tt333EE3               ",10,13
			db	"                            Et:::ztt33EEE  @Ee.,      ..,",10,13
			db	"                           ;tt:::tt333EE7 ;EEEEEEttttt33#",10,13
			db	"                          :Et:::zt333EEQ. SEEEEEttttt33QL",10,13
			db	"                          it::::tt333EEF @EEEEEEttttt33F ",10,13
			db	"                         ;3=*^```'*4EEV :EEEEEEttttt33@. ",10,13
			db	"                         ,.=::::it=., ` @EEEEEEtttz33QF  ",10,13
			db	"                        ;::::::::zt33)   '4EEEtttji3P*   ",10,13
			db	"                       :t::::::::tt33.:Z3z..  `` ,..g.   ",10,13
			db	"                       i::::::::zt33F AEEEtttt::::ztF    ",10,13
			db	"                      ;:::::::::t33V ;EEEttttt::::t3     ",10,13
			db	"                      E::::::::zt33L @EEEtttt::::z3F     ",10,13
			db	"                     {3=*^```'*4E3) ;EEEtttt:::::tZ`     ",10,13
			db	"                                 ` :EEEEtttt::::z7       ",10,13
			db	"                                     'VEzjt:;;z>*`       ",10,13,10,13,10,13
			db	"                              Windows loading...",10,13,10,13
			db	"                Text Logo Example by Yuri Prokushev (C) 2022$"
;https://github.com/nijikokun/WinScreeny/blob/master/screeny
;$f1         ,.=:^!^!t3Z3z.,                
;$f1        :tt:::tt333EE3                  
;$f1        Et:::ztt33EEE  $f2@Ee.,      ..,   
;$f1       ;tt:::tt333EE7 $f2;EEEEEEttttt33#   
;$f1      :Et:::zt333EEQ.$f2 SEEEEEttttt33QL   
;$f1      it::::tt333EEF $f2@EEEEEEttttt33F    
;$f1     ;3=*^\`\`\`'*4EEV $f2:EEEEEEttttt33@. 
;$f4     ,.=::::it=., $f1\` $f2@EEEEEEtttz33QF 
;$f4    ;::::::::zt33)   $f2'4EEEtttji3P*      
;$f4   :t::::::::tt33.$f3:Z3z..  $f2\`\` $f3,..g.     
;$f4   i::::::::zt33F$f3 AEEEtttt::::ztF      
;$f4  ;:::::::::t33V $f3;EEEttttt::::t3       
;$f4  E::::::::zt33L $f3@EEEtttt::::z3F       
;$f4 {3=*^\`\`\`'*4E3) $f3;EEEtttt:::::tZ\`   
;$f4             \` $f3:EEEEtttt::::z7        
;$f3                 $f3'VEzjt:;;z>*\`        
endif
Init:			push	ds
			push	cs
			pop	ds
			@GetMode
			mov	[CurrentVideoMode], al
if			GRAPH
			; Open file

			@OpenFil offset filename, 0
			jc openerror
			mov [filehandle], ax

			; Process BMP file
			call	ReadHeader
			call	ReadPalette
			@SetMode 13h		; 13h 40x25 320x200 256 colors VGA
			call	CopyPal
			call	CopyBitmap
			@ClosFil [filehandle]
			jmp	skip

			openerror:
			@DispStr offset ErrorMsg
			jmp	skip


ReadHeader:

    ; Read BMP file header, 54 bytes

    mov ah,3fh
    mov bx, [filehandle]
    mov cx,54
    mov dx,offset Header
    int 21h
    ret

ReadPalette:

    ; Read BMP file color palette, 256 colors * 4 bytes (400h)

    mov ah,3fh
    mov cx,400h
    mov dx,offset Palette
    int 21h
    ret

CopyPal:

    ; Copy the colors palette to the video memory
    ; The number of the first color should be sent to port 3C8h
    ; The palette is sent to port 3C9h

    mov si,offset Palette
    mov cx,256
    mov dx,3C8h
    mov al,0

    ; Copy starting color to port 3C8h

    out dx,al

    ; Copy palette itself to port 3C9h

    inc dx
    PalLoop:

    ; Note: Colors in a BMP file are saved as BGR values rather than RGB.

    mov al,[si+2] ; Get red value.
    shr al,1 ; Max. is 255, but video palette maximal
    shr al,1 ; Max. is 255, but video palette maximal

    ; value is 63. Therefore dividing by 4.

    out dx,al ; Send it.
    mov al,[si+1] ; Get green value.
    shr al,1
    shr al,1
    out dx,al ; Send it.
    mov al,[si] ; Get blue value.
    shr al,1
    shr al,1
    out dx,al ; Send it.
    add si,4 ; Point to next color.

    ; (There is a null chr. after every color.)

    loop PalLoop
    ret

CopyBitmap:

    ; BMP graphics are saved upside-down.
    ; Read the graphic line by line (200 lines in VGA format),
    ; displaying the lines from bottom to top.

    mov ax, 0A000h
    mov es, ax
    mov cx,200
    PrintBMPLoop:
    push cx

    ; di = cx*320, point to the correct screen line

    mov di,cx
    shl cx,1
    shl cx,1
    shl cx,1
    shl cx,1
    shl cx,1
    shl cx,1
    shl di,1
    shl di,1
    shl di,1
    shl di,1
    shl di,1
    shl di,1
    shl di,1
    shl di,1
    add di,cx

    ; Read one line

    mov ah,3fh
    mov cx,320
    mov dx,offset ScrLine
    int 21h

    ; Copy one line into video memory

    cld 

    ; Clear direction flag, for movsb

    mov cx,320
    mov si,offset ScrLine
    rep movsb 

    ; Copy line to the screen
    ;rep movsb is same as the following code:
    ;mov es:di, ds:si
    ;inc si
    ;inc di
    ;dec cx
    ;loop until cx=0

    pop cx
    loop PrintBMPLoop
    ret

skip:

else
			@SetMode 3
			@DispStr Hello
endif
			lea	ax, DeadSpace
			pop	ds
			retf

code			ends
			end strt
