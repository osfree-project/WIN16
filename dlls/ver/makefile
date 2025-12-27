#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = ver
TRGT     = $(PROJ).dll
DESC     = Windows ver API Library
srcfiles = $(p)version$(e) #$(p)movefile$(e)
# linking dll file
DLL     = 1
EXPORTS = &
	GetFileResourceSize.2, &
	GetFileResource.3, &
	GetFileVersionInfoSize.6, &
	GetFileVersionInfo.7, &
	VerFindFile.8, &
	VerInstallFile.9, &
	VerLanguageName.10, &
	VerQueryValue.11

OPTIONS = heapsize=32K lib lzexpand.lib 
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system
#ADD_COPT=-pl

!include $(%ROOT)tools/mk/appsw16.mk
