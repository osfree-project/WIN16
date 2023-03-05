#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = user1
PROJ1    = user
TRGT     = $(PROJ1).exe
DESC     = Windows USER.EXE
srcfiles = $(p)network$(e) $(p)SysMetrics$(e) $(p)Listbox$(e) $(p)Windows$(e) $(p)Controls$(e) $(p)Rect$(e) $(p)WinInit$(e) $(p)Icon$(e) $(p)Menu$(e) $(p)Strings$(e) $(p)Accelerators$(e) $(p)Events$(e) $(p)Misc$(e) $(p)Classes$(e) $(p)libmain$(e)
ADD_COPT = -zc -s
ADD_ASMOPT = -i=..$(SEP)tables$(SEP)xlat
#NOLIBS   = 1
#NODEFAULTLIBS = 1
#ADD_LINKOPT = OPTION NOD
STACKSIZE = 1k

# linking dll file
DLL     = 1
EXPORTS = &
	OldExitWindows .2, &
	CreateWindow .41, &
	RegisterClass .57, &
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
	LoadString .176, &
	LoadAccelerators .177, &
	GetSystemMetrics .179, &
	GetSysColor .180, &
	GrayString .185, &
	UserSeeUserDo .216, &
	LoadMenuIndirect .220, &
	EqualRect.244, &
	OldSetDeskPattern .279, &
	GetFreeSystemResources .284, &
	mouse_event .299, &
	FillWindow .324, &
	PaintRect .325, &
	GetControlBrush .326, &
	IsUserIdle .333, &
	GetMouseEventProc .337, &
	CopyIcon .368, &
	CopyCursor .369, &
	SubtractRect .373, &
	DllEntryPoint .374, &
	RegisterClassEx .397, &
	GetClassInfoEx .398, &
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
	_wsprintf .420, &
	wvsprintf .421, &
	lstrcmp .430, &
	AnsiUpper .431, &
	AnsiLower .432, &
	ISCHARALPHA .433, &
	ISCHARALPHANUMERIC .434, &
	IsCharUpper .435, &
	IsCharLower .436, &
	ANSIUPPERBUFF .437, &
	ANSILOWERBUFF .438, &
	DestroyIcon .457, &
	DestroyCursor .458, &
	DrawFocusRect .466, &
	LSTRCMPI .471, &
	AnsiNext .472, &
	AnsiPrev .473, &
	WNetErrorText .499, &
	WNetOpenJob .501, &
	WNetCloseJob .502, &
	WNetAbortJob .503, &
	WNetHoldJob .504, &
	WNetReleaseJob .505, &
	WNetCancelJob .506, &
	WNetSetJobCopies .507, &
	WNetWatchQueue .508, &
	WNetUnwatchQueue .509, &
	WNetLockQueueData .510, &
	WNetUnlockQueueData .511, &
	WNetGetCaps .513, &
	WNetDeviceMode .514, &
	WNetBrowseDialog .515, &
	WNetAddConnection .517, &
	WNetCancelConnection .518, &
	WNetGetError .519, &
	WNetGetErrorText .520, &
	WNetRestoreConnection .523, &
	WNetWriteJob .524, &
	WNetConnectDialog .525, &
	WNetDisconnectDialog .526, &
	WNetConnectionDialog .527, &
	WNetViewQueueDialog .528, &
	WNetPropertyDialog .529, &
	WNetDirectoryNotify .531, &
	WNetGetPropertyText .532


IMPORTS = GETEXEVERSION KERNEL.105, &
	GETEXEPTR KERNEL.133, &
	LOCALCOUNTFREE KERNEL.161, &
	LOCALHEAPSIZE KERNEL.162, &
	FARSETOWNER KERNEL.403

# GETPRIVATEPROFILEINT KERNEL.127, &
#          __AHSHIFT KERNEL.113

#OPTIONS = heapsize=0
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).dll
 @$(CP) $(PATH)$(PROJ).dll $(PATH)$(PROJ1).exe $(BLACKHOLE)
