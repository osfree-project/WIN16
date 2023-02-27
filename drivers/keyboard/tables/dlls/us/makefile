#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = kbdus
TRGT     = $(PROJ).dll
DESC     = Windows KBDUS Library
srcfiles = $(p)keyboard$(e)
# linking dll file
DLL     = 1
STACKSIZE = 8k
HEAPSIZE = 32K
EXPORTS = &
	GetKbdTable.1, &
	GetKeyString.2

# WEP Need this?

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk
