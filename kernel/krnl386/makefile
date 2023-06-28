#
# A Makefile for KRNL386.EXE
# (c) osFree project,
#



PROJ = krnl386
TRGT = $(PROJ).exe
DESC = Windows/386 Kernel

srcfiles = $(p)ldboot$(e) $(p)hmemset$(e) $(p)profstrg$(e) $(p)kernel16$(e) $(p)global$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)resource$(e) $(p)error$(e) &
$(p)profile$(e) $(p)strings$(e) $(p)rom$(e) $(p)file$(e) $(p)main$(e) $(p)local$(e) &
$(p)catch$(e) $(p)nls$(e) $(p)taskc$(e) $(p)enablec$(e) $(p)div$(e) &
$(p)mthread$(e) $(p)3protect$(e) $(p)protectc$(e) $(p)printf$(e) $(p)3ginterf$(e) $(p)selectorc$(e) $(p)task$(e)  $(p)ld$(e)

# 32BIT must be 1 here, but some files not added from HXDPMI yet
ADD_ASMOPT = -D?32BIT=0 -D?DEBUG=1 -D?REAL=0 -I=$(MYDIR).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
ADD_COPT = -s -3 -I=$(%WATCOM)$(SEP)h$(SEP)win
JWASM = 1
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system


!include $(%ROOT)tools/mk/appsdos.mk

.asm: $(MYDIR);$(MYDIR)..
.c: $(MYDIR)..


