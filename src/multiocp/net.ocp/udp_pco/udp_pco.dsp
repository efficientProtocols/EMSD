# Microsoft Developer Studio Project File - Name="udp_pco" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Static Library" 0x0904

CFG=udp_pco - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "udp_pco.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "udp_pco.mak" CFG="udp_pco - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "udp_pco - Win32 (WCE SH) Debug" (based on\
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
# PROP Output_Dir "..\..\results\arch\sh3\libc"
# PROP Intermediate_Dir "WCESHDbg"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /W3 /Zi /Od /I "..\..\this\include" /I "..\..\include\wce" /I "..\..\mts_ua\include" /I "..\..\esros\include" /I "..\..\ocp\include" /D "NO_UPSHELL" /D "TM_ENABLED" /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "WINCE" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\results\wce\libc\udp_pco.lib"
# Begin Target

# Name "udp_pco - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\local.h
# End Source File
# Begin Source File

SOURCE=.\udp_if.c
DEP_CPP_UDP_I=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sch.h"\
	"..\..\include\wce\sf.h"\
	"..\..\include\wce\target.h"\
	"..\include\addr.h"\
	"..\include\byteordr.h"\
	"..\include\du.h"\
	"..\include\estd.h"\
	"..\include\inetaddr.h"\
	"..\include\modid.h"\
	"..\include\queue.h"\
	"..\include\rc.h"\
	"..\include\sap.h"\
	"..\include\tm.h"\
	".\local.h"\
	".\udp_if.h"\
	".\udp_pc.h"\
	".\udp_po.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_UDP_I=\
	".\nm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\udp_if.h
# End Source File
# Begin Source File

SOURCE=.\udp_pc.c
DEP_CPP_UDP_P=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\include\estd.h"\
	"..\include\modid.h"\
	"..\include\queue.h"\
	"..\include\rc.h"\
	"..\include\tm.h"\
	".\udp_pc.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\udp_pc.h
# End Source File
# Begin Source File

SOURCE=.\udp_po.c
DEP_CPP_UDP_PO=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\include\addr.h"\
	"..\include\du.h"\
	"..\include\estd.h"\
	"..\include\modid.h"\
	"..\include\queue.h"\
	"..\include\rc.h"\
	"..\include\tm.h"\
	".\local.h"\
	".\udp_po.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\udp_po.h
# End Source File
# End Target
# End Project
