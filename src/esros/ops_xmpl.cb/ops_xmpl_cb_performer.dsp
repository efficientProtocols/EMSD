# Microsoft Developer Studio Project File - Name="ops_xmpl_cb_performer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Application" 0x0901

CFG=ops_xmpl_cb_performer - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ops_xmpl_cb_performer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ops_xmpl_cb_performer.mak"\
 CFG="ops_xmpl_cb_performer - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ops_xmpl_cb_performer - Win32 (WCE SH) Debug" (based on\
 "Win32 (WCE SH) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=shcl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ops_xmpl"
# PROP BASE Intermediate_Dir "ops_xmpl"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ops_xmpl"
# PROP Intermediate_Dir "ops_xmpl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /W3 /Zi /Od /I "..\..\this\include" /I "..\..\include\wce" /I "..\..\mts_ua\include" /I "..\..\esros\include" /I "..\..\ocp\include" /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "WINCE" /D "NO_UPSHELL" /D "TM_ENABLED" /YX /c
# ADD BASE RSC /l 0x409 /r /d "SH3" /d "_SH3_" /d "_WIN32_WCE" /d "DEBUG"
# ADD RSC /l 0x409 /r /d "SH3" /d "_SH3_" /d "_WIN32_WCE" /d "DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib libcd.lib /nologo /debug /machine:SH3 /subsystem:windowsce
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib libcd.lib gf.lib sf.lib erop_eng.lib erop_sh.lib udp_pco.lib winsock.lib fsm.lib /nologo /debug /machine:SH3 /libpath:"..\..\results\wce\libc" /subsystem:windowsce
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY
# Begin Target

# Name "ops_xmpl_cb_performer - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\extfuncs.h
# End Source File
# Begin Source File

SOURCE=.\ginit.c
DEP_CPP_GINIT=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sch.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\..\ocp\include\udp_po.h"\
	"..\include\esro_cb.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\nm.c
DEP_CPP_NM_C2=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\rc.h"\
	"..\include\nm.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\performr.c
DEP_CPP_PERFO=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sch.h"\
	"..\..\include\wce\sf.h"\
	"..\..\include\wce\target.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\getopt.h"\
	"..\..\ocp\include\inetaddr.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\pf.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\sap.h"\
	"..\..\ocp\include\tm.h"\
	"..\..\ocp\include\udp_if.h"\
	"..\..\ocp\include\udp_po.h"\
	"..\include\esro_cb.h"\
	".\extfuncs.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# End Target
# End Project
