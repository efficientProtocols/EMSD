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
 * File: perfact.c
 *
 * Description:
 *	This file contains the actions to be taken at each step 
 *      of the ESROS transition Diagrams (performer side).
 *
 * Functions:
 *   tr_clPerformer01(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer02(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer03(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer04(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer05(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer06(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer07(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer08(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer09(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer10(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer11(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clPerformer12(Void *machine, Void *userData, FSM_EventId evtId)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: perfact.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"

#include "invoke.h"
#include "eropfsm.h"
#include "layernm.h"
#include "eropdu.h"
#include "udp_if.h"

#include "target.h"

#include "sch.h"

#include "extfuncs.h"
#include "local.h"

extern Pdu erop_pdu;

extern Void (*erop_freeLopsInvoke)(InvokeInfo *);

Bool dropResultConfirm = FALSE;

/*
 *
 *  EVENT:
 *	ESRO-INVOKE-PDU received.
 *
 *  CURRENT STATE:
 *	CL Performer Start
 *
 *  ACTION:
 *	Issue ESROS-INVOKE.indication primitive.
 *
 *  RESULTING STATE:
 *	Invoke PDU Received.
 */

Int
tr_clPerformer01(Void *machine, Void *userData, FSM_EventId evtId)
{
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != INVOKE_PDU ) {
	EH_problem("tr_clPerformer01: PDU is not invoke PDU\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer01: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    invoke->remEsroSapSel = erop_pdu.fromESROSap;
    invoke->remTsapSel    = *erop_pdu.fromTsapSel;
    invoke->remNsapAddr   = *erop_pdu.fromNsapAddr;
    invoke->encodingType  = erop_pdu.encodingType;
    invoke->invokeRefNu   = erop_pdu.invokeRefNu;


    erop_tmrSetValues(invoke);

    if (erop_invokeInd(invoke,
		   	invoke->locSap->sapSel,
		   	invoke->remEsroSapSel,
		   	&invoke->remTsapSel,
		   	&invoke->remNsapAddr,
		   	erop_pdu.operationValue,
		   	invoke->encodingType,
		   	(DU_View) erop_pdu.data) == FAIL) {

	EH_problem("tr_clPerformer01: erop_invokeInd failed\n");
        TM_TRACE((DU_modCB, DU_MALLOC, 
	         "tr_clPerformer01: DU_free: erop_pdu.data=0x%lx\n", 
		 erop_pdu.data));

	DU_free(erop_pdu.data);

        erop_sendFailurePdu(&invoke->remTsapSel, &invoke->remNsapAddr, 
			    invoke->invokeRefNu, ESRO_FailureUserNotResponding);
	return (FAIL);
    }

#ifndef NO_UPSHELL
    if (erop_tmrCreate(invoke, erop_perfNoResponseTimer, 
		       invoke->perfNoResponse, 0) == FAIL) {
	EH_problem("tr_clPerformer01: erop_tmrCreate failed\n");
	return (FAIL);
    }
#endif

    NM_incCounter(NM_ESROS, &erop_completeOperationCounter, 1);	

    return (SUCCESS);
}


/*
 *
 *  EVENT:
 *	ESROS-RESULT.request or ESROS-ERROR-request received.
 *
 *  CURRENT STATE:
 *	Invoke PDU Received.
 *
 *  ACTION:
 *	Add invoke reference number to the active list.
 *	Transmit ESRO-RESULT-PDU or ESRO-ERROR-PDU.
 *	Set ESRO-RESULT-PDU or ESRO-ERROR-PDU retransmission timer.
 *
 *  RESULTING STATE:
 *
 */

Int
tr_clPerformer02(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;
    DU_View du;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

#ifdef FUTURE
    if ( DU_size(FSM_evtInfo.resultReq.data) > RESULT_REQ_SIZE) {
	EH_problem("tr_clPerformer02: Result Request too long\n");
	return (FAIL);
    }
#endif

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer02: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    /* To be used for resultCnf */
    invoke->isResultNotError = FSM_evtInfo.resultReq.isResultNotError;

    invoke->encodingType  = FSM_evtInfo.resultReq.encodingType;
    invoke->userInvokeRef = FSM_evtInfo.resultReq.userInvokeRef;

    if (erop_refKeep(invoke->invokeRefNu, invoke) == FAIL) {
	EH_problem("tr_clPerformer02: erop_refKeep failed\n");
	return (FAIL);
    }

    erop_tmrSetValues(invoke);

    if (invoke->isResultNotError) {
        if ((du = erop_resultPdu(invoke, 
			         invoke->invokeRefNu, 
			         invoke->locSap->sapSel,
			         invoke->remEsroSapSel,
			         FSM_evtInfo.resultReq.encodingType,
			         FSM_evtInfo.resultReq.parameter)) == NULL) {
	    EH_problem("tr_clPerformer02: erop_resultPdu failed\n");
	    return (FAIL);
    	}
    } else {
        if ((du = erop_errorPdu(invoke, 
			        invoke->invokeRefNu, 
			        invoke->locSap->sapSel,
			        invoke->remEsroSapSel,
			        FSM_evtInfo.errorReq.encodingType,
			        FSM_evtInfo.errorReq.errorValue,
			        FSM_evtInfo.errorReq.parameter)) == NULL) {
	    EH_problem("tr_clPerformer02: erop_errorPdu failed\n");
	    return (FAIL);
    	}
    }

#ifdef FUTURE
    if (erop_pduKeep(invoke, &invoke->resultPduSeq, du) == FAIL) {
	EH_problem("tr_clPerformer02: erop_pduKeep failed\n");
        DU_free(du); 
	return (FAIL);
    }
#else
    erop_pduKeep(invoke, &invoke->resultPduSeq, du);
#endif

    if ((du = erop_pduRetrieve(invoke, &invoke->resultPduSeq)) == NULL) {
        EH_problem("tr_clPerformer02: PDU can not be retrieved "
		   "for retransmission\n");
        DU_free(du); 
        return (FAIL);
    }
    if (UDP_dataReq(erop_udpSapDesc,
		    &(invoke->remTsapSel),
		    &(invoke->remNsapAddr),
		    du) 
        == FAIL) {
	EH_problem("tr_clPerformer02: UDP_dataReq failed\n");
	retVal = FAIL;
    }

    TM_TRACE((DU_modCB, DU_MALLOC, "tr_clPerformer02: DU_free: du=0x%lx\n", du));

    DU_free(du); 

    if (erop_tmrCreate(invoke, erop_resultPduRetranTimer, 
		       invoke->retransTimeout, 0) == FAIL) {
	EH_problem("tr_clPerformer02: erop_tmrCreate failed\n");
	return (FAIL);
    }
    erop_tmrCancel(invoke, erop_perfNoResponseTimer);

    return retVal;

} /* tr_clPerformer02() */

/*
 *  
 *  EVENT:
 *	ESRO-ACK-PDU received.
 *
 *  CURRENT STATE:
 *	ACK-PDU Wait.
 *
 *  ACTION:
 *	Initialize invoke refernce number timer.
 *	Issue ESROS-RESULT.confirm or ESROS-ERROR.confirm.
 *
 *  RESULTING STATE:
 *	Performer RefNu Wait.
 *
 */

Int
tr_clPerformer03(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

#ifdef FUTURE
    if ( erop_pdu.pdutype != ACK_PDU ) {
	EH_problem("tr_clPerformer03: PDU is not ACK PDU\n");
	return (FAIL);
    }
#endif

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer03: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    erop_tmrCancel(invoke, erop_allTimer);

    if (erop_relPdu(invoke, &invoke->resultPduSeq) == FAIL) {
	EH_problem("tr_clPerformer03: erop_relPdu failed\n");
	retVal = FAIL;
    }

/*** REMOVE (begin) ***/
if (dropResultConfirm)	{		/* this is used for testing only */

    invoke->failureReason = ESRO_FailureTransmission;

    if (erop_failureInd(invoke, 
			invoke->userInvokeRef,
			invoke->failureReason) == FAIL) {
	EH_problem("tr_clPerformer09: erop_failureInd failed\n");
	retVal = FAIL;
    }

    erop_tmrCancel(invoke, erop_allTimer);

    if (erop_relPdu(invoke, &invoke->resultPduSeq) == FAIL) {
	EH_problem("tr_clPerformer09: erop_relPdu failed\n");
	retVal = FAIL;
    }

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	/* Release refNu */
	EH_problem("tr_clPerformer09: erop_tmrCreate failed\n");
  	return FAIL;  
    }
    return retVal;
}
/*** REMOVE (end) ***/

    if (invoke->isResultNotError) {
	retVal = erop_resultCnf(invoke, invoke->userInvokeRef);
    } else {
	retVal = erop_errorCnf(invoke, invoke->userInvokeRef);
    }	

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	/* Release refNu */
	EH_problem("tr_clPerformer03: erop_tmrCreate failed\n");
  	return FAIL;  
    }

    return retVal;

} /* tr_clPerformer03() */

/*
 *
 *  EVENT:
 *	Duplicate ESRO-INVOKE-PDU received.
 *
 *  CURRENT STATE:
 *	Invoke PDU Received.
 *
 *  ACTION:
 *	No action (ignore the duplicate ESRO-INVOKE-PDU).
 *
 *  RESULTING STATE:
 *	Stay in "Inovke PDU Received".
 *
 */

Int
tr_clPerformer04(Void *machine, Void *userData, FSM_EventId evtId)
{
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if ( erop_pdu.pdutype != INVOKE_PDU ) {
	EH_problem("tr_clPerformer04: PDU is not invoke PDU\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer04: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    TM_TRACE((EROP_modCB, TM_PDUIN, 
	     "<- (DUPLICATE) Invoke PDU while waiting for ESROS user (Performer machine)"));

    if (erop_pdu.data != (DU_View)0) {

        TM_TRACE((DU_modCB, DU_MALLOC, 
	         "tr_clPerformer04: DU_free: erop_pdu.data=0x%lx\n", 
		 erop_pdu.data));

	DU_free(erop_pdu.data);
    }

    return (SUCCESS);
}

/*
 *  
 *  EVENT:
 *	ESRO-RESULT-PDU or ESRO-ERROR-PDU retransmission timer expired.
 *
 *  CURRENT STATE:
 *	ACK-PDU Wait.
 *
 *  ACTION:
 *	Retransmit ESRO-RESULT-PDU or ESRO-ERROR-PDU while number of 
 *	  retransmissions is less than maximum number of retransmissions.
 *	Increment the number of retransmissions timer.
 *
 *  RESULTING STATE:
 *	Stay in ACK-PDU Wait.
 *
 */

Int
tr_clPerformer05(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;
    DU_View du;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

#ifdef FUTURE
    if (FSM_evtInfo.tmrInfo.name != erop_resultPduRetranTimer) {
	EH_problem("tr_clPerformer05: Event is not Result PDU Retrans Timer\n");
	return (FAIL);
    }
#endif

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer05: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    if (FSM_evtInfo.tmrInfo.datum >= invoke->nuOfRetrans) {
#if 0
        if ((du = erop_pduRetrieve(invoke, &invoke->resultPduSeq)) 
	    != (DU_View)0) {
            DU_free(du); 
	}
#endif
	if (erop_tmrCreate(invoke, erop_lastTimer, invoke->rwait, 0) == FAIL) {
	    EH_problem("tr_clPerformer05: erop_tmrCreate failed\n");
	    return (FAIL);
	} else {
            return (SUCCESS);
	}
    } else {
	if ((du = erop_pduRetrieve(invoke, &invoke->resultPduSeq)) == NULL) {
	    EH_problem("tr_clPerformer05: PDU can not be retrieved for retransmission\n");
	    return (FAIL);
        }
	if (UDP_dataReq(erop_udpSapDesc,
		        &(invoke->remTsapSel),
		        &(invoke->remNsapAddr),
		        du)
	    == FAIL) {
	    EH_problem("tr_clPerformer05: UDP_dataReq failed\n");
	    retVal = FAIL;
    	}

        TM_TRACE((EROP_modCB, TM_PDUIN, 
	         "\n-> (RETRANSMIT) Result PDU while waiting for ACK and retrans timer went off (Performer)"));

        TM_TRACE((DU_modCB, DU_MALLOC, 
		 "tr_clPerformer05: DU_free: du=0x%lx\n", du));

	DU_free(du);

	if (erop_tmrCreate(invoke,
		       erop_resultPduRetranTimer, 
		       invoke->retransTimeout,
		       FSM_evtInfo.tmrInfo.datum + 1) == FAIL) {
	    EH_problem("tr_clPerformer05: erop_tmrCreate failed\n");
	    return (FAIL);
	}

	if (retVal == SUCCESS) {
	    NM_incCounter(NM_ESROS, &erop_pduRetranCounter, 1);
	}
    }

    return retVal;

} /* tr_clPerformer05() */


/*
 *
 *  EVENT:
 *	Duplicate ESRO-INVOKE-PDU received.
 *
 *  CURRENT STATE:
 *	ACK-PDU Wait.
 *
 *  ACTION:
 *	Send result PDU.
 *
 *  RESULTING STATE:
 *	Stay in ACK-PDU Wait state.
 * 
 */

Int
tr_clPerformer06(Void *machine, Void *userData, FSM_EventId evtId)
{
    DU_View du;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

#ifdef FUTURE
    if (erop_pdu.pdutype != INVOKE_PDU ) {
	EH_problem("tr_clPerformer06: PDU is not invoke PDU\n");
	return (FAIL);
    }
#endif

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer06: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    TM_TRACE((EROP_modCB, TM_PDUIN, 
	     "<- (DUPLICATE) Invoke PDU while waiting for ACK (Performer machine)"));

    if (erop_pdu.data != (DU_View)0) {

	TM_TRACE((DU_modCB, DU_MALLOC, 
                 "tr_clPerformer06: DU_free: erop_pdu.data=0x%lx\n",
		 erop_pdu.data));

	DU_free(erop_pdu.data);
    }

    if ((du = erop_pduRetrieve(invoke, &invoke->resultPduSeq)) == NULL) {
        EH_problem("tr_clPerformer06: PDU can not be retrieved for retransmission\n");
        return (FAIL);
    }
    if (UDP_dataReq(erop_udpSapDesc,
		    &(invoke->remTsapSel),
		    &(invoke->remNsapAddr),
		    du) 
	== FAIL) {

	EH_problem("tr_clPerformer06: UDP_dataReq failed\n");
        TM_TRACE((DU_modCB, DU_MALLOC, 
		 "tr_clPerformer06: DU_free: du=0x%lx\n", du));

	DU_free(du);
	return FAIL;
    }

    TM_TRACE((EROP_modCB, TM_PDUIN, 
	     "\n-> (RETRANSMIT) Result PDU because duplicate invoke PDU received (Performer)"));

    TM_TRACE((DU_modCB, DU_MALLOC, "tr_clPerformer06: DU_free: du=0x%lx\n", du));

    DU_free(du);

    erop_tmrCancel(invoke, erop_resultPduRetranTimer);

    if (erop_tmrCreate(invoke, erop_resultPduRetranTimer, 
		       invoke->retransTimeout, 0) == FAIL) {
	EH_problem("tr_clPerformer06: erop_tmrCreate failed\n");
	return (FAIL);
    }

    return (SUCCESS);
}


/*
 *
 *  EVENT:
 *	Duplicate Invoke PDU received.
 *
 *  CURRENT STATE:
 *	Performer RefNu Wait.
 *
 *  ACTION:
 *	Reset Invoke Reference Number timer.
 *
 *  RESULTING STATE:
 *	Stay in Performer RefNu Wait.
 * 
 */

Int
tr_clPerformer07(Void *machine, Void *userData, FSM_EventId evtId)
{
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != INVOKE_PDU) {
	EH_problem("tr_clPerformer07: PDU is not Invoke\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer07: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    if (erop_pdu.data != (DU_View)0) {

	TM_TRACE((DU_modCB, DU_MALLOC, 
		 "tr_clPerformer07: DU_free: erop_pdu.data=0x%lx\n",
		 erop_pdu.data));

	DU_free(erop_pdu.data);
    }

    erop_tmrCancel(invoke, erop_refNuTimer);

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	EH_problem("tr_clPerformer07: erop_tmrCreate failed\n");
	return FAIL;
    }

    return (SUCCESS);
}


/*
 *
 *  EVENT:
 *	Internal failure.
 *
 *  CURRENT STATE:
 *	Inoke PDU Received.
 *
 *  ACTION:
 *	Send ESRO-FAILURE-PDU.
 *
 *  RESULTING STATE:
 *	CL Performer Start.
 * 
 */

Int
tr_clPerformer08(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;

#ifdef TM_ENABLED
    static char taskN[100] = "Release invoke: ";
#endif

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer08: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    invoke->failureReason = ESRO_FailureLocResource;

    if (erop_failureInd(invoke, 
			invoke->userInvokeRef, 
			invoke->failureReason) == FAIL) {
	EH_problem("tr_clPerformer08: erop_failureInd failed\n");
	retVal = FAIL;
    }

    if (erop_sendFailurePdu(&invoke->remTsapSel, &invoke->remNsapAddr, 
		            invoke->invokeRefNu, 
			    ESRO_FailureUserNotResponding) == FAIL) { 
						/* it can be local resource too */
	EH_problem("tr_2clPerformer04: erop_sendFailurePdu failed\n");
	retVal = FAIL;
    }

    erop_tmrCancel(invoke, erop_allTimer);

    if (erop_relPdu(invoke, &invoke->invokePduSeq) == FAIL) {
	EH_problem("tr_clPerformer08: erop_relPdu failed\n");
	retVal = FAIL;
    }

#if 0
    erop_refFree(invoke);
    erop_freeLopsInvoke(invoke);
    erop_invokeInfoFree(invoke);
#endif
#ifdef TM_ENABLED
        strcpy(taskN, "Release invoke: ");
    	SCH_submit ((Void *)erop_relInvoke, 
		           (Ptr) invoke, 
		           SCH_PSEUDO_EVENT, 
		           (String) strcat(taskN, TM_here()));
#else
        SCH_submit ((Void *)erop_relInvoke, 
		           (Ptr) invoke, 
		           SCH_PSEUDO_EVENT);
#endif

    TM_TRACE((DU_modCB, DU_MALLOC, 
	     "tr_clPerformer08: DU_free: erop_pdu.data=0x%lx\n", erop_pdu.data));

#if 0   /* check this */
    DU_free(erop_pdu.data);
#endif

    NM_incCounter(NM_ESROS, &erop_completeOperationCounter, -1);

    return retVal;

}


/*
 *
 *  EVENT:
 *	Last timer.
 *
 *  CURRENT STATE:
 *	ACK-PDU Wait.
 *
 *  ACTION:
 *	Issue ESROS-FAILURE.indication.
 *	Initialize invoke reference number timer.
 *
 *  RESULTING STATE:
 *	Performer RefNu Wait.
 * 
 */

Int
tr_clPerformer09(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = (SUCCESS);

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    invoke->failureReason = ESRO_FailureTransmission;

    if (erop_failureInd(invoke, 
			invoke->userInvokeRef,
			invoke->failureReason) == FAIL) {
	EH_problem("tr_clPerformer09: erop_failureInd failed\n");
	retVal = FAIL;
    }

    erop_tmrCancel(invoke, erop_allTimer);

    if (erop_relPdu(invoke, &invoke->resultPduSeq) == FAIL) {
	EH_problem("tr_clPerformer09: erop_relPdu failed\n");
	retVal = FAIL;
    }

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	/* Release refNu */
	EH_problem("tr_clPerformer09: erop_tmrCreate failed\n");
  	return FAIL;  
    }

    return retVal;
}


/*
 *
 *  EVENT:
 *	Reference number timer expired.
 *
 *  CURRENT STATE:
 *	Performer RefNu Wait.
 *
 *  ACTION:
 *	Release the invoke reference number.
 *
 *  RESULTING STATE:
 *	CL Performer Start.
 * 
 */

Int
tr_clPerformer10(Void *machine, Void *userData, FSM_EventId evtId)
{
#ifdef TM_ENABLED
    static char taskN[100] = "Release invoke: ";
#endif

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (FSM_evtInfo.tmrInfo.name != erop_refNuTimer) {
	EH_problem("tr_clPerformer10: Event is not reference number timer\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer10: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif
    
#if 0
    erop_refFree(invoke);
    erop_freeLopsInvoke(invoke);
    erop_invokeInfoFree(invoke);
#endif

#ifdef TM_ENABLED
        strcpy(taskN, "Release invoke: ");
    	return SCH_submit ((Void *)erop_relInvoke, 
		           (Ptr) invoke, 
		           SCH_PSEUDO_EVENT, 
		           (String) strcat(taskN, TM_here()));
#else
    	return SCH_submit ((Void *)erop_relInvoke, 
		           (Ptr) invoke, 
		           SCH_PSEUDO_EVENT);
#endif


    return (SUCCESS);
}


/*
 *
 *  EVENT:
 *	Duplicate ESRO-ACK-PDU received.
 *
 *  CURRENT STATE:
 *	Performer RefNu Wait.
 *
 *  ACTION:
 *	Reset Invoke Reference Number timer.
 *
 *  RESULTING STATE:
 *	Stay in Performer RefNu Wait state.
 * 
 */

Int
tr_clPerformer11(Void *machine, Void *userData, FSM_EventId evtId)
{
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != ACK_PDU) {
	EH_problem("tr_clPerformer11: PDU is not Ack\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clPerformer11: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    if (erop_pdu.data != (DU_View)0) {
	DU_free(erop_pdu.data);

	TM_TRACE((DU_modCB, DU_MALLOC, "tr_clPerformer11: DU_free: du=0x%lx\n",
		 erop_pdu.data));
    }

    erop_tmrCancel(invoke, erop_refNuTimer);

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	EH_problem("tr_clPerformer11: erop_tmrCreate failed\n");
	return FAIL;
    }

    return (SUCCESS);
}


/*
 *
 *  EVENT:
 *	Hold-on ACK request.
 *
 *  CURRENT STATE:
 *	Invoke PDU Received.
 *
 *  ACTION:
 *	Send Hold-on ESRO-ACK-PDU.
 *
 *  RESULTING STATE:
 *	Stay in Invoke PDU Received.
 * 
 */

Int
tr_clPerformer12(Void *machine, Void *userData, FSM_EventId evtId)
{
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    EH_problem("tr_clPerformer12: Hold-on ack is not implemented yet\n");

    /* NOTYET */
/*
    User said hang on while I perform
*/
	return (SUCCESS);
}
