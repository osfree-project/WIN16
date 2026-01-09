#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = user
TRGT     = $(PROJ).exe
DESC     = Windows USER.EXE
srcfiles = $(p)smallcode$(e) $(p)libentry$(e) $(p)libmain$(e) $(p)dce$(e) $(p)cursor$(e) $(p)queue$(e) $(p)GlobalData1$(e) $(p)div$(e) $(p)hook$(e) $(p)dialog$(e) $(p)Time$(e) $(p)Atoms$(e) $(p)comm$(e) $(p)Caret$(e) $(p)DrawText$(e)  $(p)SysMetrics$(e) $(p)Listbox$(e) $(p)Windows$(e) $(p)Controls$(e) $(p)Rect$(e) $(p)WinInit$(e) $(p)Icon$(e) $(p)Menu$(e) $(p)Strings$(e) $(p)Accelerators$(e) $(p)Events$(e) $(p)Resources$(e) $(p)Misc$(e) $(p)Classes$(e) $(p)printf$(e) # $(p)Desktop$(e)  $(p)StringFunc$(e) $(p)network$(e) $(p)InitApp$(e)
ADD_COPT = -zW -s -2 -DDEBUG=1 # optimize prolog, no stack checks, put constants in code segment 
# -zc
ADD_LINKOPT = SEGMENT CLASS DATA FIXED SHARE
NOLIBS = 1 # Don't use default libentry
NODEFAULTLIBS = 1 # Don'y use libc

STACKSIZE = 10k
HEAPSIZE = 20k

# linking dll file
DLL     = 1
EXPORTS = &
	MessageBox.1, &
	OldExitWindows .2, &
	DisableOEMLayer .4, &
	ExitWindows .7, &
	BEAR11 .11, &
	GetTickCount .13, &
	GetCurrentTime .15, &
	SetCapture .18, &
	ReleaseCapture .19, &
	SetFocus .22, &
	GetFocus .23, &
	ScreenToClient .29, &
	GetWindowRect .32, &
	GetClientRect .33, &
	GetWindowText .36, &
	SetWindowText .37, &
	GetWindowTextLength .38, &
	BeginPaint .39, &
	EndPaint .40, &
	CreateWindow .41, &
	GetParent .46, &
	IsWindow .47, &
	IsWindowVisible .49, &
	DestroyWindow .53, &
	RegisterClass .57, &
	ScrollWindow .61, &
	SetScrollPos .62, &
	GetScrollPos .63, &
	SetScrollRange .64, &
	GetScrollRange .65, &
	GetDC .66, &
	ReleaseDC .68, &
	SetRect .72, &
	SetRectEmpty .73, &
	CopyRect .74, &
	IsRectEmpty .75, &
	PtInRect .76, &
	OffsetRect .77, &
	InflateRect .78, &
	IntersectRect .79, &
	UnionRect .80, &
	FillRect .81, &
	InvertRect .82, &
	FrameRect .83, &
	DrawIcon .84, &
	DialogBox .87, &
	CreateDialog .89, &
	GetDlgItem .91, &
	SetDlgItemText .92, &
	GetDlgItemText .93, &
	CheckDlgButton .97, &
	IsDlgButtonChecked  .98, &
	DlgDirSelect .99, &
	SendDlgItemMessage .101, &
	GetKeyState .106, &
	DefWindowProc .107, &
	GetMessage .108, &
	PeekMessage  .109, &
	PostMessage .110, &
	SendMessage .111, &
	DispatchMessage .114, &
	SetWindowsHook .121, &
	CallMsgFilter .123, &
	UpdateWindow .124, &
	InvalidateRect .125, &
	InvalidateRgn .126, &
	ValidateRect .127, &
	ValidateRgn .128, &
	GetWindowWord .133, &
	SetWindowWord .134, &
	GetWindowLong .135, &
	SetWindowLong .136, &
	LoadMenu .150, &
	ChangeMenu .153, &
	CheckMenuItem .154, &
	EnableMenuItem .155, &
	GetSystemMenu .156, &
	GetMenu .157, &
	SetMenu .158, &
	GetSubMenu .159, &
	DrawMenuBar .160, &
	GetMenuString .161, &
	HiliteMenuItem .162, &
	CreateCaret .163, &
	DestroyCaret .164, &
	SetCaretPos .165, &
	HideCaret .166, &
	ShowCaret .167, &
	SetCaretBlinkTime .168, &
	GetCaretBlinkTime .169, &
	LoadBitmap.175, &
	LoadString .176, &
	LoadAccelerators .177, &
	GetSystemMetrics .179, &
	GetSysColor .180, &
	SetSysColors .181, &
	BEAR182 .182, &
	GetCaretPos .183, &
	GrayString .185, &
	DlgDirSelectComboBox .194, &
	TabbedTextOut .196, &
	GetTabbedTextExtent .197, &
	OpenComm .200, &
	SetCommState .201, &
	GetCommState .202, &
	GetCommError .203, &
	ReadComm .204, &
	WriteComm .205, &
	TransmitCommChar .206, &
	CloseComm .207, &
	SetCommEventMask .208, &
	GetCommEventMask .209, &
	SetCommBreak .210, &
	ClearCommBreak .211, &
	UngetCommChar .212, &
	BuildCommDCB .213, &
	EscapeCommFunction .214, &
	FlushComm .215, &
	UserSeeUserDo .216, &
	DialogBoxIndirect .218, &
	CreateDialogIndirect .219, &
	LoadMenuIndirect .220, &
	GetWindowTask .224, &
	SetWindowPos .232, &
	DefHookProc .235, &
	DialogBoxParam .239, &
	DialogBoxIndirectParam .240, &
	CreateDialogParam .241, &
	CreateDialogIndirectParam .242, &
	EqualRect.244, &
	EnableCommNotification .245, &
	GetAsyncKeyState .249, &
	GetMenuState .250, &
	MapWindowPoints .258, &
	GetMenuItemCount .263, &
	GetMenuItemID .264, &
	ShowScrollBar .267, &
	GlobalAddAtom .268, &
	GlobalDeleteAtom .269, &
	GlobalFindAtom .270, &
	GlobalGetAtomName .271, &
	GetDlgCtrlID .277, &
	SetDeskPattern .279, &
	GetFreeSystemResources .284, &
	RedrawWindow .290, &
	SetWindowsHookEx .291, &
	CallNextHookEx .293, &
	mouse_event .299, &
	SignalProc .314, &
	SysErrorBox .320, &
	FillWindow .324, &
	PaintRect .325, &
	GetControlBrush .326, &
	UserYield .332, &
	IsUserIdle .333, &
	GetMouseEventProc .337, &
	IsMenu .358, &
	CopyIcon .368, &
	CopyCursor .369, &
	SubtractRect .373, &
	DllEntryPoint .374, &
	FinalUserInit .400, &
	UnregisterClass .403, &
	GetClassInfo .404, &
	CreateCursor .406, &
	CreateIcon .407, &
	CreateCursorIconIndirect .408, &
	InsertMenu .410, &
	AppendMenu .411, &
	RemoveMenu .412, &
	DeleteMenu .413, &
	ModifyMenu .414, &
	CreatePopupMenu .415, &
	TrackPopupMenu .416, &
	GetMenuCheckMarkDimensions .417, &
	SetMenuItemBitmaps .418, &
	_wsprintf .420, &
	wvsprintf .421, &
	DlgDirSelectEx .422, &
	DlgDirSelectComboBoxEx .423, &
	lstrcmp .430, &
	AnsiUpper .431, &
	AnsiLower .432, &
	ISCHARALPHA .433, &
	ISCHARALPHANUMERIC .434, &
	IsCharUpper .435, &
	IsCharLower .436, &
	ANSIUPPERBUFF .437, &
	ANSILOWERBUFF .438, &
	CreateWindowEx .452, &
	DestroyIcon .457, &
	DestroyCursor .458, &
	DrawFocusRect .466, &
	StringFunc .470, &
	LSTRCMPI .471, &
	AnsiNext .472, &
	AnsiPrev .473, &
	EnableScrollBar .482, &
	SystemParametersInfo .483#, &
#	RegisterClassEx .397, &
#	GetClassInfoEx .398, &
#	WNetErrorText .499, &
#	WNetOpenJob .501, &
#	WNetCloseJob .502, &
#	WNetAbortJob .503, &
#	WNetHoldJob .504, &
#	WNetReleaseJob .505, &
#	WNetCancelJob .506, &
#	WNetSetJobCopies .507, &
#	WNetWatchQueue .508, &
#	WNetUnwatchQueue .509, &
#	WNetLockQueueData .510, &
#	WNetUnlockQueueData .511, &
#	WNetGetCaps .513, &
#	WNetDeviceMode .514, &
#	WNetBrowseDialog .515, &
#	WNetAddConnection .517, &
#	WNetCancelConnection .518, &
#	WNetGetError .519, &
#	WNetGetErrorText .520, &
#	WNetRestoreConnection .523, &
#	WNetWriteJob .524, &
#	WNetConnectDialog .525, &
#	WNetDisconnectDialog .526, &
#	WNetConnectionDialog .527, &
#	WNetViewQueueDialog .528, &
#	WNetPropertyDialog .529, &
#	WNetDirectoryNotify .531, &
#	WNetGetPropertyText .532


IMPORTS = GETEXEVERSION KERNEL.105, &
  GetTaskQueue KERNEL.35, &
  GETEXEPTR KERNEL.133, &
  LOCALCOUNTFREE KERNEL.161, &
  LOCALHEAPSIZE KERNEL.162, &
  FARSETOWNER KERNEL.403, &
  FarGetOwner KERNEL.404, &
  CreateSystemTimer SYSTEM.2, &
  EnableSystemTimers SYSTEM.4, &
  GetSystemMSecCount SYSTEM.6, &
  Resurrection GDI.122, &
  InquireMouse MOUSE.1, &
  InquireKeyboard KEYBOARD.1, &
  InquireDisplay DISPLAY.101, &
  DisplaySetCursor DISPLAY.102, &
  MoveCursor DISPLAY.103, &
  CheckCursor DISPLAY.104, &
  SetTaskQueue KERNEL.34, &
  MakeObjectPrivate GDI.463, &
  GetDCState GDI.179, &
  SetDCState GDI.180, &
  SetSpeed KEYBOARD.7, &
  KeyboardEnable KEYBOARD.2, &
  MouseEnable MOUSE.2

# GETPRIVATEPROFILEINT KERNEL.127, &
#          __AHSHIFT KERNEL.113

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ).exe

.ico: $(MYDIR)res

$(PATH)$(PROJ).exe: $(PATH)$(PROJ).dll $(MYDIR)$(PROJ).rc
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -q -bt=windows -30 $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win

