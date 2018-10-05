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
#if defined(OS_VARIANT_WinCE) || defined(OS_VARIANT_WinNT)
#include <stdarg.h>
#endif


#include "direct.h"
#include "dir.h"

#ifdef TM_ENABLED
TM_ModuleCB *OS_tmDesc;	/* OS_ module's trace control block */
#endif

typedef WIN32_FIND_DATA DirDta;

static char *
os_DirDta_name(DirDta *dirDta)
{
#if defined(OS_VARIANT_WinCE) || defined(OS_VARIANT_WinNT)
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


char *
getAbsPath(char *fileName);

void
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
DIR *
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
struct direct *
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
void
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
long
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
char *
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

int
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
int
alphasort(const void * d1,
	  const void * d2)
{
	return(strcmp((*(struct dirent **)d1)->d_name,
	    (*(struct dirent **)d2)->d_name));
}
