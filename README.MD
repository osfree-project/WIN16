====== osFree Janus: Open-Source Windows 3.0 Clone ======

**osFree Janus** is a project to create a complete, open-source,
compatible reimplementation of the classic 16-bit Microsoft
Windows 3.0 operating system. This is a full clone of the 
Windows 3.0 environment, not just its kernel, developed as
part of the osFree project. The goal is to build an independent,
modern reimplementation of historical Windows, including the
kernel (KERNEL), graphics subsystem (GDI, USER), system drivers,
and core components, capable of running both standalone on "pure"
DOS and within a Virtual DOS Machine (VDM) session.

This project is an attempt to reimplement the 16-bit Windows
kernel for use in VDM and standalone environments. Development
heavily utilizes code from the **TWIN**, **WINE**, **ReactOS**,
and **HX-DOS** projects. The reconstruction of the internal
kernel and system structure relies significantly on 
**Matt Pietrek's "Windows Internals"**, **Andrew Schulman's "Undocumented Windows"**,
and **Daniel Norton's Writing Windows Device Drivers** books.

===== Design and Architecture =====

The original OS/2 used a largely unmodified Windows, so this
project can be considered a reimplementation of classic,
"vanilla" Windows 3.x. This means most code is written to
run without any underlying system (e.g., Win32 or XFree).
The **KRNL286/386** kernels must start from pure DOS and
operate autonomously.

Most initialization code and the module manager are derived
from **HX-DOS**, which serves as the starting implementation base.

Most other API functions are taken from **Wine** and **TWIN**,
depending on the difficulty of porting to a pure 16-bit environment.
Many functions have been ported back to a pure DOS 16-bit environment.

===== Project Status =====

The project is at a **very early alpha development stage**.
The graphics subsystem (GDI) are not yet implemented. 
User interface library (USER) now has lot of working stuffs.
Most higher-level DLLs are also implemented on some limited level.

**The first goal** is to correctly launch a simple system
application, such as **WinVer**, **Clock** and so on.
This requires the kernel (KERNEL) to correctly load
and initialize drivers (system, mouse, keyboard) as
well as **GDI.EXE** and **USER.EXE**.

**The target Windows version is 3.0.**

===== Project Structure =====

The project includes reimplementations of all key Windows 3.0 system components:

^ Directory       ^ Description ^
| applications    | Standard Windows applications. A mix of sources from TWIN, Wine, ReactOS, and MS File Manager. |
| resources       | Icons, bitmaps, cursors from TWIN, fonts from Wine. |
| dlls            | Standard Windows DLLs. |
| docs            | Documentation included with the distribution. |
| dosx            | 286 DPMI Host and Extender (not yet implemented, HXDOS used as a base). |
| drivers         | Standard Windows/DOS drivers. |
| include         | Include files. For reference only. OpenWatcom headers are used. |
| kernel          | KERNEL.EXE/KRNL286.EXE/KRNL386.EXE — the new kernel. |
| MME             | Multimedia Extensions. |
| pal             | Platform Abstraction Layer — drivers for X11 (not used, for reference only). |
| samples         | Various sample programs. |
| tests           | WIN16 unit tests. |
| user            | USER.EXE. |
| utilities       | Utility programs. |
| win             | WIN.COM — Windows loader. |
| winkrnl         | Windows kernel, gdi, user from the TWIN project. For reference only. The current kernel is in the `kernel` directory. |

===== Repository and Keywords =====

**Project Repository:** https://github.com/osfree-project/WIN16

**Keywords:** Windows 3.0 clone, Windows 3.0 implementation, Windows 3.0 reimplementation, Open-source Windows 3.0, Win16, KRNL286, KRNL386, 16-bit Windows, osFree, Windows 3.0 compatibility, Run old Windows programs, DOS Windows, Windows 3.0 repository.
