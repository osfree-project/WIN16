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
;                             Interrupt 2Fh - Multiplex DOS-LOGO Support 	
;				Input					Output
;Subservice 	Name 	Description 	Register 	Value 	Description
;AX=4A32h 	BL 	00h 	DOS-LOGO Is Session & Get Logo Checksum 	Check to see if the boot logo is active. Since the boot logo is invisible to programs and device drivers, this is the real way of knowing if the boot logo has been activated.
;						The checksum is used internally by the operating system to see if the logo pixel data has been untouched. 	AX 	0000h 	Logo is in session (turned on).
;													FFFFh 	Logo is not in session (turned off).
;										DX 	**** 	Checksum of the logo's pixels in video ram (A000h:8000h) from when it was first drawn. Compare this checksum against the current video ram's checksum to see if the logo can be put back into session once more.
;				01h 	DOS-LOGO Resume Session 	DOS-LOGO allows the operating system and device drivers to have priority access to the interrupt table when the boot logo is in session. After the DOS-LOGO session has been paused, use this subservice to resume it.
;
;						Note: A paused DOS-LOGO session cannot restore the video mode. 	N/A
;				02h 	DOS-LOGO Pause Session 	The operating system or other device drivers might want the priority to the interrupt table when the boot logo is in session. Pausing the boot logo will not change the video mode, but will allow access to unmodified DOS-LOGO resources. Use subservice 01h (DOS-LOGO ResumeSession) to actiavate the boot logo after it has been paused. 	N/A
;				03h 	DOS-LOGO Restore Previous Video Mode 	A DOS-LOGO session will automatically restore the previous video mode and its text if the user presses the keyboard key Escape, or if the current running program requests user input.
;						But in order for a program or device driver to restore the video mode in other ways, calling this subservice is the correct way to do it.
;
;						Note: Changing video mode from a program/device driver during a DOS-LOGO session will not restore the previous video mode and its text. 	N/A
;				04h 	DOS-LOGO Turn On Session 	The operating system calls this subservice to start the DOS-LOGO session, meaning that the boot logo will be animated and can be aborted by the user pressing the keyboard key Escape. The return value is the previous animation rotation info from when the boot logo was last in session.
;
;						Note: This subservice is used in complex redisplay boot logo operations by the operating system. It is only supported for backward compatibility.
;						Use subservice 06h (DOS-LOGO Redisplay and Enter Session). 	AX 	**** 	The animation rotation info from the previous session.
;				05h 	DOS-LOGO Turn Off Session 	Turns off the DOS-LOGO session. 	N/A
;				06h 	DOS-LOGO Redisplay and Enter Session 	This subservice is new for the DOS-LOGO device driver. It does the hard work of redisplaying the boot logo once it has been aborted by a program or by the user. In order for a boot logo to be redisplayed, the video ram must have been untouched since the last time the boot logo was displayed. A graphics mode switch without the 7th bit turned on will cause the video buffer to be erased, thus destroying the logo's loaded pixel art.
;
;						DOS-LOGO Redisplay and Enter Session does all of the work from the subservices above. Therefore use this subservice in first hand. When it comes to redisplaying the boot logo, the above subservices are left for backward compatibility only. 	N/A
;
;

.8086
			; MacroLib
			include	bios.inc
			include	dos.inc

code		segment
			org	0h

strt:
			db	"LOGO"
			jmp	near ptr Init
;			jmp	near ptr Done
Done:		; Check is Boot Logo API active
			mov		ax, 4A32h
			mov		bl, 0
			int		2fh
			cmp		ax, 0
			je		dologo
			cmp		ax, 0ffffh
			je		dologo

			; No Boot Logo API, use BIOS
			@SetMode [CurrentVideoMode]
			retf

dologo:		; Restore text move via Boot Logo API
			mov		ax, 4A32h
			mov		bl, 0
			int		2fh
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

Init:		; Check is boot logo active
			mov		ax, 4A32h
			mov		bl, 0
			int		2fh
			cmp		ax, 0		; Is logo turned on?
			je		exit		; do nothing
			cmp		ax, 0ffffH	; is logo turned off?
			je		turnon		; turn it on

			; No Boot logo API
			push	ds
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

turnon:
			; Turn on logo and exit
			mov		ax, 4A32h
			mov		bl, 4
			int		2fh

			jmp exit
			
code		ends
			end strt
