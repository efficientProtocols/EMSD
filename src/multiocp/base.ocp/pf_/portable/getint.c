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
 * File:	getint.c
 *
 * Description: Get integer.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: getint.c,v 1.8 1996/11/07 02:46:04 kamran Exp $";
#endif /*}*/

/* #includes */
#include "estd.h"
 



/*<
 * File name: getint.c
 *
 * Function:  PF_getInt(char *sp, Int *pl, Int def, Int min, Int max)
 *
 * Description:
 *  Convert an ascii string to a long  decimal integer
 *
 * Returns:
 *      0       If every thing went fine.
 *      1       If there had been an invalid entry
 *      2       If the entry was not with in the specified range
 * 
>*/
PUBLIC Int
PF_getInt(char *sp, Int *pl, Int def, Int min, Int max)
                       /* pointer to the string in question */
                       /* pointer to an Int where the result goes */
                       /* the default value */
                       /* minimum acceptable value */
                       /* maximum acceptable value */
{
    register Char *rsp;
    LgInt number;
    Int sign;
    
    rsp = sp;
    number = 0;
    if (!*rsp) {
        *pl = def;
        return 0;
    }
    if (*rsp == '-') {
        sign = -1;
        rsp++;
    } else {
        sign = 1;
    }
    while (isdigit(*rsp)) {
        number = 10 * number + ((*rsp++) - '0');
    }
    if (!*rsp) {
        if ((number*sign) < min || (number*sign) > max) {
            return 2;
        } else {
            *pl = number*sign;
            return 0;
        }
    }
    return 1;
}

