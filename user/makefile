#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = user1
PROJ1    = user
TRGT     = $(PROJ1).exe
DESC     = Windows USER.EXE
srcfiles = $(p)Rect$(e) $(p)WinInit$(e) $(p)Icon$(e) $(p)Menu$(e) $(p)Strings$(e) $(p)Accelerators$(e) $(p)Events$(e) $(p)libmain$(e)
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
	LoadString .176, &
	LoadAccelerators .177, &
	UserSeeUserDo .216, &
	LoadMenuIndirect .220, &
	EqualRect.244, &
	OldSetDeskPattern .279, &
	GetFreeSystemResources .284, &
	mouse_event .299, &
	IsUserIdle .333, &
	GetMouseEventProc .337, &
	CopyIcon .368, &
	CopyCursor .369, &
	SubtractRect .373, &
	DllEntryPoint .374, &
	CreateCursor .406, &
	CreateIcon .407, &
	CreateCursorIconIndirect .408, &
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
	LSTRCMPI .471, &
	AnsiNext .472, &
	AnsiPrev .473

#	RegisterClass .57, &

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
