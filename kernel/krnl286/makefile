#
# A Makefile for KRNL286.EXE
# (c) osFree project,
#



PROJ = krnl286
TRGT = $(PROJ).exe
DESC = Windows/286 Kernel
srcfiles = $(p)ldboot$(e) $(p)hmemset$(e) $(p)profstrg$(e) $(p)kernel16$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)memset$(e) $(p)memcpy$(e) &
$(p)profile$(e) $(p)strings$(e) $(p)rom$(e) $(p)file$(e) $(p)main$(e) &
$(p)local$(e) $(p)catch$(e) $(p)nls$(e) &
$(p)mthread$(e) $(p)2protect$(e) $(p)2ginterf$(e) $(p)istask$(e)  $(p)GetExeVersion$(e)  $(p)longptradd$(e) $(p)ld$(e)

ADD_ASMOPT = -D?32BIT=0 -D?DEBUG=0 -D?REAL=0 -I=$(MYDIR).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
ADD_COPT = -s -2 -I=$(%WATCOM)$(SEP)h$(SEP)win
JWASM = 1
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR);$(MYDIR)..
.c: $(MYDIR)..


