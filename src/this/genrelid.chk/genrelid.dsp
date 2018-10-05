# Microsoft Developer Studio Project File - Name="genrelid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Static Library" 0x0904

CFG=genrelid - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "genrelid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "genrelid.mak" CFG="genrelid - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "genrelid - Win32 (WCE SH) Release" (based on\
 "Win32 (WCE SH) Static Library")
!MESSAGE "genrelid - Win32 (WCE SH) Debug" (based on\
 "Win32 (WCE SH) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=shcl.exe

!IF  "$(CFG)" == "genrelid - Win32 (WCE SH) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCESHRel"
# PROP BASE Intermediate_Dir "WCESHRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCESHRel"
# PROP Intermediate_Dir "WCESHRel"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /ML /W3 /GX- /O2 /D "NDEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /ML /W3 /GX- /O2 /D "NDEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "genrelid - Win32 (WCE SH) Debug"

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
# ADD BASE CPP /nologo /MLd /W3 /GX- /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /MLd /W3 /GX- /Zi /Od /I "..\include" /I "..\..\include\wce" /I "..\..\mts_ua\include" /I "..\..\esros\include" /I "..\..\ocp\include" /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "WINCE" /D "NO_UPSHELL" /D "TM_ENABLED" /D "DELIVERY_CONTROL" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\results\wce\libc\relid.lib"

!ENDIF 

# Begin Target

# Name "genrelid - Win32 (WCE SH) Release"
# Name "genrelid - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\compat.c

!IF  "$(CFG)" == "genrelid - Win32 (WCE SH) Release"

!ELSEIF  "$(CFG)" == "genrelid - Win32 (WCE SH) Debug"

DEP_CPP_COMPA=\
	".\relid.h"\
	
NODEP_CPP_COMPA=\
	".\cpr.h"\
	".\estd.h"\
	".\pf.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\relid.c

!IF  "$(CFG)" == "genrelid - Win32 (WCE SH) Release"

!ELSEIF  "$(CFG)" == "genrelid - Win32 (WCE SH) Debug"

NODEP_CPP_RELID=\
	".\estd.h"\
	".\pf.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\relid.h
# End Source File
# End Target
# End Project
