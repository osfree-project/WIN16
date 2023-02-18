#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = keyboard1
PROJ1    = keyboard
TRGT     = $(PROJ1).drv
DESC     = Windows keyboard driver
srcfiles = $(p)keyboard$(e)
ADD_COPT = -dCARDSTYLE_DEFAULT

# linking dll file
DLL     = 1
EXPORTS = &
	ToAscii 	.4, &
	AnsiToOem 	.5, &
	OemToAnsi 	.6, &
	OemKeyScan 	.128, &
	VkKeyScan 	.129, &
	GetKeyboardType .130, &
	MapVirtualKey 	.131, &
	GetKBCodePage 	.132, &
	GetKeyNameText 	.133, &
	AnsiToOemBuff 	.134, &
	OemToAnsiBuff 	.135

OPTIONS = heapsize=32K libfile libentry.obj
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).drv

$(PATH)$(PROJ1).drv: $(PATH)$(PROJ).dll
 @$(CP) $(PATH)$(PROJ).dll $(PATH)$(PROJ1).drv $(BLACKHOLE)
