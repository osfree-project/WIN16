#
# A Makefile for WinOS/2 Help
# (c) osFree project,
#

PROJ  = winhelp1
PROJ1 = winhelp
TRGT = $(PROJ1).exe
DESC = Windows Help
srcfiles = &
		$(p)accel$(e) &
		$(p)back$(e) &
		$(p)btnbar$(e) &
		$(p)dialogs$(e) &
		$(p)dispbuff$(e) &
		$(p)display$(e) &
		$(p)elements$(e) &
		$(p)filebuff$(e) &
		$(p)files$(e) &
		$(p)fontbuff$(e) &
		$(p)globals$(e) &
		$(p)helplist$(e) &
		$(p)history$(e) &
		$(p)hlpfile$(e) &
		$(p)hlpmacro$(e) &
		$(p)inifile$(e) &
		$(p)libmain$(e) &
		$(p)macroeng$(e) &
		$(p)menu$(e) &
		$(p)msgbox$(e) &
		$(p)portable$(e) &
		$(p)search$(e) &
		$(p)secwnd$(e) &
		$(p)topicdat$(e) &
		$(p)winmem$(e) &
		$(p)wnddata$(e) &
		$(p)wndlist$(e) $(p)wndpos$(e)


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

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)twinhelp.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
