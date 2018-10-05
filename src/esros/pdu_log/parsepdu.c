/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File: parsePdu.c
 * 
 * Module: PDU Logger (ESROS)
 *
 * Description: Parse PDU.
 *
 * Functions: parsePdu ()
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: parsepdu.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "byteordr.h"
#include "du.h"
#include "eropdu.h"
#include "udp_po.h"

Int
parsePdu(DU_View du, Pdu *pPdu) 
{
    unsigned char c;
    char *p;

    p = (char *)DU_data(du); 

    /* PCI, Byte1 , PDU-TYPE + remEsroSapSel or encodingType ... */

    BO_get1(c, p);

    pPdu->pdutype = c & 0x07;
    switch (c & 0x07) {

    case INVOKE_PDU:

	/* PCI, BYTE-1 */
	pPdu->toESROSap   =   (c & 0xF0) >> 4;	
	pPdu->fromESROSap =  ((c & 0xF0) >> 4) - 1;

	/* PCI Byte-2 */
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

	/* PCI, Byte-3 , operationValue + EncodingType */
	BO_get1(c, p);  

	pPdu->operationValue = c & 0x3F;
	pPdu->encodingType = (c & 0xC0) >> 6;

	break;

    case RESULT_PDU:
	/* PCI, BYTE-1 */
	pPdu->encodingType = (c & 0xC0) >> 6;

	/* PCI Byte-2 */
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

	break;

    case ERROR_PDU:

	/* PCI, BYTE-1 */
	pPdu->encodingType = (c & 0xC0) >> 6;

	/* PCI Byte-2 */
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

	break;

    case ACK_PDU:

	/* PCI Byte-1 */
	pPdu->encodingType = (c & 0x10) >> 4;

	/* PCI Byte-2 */
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

        break; 

    case FAILURE_PDU:

	BO_get1(c, p);
	pPdu->invokeRefNu = c;

	break;

    default:
	fprintf (stderr, "\nUnknown PDU type %d", c & 0x07);
	break;
    }

    return (SUCCESS);

} /* parsePdu() */

