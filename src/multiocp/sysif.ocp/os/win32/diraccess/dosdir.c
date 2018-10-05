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
static char sccs[] = "@(#)dosdir.c	1.1    Released: 1/31/88";
#endif /*}*/

#include <stdio.h>
#include <dos.h>
#include "dosos.h"
/*#include <sys/dir.h>*/
#include <dir.h>


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
int
findFirst(path, dirDta, mask)
char *path;
DirDta  *dirDta;
unsigned int mask;
{
    union REGS inRegs;
    union REGS outRegs;
    struct SREGS segRegs;
    struct DOSERROR dosErr;

    inRegs.x.ax = 0x1A00;	/* DOS interrupt 1A */
    inRegs.x.dx = GET_OFF(dirDta);
    segRegs.ds = GET_SEG(dirDta);
    
    intdosx(&inRegs, &outRegs, &segRegs);    /* Set Data Transfer Address */

    inRegs.x.ax = 0x4E00;	/* DOS interrupt 4E */
    inRegs.x.cx = mask;
    inRegs.x.dx = GET_OFF(path);
    segRegs.ds = GET_SEG(path);
    intdosx(&inRegs, &outRegs, &segRegs);    /* dirDta contains the result */

    if ( outRegs.x.cflag ) {
	dosexterr(&dosErr);
	/* Code 2 is File Not Found */
	if ( dosErr.exterror != 2 ) {
	    printf("exterror=%d, class=%d, action=%d, locus=%d\n",
	        dosErr.exterror, dosErr.class, dosErr.action, dosErr.locus);
	}
	return ( -1 );
    }
    return ( 0 );
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

findNext( dirDta )
DirDta *dirDta;
{
    union REGS inRegs;
    union REGS outRegs;
    struct SREGS segRegs;
    struct DOSERROR dosErr;

    inRegs.x.ax = 0x1A00;	/* DOS interrupt 1A */
    inRegs.x.dx = GET_OFF(dirDta);
    segRegs.ds = GET_SEG(dirDta);
    
    intdosx(&inRegs, &outRegs, &segRegs);    /* Set Data Transfer Address */

    inRegs.x.ax = 0x4F00;	/* DOS interrupt 4E */
    intdosx(&inRegs, &outRegs, &segRegs);    /* dirDta contains the result */

    if ( outRegs.x.cflag ) {
	dosexterr(&dosErr);
	/* Code 18 is No more files */
	if ( dosErr.exterror != 18 ) {
	    printf("exterror=%d, class=%d, action=%d, locus=%d\n",
	        dosErr.exterror, dosErr.class, dosErr.action, dosErr.locus);
	}
	return ( -1 );
    }
    return ( 0 );
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
int
findPath(path, dirDta)
char *path;
DirDta  *dirDta;
{
    if ( findFirst("*.*", dirDta, 0x3F) ) {
	return ( -1 );
    }

    while ( 1 ) {
	if  ( findNext(dirDta) ) {
             return ( -1 );
        }
        if ( ! strcmp(path, dirDta->fname) ) {
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
int
findLoc(loc, dirDta)
long loc;
DirDta  *dirDta;
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
