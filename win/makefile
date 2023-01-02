#
# A Makefile for WIN.COM
# (c) osFree project,
# author, date
#

PROJ       = win
TRGT       = $(PROJ).com
DESC       = Windows Loader
srcfiles   = $(p)execlogo$(e)
JWASM      = 1
COM        = 1
NOLIBS     = 1
DEST       = os2$(SEP)mdos$(SEP)winos2

!include $(%ROOT)tools/mk/appsdos.mk

$(PATH)$(PROJ).exe: .symbolic
