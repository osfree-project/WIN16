#
# A Makefile for HDPMI16
# (c) osFree project,
# author, date
#

PROJ       = hdpmi16
TRGT       = $(PROJ).exe
DESC       = 286 MS-DOS Extender
srcfiles   = &
	$(p)HDPMI$(e) &
	$(p)A20GATE$(e) &
	$(p)CLIENTS$(e) &
	$(p)EXCEPT$(e) &
	$(p)HEAP$(e) &
	$(p)HELPERS$(e) &
	$(p)I2FHDPMI$(e) &
	$(p)I31DEB$(e) &
	$(p)I31DOS$(e) &
	$(p)I31FPU$(e) &
	$(p)I31INT$(e) &
	$(p)I31MEM$(e) &
	$(p)I31SEL$(e) &
	$(p)I31SWT$(e) &
	$(p)INIT$(e) &
	$(p)INT13API$(e) &
	$(p)INT21API$(e) &
	$(p)INT2FAPI$(e) &
	$(p)INT2XAPI$(e) &
	$(p)INT31API$(e) &
	$(p)INT33API$(e) &
	$(p)INT41API$(e) &
	$(p)INTXXAPI$(e) &
	$(p)MOVEHIGH$(e) &
	$(p)PAGEMGR$(e) &
	$(p)PUTCHR$(e) &
	$(p)PUTCHRR$(e) &
	$(p)SWITCH$(e) &
	$(p)VXD$(e)

JWASM      = 1
DEST       = os2$(SEP)mdos$(SEP)winos2
STACKSIZE = 0

!include $(%ROOT)tools/mk/appsdos.mk
!include $(MYDIR)hdpmi.mk

ADD_ASMOPT = -D?32BIT=0 -D?LFNSUPPORT=0 -D?HSINEXTMEM=0 -c -Cp -Sg -I=$(PORT_BASE)Include

.inc: $(PORT_BASE)Include;$(PORT_BASE)Src$(SEP)HDPMI

.asm: $(PORT_BASE)Src$(SEP)HDPMI

