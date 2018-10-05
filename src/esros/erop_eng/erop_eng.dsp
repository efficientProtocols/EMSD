# Microsoft Developer Studio Project File - Name="erop_eng" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Static Library" 0x0904

CFG=erop_eng - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "erop_eng.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "erop_eng.mak" CFG="erop_eng - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "erop_eng - Win32 (WCE SH) Debug" (based on\
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
# ADD LIB32 /nologo /out:"..\..\results\wce\libc\erop_eng.lib"
# Begin Target

# Name "erop_eng - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\clinvktd.c
DEP_CPP_CLINV=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\clinvtd2.c
DEP_CPP_CLINVT=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\clperftd.c
DEP_CPP_CLPER=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\clpertd2.c
DEP_CPP_CLPERT=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\conffunc.c
DEP_CPP_CONFF=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\config.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\getopt.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\esro.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\extfuncs.h
# End Source File
# Begin Source File

SOURCE=.\invact2.c
DEP_CPP_INVAC=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sch.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\..\ocp\include\udp_if.h"\
	"..\include\erop.h"\
	"..\include\eropdu.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_cfg.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\invokact.c
DEP_CPP_INVOK=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sch.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\..\ocp\include\udp_if.h"\
	"..\include\erop.h"\
	"..\include\eropdu.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_cfg.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\invoke.c
DEP_CPP_INVOKE=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sf.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\sap.h"\
	"..\..\ocp\include\seq.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\invoke.h
# End Source File
# Begin Source File

SOURCE=.\keep.c
DEP_CPP_KEEP_=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\layernm.c
DEP_CPP_LAYER=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\layernm.h
# End Source File
# Begin Source File

SOURCE=.\local.h
# End Source File
# Begin Source File

SOURCE=.\lowerind.c
DEP_CPP_LOWER=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\eropdu.h"\
	"..\include\esro.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\erop_cfg.c
DEP_CPP_EROP_=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\inetaddr.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\sap.h"\
	"..\..\ocp\include\tm.h"\
	"..\..\ocp\include\udp_if.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_cfg.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\erop_cfg.h
# End Source File
# Begin Source File

SOURCE=.\erop_sap.c
DEP_CPP_EROP_S=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\seq.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_cfg.h"\
	".\erop_sap.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\erop_sap.h
# End Source File
# Begin Source File

SOURCE=.\eropfsm.h
# End Source File
# Begin Source File

SOURCE=.\pduin.c
DEP_CPP_PDUIN=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\byteordr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\sap.h"\
	"..\..\ocp\include\tm.h"\
	"..\..\ocp\include\udp_if.h"\
	"..\include\erop.h"\
	"..\include\eropdu.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\pduout.c
DEP_CPP_PDUOU=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\byteordr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\eropdu.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\pdupr.c
DEP_CPP_PDUPR=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\byteordr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\eropdu.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\perfact.c
DEP_CPP_PERFA=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sch.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\..\ocp\include\udp_if.h"\
	"..\include\erop.h"\
	"..\include\eropdu.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\perfact2.c
DEP_CPP_PERFAC=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sch.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\..\ocp\include\udp_if.h"\
	"..\include\erop.h"\
	"..\include\eropdu.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tmr_if.c
DEP_CPP_TMR_I=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tmr_if.h
# End Source File
# Begin Source File

SOURCE=.\userin.c
DEP_CPP_USERI=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\local.h"\
	".\erop_cfg.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\userout.c
DEP_CPP_USERO=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\include\wce\sch.h"\
	"..\..\include\wce\sf.h"\
	"..\..\include\wce\target.h"\
	"..\..\include\wce\tmr.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\du.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\fsm.h"\
	"..\..\ocp\include\fsmtrans.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\erop.h"\
	"..\include\esro.h"\
	"..\include\nm.h"\
	".\extfuncs.h"\
	".\invoke.h"\
	".\layernm.h"\
	".\local.h"\
	".\erop_sap.h"\
	".\eropfsm.h"\
	".\tmr_if.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# End Target
# End Project
