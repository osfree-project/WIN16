#
# A Makefile for HDPMI16
# (c) osFree project,
# author, date
#

PROJ       = hdpmi16
TRGT       = $(PROJ).exe
DESC       = 286 MS-DOS Extender
srcfiles   = &
	$(p)hdpmi$(e) &
	$(p)a20gate$(e) &
	$(p)clients$(e) &
	$(p)except$(e) &
	$(p)heap$(e) &
	$(p)helpers$(e) &
	$(p)i2fhdpmi$(e) &
	$(p)i31deb$(e) &
	$(p)i31dos$(e) &
	$(p)i31fpu$(e) &
	$(p)i31int$(e) &
	$(p)i31mem$(e) &
	$(p)i31sel$(e) &
	$(p)i31swt$(e) &
	$(p)init$(e) &
	$(p)int13api$(e) &
	$(p)int21api$(e) &
	$(p)int2fapi$(e) &
	$(p)int2xapi$(e) &
	$(p)int31api$(e) &
	$(p)int33api$(e) &
	$(p)int41api$(e) &
	$(p)intxxapi$(e) &
	$(p)movehigh$(e) &
	$(p)pagemgr$(e) &
	$(p)putchr$(e) &
	$(p)putchrr$(e) &
	$(p)switch$(e) &
	$(p)vxd$(e)

JWASM      = 1
DEST       = os2$(SEP)mdos$(SEP)winos2$(SEP)system
STACKSIZE = 0

!include $(%ROOT)tools/mk/appsdos.mk
!include $(MYDIR)hdpmi.mk

ADD_ASMOPT = -D?32BIT=0 -D?LFNSUPPORT=0 -D?HSINEXTMEM=0 -c -Cp -Sg -I=$(PORT_BASE)Include

.inc: $(PORT_BASE)Include$(PS)$(PORT_BASE)Src$(SEP)HDPMI

.asm: $(PORT_BASE)Src$(SEP)HDPMI

