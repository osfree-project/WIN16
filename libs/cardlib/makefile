#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ      = cards
TRGT      = $(PROJ).lib
ADD_COPT  = -i=$(MYDIR)..$(SEP)include
srcfiles = $(p)cardbitmaps$(e) &
		$(p)cardcolor$(e) &
		$(p)cardcount$(e) &
		$(p)cardlib$(e) &
		$(p)cardrgndraw$(e) &
		$(p)cardstack$(e) &
		$(p)cardbutton$(e) &
		$(p)cardregion$(e) &
		$(p)cardrgnmouse$(e) &
		$(p)cardwindow$(e) &
		$(p)dropzone$(e)

TARGETS  = $(PATH)$(TRGT)

!include $(%ROOT)tools/mk/libsw16.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
