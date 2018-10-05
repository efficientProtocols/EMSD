# Microsoft Developer Studio Project File - Name="relidusr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Application" 0x0901

CFG=relidusr - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "relidusr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "relidusr.mak" CFG="relidusr - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "relidusr - Win32 (WCE SH) Debug" (based on\
 "Win32 (WCE SH) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=shcl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESHDbg"
# PROP BASE Intermediate_Dir "WCESHDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESHDbg"
# PROP Intermediate_Dir "WCESHDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /I "..\include" /I "..\..\include\wce" /I "..\..\mts_ua\include" /I "..\..\esros\include" /I "..\..\ocp\include" /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "TM_ENABLED" /YX /c
# ADD BASE RSC /l 0x409 /r /d "SH3" /d "_SH3_" /d "_WIN32_WCE" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x409 /r /d "SH3" /d "_SH3_" /d "_WIN32_WCE" /d "UNICODE" /d "DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:SH3 /subsystem:windowsce
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib cpright.lib relid.lib sf.lib gf.lib /nologo /debug /machine:SH3 /libpath:"..\..\results\wce\libc" /subsystem:windowsce
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY
# Begin Target

# Name "relidusr - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\main.c
DEP_CPP_MAIN_=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\getopt.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\rc.h"\
	"..\include\cpr.h"\
	"..\include\relid.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# End Target
# End Project
