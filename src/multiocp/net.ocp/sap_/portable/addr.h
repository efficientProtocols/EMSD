/*
 * This file is part of the Open C Platform (OCP) Library. OCP is a
 * portable library for development of Data Communication Applications.
 *
 * Copyright (C) 1995 Neda Communications, Inc.
 *	Prepared by Mohsen Banan (mohsen@neda.com)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.  This library is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.  You should have received a copy of the GNU
 * Library General Public License along with this library; if not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
*/
/*+
 * File name: addr.h
 *
 * Description: Service Access Point structures.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: addr.h,v 1.4 2002/10/08 19:04:43 mohsen Exp $
 */


#ifndef _ADDR_H_
#define _ADDR_H_

#include "estd.h"

#define NODSIZ	6	/* fixed length of an 802.[34] node address */
#define MACSIZ	20	/* max length of MAC address */
#define NSAPSZ	20	/* max length of Network SAP address */
#define SUBNSZ	20	/* max length of subnetwork address */
#define TSAPSZ	32	/* max length of Transport SAP selector */
#define SSAPSZ	16	/* max length of Session SAP selector */
#define PSAPSZ  16	/* max length of Presentation SAP selector */


typedef struct NodeAddr{
	unsigned char	addr[NODSIZ];	/* the address bytes */
} NodeAddr;

/*
 *  Generic MAC address structure
 */

typedef	struct MacAddr {
	int		len;		/* MAC address length */
	unsigned char	addr[MACSIZ];	/* MAC address */
}  MacAddr;

/*
 *  Subnetwork address.
 */

typedef struct SN_Addr {
	int		len;
	unsigned char	addr[SUBNSZ];
} SN_Addr;

typedef unsigned char	L_SapSelector;	/* data link layer SAP selector */


typedef struct N_SapAddr {
	int		len;		/* length of address field */
	unsigned char	addr[NSAPSZ];	/* binary representation of address */
} N_SapAddr;

typedef unsigned char N_SapSel;	/* Network layer SAP selector */

/*
 *  Transport layer SAP selector.
 */

typedef struct T_SapSel {
	int		len;		/* length of address field */
	unsigned char	addr[TSAPSZ];	/* the address field */
} T_SapSel;

/*
 *  Transport SAP address.
 */

typedef struct T_SapAddr {
	T_SapSel	tsap;		/* Transport SAP id */
	N_SapAddr	nsap;		/* Network SAP address */
} T_SapAddr;

/*
 *  Generic Sap Selector.
 */

typedef struct SAP_Sel {
	int		len;		/* length of address field */
	unsigned char	addr[TSAPSZ];	/* the address field */
} SAP_Sel;

/*
 *  ESROS SAP selector.
 */

typedef Byte ESRO_SapSel;

/*
 *  Session SAP selector.
 */

typedef struct S_SapSel {
	int		len;		/* length of address field */
	unsigned char	addr[SSAPSZ];	/* the address field */
} S_SapSel;

/*
 *  Session SAP address.
 */

typedef struct S_SapAddr {
	S_SapSel	ssap;		/* Session SAP id */
	T_SapSel	tsap;		/* Transport SAP id */
	N_SapAddr	nsap;		/* Network SAP address */
}  S_SapAddr;

/*
 *  Presentation SAP selector.
 *  Variable length, 0-16 bytes.
 */

typedef struct P_SapSel {
	int		len;		/* length of address field */
	unsigned char	addr[PSAPSZ];	/* the address field */
} P_SapSel;

/*
 *  Presentation SAP address.
 */

typedef struct P_SapAddr {
	P_SapSel	psap;		/* Presentation SAP id */
	S_SapSel	ssap;		/* Session SAP id */
	T_SapSel	tsap;		/* Transport SAP id */
	N_SapAddr	nsap;		/* Network SAP address */
} P_SapAddr;

#endif
