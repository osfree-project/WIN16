#
# A Makefile for LOGO.LGO
# (c) osFree project,
# author, date
#

PROJ  = xlat8661
PROJ1 = xlat866
TRGT = $(PROJ1).bin
srcfiles    = $(p)xlat866$(e)
JWASM       = 1
COM         = 1
NOLIBS      = 1
#INSTALL_ADD = 1
#ADD_ASMOPT  = -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)macrolib
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsdos.mk

.SUFFIXES: .bin

TARGETS = $(PATH)$(PROJ1).bin

#.bin: $(PATH)

#$(PATH)$(PROJ1).bin: $(PATH)$(PROJ).com 
# @$(CP) $(PATH)$(PROJ).com $(PATH)$(PROJ).bin $(BLACKHOLE)
