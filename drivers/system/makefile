#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = system1
PROJ1    = system
TRGT     = $(PROJ1).drv
DESC     = Windows keyboard driver
srcfiles = $(p)system$(e) $(p)libentry$(e) 
ADD_COPT = -zc -s
ADD_ASMOPT = -i=..$(SEP)tables$(SEP)xlat
NOLIBS   = 1
NODEFAULTLIBS = 1
ADD_LINKOPT = OPTION NOD
STACKSIZE = 1k

# linking dll file
DLL     = 1
EXPORTS = &
	InquireSystem.1, &
	CreateSystemTimer.2, &
	KillSystemTimer.3, &
	EnableSystemTimers.4, &
	DisableSystemTimers.5, &
	GetSystemMSecCount.6, &
	Get80x87SaveSize.7, &
	Save80x87State.8, &
	Restore80x87State.9, &
	A20_Proc.20

#13 stub INQUIRELONGINTS # W1.1, W2.0
#14 stub ordinal only W1.1

IMPORTS = __AHSHIFT KERNEL.113
#GETPRIVATEPROFILEINT KERNEL.127, &

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).drv

$(PATH)$(PROJ1).drv: $(PATH)$(PROJ).dll
 @$(CP) $(PATH)$(PROJ).dll $(PATH)$(PROJ1).drv $(BLACKHOLE)
