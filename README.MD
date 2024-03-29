## osFree Windows Kernel

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

### Dirs


    resources	Icons, bitmaps, cursors from TWIN, fonts from Wine
    dlls		Standard Windows DLLs
    doc			Some documentation. Mostly outdated and for reference only.
    dosx		286 DOS Extender
    drivers		Standard Windows drivers
    include		Include files. For reference only.
    kernel		KERNEL.EXE/KRNL286.EXE/KRNL386.EXE - new kernel
    mmsystem
    multimedia
    pal			Platform Abstration Layer - drivers for X11 (not used, for reference only)
    programs	Standard Windows programs. Mix of TWIN, Wine sources, as well as MS File Manager
    samples		Various Sample programs
    tests		WIN16 unit tests
    tools		various tools
    user    USER.EXE
    utilities
    win			WIN.COM - Windows loader
    winkrnl		TWIN Windows kernel, gdi, user. For reference only. Current kernel in kernel directory.
