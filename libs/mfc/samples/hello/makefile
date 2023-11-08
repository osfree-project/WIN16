#
# A Makefile for WinOS/2 PIF Editor
# (c) osFree project,
#

PROJ  = hello1
PROJ1 = hello
TRGT = $(PROJ1).exe
DESC = OFC Hello
srcfiles = $(p)hello$(e) &
		$(p)CArchive$(e) &
		$(p)CCmdTarget$(e) &
		$(p)CCmdUI$(e) &
		$(p)CDocTemplate$(e) &
		$(p)CDocument$(e) &
		$(p)CException$(e) &
		$(p)CFile$(e) &
		$(p)CFrameWnd$(e) &
		$(p)CGdiObject$(e) &
		$(p)CObject$(e) &
		$(p)CObjFromHandle$(e) &
		$(p)CPoint$(e) &
		$(p)CRect$(e) &
		$(p)CSize$(e) &
		$(p)CView$(e) &
		$(p)CWinApp$(e) &
		$(p)CWnd$(e) &
		$(p)ofcglobals$(e) &
		$(p)CString$(e)

#$(p)Debug$(e) &

# defines additional options for C compiler
ADD_COPT = -od -xs -i=..$(SEP)..$(SEP)Include -i=c:\watcom\h
ADD_LINKOPT = LIB commdlg.lib, ole2disp.lib

HEAPSIZE = 18k
STACKSIZE = 18k

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe

.ico: $(MYDIR)res
.cpp: $(MYDIR)..$(SEP)..$(SEP)src

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)hello.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
