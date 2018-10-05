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
 * File name: chrcicmp.c
 *
 * Description: Compare two character while ignoring the case.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: chrcicmp.c,v 1.2 1995/12/28 06:57:31 mohsen Exp $";
#endif /*}*/

#include  "estd.h"

/*<
 * Function:	PS_charCaseIgnoreCmp
 *
 * Description:	Compare to characters while ignoring the case
 *
 * Arguments:	two characters
 *
 * Returns:	Comarison result: 0 if identical, code difference otherwise.
 *
 * 
>*/
PUBLIC Int
PF_charCaseIgnoreCmp(Char c1, Char c2)
{
    c2 = tolower(c2);
    c1 = tolower(c1);
    return (c1 - c2);
}
