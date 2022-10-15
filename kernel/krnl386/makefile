#
# A Makefile for KRNL386.EXE
# (c) osFree project,
#



PROJ = krnl386
TRGT = $(PROJ).exe
DESC = Windows/386 Kernel
srcfiles = $(p)dpmildr$(e) $(p)hmemset$(e) $(p)lstrcat$(e) $(p)lstrcpy$(e) $(p)lstrlen$(e) $(p)profstrg$(e) $(p)kernel16$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)lstrcpyn$(e) $(p)memset$(e) $(p)toupper$(e) $(p)strnicmp$(e) $(p)memcpy$(e) &
$(p)profile$(e) $(p)ansi$(e) $(p)tolower$(e) $(p)lstrcmp$(e) $(p)rom$(e)

ADD_ASMOPT = -D?32BIT=0 -D?DEBUG=0 -D?REAL=0 -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
ADD_COPT = -s -3 -I=$(%WATCOM)$(SEP)h$(SEP)win
JWASM = 1
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system


!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR)..
.c: $(MYDIR)..


