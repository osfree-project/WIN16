#
# A Makefile for SETUP
# (c) osFree project,
#

PROJ      = setup
TRGT      = $(PROJ).exe
DESC      = osFree Windows Setup
srcfiles  = $(p)setup$(e) $(p)ui$(e) $(p)resources$(e) $(p)log$(e) $(p)help$(e) $(p)video$(e) $(p)dialog$(e) $(p)screens$(e) $(p)file_utils$(e) $(p)system_info$(e) $(p)setup_core$(e) $(p)section_list$(e) $(p)inf_parser$(e) $(p)file_list$(e) $(p)disk_list$(e) $(p)machine_list$(e) $(p)display_list$(e) $(p)mouse_list$(e) $(p)kbdtype_list$(e) $(p)kbdtable_list$(e) $(p)language_list$(e) $(p)network_list$(e)
ADD_LINKOPT = lib $(BLD)lib$(SEP)dos_shared.lib
DEST        = os2$(SEP)mdos$(SEP)winos2
STACKSIZE = 16384

!include $(%ROOT)tools/mk/appsdos.mk
