/* rexx */

parse arg dir

call directory dir

call directory 'Include'

call rename 'ASCII.INC',      'ascii.inc'
call rename 'DDRAW.INC',      'ddraw.inc'
call rename 'DINPUT.INC',     'dinput.inc'
call rename 'DPMI.INC',       'dpmi.inc'
call rename 'DSOUND.INC',     'dsound.inc'
call rename 'EXCPT.INC',      'excpt.inc'
call rename 'FAT32.INC',      'fat32.inc'
call rename 'FIXUPS.INC',     'fixups.inc'
call rename 'FUNCTION.INC',   'function.inc'
call rename 'HEAP32.INC',     'heap32.inc'
call rename 'HXVDD.INC',      'hxvdd.inc'
call rename 'INITPM.INC',     'initpm.inc'
call rename 'INITPM16.INC',   'initpm16.inc'
call rename 'ISVBOP.INC',     'isvbop.inc'
call rename 'KEYBOARD.INC',   'keyboard.inc'
call rename 'MACROS.INC',     'macros.inc'
call rename 'MMSYSTEM.INC',   'mmsystem.inc'
call rename 'NT_VDD.INC',     'nt_vdd.inc'
call rename 'OBJBASE.INC',    'objbase.inc'
call rename 'OLEAUTO.INC',    'oleauto.inc'
call rename 'SB16.INC',       'sb16.inc'
call rename 'SHELLAPI.INC',   'shellapi.inc'
call rename 'STDDEFS.INC',    'stddefs.inc'
call rename 'TLHELP32.INC',   'tlhelp32.inc'
call rename 'VDDSVC.INC',     'vddsvc.inc'
call rename 'VESA32.H',       'vesa32.h'
call rename 'VESA32.INC',     'vesa32.inc'
call rename 'VK.INC',         'vk.inc'
call rename 'VWIN32.INC',     'vwin32.inc'
call rename 'WINBASE.INC',    'winbase.inc'
call rename 'WINCON.INC',     'wincon.inc'
call rename 'WINDEF.INC',     'windef.inc'
call rename 'WINERROR.INC',   'winerror.inc'
call rename 'WINGDI.INC',     'wingdi.inc'
call rename 'WINIOCTL.INC',   'winioctl.inc'
call rename 'WINNLS.INC',     'winnls.inc'
call rename 'WINNT.INC',      'winnt.inc'
call rename 'WINREG.INC',     'winreg.inc'
call rename 'WINSOCK.INC',    'winsock.inc'
call rename 'WINUSER.INC',    'winuser.inc'
call rename 'WNASPI32.INC',   'wnaspi32.inc'
call rename 'WTYPES.INC',     'wtypes.inc'

call directory '..'
call directory 'Src'

call directory 'HDPMI'

call rename 'A20GATE.ASM',      'a20gate.asm'
call rename 'CLIENTS.ASM',      'clients.asm'
call rename 'DEBUGSYS.INC',     'debugsys.inc'
call rename 'EXCEPT.ASM',       'except.asm'
call rename 'EXTERNAL.INC',     'external.inc'
call rename 'HDPMI.ASM',        'hdpmi.asm'
call rename 'HDPMI.INC',        'hdpmi.inc'
call rename 'HDPMI.TXT',        'hdpmi.txt'
call rename 'HDPMIAPI.TXT',     'hdpmiapi.txt'
call rename 'HDPMIHIS.TXT',     'hdpmihis.txt'
call rename 'HDPMIST.ASM',      'hdpmist.asm'
call rename 'HEAP.ASM',         'heap.asm'
call rename 'HELPERS.ASM',      'helpers.asm'
call rename 'I2FHDPMI.ASM',     'i2fhdpmi.asm'
call rename 'I31DEB.ASM',       'i31deb.asm'
call rename 'I31DOS.ASM',       'i31dos.asm'
call rename 'I31FPU.ASM',       'i31fpu.asm'
call rename 'I31INT.ASM',       'i31int.asm'
call rename 'I31MEM.ASM',       'i31mem.asm'
call rename 'I31SEL.ASM',       'i31sel.asm'
call rename 'I31SWT.ASM',       'i31swt.asm'
call rename 'INIT.ASM',         'init.asm'
call rename 'INT13API.ASM',     'int13api.asm'
call rename 'INT21API.ASM',     'int21api.asm'
call rename 'INT2FAPI.ASM',     'int2fapi.asm'
call rename 'INT2XAPI.ASM',     'int2xapi.asm'
call rename 'INT31API.ASM',     'int31api.asm'
call rename 'INT31API.INC',     'int31api.inc'
call rename 'INT33API.ASM',     'int33api.asm'
call rename 'INT41API.ASM',     'int41api.asm'
call rename 'INTXXAPI.ASM',     'intxxapi.asm'
call rename 'MODULES.INC',      'modules.inc'
call rename 'MOVEHIGH.ASM',     'movehigh.asm'
call rename 'PAGEMGR.ASM',      'pagemgr.asm'
call rename 'PAGEMGR.INC',      'pagemgr.inc'
call rename 'PUTCHR.ASM',       'putchr.asm'
call rename 'PUTCHRR.ASM',      'putchrr.asm'
call rename 'SWITCH.ASM',       'switch.asm'
call rename 'VERSION.INC',      'version.inc'
call rename 'VXD.ASM',          'vxd.asm'

call directory '..'
call directory '..'


exit 0
/* ------------------------------------------ */
rename: procedure
src = arg(1)
dst = arg(2)

parse source os .

if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32' | os = 'WIN64'
then MV = 'move'
else MV = 'mv'

ret = stream(src, 'c', 'query exists')

if ret \= '' then do
    MV' 'src' 'dst'1'
    MV' 'dst'1 'dst
end

return
/* ------------------------------------------ */
