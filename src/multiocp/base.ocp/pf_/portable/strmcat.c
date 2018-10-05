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
 * File:	strmcat.c
 *
 * Description:
 *
-*/

/*
 * History:
 *	Warren Seltzer wrote it originally.
 *	Mohsen Banan modified it fit my needs.
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: strmcat.c,v 1.4 1996/11/07 02:46:11 kamran Exp $";
#endif /*}*/

/*<
 * FUNCTION:        strmcat()
 * 
 * Description:     Append (dst = dst plus src) until dstend or end of string.
 *                  Like strncat but the param used is more useful.
 *                  The string is copied, but
 *              1) result is guaranteed to be a valid null terminated string.
 *              2) The routine will not overrun beyond the end.
 *              3) The result pointer points the terminating null of the
 *                 destination, even if it is the only changed char of the
 *                 destination string.
 *              4) The pointer returned can be used as the next "dst".
 *
 * Arguments:       dst ptr, src ptr, last end ptr, is reserved for '\0'.
 *
 * Returns:         Pointer to null at end, for more work at the end.
 *
 * Side Effects:
 *
 * Calls:
 * 
>*/
char *
PF_strmcat (char *dst, char *src, char *dstend)          /* Overwrite src directly on to dst */
                         
{
    if (dst <= dstend) {
        while (*dst) dst++;
        while ((dst <= dstend) && (*dst++ = *src++)) {
            ;
        }
        /* dst is ptr to next available character.
         *  (*dst) is '\0' OR we ran out of space.
         */

        *--dst = '\0';
        return dst;
    } else {
	/* EH_oops() */
	return (char *) 0;
    }
}

