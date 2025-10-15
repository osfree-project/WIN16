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
; @todo autmaticaly detect video?
;
; DOS 7+ Boot Logo API:
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
			include bmp.inc

code		segment
			org	0h

strt:
			db	"LOGO"
			jmp	near ptr Init
;			jmp	near ptr Done
;SwitchToTextMsg     db 'Switching to text. Press a key.', 13, 10,'$'
Done:		; Check is Boot Logo API active
			mov		ax, 4A32h
			mov		bl, 0
			int		2fh
			cmp		ax, 0
			je		dologo
			cmp		ax, 0ffffh
			je		dologo

			; No Boot Logo API, use BIOS
            ;@DispStr offset SwitchToTextMsg

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
ErrorFormatMsg	db 'Incorrect LOGO.BMP format', 13, 10,'$'
SwitchToGraphMsg	db 'Switching to graphics. Press a key.', 13, 10,'$'
filename		db 'logo.bmp',0
filehandle		dw ?
FileHeader		BITMAPFILEHEADER <> ;db 14 dup (0)
InfoHeader      BITMAPINFOHEADER <> ;db 40 dup (0)
Palette			db 256*4 dup (0)
ScrLine			db 320 dup (0)

Init:
			; Check is boot logo active
			mov		ax, 4A32h
			mov		bl, 0
			int		2fh
			cmp		ax, 0		; Is logo turned on?
			je		exit		; do nothing
			cmp		ax, 0ffffH	; is logo turned off?
			je		turnon		; turn it on

			; No Boot logo API
			push	ds          ; Save DS
			push	cs          ; Set DS=CS
			pop	ds
			@GetMode            ; Get current video mode
			mov	[CurrentVideoMode], al

            ; @todo Try to find bitmap in memory first, it no, then try to load from file
			; Open file
			@OpenFil offset filename, 0
			jc openerror
			mov [filehandle], ax

			; Read BMP file header, 14 bytes
			@Read	FileHeader, size BITMAPFILEHEADER, [filehandle]
            cmp	word ptr FileHeader.bfType, 'MB'
            jnz bmpformaterror

			; Read BMP info header, 40 bytes
            ;
			@Read	InfoHeader, size BITMAPINFOHEADER, [filehandle]

            ; @todo Seek to correct position using bfOffBits
			; Read BMP file color palette, 256 colors * 4 bytes (400h)
			@Read	Palette, 400h, [filehandle]

            ; Switch to graphics
            @DispStr offset SwitchToGraphMsg
            @GetKey 0,0,1
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

			cld			; Clear direction flag, for movsb

			mov cx,320
			mov si,offset ScrLine
			rep movsb 

			pop cx
			loop PrintBMPLoop

			@ClosFil [filehandle]
			jmp	exit

bmpformaterror:
			@DispStr offset ErrorFormatMsg
            jmp exit

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


			if 0
;*********************************************************************;
;*                            V I O S P A                            *;
;*-------------------------------------------------------------------*;
;*    Task           : Creates a function for determining the type   *;
;*                     of video card installed on a system. This     *;
;*                     routine must be assembled into an OBJ file,   *;
;*                     then linked to a Turbo Pascal program.        *;
;*-------------------------------------------------------------------*;
;*    Author         : Michael Tischer                               *;
;*    Developed on   : 10/02/88                                      *;
;*    Last update    : 02/18/92                                      *;
;*-------------------------------------------------------------------*;
;*    Assembly       : MASM VIOSPA;                                  *;
;*                     ... Link to a Turbo Pascal program            *;
;*                         using the {$L VIOSPA} compiler directive  *;
;*********************************************************************;
                                                                        
;== Constants for the VIOS structure ==================================
                                                                        
                                  ;Video card constants
NO_VIOS    = 0                    ;No video card/unrecognized card
VGA        = 1                    ;VGA card
EGA        = 2                    ;EGA card
MDA        = 3                    ;Monochrome Display Adapter
HGC        = 4                    ;Hercules Graphics Card
CGA        = 5                    ;Color Graphics Adapter
                                                                        
                                  ;Monitor constants
NO_MON     = 0                    ;No monitor/unrecognized code
MONO       = 1                    ;Monochrome monitor
COLOR      = 2                    ;Color Monitor
EGA_HIRES  = 3                    ;High-resolution/multisync monitor
ANLG_MONO  = 4                    ;Monochrome analog monitor
ANLG_COLOR = 5                    ;Analog color monitor
                                                                        
;== Data segment ======================================================
                                                                        
DATA   segment word public        ;Turbo data segment
                                                                        
DATA   ends
                                                                        
;== Code segment ======================================================
                                                                        
CODE       segment byte public    ;Turbo code segment
                                                                        
           assume cs:CODE, ds:DATA
                                                                        
public     getvios
                                                                        
;-- Initialized global variables must be placed in the code segment ---
                                                                        
vios_tab   equ this word
                                                                        
           ;-- Conversion table for supplying return values of VGA  ---
           ;-- BIOS function 1AH, sub-function 00H              ---
                                                                        
           db NO_VIOS, NO_MON     ;No video card
           db MDA    , MONO       ;MDA card/monochrome monitor
           db CGA    , COLOR      ;CGA card/color monitor
           db ?      , ?          ;Code 3 unused
           db EGA    , EGA_HIRES  ;EGA card/hi-res monitor
           db EGA    , MONO       ;EGA card/monochrome monitor
           db ?      , ?          ;Code 6 unused
           db VGA    , ANLG_MONO  ;VGA card/analog mono monitor
           db VGA    , ANLG_COLOR ;VGA card/analog color monitor
                                                                        
ega_dips   equ this byte
                                                                        
           ;-- Conversion table for EGA card DIP switches -----
                                                                        
           db COLOR, EGA_HIRES, MONO
           db COLOR, EGA_HIRES, MONO
                                                                        
;----------------------------------------------------------------------
;-- GETVIOS: Determines type(s) of installed video card(s) ------------
;-- Pascal call : GetVios ( vp : ViosPtr ); external;
;-- Declaration : Type Vios = record VCard, Monitor: byte;
;-- Return Value: None
                                                                        
getvios  proc near
                                                                        
sframe     struc                  ;Stack access structure
cga_possi  db ?                   ;Local variables
ega_possi  db ?                   ;Local variables
mono_possi db ?                   ;Local variables
bptr       dw ?                   ;BPTR
ret_adr    dw ?                   ;Return address of calling program
vp         dd ?                   ;Pointer to first VIOS structure
sframe     ends                   ;End of structure
                                                                        
frame      equ [ bp - cga_possi ] ;Address elements of structure
                                                                        
           push bp                ;Push BP onto stack
           sub  sp,3              ;Allocate memory for local variables
           mov  bp,sp             ;Move SP to BP
                                                                        
           mov  frame.cga_possi,1 ;Is it a CGA?
           mov  frame.ega_possi,1 ;Is it an EGA?
           mov  frame.mono_possi,1;Is it an MDA or HGC?
                                                                        
           mov  di,word ptr frame.vp    ;Get offset addr. of structure
           mov  word ptr [di],NO_VIOS   ;No video system or unknown
           mov  word ptr [di+2],NO_VIOS ;system found
                                                                        
           call test_vga          ;Test for VGA card
           cmp  frame.ega_possi,0 ;Or is it an EGA card?
           je   gv1               ;No --> Go to CGA test
                                                                        
           call test_ega          ;Test for EGA card
gv1:       cmp  frame.cga_possi,0 ;Or is it a CGA card?
           je   gv2               ;No --> Go to MDA/HGC test
                                                                        
           call test_cga          ;Test for CGA card
gv2:       cmp  frame.mono_possi,0;Or is it an MDA or HGC card?
           je   gv3               ;No --> End tests
                                                                        
           call test_mono         ;Test for MDA/HGC card
                                                                        
           ;-- Determine video configuration --------------------------
                                                                        
gv3:       cmp  byte ptr [di],VGA ;VGA card?
           je   gvi_end           ;Yes --> Active card indicated
           cmp  byte ptr [di+2],VGA;VGA card part of secondary system?
           je   gvi_end           ;Yes --> Active card indicated
                                                                        
           mov  ah,0Fh            ;Determine video mode using
           int  10h               ;BIOS video interrupt
                                                                        
           and  al,7              ;Only modes 0-7 are of interest
           cmp  al,7              ;Mono card active?
           jne  gv4               ;No --> CGA or EGA mode
                                                                        
           ;-- MDA, HGC or EGA card (mono) currently active -----------
                                                                        
           cmp  byte ptr [di+1],MONO ;Mono monitor in first structure?
           je   gvi_end           ;Yes --> Sequence O.K.
           jmp  short switch      ;No --> Switch sequence
                                                                        
           ;-- CGA or EGA card currently active -----------------------
                                                                        
gv4:       cmp  byte ptr [di+1],MONO ;Mono monitor in first structure?
           jne  gvi_end           ;No -->Sequence O.K.
                                                                        
switch:    mov  ax,[di]           ;Get contents of first structure
           xchg ax,[di+2]         ;Switch with second structure
           mov  [di],ax
                                                                        
gvi_end:   add  sp,3              ;Add local variables from stack
           pop  bp                ;Pop BP off of stack
           ret  4                 ;Clear variables off of stack;
                                  ;Return to Turbo
getvios    endp
                                                                        
;----------------------------------------------------------------------
;-- TEST_VGA: Determines whether a VGA card is installed
                                                                        
test_vga   proc near
                                                                        
           mov  ax,1a00h          ;Function 1AH, sub-function 00H
           int  10h               ;Call VGA-BIOS
           cmp  al,1ah            ;Function supported?
           jne  tvga_end          ;No --> End routine
                                                                        
           ;-- If function is supported, BL contains the code of the --
           ;-- active video system, while BH contains the code of    --
           ;-- the inactive video system                             --
                                                                        
           mov  cx,bx             ;Move result to CX
           xor  bh,bh             ;Set BH to 0
           or   ch,ch             ;Only one video system?
           je   tvga_1            ;Yes --> Display first system's code
                                                                        
           ;-- Convert code of second system --------------------------
                                                                        
           mov  bl,ch             ;Move second system's code to BL
           add  bl,bl             ;Add offset to table
           mov  ax,vios_tab[bx]   ;Get code from table and move into
           mov  [di+2],ax         ;caller's structure
           mov  bl,cl             ;Move first system's code into BL
                                                                        
           ;-- Convert code of second system --------------------------
                                                                        
tvga_1:    add  bl,bl             ;Add offset to table
           mov  ax,vios_tab[bx]   ;Get code from table
           mov  [di],ax           ;and move into caller's structure
                                                                        
           mov  frame.cga_possi,0 ;CGA test fail?
           mov  frame.ega_possi,0 ;EGA test fail?
           mov  frame.mono_possi,0 ;Test for mono
                                                                        
           mov  bx,di             ;Address of active structure
           cmp  byte ptr [bx],MDA ;Monochrome system online?
           je   do_tmono          ;Yes --> Execute MDA/HGC test
                                                                        
           add  bx,2              ;Address of inactive structure
           cmp  byte ptr [bx],MDA ;Monochrome system online?
           jne  tvga_end          ;No --> End routine
                                                                        
do_tmono:  mov  word ptr [bx],0   ;Emulate if this system
                                  ;isn't available
           mov  frame.mono_possi,1;Execute monochrome test
                                                                        
tvga_end:  ret                    ;Return to caller
                                                                        
test_vga   endp
                                                                        
;----------------------------------------------------------------------
;-- TEST_EGA: Determine whether an EGA card is installed
                                                                        
test_ega   proc near
                                                                        
           mov  ah,12h            ;Function 12H
           mov  bl,10h            ;Sub-function 10H
           int  10h               ;Call EGA-BIOS
           cmp  bl,10h            ;Is this function supported?
           je   tega_end          ;No --> End routine
                                                                        
           ;-- If the function IS supported, CL contains the        ---
           ;-- EGA card DIP switch settings                         ---
                                                                        
           mov  bl,cl             ;Move DIP switches to BL
           shr  bl,1              ;Shift one position to the right
           xor  bh,bh             ;Index high byte to 0
           mov  ah,ega_dips[bx]   ;Get element from table
           mov  al,EGA            ;Is it an EGA card?
           call found_it          ;Transfer data to the vector
                                                                        
           cmp  ah,MONO           ;Mono monitor connected?
           je   is_mono           ;Yes --> Not MDA or HGC
                                                                        
           mov  frame.cga_possi,0 ;No CGA card possible
           jmp  short tega_end    ;End routine
                                                                        
is_mono:   mov  frame.mono_possi,0;EGA can either emulate MDA or HGC,
                                  ;if mono monitor is attached
                                                                        
tega_end:  ret                    ;Back to caller
                                                                        
test_ega   endp
                                                                        
;----------------------------------------------------------------------
;-- TEST_CGA: Determines whether a CGA card is installed
                                                                        
test_cga   proc near
                                                                        
           mov  dx,3D4h           ;Port addr. of CGA's CRTC addr. reg.
           call test_6845         ;Test for installed 6845 CRTC
           jc   tega_end          ;No --> End test
                                                                        
           mov  al,CGA            ;Yes --> CGA installed
           mov  ah,COLOR          ;CGA uses color monitor
           jmp  found_it          ;Transfer data to vector
                                                                        
test_cga   endp
                                                                        
;----------------------------------------------------------------------
;-- TEST_MONO: Checks for MDA or HGC card
                                                                        
test_mono  proc near
                                                                        
           mov  dx,3B4h           ;Port addr. of MONO's CRTC addr. reg.
           call test_6845         ;Test for installed 6845 CRTC
           jc   tega_end          ;No --> End test
                                                                        
           ;-- Monochrome video card installed ------------------------
           ;-- 
           mov  dl,0BAh           ;MONO status port at 3BAH
           in   al,dx             ;Read status port
           and  al,80h            ;Separate bit 7 and
           mov  ah,al             ;move to AH
                                                                        
           ;-- If the contents of bit 7 in the status port change   ---
           ;-- during the following readings, it is handled as an   ---
           ;-- HGC                                                  ---
                                                                        
           mov  cx,8000h          ;Maximum 32768 loop executions
test_hgc:  in   al,dx             ;Read status port
           and  al,80h            ;Isolate bit 7
           cmp  al,ah             ;Contents changed?
           jne  is_hgc            ;Bit 7 = 1 --> HGC
           loop test_hgc          ;Continue
                                                                        
           mov  al,MDA            ;Bit 7 <> 1 --> MDA
           jmp  set_mono          ;Set parameters
                                                                        
is_hgc:    mov  al,HGC            ;Bit 7 = 1 --> HGC
set_mono:  mov  ah,MONO           ;MDA and HGC set as mono screen
           jmp  found_it          ;Set parameters
                                                                        
test_mono  endp
                                                                        
;----------------------------------------------------------------------
;-- TEST_6845: Returns set carry flag if 6845 doesn't lie in the
;--            port address in DX
                                                                        
test_6845  proc near
                                                                        
           mov  al,0Ah            ;Register 0AH
           out  dx,al             ;Register number in CRTC address reg.
           inc  dx                ;DX now in CRTC data register
                                                                        
           in   al,dx             ;Get contents of register 0AH
           mov  ah,al             ;and move to AH
                                                                        
           mov  al,4Fh            ;Any value
           out  dx,al             ;Write to register 0AH
                                                                        
           mov  cx,100            ;Short wait loop to which
waitforit: loop waitforit         ;6845 can react
                                                                        
           in   al,dx             ;Read contents of register 0AH
           xchg al,ah             ;Exchange Ah and AL
           out  dx,al             ;Send value
                                                                        
           cmp  ah,4Fh            ;Written value been read?
           je   t6845_end         ;Yes --> End test
                                                                        
           stc                    ;No --> Set carry flag
                                                                        
t6845_end: ret                    ;Back to caller
                                                                        
test_6845  endp
                                                                        
;----------------------------------------------------------------------
;-- FOUND_IT: Transfers type of video card to AL and type of       ----
;--           monitor in AH in the video vector                    ----
                                                                        
found_it   proc near
                                                                        
           mov bx,di              ;Address of active structure
           cmp word ptr [bx],0    ;Video system already  onboard?
           je  set_data           ;No --> Data in  active structure
                                                                        
           add bx,2               ;Yes --> Inactive structure address
                                                                        
set_data:  mov [bx],ax            ;Place data in structure
           ret                    ;Back to caller
                                                                        
found_it   endp
                                                                        
;----------------------------------------------------------------------
                                                                        
code       ends                   ;End of code segment
           end                    ;End of program

				endif