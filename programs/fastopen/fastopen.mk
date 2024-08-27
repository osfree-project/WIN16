
PORT_NAME = win16$(SEP)fastopen
PORT_TYPE = git
PORT_URL  = https://github.com/microsoft/MS-DOS
PORT_REV  = main
PORT_PATCHES  = fastopen.diff

!include $(%ROOT)tools/mk/port.mk
