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
 * File name:   inetaddr.h
 *
 * Description: Internetworking header file.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */


#ifndef _INETADDR_H_
#define _INETADDR_H_

#include "estd.h"
#include "sap.h"

#ifdef __STDC__	/* Arg. checking enabled */

extern SuccFail INET_nsapAddrToin_addr(N_SapAddr *nsapAddr, struct in_addr *in);
extern SuccFail INET_in_addrToNsapAddr(struct in_addr *in, N_SapAddr *nsapAddr);
extern SuccFail INET_portNuToTsapSel  (MdInt portNu, T_SapSel *tsapSel);
extern SuccFail INET_tsapSelToPortNu  (T_SapSel *tsapSel, Int *portNu);
extern SuccFail INET_gethostid(char *buf, size_t buflen);

#else

extern SuccFail INET_nsapAddrToin_addr();
extern SuccFail INET_in_addrToNsapAddr();
extern SuccFail INET_portNuToTsapSel  ();
extern SuccFail INET_tsapSelToPortNu  ();
extern SuccFail	INET_gethostid	      ();
#endif

#endif







