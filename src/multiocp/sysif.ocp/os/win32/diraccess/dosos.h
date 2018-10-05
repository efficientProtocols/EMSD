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

/*
 * SCCS Revision: @(#)dosos.h	1.1    Released: 1/31/88
 */

#ifndef _DOSOS_H_	/*{*/
#define _DOSOS_H_

#include <dos.h>

#define NAMESIZ	13	/* 12 character name + '\0' */

/* DOS Disk Transfer Address table */
typedef struct DirDta {
    char reserved[21];	       	        /* used in "find next" operation */
    char attr;				/* file attribute byte */
    unsigned short ftime;		/* time of last modification */
    unsigned short fdate;		/* date of last modification */
    unsigned long fsize;		/* file size in bytes */
    char fname[NAMESIZ];	        /* filename and extension */
} DirDta;

#ifdef M_I86SM 
#define GET_OFF(ptr)   ((unsigned) ptr)
static unsigned _showds() { struct _SREGS r; _segread(&r); return r.ds;}
#define GET_SEG(ptr)   _showds();
#endif

#endif	/*}*/
