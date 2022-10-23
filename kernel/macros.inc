if ?32BIT
	.386
endif

@return  macro xx
	retf xx
	endm

@loadbx macro
if ?32BIT
	mov ebx,esp
else
	mov bx,sp
endif
	endm

@loadparm macro ofs,xx
if ?32BIT
	mov xx,ss:[ebx+4+ofs]
else
	mov xx,ss:[bx+4+ofs]
endif
	endm

ENTRY struct
bSegm	db ?
wOfs	dw ?
ENTRY ends

WF_PMODE	equ 1
WF_CPU286	equ 2
WF_CPU386	equ 4
WF_CPU486	equ 8
WF_STANDARD equ 10h
WF_ENHANCED	equ 20h

@SetKernelDS	macro segm
		ifnb <segm>
			mov segm,cs:[wKernelDS]
		else
			mov ds,cs:[wKernelDS]
		endif
		endm
