#
# A Makefile for FASTOPEN
# (c) osFree project,
#

PROJ = fastopen
TRGT = $(PROJ).exe
DESC = File cache
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)fastopen$(e) $(p)fastseek$(e) $(p)fastinit$(e) $(p)fastp$(e) $(p)fastsm$(e)
# defines additional options for C compiler
#STUB     = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
UNI2H = 0
JWASM = 1


!include $(%ROOT)tools/mk/appsdos.mk
!include $(MYDIR)fastopen.mk

ADD_COPT = -i=$(PORT_BASE)v4.0$(SEP)src$(SEP)H
ADD_ASMOPT = -Zm -Zg -I=$(PORT_BASE)v4.0$(SEP)src$(SEP)INC
ADD_LINKOPT = op nocase

.c: $(PORT_BASE)v4.0$(SEP)src$(SEP)CMD$(SEP)FASTOPEN
.asm: $(PORT_BASE)v4.0$(SEP)src$(SEP)CMD$(SEP)FASTOPEN

.h: $(PORT_BASE)v4.0$(SEP)src$(SEP)CMD$(SEP)FASTOPEN
.h: $(PORT_BASE)v4.0$(SEP)src$(SEP)H
.inc: $(PORT_BASE)v4.0$(SEP)src$(SEP)INC

