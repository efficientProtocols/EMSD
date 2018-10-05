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


#include "tm.h"
#include "sf.h"

#ifdef unix
#include "sys/time.h"
#ifdef SOLARIS_2_5
     int gettimeofday(struct timeval *, void *);
#else
/*      int gettimeofday(struct timeval *); */
#endif /* SOLARIS_2_5 */
     struct tm *localtime_r(struct timeval *, struct tm *);
#endif



#ifdef SOLARIS_2_5
/* NOTYET, see upd_if.c, Temporary July 2001, Sol 8 */
int sigmask(int i){return 0;}
#endif

typedef struct
{
    int 		numFiles;
    struct dirent ** 	ppDirEntries;
    int			currentFile;
    char		returnedFileName[OS_MAX_FILENAME_LEN];
    char 		directoryName[1];
} DirectoryData;


static OS_Boolean    (* pfDirFilter)(const char * pFileName);

static int
scanDirectory(const char * dirname,
	      struct dirent *** namelist,
	      int (*selected)(const struct dirent *),
	      int (*dcomp)(const struct dirent * const *,
			   const struct dirent * const *));

static int
alphaSort(const struct dirent * const * d1,
	  const struct dirent * const * d2);



ReturnCode
OS_init(void)
{
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
    extern time_t   timezone;
    extern time_t   altzone;
    extern int      daylight;

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

    timezone = 0;
    altzone = 0;
    daylight = 0;

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
    pDateTime->year   = pUnixTime->tm_year + 1900;
    pDateTime->month  = pUnixTime->tm_mon + 1;
    pDateTime->day    = pUnixTime->tm_mday;
    pDateTime->hour   = pUnixTime->tm_hour;
    pDateTime->minute = pUnixTime->tm_min;
    pDateTime->second = pUnixTime->tm_sec;

    /* We did it! */
    return Success;
}

ReturnCode
OS_timestamp (char *strBuf, OS_Uint32 strLen, 
	      OS_Uint32 mask, char *moduleName, 
	      char *loc)
{
    struct timeval  tv;
    struct tm       tm;
    int             tmpLen = 0;
    char            tmpBuf[128];

#ifdef SOLARIS_2_5
    gettimeofday (&tv, NULL);
#else
    gettimeofday (&tv);
#endif

    localtime_r (&tv, &tm);

    sprintf (tmpBuf, "[%02d%02d%02d%02d%02d%02d%04d %1.16s %x %d %1.64s]\t",
	     tm.tm_year % 100, tm.tm_mon + 1, tm.tm_mday,
	     tm.tm_hour, tm.tm_min, tm.tm_sec, (int) (tv.tv_usec / 100),
	     moduleName ? moduleName : "null",
	     (int)mask, (int)getpid(), loc);

    tmpLen = strlen(tmpBuf) + 1;  /* allow for null terminater */
    strncpy (strBuf, tmpBuf, strLen < tmpLen ? strLen : tmpLen);

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
  struct timeval tv;

#ifdef SOLARIS_2_5
    gettimeofday (&tv, NULL);
#else
    gettimeofday (&tv);
#endif

  return(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
	Current min & sec in seconds
 */
OS_Uint32
OS_timeMinSec(void)
{
    struct timeval tv;
    struct tm      tm;

#ifdef SOLARIS_2_5
    gettimeofday (&tv, NULL);
#else
    gettimeofday (&tv);
#endif

    localtime_r (&tv, &tm);

    return (tm.tm_min * 60 + tm.tm_sec);
}

/*
	Current min & sec & hour in seconds
 */
OS_Uint32
OS_timeHourMinSec(void)
{
    struct timeval tv;
    struct tm      tm;

#ifdef SOLARIS_2_5
    gettimeofday (&tv, NULL);
#else
    gettimeofday (&tv);
#endif

    localtime_r (&tv, &tm);

    return (1000 * (tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec) + 
	    (int) (tv.tv_usec / 1000));
}

ReturnCode
OS_timeString (char *strBuf)
{
    struct timeval tv;
    struct tm      tm;
    char 	   tmpBuf[128];

#ifdef SOLARIS_2_5
    gettimeofday (&tv, NULL);
#else
    gettimeofday (&tv);
#endif

    localtime_r (&tv, &tm);

    sprintf (tmpBuf, "%02d:%02d:%02d.%01ld",
	     tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 100000);

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
    case ENAMETOOLONG:
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


void
OS_dirSetFilter(OS_Boolean (* pfFilter)(const char * pFileName))
{
    pfDirFilter = pfFilter;
}


static int
os_dirSelect(const struct dirent * pDirEnt)
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
	 scanDirectory(pDirectoryName, &pDirData->ppDirEntries,
		       os_dirSelect, alphaSort)) < 0)
    {
	OS_free(pDirData);
	return FAIL_RC(OS_RC_DirectoryRead,
		       ("OS_dirOpen: scanDirectory failed."));
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
	    *OS_DIR_PATH_SEPARATOR,
	    pDirData->ppDirEntries[pDirData->currentFile]->d_name);

    /*
     * Give 'em a pointer to the current file in the list.  We copy
     * it in case our allocation routine is different from the one
     * scanDirectory() uses.
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
    for (i = pDirData->numFiles - 1; i >= 0; i--)
    {
	OS_free(pDirData->ppDirEntries[i]);
    }

    if (pDirData->ppDirEntries != NULL)
    {
	OS_free(pDirData->ppDirEntries);
	pDirData->ppDirEntries = NULL;
    }

    /* Rescan the directory. */
    if ((pDirData->numFiles =
	 scanDirectory(pDirData->directoryName,
		       &pDirData->ppDirEntries,
		       os_dirSelect, alphaSort)) < 0)
    {
	pDirData->numFiles = 0;
	return FAIL_RC(OS_RC_DirectoryRead,
		       ("OS_dirFindNext: scandir failed, reason %d.",
			errno));
    }

    /* Reset our current file index */
    pDirData->currentFile = 0;

    return Success;
}


char *
OS_dirGetPath(void * hDirData)
{
    DirectoryData *	pDirData = hDirData;

    return pDirData->directoryName;
}


ReturnCode
OS_moveFile(char * pNewName, char * pOldName)
{
    char	    moveCommand[1024];

    /* this declaration is _supposed_ to be in <unistd.h> */
    extern int rename(const char *oldpath, const char *newpath);

    if (!strlen(pNewName) || !strlen(pOldName)) {
        printf("OS_moveFile: Insufficient arguments: arg1='%s', arg2='%s'\n",
	       pOldName, pNewName);
        return Fail;
    }

    printf("renaming '%s' to '%s'\n", pOldName, pNewName);

    /* Try to rename the file */
    if (rename(pOldName, pNewName) != 0)
    {

	printf("*** rename(%s, %s) failed, errno=%d; using mv instead ***\n",
	       pOldName, pNewName, errno);

	/* We couldn't rename it.  Run "mv" instead. */
	sprintf(moveCommand, "mv %s %s", pOldName, pNewName);
	if (system(moveCommand) != 0)
	{
	    return Fail;
	}
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
    struct utsname	sysinfo;

    if (uname(&sysinfo) == -1)
    {
	return Fail;
    }

    strncpy(buf, sysinfo.nodename, bufLen - 1);
    buf[bufLen - 1] = '\0';

    return Success;
}



char *
OS_allocStringCopy(char * pSourceString)
{
    char *	    p;

    if ((p = OS_alloc(strlen(pSourceString) + 1)) == NULL)
    {
	return NULL;
    }

    strcpy(p, pSourceString);

    return p;
}


char *
OS_allocStringNCopy(char * pSourceString,
		    OS_Uint16 len)
{
    char *	    p;

    if ((p = OS_alloc(len + 1)) == NULL)
    {
	return NULL;
    }

    strncpy(p, pSourceString, len);
    p[len] = '\0';

    return p;
}



/*
 * Copyright (c) 1983 Regents of the University of California.  All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement: This
 *    product includes software developed by the University of
 *    California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its
 *    contributors may be used to endorse or promote products
 *    derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)scandir.c	5.10 (Berkeley) 2/23/91";
#endif /* LIBC_SCCS and not lint */

/*
 * Scan the directory dirname calling select to make a list of
 * selected directory entries then sort using qsort and compare
 * routine dcomp.  Returns the number of entries and a pointer to a
 * list of pointers to struct dirent (through namelist). Returns -1
 * if there were any errors.
 */

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct
 * dirent without the d_name field, plus enough space for the name
 * with a terminating null byte (dp->d_namlen+1), rounded up to a 4
 * byte boundary.
 */
#undef DIRSIZ

#define DIRSIZ(dp)				\
    (sizeof(struct dirent) + strlen((dp)->d_name))

static int
scanDirectory(const char * dirname,
	      struct dirent *** namelist,
	      int (*selected)(const struct dirent *),
	      int (*dcomp)(const struct dirent * const *,
			   const struct dirent * const *))
{
    int				rc;
    register struct dirent *	d;
    register struct dirent * 	p;
    register struct dirent ** 	names;
    register size_t 		nitems;
    struct stat 		stb;
    long 			arraysz;
    DIR				dir;
    DIR * 			dirp = &dir;
    struct
    {
	struct dirent		    dirEntry;
	char			    extraSpace[OS_MAX_FILENAME_LEN];
/***/   char                        extraExtra[1024*8];      /* getdents was overwritting */
    } dirEnt;

    if ((dirp->dd_fd = open(dirname, 0)) < 0)
    {
	return -1;
    }

    if (fstat(dirp->dd_fd, &stb) < 0)
    {
	closedir(dirp);
	return(-1);
    }

    /*
     * estimate the array size by taking the size of the directory
     * file and dividing it by a multiple of the minimum size entry.
     */
    arraysz = (stb.st_size / 12);
    names = (struct dirent **)OS_alloc(arraysz * (128 + sizeof(struct dirent *)));

    if (names == NULL)
    {
	closedir(dirp);
	return(-1);
    }

    nitems = 0;

    d = (struct dirent *) &dirEnt;
    while ((rc = getdents(dirp->dd_fd, d, OS_MAX_FILENAME_LEN)) > 0)
    {
	for (; rc > 0; rc -= d->d_reclen, d = (struct dirent *) (((unsigned char *) d) + d->d_reclen))
	{
	    if (selected != NULL && !(*selected)(d))
	    {
		continue;	/* just selected names */
	    }

	    /*
	     * Make a minimum size copy of the data
	     */
	    p = (struct dirent *)OS_alloc(DIRSIZ(d));
	    if (p == NULL)
	    {
		closedir(dirp);
		return(-1);
	    }

	    p->d_ino = d->d_ino;
	    p->d_reclen = d->d_reclen;
	    strcpy(p->d_name, d->d_name);

	    /*
	     * Check to make sure the array has space left and
	     * realloc the maximum size.
	     */
	    if (++nitems >= arraysz)
		{
		    if (fstat(dirp->dd_fd, &stb) < 0)
			{
			    closedir(dirp);
			    return(-1);	/* just might have grown */
			}

		    arraysz = stb.st_size / 6;
		    names = (struct dirent **)OS_realloc((char *)names, 
			         (arraysz * (128 + sizeof(struct dirent *))));
		    if (names == NULL)
			{
			    close(dirp->dd_fd);
			    return(-1);
			}
		}
	    names[nitems-1] = p;
	}
    }

    close(dirp->dd_fd);

    if (nitems && dcomp != NULL)
    {
	qsort(names,
	      nitems,
	      sizeof(struct dirent *),
	      (int (*)(const void *, const void *)) dcomp);
    }

    *namelist = names;

    return(nitems);
}

/*
 * Alphabetic order comparison routine for those who want it.
 */
static int
alphaSort(const struct dirent * const * d1,
	  const struct dirent * const * d2)
{
    return(strcmp((*(struct dirent **)d1)->d_name,
		  (*(struct dirent **)d2)->d_name));
}

int sigsetmask (int mask)
{
    return mask;
}

int sigblock (int mask)
{
    return (mask);
}
