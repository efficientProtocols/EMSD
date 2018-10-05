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
 * File name: bs.c
 *
 * Description: Byte string copy.
 *
 * Functions:
 *   Byte *BS_memCopy(Byte *src, Byte *dst, Int nuOfBytes)
 *   Int BS_memCmp(Byte *src, Byte *dst, Int nuOfBytes)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: bs.c,v 1.6 1996/11/07 02:45:39 kamran Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"


/*<
 * Function:    BS_memCopy(Byte *src, Byte *dst, Int nuOfBytes)
 *
 * Description: Copy memory area.
 *
 * Arguments:   Source address, destination address, number of bytes.
 *
 * Returns:     End of data in destination area.
 *
>*/

Byte *
BS_memCopy(Byte *src, Byte *dst, Int nuOfBytes)
{
    if (nuOfBytes <= 0) {
	EH_problem("BS_memCopy: noOfBytes is negative");
	return ((Byte *)0);
    }
    while (nuOfBytes--) {
	*dst++ = *src++;
    }
    return (dst);
}


/*<
 * Function:    BS_memCmp(Byte *src, Byte *dst, Int nuOfBytes)
 *
 * Description: Copy memory.
 *
 * Arguments:   Source address, destination address, number of bytes.
 *
 * Returns:     0 on success, -1 otherwise.
 *
>*/

Int 
BS_memCmp(Byte *src, Byte *dst, Int nuOfBytes)
{
    if (nuOfBytes < 0) {
	EH_problem("Negative number of bytes passed to BS_memCmp\n");
	return (-1);
    }
    while (nuOfBytes--) {
	if (*src++ != *dst++) {
	    if (*--src < *--dst) {
		return (-1);
	    } else {
		return (1);
	    }
        }
    }
    return (0);
}	
