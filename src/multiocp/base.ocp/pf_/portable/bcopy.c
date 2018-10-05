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
 * File: 	bcopy.c
 *
 * Description: Binary copy.
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: bcopy.c,v 1.4 1996/11/07 02:46:01 kamran Exp $";
#endif /*}*/


/*<
 * Function:    bcopy
 *
 * Description: Binary copy
 *
 * Arguments:	Source, destination, length
 *
 * Returns: 	None.
 *
>*/
void
bcopy(register char *s1, register char *s2, register int len)
{
  if (len <= 0)
    return;

  while(len--)
    *s2++ = *s1++;
}


/*<
 * Function:    bcmp
 *
 * Description: Binary compare.
 *
 * Arguments:	First string, second string, length.
 *
 * Returns: 	
 *
>*/
int
bcmp(register char *s1, register char *s2, register int len)
{
  if (len <= 0)
    return (-1) ;

  while(len--)
    if (*s2++ != *s1++)
      return(1);
  
  return(0);
}
