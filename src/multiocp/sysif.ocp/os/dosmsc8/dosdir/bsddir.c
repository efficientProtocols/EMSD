/*+
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)bsddir.c	1.1    Released: 1/31/88";
#endif /*}*/

#include <stdio.h>
#include <sys/types.h>
/*#include <sys/dir.h>*/
#include <dir.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dos.h>
#include <direct.h>
#include "dosos.h"

typedef struct DirsCB {
    DirDta dirDta;
    DIR *lastActive;
    int opensCount;
} DirsCB;

static DirsCB dirsCB;
static char  cwd[128];



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
opendir(fileName)
char *fileName;
{
    DIR *thisDir;
    int i;
    struct stat	buf;
    extern char *getAbsPath();
#define	isdir(path) (stat(path, &buf) ? 0 : (buf.st_mode&S_IFMT)==S_IFDIR)

#if 0
    printf("opendir fileName=%s\n", fileName);
#endif
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
struct direct *
readdir( dirp )
DIR *dirp;
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
    strcpy(dirp->indexName, dirsCB.dirDta.fname);

    /* Do strcpy and change the case to lower */
    for (srcPtr=dirsCB.dirDta.fname, dstPtr=dir.d_name; *srcPtr;
	  ++srcPtr, ++dstPtr) {
	*dstPtr = tolower(*srcPtr);
    }
    *dstPtr = *srcPtr;	/* '\0' terminate it */

    dir.d_namlen = strlen(dirsCB.dirDta.fname);
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
closedir( dirp )
DIR *dirp;
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
telldir( dirp )
DIR *dirp;
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
void
seekdir( dirp, loc )
DIR *dirp;
long loc;
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
getAbsPath(fileName)
char *fileName;
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
