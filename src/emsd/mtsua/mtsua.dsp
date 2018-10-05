# Microsoft Developer Studio Project File - Name="mtsua" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Static Library" 0x0904

CFG=mtsua - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mtsua.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mtsua.mak" CFG="mtsua - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mtsua - Win32 (WCE SH) Debug" (based on\
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
# ADD LIB32 /nologo /out:"..\..\results\wce\libc\mtsua.lib"
# Begin Target

# Name "mtsua - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\ctrlfile.c
DEP_CPP_CTRLF=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\asn.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\mm.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\strfunc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\emsd.h"\
	"..\include\emsdmail.h"\
	"..\include\mtsua.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\emsd2rfc.c
DEP_CPP_EMSD2R=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\asn.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\strfunc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\emsd.h"\
	"..\include\emsd822.h"\
	"..\include\emsdmail.h"\
	"..\include\mtsua.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\emsdaddr.c
DEP_CPP_EMSDAD=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\asn.h"\
	"..\..\ocp\include\buf.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\strfunc.h"\
	"..\include\emsd.h"\
	"..\include\emsdmail.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\emsdalloc.c
DEP_CPP_EMSDAL=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\asn.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\strfunc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\emsd.h"\
	"..\include\emsdmail.h"\
	"..\include\mtsua.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\emsdasn.c
DEP_CPP_EMSDAS=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\asn.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\strfunc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\emsd.h"\
	"..\include\emsdmail.h"\
	"..\include\mtsua.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mail.c
DEP_CPP_MAIL_=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\asn.h"\
	"..\..\ocp\include\buf.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\strfunc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\emsd.h"\
	"..\include\emsd822.h"\
	"..\include\emsdmail.h"\
	"..\include\mtsua.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\rfc822.c
DEP_CPP_RFC82=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\esros\include\esro.h"\
	"..\..\ocp\include\addr.h"\
	"..\..\ocp\include\asn.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\queue.h"\
	"..\..\ocp\include\rc.h"\
	"..\..\ocp\include\strfunc.h"\
	"..\..\ocp\include\tm.h"\
	"..\include\emsd.h"\
	"..\include\emsd822.h"\
	"..\include\emsdd.h"\
	"..\include\emsdmail.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# Begin Source File

SOURCE=.\subr.c
DEP_CPP_SUBR_=\
	"..\..\include\wce\hw.h"\
	"..\..\include\wce\oe.h"\
	"..\..\include\wce\os.h"\
	"..\..\ocp\include\estd.h"\
	"..\..\ocp\include\modid.h"\
	"..\..\ocp\include\rc.h"\
	"..\include\emsd822.h"\
	"..\include\emsdmail.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
# End Source File
# End Target
# End Project
