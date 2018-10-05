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
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sapcmp.c,v 1.9 1996/11/07 02:46:45 kamran Exp $";
#endif /*}*/

#include "estd.h"
#include "addr.h"


/*<
 * Function:    S_sapCmp
 *
 * Description: Compare two SSAP-IDs.
 *
 * Arguments:	Sap selector 1, sap selector 2.
 *
 * Returns: 	Return zero if identical, non-zero if not identical.
 *
>*/

PUBLIC Int
S_sapCmp(S_SapSel *s1, S_SapSel *s2)
{
    if (s1->len != s2->len || s1->len == 0) {
	return (-1);
    }
    return (OS_memCmp(s1->addr, s2->addr, s1->len));
}


/*<
 * Function:    T_sapCmp
 *
 * Description: Compare two TSAP-IDs.
 *
 * Arguments:	Sap selector 1, sap selector 2
 *
 * Returns:   	Return zero if identical, non-zero if not identical.
 *
>*/

PUBLIC Int
T_sapCmp(T_SapSel *t1, T_SapSel *t2)
{
    if (t1->len != t2->len || t1->len == 0) {
	return (-1);
    }
    return (OS_memCmp(t1->addr, t2->addr, t1->len));
}


/*<
 * Function:    N_sapAddrCmp
 *
 * Description: Compare two network addresses.
 *
 * Arguments:	Sap 1, sap 2.
 *
 * Returns: 	Return zero if identical, non-zero if not identical.
 *
>*/

PUBLIC Int
N_sapAddrCmp(N_SapAddr *n1, N_SapAddr *n2)
{
    if (n1->len != n2->len || n1->len == 0) {
	return (-1);
    }
    return (OS_memCmp(n1->addr, n2->addr, n1->len));
}


/*<
 * Function:    SN_addrCmp
 *
 * Description: Compare two subnetwork addresses.
 *
 * Arguments:	Subnetwork address 1, subnetwork address 2.
 *
 * Returns: 	Return zero if identical, non-zero if not identical.
 *
>*/

PUBLIC Int
SN_addrCmp (SN_Addr *n1, SN_Addr *n2)
{
    if (n1->len != n2->len || n1->len == 0) {
	return (-1);
    }
    return (OS_memCmp(n1->addr, n2->addr, n1->len));
}
