#
# A Makefile for TASKMAN
# (c) osFree project,
# author, date
#

PROJ  = taskman1
PROJ1 = taskman
TRGT = $(PROJ1).exe
DESC = Windows Task Manager
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)taskman$(e) 
# defines additional options for C compiler
ADD_COPT = -sg
HEAPSIZE = 4k

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)rsrc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
