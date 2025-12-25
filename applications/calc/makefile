#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = calc1
PROJ1 = calc
TRGT = $(PROJ1).exe
DESC = Windows Calculator
srcfiles = $(p)stats$(e) $(p)winecalc$(e)

# defines additional options for C compiler
#ADD_COPT = -0 -sg -zw -bw -bg -d3 -db -hw -ml -od #-pl
ADD_COPT =  -dDEBUG=0
ADD_LINKOPT = LIB commdlg.lib
#DEBUG    = watcom
#IMPORTS  = CHOOSEFONT     COMMDLG.15

#           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr
STACKSIZE = 34096
#HEAPSIZE = 50960

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

#$(PATH)$(PROJ1).res: $(PATH)rsrc.rc
# @$(SAY) WINRES   $^. $(LOG)
# @winres $^< -I $(%WATCOM)$(SEP)h$(SEP)win -o $^@

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)rsrc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
