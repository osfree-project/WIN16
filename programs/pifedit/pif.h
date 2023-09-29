/*' $Header:   P:/PVCS/MAX/INCLUDE/PIF.H_V   1.1   02 Jun 1997 14:37:32   BOB  $ */
/******************************************************************************
 *									      *
 * (C) Copyright 1992-93 Qualitas, Inc.  GNU General Public License version 3.		      *
 *									      *
 * PIF.H								      *
 *									      *
 * Windows .PIF structure and equates					      *
 *									      *
 ******************************************************************************/

/*

https://web.archive.org/web/20220214185118/http://www.smsoft.ru/en/pifdoc.htm
https://drdobbs.com/architecture-and-design/undocumented-corner/184409042

*/

#ifndef _INC_WINDOWS
#define STRICT			// Enable strict type checking
#define NOCOMM			// Avoid inclusion of comm driver stuff
#define OEMRESOURCE		// Include OBM_xxx
#include <windows.h>
#endif	/* #ifndef _INC_WINDOWS */

typedef struct _PIFHDR {	// Windows 2.x section taken from TopView
    BYTE	Reserved0;	// 000: Reserved
    BYTE	CheckSum;	// 001: Checksum of bytes 002 thru 170
    char	Title[30];	// 002: Program title padded with blanks
    WORD	DosMaxS;	// 020: Low DOS max /S
    WORD	DosMinS;	// 022: Low DOS min /S
    char	PgmName[63];	// 024: Program name ASCIZ // TopView uses 64 bytes
    BYTE	Flags1; 	// 063: Microsoft behavior bits // Used by TopView as part of PgmName
				//	0x01 Modifies memory (fResident)
				//	0x02 Video mode graphics /S (fGraphics)
				//	0x04 Prevent pgm switch /S (fNoSwitch)
				//	0x08 No screen exchange /S (fNoGrab)
				//	0x10 Close window on exit (fDestroy)
				//	0x40 Modify COM2 /S (fCOM2)
				//	0x80 Modify COM1 /S (fCOM1)
    BYTE	Reserved1;	// 064: Reserved
    char	StartupDir[64]; // 065: Startup directory ASCIZ
    char	CmdLineS[64];	// 0A5: Optional parameters ASCIZ /S
    BYTE	ScreenType;	// 0E5: Windows doesn't use this
				//	0x00 Reserved
				//	0x01 Reserved
				//	0x02 80x25 B/W Text mode
				//	0x03 80x25 Color Text mode
				//	0x04 320x200 Color Graphics mode
				//	0x05 320x200 B/W Graphics mode
				//	0x06 640x200 B/W Graphics mode
				//	0x07 80x25 Monochrome Text mode
				//	0x7E default DOS program
				//	0x7F default TopView program
    BYTE	ScreenPages;	// 0E6: Windows doesn't use this (always 1 for Windows)
				//	1-8 pages
    BYTE	IntVecLow;	// 0E7: Low bound of INT vectors (always 0 for Windows)
    BYTE	IntVecHigh;	// 0E8: High bound of INT vectors (always FF for Windows)
    BYTE	ScrnRows;	// 0E9: 0-127 Windows doesn't use this
    BYTE	ScrnCols;	// 0EA: 0-127 Windows doesn't use this
    BYTE	RowOffs;	// 0EB: 0-127 Windows doesn't use this
    BYTE	ColOffs;	// 0EC: 0-127 Windows doesn't use this
    WORD	SystemMem;	// 0ED: System memory in Kb (7 - text, 23 - graphics) needed for buffers
    char	SharedProg[64]; // 0EF: Name of shared program. Windows doesn't use this
    char	SharedData[64]; // 12F: Shared program data. Windows doesn't use this
    BYTE	Flags2; 	// 16F: BehavByte
				//	0x80 fFullScreen
				//	0x40 Run in foreground (TopView)
				//	0x20 Uses math co-processor (TopView)
				//	0x10 Modify keyboard /S
	BYTE	SystemFlags;	// 170: TopView behavior bits
				//	0x20 Swap interrupt vectors
				//	0x40 Uses parameters
				// 171:
} PIFHDR, * PPIFHDR, FAR * LPPIFHDR;

// NOTE!! Maximum whole file size is 03ffh for Windows 3.x and Windows NT 3.x series!!

// Sig				Size	Desc
//MICROSOFT PIFEX	0171h	The basic section, all OS
//WINDOWS 286 3.0	0006h	Windows 3.X in standard mode
//WINDOWS 386 3.0	0068h	Windows 3.X in enhanced mode, 95, 98, NT, 2000
//WINDOWS NT  3.1	008Eh	Windows NT, 2000
//WINDOWS NT  4.0	068Ch	Windows NT 4.0, 2000
//WINDOWS VMM 4.0	01ACh	Windows 95, 98, NT 4.0, 2000
//CONFIG  SYS 4.0	Variable	Windows 95, 98
//AUTOEXECBAT 4.0	Variable	Windows 95, 98

typedef struct _PIFSIG {
	char	Signature[16];	// 00: "MICROSOFT PIFEX", 0
				// 00: "WINDOWS 286 3.0", 0
				// 00: "WINDOWS 386 3.0", 0
				// 00: "WINDOWS VMM 4.0", 0
	WORD	NextOff;	// 10: Lseek offset to next signature, or -1
	WORD	DataOff;	// 12: Lseek offset to data section
	WORD	DataLen;	// 14: Length of data section
				// 16:
} PIFSIG, * PPIFSIG, FAR * LPPIFSIG;


typedef struct _PIF286 {
    WORD	XmsMax; 	// 00: XMS max
    WORD	XmsMin; 	// 02: XMS min
    BYTE	Flags1; 	// 04: Flags 1
				//     0x01 Alt-Tab reserved
				//     0x02 Alt-Esc reserved
				//     0x04 Alt-Prtsc reserved
				//     0x08 Prtsc reserved
				//     0x10 Ctrl-Esc reserved
				//     0x20 No save screen
    BYTE	Flags2; 	// 05: Flags 2
				//     0x40 Modify COM3
				//     0x80 Modify COM4
				// 06:
} PIF286, * PPIF286, FAR * LPPIF286;

typedef struct _PIF386 {
    WORD	DosMax; 	// 00: DOS max
    WORD	DosMin; 	// 02: DOS min
    WORD	ForePrio;	// 04: Foreground priority
    WORD	BackPrio;	// 06: Background priority
    WORD	EmsMax; 	// 08: EMS max
    WORD	EmsMin; 	// 0A: EMS min
    WORD	XmsMax; 	// 0C: XMS max
    WORD	XmsMin; 	// 0E: XMS min

// WinFlags, TaskFlags, and Flags are a DWORD called pifW386Flags internally
// The bit definitions are in \WIN386\INCLUDE\STATUSFL.INC

//  DWORD	StatusFlags	// 10: PfW386Flags

#define fEnableClose	0x00000001
#define fNewBackgrnd	0x00000002
#define fNewExclusive	0x00000004
#define fNewFullScr	0x00000008

#define fALTTABdis	0x00000020
#define fALTESCdis	0x00000040
#define fALTSPACEdis	0x00000080
#define fALTENTERdis	0x00000100
#define fALTPRTSCdis	0x00000200
#define fPRTSCdis	0x00000400
#define fCTRLESCdis	0x00000800
#define fPollingDetect	0x00001000
#define fNoHMA		0x00002000
#define fPifHasHotKey	0x00004000
#define fEMSLocked	0x00008000
#define fXMSLocked	0x00010000
#define fINT16Paste	0x00020000
#define fVMLocked	0x00040000

    BYTE	WinFlags;	// 10: Enhanced mode window flags
				//     0x01 Allow close when active
				//     0x02 Background
				//     0x04 Exclusive
				//     0x08 Full-screen
				//     0x20 Alt-Tab disabled
				//     0x40 Alt-Esc disabled
				//     0x80 Alt-Space disabled
    WORD	TaskFlags;	// 11: Enhanced mode task flags
				//     0x0001 Alt-Enter disabled
				//     0x0002 Alt-Prtsc disabled
				//     0x0004 Prtsc disabled
				//     0x0008 Ctrl-Esc disabled
				//     0x0010 Detect idle time on
				//     0x0020 Disable HMA use for VM
				//     0x0040 Has a Hot Key
				//     0x0080 EMS is locked
				//     0x0100 XMS is locked
				//     0x0200 Allow fast paste
				//     0x0400 Lock application memory
    BYTE	Flags;		// 13: More flags

// VidFlags and the next unknown byte are called VD_Flags2 internally and
// probably include the following two unknown bytes as well.
// These are defined in \WIN386\INCLUDE\STATUSFL.INC and VDD.INC

    BYTE	VidFlags;	// 14: Enhanced mode video flags
				//     This is passed to VDD_PIF_State in AX
				//     They are defined in VDD.INC
				//     0x01 Emulate text mode
				//     0x02 Monitor port text off
				//     0x04 Monitor port low graphics off
				//     0x08 Monitor port high graphics off
				//     0x10 Video mode text
				//     0x20 Video mode low graphics
				//     0x40 Video mode high graphics
				//     0x80 Retain video memory
    BYTE	unk2[3];	// 15:
    WORD	Hotkey; 	// 18: Scancode of user hotkey
    BYTE	HotkeyShift;	// 1A: Shift state of user hotkey
				//     0x01 Shift
				//     0x02 Shift
				//     0x04 Ctrl
				//     0x08 Alt
    BYTE	unk4;		// 1B:
    BYTE	Hotkey3;	// 1C: 0x0F if any hotkey defined
    BYTE	unk5;		// 1D:
    BYTE	HotkeyBits;	// 1E: Bits 24-31 of WM_KEYDOWN
    BYTE	unk6[9];	// 1F:
    char	CmdLine3[64];	// 28: Optional parameters ASCIZ
				// 68:
} PIF386, * PPIF386, FAR * LPPIF386;

typedef struct _PIFVMM4 {
	BYTE	unk1[88];		// 0000h 	88	00...0000h 	  	Unknown 	  	  	  	  	  	 
	char	iconfile[80];	// 0058h 	80	PIFMGR.DLL 	  	Name of the file containing an icon 	  	  	  	95 	NT 	ANSI character set, long filename without quotes, can contain a complete path, is trailed by a null symbol.
	WORD	iconnumber;		// 00A8h 	2	0000h 	  	Number of an icon 	  	  	  	95 	NT 	The first icon in a file has number 0.
	WORD	runinback;		//00AAh 	2	0002h 		0002h 	Continue to run in background mode 	  	  	  	95 	NT 	Bit mask.
														//0010h 	Not warn on exit 	  	  	  	95 	NT
														// 0020h 	Disallow Screen Saver 	  	  	  	95 	NT
	BYTE	unk2[10];		//00ACh 	10 	00...0000h 	  	Unknown 	  	  	  	  	  	 
	WORD	priority;		//00B6h 	2 	0032h 	  	Priority 	  	  	  	95 	NT 	Number from 0 up to 100, 0 - maximal priority, 100 - minimal.
	WORD	VRAMEmulation;	//00B8h 	2 	0001h 	0001h 	Video-ROM emulation 	  	  	  	95 	NT 	Bit mask.
								//0080h 	Do not dynamically allocate video memory 	  	  	  	95 	NT
								//0100h 	Full-screen mode 	  	  	  	95 	NT
	BYTE	unk3[8];		//00BAh 	8 	00...0000h 	  	Unknown 	  	  	  	  	  	 
	WORD	lines;			////00C2h 	2 	0000h 	  	Number of text lines in a window 	  	  	  	95 	  	Auto - 0, or appropriate number (25, 43 or 50).
	WORD	Flags;			//00C4h 	2 	0001h 	0001h 	Fast paste 	  	  	  	95 	NT 	Bit mask.
								//0020h 	Not use Alt+Tab 	  	  	  	95 	NT
								//0040h 	Not use Alt+Esc 	  	  	  	95 	NT
								//0080h 	Not use Alt+Space 	  	  	  	95 	NT
								//0100h 	Not use Alt+Enter 	  	  	  	95 	NT
								//0200h 	Not use Alt+PrtSc 	  	  	  	95 	NT
								//0400h 	Not use PrtSc 	  	  	  	95 	NT
								//0800h 	Not use Ctrl+Esc 	  	  	  	95 	NT
	WORD	unk4;			//00C6h 	2 	0000h 	  	Unknown 	  	  	  	  	  	 
	WORD	unk5;			//00C8h 	2 	0005h 	  	Unknown 	  	  	  	95 	NT 	 
	WORD	unk6;			//00CAh 	2 	0019h 	  	Unknown 	  	  	  	95 	NT 	 
	WORD	unk7;			//00CCh 	2 	0003h 	  	Unknown 	  	  	  	95 	NT 	 
	WORD	unk8;			//00CEh 	2 	00C8h 	  	Unknown 	  	  	  	95 	NT 	 
	WORD	unk9;			//00D0h 	2 	03E8h 	  	Unknown 	  	  	  	95 	NT 	 
	WORD	unk10;			//00D2h 	2 	0002h 	  	Unknown 	  	  	  	95 	NT 	 
	WORD	unk11;			//00D4h 	2 	000Ah 	  	Unknown 	  	  	  	95 	NT 	 
	WORD	MouseFlags;		//00D6h 	2 	0001h 	0001h 	Not use the mouse for selection 	  	  	  	95 	  	Bit mask.
								//0002h 	Exclusive use of the mouse 	  	  	  	95 	NT
	BYTE	unk12[6];		//00D8h 	6 	00...0000h 	  	Unknown 	  	  	  	  	  	 
	WORD	FontFlags;		//00DEh 	2 	001Ch 	0004h 	Use raster fonts 	  	  	  	95 	NT 	Bit mask.
								//0008h 	Use TrueType fonts 	  	  	  	95 	NT
								//0010h 	Automatically choose the font size 	  	  	  	95 	NT
								//0400h 	The current font is raster 	  	  	  	95 	NT
								//0800h 	The current font is TrueType 	  	  	  	95 	NT
	WORD	unk13;			//00E0h 	2 	0000h 	  	Unknown 	  	  	  	  	  	 
	WORD	RasterFontHSize;		////00E2h 	2 	0000h 	  	Horizontal size of the current font (only for raster fonts). 	  	  	  	95 	NT 	Auto or TrueType - 0000h, raster font - horizontal size in pixels.
	WORD	FontVSize;		//00E4h 	2 	0000h 	  	Vertical size of the current font. 	  	  	  	95 	NT 	Auto - any value, otherwise vertical size in pixels.
	WORD	FontHSize;		//00E6h 	2 	0000h 	  	Horizontal size of the current font. 	  	  	  	95 	NT 	Auto - any value, otherwise horizontal size in pixels.
	WORD	FontVSize2;		//00E8h 	2 	0000h 	  	Vertical size of the current font. 	  	  	  	95 	NT 	Auto - any value, otherwise vertical size in pixels.
	char	RasterFont[32];	//00EAh 	32 	Terminal 	  	The name of a raster font 	  	  	  	95 	NT 	Not used, trailed by a null symbol.
	char	VectorFont[32];	//010Ah 	32 	Lucida Console 	  	The name of a TrueType font 	  	  	  	95 	NT 	Not used, trailed by a null symbol.
	WORD	unk14;			//012Ah 	2 	04E3h in Windows NT/2000, 0000h in Windows 95/98 	  	Unknown 	  	  	  	  	NT 	 
	WORD	Flags2;			//012Ch 	2 	0003h 	0001h 	Unknown 	  	  	  	95 	NT 	Bit mask.
									//0002h 	Show toolbar 	  	  	  	95 	NT
	WORD	IgnoreSetting;	//012Eh 	2 	0000h 	0001h 	Not restore settings at startup 	  	  	  	95 	NT 	 
	WORD	ScreenWidth;	//0130h 	2 	0000h 	  	?? The horizontal size of the screen in symbols 	  	  	  	95 	  	After the first start of the program the value becomes equal to 80.
	WORD	ScreenHeight;	//0132h 	2 	0000h 	  	?? The vertical size of the screen in symbols 	  	  	  	95 	  	After the first start of the program the value becomes equal to 25.
	WORD	ClientWidthPx;	//0134h 	2 	0000h 	  	The horizontal size of the window client area 	  	  	  	95 	  	In pixels.
	WORD	ClientHeightPx;	//0136h 	2 	0000h 	  	The vertical size of the window client area 	  	  	  	95 	  	In pixels.
	WORD	WindowWidth;	//0138h 	2 	0000h 	  	The horizontal size of the window 	  	  	  	95 	  	In pixels.
	WORD	WindowHeight;	//013Ah 	2 	0000h 	  	The vertical size of the window 	  	  	  	95 	  	In pixels.
	WORD	unk15;			//013Ch 	2 	0016h 	  	Unknown 	  	  	  	95 	NT 	 
	WORD	LastMaximized;	//013Eh 	2 	0000h 	0002h 	At last start the window was maximized 	  	  	  	95 	  	Bit mask. If the given value is not set, the value at offset 0140h is not interpreted.
	WORD	LastState;		//0140h 	2 	0001h 	0001h 	At last start the window was of the normal size 	  	  	  	95 	  	Is not a bit mask. The value 0002h is interpreted as the maximized window.
								//0002h 	At last start the window was minimized 	  	  	  	95 	 
								//0003h 	At last start the window was maximized 	  	  	  	95 	 
	WORD	unk16;			//0142h 	2 	FFFFh 	  	Unknown 	  	  	  	95 	  	 
	WORD	unk17;			//0144h 	2 	FFFFh 	  	Unknown 	  	  	  	95 	  	 
	WORD	RightPos;		//0146h 	2 	FFFFh 	  	The right window border position in maximized window 	  	  	  	95 	  	In pixels, if the given value is less than or equal to the left border position, and the values at offsets 013Eh, 0140h specify that the window was maximized, it is considered, that the parameters of the window were not saved, and the default values are used.
	WORD	BottomPos;		//0148h 	2 	FFFFh 	  	The bottom window border position in maximized window 	  	  	  	95 	  	In pixels, if the given value is less than or equal to the top border position, and the values at offsets 013Eh, 0140h specify that the window was maximized, it is considered, that the parameters of the window were not saved, and the default values are used.
	WORD	LeftPos;		//014Ah 	2 	0000h 	  	Left window border position 	  	  	  	95 	  	In pixels.
	WORD	TopPos;			//014Ch 	2 	0000h 	  	Top window border position 	  	  	  	95 	  	In pixels.
	WORD	RightNormPos;	//014Eh 	2 	0000h 	  	The right window border position in normal window 	  	  	  	95 	  	In pixels, if the given value is less than or equal to the left border position, and the values at offsets 013Eh, 0140h specify that the window was not maximized, it is considered, that the parameters of the window were not saved, and the default values are used.
	WORD	BottonNormPos;	//0150h 	2 	0000h 	  	The bottom window border position in normal window 	  	  	  	95 	  	In pixels, if the given value is less than or equal to the top border position, and the values at offsets 013Eh, 0140h specify that the window was not maximized, it is considered, that the parameters of the window were not saved, and the default values are used.
	BYTE	unk18[4];		//0152h 	4 	00000000h 	  	Unknown 	  	  	  	  	  	 
	char	BATName[80];	//0156h 	80 	  	  	Name of the BAT file 	  	  	  	95 	NT 	OEM character set. Can contain a complete path. If the long filename contains spaces, it must be enclosed in quotes. Is trailed by a null symbol.
	WORD	MemSize;		//01A6h 	2 	0000h 	  	Memory amount for environment 	  	  	  	95 	NT 	Auto - 0, otherwise amount of memory in kilobytes, max - 4096.
	WORD	DPMIMemSize;	//01A8h 	2 	0000h 	  	Volume of memory DPMI 	  	  	  	95 	NT 	Auto - 0, otherwise amount of memory in kilobytes, max - 16384.
	WORD	unk19;			//01AAh 	2 	0001h 	  	Unknown 	  	  	  	95 	NT 	 
} PIFVMM4, * PPIFVMM4, FAR * LPPIFVMM4;

// Hey!!! According docs extra block can be in any order!!!
// But use less or more standard order.
typedef struct _PIF {
	PIFHDR	pifHdr; 	// Old-style .PIF and header
	PIFSIG	pifSigEX;	// "MICROSOFT PIFEX" signature
	PIFSIG	pifSig286;	// "WINDOWS 286 3.0" signature
	PIF286	pif286;		// Standard mode data
	PIFSIG	pifSig386;	// "WINDOWS 386 3.0" signature
	PIF386	pif386;		// Enhanced mode data
//	PIFSIG	pifSigVMM3;	// "WINDOWS VMM 4.0" signature
//	PIFVMM4	pifVMM4; 	// Windows 95 mode data
} PIF, * PPIF, FAR * LPPIF;

