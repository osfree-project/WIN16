#
# A Makefile for lzexpand.dll
# (c) osFree project
#

DESCRIPTION     = Janus file expansion library
TARGET_CLASS    = LIBRARY
EXPORTS = LZCOPY          .1, &
          LZOPENFILE      .2, &
          LZINIT          .3, &
          LZSEEK          .4, &
          LZREAD          .5, &
          LZCLOSE         .6, &
          LZSTART         .7, &
          COPYLZFILE      .8, &
          LZDONE          .9, &
          GETEXPANDEDNAME .10

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/build.mk
