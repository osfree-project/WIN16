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

ErrorMsg		db 'Error loading LOGO.BMP', 13, 10,'$'
filename		db 'logo.bmp',0
filehandle		dw ?
Header			db 54 dup (0)
Palette			db 256*4 dup (0)
ScrLine			db 320 dup (0)

Init:			push	ds
			push	cs
			pop	ds
			@GetMode
			mov	[CurrentVideoMode], al

			; Open file
			@OpenFil offset filename, 0
			jc openerror
			mov [filehandle], ax

			; Read BMP file header, 54 bytes
			@Read	Header, 54, [filehandle]
			; Read BMP file color palette, 256 colors * 4 bytes (400h)
			@Read	Palette, 400h, [filehandle]
			@SetMode 13h		; 13h 40x25 320x200 256 colors VGA

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

			@Read	ScrLine, 320, [filehandle]

			; Copy one line into video memory

			cld 

			; Clear direction flag, for movsb

			mov cx,320
			mov si,offset ScrLine
			rep movsb 

			pop cx
			loop PrintBMPLoop

			@ClosFil [filehandle]
			jmp	exit

			openerror:
			@DispStr offset ErrorMsg

exit:
			lea	ax, DeadSpace
			pop	ds
			retf

code			ends
			end strt
