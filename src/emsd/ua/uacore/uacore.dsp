# Microsoft Developer Studio Project File - Name="uacore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Static Library" 0x0904

CFG=uacore - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "uacore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "uacore.mak" CFG="uacore - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "uacore - Win32 (WCE SH) Debug" (based on\
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
# ADD CPP /nologo /W3 /Zi /Od /I "..\..\..\this\include" /I "..\..\..\include\wce" /I "..\..\..\mts_ua\include" /I "..\..\..\esros\include" /I "..\..\..\ocp\include" /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "WINCE" /D "NO_UPSHELL" /D "TM_ENABLED" /D "DELIVERY_CONTROL" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\results\wce\libc\uacore.lib"
# Begin Target

# Name "uacore - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\delctrl.c
DEP_CPP_DELCT=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\include\wce\tmr.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\buf.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\fsmtrans.h"\
	"..\..\..\ocp\include\mm.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\dup.c
DEP_CPP_DUP_C=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\buf.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\inetaddr.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sap.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	".\dup.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\dup.h
# End Source File
# Begin Source File

SOURCE=.\fromsh.c
DEP_CPP_FROMS=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\emsdcomp.c
DEP_CPP_EMSDCO=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\esroevt.c
DEP_CPP_ESROE=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\buf.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\inetaddr.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sap.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\dup.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\msgid.c
DEP_CPP_MSGID=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\include\emsd.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\oper.c
DEP_CPP_OPER_=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\submit.c
DEP_CPP_SUBMI=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\include\wce\tmr.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\buf.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\fsmtrans.h"\
	"..\..\..\ocp\include\mm.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tosh.c
DEP_CPP_TOSH_=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ua.c
DEP_CPP_UA_C12=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\sch.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\include\wce\tmr.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\config.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\fsmtrans.h"\
	"..\..\..\ocp\include\mm.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nnp.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\uacore.h
# End Source File
# Begin Source File

SOURCE=.\uadup.c
DEP_CPP_UADUP=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\buf.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\uasubr.c
DEP_CPP_UASUB=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\include\wce\tmr.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\buf.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\mm.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\dup.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\uperform.c
DEP_CPP_UPERF=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\include\wce\tmr.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\buf.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\fsmtrans.h"\
	"..\..\..\ocp\include\mm.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsd822.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\upreproc.c
DEP_CPP_UPREP=\
	"..\..\..\include\wce\hw.h"\
	"..\..\..\include\wce\oe.h"\
	"..\..\..\include\wce\os.h"\
	"..\..\..\include\wce\target.h"\
	"..\..\..\esros\include\esro_cb.h"\
	"..\..\..\ocp\include\addr.h"\
	"..\..\..\ocp\include\asn.h"\
	"..\..\..\ocp\include\buf.h"\
	"..\..\..\ocp\include\du.h"\
	"..\..\..\ocp\include\estd.h"\
	"..\..\..\ocp\include\fsm.h"\
	"..\..\..\ocp\include\fsmtrans.h"\
	"..\..\..\ocp\include\mm.h"\
	"..\..\..\ocp\include\modid.h"\
	"..\..\..\ocp\include\nvm.h"\
	"..\..\..\ocp\include\queue.h"\
	"..\..\..\ocp\include\rc.h"\
	"..\..\..\ocp\include\sbm.h"\
	"..\..\..\ocp\include\strfunc.h"\
	"..\..\..\ocp\include\tm.h"\
	"..\..\include\emsd.h"\
	"..\..\include\emsdmail.h"\
	"..\..\include\mtsua.h"\
	"..\..\include\ua.h"\
	".\uacore.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# End Target
# End Project
