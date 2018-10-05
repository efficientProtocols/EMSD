/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*
 * Author: Derrell Lipman
 * History:
 *
 */

/*
 * os.h
 *
 * This module is designed to be ported to alternate environments
 * and operating systems.  Modify at will, but follow the
 * requirements spelled out in the comments.
 */

#ifndef __OS_H__
#define	__OS_H__

#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/select.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <ctype.h>
#include <netdb.h>
#include <curses.h>
#if 0
#include "/usr/local/include/dmalloc.h"
#else
#include "malloc.h"
#endif

#include "rc.h"
#include "modid.h"


/*
 * Tell 'em what operating system and compiler they're using, and what
 * optional facilities exist here.
 */
#define	OS_NAME			"Unix, FreeBSD"
#define	OS_TYPE_UNIX
#define	OS_VARIANT_BSD
#define	OS_VARIANT_FreeBSD
#define	OS_FACILITY_BSDSockets
#define	OS_FACILITY_BSDTimers


/* ReturnCodes for the OS module. */
enum
{
    OS_RC_InvalidFileName	= (1 | ModId_OpSys),
    OS_RC_NoSuchFile		= (2 | ModId_OpSys),
    OS_RC_FileOpen		= (3 | ModId_OpSys),
    OS_RC_FileRead		= (4 | ModId_OpSys),
    OS_RC_FileWrite		= (5 | ModId_OpSys),
    OS_RC_InvalidDate		= (6 | ModId_OpSys),
    OS_RC_InvalidDirectory	= (7 | ModId_OpSys),
    OS_RC_DirectoryRead		= (8 | ModId_OpSys),
    OS_RC_FileMove		= (9 | ModId_OpSys),
    OS_RC_FileCreate		= (10 | ModId_OpSys)
};

/*
 * These types must hold AT LEAST the specified number of bits.
 * They need not hold exactly this number of bits.  Note that
 * arithmetic operations should be possible using the specified
 * type.  (For example, if the native type "short" is a 16-bit
 * signed value, it would be improper to use it for a Uint16 even
 * though it may be capable of holding that value, because 16-bit
 * unsigned arithmetic will not work properly with all values of
 * 16-bit unsigned numbers.)
 */
typedef signed char	OS_Sint8;  /* signed 8-bit value */
typedef signed short	OS_Sint16; /* signed 16-bit value */
typedef signed long	OS_Sint32; /* signed 32-bit value */
typedef unsigned char	OS_Uint8;  /* unsigned 8-bit value */
typedef unsigned short	OS_Uint16; /* unsigned 16-bit value */
typedef unsigned long	OS_Uint32; /* unsigned 32-bit value */

/*
 * Minimum and Maximum values for each of the above types
 */

/* Unsigned Minimums */
#define	OS_MIN_UINT8			((OS_Uint8) 0)
#define	OS_MIN_UINT16			((OS_Uint8) 0)
#define	OS_MIN_UINT32			((OS_Uint32) 0)

/* Unsigned Maximums */
#define	OS_MAX_UINT8			((OS_Uint32) 0xff)
#define	OS_MAX_UINT16			((OS_Uint32) 0xffff)
#define	OS_MAX_UINT32			((OS_Uint32) 0xffffffff)

/* Signed Minimums */
#define	OS_MIN_SINT8			((OS_Sint32) 0x80)
#define	OS_MIN_SINT16			((OS_Sint32) 0x8000)
#define	OS_MIN_SINT32			((OS_Sint32) 0x80000000)

/* Signed Maximums */
#define	OS_MAX_SINT8			((OS_Sint32) 0x7f)
#define	OS_MAX_SINT16			((OS_Sint32) 0x7fff)
#define	OS_MAX_SINT32			((OS_Sint32) 0x7fffffff)


/*
 * These types must hold EXACTLY the specified number of bits!
 */
typedef signed char	OS_Sbits8;  /* signed 8-bit value */
typedef signed short	OS_Sbits16; /* signed 16-bit value */
typedef signed long	OS_Sbits32; /* signed 32-bit value */
typedef unsigned char	OS_Ubits8;  /* unsigned 8-bit value */
typedef unsigned short	OS_Ubits16; /* unsigned 16-bit value */
typedef unsigned long	OS_Ubits32; /* unsigned 32-bit value */

/* A Boolean indicator */
typedef unsigned char	OS_Boolean;


/* Time stored herein should already be converted to UTC time */
typedef struct
{
    OS_Sint16	    year;     /* complete year, e.g. 1995 */
    OS_Sint8	    month;    /* 1=Jan; 12=Dec */
    OS_Sint8	    day;      /* day within the month (1-31) */
    OS_Sint8	    hour;     /* 24-hour clock (0 - 23) */
    OS_Sint8	    minute;   /* minute within the hour (0 - 59) */
    OS_Sint8	    second;   /* second within the minute (0 - 59) */
} OS_ZuluDateTime;

/*
 * Printable time string, in the format:
 *               "Wed Jun 30 21:49:08 1993\n"
 */
#define	OS_printableDateTime(pTimeval)	ctime((time_t *) (pTimeval))


/* Maximum length of a full path/file name */
#define	OS_MAX_FILENAME_LEN	(PATH_MAX + _POSIX_NAME_MAX + 2)

/*
 * Directory path separator.  The first character in the string is
 * the "primary" directory path separator: the one we insert into
 * paths we're building.  All others in the string are characters
 * that the environment uses, but "shouldn't".  (For example, on
 * DOS, the primary directory path separator is '\', but many
 * programs also allow '/' to be used.
 */
#define	OS_DIR_PATH_SEPARATOR	"/"

/*
 * Determine if an Interrupt Service Routine is active.
 * In this environment, we're "never" in an ISR.
 */
#define	OS_isrIsActive()	(0)


/*
 * Enter or Leave a section of code that must be atomic.
 */

#define	OS_EnterAtomic()
#define	OS_LeaveAtomic()

/* for backwards compatibility... */
#define	ENTER	OS_EnterAtomic
#define	LEAVE	OS_LeaveAtomic

/*
 * In this environment, we'll use dynamic memory allocation.  In a
 * small device, a statically-allocated pool of memory can be
 * created at init time.
 */

#ifndef OS_MALLOC_DEBUG

#define	OS_allocDebugInit(filename)
#define	OS_alloc(numOctets)		(malloc((size_t) numOctets))
#define	OS_realloc(p, numOctets)	(realloc((void *) p,		\
						  (size_t) numOctets))
#define	OS_free(pMem)			(free(pMem))
#define	OS_allocPrintOutstanding()
#define	OS_allocResetPrint()

#else

void
OS_allocDebugInit(char * pDebugFileName);

void *
OS_allocDebug(OS_Uint16 numBytes,
	      char * pFileName,
	      int lineNum);

#define	OS_alloc(numOctets)		(OS_allocDebug(numOctets, \
						       __FILE__,  \
						       __LINE__))

void *
OS_reallocDebug(void * p,
		OS_Uint16 numBytes,
		char * pFileName,
		int lineNum);

#define	OS_realloc(p, numOctets)	(OS_reallocDebug(p,		\
							 numOctets,	\
							 __FILE__,	\
							 __LINE__))

void
OS_freeDebug(void * p,
	     char * pFileName,
	     int lineNum);

#define	OS_free(pMem)			(OS_freeDebug(pMem,	 \
						      __FILE__,	 \
						      __LINE__))

void
OS_allocPrintOutstanding(void);

void
OS_allocResetPrint(void);

#endif /* OS_MALLOC_DEBUG */


char *
OS_allocStringCopy(char * pSourceString);

char *
OS_allocStringNCopy(char * pSourceString,
		    OS_Uint16 len);



#define	OS_copy(dest, src, len)		memcpy(dest, src, len)
#define	OS_move(dest, src, len)		memmove(dest, src, len)
#define	OS_memSet(p, octet, len)	memset(p, octet, len)
#define	OS_memCmp(p1, p2, len)		memcmp(p1, p2, len)

/* Case-insensitive comparision */
#define	OS_strcasecmp(s1, s2)		strcasecmp(s1, s2)
#define	OS_strncasecmp(s1, s2, n)	strncasecmp(s1, s2, n)

/* Search for a sub-string within a string */
#define	OS_findSubstring		strstr

/* Separate a string into tokens */
#define	OS_findStringToken		strtok

/* Sleep for a while */
#define	OS_sleep			sleep

/* Types used for Directory Search Operations */
typedef void *	    OS_Dir;

/* File operations */

#ifndef OS_FILE_DEBUG
# define	OS_fileDebugInit(pFileName)
# define	OS_fileOpen		fopen
# define	OS_fileClose		fclose
#else
void
OS_fileDebugInit(char * pDebugFileName);

void *
OS_fileOpenDebug(char * pName,
		 char * pMode,
		 char * pFileName,
		 int lineNum);

void
OS_fileCloseDebug(void * hFile,
		  char * pFileName,
		  int lineNum);

# define	OS_fileOpen(pName, pMode)			    \
					OS_fileOpenDebug(pName,	    \
							 pMode,	    \
							 __FILE__,  \
							 __LINE__)

# define	OS_fileClose(hFile)				    \
					OS_fileCloseDebug(hFile,    \
							  __FILE__, \
							  __LINE__)
#endif	/* OS_FILE_DEBUG */

#define	OS_fileGetString	fgets
#define	OS_fileRead		fread
#define	OS_filePutChar		fputc
#define	OS_filePutString	fputs
#define	OS_fileWrite		fwrite
#define	OS_filePrintf		fprintf
#define	OS_fileScanf		fscanf

ReturnCode
OS_init(void);

ReturnCode
OS_currentDateTime(OS_ZuluDateTime * pDateTime,
		   OS_Uint32 * pJulianDate);

ReturnCode
OS_dateStructToJulian(OS_ZuluDateTime * pDateTime,
		      OS_Uint32 * pJulianDate);

ReturnCode
OS_julianToDateStruct(OS_Uint32 julianDate,
		      OS_ZuluDateTime * pDateTime);

ReturnCode
OS_timestamp (char *strBuf,
	      OS_Uint32 strLen,
	      OS_Uint32 mask,
	      char *moduleName,
	      char *loc);

OS_Uint32
OS_timeMsec(void);

ReturnCode
OS_deleteFile(char * pFileName);

void
OS_dirSetFilter(OS_Boolean (* pfFilter)(const char * pFileName));

ReturnCode
OS_dirOpen(char * pDirectoryName,
	   void ** hDirData);

ReturnCode
OS_dirFindNext(void * hDirData,
	       char ** ppFileName);

ReturnCode
OS_dirRewind(void * hDirData);

char *
OS_dirGetPath(void * hDirData);

ReturnCode
OS_moveFile(char * pNewName, char * pOldName);

ReturnCode
OS_uniqueName(char * pPrototype);

ReturnCode
OS_fileSize(char * pFileName,
	    OS_Uint32 * pSize);

ReturnCode
OS_getLocalHostName(char * buf,
		    OS_Uint16 bufLen);

#endif /* __OS_H__ */
