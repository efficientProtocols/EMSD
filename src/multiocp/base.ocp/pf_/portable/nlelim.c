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
 * File:	nlelim.c
 *
 * Description: Eliminate new line character.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: nlelim.c,v 1.4 1996/11/07 02:46:06 kamran Exp $";
#endif /*}*/

/* #includes */
#include "estd.h"
 
/*<
 * Function:	PF_nlElim.
 *
 * Description:	Eliminate new line character.
 *
 * Arguments:	Line.
 *
 * Returns:	None.
 *
>*/
PUBLIC Void
PF_nlElim(char *iline)
{
    char * cp;
    cp= &iline[strlen(iline)-1]; 
    if (*cp == '\n') {
        *cp = '\0';
    }
}
