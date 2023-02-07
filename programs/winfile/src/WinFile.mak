# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=WinFile - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to WinFile - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "WinFile - Win32 Release" && "$(CFG)" !=\
 "WinFile - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinFile.mak" CFG="WinFile - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinFile - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WinFile - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "WinFile - Win32 Debug"
RSC=rc.exe
CPP=cl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "WinFile - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\WinFile.exe" "$(OUTDIR)\WinFile.bsc"

CLEAN : 
	-@erase "$(INTDIR)\dbg.obj"
	-@erase "$(INTDIR)\dbg.sbr"
	-@erase "$(INTDIR)\lfn.obj"
	-@erase "$(INTDIR)\lfn.sbr"
	-@erase "$(INTDIR)\lfnmisc.obj"
	-@erase "$(INTDIR)\lfnmisc.sbr"
	-@erase "$(INTDIR)\numfmt.obj"
	-@erase "$(INTDIR)\numfmt.sbr"
	-@erase "$(INTDIR)\res.res"
	-@erase "$(INTDIR)\suggest.obj"
	-@erase "$(INTDIR)\suggest.sbr"
	-@erase "$(INTDIR)\tbar.obj"
	-@erase "$(INTDIR)\tbar.sbr"
	-@erase "$(INTDIR)\treectl.obj"
	-@erase "$(INTDIR)\treectl.sbr"
	-@erase "$(INTDIR)\wfassoc.obj"
	-@erase "$(INTDIR)\wfassoc.sbr"
	-@erase "$(INTDIR)\wfchgnot.obj"
	-@erase "$(INTDIR)\wfchgnot.sbr"
	-@erase "$(INTDIR)\wfcomman.obj"
	-@erase "$(INTDIR)\wfcomman.sbr"
	-@erase "$(INTDIR)\wfcopy.obj"
	-@erase "$(INTDIR)\wfcopy.sbr"
	-@erase "$(INTDIR)\wfdir.obj"
	-@erase "$(INTDIR)\wfdir.sbr"
	-@erase "$(INTDIR)\wfdirrd.obj"
	-@erase "$(INTDIR)\wfdirrd.sbr"
	-@erase "$(INTDIR)\wfdirsrc.obj"
	-@erase "$(INTDIR)\wfdirsrc.sbr"
	-@erase "$(INTDIR)\wfdlgs.obj"
	-@erase "$(INTDIR)\wfdlgs.sbr"
	-@erase "$(INTDIR)\wfdlgs2.obj"
	-@erase "$(INTDIR)\wfdlgs2.sbr"
	-@erase "$(INTDIR)\wfdlgs3.obj"
	-@erase "$(INTDIR)\wfdlgs3.sbr"
	-@erase "$(INTDIR)\wfdos.obj"
	-@erase "$(INTDIR)\wfdos.sbr"
	-@erase "$(INTDIR)\wfdrives.obj"
	-@erase "$(INTDIR)\wfdrives.sbr"
	-@erase "$(INTDIR)\wfext.obj"
	-@erase "$(INTDIR)\wfext.sbr"
	-@erase "$(INTDIR)\wffile.obj"
	-@erase "$(INTDIR)\wffile.sbr"
	-@erase "$(INTDIR)\wfinfo.obj"
	-@erase "$(INTDIR)\wfinfo.sbr"
	-@erase "$(INTDIR)\wfinit.obj"
	-@erase "$(INTDIR)\wfinit.sbr"
	-@erase "$(INTDIR)\wfmem.obj"
	-@erase "$(INTDIR)\wfmem.sbr"
	-@erase "$(INTDIR)\wfprint.obj"
	-@erase "$(INTDIR)\wfprint.sbr"
	-@erase "$(INTDIR)\wfsearch.obj"
	-@erase "$(INTDIR)\wfsearch.sbr"
	-@erase "$(INTDIR)\wftree.obj"
	-@erase "$(INTDIR)\wftree.sbr"
	-@erase "$(INTDIR)\wfutil.obj"
	-@erase "$(INTDIR)\wfutil.sbr"
	-@erase "$(INTDIR)\winfile.obj"
	-@erase "$(INTDIR)\winfile.sbr"
	-@erase "$(INTDIR)\wnetcaps.obj"
	-@erase "$(INTDIR)\wnetcaps.sbr"
	-@erase "$(OUTDIR)\WinFile.bsc"
	-@erase "$(OUTDIR)\WinFile.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gz /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "FASTMOVE" /D "LFNCLIPBOARD" /D "MEMDOUBLE" /D "STRICT" /D "UNICODE" /D "_UNICODE" /D "USELASTDOT" /D _WIN32_WINNT=0x0400 /D WINVER=0x0400 /FR /YX /c
CPP_PROJ=/nologo /Gz /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "FASTMOVE" /D "LFNCLIPBOARD" /D "MEMDOUBLE" /D "STRICT" /D "UNICODE" /D\
 "_UNICODE" /D "USELASTDOT" /D _WIN32_WINNT=0x0400 /D WINVER=0x0400\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/WinFile.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/res.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/WinFile.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dbg.sbr" \
	"$(INTDIR)\lfn.sbr" \
	"$(INTDIR)\lfnmisc.sbr" \
	"$(INTDIR)\numfmt.sbr" \
	"$(INTDIR)\suggest.sbr" \
	"$(INTDIR)\tbar.sbr" \
	"$(INTDIR)\treectl.sbr" \
	"$(INTDIR)\wfassoc.sbr" \
	"$(INTDIR)\wfchgnot.sbr" \
	"$(INTDIR)\wfcomman.sbr" \
	"$(INTDIR)\wfcopy.sbr" \
	"$(INTDIR)\wfdir.sbr" \
	"$(INTDIR)\wfdirrd.sbr" \
	"$(INTDIR)\wfdirsrc.sbr" \
	"$(INTDIR)\wfdlgs.sbr" \
	"$(INTDIR)\wfdlgs2.sbr" \
	"$(INTDIR)\wfdlgs3.sbr" \
	"$(INTDIR)\wfdos.sbr" \
	"$(INTDIR)\wfdrives.sbr" \
	"$(INTDIR)\wfext.sbr" \
	"$(INTDIR)\wffile.sbr" \
	"$(INTDIR)\wfinfo.sbr" \
	"$(INTDIR)\wfinit.sbr" \
	"$(INTDIR)\wfmem.sbr" \
	"$(INTDIR)\wfprint.sbr" \
	"$(INTDIR)\wfsearch.sbr" \
	"$(INTDIR)\wftree.sbr" \
	"$(INTDIR)\wfutil.sbr" \
	"$(INTDIR)\winfile.sbr" \
	"$(INTDIR)\wnetcaps.sbr"

"$(OUTDIR)\WinFile.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/WinFile.pdb" /machine:I386 /out:"$(OUTDIR)/WinFile.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dbg.obj" \
	"$(INTDIR)\lfn.obj" \
	"$(INTDIR)\lfnmisc.obj" \
	"$(INTDIR)\numfmt.obj" \
	"$(INTDIR)\res.res" \
	"$(INTDIR)\suggest.obj" \
	"$(INTDIR)\tbar.obj" \
	"$(INTDIR)\treectl.obj" \
	"$(INTDIR)\wfassoc.obj" \
	"$(INTDIR)\wfchgnot.obj" \
	"$(INTDIR)\wfcomman.obj" \
	"$(INTDIR)\wfcopy.obj" \
	"$(INTDIR)\wfdir.obj" \
	"$(INTDIR)\wfdirrd.obj" \
	"$(INTDIR)\wfdirsrc.obj" \
	"$(INTDIR)\wfdlgs.obj" \
	"$(INTDIR)\wfdlgs2.obj" \
	"$(INTDIR)\wfdlgs3.obj" \
	"$(INTDIR)\wfdos.obj" \
	"$(INTDIR)\wfdrives.obj" \
	"$(INTDIR)\wfext.obj" \
	"$(INTDIR)\wffile.obj" \
	"$(INTDIR)\wfinfo.obj" \
	"$(INTDIR)\wfinit.obj" \
	"$(INTDIR)\wfmem.obj" \
	"$(INTDIR)\wfprint.obj" \
	"$(INTDIR)\wfsearch.obj" \
	"$(INTDIR)\wftree.obj" \
	"$(INTDIR)\wfutil.obj" \
	"$(INTDIR)\winfile.obj" \
	"$(INTDIR)\wnetcaps.obj"

"$(OUTDIR)\WinFile.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\WinFile.exe"

CLEAN : 
	-@erase "$(INTDIR)\dbg.obj"
	-@erase "$(INTDIR)\lfn.obj"
	-@erase "$(INTDIR)\lfnmisc.obj"
	-@erase "$(INTDIR)\numfmt.obj"
	-@erase "$(INTDIR)\res.res"
	-@erase "$(INTDIR)\suggest.obj"
	-@erase "$(INTDIR)\tbar.obj"
	-@erase "$(INTDIR)\treectl.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\wfassoc.obj"
	-@erase "$(INTDIR)\wfchgnot.obj"
	-@erase "$(INTDIR)\wfcomman.obj"
	-@erase "$(INTDIR)\wfcopy.obj"
	-@erase "$(INTDIR)\wfdir.obj"
	-@erase "$(INTDIR)\wfdirrd.obj"
	-@erase "$(INTDIR)\wfdirsrc.obj"
	-@erase "$(INTDIR)\wfdlgs.obj"
	-@erase "$(INTDIR)\wfdlgs2.obj"
	-@erase "$(INTDIR)\wfdlgs3.obj"
	-@erase "$(INTDIR)\wfdos.obj"
	-@erase "$(INTDIR)\wfdrives.obj"
	-@erase "$(INTDIR)\wfext.obj"
	-@erase "$(INTDIR)\wffile.obj"
	-@erase "$(INTDIR)\wfinfo.obj"
	-@erase "$(INTDIR)\wfinit.obj"
	-@erase "$(INTDIR)\wfmem.obj"
	-@erase "$(INTDIR)\wfprint.obj"
	-@erase "$(INTDIR)\wfsearch.obj"
	-@erase "$(INTDIR)\wftree.obj"
	-@erase "$(INTDIR)\wfutil.obj"
	-@erase "$(INTDIR)\winfile.obj"
	-@erase "$(INTDIR)\wnetcaps.obj"
	-@erase "$(OUTDIR)\WinFile.exe"
	-@erase "$(OUTDIR)\WinFile.ilk"
	-@erase "$(OUTDIR)\WinFile.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gz /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "FASTMOVE" /D "LFNCLIPBOARD" /D "MEMDOUBLE" /D "STRICT" /D "UNICODE" /D "_UNICODE" /D "USELASTDOT" /D _WIN32_WINNT=0x0400 /D WINVER=0x0400 /YX /c
CPP_PROJ=/nologo /Gz /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "FASTMOVE" /D "LFNCLIPBOARD" /D "MEMDOUBLE" /D "STRICT" /D\
 "UNICODE" /D "_UNICODE" /D "USELASTDOT" /D _WIN32_WINNT=0x0400 /D WINVER=0x0400\
 /Fp"$(INTDIR)/WinFile.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/res.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/WinFile.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/WinFile.pdb" /debug /machine:I386 /out:"$(OUTDIR)/WinFile.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dbg.obj" \
	"$(INTDIR)\lfn.obj" \
	"$(INTDIR)\lfnmisc.obj" \
	"$(INTDIR)\numfmt.obj" \
	"$(INTDIR)\res.res" \
	"$(INTDIR)\suggest.obj" \
	"$(INTDIR)\tbar.obj" \
	"$(INTDIR)\treectl.obj" \
	"$(INTDIR)\wfassoc.obj" \
	"$(INTDIR)\wfchgnot.obj" \
	"$(INTDIR)\wfcomman.obj" \
	"$(INTDIR)\wfcopy.obj" \
	"$(INTDIR)\wfdir.obj" \
	"$(INTDIR)\wfdirrd.obj" \
	"$(INTDIR)\wfdirsrc.obj" \
	"$(INTDIR)\wfdlgs.obj" \
	"$(INTDIR)\wfdlgs2.obj" \
	"$(INTDIR)\wfdlgs3.obj" \
	"$(INTDIR)\wfdos.obj" \
	"$(INTDIR)\wfdrives.obj" \
	"$(INTDIR)\wfext.obj" \
	"$(INTDIR)\wffile.obj" \
	"$(INTDIR)\wfinfo.obj" \
	"$(INTDIR)\wfinit.obj" \
	"$(INTDIR)\wfmem.obj" \
	"$(INTDIR)\wfprint.obj" \
	"$(INTDIR)\wfsearch.obj" \
	"$(INTDIR)\wftree.obj" \
	"$(INTDIR)\wfutil.obj" \
	"$(INTDIR)\winfile.obj" \
	"$(INTDIR)\wnetcaps.obj"

"$(OUTDIR)\WinFile.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "WinFile - Win32 Release"
# Name "WinFile - Win32 Debug"

!IF  "$(CFG)" == "WinFile - Win32 Release"

!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\dbg.c
DEP_CPP_DBG_C=\
	".\dbg.h"\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_DBG_C=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\dbg.obj" : $(SOURCE) $(DEP_CPP_DBG_C) "$(INTDIR)"

"$(INTDIR)\dbg.sbr" : $(SOURCE) $(DEP_CPP_DBG_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\dbg.obj" : $(SOURCE) $(DEP_CPP_DBG_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lfn.c
DEP_CPP_LFN_C=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_LFN_C=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\lfn.obj" : $(SOURCE) $(DEP_CPP_LFN_C) "$(INTDIR)"

"$(INTDIR)\lfn.sbr" : $(SOURCE) $(DEP_CPP_LFN_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\lfn.obj" : $(SOURCE) $(DEP_CPP_LFN_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lfnmisc.c
DEP_CPP_LFNMI=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_LFNMI=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\lfnmisc.obj" : $(SOURCE) $(DEP_CPP_LFNMI) "$(INTDIR)"

"$(INTDIR)\lfnmisc.sbr" : $(SOURCE) $(DEP_CPP_LFNMI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\lfnmisc.obj" : $(SOURCE) $(DEP_CPP_LFNMI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\numfmt.c
DEP_CPP_NUMFM=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_NUMFM=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\numfmt.obj" : $(SOURCE) $(DEP_CPP_NUMFM) "$(INTDIR)"

"$(INTDIR)\numfmt.sbr" : $(SOURCE) $(DEP_CPP_NUMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\numfmt.obj" : $(SOURCE) $(DEP_CPP_NUMFM) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\suggest.c
DEP_CPP_SUGGE=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.db"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_SUGGE=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\suggest.obj" : $(SOURCE) $(DEP_CPP_SUGGE) "$(INTDIR)"

"$(INTDIR)\suggest.sbr" : $(SOURCE) $(DEP_CPP_SUGGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\suggest.obj" : $(SOURCE) $(DEP_CPP_SUGGE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tbar.c
DEP_CPP_TBAR_=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_TBAR_=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\tbar.obj" : $(SOURCE) $(DEP_CPP_TBAR_) "$(INTDIR)"

"$(INTDIR)\tbar.sbr" : $(SOURCE) $(DEP_CPP_TBAR_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\tbar.obj" : $(SOURCE) $(DEP_CPP_TBAR_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\treectl.c
DEP_CPP_TREEC=\
	".\dbg.h"\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\treectl.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_TREEC=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\treectl.obj" : $(SOURCE) $(DEP_CPP_TREEC) "$(INTDIR)"

"$(INTDIR)\treectl.sbr" : $(SOURCE) $(DEP_CPP_TREEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\treectl.obj" : $(SOURCE) $(DEP_CPP_TREEC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfassoc.c
DEP_CPP_WFASS=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFASS=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfassoc.obj" : $(SOURCE) $(DEP_CPP_WFASS) "$(INTDIR)"

"$(INTDIR)\wfassoc.sbr" : $(SOURCE) $(DEP_CPP_WFASS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfassoc.obj" : $(SOURCE) $(DEP_CPP_WFASS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfchgnot.c
DEP_CPP_WFCHG=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFCHG=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfchgnot.obj" : $(SOURCE) $(DEP_CPP_WFCHG) "$(INTDIR)"

"$(INTDIR)\wfchgnot.sbr" : $(SOURCE) $(DEP_CPP_WFCHG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfchgnot.obj" : $(SOURCE) $(DEP_CPP_WFCHG) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfcomman.c
DEP_CPP_WFCOM=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	".\wnetcaps.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFCOM=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfcomman.obj" : $(SOURCE) $(DEP_CPP_WFCOM) "$(INTDIR)"

"$(INTDIR)\wfcomman.sbr" : $(SOURCE) $(DEP_CPP_WFCOM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfcomman.obj" : $(SOURCE) $(DEP_CPP_WFCOM) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfcopy.c
DEP_CPP_WFCOP=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFCOP=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfcopy.obj" : $(SOURCE) $(DEP_CPP_WFCOP) "$(INTDIR)"

"$(INTDIR)\wfcopy.sbr" : $(SOURCE) $(DEP_CPP_WFCOP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfcopy.obj" : $(SOURCE) $(DEP_CPP_WFCOP) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfdir.c
DEP_CPP_WFDIR=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFDIR=\
	".\heap.h"\
	".\machine.h"\
	".\wficon.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfdir.obj" : $(SOURCE) $(DEP_CPP_WFDIR) "$(INTDIR)"

"$(INTDIR)\wfdir.sbr" : $(SOURCE) $(DEP_CPP_WFDIR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfdir.obj" : $(SOURCE) $(DEP_CPP_WFDIR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfdirrd.c
DEP_CPP_WFDIRR=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFDIRR=\
	".\heap.h"\
	".\machine.h"\
	".\wficon.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfdirrd.obj" : $(SOURCE) $(DEP_CPP_WFDIRR) "$(INTDIR)"

"$(INTDIR)\wfdirrd.sbr" : $(SOURCE) $(DEP_CPP_WFDIRR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfdirrd.obj" : $(SOURCE) $(DEP_CPP_WFDIRR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfdirsrc.c
DEP_CPP_WFDIRS=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFDIRS=\
	".\heap.h"\
	".\machine.h"\
	".\wficon.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfdirsrc.obj" : $(SOURCE) $(DEP_CPP_WFDIRS) "$(INTDIR)"

"$(INTDIR)\wfdirsrc.sbr" : $(SOURCE) $(DEP_CPP_WFDIRS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfdirsrc.obj" : $(SOURCE) $(DEP_CPP_WFDIRS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfdlgs2.c
DEP_CPP_WFDLG=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	".\wnetcaps.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFDLG=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfdlgs2.obj" : $(SOURCE) $(DEP_CPP_WFDLG) "$(INTDIR)"

"$(INTDIR)\wfdlgs2.sbr" : $(SOURCE) $(DEP_CPP_WFDLG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfdlgs2.obj" : $(SOURCE) $(DEP_CPP_WFDLG) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfdlgs3.c
DEP_CPP_WFDLGS=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFDLGS=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfdlgs3.obj" : $(SOURCE) $(DEP_CPP_WFDLGS) "$(INTDIR)"

"$(INTDIR)\wfdlgs3.sbr" : $(SOURCE) $(DEP_CPP_WFDLGS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfdlgs3.obj" : $(SOURCE) $(DEP_CPP_WFDLGS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfdlgs.c
DEP_CPP_WFDLGS_=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFDLGS_=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfdlgs.obj" : $(SOURCE) $(DEP_CPP_WFDLGS_) "$(INTDIR)"

"$(INTDIR)\wfdlgs.sbr" : $(SOURCE) $(DEP_CPP_WFDLGS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfdlgs.obj" : $(SOURCE) $(DEP_CPP_WFDLGS_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfdos.c
DEP_CPP_WFDOS=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFDOS=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfdos.obj" : $(SOURCE) $(DEP_CPP_WFDOS) "$(INTDIR)"

"$(INTDIR)\wfdos.sbr" : $(SOURCE) $(DEP_CPP_WFDOS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfdos.obj" : $(SOURCE) $(DEP_CPP_WFDOS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfdrives.c
DEP_CPP_WFDRI=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\treectl.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFDRI=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfdrives.obj" : $(SOURCE) $(DEP_CPP_WFDRI) "$(INTDIR)"

"$(INTDIR)\wfdrives.sbr" : $(SOURCE) $(DEP_CPP_WFDRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfdrives.obj" : $(SOURCE) $(DEP_CPP_WFDRI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfext.c
DEP_CPP_WFEXT=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFEXT=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfext.obj" : $(SOURCE) $(DEP_CPP_WFEXT) "$(INTDIR)"

"$(INTDIR)\wfext.sbr" : $(SOURCE) $(DEP_CPP_WFEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfext.obj" : $(SOURCE) $(DEP_CPP_WFEXT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wffile.c
DEP_CPP_WFFIL=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\treectl.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFFIL=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wffile.obj" : $(SOURCE) $(DEP_CPP_WFFIL) "$(INTDIR)"

"$(INTDIR)\wffile.sbr" : $(SOURCE) $(DEP_CPP_WFFIL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wffile.obj" : $(SOURCE) $(DEP_CPP_WFFIL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfinfo.c

!IF  "$(CFG)" == "WinFile - Win32 Release"

DEP_CPP_WFINF=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	".\wnetcaps.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFINF=\
	".\heap.h"\
	".\machine.h"\
	

"$(INTDIR)\wfinfo.obj" : $(SOURCE) $(DEP_CPP_WFINF) "$(INTDIR)"

"$(INTDIR)\wfinfo.sbr" : $(SOURCE) $(DEP_CPP_WFINF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"

DEP_CPP_WFINF=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	".\wnetcaps.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFINF=\
	".\heap.h"\
	".\machine.h"\
	

"$(INTDIR)\wfinfo.obj" : $(SOURCE) $(DEP_CPP_WFINF) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfinit.c
DEP_CPP_WFINI=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	".\wnetcaps.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFINI=\
	".\heap.h"\
	".\machine.h"\
	".\wficon.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfinit.obj" : $(SOURCE) $(DEP_CPP_WFINI) "$(INTDIR)"

"$(INTDIR)\wfinit.sbr" : $(SOURCE) $(DEP_CPP_WFINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfinit.obj" : $(SOURCE) $(DEP_CPP_WFINI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfmem.c
DEP_CPP_WFMEM=\
	".\wfmem.h"\
	
NODEP_CPP_WFMEM=\
	".\heap.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfmem.obj" : $(SOURCE) $(DEP_CPP_WFMEM) "$(INTDIR)"

"$(INTDIR)\wfmem.sbr" : $(SOURCE) $(DEP_CPP_WFMEM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfmem.obj" : $(SOURCE) $(DEP_CPP_WFMEM) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfprint.c
DEP_CPP_WFPRI=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFPRI=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfprint.obj" : $(SOURCE) $(DEP_CPP_WFPRI) "$(INTDIR)"

"$(INTDIR)\wfprint.sbr" : $(SOURCE) $(DEP_CPP_WFPRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfprint.obj" : $(SOURCE) $(DEP_CPP_WFPRI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfsearch.c
DEP_CPP_WFSEA=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFSEA=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfsearch.obj" : $(SOURCE) $(DEP_CPP_WFSEA) "$(INTDIR)"

"$(INTDIR)\wfsearch.sbr" : $(SOURCE) $(DEP_CPP_WFSEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfsearch.obj" : $(SOURCE) $(DEP_CPP_WFSEA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wftree.c
DEP_CPP_WFTRE=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfcopy.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFTRE=\
	".\heap.h"\
	".\machine.h"\
	".\wficon.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wftree.obj" : $(SOURCE) $(DEP_CPP_WFTRE) "$(INTDIR)"

"$(INTDIR)\wftree.sbr" : $(SOURCE) $(DEP_CPP_WFTRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wftree.obj" : $(SOURCE) $(DEP_CPP_WFTRE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wfutil.c
DEP_CPP_WFUTI=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WFUTI=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wfutil.obj" : $(SOURCE) $(DEP_CPP_WFUTI) "$(INTDIR)"

"$(INTDIR)\wfutil.sbr" : $(SOURCE) $(DEP_CPP_WFUTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wfutil.obj" : $(SOURCE) $(DEP_CPP_WFUTI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winfile.c
DEP_CPP_WINFI=\
	".\fmifs.h"\
	".\lfn.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WINFI=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\winfile.obj" : $(SOURCE) $(DEP_CPP_WINFI) "$(INTDIR)"

"$(INTDIR)\winfile.sbr" : $(SOURCE) $(DEP_CPP_WINFI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\winfile.obj" : $(SOURCE) $(DEP_CPP_WINFI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wnetcaps.c
DEP_CPP_WNETC=\
	".\fmifs.h"\
	".\mpr.h"\
	".\numfmt.h"\
	".\suggest.h"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\winfile.h"\
	".\wnetcaps.h"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_CPP_WNETC=\
	".\heap.h"\
	".\machine.h"\
	

!IF  "$(CFG)" == "WinFile - Win32 Release"


"$(INTDIR)\wnetcaps.obj" : $(SOURCE) $(DEP_CPP_WNETC) "$(INTDIR)"

"$(INTDIR)\wnetcaps.sbr" : $(SOURCE) $(DEP_CPP_WNETC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinFile - Win32 Debug"


"$(INTDIR)\wnetcaps.obj" : $(SOURCE) $(DEP_CPP_WNETC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\res.rc
DEP_RSC_RES_R=\
	".\fmifs.h"\
	".\mcopy.cur"\
	".\mmove.cur"\
	".\mpr.h"\
	".\numfmt.h"\
	".\scopy.cur"\
	".\smove.cur"\
	".\split.cur"\
	".\suggest.db"\
	".\suggest.h"\
	".\toolbar.bmp"\
	".\wbbitmap.bmp"\
	".\wfcopy.h"\
	".\wfdir.ico"\
	".\wfdlgs.h"\
	".\wfexti.h"\
	".\wfhelp.h"\
	".\wfinfo.h"\
	".\wfmem.h"\
	".\wftrdir.ico"\
	".\wftree.ico"\
	".\winfile.dlg"\
	".\winfile.h"\
	".\winfile.ico"\
	".\xtratool.bmp"\
	{$(INCLUDE)}"\wfext.h"\
	
NODEP_RSC_RES_R=\
	".\heap.h"\
	".\machine.h"\
	

"$(INTDIR)\res.res" : $(SOURCE) $(DEP_RSC_RES_R) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
