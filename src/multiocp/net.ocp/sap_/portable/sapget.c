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

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sapget.c,v 1.6 1996/10/11 18:05:47 kamran Exp $";
#endif /*}*/

#include "estd.h"

#include "pf.h"


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Int
OSI_sapGet(char *s, int maxLen, Int *lenPtr, unsigned char *addr)
{
    int length;
    char * cp1;
    char tmp;
    unsigned int val;

    length = strlen(s);
    if ( (length & 1) || (length > (2*maxLen)) ) {
#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
	fprintf(stderr, "Invalid SAP %s\n", s);
	exit(1);
#else
        return FAIL;
#endif
    }
    *lenPtr = length / 2;
    while ( *s ) {
	cp1 = s + 2;
	tmp = *cp1;
	*cp1 = '\0';
	if ( PF_getUns(s, &val, 0, 0, 0xff) ) {
#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
	    fprintf(stderr, "Illegal hexadecimal entry\n");
	    exit(1);
#else
	    return FAIL;
#endif
	}
	*addr++ = val;
	*cp1 = tmp;
	 s = cp1;
    }
    return ( SUCCESS );
}




