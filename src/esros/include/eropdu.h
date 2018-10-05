/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of ESRO. An implementation of RFC-2188.
 *
 *ESRO is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *ESRO is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with ESRO; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 *
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 *
*/
/*+
 * File name: pdu.h
 *
 * Description: PDU codes, and PDU sturct.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: eropdu.h,v 1.2 2002/10/25 19:37:34 mohsen Exp $
 */

#ifndef _PDU_H_	/*{*/
#define _PDU_H_

#include "erop.h"
#include "esro.h"

#define INVOKE_PDU	0x00		/* Invoke PDU */
#define RESULT_PDU	0x01		/* Result PDU */
#define ERROR_PDU	0x02		/* Error PDU */

#define ACK_PDU		0x03		/* Acknowledgement PDU */
#define FAILURE_PDU	0x04		/* Failure PDU */

/*  User data field limits  */

#define PARAMETERSIZE	ESRODATASIZE		/* max size of user data */
/* NOTYET */


/* NOTYET, Pdu needs clean up */

typedef struct Pdu {
    int		version;		/* version number -- place holder */
    DU_View	data;			/* user data from PDU */
    int		pdutype;		/* type of PDU */
    int		pdu_size;		
    int		badpdu;
    EROP_OperationValue operationValue;
    Int 	invokeRefNu;
    EROP_EncodingType encodingType;	
    ESRO_SapSel fromESROSap;
    ESRO_SapSel toESROSap;
    T_SapSel	*fromTsapSel;
    T_SapSel	*toTsapSel;
    N_SapAddr	*fromNsapAddr;
    N_SapAddr	*toNsapAddr;		
    int		ack_delay;		
    EROP_ErrorValue	errorValue;		
    EROP_FailureValue	failureValue;		
} Pdu;

#endif	/*}*/
