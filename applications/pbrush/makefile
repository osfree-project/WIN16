#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = pbrush1
PROJ1 = pbrush
TRGT = $(PROJ1).exe
DESC = osFree Janus Paint
srcfiles = $(p)paint$(e) $(p)canvas$(e) $(p)toolbox$(e) $(p)palette$(e) $(p)textedit$(e) $(p)clipboard$(e) $(p)file$(e) $(p)tools$(e)

# defines additional options for C compiler
ADD_COPT = -sg -0
#ADD_LINKOPT = LIB commdlg.lib

EXPORTS = MainWndProc, CanvasWndProc, PaletteWndProc, TextEdit_SubclassProc, OpenDlgProc, SaveDlgProc, ColorDlgProc, ToolboxWndProc

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe


.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)$(PROJ1).rc
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -q -30 -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
