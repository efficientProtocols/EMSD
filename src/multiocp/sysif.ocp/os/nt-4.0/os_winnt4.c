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
**
** IMPORTANT NOTE:
**
**   This file is compiled by virtue of a #include in "os.c".  The
**   makefile does not directly compile this file.
**
**/


#include "sf.h"
#include "os.h"
#ifdef MSDOS
#include "eh.h"
#endif

/* THIS USED TO BE dir.h */
#ifdef MSDOS	/*{*/

#ifndef _DIR_H_	/*{*/
#define _DIR_H_


#define MAXNAMLEN	13

struct	direct {
        int d_namlen;
	char	d_name[MAXNAMLEN];
};

#define dirent direct

typedef enum DirState {
    DirActive,
    DirVirgin,
    DirInactive
} DirState;

typedef struct {
    char dirName[128];	/* Name of the directory, including drive */
    char *dirNameEnd;	/* End of String Location for dirName */
    DirState state;	/* See DirState */
    long index;		/* Position of a file name inside the directory */
    char indexName[MAXNAMLEN];  /* File Name corresponding to indexName */
} DIR ;

extern	DIR *opendir();
extern	struct direct *readdir();
extern	long telldir();
extern	void seekdir();
#define rewinddir(dirp)	seekdir((dirp), (long)0)
extern	void closedir();

extern int
scanDirectory(const char * dirname,
	      struct dirent *** namelist,
	      int (*select)(const struct dirent *),
	      int (*dcomp)(const struct dirent * const *,
			   const struct dirent * const *));

extern int
alphaSort(const struct dirent * const * d1,
	  const struct dirent * const * d2);

#endif	/*} _DIR_H_ */
#endif	/*} MSDOS */

#include <sys/stat.h>
#include <direct.h>
#include <time.h>


/* This used to be dosos.h */
#ifndef _DOSOS_H_	/*{*/
#define _DOSOS_H_

#include <dos.h>
#include <io.h>

typedef struct _find_t  DirDta;

#ifdef M_I86SM 
#define GET_OFF(ptr)   ((unsigned) ptr)
static unsigned _showds() { struct _SREGS r; _segread(&r); return r.ds;}
#define GET_SEG(ptr)   _showds();
#endif

#endif	/* _DOSOS_H_ */ /*}*/

volatile int OS_isrActive;
volatile int OS_atomicOperationInProgress;

int
OS_isrIsActive(void)
{
    return OS_isrActive;
}


ReturnCode
OS_init(void)
{
    OS_isrActive = 0;
    OS_atomicOperationInProgress = 0;
    return Success;
}


ReturnCode
OS_currentDateTime(OS_ZuluDateTime * pDateTime,
		   OS_Uint32 * pJulianDate)
{
    ReturnCode 	    rc;
    OS_Uint32 	    currentTime;
    
    currentTime = (OS_Uint32) time(NULL);

    if (pJulianDate != NULL)
    {
	*pJulianDate = currentTime;
    }

    if (pDateTime != NULL)
    {
	if ((rc = OS_julianToDateStruct(currentTime,
					pDateTime)) != Success)
	{
	    return FAIL_RC(rc, ("OS_currentDateTime: julianToDate"));
	}
    }

    return Success;
}

ReturnCode
OS_dateStructToJulian(OS_ZuluDateTime * pDateTime,
		      OS_Uint32 * pJulianDate)
{
    struct tm 	    unixTime;

    /* Do some elementary validation */
    if (pDateTime->year < 1970 ||
	pDateTime->month > 12 ||
	pDateTime->day > 31 ||
	pDateTime->hour > 23 ||
	pDateTime->minute > 59 ||
	pDateTime->second > 59)
    {
	return FAIL_RC(OS_RC_InvalidDate,
		       ("OS_dateStructToJulian: invalid date"));
    }

    unixTime.tm_year = pDateTime->year - 1900; /* year since 1900 */
    unixTime.tm_mon = pDateTime->month - 1;
    unixTime.tm_mday = pDateTime->day;
    unixTime.tm_hour = pDateTime->hour;
    unixTime.tm_min = pDateTime->minute;
    unixTime.tm_sec = pDateTime->second;
    unixTime.tm_isdst = 0;
#ifndef MSDOS
    unixTime.tm_gmtoff = 0;
#endif

    if ((*pJulianDate = (OS_Uint32) mktime(&unixTime)) == -1)
    {
	return FAIL_RC(OS_RC_InvalidDate,
		       ("OS_dateStructToJulian: mktime failed"));
    }

    return Success;
}


ReturnCode
OS_julianToDateStruct(OS_Uint32 julianDate,
		      OS_ZuluDateTime * pDateTime)
{
    struct tm *		pUnixTime;
    time_t		unixJulian;

    unixJulian = julianDate; /* make sure the types are compatible */

    /* Convert the julian time to a unix time struct */
    if ((pUnixTime = gmtime(&unixJulian)) == NULL)
    {
	return FAIL_RC(OS_RC_InvalidDate,
		       ("OS_julianToDateStruct: invalid date"));
    }

    /* Convert the unix time struct to one of ours */
    pDateTime->year = pUnixTime->tm_year + 1900;
    pDateTime->month = pUnixTime->tm_mon + 1;
    pDateTime->day = pUnixTime->tm_mday;
    pDateTime->hour = pUnixTime->tm_hour;
    pDateTime->minute = pUnixTime->tm_min;
    pDateTime->second = pUnixTime->tm_sec;

    /* We did it! */
    return Success;
}

ReturnCode
OS_timestamp (char *strBuf, OS_Uint32 strLen, OS_Uint32 mask, char *moduleName, char *loc)
{
    time_t     timeSec;
    struct tm  *tm;
    int        tmpLen = 0;
    char       tmpBuf[128];

#if 1

    (void) time(&timeSec);
    tm = localtime (&timeSec);

    sprintf (tmpBuf, "[%02d%02d%02d%02d%02d%02d%04d %1.16s %x %1.64s]\t",
	     tm->tm_year % 100, tm->tm_mon + 1, tm->tm_mday,
	     tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (timeSec / 100000),
	     moduleName ? moduleName : "null",
	     (int)mask, loc);

    tmpLen = strlen(tmpBuf) + 1;  /* allow for null terminater */
    if (tmpLen <= strLen)
    {
        strcpy(strBuf, tmpBuf);
    }
    else
    {
	strncpy (strBuf, tmpBuf, (int) strLen);
	strBuf[strLen - 1] = '\0';
    }

#else

    sprintf (tmpBuf, "[%1.16s %x %1.64s]\t",
	     moduleName ? moduleName : "null",
	     (int)mask, loc);
    tmpLen = strlen(tmpBuf) + 1;  /* allow for null terminater */
    strncpy (strBuf, tmpBuf, strLen < tmpLen ? strLen : tmpLen);

#endif /* NOTYET */

    return Success;
}

/*
 * OS_Uint32
 * OS_timeMsec(Void)
 *
 * Return the system time in milliseconds in a 32-bit number.
 * This wraps around about 7 times per year so don't use for
 * absolute time. Mainly useful for event relative timing.
 */
OS_Uint32
OS_timeMsec(Void)
{
  time_t timeSec;

  timeSec = time (&timeSec);

  return (timeSec * 1000);
}

/*
	Current min & sec in seconds
 */
OS_Uint32
OS_timeMinSec(Void)
{
    time_t     timeSec;
    struct tm  *tm;

    (void) time(&timeSec);
    tm = localtime (&timeSec);

    return (tm->tm_min * 60 + tm->tm_sec);
}

/*
	Current hour & min & sec in seconds
 */
OS_Uint32
OS_timeHourMinSec(Void)
{
    time_t     timeSec;
    struct tm  *tm;

    (void) time(&timeSec);
    tm = localtime (&timeSec);

    return (tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec);
}

ReturnCode
OS_timeString (char *strBuf)
{
    time_t     	timeSec;
    struct tm   *tm;
    char  	tmpBuf[128];

    (void) time(&timeSec);
    tm = localtime (&timeSec);

    sprintf (tmpBuf, "%02d:%02d:%02d.%01d",
	     tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (timeSec / 100000));

    if(strlen(tmpBuf) > 16) {
    	strcpy (strBuf, "Time not available");
    	return Fail;
    }

    strcpy (strBuf, tmpBuf);

    return Success;
}


ReturnCode
OS_deleteFile(char * pFileName)
{
    if (unlink(pFileName) == 0)
    {
	return Success;
    }

    switch(errno)
    {
    case EFAULT:
    case EACCES:
    case EPERM:
#ifdef ENAMETOOLONG
    case ENAMETOOLONG:
#endif
    case ENOTDIR:
    case EISDIR:
    case EROFS:
    default:
	return FAIL_RC(OS_RC_InvalidFileName,
		       ("OS_deleteFile: invalid file name"));
	
    case ENOENT:
	return FAIL_RC(OS_RC_NoSuchFile,
		       ("OS_deleteFile: no such file"));
	
    case ENOMEM:
	return FAIL_RC(ResourceError,
		       ("OS_deleteFile: kernel memory exhausted"));
    }
}


ReturnCode
OS_moveFile(char * pNewName, char * pOldName)
{
    /* this declaration is _supposed_ to be in <unistd.h> */
    extern int rename(const char *oldpath, const char *newpath);

    /* Try to rename the file */
    if (rename(pOldName, pNewName) != 0)
    {
#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
	printf("*** rename(%s, %s) failed, errno=%d; "
	       "using mv instead ***\n",
	       pOldName, pNewName, errno);
#endif

	/* We couldn't rename it. */
	return Fail;
    }

    return Success;
}


ReturnCode
OS_uniqueName(char * pPrototype)
{
    if (mktemp(pPrototype) == NULL)
    {
	return FAIL_RC(OS_RC_FileCreate, ("OS_uniqueName"));
    }

    return Success;
}


ReturnCode
OS_fileSize(char * pFileName,
	    OS_Uint32 * pSize)
{
    struct stat		statbuf;

    if (stat(pFileName, &statbuf) < 0)
    {
	return FAIL_RC(OS_RC_NoSuchFile, ("OS_fileSize"));
    }

    *pSize = (OS_Uint32) statbuf.st_size;

    return Success;
}

ReturnCode
OS_getLocalHostName(char * buf,
		    OS_Uint16 bufLen)
{
#if 0
    struct utsname	sysinfo;

    if (uname(&sysinfo) == -1)
    {
	return Fail;
    }

    strncpy(buf, sysinfo.nodename, bufLen - 1);
    buf[bufLen - 1] = '\0';
#endif
	strcpy(buf,"dosbox");

    return Success;
}


int
OS_strncasecmp(const char *p1, const char *p2, int n)
{
    register int	c1;
    register int	c2;

    for (; n > 0; n--)
    {
	c1 = (int) *p1++;
	c2 = (int) *p2++;

	if (isupper(c1))
	{
	    c1 = tolower(c1);
	}

	if (isupper(c2))
	{
	    c2 = tolower(c2);
	}

	if (c1 == '\0' || c2 == '\0' || c1 != c2)
	{
	    break;
	}
    }

    return c2 - c1;
}

int
OS_strcasecmp(const char *p1, const char *p2)
{
    register int	c1;
    register int	c2;

    for (;;)
    {
	c1 = (int) *p1++;
	c2 = (int) *p2++;

	if (isupper(c1))
	{
	    c1 = tolower(c1);
	}

	if (isupper(c2))
	{
	    c2 = tolower(c2);
	}

	if (c1 == '\0' || c2 == '\0' || c1 != c2)
	{
	    break;
	}
    }

    return c2 - c1;
}

char *
OS_allocStringCopy(char *s)
{
	return strdup(s);
}


char *
OS_allocStringNCopy(char * pSourceString,
		    OS_Uint16 len)
{
    char *	    p;

    if (len > strlen(pSourceString))
    {
	len = strlen(pSourceString);
    }

    if ((p = OS_alloc(len + 1)) == NULL)
    {
	return NULL;
    }

    strncpy(p, pSourceString, len);
    p[len] = '\0';

    return p;
}
