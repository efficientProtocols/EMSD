/*+
 * Description:
 * 
 *    NAME
 *
 *    SYNOPSIS
 *
 *    DESCRIPTION
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)main.c	1.1    Released: 8/8/87";
#endif /*}*/

/* #includes */
#include <signal.h>
#include <string.h>

#include  "estd.h"
#include  "getopt.h"
#include  "eh.h"
#include "tm.h"
#include "pf.h"


Void
sprintfTests()
{
    static Char bigBuf[10240];

    sprintf(bigBuf, "string=%s, digit=%d, \n hex=0x%x, HEX=0X%X, char=%c\n",
	     "Some STRANGE string",
	     22,
	     254,
	     254,
	     'X');
    printf("%s", bigBuf);
}

Void
main(argc, argv)
int argc;
char *argv[];
{

    sprintfTests();
}
