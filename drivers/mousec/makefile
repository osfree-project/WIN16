#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ      = mouse1
PROJ1     = mouse
TRGT      = $(PROJ1).drv
DESC      = VirtualBox Mouse driver
DEST      = os2$(SEP)mdos$(SEP)winos2$(SEP)system
# This is necessary; USER.EXE imports mouse functions using this module name
# We need a non-moveable segment to store our PS/2 BIOS callbacks
ADD_LINKOPT = OPTION MODNAME=MOUSE &
	 SEGMENT CALLBACKS FIXED SHARED

EXPORTS = &
	Inquire.1, &
	Enable.2, &
	Disable.3, &
	MouseGetIntVect.4

#defines object file names in format $(p)objname$(e)
srcfiles  = $(p)mousew16$(e) $(p)vbox$(e)

DLL=1
# defines additional options for C compiler
	# -bd to build DLL
	# -mc to use compact memory model (far data pointers, since ss != ds)
	# -zu for DLL calling convention (ss != ds)
	# -zc put constants on the code segment (cs)
	# -s to disable stack checks, since the runtime uses MessageBox() to abort (which we can't call from mouse.drv)

ADD_COPT  = -mc -3 -zu -zc -d


!include $(%ROOT)tools/mk/appsw16.mk
!include $(MYDIR)mouse.mk

TARGETS = $(PATH)$(PROJ1).drv

$(PATH)$(PROJ1).drv: $(PATH)$(PROJ).dll
 @$(CP) $(PATH)$(PROJ).dll $(PATH)$(PROJ1).drv $(BLACKHOLE)

.c: $(PORT_BASE)

.h: $(PORT_BASE)

