#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = progman1
PROJ1 = progman
TRGT = $(PROJ1).exe
DESC = Windows Program Manager
srcfiles = $(p)dialog$(e) $(p)group$(e) $(p)grpfile$(e) $(p)main$(e) $(p)program$(e) $(p)string$(e)

# defines additional options for C compiler
#ADD_COPT = -0 -sg -zw -bw -bg -d3 -db -hw -ml -od #-pl
ADD_COPT = -sg -DDEBUG=1
#DEBUG    = watcom
IMPORTS  = GETOPENFILENAME     COMMDLG.1, &
           GETCURRENTDIRECTORY KERNEL.411
#           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

#$(PATH)$(PROJ1).res: $(PATH)rsrc.rc
# @$(SAY) WINRES   $^. $(LOG)
# @winres $^< -I $(%WATCOM)$(SEP)h$(SEP)win -o $^@

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)rsrc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(%WATCOM)$(SEP)h$(SEP)win
