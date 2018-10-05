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
 * File: pduin.c
 *
 * Description: This file contains the code for analyzing incoming PDU headers.
 *
 * Functions:
 *   SuccFail erop_sendFailurePdu(N_SapAddr *naddr, 
 *                                short unsigned int invokeRefNu, 
 *                                unsigned char reason)
 *   erop_getInvoke(struct Pdu *pdu)
 *   erop_inPdu(DU_View du)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: pduin.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/


#include "estd.h"
#include "eh.h"
#include "addr.h"
#include "invoke.h"
#include "eropdu.h"

#include "du.h"
#include "byteordr.h"

#include "eropfsm.h"
#include "nm.h"
#include "layernm.h"

#include "target.h"
#include "sap.h"
#include "udp_if.h"
#include "erop_sap.h"

#include "local.h"
#include "extfuncs.h"


extern Counter erop_badAddrCounter;
extern Counter erop_opRefusedCounter;
extern FSM_TransDiagram *erop_CLPerformerTransDiag(void);

LOCAL Pdu erop_pdu;


/*<
 * Function:    erop_sendFailurePdu
 *
 * Description: Send a FAILURE PDU to destination reference.
 *
 * Arguments:   Network SAP, invoke reference number, failure reason.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
>*/

SuccFail 
erop_sendFailurePdu(T_SapSel *tSapSel, N_SapAddr *naddr, 
			     short unsigned int invokeRefNu, 
			     unsigned char reason)
{
#ifdef FUTURE
    union BO_Swap swap;
#endif
    register unsigned char *p;
    unsigned char *pdustart;
    DU_View du;
    int len;

    len = 3;

    TM_TRACE((EROP_modCB, TM_ENTER,
	    "erop_sendFailurePdu: (rcv %u PDU) refNu=%u reason=%u\n",
	    erop_pdu.pdutype & 0xff, invokeRefNu, reason & 0xff));

    if ( (du = DU_alloc(G_duMainPool, 0)) == 0 ) {
	EH_problem("erop_sendFailurePdu: DU_alloc failed\n");	
	return FAIL;
    }

    TM_TRACE((DU_modCB, DU_MALLOC, 
             "erop_sendFailurePdu: DU_alloc: du=0x%lx\n", du));

    DU_prepend(du, len);
    p = DU_data(du);
    pdustart = p;

    BO_put1(p, FAILURE_PDU);
    BO_put1(p, invokeRefNu);
    BO_put1(p, reason);
/*
    len = p - pdustart - 1;
    *pdustart = len;
*/
/*    (*EROP_lowDesc.dataReq)(EROP_SAP, naddr, erop_pdu.qos, du); */
    if (UDP_dataReq(erop_udpSapDesc, tSapSel ,naddr, du) == FAIL) {
	if (du) {

	    TM_TRACE((DU_modCB, DU_MALLOC, 
                     "erop_sendFailurePdu: DU_free: du=0x%lx\n", du));

	    DU_free(du);
	}
	return (FAIL);
    }

#if FUTURE
    NM_incCounter(NM_ESROS, &erop_pduSentCounter, 1);
#endif

#ifdef TM_ENABLED
    tm_pduPr(EROP_modCB, TM_PDUIN, "->", du, MAX_LOG);
#endif

    if (du) {

        TM_TRACE((DU_modCB, DU_MALLOC, 
                 "erop_sendFailurePdu: DU_free: du=0x%lx\n", du));

        DU_free(du);
    }

    return (SUCCESS);

} /* erop_sendFailurePdu */


/*<
 * Function:    erop_outOfInvokeId
 *
 * Description: LOPS is out of invoke id. Send a FAILURE PDU.
 *
 * Arguments:   invoke info, Network SAP, failure reason.
 *
 * Returns:     O if successful, -1 otherwise.
 *
>*/
SuccFail
erop_outOfInvokeId(InvokeInfo *invok, T_SapSel *tSapSel, 
			    N_SapAddr *naddr, 
			    unsigned char reason)
{
    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_outOfInvokeId: reason=%x\n", reason));
    return erop_sendFailurePdu(tSapSel, naddr, invok->invokeRefNu, reason);
}



/*<
 * Function:    erop_getInvoke
 *
 * Description: 
 *  Select an InvokeInfo for an incoming PDU.
 *  Search list of InvokeInfos and find one that matches
 *  the SAP-IDs in case of INVOKE-PDU; and InvokeRefNu
 *  for all other types of PDUs.
 *
 * Arguments: PDU.
 *
 * Returns:   Invoke info.
 *
>*/

STATIC InvokeInfo *
erop_getInvoke(struct Pdu *pdu)
{
    register InvokeInfo *invokeInfo;
    SapInfo *sap;

    /* Process INVOKE PDUs.  */

    if (pdu->pdutype == INVOKE_PDU) {

	/*
	 * If the performer SAP is not activated, send a FAILURE PDU
	 */

	if ((sap = erop_getSapInfo(pdu->toESROSap)) == (SapInfo *) NULL) {
    	    TM_TRACE((EROP_modCB, TM_ENTER, 
	     	     "erop_getInvoke: getSapInfo Failed\n"));
	    erop_sendFailurePdu(pdu->fromTsapSel, pdu->fromNsapAddr, 
			        pdu->invokeRefNu, 2);
              /* user not responding but it's really a baddAddress */
	    NM_incCounter(NM_ESROS, &erop_opRefusedCounter, 1);
	    NM_incCounter(NM_ESROS, &erop_badAddrCounter, 1);
	    return ((InvokeInfo *) NULL);
	}

	/*
	 * Check for duplicated INVOKE PDU. An INVOKE PDU is a duplicate if
	 * destination reference and UDP-SAP address matches. 
	 */

	for (invokeInfo = sap->invokeSeq.first; 
             invokeInfo != (InvokeInfo *) &sap->invokeSeq;
	     invokeInfo = invokeInfo->next) {
	    if (invokeInfo->invokeRefNu == pdu->invokeRefNu &&
		N_sapAddrCmp(&invokeInfo->remNsapAddr, pdu->fromNsapAddr)==0) {
		return (invokeInfo);
	    }
	}

	/*
	 * Non-duplicate INVOKE PDU is received. Select an available InvokeInfo
	 * in order to generate a EROP-Invoke.Ind event. 
	 */

	if ((invokeInfo = erop_invokeInfoCreate(sap)) != (InvokeInfo *) NULL) {

	    /* Select CL-Performer Transition-Diagram for invoke (machine) */
	    if (invokeInfo->locSap->functionalUnit == ESRO_2Way) {
	    	invokeInfo->transDiag = erop_2CLPerformerTransDiag();
	    } else {
	    	invokeInfo->transDiag = erop_CLPerformerTransDiag();
	    }
	    invokeInfo->curState              = invokeInfo->transDiag->state;
	    invokeInfo->performingNotInvoking = TRUE;

	    return (invokeInfo);
	}

	/*
	 * We are out of resources.
	 */

    	TM_TRACE((EROP_modCB, TM_ENTER, 
	         "erop_getInvoke: Out of resources\n"));

	if (erop_sendFailurePdu(pdu->fromTsapSel, pdu->fromNsapAddr, 
			        pdu->invokeRefNu, 1) == FAIL) {
	    return ((InvokeInfo *) NULL);	/* differentiate this case */
	}

	return ((InvokeInfo *) NULL);
    }

    /*
     * Given the refNu find me the invokeInfo.
     */

    switch (pdu->pdutype) {

    case INVOKE_PDU:
    case ACK_PDU:
	if ((invokeInfo = erop_refToPerformer(pdu->invokeRefNu, pdu->fromNsapAddr))
	    == (InvokeInfo *)NULL) {
	    EH_problem("erop_getInvoke: no invokeInfo found for ACK RefNu\n");
	    return ((InvokeInfo *) NULL);
	}
	return (invokeInfo);

    case RESULT_PDU:
	if ((invokeInfo = erop_refToInvoker(pdu->invokeRefNu, pdu->fromNsapAddr)) 
	    == (InvokeInfo *)NULL) {
	    EH_problem("erop_getInvoke: no invokeInfo found for RESULT RefNu\n");
	    erop_sendFailurePdu(pdu->fromTsapSel, pdu->fromNsapAddr, 
				pdu->invokeRefNu, 
				1 /* NOTYET mismatched_reference */);
	    return ((InvokeInfo *) NULL);
	}
	return (invokeInfo);

    case ERROR_PDU:
	if ((invokeInfo = erop_refToInvoker(pdu->invokeRefNu, pdu->fromNsapAddr)) 
	    == (InvokeInfo *)NULL) {
	    EH_problem("erop_getInvoke: no invokeInfo found for RESULT RefNu\n");
	    erop_sendFailurePdu(pdu->fromTsapSel, pdu->fromNsapAddr, 
				pdu->invokeRefNu, 
				1 /* NOTYET mismatched_reference */);
	    return ((InvokeInfo *) NULL);
	}
        invokeInfo->errorValue = pdu->errorValue;
	return (invokeInfo);

    case FAILURE_PDU:
	if ((invokeInfo = erop_refToInvoker(pdu->invokeRefNu, pdu->fromNsapAddr)) 
	    == (InvokeInfo *)NULL ) {

	    EH_problem("erop_getInvoke: no invokeInfo found for Failure RefNu\n");
	    return ((InvokeInfo *) NULL);
	}

 	if (invokeInfo->invokeRefNu != pdu->invokeRefNu) {
	    EH_problem("erop_getInvoke: invokeInfo found but RefNu doesn't match\n");
	    return ((InvokeInfo *) NULL);
	}

        invokeInfo->failureReason = pdu->failureValue;
	return (invokeInfo);

    default:
	EH_problem("erop_getInvoke: Unknown PDU type");
	return ((InvokeInfo *) NULL);
    }

} /* erop_getInvoke() */


/*<
 * Function:    erop_inPdu
 *
 * Description: 
 *   Read the PDU header on an incoming PDU. 
 *   Construct the PDU structure and strip the header from the data buffer. 
 *   If there is no data in the buffer then free it.
 *
 * Arguments: PDU
 *
 * Returns:   Invoke info.
 *
>*/

LOCAL
InvokeInfo *
erop_inPdu(DU_View du)
{
    register unsigned char *p;
    register struct Pdu    *pdu;
    InvokeInfo    *invoke;
    unsigned char *pstart;
    unsigned char c;
    int  count;
    Int  released;

    pdu = &erop_pdu;
    pdu->data    = (DU_View) NULL;
    pdu->pdutype = 0;
    pdu->badpdu  = FALSE;

    invoke = (InvokeInfo *) NULL;

    p      = DU_data(du);
    pstart = p;
    count  = DU_size(du);

    if (count == 0) {
	released = 0;

        TM_TRACE((EROP_modCB, DU_MALLOC, "erop_inPdu: DU_free: du=0x%lx\n", du));

	DU_free(du);
	return ((InvokeInfo *) NULL);
    }

    /* This SDU does contain something. */

#ifdef TM_ENABLED
    tm_pduPr( EROP_modCB, TM_PDUIN, "<-", du, MAX_LOG);
#endif


    /* PCI, Byte1 , PDU-TYPE + remEsroSapSel or encodingType ... */
    BO_get1(c, p);
    pdu->pdutype = (c & 0x07);

    /* Validate PDU type */

    if (pdu->pdutype > 5 ) {
	goto badpdu;
    }

    /* Setup default parameters */

    /* PDU type dependent processing */

    switch (pdu->pdutype) {

    case INVOKE_PDU:

	/* PCI, BYTE-1 */
	pdu->toESROSap   = (c & 0xF0) >> 4;	
	pdu->fromESROSap = pdu->toESROSap - 1;  /* From is the Invoker, To is the Performer */				
	/* PCI Byte-2 */
	BO_get1(pdu->invokeRefNu, p);

	/* PCI, Byte-3 , operationValue + EncodingType */
	BO_get1(c, p);  

	pdu->operationValue = c & 0x3F;
	pdu->encodingType   = (c & 0xC0) >> 6;

	break;

    case RESULT_PDU:

	/* PCI, BYTE-1 */
	pdu->encodingType = (c & 0xC0) >> 6;	

	/* PCI Byte-2 */
	BO_get1(pdu->invokeRefNu, p);

	break;

    case ERROR_PDU:

	/* PCI, BYTE-1 */
	pdu->encodingType = (c & 0xC0) >> 6;	

	/* PCI Byte-2 */
	BO_get1(pdu->invokeRefNu, p);

	/* PCI Byte-3 */
	BO_get1(pdu->errorValue, p);

	break;

    case ACK_PDU:

	/* PCI Byte-2 */
	BO_get1(pdu->invokeRefNu, p);

	if ((invoke = erop_getInvoke(pdu)) == (InvokeInfo *) NULL)
	    goto ignore;

	break;

    case FAILURE_PDU:

	/* PCI, BYTE-2 */
	BO_get1(pdu->invokeRefNu, p);

	/* PCI Byte-3 */
	BO_get1(pdu->failureValue, p);

	if ((invoke = erop_getInvoke(pdu)) == (InvokeInfo *) NULL)
	    goto ignore;
	break;

    default:
	EH_problem("erop_inPdu: Unknown PDU type");
	goto ignore;
    }

    /* Remove The PCI */

    DU_strip(du, p - pstart);
    count = DU_size(du);

    /* NOTYET */
    /* Verify size limits here */

    if (count == 0 && pdu->pdutype == ACK_PDU) {

        TM_TRACE((EROP_modCB, DU_MALLOC, "erop_inPdu: DU_free: du=0x%lx\n", du));

	DU_free(du);
	du = (DU_View) NULL;
	released = 1;
    }

    pdu->data = du;


    /*
     * If the received PDU has not already been associated with its
     * InvokeInfo, do the association now. 
     */

    if (invoke == (InvokeInfo *) NULL) {
	if ((invoke = erop_getInvoke(pdu)) == (InvokeInfo *) NULL) {
	    goto ignore;
	}
    }

    return (invoke);

    /*
     * PDU has been received which could not be associated with an operation
     * We will ignore this PDU. 
     */

ignore:
    if (du != (DU_View)0 ) {

    	TM_TRACE((EROP_modCB, TM_ENTER, 
		 "erop_inPdu: PDU ignored du=0x%lx\n", du));

        TM_TRACE((EROP_modCB, DU_MALLOC, 
	     	 "erop_inPdu: DU_free: du=0x%lx refcnt=%d\n", 
	         du, du->bufInfo->refcnt));

	DU_free(du);
    }

    TM_TRACE((EROP_modCB, TM_PDUIN,
             "erop_inPdu:          PDU received and ignored, type=%x\n", 
             pdu->pdutype & 0xff));

    return ((InvokeInfo *) NULL);

    /*
     * Badly formatted PDU received. Free the buffer and reset blocking
     * parameters. It will also be counted a a bad PDU for NM purposes. 
     */

badpdu:
    if (du != (DU_View)0 ) {

    	TM_TRACE((EROP_modCB, DU_MALLOC, "erop_inPdu: DU_free: du=0x%lx\n", du));

	DU_free(du);
    }
    /* NOTYET     NM_incCounter(NM_ESROS, &erop_invalidPduCounter, 1); */

    TM_TRACE((EROP_modCB, TM_PDUIN,
             "erop_inPdu:          Bad PDU received type=%x\n", 
             pdu->pdutype & 0xff));

    return ((InvokeInfo *) NULL);

} /* erop_inPdu(DU_View du) */
