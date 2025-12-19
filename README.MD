## osFree Windows Kernel

WIN16 Kernel for Windows 3.0 Compatibility - A part of the osFree OS/2 project. This is an open-source reimplementation of the Windows 3.0 kernel.

This is attempt to reimplement 16-bit Windows kernel for usage in VDM.
It is heavely reuses TWIN, WINE and HX-DOS. As well as Mett Pietrek
"Windows Internals" and Andrew Schulman "Undocumented Windows" book
heavy used to reconstruct Kernel internal structure.

### Design

Because original OS/2 uses mostly unchanged Windows so you can consider
this is reimplementation of vanilla Windows 3.x. This means most of
code is written to start without any underline system, like Win32 or
XFree or something like this. KRNL286/386 must start from pure DOS and
start working.

Most of initialization code and module manager are from HX-DOS. HX-DOS
is a staring base for implementation.

Most other API functions taken from Wine and TWIN, depending on how hard to
port to pure 16-bit environment. Many functions ported back to pure DOS 16-bit
environment.

### Status

Very-very alpha development stage. GDI and USER not handled at all. Most of higher
level dll also not implemented.

First goal is to correctly start simple program like WinVer or something like this.
This means KERNEL must correctly load and initialise drivers (system, mouse, keyboard)
and GDI.EXE with USER.EXE.

Target Windows version is 3.0

### Dirs

    applications	Standard Windows applications. Mix of TWIN, Wine, ReactOS sources, as well as MS File Manager
    resources		Icons, bitmaps, cursors from TWIN, fonts from Wine
    dlls		Standard Windows DLLs
    docs		Documentation which comes with distibution
    dosx		286 DPMI Host and Extender (not implemented yet, but HXDOS as base)
    drivers		Standard Windows/DOS drivers
    include		Include files. For reference only. OpenWatcom header is used.
    kernel		KERNEL.EXE/KRNL286.EXE/KRNL386.EXE - new kernel
    MME			Multimedia Extensions
    pal			Platform Abstration Layer - drivers for X11 (not used, for reference only)
    samples		Various Sample programs
    tests		WIN16 unit tests
    user		USER.EXE
    utilities	
    win			WIN.COM - Windows loader
    winkrnl		TWIN Windows kernel, gdi, user. For reference only. Current kernel in kernel directory.
