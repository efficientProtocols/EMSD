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
 * File: pduout.c
 *
 * Description:
 *   EROP-PDU building routines.
 *   Each function in this module takes as arguments all needed parameters, 
 *   allocates a DU_ if needed, prepends and builds the PDU header and
 *   returns a pointer to the DU_ containing a finished PDU.
 *
 * Functions:
 *   erop_invokePdu
 *   erop_resultPdu
 *   erop_failurePdu
 *   erop_ackPdu
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: pduout.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "invoke.h"
#include "eropdu.h"

#include "eropfsm.h"
#include "nm.h"
#include "layernm.h"
#include "byteordr.h"

#include "target.h"

#include "local.h"
#include "extfuncs.h"


/*<
 * Function:    erop_invokePdu
 *
 * Description: Build invoke PDU.
 *
 * Arguments:   SAP selector, invoke reference number, operation value,
 *              encoding type, data.
 *
 * Returns:     Pointer to PDU.
 *
>*/

LOCAL DU_View 
erop_invokePdu(InvokeInfo *invoke,
	       ESRO_SapSel		remEsroSapSel,	/* Remote Address */
	       Int 			invokeRefNu,
	       EROP_OperationValue	operationValue,
	       EROP_EncodingType	encodingType,
	       DU_View 			data)
{
#ifdef FUTURE
    union BO_Swap swap;
#endif
    register unsigned char *p;
    unsigned char *pdustart;
    DU_View du;


#define INVOKE_PCI_LEN 3

    if ( (du = (data == (DU_View) NULL) ? DU_alloc(G_duMainPool, 0) : data) 
	  == 0) {
	EH_problem("erop_invokePdu: DU_alloc failed\n");	
	return NULL;
    }

    TM_TRACE((DU_modCB, DU_MALLOC, "erop_invokePdu: DU_alloc: du=0x%lx\n", du));

    DU_prepend(du, INVOKE_PCI_LEN);
    p = DU_data(du);                   /* p = du->addr; */
    pdustart = p;

    /* PCI, Byte1 , PDU-TYPE + remEsroSapSel */
    {
	Byte	pciByte1;
	
		/* Easier to debug but less efficient */
	pciByte1 = (remEsroSapSel << 4) | INVOKE_PDU; 
	BO_put1(p, pciByte1);
    }

    /* PCI, Byte-2 , InvokeRefNu */
    BO_put1(p, invokeRefNu);

    /* PCI, Byte-3 , operationValue + EncodingType */
    {
	Byte	pciByte3;
	
	pciByte3 = (encodingType << 6) | operationValue;
	BO_put1(p, pciByte3);
    }

#ifdef TM_ENABLED
    tm_pduPr(EROP_modCB, TM_PDUOUT, "->", du, MAX_LOG);
#endif

    NM_incCounter(NM_ESROS, &erop_pduSentCounter, 1);
    return (du);
}


/*<
 * Function:    erop_resultPdu
 *
 * Description: Build result or error PDU.
 *
 * Arguments:   Invoke info, invoke reference number, local SAP selector, 
 *              remote SAP selectore, encoding type, data.  
 *
 * Returns:     Pointer to PDU.
>*/

LOCAL DU_View 
erop_resultPdu(InvokeInfo *invoke,
	       Int invokeRefNu,
	       ESRO_SapSel		locEsroSapSel,	
	       ESRO_SapSel		remEsroSapSel,	
	       EROP_EncodingType	encodingType,
	       DU_View data)
{
#ifdef FUTURE
    union BO_Swap swap;
#endif
    register unsigned char *p;
    unsigned char *pdustart;
    DU_View du;

#define RESULT_PCI_LEN 2
 
    if ( (du = (data == (DU_View) NULL) ? DU_alloc(G_duMainPool, 0) : data) 
	 == 0) {
	EH_problem("erop_resultPdu: DU_alloc failed\n");	
	return NULL;
    }

    TM_TRACE((DU_modCB, DU_MALLOC, "erop_resultPdu: DU_alloc: du=0x%lx\n", du));

    DU_prepend(du, RESULT_PCI_LEN);
    p = DU_data(du);
    pdustart = p;

    /* PCI, Byte1 , PDU-TYPE + encodingType */
    {
	Byte	pciByte1;
	
    	pciByte1 = (encodingType << 6) | RESULT_PDU; 

	BO_put1(p, pciByte1);
    }

    /* PCI, Byte-2 , InvokeRefNu */
    BO_put1(p, invokeRefNu);

#ifdef TM_ENABLED
    tm_pduPr(EROP_modCB, TM_PDUOUT, "->", du, MAX_LOG);
#endif

    NM_incCounter(NM_ESROS, &erop_pduSentCounter, 1);
    return (du);
}


/*<
 * Function:    erop_errorPdu
 *
 * Description: Build error PDU.
 *
 * Arguments:   Invoke info, invoke reference number, local SAP selector, 
 *              remote SAP selectore, encoding type, error value, data.  
 *
 * Returns:     Pointer to PDU.
>*/

LOCAL DU_View 
erop_errorPdu(InvokeInfo *invoke,
	      Int invokeRefNu,
	      ESRO_SapSel	locEsroSapSel,	
	      ESRO_SapSel	remEsroSapSel,	
	      EROP_EncodingType	encodingType,
	      EROP_ErrorValue	errorValue,
	      DU_View data)
{
#ifdef FUTURE
    union BO_Swap swap;
#endif
    register unsigned char *p;
    unsigned char *pdustart;
    DU_View du;

#define ERROR_PCI_LEN 3
 
    if ( (du = (data == (DU_View) NULL) ? DU_alloc(G_duMainPool, 0) : data) 
	 == 0) {
	EH_problem("erop_errorPdu: DU_alloc failed\n");	
	return NULL;
    }

    TM_TRACE((DU_modCB, DU_MALLOC, "erop_errorPdu: DU_alloc: du=0x%lx\n", du));

    DU_prepend(du, ERROR_PCI_LEN);
    p = DU_data(du);
    pdustart = p;

    /* PCI, Byte1 , PDU-TYPE + encodingType */
    {
	Byte	pciByte1;
	
    	pciByte1 = (encodingType << 6) | ERROR_PDU; 

	BO_put1(p, pciByte1);
    }

    /* PCI, Byte-2 , InvokeRefNu */
    BO_put1(p, invokeRefNu);

    /* PCI, Byte-3 , ErrorValue */
    BO_put1(p, errorValue);

#ifdef TM_ENABLED
    tm_pduPr(EROP_modCB, TM_PDUOUT, "->", du, MAX_LOG);
#endif

    NM_incCounter(NM_ESROS, &erop_pduSentCounter, 1);
    return (du);
}



/*<
 * Function:    erop_failurePdu
 *
 * Description: Build failure PDU.
 *
 * Arguments:   Invoke info, invoke reference number, failure reason, data.  
 *
 * Returns:     Pointer to PDU.
 *
>*/

DU_View 
erop_failurePdu(InvokeInfo *invoke, short unsigned int invokeRefNu, 
		unsigned char reason, DU_View data)
{
#ifdef FUTURE
    union BO_Swap swap;
#endif
    register unsigned char *p;
    unsigned char *pdustart;
    DU_View du;
    int     len;

    len = 3; /* NOTYET */

    if ( (du = (data == (DU_View) NULL) ? DU_alloc(G_duMainPool, 0) : data) 
	 == 0) {
	EH_problem("erop_failurePdu: DU_alloc failed\n");	
	return NULL;
    }

    TM_TRACE((DU_modCB, DU_MALLOC, "erop_failurePdu: DU_alloc: du=0x%lx\n", du));

    DU_prepend(du, len);
    p = DU_data(du);
    pdustart = p;

/*    BO_put1(p, 0); */
    BO_put1(p, FAILURE_PDU);
    BO_put1(p, invokeRefNu);
    BO_put1(p, reason);

/*    len = p - pdustart - 1;
    *pdustart = len;
*/

#ifdef TM_ENABLED
    tm_pduPr(EROP_modCB, TM_PDUOUT, "->", du, MAX_LOG);
#endif

    NM_incCounter(NM_ESROS, &erop_pduSentCounter, 1);
    return (du);
}


/*<
 * Function:    erop_ackPdu
 *
 * Description: Build ACK PDU.
 *
 * Arguments:   Invoke info, invoke reference number.  
 *
 * Returns:     Pointer to PDU.
 *
>*/

DU_View 
erop_ackPdu(InvokeInfo *invoke,
	    Int invokeRefNu)
{
#ifdef FUTURE
    union BO_Swap swap;
#endif
    register unsigned char *p;
    unsigned char *pdustart;
    DU_View du;

#define ACK_PCI_LEN 2

    if ((du = DU_alloc(G_duMainPool, ACK_PCI_LEN)) == 0) {
	EH_problem("erop_ackPdu: DU_alloc failed\n");	
	return NULL;
    }

    TM_TRACE((DU_modCB, DU_MALLOC, "erop_ackPdu: DU_alloc: du=0x%lx\n", du));

    p = DU_data(du);
    pdustart = p;


    /* PCI, Byte1 , PDU-TYPE  */
    {
	Byte	pciByte1;
	
	pciByte1 =  ACK_PDU; 
	BO_put1(p, pciByte1);
    }

    /* PCI, Byte-2 , InvokeRefNu */
    BO_put1(p, invokeRefNu);

#ifdef TM_ENABLED
    tm_pduPr(EROP_modCB, TM_PDUOUT, "->", du, MAX_LOG);
#endif

    NM_incCounter(NM_ESROS, &erop_pduSentCounter, 1);
    return (du);
}
