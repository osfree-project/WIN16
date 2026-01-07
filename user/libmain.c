#include "user.h"

int UT_GetIntFromProfile(UINT id, int defvalue)
{
	char section[30];
	char key[30];
	char value[30];

	FUNCTION_START

	if (!LoadString(USER_HeapSel, IDS_WINDOWS, section, sizeof(section)))
	{
		TRACE("error1");
		for(;;);
	}
	if (!LoadString(USER_HeapSel, id, key, sizeof(key)))
	{
		TRACE("error2");
		for(;;);
	}

	// Protect from empty value
	if (!GetProfileString(section, key, "", value, sizeof(value)))
	{
		FUNCTION_END
		return defvalue;
	}

	FUNCTION_END
	return GetProfileInt(section, key, defvalue);
}

VOID CreateQueue(int QueueSize)
{
	LPQUEUE lpQueue;
	int size;

	FUNCTION_START

	size=sizeof(QUEUE)+(QueueSize-1)*sizeof(INTERNALMSG);

	lpQueue=(LPQUEUE)GlobalAlloc(GPTR+GMEM_SHARE, size);
	lpQueue->OwningTask=GetCurrentTask();
	lpQueue->MessageSize=sizeof(INTERNALMSG);
	lpQueue->Size=QueueSize;
	//lpQueue->ReadPtr=???;
	//lpQueue->WritePtr=???;
	lpQueue->ExpWinVersion=GetExeVersion();
	lpQueue->WakeBits=QS_SMPARAMSFREE;
//	lpQueue->flags=QF_INIT;

	SetTaskQueue(0, (HGLOBAL)lpQueue);

	FUNCTION_END
}

VOID WINAPI LW_LoadSomeStrings()
{
	FUNCTION_START
	LoadString(USER_HeapSel, IDS_SYSTEMERROR, szSysError, sizeof(szSysError));
	LoadString(USER_HeapSel, IDS_DIVIDEBYZERO, szDivZero, sizeof(szDivZero));
	LoadString(USER_HeapSel, IDS_UNTITLED, szUntitled, sizeof(szUntitled));
	LoadString(USER_HeapSel, IDS_ERROR, szError, sizeof(szError));
	LoadString(USER_HeapSel, IDS_OK, szOk, sizeof(szOk));
	LoadString(USER_HeapSel, IDS_CANCEL, szCancel, sizeof(szCancel));
	LoadString(USER_HeapSel, IDS_ABORT, szAbort, sizeof(szAbort));
	LoadString(USER_HeapSel, IDS_RETRY, szRetry, sizeof(szRetry));
	LoadString(USER_HeapSel, IDS_IGNORE, szIgnore, sizeof(szIgnore));
	LoadString(USER_HeapSel, IDS_YES, szYes, sizeof(szYes));
	LoadString(USER_HeapSel, IDS_NO, szNo, sizeof(szNo));
	// LoadString(USER_HeapSel, 0x4b, szClose, sizeof(szClose)); not found in user.exe resources
	LoadString(USER_HeapSel, IDS_AM, szAm, sizeof(szAm));
	LoadString(USER_HeapSel, IDS_PM, szPm, sizeof(szPm));
	FUNCTION_END
}

VOID WINAPI RW_RegisterMenus()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterButton()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterStatic()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterDlg()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterEdit()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterLBoxCtl()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterSB()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterComboLBoxCtl()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterCBoxCtl()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisteMDIClient()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI LW_RegisterWindows(HINSTANCE USER_HeapSel)
{
	FUNCTION_START

	RW_RegisterMenus();
	RW_RegisterButton();
	RW_RegisterStatic();
	RW_RegisterDlg();
	RW_RegisterEdit();
	RW_RegisterLBoxCtl();
	RW_RegisterSB();
	RW_RegisterComboLBoxCtl();
	RW_RegisterCBoxCtl();
	RW_RegisteMDIClient();

	FUNCTION_END
}

// Check Windows Internals for some info
VOID WINAPI LW_DriversInit()
{
	FUNCTION_START

	TRACE("Keyboard init");
	if (InquireKeyboard(&KbInfo)==sizeof(KBINFO))
	{
		TRACE("Keyboard initdone");
		if (SetSpeed(-1)==-1) FatalExit(0x0c);
	} else 
		FatalExit(0x0c);

	TRACE("Mouse init");
	if (InquireMouse(&MouseInfo))
	{
		TRACE("Mouse initdone");
	} else 
		FatalExit(0x0c);;

	TRACE("Display init");
	if (InquireDisplay(&CursorInfo)==sizeof(CURSORINFO))
	{
		TRACE("Display initdone");
	};

	FUNCTION_END
}

VOID WINAPI LW_DCInit()
{
	FUNCTION_START

// Here we must create 5 DC and store in DCE in local user heap.. We need to implement cache DC here...
//	PDCEFirst=LocalLock(LocalAlloc(...));
//	TRACE("Create display context");
//	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);
//	TRACE("Create display context");
//	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);
//	TRACE("Create display context");
//	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);
//	TRACE("Create display context");
//	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);
//	TRACE("Create display context");
//	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);

	FUNCTION_END
}

VOID WINAPI LW_BrushInit()
{
	FUNCTION_START

	FUNCTION_END
}

// See Windows Internals
VOID WINAPI LW_LoadResources()
{
	char szWindows[0x14];
	char szBeep[0x14];
	char szSwapMouseButtons[0x14];

	FUNCTION_START

	// Get some strings out of the USER resources, for use as
	// section and key strings in GetProfilelnt() calLs
	LoadString(USER_HeapSel, IDS_WINDOWS, szWindows, sizeof(szWindows));
	LoadString(USER_HeapSel, IDS_SWAPMOUSEBUTTONS, szSwapMouseButtons, sizeof(szSwapMouseButtons));
	LoadString(USER_HeapSel, IDS_BEEP, szBeep, sizeof(szBeep));

	SetCaretBlinkTime(UT_GetIntFromProfile(IDS_CURSORBLINKRATE, 0x1F4)); // 4 = "CursorBlinkRate"

	SetDoubleClickTime(UT_GetIntFromProfile(IDS_DOUBLECLICKSPEED, 0));

	// The next three fetched vaLues are stored in static vars
	UT_GetIntFromProfile(IDS_DOUBLECLICKWIDTH, 4); // 0x61 = "DoubleClickWidth"
	UT_GetIntFromProfile(IDS_DOUBLECLICKHEIGHT, 4); // 0x62 = "DoubleClickHeight"
	UT_GetIntFromProfile(IDS_MENUDROPALIGNMENT, 0); // 0x62 = "MenuDropAlignment"


	// Get the delay times related to displaying menus.
	// 0x5E = "MenuShowDelay", 0x5F = "MenuHideDelay". The
	// defauLt vaLue is seemingLy reLated to whether the
	// WF_CPU286 fLag is set. Perhaps there's some sort of
	// timing issue?
	IDelayMenuShow = UT_GetIntFromProfile(IDS_MENUSHOWDELAY, defaultVal);
	IDelayMenuHide = UT_GetIntFromProfile(IDS_MENUHIDEDELAY, defaultVal);

	// Set the vaLue of FSwapButtons, which indicates if the Left
	// and right mouse buttons should be swapped
	GetProfileString(szWindows, szSwapMouseButtons,
			szNullString, &szSwapMouseButtons, 2);

	//FSwapButtons = some calculation involving szYes, the buffer
	//filled by GetProfiLeString(), and a call to AnsiLower().

	// Set the value of FBeep. Used by MessageBeep() to
	// determine if a sound shouLd be produced
	GetProfileString( szWindows, szBeep, szNullString,
		&szSwapMouseButtons, 2 );

	//FBeep = some calculation involving szYes, the buffer
	//filled by GetProfiLeString(), and a call to AnsiLower().

	// 0x6B = "DragFullWindows". Drag around the entire window
	// contents, rather than just the frame.
	FDragFullWindows = UT_GetIntFromProfile(IDS_DRAGFULLWINDOWS, 0);

	// 0x6F = "CoolSwitch". This has to do with fast ALT-TAB
	// switching between different tasks.
	FFastAltTab = UT_GetIntFromProfile(IDS_COOLSWITCH, 1);

	// Get the Grid granularity of the desktop. 0x50 = "Desktop"
	// 8 = "GridGranularity". It appears that szBeep[] and
	// szWindows[] are being reused, rather than creating
	// additionaL char arrays on the stack.
	LoadString(USER_HeapSel, IDS_DESKTOP, szWindows, sizeof(szWindows));
	LoadString(USER_HeapSel, IDS_GRIDGRANULARITY, szBeep, sizeof(szBeep));

	CXYGranularity = GetProfileInt(szWindows, szBeep, 0) << 3;
	if ( !CXYGranularity) // Make sure CXYGranularity is at
		CXYGranularity++; // least 1.

	// Load some heavily used cursors
	HCursNormal = LoadCursor(0, IDC_ARROW);
	HCursIBeam = LoadCursor(0, IDC_IBEAM);
	HCursUpArrow= LoadCursor(0, IDC_UPARROW);
	HIconSample = LoadIcon(0, IDI_APPLICATION);

	// Set the "resource handler" address for cursors & icons.
	// @todo Not clear... Befor was non-DIB cursors???
//	SetResourceHandler(USER_HeapSel, 1, LoadDIBCursorHandler);
//	SetResourceHandler(USER_HeapSel, 3, LoadDIBIconHandler);

	// The Windows Logo
	HIconWindows = LoadIcon(USER_HeapSel, MAKEINTRESOURCE(OCR_ICOCUR));


	// Load the "resizing" cursors (when cursor is over a border)
	HCursSizeNWSE=LoadCursor(0, IDC_SIZENWSE);
	HCursSizeNESW=LoadCursor(0, IDC_SIZENESW);
	HCursSizeNS=LoadCursor(0,IDC_SIZENS);
	HCursSizeWE=LoadCursor(0,IDC_SIZEWE);

	// Load some icons used by various diaLog boxes
	HIconHand = LoadIcon(0, MAKEINTRESOURCE(OIC_HAND));
	HIconQues = LoadIcon(0, MAKEINTRESOURCE(OIC_QUES));
	HIconBang = LoadIcon(0, MAKEINTRESOURCE(OIC_BANG));
	HIconNote = LoadIcon(0, MAKEINTRESOURCE(OIC_NOTE));

	// The "4 directions" icon
	HCursSizeAll = LoadCursor(USER_HeapSel, MAKEINTRESOURCE(OCR_SIZEALL));

	FUNCTION_END
}

VOID WINAPI LW_OEMDependentInit()
{
	FUNCTION_START
	HInstanceDisplay=GetModuleHandle(DISPLAY);
	FUNCTION_END
}

VOID WINAPI LW_OEMCursorInit()
{
	FUNCTION_START
	FUNCTION_END
}

VOID WINAPI InitSizeBorderDimensions()
{
	FUNCTION_START
	FUNCTION_END
}

VOID LW_InitSysMetrics()
{
	FUNCTION_START
	FUNCTION_END
}

VOID WINAPI LW_MouseInit()
{
	FUNCTION_START
	FUNCTION_END
}

VOID WINAPI EnableInput()
{
	HMODULE HModSound;
	FARPROC lpfnSoundEnable;

	FUNCTION_START

	// See "Undocumented Windows"
	EnableSystemTimers();

	//Initialize some memory starting at RGBKeyState to 0. Maybe
	//some sort of an array. Do the same for RGBAsyncKeyState.

	// KEYBOARD.2. See the
	// DDK examples for
	// source code for the
	// Enable() routines.
	KeyboardEnable(keybd_event, RGBKeyState);

	//CopyKeyState(); // copies keyboard state tables?

//	if ( some static variabLe)
		MouseEnable(mouse_event); // MOUSE.2

	// Look for the presence of a SOUND driver. If found, get
	// the address of its enabLe() function, and calL it.
	HModSound = GetModuleHandle("SOUND");
	if ( HModSound )
		lpfnSoundEnable = GetProcAddress(HModSound, "enable");

	if ( lpfnSoundEnable )
		lpfnSoundEnable();

	// Call WNetEnable() to initialize the network module,
	// if a network is present. See FarCallNetDriver() entry
	// in "Undocumented Windows," and "Windows Network
	// Programming" by Ralph Davis.

//	if (PNetlnfo && *(&PNetlnfo + 0x50) )
//		(&PNetlnfo + 0x50)(); // Call through a function pointer
			
	// Broadcast a message to the installable device drivers?
	// 2 = DRV_ENABLE??
//	InternalBroadcastDriverMessage(0, 2, 0, 0, 0, 0, 0, 4);

	FUNCTION_END
}

// @todo Move to Desktop.c
LRESULT WINAPI DeskTopWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
//    LOGSTR((LF_API,"DeskTopWndProc(hWnd=%.04x,wMsg=%.04x,wParam=%x,lParam=%x)\n",
//	hWnd,wMsg,wParam,lParam));

	FUNCTION_START
	FUNCTION_END
    return(1L);
}

BOOL WINAPI SetDeskWallPaper(LPSTR lpszBmpFileName)
{
	return TRUE;
}

/***********************************************************************
 *           SetDeskPattern   (USER.279)
 */
BOOL WINAPI SetDeskPattern(void)
{
    return 1;//SystemParametersInfo( SPI_SETDESKPATTERN, -1, NULL, FALSE );
}


LRESULT WINAPI SwitchWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
//    LOGSTR((LF_API,"DeskTopWndProc(hWnd=%.04x,wMsg=%.04x,wParam=%x,lParam=%x)\n",
//	hWnd,wMsg,wParam,lParam));

	FUNCTION_START
	FUNCTION_END
    return(1L);
}


LRESULT WINAPI TitleWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
//    LOGSTR((LF_API,"DeskTopWndProc(hWnd=%.04x,wMsg=%.04x,wParam=%x,lParam=%x)\n",
//	hWnd,wMsg,wParam,lParam));

	FUNCTION_START
	FUNCTION_END
    return(1L);
}

VOID WINAPI LW_InitWndMgr(HINSTANCE hInstance)
{
	WNDCLASS * pWndClass; // For use in registering classes
	HLOCAL hWndClass;

	FUNCTION_START

	//InitiaLize the following variabLes from static variabLes:
	//CXSize, CYSize, CYCaption, CXBorder, CYBorder, CYHScroLL,
	//and CXVScroLL

//	SetMinMaxInfo(); // Appears to initialize some static vars.
			 // Uses CXScreen, CYScreen, CXBorder, etc ...

	// @todo!!! Allocate 0x1A bytes.
	hWndClass = UserLocalAlloc(LT_USER_CLASS, LMEM_ZEROINIT, sizeof(WNDCLASS));
	pWndClass = (WNDCLASS *)LocalLock(hWndClass);

	pWndClass->lpszClassName = MAKELP(0, 0x8001);
	pWndClass->hCursor = LoadCursor(0, IDC_ARROW);
	pWndClass->lpfnWndProc = DeskTopWndProc;
	pWndClass->hInstance = hInstance;
	pWndClass->style = CS_DBLCLKS;
	pWndClass->hbrBackground = 2;
	RegisterClass( pWndClass); // Register the DeskTop cLass

	// Register the "switch window" class
	pWndClass->lpszClassName = MAKELP(0, 0x8003);
	pWndClass->hCursor = LoadCursor(0, IDC_ARROW);
	pWndClass->lpfnWndProc = SwitchWndProc;
	pWndClass->hInstance = hInstance;
	pWndClass->style = CS_SAVEBITS | CS_VREDRAW | CS_HREDRAW;
	pWndClass->hbrBackground = 2;
	RegisterClass( pWndClass );

	// Register the icon title cLass
	pWndClass->lpszClassName = MAKELP(0, 0x8004);
	pWndClass->hCursor = LoadCursor(0, IDC_ARROW);
	pWndClass->lpfnWndProc = TitleWndProc;
	pWndClass->hInstance = hInstance;
	pWndClass->style = 0;
	pWndClass->hbrBackground = 0;
	RegisterClass(pWndClass);

	LocalUnlock(hWndClass);
	LocalFree(hWndClass); // Don't need WNDCLASS anymore!

	// Create the desktop and switch windows
	HWndDesktop = CreateWindowEx( 0, MAKELP(0, 0x8001),
		0, WS_CLIPCHILDREN | WS_POPUP, 0, 0,
		CXScreen, CYScreen, 0, 0, hInstance, 0);

	HWndSwitch = CreateWindowEx( 0, MAKELP(0, 0x8003),
		0, WS_DISABLED | WS_POPUP, 0, 0, 0xA, 0xA,
		0, 0, hInstance, 0 );

	// Move the switch window to the center of the screen
	SetWindowPos( HWndSwitch, 0xFFFF, 0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE);

	HWndRealPopup = CreateWindowEx( 0, MAKELP(0, 0x8000), 0,
		WS_POPUP, 0, 0, 0x64, 0x64, 0, 0,
		hInstance, 0 ); // Pop-up menu???

	SetDeskPattern();
	SetDeskWallPaper((LPSTR)-1);

	// Set the wallpaper and pattern
	// Read names from the WIN.INI fiLe
	// Tell the desktop that the paLette may have changed from
	// loading the wallpaper image.
	SendMessage(HWndDesktop, WM_SYSCOLORCHANGE, 0, 0);

//	Toggle a bit in the HWndDesktop flags //?

//	InvalidateDCCache( HWndDesktop, 0); What this do??

	// Force the entire desktop to be refreshed
	InvalidateRect( HWndDesktop, 0, 1);
	UpdateWindow( HWndDesktop );

	FUNCTION_END
}

VOID WINAPI LW_DisplayDriverInit()
{
	HDC hDC;
	BOOL FOnBoardBitmap;

	// Need a device context below
	hDC = GetDC(0);//GetScreenDC(); 

	// If the display driver can save bits, get a function ptr
	// to the routine that does it. The function has an entry
	// ordinal of 92 (0x5C)
	FOnBoardBitmap = GetDeviceCaps( hDC, RASTERCAPS ) & 0x0040;
	if (FOnBoardBitmap)
		LpSaveBitmap=GetProcAddress(HInstanceDisplay,MAKELP(0,92));
	
	//Done with the device context
	ReleaseDC(0, hDC); //ReleaseCacheDC( hDC, 0 );

	// DISPLAY.500 -> UserRepaintDisable(). This function
	// tells the display driver when screen updates should be
	// enabled or disabled.
	LpDisplayCriticalSection =
		GetProcAddress( HInstanceDisplay, MAKELP(0, 500) );
}

VOID WINAPI LW_LoadFonts(VOID)
{
}

VOID WINAPI LW_DesktopIconInit(VOID)
{
}


VOID WINAPI LW_DrawIconInit(VOID)
{
}


VOID WINAPI LW_LoadTaskmanAndScreenSaver(VOID)
{
}


/**********************************************************************************
 *
 * This is main initialization code of USER.EXE, same as LoadWindows functions in
 * original Windows
 *
 */

BOOL PASCAL LibMain( HINSTANCE hInstance )
{
        HPEN hPenBlue;
        HBRUSH hBrushRed;
	RECT rect = {0,0,100,100};
	HDC desktop;

	FUNCTION_START
	TRACE("inst=%x", hInstance);
 
        // Save the module and instance handles away in global vars
	USER_HeapSel=hInstance;
	HModuleWin = GetModuleHandle(MK_FP(0, hInstance));

	// Loads USER strings variables from resources
	LW_LoadSomeStrings();


        // Get the number of entries in the system message queue and mul to 2
	CBEntries=UT_GetIntFromProfile(IDS_TYPEAHEAD, 0x3c) << 1;
        // Get the default number of messages in a task queue
	DefQueueSize=UT_GetIntFromProfile(IDS_DEFAULTQUEUESIZE, 8);

        // Create an application message
        // queue. This is needed to
        // create windows.
	CreateQueue(DefQueueSize);

	// Get the default border width for a window. Default is 3.
	ClBorder = UT_GetIntFromProfile(IDS_BORDER, 3);
	if (ClBorder < 1) // Make sure it's got a reasonable
            ClBorder = 1; // value.
        if (ClBorder > 0x32 )
            ClBorder = 0x32;


	// Setup and initialize the Keyboard,
	// mouse, and COMM drivers. The system
	// message queue is created here (in
	// DI_EventInit(), which is called from
	// LW_DriversInit()).
	LW_DriversInit();

	// LW_DCInit() is where the 5 DISPLAY device contexts are
	// created. Chapter 5 (Windows Internals) covers device contexts (DCs) in
	// detail. LW_DCInit() also calls @InitCreateRgn(),
	// GetDCState(), CreateCompatibleDC(), and GetStockObject().
	// The HFontSys and HFontSysFixed global vars are set here.
	LW_DCInit();

	// Calls GetStockObject() to set the variables
	// HBrWhite and HBrBlack. Calls CreateBitmap(),
	// CreatePatternBrush(), DeleteObject(),
	// and MakeObjectPrivate()
	LW_BrushInit();

	// Perform various initialization required by the DISPLAY
	// driver. Sets HInstanceDisplay variable by calling
	// GetModuleHandle("DISPLAY"). LW_OemDependentInit() is a very
	// large function, with numerous calls to routines such as
	// GetDeviceCaps(), GetStockObject(), SetResourceHandler(),
	// FindResource(), ODI_CreateBits(), and so on.
	LW_OEMDependentInit();


	// Sets HBmCursorBitmap and
	// HPermanentCursor variables.
	LW_OEMCursorInit();

	// Set the global variables: CLBorder, CXBorder, CXSzBorder,
	// CXSzBorderPlus1, CYBorder, CYSzBorder, CYSzBorderPlus1,
	// CXCwMargin, and CYCwMargin. Presumably these deal with
	// the size of the borders around Windows.
	InitSizeBorderDimensions();

	//Loads lots of icons and cursors.
	LW_LoadResources();


{
	TRACE("Create display context");
	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);
	TRACE("Create display context done");
	SetPixel(tempHDC, 10, 10, RGB(255, 0, 0));
        hPenBlue=CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
        SelectObject(tempHDC, hPenBlue);
        hBrushRed=CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(tempHDC, hBrushRed);
        Rectangle(tempHDC, 200, 0, 400, 200);

	DrawIcon(tempHDC, 205, 5, HCursNormal);
	DrawIcon(tempHDC, 250, 5, HCursIBeam);
	DrawIcon(tempHDC, 290, 5, HCursUpArrow);
	DrawIcon(tempHDC, 330, 5, HIconSample);

	DrawIcon(tempHDC, 205, 40, HIconWindows);
	DrawIcon(tempHDC, 250, 40, HCursSizeNWSE);
	DrawIcon(tempHDC, 290, 40, HCursSizeNESW);
	DrawIcon(tempHDC, 330, 40, HCursSizeNS);

	DrawIcon(tempHDC, 205, 80, HCursSizeWE);
	DrawIcon(tempHDC, 250, 80, HIconHand);
	DrawIcon(tempHDC, 290, 80, HIconQues);
	DrawIcon(tempHDC, 330, 80, HIconBang);

	DrawIcon(tempHDC, 205, 120, HIconNote);
	DrawIcon(tempHDC, 250, 120, HCursSizeAll);

//	DeleteDC(tempHDC);
}


	// Start out with no focus
	HWndFocus = 0; 

	// Loads values into the RGWSYSMet array.
	// These values can be retrieved via the
	// GetSystemMetrics() API
	LW_InitSysMetrics();

	// Register the windows classes for
	// "predefined" windows, such as
	// edit controls, etc. We'll come
	// back to this routine later.
//	LW_RegisterWindows(USER_HeapSel);

	// Allocate some memory for an internal buffer.
	// UserLocalAlloc() is a special version of LocalAlloc().
	// The first parameter "marks" each item in the USER local
	// heap with a value that indicates what it is. This
	// "tagging" of blocks only occurs in the debug version of
	// USER. See the TOOLHELP LocalEntry documentation for
	// a list of the various "tag" values.
	//PState = UserLocalAlloc(LT_USER_STRING, 0x40, some_static_var + 0x10 )

	// Sets X_Mickey_Rate, Y_Mickey_Rate, and
	// calls ClipCursor( 0 )
	LW_MouseInit();

	// Call the "enable" routine for various
	// input/output devices.
	EnableInput();

	// Middle of the screen
	//SetCursorPos(100, 100);
	// Middle of the screen
//	SetCursorPos(100, 100);
	MoveCursor(100,100);

	// Get the hourglass cursor, show it
	SetCursor(LoadCursor(0, IDC_WAIT));
for (;;);

	// Register the Desktop and
	// switch windows classes, and
	// create the windows.
	LW_InitWndMgr(hInstance);

	// Max button size???
	//WMaxBtnSize = MB_FindLongestString();
	
	// Create the global atom table.
	GlobalInitAtom();

//	AtomCheckPointProp = GlobalAddAtom("SysCP") // Used by
//	AtomBwlProp = GlobalAddAtom( "SysBW" ) // EnumProps()

//	MsgWinHelp = RegisterWindowMessage( "WM_WINHELP");

	// Allocate another local heap for menus
	MenuBase = HMenuHeap = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, 0x418 );

	// Allocate another local heap for menu strings
	MenuStringBase = HMenuStringHeap = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, 0x418 );

	// Initialize the menu and menu string heaps. The heaps
	// start out small (0x417 bytes), but can grow as needed.

	LocalInit(HMenuHeap, 0x12, 0x417);
	LocalInit(HMenuStringHeap, 0x12, 0x417);

	// Load the "system" menu ("Restore", "Move", "Size", etc.)
	HSysMenu = LoadMenu(USER_HeapSel, MK_FP(0, IDM_SYSMENU));

	LW_DisplayDriverInit(); // Gets entry points in display driver

	LW_LoadFonts();	// Uses AddFontResource() to load all the
			// fonts in the "fonts" section of WIN.INI

	LW_DesktopIconInit(); // Initialize things related to
				// desktop icons/fonts

	LW_DrawIconInit(); // Initializes HBmDrawIconMono and
				// HMbDrawIconColor

	LW_LoadTaskmanAndScreenSaver(); // Doesn't _load_ them. Just
					// gets configuration values

	TRACE("Create display context");
	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);
	TRACE("Create display context done");
	SetPixel(tempHDC, 10, 10, RGB(255, 0, 0));
        hPenBlue=CreatePen(PS_SOLID, 5, RGB(0, 0, 255));
        SelectObject(tempHDC, hPenBlue);
        hBrushRed=CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(tempHDC, hBrushRed);
        Rectangle(tempHDC, 200, 200, 400, 400);

	DeleteDC(tempHDC);

	FUNCTION_END
	return(1);
}

