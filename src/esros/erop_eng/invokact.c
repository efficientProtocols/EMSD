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
 * File: invokact.c (INVOKe ACTions)
 *
 * Description: This file contains the actions to be taken at each step 
 *              of the ESROS transition Diagrams.
 *
 * Functions:
 *   tr_clInvoker01(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker02(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker03(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker04(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker05(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker06(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker07(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker08(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker09(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_clInvoker10(Void *machine, Void *userData, FSM_EventId evtId)
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: invokact.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "invoke.h"
#include "erop_sap.h"

#include "eropfsm.h"
#include "eh.h"
#include "nm.h"
#include "layernm.h"
#include "eropdu.h"
#include "udp_if.h"

#include "local.h"
#include "extfuncs.h"
#include "target.h"
#include "erop_cfg.h"

#include "sch.h"

extern Pdu erop_pdu;

extern Void (*erop_freeLopsInvoke)(InvokeInfo *);

/*
 *  EVENT:
 *	An EROP_INVOKE.request event occurs at the User interface.
 *
 *  CURRENT STATE:
 *	IDLE.
 *
 *  ACTION:
 *
 *	The InvokeInfo saves the remote address,  and the parameter
 *      which the user wishes to invoke. 
 *
 *      Allocates an invoke reference number to identify the invokation. 
 *
 *      The INVOKE.REQ.PDU is formed and transmitted
 *	to the appropriate remote destination.
 *
 *      The retransmission timer is initialized.
 *
 *  RESULTING STATE:
 *	The INVOKER RESULT/ERROR PDU WAIT state.
 */

Int
tr_clInvoker01(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;
    DU_View du;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (DU_size(FSM_evtInfo.invokeReq.parameter) > invokePduSize) {
	EH_problem("tr_clInvoker01: Message should be sent connection oriented\n");
        return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker01: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    invoke->remEsroSapSel  = FSM_evtInfo.invokeReq.remESROSap;
    invoke->remTsapSel     = *FSM_evtInfo.invokeReq.remTsap;
    invoke->remNsapAddr    = *FSM_evtInfo.invokeReq.remNsap;
    invoke->operationValue = FSM_evtInfo.invokeReq.opValue;
    invoke->encodingType   = FSM_evtInfo.invokeReq.encodingType;
    invoke->userInvokeRef  = FSM_evtInfo.invokeReq.userInvokeRef;

    erop_tmrSetValues(invoke);

    if ( (invoke->invokeRefNu = erop_refNew(invoke)) == (FAIL)) {
	EH_problem("tr_clInvoker01: No more reference number available\n");
	return (FAIL);
    }

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker01: Invoke.request: refNu=%ld", 
	     invoke->invokeRefNu));

    if ((du = erop_invokePdu(invoke,
			     invoke->remEsroSapSel,
			     invoke->invokeRefNu,
			     invoke->operationValue,
			     invoke->encodingType,
			     FSM_evtInfo.invokeReq.parameter)) == NULL) {
	EH_problem("tr_clInvoker01: erop_invokePdu failed\n");
	return FAIL;
    }

#ifdef FUTURE
    if (erop_pduKeep(invoke, &invoke->invokePduSeq, du) == FAIL) {
	EH_problem("tr_clInvoker01: erop_pduKeep failed\n");
	DU_free(du);
	return (FAIL);
    }
#else
    erop_pduKeep(invoke, &invoke->invokePduSeq, du);
#endif

    if ((du = erop_pduRetrieve(invoke, &invoke->invokePduSeq)) == NULL) {
        EH_problem("tr_clInvoker01: PDU can not be retrieved\n");
        return (FAIL);
    }
    if (UDP_dataReq(erop_udpSapDesc,
		    &(invoke->remTsapSel),
		    &(invoke->remNsapAddr),
		    du) 
        == FAIL) {
	EH_problem("tr_clInvoker01: UDP_dataReq failed\n");
 	retVal = FAIL;      /* return error value */
    }

    TM_TRACE((DU_modCB, DU_MALLOC, "tr_clInvoker01: DU_free: du=0x%lx\n", du));

    DU_free(du);

    if (erop_tmrCreate(invoke, 
		       erop_invokePduRetranTimer, 
		       invoke->retransTimeout, 0) == FAIL) {
	
	EH_problem("tr_clInvoker01: erop_tmrCreate failed\n");
	return (FAIL);
    }

    if (retVal == SUCCESS) {
	NM_incCounter(NM_ESROS, &erop_completeOperationCounter, 1);
    }

    return retVal;

} /* tr_clInvoker01() */

/*
 *
 *  EVENT:
 *	A TIMER-EXPIRATION event occurs; this indicates that the retransmission
 *	interval has expired since the last PDU transmission or the first 
 *	transmission after INVOKE.request.
 *
 *  CURRENT STATE:
 *	InvokePduSent
 *
 *  ACTION:
 *	Retransmit ESRO-INVOKE-PDU while number of retransmissions is less
 *	than maximum number of retransmissions ( invoke->nuOfRetrans).
 *
 *	Increment the retransmission counter. When maximum number of
 *	retransmissions reached, start the last timer.
 *
 *  RESULTING STATE:
 *	The retransmission continues to await the arrival of the RES/ERR PDU,
 *	last timer time out (maximum number of retransmissions), or receipt 
 *      of failure PDU.
 */

Int
tr_clInvoker02(Void *machine, Void *userData, FSM_EventId evtId)
{
    DU_View du;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (FSM_evtInfo.tmrInfo.name != erop_invokePduRetranTimer) {
	EH_problem("tr_clInvoker02: Event is not retransmission timer\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker02: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker02: InvokePDU retransmit: refNu=%ld", 
	     invoke->invokeRefNu));

    if (FSM_evtInfo.tmrInfo.datum >= invoke->nuOfRetrans) {
	if (erop_tmrCreate(invoke, erop_lastTimer, invoke->rwait, 0) == FAIL) {
	    EH_problem("tr_clInvoker02: erop_tmrCreate failed\n");
	    return (FAIL);
        } else {
	    return (SUCCESS);
	} 
    } else {
	if (erop_tmrCreate(invoke, 
		           erop_invokePduRetranTimer,
		           invoke->retransTimeout,
		           FSM_evtInfo.tmrInfo.datum + 1) == FAIL) {
	    EH_problem("tr_clInvoker02: erop_tmrCreate failed\n");
	    return (FAIL);
	}

    	if ((du = erop_pduRetrieve(invoke, &invoke->invokePduSeq)) == NULL) {
            EH_problem("tr_clInvoker02: PDU can not be retrieved for retransmission\n");
            return (FAIL);
        }

	if (UDP_dataReq(erop_udpSapDesc,
		        &(invoke->remTsapSel),
		        &(invoke->remNsapAddr),
		        du)
	    == FAIL) {

	    EH_problem("tr_clInvoker02: UDP_dataReq failed\n");
	    TM_TRACE((DU_modCB, DU_MALLOC, 
		     "tr_clInvoker02: DU_free: du=0x%lx\n", du));

	    DU_free(du);
	    return (FAIL);
	}

        TM_TRACE((EROP_modCB, TM_PDUIN, 
	         "\n-> (RETRANSMIT) Invoke PDU (retrans timer) while waiting for Result PDU (Invoker)"));

        TM_TRACE((DU_modCB, DU_MALLOC, "tr_clInvoker02: DU_free: du=0x%lx\n", du));

   	DU_free(du);

	NM_incCounter(NM_ESROS, &erop_invokePduRetranCounter, 1);
	NM_incCounter(NM_ESROS, &erop_pduRetranCounter, 1);
    }
    return (SUCCESS);

} /* tr_clInvoker02() */

/*
 *
 *  EVENT:
 *	The TIMER-EXPIRATION event occurs (LastTimer) after maximum 
 *	retransmission of ESRO-INVOKE-PDU.
 *
 *  CURRENT STATE:
 *	InvokePduSent
 *
 *  ACTION:
 *	Issue ESROS-FAILURE.indication primitive.
 *	Initialize reference number timer.
 *
 *  RESULTING STATE:
 *	InvokerRefWait (wail until reference number expires).
 */

Int
tr_clInvoker03(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (FSM_evtInfo.tmrInfo.name != erop_lastTimer) {
	EH_problem("tr_clInvoker03: Event is not last timer\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker03: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker03: Last Timer: refNu=%ld", 
	     invoke->invokeRefNu));

    if (erop_relAllPdu(invoke) == FAIL) {
	EH_problem("tr_clInvoker03: erop_relAllPdu function failed\n");
	retVal = FAIL;
    }

    invoke->failureReason = ESRO_FailureUserNotResponding;

    if (erop_failureInd(invoke, 
		        invoke->userInvokeRef,
			invoke->failureReason) == FAIL) {
	EH_problem("tr_clInvoker03: erop_failureInd failed\n");
	retVal = FAIL;
    }

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	EH_problem("tr_clInvoker03: erop_tmrCreate failed\n");
  	return (FAIL);	/* return error value */
    }

    NM_incCounter(NM_ESROS, &erop_completeOperationCounter, -1);

    return retVal;

} /* tr_clInvoker03() */

/*
 * 
 *  EVENT:
 *	ESRO-RESULT-PDU or ESRO-ERROR-PDU is received.
 *
 *  CURRENT STATE:
 *	InvokePduSent.
 *
 *  ACTION:
 *	Send ESRO-ACK-PDU.
 *	Issue ESROS-RESULT.indication or ESROS-ERROR.indication privmitive.
 *	Initialize inactivity timer.
 *
 *  RESULTING STATE:
 *	ACK-PDU Send.
 *
 */

Int
tr_clInvoker04(Void *machine, Void *userData, FSM_EventId evtId)
{
    DU_View du;
    Int retVal = SUCCESS;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != RESULT_PDU && erop_pdu.pdutype != ERROR_PDU) {
	EH_problem("tr_clInvoker04: PDU is not Result or Error\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker04: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    erop_tmrCancel(invoke, erop_allTimer);

    invoke->invokeRefNu  = erop_pdu.invokeRefNu;
    invoke->encodingType = erop_pdu.encodingType;
    if (erop_pdu.pdutype == ERROR_PDU) {
	invoke->errorValue = erop_pdu.errorValue;
    }

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker04: Result/Error PDU: refNu=%ld", 
	     invoke->invokeRefNu));

    erop_tmrSetValues(invoke);

    if (erop_relPdu(invoke, &invoke->invokePduSeq) == FAIL) {
	EH_problem("tr_clInvoker04: erop_relPdu failed\n");
 	retVal = FAIL;
    }

#ifdef TIMER_DYNAMIC
    erop_tmrUpdate(invoke, TMR_diff(TMR_getFreeCnt() - erop_timeStamp));
#endif

    if (erop_pdu.pdutype == RESULT_PDU) {
      	if (erop_resultInd(invoke,
		           invoke->userInvokeRef,
		           invoke->encodingType,
		           (DU_View) erop_pdu.data) == FAIL) {
	    EH_problem("tr_clInvoker04: erop_resultInd failed\n");

            erop_sendFailurePdu(&invoke->remTsapSel, &invoke->remNsapAddr, 
			        invoke->invokeRefNu, 
				ESRO_FailureUserNotResponding);
	    return (FAIL);
	}
    } else {
      	if (erop_errorInd(invoke,
		          invoke->userInvokeRef,
		          invoke->encodingType,
			  (EROP_ErrorValue)   invoke->errorValue,
		          (DU_View) erop_pdu.data) == FAIL) {
	    EH_problem("tr_clInvoker04: erop_errorInd failed\n");

            erop_sendFailurePdu(&invoke->remTsapSel, &invoke->remNsapAddr, 
			        invoke->invokeRefNu, 
				ESRO_FailureUserNotResponding);
	    return (FAIL);
	}
    }

    if ((du = erop_ackPdu(invoke, invoke->invokeRefNu)) == NULL) {
	EH_problem("tr_clInvoker04: erop_ackPdu failed\n");
	return (FAIL);
    }

    if (UDP_dataReq(erop_udpSapDesc, &(invoke->remTsapSel), 
		    &(invoke->remNsapAddr), du) == FAIL) {
	if (du) {

	    EH_problem("tr_clInvoker04: UDP_dataReq failed\n");
	    TM_TRACE((DU_modCB, DU_MALLOC, 
		     "tr_clInvoker04: DU_free: du=0x%lx\n", du));

	    DU_free(du);
	}
	return (FAIL);
    }

    if (du) {

        TM_TRACE((DU_modCB, DU_MALLOC, "tr_clInvoker04: DU_free: du=0x%lx\n", du));

        DU_free(du);
    }

    if (erop_tmrCreate(invoke, erop_inactivityTimer, invoke->inactivityDelay, 0)
	== FAIL) {
	EH_problem("tr_clInvoker04: erop_tmrCreate failed\n");
	return (FAIL);
    }

#ifdef FUTURE
    NM_incCounter(NM_ESROS, &erop_s_o_conNOTYETCounter, 1);
#endif

    return retVal;
}


/*
 *  EVENT:
 *	ESRO-FAILURE-PDU is received.
 *
 *  CURRENT STATE:
 *	Invoke PDU sent.
 *
 *  ACTION:
 *	Issue ESROS-FAILURE.indication primitive with User not Responding
 *	  failure cause.
 *	Initialize reference number timer.
 *
 *  RESULTING STATE:
 *	Invoker RefNu Wait.
 *
 */

Int
tr_clInvoker05(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != FAILURE_PDU) {
	EH_problem("tr_clInvoker05: PDU is not Failure\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker05: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker05: Failure PDU: refNu=%ld", 
	     invoke->invokeRefNu));

    if (erop_failureInd(invoke, 
		        invoke->userInvokeRef,
			invoke->failureReason) == FAIL) {
	EH_problem("tr_clInvoker05: erop_failureInd failed\n");
	retVal = FAIL;
    }

    erop_tmrCancel(invoke, erop_allTimer);

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) == FAIL) {
	EH_problem("tr_clInvoker05: erop_tmrCreate failed\n");
   	retVal = FAIL; 	/* return error value */
    }

    DU_free(erop_pdu.data); 

    NM_incCounter(NM_ESROS, &erop_completeOperationCounter, -1);

    return retVal;

} /* tr_clInvoker05() */


/*
 *
 *  EVENT:
 *	ESRO-ACK-PDU (Hold on) received.
 *
 *  CURRENT STATE:
 *	Invoke PDU sent.
 *
 *  ACTION:
 *	For future use (no action).
 *
 *  RESULTING STATE:
 *	Stays in Invoke PDU Sent until timer time-out.
 *
 */

Int
tr_clInvoker06(Void *machine, Void *userData, FSM_EventId evtId)
{
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    TM_TRACE((EROP_modCB, TM_ENTER, "tr_clInvoker06: Hold-on Ack PDU"));

    EH_problem("tr_clInvoker06: Hold-on ACK is not implemented yet\n");
    /* Got an ack which means don't re-transmit so often */
    return (SUCCESS);
}

/*
 *  EVENT:
 *	Duplicate ESRO-RESULT-PDU or ESRO-ERROR-PDU received.
 *
 *  CURRENT STATE:
 *	ACK-PDU Sent.
 *
 *  ACTION:
 *	Initialize inactivity timer (ignore PDU).
 *	Send ESRO-ACK-PDU.
 *
 *  RESULTING STATE:
 *	Stay in "ACK-PDU Send" state until the inactivity timer time-out.
 */

Int
tr_clInvoker07(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;
    DU_View du;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != RESULT_PDU && erop_pdu.pdutype != ERROR_PDU) {
	EH_problem("tr_clInvoker07: PDU is not Result or Error\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker07: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker07: Duplicate Result/Error PDU"));
    TM_TRACE((EROP_modCB, TM_PDUIN, 
	     "<- (DUPLICATE) Result PDU in Ack PDU Sent state (Invoker)"));

    if (erop_pdu.data != (DU_View)0) {

	TM_TRACE((DU_modCB, DU_MALLOC, "tr_clInvoker07: DU_free: du=0x%lx\n",
		 erop_pdu.data));

	DU_free(erop_pdu.data);
    }

    erop_tmrCancel(invoke, erop_inactivityTimer);

    if (erop_tmrCreate(invoke, 
		       erop_inactivityTimer, 
		       invoke->inactivityDelay, 0) == FAIL) {
	EH_problem("tr_clInvoker07: erop_tmrCreate failed\n");
   	return (FAIL);	/* return error value */
    }

    if ((du = erop_ackPdu(invoke, invoke->invokeRefNu)) == NULL) {
	EH_problem("tr_clInvoker07: erop_ackPdu failed\n");
	return (FAIL);
    }

    if (UDP_dataReq(erop_udpSapDesc, &(invoke->remTsapSel), 
		    &(invoke->remNsapAddr), du) == FAIL) {
	EH_problem("tr_clInvoker07: UDP_dataReq failed\n");
	retVal = (FAIL);
    }

    TM_TRACE((EROP_modCB, TM_PDUIN, 
	     "\n-> (RETRANSMISSION) Ack PDU retransmitted"
	     " because of duplicate result PDU (Invoker)"));

    TM_TRACE((DU_modCB, DU_MALLOC, "tr_clInvoker07: DU_free: du=0x%lx\n", du));

    DU_free(du);

    return retVal;

} /* tr_clInvoker07() */

/*
 * 
 *  EVENT:
 *	RefNu Timer time-out.
 *
 *  CURRENT STATE:
 *     Invoker RefNu Wait (waiting for expiration of the reference timer).
 *
 *  ACTION:
 *	Release the invoke reference number.
 *
 *  RESULTING STATE:
 *	IDLE.
 */

Int
tr_clInvoker08(Void *machine, Void *userData, FSM_EventId evtId)
{
#ifdef TM_ENABLED
    static char taskN[100] = "Release Invoke: ";
#endif
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (FSM_evtInfo.tmrInfo.name != erop_refNuTimer) {
	EH_problem("tr_clInvoker08: Event is not Reference Number timer\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker08: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

#if 0
    erop_refFree(invoke);
    erop_freeLopsInvoke(invoke);
    erop_invokeInfoFree(invoke);
#endif

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker08: RefNu Timer: refNu=%ld", invoke->invokeRefNu));

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

} /* tr_clInvoker08() */

/*
 *
 *  EVENT:
 *	ESRO-RESULT-PDU or ESRO-ERROR-PDU received.
 *
 *  CURRENT STATE:
 *	Invoker RefNu Wait.
 *
 *  ACTION:
 *	Reset invoke reference number timer.
 *
 *  RESULTING STATE:
 *      Stays in the same state, i.e. "Invoker RefNu Wait".
 */

Int
tr_clInvoker09(Void *machine, Void *userData, FSM_EventId evtId)
{
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != RESULT_PDU && erop_pdu.pdutype != ERROR_PDU) {
	EH_problem("tr_clInvoker09: PDU is not Result or Error\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker09: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker09: Duplicate Result/Error PDU"));

    TM_TRACE((EROP_modCB, TM_PDUIN, 
	     "<- (DUPLICATE) Result PDU in Invoker RefNu Wait state (Invoker)"));

    if (erop_pdu.data != (DU_View)0) {

	TM_TRACE((DU_modCB, DU_MALLOC, 
                 "tr_clInvoker09: DU_free: erop_pdu.data=0x%lx\n",
		 erop_pdu.data));

	DU_free(erop_pdu.data);
    }

    erop_tmrCancel(invoke, erop_refNuTimer);

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	EH_problem("tr_clInvoker09: erop_tmrCreate failed\n");
   	return (FAIL);	/* return error value */
    }

    return (SUCCESS);

} /* tr_clInvoker09() */

/*
 *
 *  EVENT:
 *	Inactivity Timer expired.
 *
 *  CURRENT STATE:
 *	ACK-PDU Send.
 *
 *  ACTION:
 *	Initialize reference number timer.
 *
 *  RESULTING STATE:
 *	Invoker RefNu Wait.
 */

SuccFail
tr_clInvoker10(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (FSM_evtInfo.tmrInfo.name != erop_inactivityTimer) {
	EH_problem("tr_clInvoker10: Event is not inactivity timer\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_clInvoker10: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "tr_clInvoker10: Inactivity Timer: refNu=%ld", 
	     invoke->invokeRefNu));

    erop_tmrCancel(invoke, erop_allTimer);

    if (erop_relAllPdu(invoke) == FAIL) {
	EH_problem("tr_clInvoker10: erop_relAllPdu failed\n");
	retVal = FAIL;
    }	

    /* Operation has been completed */

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	EH_problem("tr_clInvoker10: erop_tmrCreate failed\n");
   	return (FAIL);	/* return error value */
    }

    return retVal;

} /* tr_clInvoker10() */
