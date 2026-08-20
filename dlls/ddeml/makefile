#
# A Makefile for lzexpand.dll
# (c) osFree project
#

DESCRIPTION = Janus file expansion library
TARGET_CLASS = LIBRARY
TARGET_VERSION = 310

EXPORTS = 	DdeInitialize		.2, &
	DdeUninitialize		.3, &
	DdePostAdvise		.13, &
	DdeCreateStringHandle 	.21, &
	DdeFreeStringHandle 	.22, &
	DdeKeepStringHandle 	.24, &
	DdeEnableCallback 	.26, &
	DdeNameService 		.27


DESTINATION = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/build.mk

