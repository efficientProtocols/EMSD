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

static char *
getAbsPath(char *fileName);

static int
findFirst(char *path,
	  DirDta  *dirDta,
	  unsigned int mask);

static int
findNext(DirDta *dirDta);

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


/*
 * directory
 *
 * MS-DOS compatible version of diropen() et al.
 */

/*
 * Author: Mohsen Banan.
 * History:
 *
 */


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
    DIR *thisDir;
    int i;
    struct stat	buf;
    extern char *getAbsPath();
#define	isdir(path) (stat(path, &buf) ? 0 : (buf.st_mode&S_IFMT)==S_IFDIR)

    if ( ! isdir(fileName) ) {
	EH_problem("Bad argument");
 	return ( (DIR *) 0);
    }
    if ( strlen(fileName) >= sizeof(thisDir->dirName) ) {
	EH_problem("Bad argument");
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
	if ( findNext(&dirsCB.dirDta) ) {
	    retVal = (struct direct *) 0;
            goto done;
	}
	break;

    case DirVirgin:
	if ( findFirst(dirp->indexName, &dirsCB.dirDta, 0x3F) ) {
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
    strcpy(dirp->indexName, dirsCB.dirDta.name);

    /* Do strcpy and change the case to lower */
    for (srcPtr=dirsCB.dirDta.name, dstPtr=dir.d_name; *srcPtr;
	  ++srcPtr, ++dstPtr) {
	*dstPtr = tolower(*srcPtr);
    }
    *dstPtr = *srcPtr;	/* '\0' terminate it */

    dir.d_namlen = strlen(dirsCB.dirDta.name);
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
static int
findFirst(char *path,
	  DirDta  *dirDta,
	  unsigned int mask)
{

	return	_dos_findfirst(path, mask, dirDta);
}




/*<
 * Function:
 * Description:
 * findNext - find the next file in the same directory
 *
 * Arguments:
 *
 * Returns:
 *
>*/

static int
findNext(DirDta *dirDta)
{
	return _dos_findnext(dirDta);
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
    if ( findFirst("*.*", dirDta, 0x3F) ) {
	return ( -1 );
    }

    while ( 1 ) {
	if  ( findNext(dirDta) ) {
             return ( -1 );
        }
        if ( ! strcmp(path, dirDta->name) ) {
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
    long i;
    if ( findFirst("*.*", dirDta, 0x3F) ) {
	return ( -1 );
    }

    for (i = 0; i < loc-1; ++i) {
	if  ( findNext(dirDta) ) {
             return ( -1 );
        }
    }
    return ( 0 );
}


/*
 * scandir
 * 
 * MS-DOS compatible version of scandir(3B).
 *
 * Uses MS-DOS compatible directory(3C) functions.
 *
 */


/*
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
		bcopy(d->d_name, p->d_name, p->d_namlen + 1);
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
