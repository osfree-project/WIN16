#
# A Makefile for commdlg.dll
# (c) osFree project
#

DESCRIPTION  = Janus Common Dialogs Library
TARGET_CLASS = LIBRARY
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
	PrintDlg	.20, &
	PRINTDLGPROC.21=PrintDlgProc, &
	PRINTSETUPDLGPROC.22=PrintSetupDlgProc, &
	CommDlgExtendedError .26, &
	GetFileTitle	.27

#	LOADALTERBITMAP.9, &
#	FORMATCHARDLGPROC.16, &
#	FONTSTYLEENUMPROC.18, &
#	FONTFAMILYENUMPROC.19, &
#	EDITINTEGERONLY.23, &
#	WANTARROWS.25, &
#	DWLBSUBCLASS.29, &
#	DWUPARROWHACK.30, &
#	DWOKSUBCLASS.31

DESTINATION = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/build.mk

