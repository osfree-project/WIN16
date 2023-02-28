#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = sound1
PROJ1    = sound
TRGT     = $(PROJ1).drv
DESC     = Windows keyboard driver
srcfiles = $(p)Sound$(e) $(p)libentry$(e) 
ADD_COPT = -zc -s
NOLIBS   = 1
NODEFAULTLIBS = 1
ADD_LINKOPT = OPTION NOD
STACKSIZE = 1k

# linking dll file
DLL     = 1
EXPORTS = &
	OpenSound 		.1, &
	CloseSound		.2, &
	SetVoiceQueueSize	.3, &
	SetVoiceNote		.4, &
	SetVoiceAccent		.5, &
	StartSound		.9, &
	StopSound		.10

IMPORTS = GETPRIVATEPROFILEINT KERNEL.127, &
          __AHSHIFT KERNEL.113

#OPTIONS = heapsize=0
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).drv

$(PATH)$(PROJ1).drv: $(PATH)$(PROJ).dll
 @$(CP) $(PATH)$(PROJ).dll $(PATH)$(PROJ1).drv $(BLACKHOLE)
