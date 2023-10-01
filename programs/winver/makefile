#
# A Makefile for WINVER
# (c) osFree project,
# author, date
#

PROJ  = winver1
PROJ1 = winver
TRGT = $(PROJ1).exe
DESC = Windows Version
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)winver$(e) 
# defines additional options for C compiler
ADD_COPT = -sg
IMPORTS  = SHELLABOUT SHELL.22
HEAPSIZE = 4k
#STACKSIZE = 8k

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)rsrc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
