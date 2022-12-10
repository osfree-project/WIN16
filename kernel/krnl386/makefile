#
# A Makefile for KRNL386.EXE
# (c) osFree project,
#



PROJ = krnl386
TRGT = $(PROJ).exe
DESC = Windows/386 Kernel

srcfiles = $(p)ldboot$(e) $(p)hmemset$(e) $(p)profstrg$(e) $(p)kernel16$(e) $(p)global$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)memset$(e) $(p)memcpy$(e) $(p)resource$(e) &
$(p)profile$(e) $(p)strings$(e) $(p)rom$(e) $(p)file$(e) $(p)main$(e) &
$(p)local$(e) $(p)catch$(e) $(p)nls$(e) $(p)localc$(e) $(p)taskc$(e) &
$(p)mthread$(e) $(p)3protect$(e) $(p)3ginterf$(e) $(p)GetExeVersion$(e)  $(p)longptradd$(e) $(p)ld$(e) $(p)task$(e) 

# 32BIT must be 1 here, but some files not added from HXDPMI yet
ADD_ASMOPT = -D?32BIT=0 -D?DEBUG=0 -D?REAL=0 -I=$(MYDIR).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
ADD_COPT = -s -3 -I=$(%WATCOM)$(SEP)h$(SEP)win
JWASM = 1
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system


!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR);$(MYDIR)..
.c: $(MYDIR)..


