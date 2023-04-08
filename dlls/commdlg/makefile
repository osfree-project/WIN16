#
# A Makefile for commdlg.dll
# (c) osFree project,
# prokushev, 2023
#

PROJ     = commdlg
TRGT     = $(PROJ).dll
DESC     = Windows Common Dialogs Library
srcfiles = $(p)libmain$(e) $(p)ChooseColor$(e) $(p)ChooseFont$(e) $(p)Commdlg$(e) $(p)FindReplDlg$(e) $(p)GetOpenFileName$(e) $(p)PrintDlg$(e)
# linking dll file
DLL     = 1
EXPORTS = &
	GetOpenFileName	.1, &
	GetSaveFileName .2, &
	ChooseColor	.5, &
	FILEOPENDLGPROC.6=WGOFNDialogProc, &
	FILESAVEDLGPROC.7=WGOFNDialogProc, &
	COLORDLGPROC.8=WCCDialogProc, &
	FindText	.11, &
	ReplaceText	.12, &
	FINDTEXTDLGPROC.13=FindDlgProc, &
	REPLACETEXTDLGPROC.14=ReplaceDlgProc, &
	ChooseFont	.15, &
	CommDlgExtendedError .26, &
	GetFileTitle	.27

#	LOADALTERBITMAP.9, &
#	FORMATCHARDLGPROC.16, &
#	FONTSTYLEENUMPROC.18, &
#	FONTFAMILYENUMPROC.19, &
#	PrintDlg	.20, &
#	PRINTDLGPROC.21=PrintDlgProc, &
#	PRINTSETUPDLGPROC.22=PrintSetupDlgProc, &
#	EDITINTEGERONLY.23, &
#	WANTARROWS.25, &
#	DWLBSUBCLASS.29, &
#	DWUPARROWHACK.30, &
#	DWOKSUBCLASS.31

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system
#ADD_COPT=-pl

!include $(%ROOT)tools/mk/appsw16.mk
