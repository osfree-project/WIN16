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
			call	CloseFile
			jmp	skip

			openerror:
			@DispStr offset ErrorMsg
			jmp	skip

filename db 'logo.bmp',0
filehandle dw ?
Header db 54 dup (0)
Palette db 256*4 dup (0)
ScrLine db 320 dup (0)

CloseFile:
  @ClosFil [filehandle]
  ret

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

if			GRAPH
apple:

                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FBh, 0F7h, 0D2h 
                        db 0D2h, 0D2h, 0D2h, 0D1h, 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0F6h, 0F6h, 0F6h, 0FBh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FBh, 0D7h, 0D2h, 0AEh, 0AEh, 08Ah, 08Ah, 089h, 085h 
                        db 069h, 089h, 08Ah, 0AEh, 0B2h, 0DBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0F7h, 0AEh, 0A9h, 0A9h, 0A9h, 0A9h, 0CDh, 0CDh, 0CDh, 0F1h, 0F2h, 0F2h, 0F1h, 0F2h, 0F2h, 0F1h 
                        db 0F2h, 0F1h, 0CDh, 0CDh, 0CDh, 0ADh, 0D6h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FBh, 0D3h, 0D2h, 0AEh, 0AEh, 0AEh, 0AAh 
                        db 089h, 08Ah, 08Ah, 066h, 085h, 065h, 065h, 065h, 065h, 061h, 041h, 045h, 065h, 065h, 08Eh, 0DBh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0D2h, 0A5h, 081h, 080h, 085h, 0A5h, 0A5h, 0A9h, 0A9h, 0C9h, 0CDh, 0CDh 
                        db 0CDh, 0D1h, 0D2h, 0F1h, 0F2h, 0F2h, 0F1h, 0F2h, 0D1h, 0D1h, 0ADh, 0C9h, 0A9h, 0A9h, 0ADh, 0D7h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FBh, 0D7h, 0D2h, 0D2h 
                        db 0CEh, 0CEh, 0CEh, 0AEh, 0AAh, 0AEh, 0AAh, 08Ah, 08Ah, 08Ah, 085h, 065h, 065h, 065h, 045h, 065h 
                        db 065h, 061h, 061h, 061h, 061h, 041h, 069h, 0B2h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0D2h, 085h, 080h, 080h, 081h, 081h, 0A4h, 084h 
                        db 0A5h, 0A5h, 0A9h, 0A9h, 0C9h, 0CDh, 0CDh, 0CEh, 0F1h, 0F1h, 0F2h, 0F2h, 0F2h, 0F2h, 0F1h, 0D1h 
                        db 0CDh, 0CDh, 0ADh, 0A9h, 0A5h, 0A5h, 0A9h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0D3h, 0CEh, 0AEh, 0CEh, 0D2h, 0CEh, 0AEh, 0CEh, 0AEh, 0AEh, 0AEh, 0AAh, 0AAh, 08Ah, 08Ah 
                        db 085h, 065h, 065h, 085h, 065h, 065h, 065h, 065h, 065h, 061h, 061h, 065h, 061h, 045h, 065h, 0DBh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FBh, 0A9h, 080h, 084h 
                        db 080h, 081h, 080h, 081h, 084h, 084h, 084h, 0A5h, 0A5h, 0A9h, 0A9h, 0A9h, 0CDh, 0CDh, 0CDh, 0F1h 
                        db 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0D1h, 0CDh, 0CDh, 0C9h, 0A9h, 0A9h, 0A5h, 0A5h, 0CEh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FBh, 0AEh, 0AEh, 0AEh, 0CEh, 0AEh, 0CEh, 0CEh, 0CEh, 0CEh, 0AEh 
                        db 0CEh, 0AEh, 0AEh, 0AAh, 0AEh, 08Ah, 08Ah, 089h, 065h, 085h, 065h, 065h, 065h, 065h, 061h, 061h 
                        db 065h, 045h, 061h, 061h, 061h, 061h, 0B2h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0D6h, 085h, 084h, 080h, 080h, 080h, 081h, 081h, 080h, 080h, 085h, 085h, 081h, 085h, 0A5h 
                        db 0A9h, 0A9h, 0CDh, 0CDh, 0CDh, 0CDh, 0F1h, 0F1h, 0F1h, 0F2h, 0F2h, 0F1h, 0F1h, 0F1h, 0CDh, 0CDh 
                        db 0C9h, 0A9h, 0A5h, 0A5h, 085h, 0A9h, 0FBh, 0FFh, 0FFh, 0FFh, 0DBh, 0AAh, 0AAh, 0AEh, 0AEh, 0CEh 
                        db 0AEh, 0CEh, 0CEh, 0D2h, 0CEh, 0D2h, 0CEh, 0AEh, 0CEh, 0AEh, 0AEh, 08Ah, 08Ah, 08Ah, 08Ah, 085h 
                        db 085h, 065h, 085h, 065h, 065h, 061h, 061h, 065h, 061h, 061h, 061h, 061h, 045h, 0B2h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0D2h, 081h, 080h, 080h, 084h, 081h, 080h, 080h, 080h, 085h 
                        db 080h, 081h, 084h, 081h, 084h, 085h, 0A9h, 0A9h, 0A9h, 0ADh, 0CDh, 0CDh, 0CDh, 0F1h, 0F2h, 0F2h 
                        db 0F2h, 0F2h, 0F5h, 0F2h, 0D1h, 0D1h, 0CDh, 0A9h, 0A9h, 0A9h, 0A5h, 085h, 085h, 0FBh, 0FFh, 0FBh 
                        db 089h, 08Ah, 08Ah, 0AAh, 0AEh, 0AEh, 0AEh, 0AEh, 0D2h, 0CEh, 0CFh, 0CEh, 0CEh, 0CEh, 0AEh, 0AEh 
                        db 0AEh, 0AEh, 08Ah, 0AAh, 08Ah, 089h, 069h, 065h, 065h, 065h, 065h, 065h, 045h, 061h, 065h, 065h 
                        db 041h, 061h, 041h, 065h, 0D7h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0D2h, 080h, 084h, 081h, 080h 
                        db 084h, 085h, 080h, 080h, 080h, 084h, 085h, 081h, 080h, 081h, 085h, 084h, 0A4h, 085h, 0A9h, 0A9h 
                        db 0C9h, 0CDh, 0CDh, 0D1h, 0F1h, 0F1h, 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0F1h, 0CDh, 0CDh, 0CDh, 0A9h 
                        db 0A9h, 0A5h, 0A5h, 0A5h, 0D6h, 069h, 085h, 086h, 08Ah, 0AAh, 0AAh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh 
                        db 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0AEh, 0AEh, 0AEh, 0AEh, 0AAh, 08Ah, 08Ah, 089h, 085h, 065h, 065h 
                        db 065h, 065h, 061h, 041h, 065h, 041h, 061h, 065h, 065h, 041h, 065h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0D6h, 080h, 080h, 084h, 081h, 084h, 084h, 080h, 080h, 084h, 081h, 084h, 080h, 081h, 080h, 084h 
                        db 080h, 085h, 084h, 0A5h, 0A5h, 0A9h, 0A9h, 0C9h, 0CDh, 0CDh, 0D1h, 0D1h, 0F2h, 0F2h, 0F1h, 0F2h 
                        db 0F2h, 0F1h, 0D1h, 0D1h, 0CDh, 0ADh, 0A9h, 0A9h, 085h, 085h, 085h, 065h, 065h, 085h, 086h, 08Ah 
                        db 08Ah, 0AAh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh, 0CEh, 0D2h, 0CEh, 0CEh, 0CEh, 0D2h, 0CEh, 0AEh, 0AEh 
                        db 08Ah, 08Ah, 085h, 089h, 085h, 085h, 065h, 065h, 065h, 061h, 061h, 065h, 061h, 061h, 061h, 061h 
                        db 061h, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FBh, 0A5h, 080h, 084h, 080h, 080h, 080h, 081h, 081h, 081h, 081h 
                        db 085h, 080h, 080h, 080h, 085h, 084h, 084h, 080h, 084h, 085h, 085h, 0A9h, 0A9h, 0C9h, 0C9h, 0CDh 
                        db 0CDh, 0D1h, 0F1h, 0D1h, 0F2h, 0F1h, 0F1h, 0F2h, 0F2h, 0F2h, 0D1h, 0CDh, 0ADh, 0A9h, 0A9h, 0A5h 
                        db 085h, 085h, 065h, 065h, 085h, 089h, 08Ah, 08Ah, 0AAh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh, 0CEh, 0D2h 
                        db 0AEh, 08Ah, 069h, 069h, 089h, 08Ah, 0AEh, 0AAh, 08Ah, 08Ah, 08Ah, 065h, 065h, 065h, 065h, 065h 
                        db 065h, 061h, 061h, 061h, 041h, 065h, 061h, 065h, 0DBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 089h, 081h, 080h, 080h, 081h 
                        db 081h, 084h, 081h, 080h, 081h, 084h, 080h, 080h, 080h, 080h, 081h, 084h, 084h, 081h, 080h, 080h 
                        db 085h, 0A5h, 0A5h, 0A5h, 0A9h, 0A9h, 0CDh, 0CDh, 0CDh, 0D2h, 0F2h, 0F2h, 0F2h, 0F2h, 0F1h, 0F2h 
                        db 0CEh, 0CDh, 0CDh, 0C9h, 0A9h, 0A9h, 0A5h, 0A5h, 065h, 065h, 065h, 089h, 089h, 08Ah, 08Ah, 0AAh 
                        db 0AEh, 0AEh, 0CEh, 0CEh, 0AEh, 049h, 024h, 024h, 024h, 024h, 000h, 000h, 040h, 089h, 0AEh, 08Ah 
                        db 086h, 089h, 085h, 085h, 065h, 065h, 065h, 065h, 065h, 061h, 061h, 061h, 065h, 065h, 0B2h, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 08Eh, 085h, 080h, 084h, 080h, 080h, 080h, 084h, 080h, 084h, 080h, 081h, 085h, 080h, 085h, 085h 
                        db 080h, 080h, 080h, 085h, 081h, 085h, 085h, 084h, 085h, 085h, 0A9h, 0C9h, 0CDh, 0CDh, 0CDh, 0CDh 
                        db 0D1h, 0D2h, 0F1h, 0F1h, 0F2h, 0F2h, 0F1h, 0D1h, 0CDh, 0CDh, 0A9h, 0C9h, 0A9h, 0A5h, 0A5h, 065h 
                        db 065h, 085h, 085h, 086h, 08Ah, 0AAh, 0AAh, 0AEh, 0CEh, 0AEh, 06Dh, 0B6h, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0DBh, 092h, 024h, 000h, 045h, 0AEh, 08Ah, 08Ah, 089h, 085h, 065h, 065h, 065h, 065h, 065h, 061h 
                        db 065h, 061h, 041h, 065h, 065h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 084h, 081h, 080h, 080h, 084h, 080h, 080h, 084h, 0A0h, 080h 
                        db 080h, 081h, 081h, 081h, 080h, 0A0h, 081h, 080h, 084h, 060h, 040h, 020h, 000h, 000h, 000h, 000h 
                        db 020h, 064h, 0A9h, 0C9h, 0CDh, 0CDh, 0D1h, 0D1h, 0F1h, 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0F1h, 0CDh 
                        db 0CDh, 0CDh, 0A9h, 0A9h, 0A9h, 085h, 065h, 065h, 065h, 085h, 086h, 08Ah, 08Ah, 08Eh, 0AEh, 0B2h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 000h, 065h, 0AEh, 08Ah, 08Ah, 089h 
                        db 085h, 085h, 065h, 065h, 065h, 065h, 061h, 065h, 061h, 061h, 061h, 0DBh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 069h, 085h, 080h, 085h, 081h 
                        db 081h, 081h, 085h, 080h, 081h, 080h, 084h, 084h, 084h, 080h, 080h, 085h, 0A1h, 060h, 020h, 024h 
                        db 06Dh, 092h, 0B6h, 0B6h, 092h, 06Dh, 024h, 000h, 000h, 064h, 0CDh, 0CDh, 0CDh, 0D1h, 0F1h, 0F2h 
                        db 0F2h, 0F1h, 0F2h, 0F2h, 0F2h, 0D1h, 0CDh, 0ADh, 0CDh, 0A9h, 0A9h, 0A5h, 065h, 065h, 065h, 085h 
                        db 086h, 089h, 089h, 0AAh, 0AAh, 0DBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 049h, 020h, 0AEh, 0AAh, 08Ah, 08Ah, 089h, 065h, 069h, 065h, 065h, 065h, 061h, 061h, 061h, 061h 
                        db 065h, 0D6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0DBh, 084h, 084h, 080h, 080h, 081h, 080h, 081h, 081h, 080h, 080h, 080h, 080h, 080h, 080h, 080h 
                        db 0A0h, 0A5h, 060h, 049h, 0B6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 049h, 000h 
                        db 040h, 0A9h, 0CDh, 0CEh, 0D1h, 0D1h, 0F2h, 0F2h, 0F1h, 0F6h, 0F2h, 0D2h, 0CDh, 0CDh, 0CDh, 0CDh 
                        db 0A9h, 0A9h, 085h, 065h, 065h, 065h, 065h, 086h, 085h, 089h, 08Ah, 0D7h, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 000h, 089h, 0AEh, 0AAh, 08Ah, 08Ah, 08Ah, 085h, 085h 
                        db 065h, 065h, 065h, 065h, 045h, 061h, 065h, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 069h, 080h, 080h, 081h, 084h, 080h, 080h, 085h, 081h, 085h 
                        db 081h, 081h, 081h, 081h, 080h, 080h, 085h, 060h, 06Dh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 092h, 000h, 024h, 0CDh, 0CDh, 0CDh, 0CDh, 0F1h, 0D2h, 0F2h, 0F2h 
                        db 0F2h, 0F2h, 0F2h, 0CDh, 0CDh, 0CDh, 0CDh, 0A9h, 0A9h, 061h, 061h, 065h, 065h, 065h, 085h, 085h 
                        db 08Ah, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 065h, 0CEh 
                        db 0AEh, 0AAh, 08Ah, 089h, 069h, 065h, 065h, 065h, 045h, 041h, 045h, 040h, 045h, 0D7h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 064h, 084h, 084h, 080h 
                        db 080h, 080h, 081h, 080h, 080h, 080h, 081h, 081h, 080h, 080h, 085h, 085h, 081h, 0B2h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 000h, 064h, 0CDh 
                        db 0CDh, 0CDh, 0CDh, 0F1h, 0F1h, 0F1h, 0F2h, 0F2h, 0F2h, 0D1h, 0D1h, 0D1h, 0CDh, 0CDh, 0A9h, 061h 
                        db 041h, 061h, 065h, 065h, 085h, 085h, 089h, 08Ah, 08Ah, 0D7h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 092h, 000h, 020h, 020h, 000h, 000h, 000h, 000h, 000h, 004h, 000h, 024h, 024h 
                        db 024h, 049h, 06Dh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 092h, 080h, 084h, 081h, 081h, 080h, 081h, 080h, 080h, 080h, 080h, 080h, 085h, 084h, 080h 
                        db 080h, 084h, 085h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 06Dh, 000h, 0A9h, 0CDh, 0C9h, 0CDh, 0D1h, 0D1h, 0F2h, 0F2h, 0F2h, 0F2h, 0D2h 
                        db 0F2h, 0D1h, 0CDh, 0CDh, 0C9h, 085h, 061h, 061h, 065h, 061h, 065h, 065h, 085h, 089h, 085h, 08Ah 
                        db 0AAh, 0D2h, 0D7h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 092h, 092h, 0B6h, 0B6h, 0DBh, 0DBh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 085h, 085h, 084h, 080h, 084h, 084h, 080h, 080h 
                        db 084h, 084h, 084h, 085h, 084h, 080h, 084h, 084h, 0D6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 000h, 064h, 0C9h, 0ADh, 0CDh, 0CDh 
                        db 0F1h, 0F1h, 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0F1h, 0CEh, 0D1h, 0CDh, 089h, 061h, 061h, 041h, 065h 
                        db 065h, 065h, 065h, 065h, 089h, 08Ah, 08Ah, 0AEh, 0AEh, 0AEh, 0AEh, 0D2h, 0D7h, 0FBh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 044h, 0A5h, 085h 
                        db 085h, 085h, 081h, 081h, 084h, 081h, 084h, 080h, 084h, 085h, 080h, 080h, 080h, 085h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 04Dh, 020h, 0A9h, 0A9h, 0A9h, 0CDh, 0D1h, 0D1h, 0EDh, 0F2h, 0F2h, 0F6h, 0F6h, 0F1h, 0F1h, 0D1h 
                        db 0CDh, 0A9h, 045h, 065h, 065h, 061h, 065h, 065h, 065h, 065h, 085h, 08Ah, 08Ah, 08Ah, 0AAh, 0AEh 
                        db 0AEh, 0CEh, 0CEh, 0CEh, 0D2h, 0D3h, 0F7h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0DBh, 044h, 0C9h, 0A5h, 084h, 085h, 081h, 081h, 084h, 080h, 081h, 080h, 080h, 084h 
                        db 081h, 080h, 084h, 089h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 000h, 0A5h, 0A9h, 0A9h, 0C9h, 0CDh, 0CDh, 0CDh, 0F1h 
                        db 0D2h, 0D2h, 0F2h, 0F2h, 0F1h, 0F1h, 0D1h, 0CDh, 065h, 061h, 065h, 061h, 061h, 065h, 065h, 065h 
                        db 065h, 065h, 089h, 08Ah, 08Ah, 0AAh, 0AEh, 0AEh, 0AEh, 0AEh, 0CEh, 0D2h, 0B2h, 0CEh, 0D2h, 0D3h 
                        db 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 069h, 0C9h, 0A9h, 0A5h, 085h, 0A4h, 081h 
                        db 081h, 081h, 080h, 080h, 080h, 084h, 084h, 084h, 080h, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 000h, 064h, 0A5h 
                        db 0A5h, 0A9h, 0CDh, 0CDh, 0D1h, 0D1h, 0F1h, 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0F1h, 0D1h, 085h, 065h 
                        db 061h, 065h, 065h, 065h, 061h, 065h, 065h, 085h, 085h, 085h, 089h, 0AAh, 0AEh, 0AAh, 0AEh, 0AEh 
                        db 0AEh, 0CEh, 0CEh, 0D2h, 0CEh, 0CEh, 0CEh, 0CEh, 0D3h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 092h, 0A9h 
                        db 0CDh, 0A9h, 0A9h, 085h, 085h, 085h, 084h, 080h, 080h, 085h, 084h, 080h, 080h, 084h, 080h, 0D2h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 024h, 060h, 0A5h, 0A5h, 0A5h, 0A9h, 0ADh, 0CDh, 0ADh, 0CDh, 0D1h, 0D2h, 0F2h 
                        db 0F2h, 0F2h, 0F2h, 0F2h, 089h, 041h, 065h, 061h, 065h, 065h, 041h, 065h, 065h, 065h, 065h, 085h 
                        db 089h, 08Ah, 0AAh, 0AAh, 0AEh, 0AEh, 0CEh, 0AEh, 0CEh, 0CEh, 0D2h, 0CEh, 0CEh, 0CEh, 0AEh, 0AEh 
                        db 0D2h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 0ADh, 0CDh, 0C9h, 0A9h, 0A9h, 085h, 0A5h, 085h, 085h, 081h, 084h 
                        db 080h, 080h, 084h, 084h, 081h, 0D7h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 040h, 0A5h, 085h, 0A5h, 0A9h, 0A9h 
                        db 0CDh, 0CDh, 0CDh, 0CDh, 0D1h, 0F2h, 0F2h, 0F2h, 0D2h, 0F2h, 085h, 061h, 065h, 061h, 061h, 061h 
                        db 061h, 065h, 065h, 065h, 065h, 065h, 089h, 08Ah, 08Ah, 08Ah, 0AEh, 0AEh, 0AEh, 0AEh, 0AEh, 0CEh 
                        db 0D2h, 0CEh, 0CEh, 0CEh, 0CEh, 0AEh, 0AEh, 0AEh, 0D2h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0CDh, 0EDh, 0CDh, 0C9h, 0A9h 
                        db 0A9h, 0A5h, 085h, 081h, 080h, 080h, 084h, 081h, 080h, 085h, 080h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh 
                        db 040h, 085h, 084h, 085h, 0A9h, 0A9h, 0A9h, 0CDh, 0CDh, 0CDh, 0CDh, 0F2h, 0F1h, 0F2h, 0F2h, 0F5h 
                        db 089h, 061h, 061h, 061h, 045h, 045h, 065h, 061h, 061h, 065h, 065h, 065h, 065h, 085h, 085h, 08Ah 
                        db 0AAh, 0AAh, 0AEh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0AEh, 0AEh, 0AEh, 0AAh, 0AEh 
                        db 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 024h, 0CDh, 0F1h, 0CDh, 0CDh, 0ADh, 0A9h, 0A9h, 0A5h, 0A5h, 0A0h, 081h, 081h, 081h, 084h, 080h 
                        db 085h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 092h, 040h, 081h, 081h, 0A5h, 0A5h, 0A5h, 0A9h, 0ADh, 0CDh, 0CDh 
                        db 0CDh, 0CDh, 0F1h, 0F1h, 0F2h, 0F2h, 089h, 041h, 061h, 061h, 061h, 061h, 061h, 065h, 061h, 061h 
                        db 061h, 065h, 065h, 065h, 085h, 085h, 089h, 0AAh, 0AAh, 0AEh, 0AEh, 0AEh, 0CEh, 0D2h, 0CEh, 0D2h 
                        db 0CEh, 0AEh, 0CEh, 0AEh, 0CEh, 0AAh, 0AAh, 0DBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 024h, 0CDh, 0F2h, 0D1h, 0CDh, 0CDh, 0A9h, 0A9h, 0A9h, 0A5h 
                        db 0A5h, 085h, 084h, 080h, 081h, 080h, 085h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 092h, 040h, 084h, 080h, 081h 
                        db 085h, 0A4h, 0A9h, 0A9h, 0A9h, 0CDh, 0CDh, 0CDh, 0F1h, 0F1h, 0F2h, 0F2h, 089h, 061h, 061h, 065h 
                        db 061h, 061h, 065h, 061h, 061h, 065h, 065h, 065h, 061h, 065h, 085h, 065h, 086h, 08Ah, 08Ah, 0AAh 
                        db 0AEh, 0AEh, 0AEh, 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0B2h, 0CEh, 0CEh, 0AEh, 0AEh, 0AAh, 0FBh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 024h, 0D1h, 0F2h, 0F1h 
                        db 0CDh, 0CDh, 0CDh, 0C9h, 0A9h, 0A9h, 0A5h, 085h, 080h, 084h, 084h, 080h, 080h, 0FBh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 092h, 040h, 085h, 080h, 085h, 085h, 085h, 0A5h, 085h, 0A9h, 0A9h, 0A9h, 0CDh, 0CDh, 0CDh 
                        db 0D2h, 0F1h, 065h, 041h, 061h, 061h, 061h, 061h, 061h, 061h, 065h, 065h, 065h, 061h, 065h, 065h 
                        db 065h, 065h, 089h, 089h, 08Ah, 08Ah, 0AEh, 0AAh, 0AEh, 0CEh, 0CEh, 0CEh, 0B2h, 0D2h, 0CEh, 0CEh 
                        db 0CEh, 0CEh, 0AEh, 0AEh, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 024h, 0CDh, 0F2h, 0F2h, 0D1h, 0CDh, 0CDh, 0CDh, 0ADh, 0A9h, 0A9h, 0A5h, 085h, 085h 
                        db 084h, 084h, 081h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 092h, 060h, 080h, 080h, 080h, 080h, 084h, 0A5h, 085h 
                        db 0A5h, 0A9h, 0A9h, 0A9h, 0CDh, 0CDh, 0D1h, 0F2h, 044h, 000h, 020h, 041h, 061h, 065h, 065h, 045h 
                        db 065h, 061h, 065h, 065h, 065h, 065h, 065h, 065h, 065h, 085h, 089h, 08Ah, 0AAh, 0AEh, 0AEh, 0AEh 
                        db 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0CEh, 0AEh, 0D7h, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 024h, 0ADh, 0F2h, 0F1h, 0F2h, 0F2h, 0D1h, 0CDh 
                        db 0CDh, 0C9h, 0A9h, 0A9h, 0A5h, 085h, 085h, 085h, 080h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 060h, 080h 
                        db 084h, 080h, 080h, 080h, 080h, 085h, 0A5h, 0A9h, 0A9h, 0A9h, 0CDh, 0C9h, 0CDh, 0D1h, 0D6h, 092h 
                        db 028h, 000h, 000h, 020h, 041h, 061h, 061h, 061h, 045h, 065h, 061h, 061h, 065h, 065h, 065h, 089h 
                        db 069h, 089h, 08Ah, 08Ah, 0AEh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh, 0CEh, 0D2h, 0CEh, 0D2h, 0CEh, 0AEh 
                        db 0AEh, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 028h, 0ADh 
                        db 0F6h, 0F2h, 0F2h, 0F1h, 0D1h, 0CDh, 0CDh, 0CDh, 0ADh, 0A9h, 0A9h, 085h, 085h, 085h, 081h, 0D6h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 06Dh, 060h, 081h, 085h, 081h, 080h, 085h, 085h, 084h, 085h, 0A5h, 0A5h, 0A9h 
                        db 0A9h, 0CDh, 0CDh, 0D1h, 0FBh, 0FFh, 0FFh, 0B6h, 049h, 000h, 000h, 000h, 020h, 040h, 041h, 061h 
                        db 065h, 061h, 065h, 061h, 065h, 065h, 085h, 085h, 089h, 08Ah, 08Ah, 0AEh, 0AEh, 0AEh, 0AEh, 0AEh 
                        db 0CEh, 0D2h, 0CEh, 0CEh, 0CEh, 0AEh, 0AEh, 0CEh, 0D7h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ADh, 0F6h, 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0CDh, 0CDh, 0CDh, 0ADh 
                        db 0A9h, 0A5h, 0A5h, 085h, 085h, 0D2h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 064h, 080h, 080h, 080h, 080h, 080h 
                        db 080h, 080h, 085h, 0A5h, 0A5h, 0A5h, 0A5h, 0C9h, 0C9h, 0CDh, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0DAh, 06Dh, 024h, 000h, 000h, 000h, 020h, 040h, 045h, 061h, 045h, 065h, 065h, 085h, 089h, 06Ah 
                        db 08Ah, 0AAh, 0AAh, 0AEh, 0AEh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh, 0D2h, 0CEh, 0AEh, 0AEh, 0D2h, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 069h, 0F2h, 0F2h, 0F2h, 0F2h 
                        db 0F2h, 0F2h, 0F1h, 0D2h, 0CDh, 0CDh, 0A9h, 0A9h, 0A5h, 085h, 085h, 0A9h, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 044h 
                        db 081h, 080h, 080h, 080h, 081h, 080h, 085h, 084h, 081h, 081h, 0A5h, 085h, 0A5h, 0A9h, 0A9h, 0CDh 
                        db 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 06Dh, 024h, 000h, 000h, 000h 
                        db 000h, 020h, 045h, 065h, 085h, 065h, 089h, 08Ah, 08Ah, 08Ah, 0AAh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh 
                        db 0CEh, 0CEh, 0D2h, 0CEh, 0CEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 096h, 044h, 0EDh, 0F2h, 0F1h, 0F2h, 0F2h, 0F2h, 0F2h, 0CDh, 0D2h, 0CDh, 0C9h, 0C9h, 0A9h, 0A5h 
                        db 085h, 0A5h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 044h, 080h, 084h, 085h, 084h, 085h, 085h, 084h, 080h, 084h, 080h 
                        db 084h, 085h, 085h, 0A5h, 0A9h, 0C9h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 04Dh, 000h, 000h, 000h, 020h, 065h, 065h, 065h, 08Ah, 08Ah, 08Ah 
                        db 08Ah, 0AAh, 0AEh, 0CEh, 0CEh, 0AEh, 0D2h, 0CEh, 0D2h, 0CEh, 0CEh, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 000h, 0CDh, 0D1h, 0D1h, 0F1h, 0F2h, 0F1h, 0F1h, 0F1h 
                        db 0D1h, 0D1h, 0CDh, 0C9h, 0C9h, 0A9h, 0A9h, 0A5h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 060h, 0A1h, 085h, 080h, 080h 
                        db 085h, 084h, 085h, 081h, 085h, 081h, 085h, 085h, 085h, 085h, 085h, 085h, 08Eh, 08Ah, 089h, 089h 
                        db 089h, 08Eh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 024h, 000h 
                        db 020h, 065h, 065h, 089h, 089h, 089h, 08Ah, 0AAh, 0AEh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh, 0CEh, 0D3h 
                        db 0CEh, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 000h, 0ADh, 0F1h 
                        db 0D1h, 0F1h, 0D2h, 0F6h, 0F2h, 0F2h, 0D2h, 0F1h, 0CDh, 0CDh, 0CDh, 0ADh, 0A9h, 0A9h, 0D2h, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 092h, 080h, 081h, 081h, 080h, 080h, 0A0h, 085h, 085h, 0AAh, 08Ah, 089h, 089h, 085h, 065h, 065h 
                        db 065h, 065h, 065h, 065h, 061h, 065h, 061h, 065h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 04Dh, 000h, 020h, 085h, 085h, 069h, 089h, 08Ah, 08Ah, 0AAh, 0AEh 
                        db 0AEh, 0AEh, 0CEh, 0D2h, 0CEh, 0CEh, 0CEh, 0DBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 024h, 069h, 0CDh, 0CDh, 0D1h, 0F1h, 0F2h, 0F2h, 0F2h, 0F1h, 0F1h, 0F1h, 0CDh 
                        db 0CDh, 0CDh, 0CDh, 0A9h, 0A9h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 065h, 0A5h, 080h, 081h, 084h, 084h, 085h, 060h, 020h, 0AAh 
                        db 0AAh, 089h, 089h, 086h, 086h, 085h, 085h, 065h, 065h, 065h, 065h, 065h, 061h, 061h, 0D7h, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 000h, 065h, 065h 
                        db 065h, 085h, 089h, 08Ah, 08Ah, 08Ah, 0AEh, 0AEh, 0AEh, 0AEh, 0CEh, 0CEh, 0D2h, 0FBh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 020h, 0C9h, 0CDh, 0CDh, 0CDh, 0F1h 
                        db 0F2h, 0F2h, 0F2h, 0F2h, 0F2h, 0D2h, 0CEh, 0CDh, 0CDh, 0A9h, 0C9h, 0CDh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 084h, 081h, 080h, 080h 
                        db 084h, 080h, 084h, 060h, 000h, 0AAh, 08Ah, 08Ah, 089h, 08Ah, 089h, 089h, 085h, 065h, 065h, 065h 
                        db 065h, 065h, 065h, 065h, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0B6h, 000h, 065h, 065h, 065h, 065h, 065h, 089h, 089h, 08Ah, 0AAh, 0AEh, 0AEh, 0AEh 
                        db 0AEh, 0CEh, 0B2h, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DAh 
                        db 020h, 0A9h, 0CDh, 0CDh, 0EDh, 0D1h, 0F1h, 0F1h, 0F1h, 0F1h, 0F1h, 0F1h, 0D1h, 0F1h, 0CDh, 0CDh 
                        db 0CDh, 0A9h, 0D6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 089h, 080h, 081h, 081h, 084h, 080h, 081h, 085h, 080h, 000h, 069h, 0AAh, 08Ah, 08Ah, 089h 
                        db 08Ah, 089h, 085h, 065h, 065h, 085h, 065h, 065h, 065h, 065h, 065h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 020h, 065h, 065h, 065h, 065h, 065h, 085h 
                        db 085h, 08Ah, 08Ah, 0AEh, 0AAh, 0AEh, 0AEh, 0CEh, 0CEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0F4h, 0F0h, 0D0h, 0D0h, 0F4h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h 
                        db 0D0h, 0F4h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F9h, 0F9h, 0F9h, 0F9h, 0FDh, 0FDh, 0FDh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0D6h, 085h, 080h, 0A5h, 080h, 084h, 080h, 080h, 080h, 080h, 084h 
                        db 020h, 045h, 0AAh, 08Ah, 08Ah, 08Ah, 08Ah, 089h, 085h, 085h, 065h, 085h, 065h, 065h, 065h, 061h 
                        db 065h, 08Eh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 040h 
                        db 065h, 061h, 065h, 065h, 065h, 065h, 089h, 089h, 089h, 08Ah, 08Ah, 0AEh, 0AEh, 0CEh, 0D2h, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h, 0D0h, 0D0h 
                        db 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F4h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F9h 
                        db 0F9h, 0F9h, 0F9h, 0F9h, 0F9h, 0FDh, 0FFh, 0FFh, 0FBh, 0D2h, 0A9h, 081h, 081h, 080h, 080h, 080h 
                        db 084h, 081h, 080h, 081h, 080h, 084h, 040h, 000h, 0AAh, 0AAh, 08Ah, 08Ah, 08Ah, 08Ah, 089h, 065h 
                        db 085h, 085h, 065h, 065h, 065h, 065h, 065h, 065h, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 092h, 065h, 061h, 061h, 065h, 065h, 065h, 065h, 065h, 085h, 086h, 08Ah 
                        db 08Ah, 0AAh, 0AAh, 0AEh, 0D7h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 049h, 0ACh, 0F0h, 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F4h, 0F0h 
                        db 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F9h, 0F9h, 0F9h, 0F9h, 0F9h, 0F9h, 0A9h, 0A5h, 084h, 080h 
                        db 080h, 080h, 080h, 080h, 084h, 080h, 084h, 084h, 080h, 080h, 085h, 0A4h, 060h, 000h, 089h, 0AAh 
                        db 08Eh, 08Ah, 08Ah, 089h, 089h, 089h, 089h, 085h, 085h, 065h, 065h, 065h, 065h, 065h, 065h, 08Ah 
                        db 0DBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B2h, 065h, 061h, 061h, 061h, 065h, 065h 
                        db 061h, 065h, 065h, 065h, 085h, 08Ah, 08Ah, 0AAh, 08Ah, 0AAh, 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0B0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h 
                        db 0D0h, 0D0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F4h, 0D4h, 0F4h, 0F4h, 0D4h, 0F4h, 0F9h, 0F8h, 0F9h 
                        db 0FDh, 0F9h, 0A4h, 081h, 084h, 081h, 084h, 081h, 080h, 081h, 080h, 081h, 080h, 080h, 080h, 084h 
                        db 081h, 084h, 080h, 000h, 020h, 0AEh, 0AEh, 0AAh, 0AAh, 08Ah, 0AAh, 08Ah, 086h, 089h, 085h, 089h 
                        db 065h, 065h, 065h, 065h, 065h, 065h, 061h, 08Ah, 0B2h, 0D7h, 0D7h, 0D7h, 0D7h, 0B2h, 069h, 061h 
                        db 061h, 061h, 061h, 065h, 061h, 061h, 061h, 065h, 065h, 065h, 065h, 085h, 08Ah, 08Ah, 08Ah, 0AEh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h 
                        db 0D0h, 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D4h, 0D0h, 0D0h, 0F0h, 0F4h 
                        db 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F9h, 0F9h, 0A4h, 085h, 080h, 084h, 085h, 080h, 081h, 080h 
                        db 081h, 080h, 084h, 080h, 085h, 081h, 080h, 085h, 081h, 040h, 000h, 08Ah, 0AEh, 0AAh, 08Ah, 0AAh 
                        db 08Ah, 08Ah, 086h, 089h, 089h, 085h, 085h, 065h, 085h, 065h, 065h, 065h, 065h, 065h, 061h, 061h 
                        db 041h, 061h, 061h, 061h, 061h, 061h, 065h, 065h, 061h, 065h, 065h, 061h, 045h, 045h, 065h, 065h 
                        db 065h, 065h, 085h, 085h, 08Ah, 0D7h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 049h, 0ACh, 0F4h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h 
                        db 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F9h, 0F9h, 0F9h, 0A9h, 084h 
                        db 081h, 081h, 081h, 080h, 080h, 084h, 080h, 084h, 0A0h, 084h, 080h, 084h, 084h, 080h, 084h, 080h 
                        db 000h, 024h, 0AEh, 0AAh, 0AAh, 0AAh, 08Ah, 08Ah, 08Ah, 08Ah, 089h, 065h, 065h, 085h, 065h, 065h 
                        db 065h, 065h, 065h, 065h, 061h, 065h, 061h, 061h, 045h, 061h, 061h, 065h, 041h, 061h, 061h, 061h 
                        db 061h, 041h, 061h, 061h, 065h, 065h, 065h, 065h, 065h, 085h, 0AEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F4h, 0F4h, 0F0h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h 
                        db 0F8h, 0F8h, 0F9h, 0F8h, 0A9h, 085h, 084h, 081h, 080h, 084h, 084h, 080h, 081h, 080h, 081h, 080h 
                        db 081h, 081h, 080h, 080h, 081h, 081h, 040h, 000h, 069h, 0AEh, 08Ah, 0AAh, 0AEh, 0AAh, 08Ah, 08Ah 
                        db 08Ah, 089h, 089h, 085h, 085h, 065h, 065h, 065h, 065h, 065h, 065h, 065h, 061h, 061h, 065h, 061h 
                        db 045h, 061h, 061h, 065h, 061h, 061h, 041h, 065h, 065h, 061h, 065h, 041h, 061h, 065h, 065h, 085h 
                        db 0FBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh 
                        db 0D0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 068h, 044h, 044h, 044h, 044h, 044h 
                        db 044h, 044h, 044h, 044h, 048h, 048h, 048h, 068h, 068h, 0ADh, 0A9h, 0A5h, 0A5h, 085h, 080h, 084h 
                        db 080h, 084h, 084h, 080h, 081h, 080h, 080h, 080h, 080h, 080h, 080h, 080h, 080h, 000h, 000h, 089h 
                        db 0AEh, 0AEh, 0AAh, 0AAh, 0AAh, 08Ah, 08Ah, 08Ah, 089h, 069h, 085h, 065h, 085h, 065h, 065h, 065h 
                        db 065h, 065h, 065h, 045h, 061h, 061h, 061h, 061h, 065h, 061h, 065h, 061h, 061h, 065h, 065h, 065h 
                        db 061h, 065h, 061h, 065h, 045h, 0B2h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h 
                        db 044h, 020h, 040h, 024h, 044h, 044h, 044h, 044h, 044h, 044h, 044h, 044h, 044h, 040h, 044h, 085h 
                        db 0A9h, 0A5h, 085h, 085h, 080h, 081h, 081h, 080h, 060h, 081h, 0A5h, 0A5h, 0A5h, 080h, 080h, 080h 
                        db 080h, 080h, 069h, 0DBh, 020h, 000h, 089h, 0AEh, 0AAh, 08Ah, 0AEh, 08Ah, 08Ah, 08Ah, 08Ah, 08Ah 
                        db 089h, 085h, 085h, 085h, 065h, 065h, 065h, 065h, 065h, 065h, 061h, 061h, 065h, 061h, 061h, 065h 
                        db 061h, 061h, 065h, 065h, 065h, 061h, 061h, 061h, 061h, 041h, 06Dh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h, 0F0h, 0D0h 
                        db 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0A9h, 0C9h, 0C9h, 0CDh, 0EDh, 0F1h, 0F2h, 0F2h, 0F6h, 0F2h 
                        db 0F6h, 0F2h, 0F1h, 0F1h, 0F1h, 0D1h, 0D1h, 0D1h, 0D1h, 0CDh, 0CDh, 0A9h, 080h, 080h, 0A8h, 0D5h 
                        db 0FDh, 0FDh, 0F9h, 0F9h, 0CDh, 0A9h, 060h, 069h, 0FFh, 0FFh, 0D1h, 004h, 000h, 089h, 0AEh, 0AEh 
                        db 08Ah, 08Ah, 08Ah, 08Ah, 08Ah, 08Ah, 08Ah, 085h, 085h, 089h, 085h, 065h, 065h, 085h, 065h, 065h 
                        db 061h, 065h, 061h, 065h, 061h, 065h, 065h, 061h, 061h, 061h, 061h, 061h, 061h, 061h, 044h, 06Dh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 049h, 0ACh, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 084h, 0A5h, 0C9h, 0A9h 
                        db 0CDh, 0CDh, 0F1h, 0F1h, 0F1h, 0F2h, 0F2h, 0CDh, 0D0h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F8h, 0F8h 
                        db 0FDh, 0D1h, 081h, 0ADh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h, 0B5h, 0FAh, 0F9h, 0F8h 
                        db 0F8h, 0F4h, 024h, 000h, 045h, 0AAh, 0AEh, 0AAh, 0AAh, 08Ah, 08Ah, 08Ah, 089h, 085h, 089h, 069h 
                        db 085h, 085h, 085h, 065h, 065h, 065h, 065h, 065h, 045h, 061h, 061h, 061h, 065h, 065h, 061h, 065h 
                        db 065h, 061h, 041h, 020h, 0ACh, 0F4h, 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h 
                        db 0F0h, 0D0h, 000h, 000h, 064h, 0A9h, 0CDh, 0CDh, 0CDh, 0D2h, 0F1h, 0F1h, 0F6h, 044h, 088h, 0F4h 
                        db 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F9h, 0D1h, 0A9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh 
                        db 0FDh, 0F9h, 0FDh, 0FDh, 0F9h, 0F9h, 0F5h, 0F8h, 0F4h, 068h, 000h, 000h, 045h, 0AEh, 0AAh, 0AAh 
                        db 0AAh, 08Ah, 08Ah, 089h, 085h, 089h, 085h, 085h, 065h, 065h, 065h, 065h, 065h, 065h, 061h, 061h 
                        db 065h, 065h, 065h, 065h, 065h, 061h, 041h, 020h, 068h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D6h, 049h, 000h, 000h, 020h, 084h, 0A9h, 0CDh 
                        db 0D1h, 0F2h, 0F6h, 045h, 088h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0D1h, 0F5h, 0F9h 
                        db 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h, 0F9h, 0F9h, 0F8h, 0F8h, 0F8h, 0F4h 
                        db 0D0h, 044h, 000h, 000h, 020h, 045h, 08Ah, 0AAh, 08Ah, 0AAh, 08Ah, 08Ah, 086h, 089h, 085h, 065h 
                        db 065h, 065h, 065h, 065h, 065h, 065h, 065h, 061h, 065h, 021h, 020h, 020h, 044h, 0ACh, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F5h, 0F5h 
                        db 0B1h, 068h, 068h, 068h, 088h, 08Ch, 0D0h, 0D1h, 0F1h, 0ACh, 0B0h, 0F4h, 0F0h, 0F4h, 0F0h, 0F4h 
                        db 0F4h, 0F4h, 0F8h, 0D5h, 0F9h, 0F9h, 0FDh, 0F9h, 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h 
                        db 0F9h, 0F9h, 0F9h, 0F9h, 0F9h, 0F8h, 0F8h, 0F8h, 0F0h, 08Ch, 044h, 000h, 000h, 000h, 000h, 020h 
                        db 020h, 045h, 045h, 065h, 065h, 065h, 065h, 045h, 045h, 041h, 021h, 020h, 000h, 000h, 000h, 024h 
                        db 088h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0FEh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h 
                        db 0D0h, 0D0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F4h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h 
                        db 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F9h, 0F9h, 0F9h, 0F9h, 0FDh 
                        db 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h, 0F9h, 0F9h, 0F9h, 0F8h, 0B0h, 0B0h, 0D4h, 0F4h 
                        db 0F4h, 0D0h, 0ACh, 088h, 068h, 044h, 024h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 020h 
                        db 020h, 068h, 068h, 08Ch, 0B0h, 0B0h, 0ACh, 0CCh, 0F4h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h 
                        db 0D0h, 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0D0h, 0D0h, 0F0h, 0D0h, 0F0h 
                        db 0D0h, 0D0h, 0F4h, 0D0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F4h, 0F0h, 0F4h, 0F8h, 0F5h 
                        db 0F4h, 0F9h, 0F9h, 0F9h, 0FDh, 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h, 0FDh, 0F9h 
                        db 08Ch, 024h, 000h, 000h, 000h, 048h, 0F4h, 0F4h, 0F4h, 0F4h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h 
                        db 0CCh, 0D0h, 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F4h, 0F4h, 0ACh, 044h, 020h, 000h, 000h, 024h, 0ACh 
                        db 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F9h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h, 0D0h, 0D0h, 0D0h, 0D0h, 0F0h, 0D0h, 0D0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h 
                        db 0F0h, 0F0h, 0F0h, 0D4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0B0h, 08Ch, 0D5h, 0FDh, 0FDh, 0FDh, 0FDh 
                        db 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 091h, 0B6h, 0FFh, 0FFh, 0DBh, 092h, 000h, 044h, 0F4h, 0F4h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0ACh, 092h 
                        db 0DBh, 0FFh, 0FFh, 0B6h, 028h, 000h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0D0h, 0F0h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h, 0F0h, 0D0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 068h, 049h 
                        db 024h, 0D5h, 0FDh, 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0B6h, 000h, 0B0h, 0F4h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h 
                        db 0D0h, 0F0h, 0F0h, 0F0h, 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 024h, 044h, 0D0h, 0D0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 049h, 0ACh, 0F0h, 0D0h, 0D0h, 0D0h, 0D0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h 
                        db 0F0h, 0D0h, 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h 
                        db 0D0h, 0F4h, 0F4h, 0B1h, 0FFh, 0FFh, 08Dh, 0F9h, 0F9h, 0FDh, 0FDh, 0FDh, 0F9h, 0FDh, 0FDh, 0FDh 
                        db 0FEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 068h, 0F4h, 0F4h, 0F4h, 0F4h, 0D0h, 0D0h 
                        db 0D0h, 0D0h, 0D0h, 0F0h, 0D0h, 0D0h, 0D0h, 0D0h, 0F0h, 0F4h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0BBh, 000h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F5h, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0B0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h 
                        db 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F4h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0D0h, 0F4h, 0FFh, 0FFh, 0DBh, 08Ch, 0F9h, 0F9h, 0F9h 
                        db 0F9h, 0F9h, 0FDh, 0F9h, 0FDh, 0FDh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 048h 
                        db 0F8h, 0F4h, 0F4h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0D0h, 0D0h, 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F5h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 024h, 0ACh, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h 
                        db 0F0h, 0D0h, 0F5h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 08Ch, 08Ch, 088h, 088h, 08Ch, 088h, 08Ch, 088h 
                        db 08Ch, 088h, 08Ch, 088h, 0CCh, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F5h, 0FFh 
                        db 0FFh, 092h, 08Ch, 0F8h, 0F8h, 0F9h, 0F9h, 0F9h, 0F9h, 0F9h, 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh 
                        db 0F9h, 0F9h, 0F9h, 0FDh, 0F9h, 0F8h, 0F8h, 0F4h, 0F4h, 0F4h, 0F4h, 0F0h, 0F4h, 0D0h, 0D0h, 0F0h 
                        db 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h 
                        db 0D0h, 0F0h, 0F0h, 0D0h, 0D0h, 0D0h, 0F0h, 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 049h, 0B0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 024h, 024h 
                        db 024h, 024h, 024h, 024h, 024h, 024h, 024h, 024h, 024h, 000h, 08Ch, 0F0h, 0D0h, 0D0h, 0D0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0FAh, 0FFh, 0FFh, 06Dh, 0ACh, 0F8h, 0F8h, 0F8h, 0F8h, 0F9h, 0F9h, 0F9h 
                        db 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h, 0F9h, 0F9h, 0F9h, 0F9h, 0F5h, 0F8h, 0F4h, 0F4h 
                        db 0F4h, 0F4h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h 
                        db 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh 
                        db 08Ch, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0FAh, 0FFh, 0FFh, 049h, 0B0h, 0F4h 
                        db 0F4h, 0F8h, 0F8h, 0F9h, 0F9h, 0F9h, 0F9h, 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h, 0FDh, 0F9h 
                        db 0F9h, 0F9h, 0F9h, 0F8h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0D0h, 0F4h, 0F0h, 0F0h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0D0h, 0D0h, 0F0h, 0D0h, 0D0h, 0D0h, 0D0h, 0D0h, 0D0h 
                        db 0D0h, 0F0h, 0D0h, 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh 
                        db 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 08Ch, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0D0h, 0D0h, 0D0h 
                        db 0FFh, 0FFh, 0FFh, 049h, 08Ch, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F9h, 0F9h, 0F9h, 0FDh, 0FDh, 0FDh 
                        db 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h, 0F9h, 0F8h, 0F8h, 0F4h, 0F4h, 0F4h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F4h, 0F4h, 0F0h, 0F0h, 0F4h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F4h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 08Ch, 0F0h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 049h, 08Ch, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h 
                        db 0F8h, 0F9h, 0F9h, 0F9h, 0D5h, 0B4h, 0B4h, 0D5h, 0D5h, 0B5h, 0B5h, 0B4h, 0B4h, 0D0h, 0B0h, 0B4h 
                        db 0B4h, 0B0h, 0B0h, 0B0h, 0F4h, 0F4h, 0D4h, 0F4h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0ACh, 08Ch 
                        db 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 0ACh, 08Ch 
                        db 0D0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0D0h, 0F0h, 0D0h 
                        db 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 06Dh, 088h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0FFh, 0FFh, 0FFh, 06Dh 
                        db 068h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F9h, 0F9h, 0FDh, 048h, 000h, 000h, 000h, 020h, 004h 
                        db 004h, 000h, 000h, 000h, 020h, 000h, 020h, 000h, 000h, 000h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0D0h 
                        db 0F0h, 0D0h, 0F0h, 08Ch, 000h, 004h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 004h, 000h 
                        db 000h, 000h, 000h, 000h, 000h, 000h, 0B6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 049h, 0ACh, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0ECh, 0D0h, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 08Ch, 0F4h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 092h, 040h, 0F4h, 0D0h, 0F4h, 0F4h, 0F4h, 0F8h, 0F4h, 0F9h, 0F9h 
                        db 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 000h 
                        db 0D4h, 0F8h, 0F4h, 0F4h, 0F4h, 0F0h, 0F4h, 0F0h, 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h 
                        db 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 08Ch, 0F4h 
                        db 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 0DBh, 000h, 0ACh, 0F0h, 0F0h 
                        db 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F9h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FEh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 068h, 0F8h, 0F4h, 0F4h, 0F4h, 0F4h, 0D4h, 0F4h, 0D0h, 0F0h 
                        db 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FAh, 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0D0h 
                        db 0D0h, 0D0h, 0F0h, 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 06Dh, 08Ch, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 049h, 068h, 0F4h, 0F0h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F9h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FEh, 0FDh, 0FDh, 0FDh, 0F9h, 0FDh, 0FDh, 0F9h, 0D4h, 0F8h, 0F4h, 0F4h 
                        db 0F4h, 0F4h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0FEh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0F4h, 0F0h 
                        db 0D0h, 0D0h, 0F0h, 0D0h, 0F4h, 0F4h, 0F4h, 0F5h, 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 049h, 0ACh, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 088h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h 
                        db 0D0h, 0F0h, 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 092h, 000h, 0D0h, 0F0h, 0F0h, 0F0h, 0F4h, 0F4h 
                        db 0F4h, 0F4h, 0F8h, 0F9h, 0F9h, 0FEh, 0FFh, 0FFh, 0FFh, 0FEh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh 
                        db 0FDh, 0FDh, 0F9h, 0F9h, 0F8h, 0F8h, 0F4h, 0F4h, 0F4h, 0F4h, 0F0h, 0D0h, 0D0h, 0F0h, 0F5h, 0FEh 
                        db 0FFh, 0FFh, 0F9h, 0F0h, 0D0h, 0D0h, 0F0h, 0F4h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0CCh, 0F0h, 0F0h, 0D4h, 0F0h, 0F0h, 0F0h 
                        db 0F0h, 0D0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh 
                        db 08Ch, 0F0h, 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 024h 
                        db 044h, 0F0h, 0F0h, 0F0h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F9h, 0F9h, 0F9h, 0FDh 
                        db 0FDh, 0FDh, 0FDh, 0F9h, 0F9h, 0FDh, 0F9h, 0FDh, 0FDh, 0D5h, 0D4h, 0F9h, 0F4h, 0F4h, 0F4h, 0F4h 
                        db 0F4h, 0D4h, 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h 
                        db 0F0h, 0F0h, 0D0h, 0FAh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0ACh 
                        db 0F0h, 0D0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 08Ch, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 000h, 064h, 0F4h, 0F0h, 0F4h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h 
                        db 0F8h, 0F8h, 0F9h, 0F9h, 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0FDh, 0F9h, 0FDh, 0D9h, 024h 
                        db 000h, 0D8h, 0F9h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F0h, 0F4h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0B6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0CCh, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0D0h, 0F0h, 0D0h 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 088h, 0F0h, 0F0h, 0F0h 
                        db 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 092h, 000h, 068h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F9h, 0F9h, 0F9h, 0FDh, 0FDh, 0FDh, 0FDh 
                        db 0FDh, 0FDh, 0FDh, 0D9h, 0DBh, 0FFh, 000h, 024h, 0D5h, 0FCh, 0F4h, 0F4h, 0F4h, 0F4h, 0F0h, 0F4h 
                        db 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0D0h, 0F0h, 0F4h, 0D0h, 08Dh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 0B0h, 0F4h, 0F0h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 06Dh, 088h, 0F0h, 0F0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h, 0D0h, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 071h, 000h, 044h, 0CCh, 0F4h, 0F4h, 0F0h, 0D4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F5h 
                        db 0F9h, 0F9h, 0FDh, 0F9h, 0FDh, 0FDh, 0FDh, 0FDh, 090h, 0B6h, 0FFh, 0FFh, 0DBh, 004h, 000h, 0B0h 
                        db 0F8h, 0F9h, 0F8h, 0F4h, 0F4h, 0F4h, 0F0h, 0D0h, 0D0h, 0D0h, 0F0h, 0F0h, 0D0h, 0F0h, 0F0h, 0F0h 
                        db 0F0h, 0ACh, 08Dh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 049h, 0ACh, 0D0h, 0D0h, 0D0h, 0D0h, 0CCh, 0B0h, 0D0h, 0D0h, 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 08Ch, 0F0h, 0D0h, 0D0h, 0D0h, 0D0h, 0D0h, 0D0h 
                        db 0D0h, 0F0h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 024h, 000h, 068h, 08Ch, 0D0h 
                        db 0F4h, 0F4h, 0F4h, 0F4h, 0F8h, 0F8h, 0F8h, 0F9h, 0F9h, 0FDh, 0F9h, 0D5h, 06Ch, 06Dh, 0DBh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0DBh, 049h, 000h, 024h, 090h, 0D0h, 0F4h, 0F4h, 0F4h, 0F4h, 0F4h, 0F0h, 0F0h 
                        db 0F0h, 0F0h, 0F0h, 0F0h, 0ACh, 08Ch, 068h, 092h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 049h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h 
                        db 000h, 0D6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 06Dh, 000h, 000h 
                        db 000h, 000h, 000h, 000h, 000h, 000h, 000h, 0D6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 071h, 024h, 000h, 000h, 024h, 048h, 088h, 06Ch, 08Ch, 08Ch, 08Ch, 08Ch, 06Ch, 024h 
                        db 044h, 049h, 0B6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 049h, 000h, 000h, 024h 
                        db 044h, 068h, 088h, 08Ch, 088h, 088h, 088h, 068h, 044h, 024h, 045h, 06Dh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 0DBh, 0DBh, 0DBh 
                        db 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0DBh, 092h, 06Dh, 06Dh, 049h, 049h 
                        db 049h, 049h, 049h, 06Dh, 092h, 0B6h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0B6h, 06Dh, 06Dh, 069h, 049h, 049h, 049h, 049h, 06Dh, 06Dh, 0B2h, 0DBh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh 
                        db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh    
if 0
                        db  000h,000h,000h,000h,000h,000h,030h,002h,000h,000h,000h,000h
                        db  000h,000h,000h,000h,000h,030h,002h,000h,000h,000h,000h,000h
                        db  000h,000h,00Ch,00Ch,00Ch,030h,002h,00Ch,00Ch,00Ch,000h,000h
                        db  000h,00Ch,00Ch,00Fh,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,000h
                        db  00Ch,00Ch,00Fh,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,070h,00Ch,00Ch
                        db  00Ch,00Fh,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,070h
                        db  00Ch,00Fh,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,070h,00Ch,070h,070h
                        db  000h,00Ch,00Ch,00Ch,00Ch,00Ch,070h,00Ch,00Ch,00Ch,070h,000h
                        db  000h,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,00Ch,070h,070h,070h,000h
                        db  000h,000h,00Ch,00Ch,00Ch,00Ch,00Ch,070h,00Ch,070h,000h,000h
                        db  000h,000h,000h,00Ch,00Ch,00Ch,070h,070h,070h,000h,000h,000h
                        db  000h,000h,000h,000h,000h,030h,030h,000h,000h,000h,000h,000h
			endif
endif
code			ends
			end strt
