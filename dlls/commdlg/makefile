#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = commdls
TRGT     = $(PROJ).dll
DESC     = Windows Common Dialogs Library
srcfiles = $(p)ChooseColor$(e) $(p)ChooseFont$(e) $(p)Commdlg$(e) $(p)FindReplDlg$(e) $(p)GetOpenFileName$(e) $(p)PrintDlg$(e)
# linking dll file
DLL     = 1
EXPORTS = &
	GetOpenFileName	.1, &
	GetSaveFileName .2, &
	ChooseColor	.5, &
	FindText	.11, &
	ReplaceText	.12, &
	ChooseFont	.15, &
	PrintDlg	.20, &
	CommDlgExtendedError .26, &
	GetFileTitle	.27


OPTIONS = heapsize=32K libfile libentry.obj
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system
#ADD_COPT=-pl

!include $(%ROOT)tools/mk/appsw16.mk
