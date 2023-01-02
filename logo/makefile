#
# A Makefile for LOGO.LGO
# (c) osFree project,
# author, date
#

PROJ        = logo
TRGT        = $(PROJ).com
srcfiles    = $(p)logo$(e)
JWASM       = 1
COM         = 1
NOLIBS      = 1
INSTALL_ADD = 1
ADD_ASMOPT  = -I=$(MYDIR)..$(SEP)..$(SEP)macrolib
DEST        = os2$(SEP)mdos$(SEP)winos2

!include $(%ROOT)tools/mk/appsdos.mk

.SUFFIXES: .lgo .com

.lgo: $(PATH)

install_add: .symbolic
 @$(SAY) INST     $(PROJ).lgo $(LOG)
 @$(CP) $(PATH)$(PROJ).com $(PATH)$(PROJ).lgo $(BLACKHOLE)
 @$(CP) $(PATH)$(PROJ).lgo $(DEST) $(BLACKHOLE)

#$(PATH)$(PROJ).exe: .symbolic
