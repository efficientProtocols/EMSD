# Microsoft Developer Studio Project File - Name="asn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Static Library" 0x0904

CFG=asn - Win32 (WCE SH) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "asn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "asn.mak" CFG="asn - Win32 (WCE SH) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "asn - Win32 (WCE SH) Debug" (based on\
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
# ADD CPP /nologo /W3 /Zi /Od /I "..\..\this\include" /I "..\..\include\wce" /I "..\..\mts_ua\include" /I "..\..\esros\include" /I "..\..\ocp\include" /D "NO_UPSHELL" /D "TM_ENABLED" /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "WINCE" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\results\wce\libc\asn.lib"
# Begin Target

# Name "asn - Win32 (WCE SH) Debug"
# Begin Source File

SOURCE=.\asn.c
DEP_CPP_ASN_C=\
	".\asn.h"\
	".\asnlocal.h"\
	
NODEP_CPP_ASN_C=\
	".\estd.h"\
	".\queue.h"\
	".\strfunc.h"\
	".\tm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\asn.h
# End Source File
# Begin Source File

SOURCE=.\asnber.c
DEP_CPP_ASNBE=\
	".\asn.h"\
	".\asnlocal.h"\
	
NODEP_CPP_ASNBE=\
	".\buf.h"\
	".\estd.h"\
	".\queue.h"\
	".\strfunc.h"\
	".\tm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\asndebug.c
DEP_CPP_ASNDE=\
	".\asn.h"\
	
NODEP_CPP_ASNDE=\
	".\estd.h"\
	".\queue.h"\
	".\strfunc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\asnlocal.h
# End Source File
# End Target
# End Project
