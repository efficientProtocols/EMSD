/*
 *  
 *  Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
 * 
 */

/*+
 * File: cpright.tpl
 *
 * Description: Template file for Check Copyright message function.
 *
 * Function: ocpCopyRightGet(void)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: ocpcopyr.tpl,v 1.2 1999/09/30 00:05:01 mohsen Exp $";
#endif /*}*/

#if 0
#define SHOW_CRC	/* Define this to display the length and CRC */
#define TEST 		/* Define this to check if working */
#endif

#include  "estd.h"

#include "pf.h"


/*<
 * Function:    ocpCopyRightGet
 *
 * Description: Check copyright message for OCP
 *
 * Arguments:   None.
 *
 * Returns:     Pointer to copyright notice on successful completion, 
 *              0 otherwise.
 *
>*/

PUBLIC Char * 
ocpCopyRightGet(void)
{
static Char cpRightNotice[]= "%MODULE_NAME% Version %RELEASE%\n%MODULE_NU%\n\
Copyright (C) 1995, 1996 Neda Communications, Inc. All rights reserved.\n\
\n";

#define NOTICE_LEN	192
#define NOTICE_CRC	0x4567

    MdUns crc;
    Int len;
    
    len = strlen(cpRightNotice);
    crc = PF_crc16(cpRightNotice, len, 0);

#ifdef SHOW_CRC
    printf("\nlen=%d  crc=0x%x\n", len, crc);
    exit(0);
#endif

    if ( (len != NOTICE_LEN) || (crc != NOTICE_CRC) ) {
	return ( (Char *) 0 );
    } else {
	return ( cpRightNotice );
    }

} /* ocpCopyRightGet(void) */


#ifdef TEST
void 
main()
{
    char *cpRightNotice;

    if ( ! (cpRightNotice = ocpCopyRightGet()) ) {
        printf("CRC Check for copy right notice failed\n");
	exit(1);
    }
    printf("%s\n", cpRightNotice);
}
#endif

#ifdef SHOW_CRC
void 
main()
{
    ocpCopyRightGet();
}
#endif
