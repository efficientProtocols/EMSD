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
 * Author: Pean Lim
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
#include "bs.h"
#include "os.h"
#include "eh.h"
#include "tm.h"
#if defined(OS_VARIANT_WinCE)
#include <stdarg.h>
#endif

#ifdef TM_ENABLED
TM_ModuleCB *OS_tmDesc;	/* OS_ module's trace control block */
#endif

#define MAXNAMLEN	13

struct direct {
  int 	d_namlen;
  char	d_name[MAXNAMLEN];
};

#define dirent direct

typedef enum DirState {
    DirActive,
    DirVirgin,
    DirInactive
} DirState;

typedef struct {
    char 	dirName[128];	/* Name of the directory, including drive */
    char 	*dirNameEnd;	/* End of String Location for dirName */
    DirState 	state;		/* See DirState */
    HANDLE	hDir;		/* directory handle */
    long 	index;		/* Position of a file name inside the directory */
    char 	indexName[MAXNAMLEN];  /* File Name corresponding to indexName */
} DIR;

extern	DIR *opendir();
extern	struct direct *readdir();
extern	long telldir();
extern	void seekdir();
#define rewinddir(dirp)	seekdir((dirp), (long)0)
extern	void closedir();

typedef WIN32_FIND_DATA DirDta;

static char *
os_DirDta_name(DirDta *dirDta)
{
#if defined(OS_VARIANT_WinCE)
    int		dirlen;
    static char	name[OS_MAX_FILENAME_LEN];
    
    dirlen = lstrlen(dirDta->cFileName);
    if ( sizeof(name) < dirlen ) {
	EH_fatal("os_DirDta_name: dir name too big.");
	return(NULL);
    }
    else {
	wcstombs(name, dirDta->cFileName, sizeof(name));
	return(name);
    }
#else
    dirDta->name;
#endif
}


typedef struct DirsCB
{
    DirDta dirDta;
    DIR *lastActive;
    int opensCount;
} DirsCB;

static DirsCB 		dirsCB;
static char 		cwd[128];

typedef struct
{
    int 		numFiles;
    struct dirent ** 	ppDirEntries;
    int			currentFile;
    char		returnedFileName[OS_MAX_FILENAME_LEN];
    char 		directoryName[1];
} DirectoryData;


static OS_Boolean    (* pfDirFilter)(const char * pFileName);

static DIR *
opendir(char *fileName);

static struct direct *
readdir(DIR *dirp);

static void
closedir(DIR *dirp);

static long
telldir(DIR *dirp);

static void
seekdir(DIR *dirp,
	long loc);

static char *
getAbsPath(char *fileName);

static void
seekdir(DIR *dirp,
	long loc);

static SuccFail
findFirst(char *path, DirDta *dirDta, HANDLE *phDir);

static SuccFail
findNext(HANDLE hDir, DirDta *dirDta);

static int
findPath(char *path,
	 DirDta  *dirDta);

static int
findLoc(long loc,
	DirDta  *dirDta);

static int
scandir(const char *dirname,
	struct dirent *(*namelist[]),
	int (*select)(struct dirent *),
	int (*dcomp)(const void *, const void *));

static int
alphasort(const void * d1,
	  const void * d2);


static void
os_mangleFormatString(const char *fmt, char *newfmt, int len);

volatile int OS_isrActive;
volatile int OS_atomicOperationInProgress;

int
OS_isrIsActive(void)
{
    return OS_isrActive;
}


static SYSTEMTIME	initUTC;

ReturnCode
OS_init(void)
{

    if ( TM_OPEN(OS_tmDesc, "OS_") == NULL ) {
	EH_fatal("OS_init: TM_open failed.");
    }
    else {
	TM_SETUP("OS_,ffff");	/* always done */
    }


    GetSystemTime(&initUTC);
    OS_isrActive = FALSE;
    OS_atomicOperationInProgress = 0;
    return Success;
}

/* *** WARNING!! The following function doesn't handle overflow
 * situations.  We are shoehorning NT's 64-bit FILETIME (100ns since
 * 1/1/1601) into UNIX's 32-bit time_t (seconds since 1/1/1970).  */

ReturnCode
OS_currentDateTime(OS_ZuluDateTime * pDateTime,
		   OS_Uint32 * pJulianDate)
{
#if defined(OS_VARIANT_WinCE)
    SYSTEMTIME		currentSystemTime;
    FILETIME		currentFileTime;
    static FILETIME 	unixEpochFileTime;
    static LPFILETIME	pUnixEpochFileTime;
    static SYSTEMTIME	unixEpochSystemTime;
    static LPSYSTEMTIME	pUnixEpochSystemTime;
    FILETIME		currentUnixEpochFileTime;
    OS_Uint32		currentTime;

    if ( NULL == pUnixEpochFileTime ) {
	/* setup SYSTEMTIME for UNIX Epoch */
	unixEpochSystemTime.wYear = 1970;
	unixEpochSystemTime.wMonth = 1;
	unixEpochSystemTime.wDay = 1;
	unixEpochSystemTime.wHour = 0;
	unixEpochSystemTime.wMinute = 0;
	unixEpochSystemTime.wSecond = 0;
	unixEpochSystemTime.wMilliseconds = 0;

	pUnixEpochSystemTime = &unixEpochSystemTime;

	/* get the FILETIME for it */
	pUnixEpochFileTime = &unixEpochFileTime;
	SystemTimeToFileTime(pUnixEpochSystemTime,
			     pUnixEpochFileTime);
    }

    GetSystemTime(&currentSystemTime);
    SystemTimeToFileTime(&currentSystemTime,
			 &currentFileTime);

    if (pJulianDate != NULL) {
	/*
	 * low order 32 bits = num of nano secs * 100 since epoch
	 */
	currentUnixEpochFileTime.dwLowDateTime
	    = currentFileTime.dwLowDateTime - unixEpochFileTime.dwLowDateTime;
	/*
	 * high order 32 bits = num of nano secs * 100 * 4294967296 (i.e., 2^32)
	 * since epoch
	 */
	currentUnixEpochFileTime.dwHighDateTime
	    = currentFileTime.dwHighDateTime - unixEpochFileTime.dwHighDateTime;

	currentTime = (currentUnixEpochFileTime.dwLowDateTime / 10000000)
	    + (429.4967296 * currentUnixEpochFileTime.dwHighDateTime);

	*pJulianDate = currentTime;
    }

    if (pDateTime != NULL) {
	pDateTime->year		= currentSystemTime.wYear;
	pDateTime->month 	= currentSystemTime.wMonth;
	pDateTime->day 		= currentSystemTime.wDay;
	pDateTime->hour 	= currentSystemTime.wHour;
	pDateTime->minute	= currentSystemTime.wMinute;
	pDateTime->second	= currentSystemTime.wSecond;
    }

    return Success;
#else
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
#endif
}

#if NOTWCE

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


#endif /* NOTWCE */

/*
 * Return the OS_ZuluDateTime corresponding to julianDate (which is
 * seconds since UNIX Epoch).
 */
ReturnCode
OS_julianToDateStruct(OS_Uint32		julianDate, /* secs since UNIX epoch */
		      OS_ZuluDateTime 	*pDateTime)
{
#if defined(OS_VARIANT_WinCE)
    OS_Uint32		scratchLow32;
    OS_Uint32		scratchHigh32;
    static FILETIME	unixEpochFileTime;
    static LPFILETIME	pUnixEpochFileTime;
    SYSTEMTIME		unixEpochSystemTime;
    FILETIME		julianDateFileTime;
    SYSTEMTIME		julianDateSystemTime;


    /* onetime setup SYSTEMTIME for UNIX Epoch */
    if ( NULL == pUnixEpochFileTime ) {
	unixEpochSystemTime.wYear 	= 1970;
	unixEpochSystemTime.wMonth 	= 1;
	unixEpochSystemTime.wDay 	= 1;
	unixEpochSystemTime.wHour 	= 0;
	unixEpochSystemTime.wMinute 	= 0;
	unixEpochSystemTime.wSecond 	= 0;
	unixEpochSystemTime.wMilliseconds = 0;

	/* now get the FILETIME for it */
	if (FALSE == SystemTimeToFileTime(&unixEpochSystemTime, &unixEpochFileTime)) {
	    return FAIL_RC(OS_RC_InvalidDate, ("OS_julianToDateStruct: failed to get SystemTime for UNIX epoch"));
	}
	pUnixEpochFileTime = &unixEpochFileTime;
    }

    /*
     * convert seconds to 10^-7 seconds
     *
     * 10^7 == 2#100110001001011010000000 == (2^7+2^9+2^10+2^12+2^15+2^19+2^20+2^23)
     */

    scratchLow32 = 0L;
    scratchHigh32 = 0L;

    scratchHigh32 += julianDate >> 25; /* save high order 7 */
    scratchLow32 += julianDate << 7;

    scratchHigh32 += julianDate >> 23; /* save high order 9 */
    scratchLow32 += julianDate << 9;

    scratchHigh32 += julianDate >> 22; /* save high order 10 */
    scratchLow32 += julianDate << 10;

    scratchHigh32 += julianDate >> 20; /* save high order 12 */
    scratchLow32 += julianDate << 12;

    scratchHigh32 += julianDate >> 17; /* save high order 15 */
    scratchLow32 += julianDate << 15;

    scratchHigh32 += julianDate >> 13; /* save high order 19 */
    scratchLow32 += julianDate << 19;

    scratchHigh32 += julianDate >> 12; /* save high order 20 */
    scratchLow32 += julianDate << 20;

    scratchHigh32 += julianDate >> 9; /* save high order 23 */
    scratchLow32 += julianDate << 23;

    /* julianDateFileTime + unixEpochFileTime (avoid MSB overflow by dropping LSB) */

    scratchLow32 = (scratchLow32 >> 1) + (julianDateFileTime.dwLowDateTime >> 1);
    julianDateFileTime.dwLowDateTime = scratchLow32 << 1;
    julianDateFileTime.dwHighDateTime = ((scratchLow32 & 0x80000000) /* carry from dwLowDateTime */
					 + scratchHigh32
					 + unixEpochFileTime.dwHighDateTime);

    if ( FALSE == FileTimeToSystemTime(&julianDateFileTime, &julianDateSystemTime) ) {
	return FAIL_RC(OS_RC_InvalidDate, ("OS_julianToDateStruct: invalid date"));
    }

    /* copy the system time to pDateTime */
    if (pDateTime != NULL) {
	pDateTime->year		= julianDateSystemTime.wYear;
	pDateTime->month	= julianDateSystemTime.wMonth;
	pDateTime->day 		= julianDateSystemTime.wDay;
	pDateTime->hour 	= julianDateSystemTime.wHour;
	pDateTime->minute	= julianDateSystemTime.wMinute;
	pDateTime->second	= julianDateSystemTime.wSecond;
    }

    /* We did it! */
    return Success;
#else
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
#endif
}


ReturnCode
OS_timestamp(char	*strBuf,
	     OS_Uint32	strLen,
	     OS_Uint32	mask,
	     char	*moduleName,
	     char 	*loc)
{
    SYSTEMTIME	tm;
    OS_Uint32	tmpLen = 0;
    char	tmpBuf[128];

    GetLocalTime(&tm);

    sprintf(tmpBuf, "[%02d%02d%02d%02d%02d%02d %1.16s %x %1.64s]   ",
	    tm.wYear % 100, tm.wMonth, tm.wDay,
	    tm.wHour, tm.wMinute, tm.wSecond,
	    moduleName ? moduleName : "null",
	    mask,
	    loc);

    tmpLen = strlen(tmpBuf) + 1; /* allow for null terminator */
    if (tmpLen <= strLen)
    {
	strcpy(strBuf, tmpBuf);
    }
    else 
    {
	strncpy(strBuf, tmpBuf, (int) strLen);
	strBuf[strLen - 1] = '\0';
    }

    return Success;
}

#if NOTWCE

/*
 * OS_Uint32
 * OS_timeMsec(Void)
 *
 * Return the system time in milliseconds in a 32-bit number since
 * epoch.  As epoch may vary this counter may wrap as frequently as 7
 * times per year (UNIX) so don't use for absolute time. Mainly useful
 * for event relative timing.  */
OS_Uint32
OS_timeMsec(Void)
{
    SYSTEMTIME	nowUTC;

    GetSystemTime(&nowUTC);

  timeSec = time (&timeSec);

  return (timeSec * 1000);
}

#endif /* NOTWCE */

/*
	Current min & sec in seconds
 */
OS_Uint32
OS_timeMinSec(Void)
{
    SYSTEMTIME now;
    OS_Uint32 result;

    GetLocalTime(&now);
    result = (now.wMinute * 60) + (now.wSecond);
    return result;
}

/*
	Current hour & min & sec in seconds
 */
OS_Uint32
OS_timeHourMinSec(Void)
{
    SYSTEMTIME	now;
    OS_Uint32	result;

    GetLocalTime(&now);
    result = (now.wHour * 3600) + (now.wMinute * 60) + now.wSecond;
    return result;
}

#ifdef NOTWCE

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


#endif NOTWCE

ReturnCode
OS_deleteFile(char * pFileName)
{
#if defined(OS_VARIANT_WinCE)
    static WCHAR szFileName[256];
    DWORD dwLastErr;

    wsprintf(szFileName, TEXT("%S"), pFileName);

    if ( DeleteFile(szFileName) == TRUE ) {
	return Success;
    }

    dwLastErr = GetLastError();
    switch (dwLastErr) {

    case ERROR_FILE_NOT_FOUND:
	return FAIL_RC(OS_RC_NoSuchFile,
		       ("OS_deleteFile: no such file"));

    case ERROR_FILE_INVALID:
	return FAIL_RC(OS_RC_InvalidFileName,
		       ("OS_deleteFile: invalid file name"));
    default:
	return FAIL_RC(OS_RC_InvalidFileName,
		       ("OS_deleteFile: error"));
    }
#else
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
#endif /* OS_VARIANT_WinCE */
}


void
OS_dirSetFilter(OS_Boolean (* pfFilter)(const char * pFileName))
{
    pfDirFilter = pfFilter;
}


static int
os_dirSelect( struct dirent * pDirEnt)
{
    /* If no filter is set, include all files. */
    if (pfDirFilter == NULL)
	return 1;

    return (* pfDirFilter)(pDirEnt->d_name) ? 1 : 0;
}


ReturnCode
OS_dirOpen(char * pDirectoryName,
	   void ** hDirData)
{
    DirectoryData * pDirData;
    char *	    p;
    int 	    len;

    /*
     * Remove any trailing whitespace and directory path separators
     * from the directory name.
     */
    for (p = pDirectoryName + strlen(pDirectoryName) - 1;
	 (p > pDirectoryName &&
	  (isspace(*p) ||
	   strchr(OS_DIR_PATH_SEPARATOR, *p) != NULL));
	 p--)
     {
	 /* do nothing; just scan until we exit the loop somehow. */
     }

    /* Make sure we didn't get an empty string */
    if (p == pDirectoryName &&
	(*p == '\0' || strchr(OS_DIR_PATH_SEPARATOR, *p) == NULL))
    {
	return FAIL_RC(OS_RC_InvalidDirectory,
		       ("OS_dirOpen: invalid directory name"));
    }

    /* Null terminate at the new location. */
    *++p = '\0';

    /* Allocate a Directory Data structure */
    len = sizeof(DirectoryData) + (p - pDirectoryName);
    if ((pDirData = OS_alloc(len)) == NULL)
    {
	return FAIL_RC(ResourceError,
		       ("OS_dirOpen: alloc Directory Data"));
    }

    /* Save the directory name */
    strcpy(pDirData->directoryName, pDirectoryName);

    /*
     * In this implementation, we'll use scandir(), which searches
     * the complete directory, sorts the entries that we select for
     * inclusion, and stores them in an array of pointers to dirent
     * structures.
     */
    if ((pDirData->numFiles =
	 scandir(pDirectoryName, &pDirData->ppDirEntries,
		 os_dirSelect, alphasort)) < 0)
    {
	free(pDirData);
	return FAIL_RC(OS_RC_DirectoryRead,
		       ("OS_dirOpen: scandir failed."));
    }

    /* Specify the first file we'll give 'em */
    pDirData->currentFile = 0;

    /* Give 'em what they came for */
    *hDirData = pDirData;

    return Success;
}


ReturnCode
OS_dirFindNext(void * hDirData,
	       char ** ppFileName)
{
    DirectoryData *	pDirData = (DirectoryData *) hDirData;

    /* Are there any files in the list? */
    if (pDirData->currentFile == pDirData->numFiles)
	{
	    /* Nope.  Let 'em know. */
	    *ppFileName = NULL;
	    return Success;
	}

    /* Give 'em the full path name for the file */
    sprintf(pDirData->returnedFileName, "%s%c%s",
	    pDirData->directoryName,
	    OS_DIR_PATH_SEPARATOR[0],
	    pDirData->ppDirEntries[pDirData->currentFile]->d_name);

    TM_TRACE((OS_tmDesc, TM_ENTER, "OS_dirFindNext(): found <%s>", pDirData->returnedFileName));

    /*
     * Give 'em a pointer to the current file in the list.  We copy
     * it in case our allocation routine is different from the one
     * scandir() uses.
     */
    if ((*ppFileName =
	 OS_alloc(strlen(pDirData->returnedFileName) + 1)) == NULL)
	{
	    *ppFileName = NULL;
	    return ResourceError;
	}

    strcpy(*ppFileName, pDirData->returnedFileName);

    /* We've used the current file.  Move to the next one. */
    ++pDirData->currentFile;

    return Success;
}


ReturnCode
OS_dirRewind(void * hDirData)
{
    int			i;
    DirectoryData *	pDirData = (DirectoryData *) hDirData;

    /* Free previous file-name memory */
    for (i = pDirData->numFiles - 1; i >= 0; i--) {
	free(pDirData->ppDirEntries[i]);
    }

    if (pDirData->ppDirEntries != NULL)	{
	free(pDirData->ppDirEntries);
	pDirData->ppDirEntries = NULL;
    }

    /* Rescan the directory. */
    if ((pDirData->numFiles =
	 scandir(pDirData->directoryName,
		 &pDirData->ppDirEntries,
		 os_dirSelect, alphasort)) < 0)	{
	pDirData->numFiles = 0;
	return FAIL_RC(OS_RC_DirectoryRead,
		       ("OS_dirFindNext: scandir failed, reason %d.",
			errno));
    }

    /* Reset our current file index */
    pDirData->currentFile = 0;

    return Success;
}


#ifdef NOTWCE

char *
OS_dirGetPath(void * hDirData)
{
    DirectoryData *	pDirData = hDirData;

    return pDirData->directoryName;
}


ReturnCode
OS_moveFile(char * pNewName, char * pOldName)
{
    /* this declaration is _supposed_ to be in <unistd.h> */
    extern int rename(const char *oldpath, const char *newpath);

    /* Try to rename the file */
    if (rename(pOldName, pNewName) != 0)
    {
#if ! defined(OS_VARIANT_WinCE) && ! (defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Win16))
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

#endif /* NOTWCE */

ReturnCode
OS_fileSize(char *pFileName, OS_Uint32 *pSize)
{
    DWORD		dwFileSizeLow;	/* low order DWORD of the filesize */
    WCHAR		pwFileName[OS_MAX_FILENAME_LEN];
    HANDLE	hFile;

    wsprintf(pwFileName, TEXT("%S"), pFileName);
    hFile = CreateFile(pwFileName, GENERIC_READ, 0,
		       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if ( INVALID_HANDLE_VALUE == hFile ) {
	return FAIL_RC(OS_RC_NoSuchFile, ("OS_fileSize"));
    }

    dwFileSizeLow = GetFileSize(hFile, NULL);	/* NULL => ignore the high order DWORD */
    CloseHandle(hFile);

    if ( 0xFFFFFFFF == dwFileSizeLow ) {
	return FAIL_RC(OS_RC_NoSuchFile, ("OS_fileSize"));
    }

    *pSize = (OS_Uint32)dwFileSizeLow;

    return Success;
}

#ifdef NOTWCE

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
#endif /* 0 */
	strcpy(buf,"dosbox");

    return Success;
}

#endif /* NOTWCE */

/*
 * directory
 *
 * MS-DOS compatible version of diropen() et al.
 */

#if defined(OS_VARIANT_WinCE)

/* 
 * WinCE has no concept of cwd so we track it
 */
static char os_cwd[MAX_PATH] = "\\";

static int
chdir(char *new_cwd)
{
    sprintf(os_cwd, "%s", new_cwd);
    return 0;
}


static char *
getcwd(char *pt, size_t size)
{
    size_t	cwd_size;

    cwd_size = strlen(os_cwd) + 1;

    if ( pt ) {	
	if ( cwd_size <= size )
	    sprintf(pt, "%s", os_cwd);
	else
	    return(NULL);
    }
    else {
	pt = (char *)malloc(cwd_size);
	sprintf(pt, "%s", os_cwd);
    }
    return(pt); 
}
#endif /* OS_VARIANT_WinCE */


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static DIR *
opendir(char *fileName)
{
    DIR		*thisDir;
    DWORD	dwAttribs;
    WCHAR	pwFileName[OS_MAX_FILENAME_LEN];
    int 	i;

    wsprintf(pwFileName, TEXT("%S"), fileName);
    dwAttribs = GetFileAttributes(pwFileName);
    if (!( dwAttribs & FILE_ATTRIBUTE_DIRECTORY )) {
	EH_problem("Bad argument.  Not a directory.");
	return ( (DIR *) 0);
    }

    if ( strlen(fileName) >= sizeof(thisDir->dirName) ) {
	EH_problem("Bad argument.  Directory name too long.");
	return ( (DIR *) 0);
    }

    thisDir = (DIR *)malloc(sizeof(*thisDir));
    if ( ! thisDir ) {
	EH_problem("Malloc");
	return ( thisDir );
    }

    /* Keep Dir Name, It may be needed Later */
    strcpy(thisDir->dirName, getAbsPath(fileName));
    i = strlen(thisDir->dirName);
    thisDir->dirNameEnd = thisDir->dirName + i;

    thisDir->state = DirVirgin;
    thisDir->index = 0;
    strcpy(thisDir->indexName, "*.*");

    return ( thisDir );
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static struct direct *
readdir(DIR *dirp)
{
    static struct direct dir;
    struct direct *retVal;
    char * srcPtr;
    char * dstPtr;

    if (getcwd(cwd, sizeof(cwd)) == (char *)0) {
	perror("getcwd");
        exit(1);
    }
    if (chdir(dirp->dirName)) {
	perror("getcwd");
        exit(1);
    }

    switch ( dirp->state ) {
    case DirInactive:
	if ( findLoc(dirp->index, &dirsCB.dirDta) ) {
	    retVal = (struct direct *) 0;
            goto done;
	}
	if ( dirsCB.lastActive ) {
	    dirsCB.lastActive->state = DirInactive;
	}
	dirsCB.lastActive = dirp;
	dirp->state = DirActive;
	/* Now it is as if it was active,
	 * Also execute DirActive Code, do NOT break.
	 */

    case DirActive:
	if ( findNext(dirp->hDir, &dirsCB.dirDta) == Fail ) {
	    retVal = (struct direct *) 0;
	    goto done;
	}
	break;

    case DirVirgin:
	if ( findFirst(dirp->indexName, &dirsCB.dirDta, &dirp->hDir) == Fail ) {
	    retVal = (struct direct *) 0;
            goto done;
	}
	if ( dirsCB.lastActive ) {
	    dirsCB.lastActive->state = DirInactive;
	}
	dirsCB.lastActive = dirp;
	dirp->state = DirActive;
	break;

    default:
	EH_oops();
	break;
    }

    /* Now dirsCB.dirDta contains the information we need */
    dirp->index++;
    strcpy(dirp->indexName, os_DirDta_name(&dirsCB.dirDta));

    /* Do strcpy and change the case to lower */
    for (srcPtr=os_DirDta_name(&dirsCB.dirDta), dstPtr=dir.d_name;
	 *srcPtr;
	  ++srcPtr, ++dstPtr) {
	*dstPtr = tolower(*srcPtr);
    }
    *dstPtr = *srcPtr;	/* '\0' terminate it */

    dir.d_namlen = strlen(os_DirDta_name(&dirsCB.dirDta));
    retVal = &dir;

done:
    if (chdir(cwd)) {
	perror("chdir");
        exit(1);
    }

    return ( retVal );

}



/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static void
closedir(DIR *dirp)
{
    if ( dirp->state == DirActive ) {
	dirsCB.lastActive = (DIR *)0;
    }
    free(dirp);
}

#ifdef NOTWCE 


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static long
telldir(DIR *dirp)
{
      return ((long) dirp->index);
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static void
seekdir(DIR *dirp,
	long loc)
{
        dirp->index = loc;
}

#endif /* NOTWCE */


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static char *
getAbsPath(char *fileName)
{
#if defined(OS_VARIANT_WinCE)	/* WinCE does not have concept of cwd */
    static char absPath[128];
    sprintf(absPath, "%s", fileName);

    return (absPath);
#else
    static char absPath[128];

    if (getcwd(cwd, sizeof(cwd)) == (char *)0) {
	perror("getcwd");
        exit(1);
    }
    if (chdir(fileName)) {
	perror("chdir");
        exit(1);
    }
    if (getcwd(absPath, sizeof(absPath)) == (char *)0) {
	perror("getcwd");
        exit(1);
    }
    if (chdir(cwd)) {
	perror("chdir");
        exit(1);
    }
    return (absPath);
#endif
}


/*<
 * Function:
 * Description:
 * findFirst - find first file in chosen directory
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static SuccFail
findFirst(char *path, DirDta *dirDta, HANDLE *phDir)
{
    WCHAR	wpath[128];
    HANDLE	hDir;

    wsprintf(wpath, TEXT("%S%C%S"),
	     getcwd((char *)NULL, (size_t)0),
	     OS_DIR_PATH_SEPARATOR[0],
	     path);

    hDir = FindFirstFile(wpath, dirDta);
    if ( hDir == INVALID_HANDLE_VALUE ) {
	*phDir = NULL; 
	return(Fail);
    }
    else {
	*phDir = hDir;
	return(Success);
    }
}




/*<
 * Function:
 * Description:
 * findNext - find the next file in the same directory
 *
 * Arguments:
 *
 * Returns:	SuccFail
 *
>*/

static SuccFail
findNext(HANDLE hDir,		/* search handle */
	 DirDta *dirDta)
{
    if ( FindNextFile(hDir, dirDta) )
	return(Success); 
    else
	return(Fail);
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static int
findPath(char *path,
	 DirDta  *dirDta)
{
    HANDLE hDir;

    if ( findFirst("*.*", dirDta, &hDir) == Fail ) {
	return ( -1 );
    }

    while ( 1 ) {
	if  ( findNext(hDir, dirDta) == Fail ) {
	    return ( -1 );
        }
        if ( ! strcmp(path, os_DirDta_name(dirDta)) ) {
	    return ( 0 );
        }
    }
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
static int
findLoc(long loc,
	DirDta  *dirDta)
{
    HANDLE hDir;
    long i;

    if ( findFirst("*.*", dirDta, &hDir) == Fail ) {
	return ( -1 );
    }

    for (i = 0; i < loc-1; ++i) {
	if  ( findNext(hDir, dirDta) == Fail ) {
	    return ( -1 );
        }
    }
    return ( 0 );
}


/*
 * scandir
 *
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct dirent (through namelist). Returns -1 if there were any errors.
 */

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct dirent
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp) \
    ((sizeof (struct dirent) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

static int
scandir(const char *dirname,
	struct dirent *(*namelist[]),
	int (*select)(struct dirent *),
	int (*dcomp)(const void *, const void *))
{
	register struct dirent *d, *p, **names;
	register size_t nitems;
	long arraysz;
	DIR *dirp;

	if ((dirp = opendir((char *) dirname)) == NULL)
		return(-1);

	/*
	 * Calculate the array size by counting the number of entries. This
	 * is really the only way to do this under DOS
	 */
	for (arraysz = 0; (d = readdir(dirp)) != NULL; arraysz++)
            ;

	names = (struct dirent **)
		malloc((int) (arraysz * sizeof(struct dirent *)));
	if (names == NULL)
		return(-1);

	/*
	 * Now reinit the directory
         */
	closedir(dirp);
	if ((dirp = opendir((char *) dirname)) == NULL)
		return(-1);

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !(*select)(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			return(-1);
		p->d_namlen = d->d_namlen;
		BS_memCopy(d->d_name, p->d_name, p->d_namlen + 1);
		++nitems;
		names[nitems-1] = p;
	}
	closedir(dirp);
	if (nitems && dcomp != NULL)
		qsort(names, nitems, sizeof(struct dirent *), dcomp);
	*namelist = names;
	return(nitems);
}

/*
 * Alphabetic order comparison routine for those who want it.
 */
static int
alphasort(const void * d1,
	  const void * d2)
{
	return(strcmp((*(struct dirent **)d1)->d_name,
	    (*(struct dirent **)d2)->d_name));
}

int
OS_strncasecmp(const char *p1, const char *p2, int n)
{
#if defined(OS_VARIANT_WinCE)
    WCHAR	*pw1;
    WCHAR	*pw2;
    int		result;

    pw1 = (WCHAR *)OS_alloc(sizeof(WCHAR) * (strlen(p1) + 1));
    pw2 = (WCHAR *)OS_alloc(sizeof(WCHAR) * (strlen(p2) + 1));

    wsprintf(pw1, TEXT("%S"), p1);
    wsprintf(pw2, TEXT("%S"), p2);

    result = CompareString(LOCALE_SYSTEM_DEFAULT,
			   NORM_IGNORECASE,
			   pw1, n,
			   pw2, n);

    OS_free(pw1);
    OS_free(pw2);

    return result - 2;
#else
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
#endif
}


int
OS_strcasecmp(const char *p1, const char *p2)
{
#if defined(OS_VARIANT_WinCE)
    WCHAR	*pw1;
    WCHAR	*pw2;
    int		result;

    pw1 = (WCHAR *)OS_alloc(sizeof(WCHAR) * (strlen(p1) + 1));
    pw2 = (WCHAR *)OS_alloc(sizeof(WCHAR) * (strlen(p2) + 1));

    wsprintf(pw1, TEXT("%S"), p1);
    wsprintf(pw2, TEXT("%S"), p2);

    result = CompareString(LOCALE_SYSTEM_DEFAULT,
			   NORM_IGNORECASE,
			   pw1, -1,
			   pw2, -1);

    OS_free(pw1);
    OS_free(pw2);

    return result - 2;
#else
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
#endif
}

char *
OS_allocStringCopy(char *orig)
{
    int	len = strlen(orig) + 1;
    int	idx;
    char *dup;

    dup = LocalAlloc(LMEM_FIXED, (len * sizeof(char)));
    if ( dup ) {
	for (idx = 0; idx < len; idx++)
	    dup[idx] = orig[idx];
    }

    return dup;
}

#ifdef NOTWCE


char *
OS_allocStringNCopy(char *pSourceString, OS_Uint16 len)
{
    char *	    p;

    if (len > strlen(pSourceString)) {
	len = strlen(pSourceString);
    }

    if ((p = OS_alloc(len + 1)) == NULL) {
	return NULL;
    }

    strncpy(p, pSourceString, len);
    p[len] = '\0';

    return p;
}

#endif /* NOTWCE */

/*
 * =====================================================================
 * WinCE UNIX functions -- quick and dirty
 * =====================================================================
 */

/* --------------------------------------------------------------------
 * file functions
 * --------------------------------------------------------------------
 */

FILE *
fopen(const char *file, const char *mode)
{
    HANDLE	hFile;
    char	*modeChar;
    WCHAR 	szFileName[256];
    DWORD	dwDesiredAccess = 0;
    DWORD	dwCreationDistribution;
    Bool	isRead = FALSE;	/* default */

    wsprintf(szFileName, TEXT("%S"), file);

    TM_TRACE((OS_tmDesc, TM_ENTER, "entering fopen() for <%s> with mode <%s>", file, mode)); // debug

    for (modeChar = (char *)mode; *modeChar ; modeChar++) {
	switch ( *modeChar ) {
	case 'r':
	    isRead = TRUE;
	    dwDesiredAccess = dwDesiredAccess | GENERIC_READ;
	    dwCreationDistribution = OPEN_EXISTING;
	    break;
	case 'w':
	    isRead = FALSE;
	    dwDesiredAccess = dwDesiredAccess | GENERIC_WRITE;
	    dwCreationDistribution = CREATE_ALWAYS;
	    break;
	case '+':
	    dwDesiredAccess = dwDesiredAccess | GENERIC_WRITE;
	    dwCreationDistribution = isRead?OPEN_EXISTING:CREATE_ALWAYS;
	    break;
	default:
	    TM_TRACE((OS_tmDesc, TM_ENTER, "fopen(): mode <%c> not supported--ignoring", *modeChar));
	    break;
	}
    }

    hFile = CreateFile(szFileName,
		       dwDesiredAccess, 0, NULL,
		       dwCreationDistribution, FILE_ATTRIBUTE_NORMAL, NULL);
    if ( INVALID_HANDLE_VALUE == hFile ) {
	TM_TRACE((OS_tmDesc, TM_ENTER, "fopen(): returning NULL")); // *** debug
	return(NULL);
    }
    else {
	TM_TRACE((OS_tmDesc, TM_ENTER, "fopen(): returning %u", hFile)); // *** debug
	return ((FILE *)hFile);
    }
}


int
fclose(FILE *fp)
{
    HANDLE	hFile = (HANDLE)fp;

    if (CloseHandle(hFile))
	return 0;
    else
	return UNIX_EOF;
}


char *
fgets(char *buf, int n, FILE *fp)
{
    HANDLE hFile = (HANDLE)fp;
    BOOL res;
    DWORD bytesRead;
    register char *s;
    register char ch;

    if (n == 0)		/* sanity check */
	return (NULL);

    s = buf;
    n--;			/* leave space for NUL */
    while (n != 0) {

#define BYTES_TO_READ 1

	/* Scan through at most n bytes looking for '\n'.  If found,
	 * copy up to and including newline, and stop.  Otherwise,
	 * copy entire chunk and loop.  */

	res = ReadFile(hFile, s, BYTES_TO_READ, &bytesRead, NULL);
	if (( res == FALSE ) || (bytesRead != BYTES_TO_READ))
	    return NULL; /* ReadFile() failed */

	ch = *s;
	if ('\n' == ch) {
	    *s = 0;
	    break;
	}

	n--;
	s++;
    }

    if ( n == 0 ) {
	*s = 0;
    }

    return (buf);
}


int
fputs(const char *s, FILE *fp)
{
    HANDLE	hFile = (HANDLE)fp;
    DWORD	dwBytesWritten;

    if ( WriteFile(hFile, s, strlen(s), &dwBytesWritten, NULL) )
	return (int)dwBytesWritten;
    else
	return UNIX_EOF;
}


/* fputc() */
int
fputc(int c, FILE *fp)
{
    HANDLE	hFile = (HANDLE)fp;
    DWORD 	dwBytesWritten;

    /* if output to stdout or stderr, use OS_tmDesc */
    if (( fp == stdout ) || ( fp == stderr )) {
	TM_TRACE((OS_tmDesc, TM_ENTER, "%c", c));
	return TRUE;
    }
    else {
	if ( WriteFile(hFile, &c, sizeof(char)*1, &dwBytesWritten, NULL) )
	    return c;
	else
	    return UNIX_EOF;
    }
}


size_t
fread(void *buf, size_t size, size_t count, FILE *fp)
{
    HANDLE	hFile = (HANDLE)fp;
    DWORD 	dwBytesToRead = count * size;
    DWORD	dwBytesRead;

    if ( ReadFile(hFile, buf, dwBytesToRead, &dwBytesRead, NULL) )
	return (size_t)dwBytesRead;
    else
	return 0;
}


/*
 * Seek the given file to the given offset.
 * `Whence' must be one of the three SEEK_* macros.
 */
int
fseek(FILE *fp, long offset, int whence)
{
#define SFP_FAILURE 0xFFFFFFFF

    HANDLE	hFile = (HANDLE)fp;
    DWORD	result = SFP_FAILURE;

    switch ( whence ) {
    case SEEK_SET:
	/* set file pointer to offset */
	result = SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
	break;
    case SEEK_CUR:
	/* set file pointer to curr+offset */
	result = SetFilePointer(hFile, offset, NULL, FILE_CURRENT);
	break;
    case SEEK_END:
	/* set file pointer to end */
	result = SetFilePointer(hFile, offset, NULL, FILE_END);
	break;
    default:
	break;
    }

    if ( SFP_FAILURE == result )
	return -1;
    else
	return 0;
}


/* return the current offset */
long
ftell(FILE *fp)
{
    HANDLE hFile = (HANDLE)fp;
    DWORD result;

    result = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
    return (long)result;
}



/*
 * Write `count' objects (each size `size') from memory to the given file.
 * Return the number of whole objects written.
 */
size_t
fwrite(const void *buf, size_t size, size_t count, FILE *fp)
{
    HANDLE	hFile = (HANDLE)fp; 
    DWORD	dwBytesToWrite = size * count;
    DWORD	dwBytesWritten;

    if ( WriteFile(hFile, buf, dwBytesToWrite, &dwBytesWritten, NULL) )
	return (size_t)(dwBytesWritten / size);
    else
	return 0;
}


void
rewind(FILE *stream)
{
    fseek(stream, 0L, SEEK_SET); 
}


void
setbuf(FILE *fp, char *buf)
{
    TM_TRACE((OS_tmDesc, TM_DEBUG, "setbuf(): not supported!"));
}


/* ---------------------------------------------------------------------
 * miscellaneous system functions
 * ---------------------------------------------------------------------
 */

/*
 * returns the local time in a 26 char string
 * e.g.,  Fri Sep 13 00:00:00 1986\n\0
 */

char *
ctime(const time_t *clock)	/* seconds since UNIX epoch */
{
    OS_ZuluDateTime	zdt;
    SYSTEMTIME		sysTime;
    FILETIME		fileTime;
    SYSTEMTIME		lclTime;
    FILETIME		lclFileTime;

    OS_julianToDateStruct(*clock, &zdt); /* cvt to GMT */

    sysTime.wYear 	= zdt.year;
    sysTime.wMonth 	= zdt.month;
    sysTime.wDay	= zdt.day;
    sysTime.wHour	= zdt.hour;
    sysTime.wMinute	= zdt.minute;
    sysTime.wSecond	= zdt.second;

    SystemTimeToFileTime(&sysTime, &fileTime);
    FileTimeToLocalFileTime(&fileTime, &lclFileTime);
    FileTimeToSystemTime(&lclFileTime, &lclTime);

    {
	/* "inspired" by 4.4 BSD */
	static const char wday_name[7][3] = {"Sun", "Mon", "Tue", "Wed",
					     "Thu", "Fri", "Sat"};
	static const char mon_name[12][3] = {"Jan", "Feb", "Mar", "Apr",
					     "May", "Jun", "Jul", "Aug",
					     "Sep", "Oct", "Nov", "Dec"};
	static char	result[26];
	int len;

	(void) sprintf(result, "%.3S %.3S%3d %02.2d:%02.2d:%02.2d %d\n",
		       wday_name[lclTime.wDayOfWeek],
		       mon_name[lclTime.wMonth - 1],
		       lclTime.wDay,
		       lclTime.wHour,
		       lclTime.wMinute,
		       lclTime.wSecond,
		       lclTime.wYear);
	len = strlen(result);
	return result;
    }
}


/* ---------------------------------------------------------------------
 * character functions
 * ---------------------------------------------------------------------
 */

/* *** NOTYET, but for now... */
int
isalnum(int c)
{
    char ch = (char)c;

    return (( ch >= 'A' && ch <='Z' )
	    || ( ch >= 'a' && ch <='z' )
	    || isdigit(c));
}


/* *** NOTYET, but for now... */
int
isdigit(int c)
{
    char ch = (char)c;

    return ( ch >= '0') && ( ch <= '9' );
}


/* *** NOTYET, but for now... */
int
islower(int c)
{
    char ch = (char)c;

    return ( ch >= 'a') && ( ch <= 'z' );
}


/* *** NOTYET, but for now... */
int
isupper(int c)
{
    char ch = (char)c;

    return ( ch >= 'A') && ( ch <= 'Z' );
}


/* *** NOTYET, but for now... */
int
isspace(int c)
{
    if (((int)c == (int)' ')	/* space */
	|| ((int)c == (int)'\t') /* tab */
	|| ((int)c == (int)'\r') /* carriage return */
	|| ((int)c == (int)'\n') /* new line */
	|| ((int)c == (int)'\v') /* vert tab */
	)
	return TRUE;
    else
	return FALSE;
}


/* *** NOTYET, but for now... */
int				/* return non-zero for true, zero for false */
isprint(int c)
{
    /* true if in range of ' ' and '~' */
    if (((int)c >= (int)' ') && ((int)c <= (int)'~'))
	return TRUE;
    else
	return FALSE;
}


/* *** NOTYET, but for now ... */
int
toupper(int c)
{
    int	mapped = c;

    if ( islower(c) ) {
	mapped = c - ('a' - 'A');
    }
    return mapped;
}


/* *** NOTYET, but for now ... */
int
tolower(int c)
{
    int	mapped = c;

    if ( isupper(c) ) {
	mapped = c + ('a' - 'A');
    }
    return mapped;
}



/* ---------------------------------------------------------------------
 * string functions
 * ---------------------------------------------------------------------
 */


/* NOTYET but for now... */
int
sscanf(const char *str, char const *fmt, ...)
{
    TM_TRACE((OS_tmDesc, TM_ENTER, "*** sscanf(): not yet implemented!"));
    return 0;
}

/* fprintf() */
int
fprintf(FILE *fp, const char *fmt, ...)
{
    va_list ap;
    int len;
    static char str[OS_WCE_FMT_MAXLEN];
    DWORD 	dwBytesWritten;

    va_start(ap, fmt);
    len = vsprintf(str, fmt, ap);
    va_end(ap);
    /* if output to stdout or stderr, use OS_tmDesc */
    if (( fp == stdout ) || ( fp == stderr )) {
	TM_TRACE((OS_tmDesc, TM_ENTER, str));
    }
    else {
	WriteFile(fp, str, sizeof(char)*strlen(str), &dwBytesWritten, NULL);
    }
    return(len);
}


/* printf() */
int
printf(const char *fmt, ...)
{
    va_list ap;
    int len;
    static char str[OS_WCE_FMT_MAXLEN];

    va_start(ap, fmt);
    len = vsprintf(str, fmt, ap);
    TM_TRACE((OS_tmDesc, TM_ENTER, str));
    va_end(ap);
    return(len);
}


void
perror(const char *msg)
{
    /* *** TODO: check for possible integration with GetLastError() */

    TM_TRACE((OS_tmDesc, TM_ENTER, "perror(): called."));
    TM_TRACE((OS_tmDesc, TM_ENTER, (char *)msg));
}


char *
strrchr(const char *p, int ch)
{
    register char *save;

    for (save = NULL;; ++p) {
	if (*p == ch)
	    save = (char *)p;
	if (!*p)
	    return(save);
    }
    /* NOTREACHED */
}

/*
 * Find the first occurrence in s1 of a character in s2 (excluding NUL).
 */
char *
strpbrk(const char *s1, const char *s2)
{
    register const char *scanp;
    register int c, sc;

    while ((c = *s1++) != 0) {
	for (scanp = s2; (sc = *scanp++) != 0;)
	    if (sc == c)
		return ((char *)(s1 - 1));
    }
    return (NULL);
}


/* --------------------------------------------------------------------- 
 * internal helper functions                                             
 * --------------------------------------------------------------------- 
 */

/* currently just converts %s to %S; and \n to \r\n */
static void
os_mangleFormatString(const char *fmt, char *newfmt, int len)
{
    int		pct_pos = INT_MAX;	/* the latest % char */
    Bool	pct_s_found = FALSE;
    Bool	pct_c_found = FALSE;
    Bool	lf_found = FALSE; /* \n found? */
    int		fmt_idx;
    int		newfmt_idx;

    /* mangle the fmt string */
    for (fmt_idx = 0, newfmt_idx = 0;
	 (fmt_idx < len) && (newfmt_idx < len);
	 fmt_idx++, newfmt_idx++) {

	switch ( fmt[fmt_idx] ) {
	case '\n':
	    lf_found = TRUE;
	    break;
	case '%':
	    pct_pos = fmt_idx;
	    break;
	case 's':
	    if ( pct_pos == fmt_idx - 1 ) pct_s_found = TRUE;
	    break;
	case 'c':
	    if ( pct_pos == fmt_idx - 1 ) pct_c_found = TRUE;
	    break;
	default:
	    break;
	}

	if ( pct_s_found ) {
	    newfmt[newfmt_idx] = 'S';
	    pct_s_found = FALSE; /* reset */
	}
	else if ( pct_c_found ) {
	    newfmt[newfmt_idx] = 'C';
	    pct_c_found = FALSE; /* reset */
	}
	else if ( lf_found ) {
	    newfmt[newfmt_idx++] = '\r';
	    newfmt[newfmt_idx] = '\n';
	    lf_found = FALSE; /* reset */
	}
	else {
	    newfmt[newfmt_idx] = fmt[fmt_idx];
	}
    }
    newfmt[newfmt_idx] = '\0';		/* null terminate */
}
