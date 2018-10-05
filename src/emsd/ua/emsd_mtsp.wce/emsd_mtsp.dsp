# Microsoft Developer Studio Project File - Name="emsd_mtsp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Dynamic-Link Library" 0x0902

CFG=emsd_mtsp - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "emsd_mtsp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "emsd_mtsp.mak" CFG="emsd_mtsp - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "emsd_mtsp - Win32 (WCE SH) Debug" (based on\
 "Win32 (WCE SH) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=shcl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\WCESHDbg"
# PROP BASE Intermediate_Dir ".\WCESHDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\WCESHDbg"
# PROP Intermediate_Dir ".\WCESHDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "PEGASUS" /D "UNICODE" /YX /c
# ADD CPP /nologo /W3 /Zi /Od /I "../../../this/include" /I "../../../include/wce" /I "../../../mts_ua/include" /I "../../../ocp/include" /D "DEBUG" /D "SH3" /D "_SH3_" /D "PEGASUS" /D "UNICODE" /D "TM_ENABLED" /D "WINCE" /YX /c
# ADD BASE RSC /l 0x409 /r /d "SH3" /d "_SH3_" /d "PEGASUS" /d "DEBUG"
# ADD RSC /l 0x409 /r /d "SH3" /d "_SH3_" /d "PEGASUS" /d "DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib libcd.lib /nologo /dll /debug /machine:SH3 /subsystem:pegasus
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib libcd.lib coredll.lib msgstore.lib sf.lib gf.lib /nologo /dll /debug /machine:SH3 /libpath:"../../../results/wce/libc" /subsystem:pegasus
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY
# Begin Target

# Name "emsd_mtsp - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\EMSD_MTSP.DEF
# End Source File
# Begin Source File

SOURCE=.\EMSD_MTSP.H
# End Source File
# Begin Source File

SOURCE=.\EMSD_MTSP.RC
# End Source File
# Begin Source File

SOURCE=.\SERVICE.C
DEP_CPP_SERVI=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\tm.h"\
	"..\..\..\ocp\include\config.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	".\EMSD_MTSP.H"\
	".\transprt.h"\
	{$(INCLUDE)}"MSGSTORE.H"\
	{$(INCLUDE)}"RAPI.H"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"WINDBASE.H"\
	
# End Source File
# Begin Source File

SOURCE=.\SUPPORT.C
DEP_CPP_SUPPO=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\tm.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	".\EMSD_MTSP.H"\
	".\transprt.h"\
	{$(INCLUDE)}"MSGSTORE.H"\
	{$(INCLUDE)}"RAPI.H"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"WINDBASE.H"\
	
# End Source File
# Begin Source File

SOURCE=.\transprt.h
# End Source File
# End Target
# End Project
