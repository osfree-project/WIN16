#
# A Makefile for WIN.COM
# (c) osFree project,
# Yuri Prokuskev, 11.09.2022
#

PROJ       = win
TRGT       = $(PROJ).com
DESC       = Windows Loader
srcfiles   = $(p)execlogo$(e)
JWASM      = 1
COM        = 1
NOLIBS     = 1
DEST       = os2$(SEP)mdos$(SEP)winos2
ADD_ASMOPT = -I=$(MYDIR)..$(SEP)..$(SEP)macrolib

!include $(%ROOT)tools/mk/appsdos.mk

#$(PATH)$(PROJ).exe: .symbolic
