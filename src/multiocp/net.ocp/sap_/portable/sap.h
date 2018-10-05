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
 * File name: sap.h
 *
 * Description: extern definition of SAP manipulation functions.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: sap.h,v 1.5 1996/10/04 23:42:49 kamran Exp $
 */

#ifndef _SAP_H_	/*{*/
#define _SAP_H_

#include "addr.h"

#ifdef LINT_ARGS /*{*/

Char *SAP_selHexGet(struct SAP_Sel  *, int, char  *);
Char *SAP_sepHexPr (struct SAP_Sel  *, char  *, char  *);
Int   SAP_selCmp   (struct SAP_Sel  *, struct SAP_Sel  *);

Int S_sapCmp    (S_SapSel *, S_SapSel *);
Int T_sapCmp    (T_SapSel *, T_SapSel *);
Int N_sapAddrCmp(N_SapAddr *, N_SapAddr *);
Int SN_addrCmp  (SN_Addr *, SN_Addr *);

Int OSI_sapGet (char *, int, int *, unsigned char *);
PUBLIC Int SAP_nSapDomainCmp (N_SapAddr *n1, N_SapAddr *n2);

#else /*}{*/

Char *SAP_selHexGet();
Char *SAP_sepHexPr();
Int  SAP_selCmp();

Int S_sapCmp ();
Int T_sapCmp ();
Int N_sapAddrCmp ();
Int SN_addrCmp ();

Int OSI_sapGet ();
PUBLIC Int SAP_nSapDomainCmp ();

#endif /*}*/

#endif	/*}*/
