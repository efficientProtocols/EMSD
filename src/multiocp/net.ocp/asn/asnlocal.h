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

#ifndef __ASNLOCAL_H__
#define	__ASNLOCAL_H__

#include "estd.h"
#include "tm.h"
#include "asn.h"

extern TM_ModuleCB *	asn_hModCB;


ReturnCode
asn_berFormat(ASN_TableEntry *  pTab,
	      void *            hBuf,
	      unsigned char *   pCStruct,
	      OS_Uint32 *       pFormattedLength,
	      OS_Uint8          depth);

ReturnCode
asn_berParse(ASN_TableEntry *   pTab,
	     void *             hBuf,
	     unsigned char *    pCStruct,
	     OS_Uint32 *        pduLength,
	     OS_Uint8           depth);

#endif /* __ASN_LOCAL_H__ */;
