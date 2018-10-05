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
 * File:	strfind.c
 *
 * Description: Find string.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: strfind.c,v 1.5 1996/11/07 02:46:09 kamran Exp $";
#endif /*}*/

#include  "estd.h"


/*<
 * Function:	PF_strFind
 *
 * Description:	Find string.
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Char *
PF_strFind (Char *inStr, Char *matchStr)
{
    Char * cPtr;
    Char * retVal;
	    
    cPtr = matchStr;
    for (retVal=inStr; *retVal; ++retVal) {
	if ( cPtr == matchStr ) {
	    if ( *retVal == *cPtr ) {
		++cPtr;
	    }
	} else {
	    if ( ! *cPtr ) {
		/* So matchStr was detected. */
		break;
	    }
	    if ( *retVal != *cPtr ) {
		cPtr = matchStr;
	    } else {
		++cPtr;
	    }
	}
    }
    if ( *retVal == '\0' && *cPtr != '\0' ) {
	retVal = (Char *)NULL;
    }
    return ( retVal );
}
	
