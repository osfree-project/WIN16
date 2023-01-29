## osFree Windows Kernel

This is attempt to reimplement 16-bit Windows kernel for usage in VDM.
It is heavely reuses TWIN, WINE and HX-DOS. As well as Mett Pietrek
"Windows Internals" and Andrew Schulman "Undocumented Windows" book
heavy used to reconstruct Kernel internal structure.

### Design

Because original OS/2 uses mostly unchanged Windows so you can consider
this is reimplementation of vanilla Windows 3.x. This means most of
code is written to start without any underline system, like Win32 or
XFree or something like this. KRNL386/386 must start from pure DOS and
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
