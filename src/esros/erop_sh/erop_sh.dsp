# Microsoft Developer Studio Project File - Name="erop_sh" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Static Library" 0x0904

CFG=erop_sh - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "erop_sh.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "erop_sh.mak" CFG="erop_sh - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "erop_sh - Win32 (WCE SH) Debug" (based on\
 "Win32 (WCE SH) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=shcl.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESHDbg"
# PROP BASE Intermediate_Dir "WCESHDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESHDbg"
# PROP Intermediate_Dir "WCESHDbg"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /W3 /Zi /Od /I "..\..\this\include" /I "..\..\include\wce" /I "..\..\mts_ua\include" /I "..\..\esros\include" /I "..\..\ocp\include" /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "WINCE" /D "NO_UPSHELL" /D "TM_ENABLED" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\results\wce\libc\erop_sh.lib"
# Begin Target

# Name "erop_sh - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\extfuncs.h
# End Source File
# Begin Source File

SOURCE=.\getconf.c
DEP_CPP_GETCO=\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\config.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\esro.h"\
	
NODEP_CPP_GETCO=\
	"..\..\ocp\include\hw.h"\
	"..\..\ocp\include\oe.h"\
	"..\..\ocp\include\os.h"\
	
# End Source File
# Begin Source File

SOURCE=.\heartbea.c
DEP_CPP_HEART=\
	"..\..\ocp\include\estd.h"\
	
NODEP_CPP_HEART=\
	"..\..\ocp\include\hw.h"\
	"..\..\ocp\include\oe.h"\
	"..\..\ocp\include\os.h"\
	".\sch.h"\
	
# End Source File
# Begin Source File

SOURCE=.\local.h
# End Source File
# Begin Source File

SOURCE=.\esroinit.c
DEP_CPP_ESROI=\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\config.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\esro_cb.h"\
	
NODEP_CPP_ESROI=\
	"..\..\ocp\include\hw.h"\
	"..\..\ocp\include\oe.h"\
	"..\..\ocp\include\os.h"\
	"..\include\target.h"\
	".\sch.h"\
	".\target.h"\
	
# End Source File
# End Target
# End Project
