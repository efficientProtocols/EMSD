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
 * Filename: sap.c
 *
 * Description: Service Access Point
 *
 * Functions:
 *   SAP_selHexGet(SAP_Sel *sel, int selSize, String hexStr)
 *   SAP_sepHexPr(SAP_Sel *sel, Char *first, Char *last)
 *   SAP_selCmp (SAP_Sel *n1, SAP_Sel *n2)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sap.c,v 1.6 1996/11/11 00:12:08 fletch Exp $";
#endif /*}*/

#include "estd.h"
#include "addr.h"
#include "pf.h"
#include "bs.h"


/*<
 * Function:    SAP_selHexGet
 *
 * Description:
 *
 * Arguments:
 *
 * Returns:
 * 
>*/
PUBLIC Char *
SAP_selHexGet(SAP_Sel *sel, int selSize, String hexStr)
{
    Int length;
    Char * cp1;
    Char tmp;
    unsigned int val;
    Byte *addr;
    /*
     * -Recognize Seperators, such as ":"
     * -Get rid of .
     */
    addr = sel->addr;
    length = strlen(hexStr);
    if ( (length & 1) || (length > (selSize -2)) ) {
	return ((Char *) 0);
    }
    sel->len = length / 2;
    while (*hexStr) {
	cp1 = hexStr + 2;
	tmp = *cp1;
	*cp1 = '\0';
	if ( PF_getUns(hexStr, &val, 0, 0, 0xff) ) {
	    return ((Char *) 0);
	}
	*addr++ = val;
	*cp1 = tmp;
	 hexStr = cp1;
    }
    return (hexStr);
}


/*<
 * Function:    SAP_selHexPr
 *
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Char *
SAP_sepHexPr(SAP_Sel *sel, Char *first, Char *last)
{
    /* NOTYET */
    return ( (Char *) 0);
}


/*<
 * Function:    SAP_selCmp
 *
 * Description: Compare SAP selectors
 *
 * Arguments:   Two SAP selectors
 *
 * Returns: 	zero if identical, non-zero if not identical.
 * 
>*/
PUBLIC Int
SAP_selCmp (SAP_Sel *n1, SAP_Sel *n2)
{
    if (n1->len != n2->len || n1->len == 0) {
	return (-1);
    }
    return (BS_memCmp(n1->addr, n2->addr, n1->len));
}


/*<
 * Function:    SAP_nSapDomainCmp
 *
 * Description: Compare domain of two network addresses 
 *
 * Arguments:   Two Network SAP selectors
 *
 * Returns: 	zero if identical, non-zero if not identical.
 * 
>*/
PUBLIC Int
SAP_nSapDomainCmp (N_SapAddr *n1, N_SapAddr *n2)
{
    if (n1->len != n2->len || n1->len == 0) {
	return (-1);
    }
    return (BS_memCmp(n1->addr, n2->addr, n1->len - 1));
}

