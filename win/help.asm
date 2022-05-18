HelpMsg:
	db	'osFree Windows loader'',0dh,0ah,0dh,0ah
	db	'WIN [/R] [/3] [/S] [/B] [/D:[F][S][V][X]]',0dh,0ah,0dh,0ah
	db	'   /?  Prints this instruction banner.',0dh,0ah
	db	'   /h  Synonym for the /? switch.',0dh,0ah
	db	'   /3  Starts Windows in 386 enhanced mode.',0dh,0ah
	db	'   /S  Starts Windows in standard mode.',0dh,0ah
	db	'   /2  Synonym for the /S switch.',0dh,0ah
	db	'   /R  Starts Windows in real mode.',0dh,0ah
	db	'   /B  Creates a file, BOOTLOG.TXT, that records system messages.',0dh,0ah
	db	'       generated during system startup (boot).',0dh,0ah
	db	'   /D  Used for troubleshooting when Windows does not start',0dh,0ah
	db	'       correctly.',0dh,0ah
	db	'   :F  Turns off 32-bit disk access. Equivalent to SYSTEM.INI [386enh]',0dh,0ah
	db	'       setting: 32BitDiskAccess=FALSE.',0dh,0ah
	db	'   :S  Specifies that Windows should not use ROM address space between',0dh,0ah
	db	'       F000:0000 and 1 MB for a break point. Equivalent to SYSTEM.INI',0dh,0ah
	db	'       [386enh] setting: SystemROMBreakPoint=FALSE.',0dh,0ah
	db	'   :V  Specifies that the ROM routine handles interrupts from the hard',0dh,0ah
	db	'       drive controller. Equivalent to SYSTEM.INI [386enh] setting:',0dh,0ah
	db	'       VirtualHDIRQ=FALSE.',0dh,0ah
	db	'   :X  Excludes all of the adapter area from the range of memory that',0dh,0ah
	db	'       Windows scans to find unused space. Equivalent to SYSTEM.INI',0dh,0ah
	db	'       [386enh] setting: EMMExclude=A000-FFFF.',0dh,0ah, '$'
