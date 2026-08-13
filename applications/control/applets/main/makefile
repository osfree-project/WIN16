#
# A Makefile for osFree Janus Main Control Panel applet
# (c) osFree project
#

DESC         = Main System Settings
TARGET_API   = WIN
TARGET_CLASS = LIBRARY
TARGET_EXT   = cpl
SOURCES      = main mouse keyboard time desktop intl_main intl_date intl_time intl_number intl_currency
LIBS         = nlsapi

ADD_COPT    = -ms -sg -I=$(MYDIR)..$(SEP).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)include


EXPORTS = CPlApplet.1, &
	MouseDlgProc.2, &
	DesktopDlgProc.3, &
	SystemTimerProc.4, &
	DateTimeDlgProc.5, &
	AnalogClockSubclassProc.6, &
	CalendarSubclassProc.7, &
	InternationalDlgProc.8, &
	TimeFmtDlgProc.9, &
	DateFmtDlgProc.10, &
	CurrencyFmtDlgProc.11, &
	NumberFmtDlgProc.12

IMPORTS = CreateSystemTimer SYSTEM.2, &
	KillSystemTimer SYSTEM.3

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system
                       
!include $(%ROOT)tools/mk/build.mk

