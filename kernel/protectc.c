#include <win16.h>

extern void _cdecl printf (const char *format,...);

void WINAPI Copyright()
{
	printf("\nosFree Windows Kernel version 0.1\n" 
		   "Copyright (C) 2022 osFree\n"
		   "Based on HX DPMI loader, WINE and TWIN\n"
		   "Copyright (C) 1993-2022 Japheth\n"
		   "Copyright (C) 1993-2022 the Wine project authors\n"
		   "Copyright (C) 1997 Willows Software, Inc.\n"
		   "\n\r");
}

void WINAPI DumpDPMIInfo()
{
	printf("234\n\r");
}
