#
# A Makefile for lzexpand.dll
# (c) osFree project,
# author, date
#

PROJ     = ddeml
TRGT     = $(PROJ).dll
DESC     = Windows file expansion library
srcfiles = $(p)DdeML$(e)
# linking dll file
DLL     = 1
EXPORTS = 	DdeInitialize		.2, &
	DdeUninitialize		.3, &
	DdePostAdvise		.13, &
	DdeCreateStringHandle 	.21, &
	DdeFreeStringHandle 	.22, &
	DdeKeepStringHandle 	.24, &
	DdeEnableCallback 	.26, &
	DdeNameService 		.27


DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system
OPTIONS = heapsize=1K lib lzexpand.lib

!include $(%ROOT)tools/mk/appsw16.mk

