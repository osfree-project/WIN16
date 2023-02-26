#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = user1
PROJ1    = user
TRGT     = $(PROJ1).exe
DESC     = Windows USER.EXE
srcfiles = $(p)Rect$(e)
ADD_COPT = -zc -s
ADD_ASMOPT = -i=..$(SEP)tables$(SEP)xlat
#NOLIBS   = 1
#NODEFAULTLIBS = 1
#ADD_LINKOPT = OPTION NOD
STACKSIZE = 1k

# linking dll file
DLL     = 1
EXPORTS = &
	INQUIRE     	.1, &
	ENABLE      	.2, &
	DISABLE     	.3, &
	TOASCII 	.4, &
	ANSITOOEM 	.5, &
	OEMTOANSI 	.6, &
	SETSPEED    	.7, &
	SCREENSWITCHENABLE.100, &
	GETTABLESEG     .126, &
	NEWTABLE	.127, &
	OEMKEYSCAN 	.128, &
	VKKEYSCAN 	.129, &
	GETKEYBOARDTYPE .130, &
	MAPVIRTUALKEY 	.131, &
	GETKBCODEPAGE 	.132, &
	GETKEYNAMETEXT 	.133, &
	ANSITOOEMBUFF 	.134, &
	OEMTOANSIBUFF 	.135, &
	ENABLEKBSYSREQ	.136, &
	GETBIOSKEYPROC	.137
#IMPORTS = GETPRIVATEPROFILEINT KERNEL.127, &
#          __AHSHIFT KERNEL.113

#OPTIONS = heapsize=0
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).dll
 @$(CP) $(PATH)$(PROJ).dll $(PATH)$(PROJ1).exe $(BLACKHOLE)
