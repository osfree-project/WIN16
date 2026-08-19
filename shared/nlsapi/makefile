#
# Makefile for common shared libs
#

TARGET_CLASS = LIBRARY
TARGET_SUBCLASS = STATIC
SOURCES = nlsapi_enum nlsapi_locale nlsapi_main nlsapi_utils nlsapi_num nlsapi_currency nlsapi_keyboard
ADD_COPT = -I=$(MYDIR)..$(SEP)..$(SEP)include -DSTATIC
LIBS = setupinf

!include $(%ROOT)tools/mk/build.mk

