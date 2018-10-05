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
 * File: 	getlguns.c
 *
 * Description: Get long unsigned.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: getlguns.c,v 1.9 1997/11/03 21:29:37 mohsen Exp $";
#endif /*}*/

/* #includes */
#include  "estd.h"
#include  "tm.h"

#ifndef isxdigit
EXTERN Int
isxdigit(Char c);
#endif



/*<
 * Function:	PF_getLgUns
 *
 * Description: Convert an ascii string to a long  decimal integer
 *
 * Returns:
 *      0       If every thing went fine.
 *      1       If there had been an invalid entry
 *      2       If the entry was not with in the specified range
 *
 * 
>*/
PUBLIC Int
PF_getLgUns(Char *sp, LgUns *pi, LgUns def, LgUns min, LgUns max)
                               /* pinter to the string in question */
                            /* pointer to a UINT where the result goes   */
                      /* The default value */
                      /* minimum acceptable value */
                      /* maximum acceptable value */
{
    char *rsp;
    unsigned long number;
    
    rsp = sp;
    number = 0;
    if (!*rsp) {
        *pi= def;
        return 0;
    }
    while (isxdigit(*rsp)){
        number = 16 * number;
        if (isdigit(*rsp))
            number = number + ((*rsp++) - '0');
        else if (islower(*rsp))
            number = number + ((*rsp++) - 'a' + 10);
        else 
            number = number + ((*rsp++) - 'A' + 10);
    }
    if (!*rsp) {
        if  (number < min || number > max) {
            return 2;
        } else {
            *pi = number;
            return 0;
        }
    }
    return 1;
}
