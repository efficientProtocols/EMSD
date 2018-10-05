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
 * File:	pathname.c
 *
 * Description:
 *
 *  The following components of a pathname
 *  head, tail, root, extension are identified as follows:
 *
 *   <----head--> <--tail-->
 *   /junk/fooooo/ss.bar.xxx
 *   <------root---> <-ext->
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

extern char *strchr(const char *, int);

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: pathname.c,v 1.7 1996/11/11 00:12:07 fletch Exp $";
#endif /*}*/

#include "estd.h"
#include "pf.h"



/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

String
PN_getFullPathName(String base, String relative, Char *result, Char *resultEnd)
{
    char	    sep[2] = { '\0', '\0' };

    if (strchr(OS_DIR_PATH_SEPARATOR, *relative) != NULL) {
	return relative;
    } else {
	Char *p;

	sep[0] = *OS_DIR_PATH_SEPARATOR;
	
	p = result;
	p = PF_strmcpy(p, base, resultEnd);
	p = PF_strmcat(p, sep, resultEnd);
	p = PF_strmcat(p, relative, resultEnd);
    }
    return (result);
}


/*<
 * Function:
 * Description:
 * Get the head of a path name
 * Similar to CSH:
 *    h	    Remove a trailing pathname component, leaving the head.
 * So:
 *  "/foo/bar.c"  becomes "/foo"
 *
 * Over Writes path.
 *
 * Arguments:
 *
 * Returns:
 *
 *
>*/
PUBLIC String
PN_getHead(String path)
{
    char * 	    p1;
    char * 	    p2;

    p1 = path + strlen(path) - 1;
    do
    {
	if ((p2 = strchr(OS_DIR_PATH_SEPARATOR, *p1)) != NULL)
	{
	    *p2 = '\0';
	    return path;
	}
    } while (p1-- > path);

    return ZPTR;
}


/*<
 * Function:
 * Description:
 * Get the root of a path name
 * Similar to CSH:
 *    r	    Remove a trailing ".xxx" component, leaving the root.
 * So:
 *  "/foo/bar.c"  becomes "/foo/bar"
 *
 * Over Writes path.
 *
 * Arguments:
 *
 * Returns:
 *
 *
>*/
PUBLIC String
PN_getRoot(String path)
{
    Char *p;

    /* Find the last directory path separator, if it exists. */
    p = path + strlen(path) - 1;
    do
    {
	if (strchr(OS_DIR_PATH_SEPARATOR, *p) != NULL)
	{
	    break;
	}
    } while (p-- > path);

    /* Now, from this location, find the last dot. */
    if ((p = strrchr(p, '.')) != NULL)
    {
	*p = '\0';
    } 
    return ( path );
}



/*<
 * Function:
 * Description:
 * Get the extension of a pathname
 * Similar to CSH:
 *    e	    Remove all but the extension ".xxx" part.
 * So:
 *  "/foo/bar.c"  becomes "c"
 *
 * Over Writes path.
 *
 * Arguments:
 *
 * Returns:
 *
 *
>*/
PUBLIC String
PN_getExtension(String path)
{
    char * 	    p;

    /* Find the last directory path separator, if it exists. */
    p = path + strlen(path) - 1;
    do
    {
	if (strchr(OS_DIR_PATH_SEPARATOR, *p) != NULL)
	{
	    break;
	}
    } while (p-- > path);

    /* Now, from this location, find the last dot. */
    if ((p = strrchr(p, '.')) != NULL)
    {
	return(p + 1);
    }

    return ZPTR;
}


/*<
 * Function:
 * Description:
 * Get the tail of a pathname
 * Similar to CSH:
 *    t	    Remove all  leading pathname components, leaving the tail.
 * So:
 *  "/foo/bar.c"  becomes "bar.c"
 *
 *
 * Arguments:
 *
 * Returns:
 *
 *
>*/
PUBLIC String
PN_getTail(String path)
{
    Char *p;

    /* Find the last directory path separator, if it exists. */
    p = path + strlen(path) - 1;
    do
    {
	if (strchr(OS_DIR_PATH_SEPARATOR, *p) != NULL)
	{
	    break;
	}
    } while (p-- > path);

    if (strchr(OS_DIR_PATH_SEPARATOR, *p) != NULL)
	return(p + 1);

    return ZPTR;
}
