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
!MESSAGE "genrelid - Win32 (WCE SH) Debug" (based on\
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
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /I "..\include" /I "..\..\include\wce" /I "..\..\mts_ua\include" /I "..\..\esros\include" /I "..\..\ocp\include" /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "TM_ENABLED" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\results\wce\libc\relid.lib"
# Begin Target

# Name "genrelid - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\compat.c
DEP_CPP_COMPA=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\pf.h"\
	"..\..\ocp\include\rc.h"\
	"..\include\cpr.h"\
	".\relid.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\makefile
# Begin Custom Build
InputPath=.\makefile

"relid.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	make relid.c

# End Custom Build
# End Source File
# Begin Source File

SOURCE=.\relid.c
DEP_CPP_RELID=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\pf.h"\
	"..\..\ocp\include\rc.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\relid.h
# End Source File
# End Target
# End Project
