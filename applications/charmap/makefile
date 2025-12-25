#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = charmap1
PROJ1 = charmap
TRGT = $(PROJ1).exe
DESC = Windows charmap
srcfiles = $(p)charmap$(e) $(p)lrgcell$(e) $(p)map$(e) $(p)settings$(e) $(p)about$(e)

# defines additional options for C compiler
#ADD_COPT = -0 -sg -zw -bw -bg -d3 -db -hw -ml -od #-pl
ADD_COPT = -mh -sg -DDEBUG=1
#DEBUG    = watcom
IMPORTS  = CHOOSEFONT     COMMDLG.15

#           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

#$(PATH)$(PROJ1).res: $(PATH)rsrc.rc
# @$(SAY) WINRES   $^. $(LOG)
# @winres $^< -I $(%WATCOM)$(SEP)h$(SEP)win -o $^@

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)charmap.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
