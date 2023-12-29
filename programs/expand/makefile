#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = expand
TRGT = $(PROJ).exe
DESC = Windows clock
srcfiles = $(p)lzc$(e)

# defines additional options for C compiler
ADD_COPT = -sg -DEXPAND -I$(%WATCOM)$(SEP)h$(SEP)win


!include $(%ROOT)tools/mk/appsdos.mk

TARGETS = $(PATH)$(PROJ).exe

