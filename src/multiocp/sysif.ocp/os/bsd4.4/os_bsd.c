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


typedef struct
{
    int 		numFiles;
    struct dirent ** 	ppDirEntries;
    int			currentFile;
    char		returnedFileName[OS_MAX_FILENAME_LEN];
    char 		directoryName[1];
} DirectoryData;


static OS_Boolean    (* pfDirFilter)(const char * pFileName);



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
    unixTime.tm_gmtoff = 0;

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
OS_timestamp (char *strBuf,
	      OS_Uint32 strLen,
	      OS_Uint32 mask,
	      char *moduleName,
	      char *loc)
{
    struct timeval  tv;
    struct tm *     pTm;
    time_t	    t;
    int             tmpLen = 0;
    char            tmpBuf[128];

    gettimeofday(&tv, NULL);
    t = tv.tv_sec;
    pTm = localtime(&t);

    sprintf (tmpBuf, "[%02d%02d%02d%02d%02d%02d%04d %1.16s %x %d %1.64s]\t",
	     pTm->tm_year % 100, pTm->tm_mon + 1, pTm->tm_mday,
	     pTm->tm_hour, pTm->tm_min, pTm->tm_sec, (int) (tv.tv_usec / 100),
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
OS_timeMsec(void)
{
  struct timeval tv;

    gettimeofday (&tv, NULL);

  return(tv.tv_sec * 1000 + tv.tv_usec / 1000);
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
	    *OS_DIR_PATH_SEPARATOR,
	    pDirData->ppDirEntries[pDirData->currentFile]->d_name);

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
    for (i = pDirData->numFiles - 1; i >= 0; i--)
    {
	free(pDirData->ppDirEntries[i]);
    }

    if (pDirData->ppDirEntries != NULL)
    {
	free(pDirData->ppDirEntries);
	pDirData->ppDirEntries = NULL;
    }

    /* Rescan the directory. */
    if ((pDirData->numFiles =
	 scandir(pDirData->directoryName,
		 &pDirData->ppDirEntries,
		 os_dirSelect, alphasort)) < 0)
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

    /* Try to rename the file */
    if (rename(pOldName, pNewName) != 0)
    {

	printf("*** rename(%s, %s) failed, errno=%d; "
	       "using mv instead ***\n",
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
