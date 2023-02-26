#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = user1
PROJ1    = user
TRGT     = $(PROJ1).exe
DESC     = Windows USER.EXE
srcfiles = $(p)Rect$(e) $(p)WinInit$(e) $(p)Icon$(e) $(p)Menu$(e) $(p)Strings$(e) $(p)Accelerators$(e) $(p)libmain$(e)
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
	LoadMenu .150, &
	ChangeMenu .153, &
	LoadString .176, &
	LoadAccelerators .177, &
	EqualRect.244, &
	SubtractRect .373, &
	lstrcmp .430, &
	AnsiUpper .431, &
	AnsiLower .432, &
	AnsiNext .472, &
	AnsiPrev .473

#IMPORTS = GETPRIVATEPROFILEINT KERNEL.127, &
#          __AHSHIFT KERNEL.113

#OPTIONS = heapsize=0
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).dll
 @$(CP) $(PATH)$(PROJ).dll $(PATH)$(PROJ1).exe $(BLACKHOLE)
