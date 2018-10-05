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
 * File: invokact2.c (INVOKe ACTions)
 *
 * Description: This file contains the actions to be taken at each step 
 *              of the ESROS transition Diagrams.
 *
 * Functions:
 *   tr_2clInvoker01(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_2clInvoker02(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_2clInvoker03(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_2clInvoker04(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_2clInvoker05(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_2clInvoker06(Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_2clInvoker07(Void *machine, Void *userData, FSM_EventId evtId)
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: invact2.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
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
tr_2clInvoker01(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;
    DU_View du;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (DU_size(FSM_evtInfo.invokeReq.parameter) > invokePduSize) {
	EH_problem("tr_2clInvoker01: Message should be sent connection oriented");
        return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_2clInvoker01: Called with invalid invoke pointer\n");
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
	EH_problem("tr_2clInvoker01: No more reference number available");
	return (FAIL);
    }

    if ((du = erop_invokePdu(invoke,
			     invoke->remEsroSapSel,
			     invoke->invokeRefNu,
			     invoke->operationValue,
			     invoke->encodingType,
			     FSM_evtInfo.invokeReq.parameter)) == NULL) {
	return FAIL;
    }

#ifdef FUTURE
    if (erop_pduKeep(invoke, &invoke->invokePduSeq, du) == FAIL) {
	EH_problem("tr_2clInvoker01: erop_pduKeep failed\n");
	DU_free(du);
	return (FAIL);
    }
#else
    erop_pduKeep(invoke, &invoke->invokePduSeq, du);
#endif

    if ((du = erop_pduRetrieve(invoke, &invoke->invokePduSeq)) == NULL) {
        EH_problem("tr_2clInvoker01: PDU can not be retrieved\n");
        return (FAIL);
    }
    if (UDP_dataReq(erop_udpSapDesc,
		    &(invoke->remTsapSel),
		    &(invoke->remNsapAddr),
		    du) 
        == FAIL) {
 	retVal = FAIL;      /* return error value */
    }

    TM_TRACE((DU_modCB, DU_MALLOC, "tr_2clInvoker01: DU_free: du=0x%lx\n", du));

    DU_free(du);

    if (erop_tmrCreate(invoke, 
		       erop_invokePduRetranTimer, 
		       invoke->retransTimeout, 0) == FAIL) {
	
        EH_problem("tr_2clInvoker01: erop_tmrCreate failed\n");
	return (FAIL);
    }

    if (retVal == SUCCESS) {
	NM_incCounter(NM_ESROS, &erop_completeOperationCounter, 1);
    }

    return retVal;

} /* tr_2clInvoker01() */

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
tr_2clInvoker02(Void *machine, Void *userData, FSM_EventId evtId)
{
    DU_View du;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (FSM_evtInfo.tmrInfo.name != erop_invokePduRetranTimer) {
	EH_problem("tr_2clInvoker02: Event is not retransmission timer");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_2clInvoker02: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    if (FSM_evtInfo.tmrInfo.datum >= invoke->nuOfRetrans) {
	if (erop_tmrCreate(invoke, erop_lastTimer, invoke->rwait, 0) == FAIL) {
	    EH_problem("tr_2clInvoker02: erop_tmrCreate failed\n");
	    return (FAIL);
        } else {
	    return (SUCCESS);
	} 
    } else {
	if (erop_tmrCreate(invoke, 
		           erop_invokePduRetranTimer,
		           invoke->retransTimeout,
		           FSM_evtInfo.tmrInfo.datum + 1) == FAIL) {
	    EH_problem("tr_2clInvoker02: erop_tmrCreate failed\n");
	    return (FAIL);
	}

    	if ((du = erop_pduRetrieve(invoke, &invoke->invokePduSeq)) == NULL) {
            EH_problem("tr_2clInvoker02: PDU can not be retrieved for retransmission\n");
            return (FAIL);
        }

	if (UDP_dataReq(erop_udpSapDesc,
		        &(invoke->remTsapSel),
		        &(invoke->remNsapAddr),
		        du)
	    == FAIL) {

	    TM_TRACE((DU_modCB, DU_MALLOC, 
		     "tr_2clInvoker02: DU_free: du=0x%lx\n", du));

	    DU_free(du);
	    return (FAIL);
	}

        TM_TRACE((DU_modCB, DU_MALLOC, 
		 "tr_2clInvoker02: DU_free: du=0x%lx\n", du));

   	DU_free(du);

	NM_incCounter(NM_ESROS, &erop_invokePduRetranCounter, 1);
	NM_incCounter(NM_ESROS, &erop_pduRetranCounter, 1);
    }
    return (SUCCESS);

} /* tr_2clInvoker02() */

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
tr_2clInvoker03(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (FSM_evtInfo.tmrInfo.name != erop_lastTimer) {
	EH_problem("tr_2clInvoker03: Event is not last timer");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_2clInvoker03: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    if (erop_relAllPdu(invoke) == FAIL) {
	EH_problem("tr_2clInvoker03: erop_relAllPdu function failed\n");
	retVal = FAIL;
    }

    invoke->failureReason = ESRO_FailureUserNotResponding;

    if (erop_failureInd(invoke, 
			invoke->userInvokeRef, 
			invoke->failureReason) == FAIL) {
	retVal = FAIL;
    }

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) 
	== FAIL) {
	EH_problem("tr_2clInvoker03: erop_tmrCreate failed\n");
  	return (FAIL);	/* return error value */
    }

    NM_incCounter(NM_ESROS, &erop_completeOperationCounter, -1);

    return retVal;

} /* tr_2clInvoker03() */

/*
 * 
 *  EVENT:
 *	ESRO-RESULT-PDU or ESRO-ERROR-PDU is received.
 *
 *  CURRENT STATE:
 *	InvokePduSent.
 *
 *  ACTION:
 *	Issue ESROS-RESULT.indication or ESROS-ERROR.indication privmitive.
 *	Initialize reference number timer.
 *
 *  RESULTING STATE:
 *	Invoke RefNu Wait.
 *
 */

Int
tr_2clInvoker04(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != RESULT_PDU && erop_pdu.pdutype != ERROR_PDU) {
	EH_problem("tr_2clInvoker04: PDU is not Result or Error");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_2clInvoker04: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    erop_tmrCancel(invoke, erop_allTimer);

    invoke->invokeRefNu  = erop_pdu.invokeRefNu;
    invoke->encodingType = erop_pdu.encodingType;
    if (erop_pdu.pdutype == ERROR_PDU) {
	invoke->errorValue = erop_pdu.errorValue;
    }

    erop_tmrSetValues(invoke);

    if (erop_relPdu(invoke, &invoke->invokePduSeq) == FAIL) {
	EH_problem("tr_2clInvoker04: erop_relPdu failed\n");
 	retVal = FAIL;
    }

#ifdef TIMER_DYNAMIC
    erop_tmrUpdate(invoke, TMR_diff(TMR_getFreeCnt() - erop_timeStamp));
#endif


    if (erop_pdu.pdutype == RESULT_PDU) {
      	if (erop_resultInd(invoke,
		           invoke->userInvokeRef,
		           (EROP_EncodingType) invoke->encodingType,
		           (DU_View) erop_pdu.data) == FAIL) {
	    return (FAIL);
	}
    } else {
      	if (erop_errorInd(invoke,
		          invoke->userInvokeRef,
		          (EROP_EncodingType) invoke->encodingType, 
			  invoke->errorValue,
		          (DU_View) erop_pdu.data) == FAIL) {
	    return (FAIL);
	}
    }

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) == FAIL) {
	EH_problem("tr_2clInvoker04: erop_tmrCreate failed\n");
	return (FAIL);
    }

#ifdef FUTURE
    NM_incCounter(NM_ESROS, &erop_s_o_conNOTYETCounter, 1);
#endif

    return retVal;

} /* tr_2clInvoker04() */


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
tr_2clInvoker05(Void *machine, Void *userData, FSM_EventId evtId)
{
    Int retVal = SUCCESS;

    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != FAILURE_PDU) {
	EH_problem("tr_2clInvoker05: PDU is not Failure\n");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_2clInvoker05: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    if (erop_failureInd(invoke, 
		        invoke->userInvokeRef,
			invoke->failureReason) == FAIL) {
	EH_problem("tr_2clInvoker05: erop_failureInd failed\n");
	retVal = FAIL;
    }

    erop_tmrCancel(invoke, erop_allTimer);

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) == FAIL) {
	EH_problem("tr_2clInvoker05: erop_tmrCreate failed\n");
   	retVal = FAIL; 	/* return error value */
    }

    DU_free(erop_pdu.data); 

    NM_incCounter(NM_ESROS, &erop_completeOperationCounter, -1);

    return retVal;

} /* tr_2clInvoker05() */

/*
 *  EVENT:
 *	Duplicate ESRO-RESULT-PDU or ESRO-ERROR-PDU received.
 *
 *  CURRENT STATE:
 *	Invoke Reference Number Wait.
 *
 *  ACTION:
 *	Initialize invoke refNu timer (ignore PDU).
 *
 *  RESULTING STATE:
 *	Stay in invoke refNu Wait state until the refNu timer time-out.
 */

Int
tr_2clInvoker06(Void *machine, Void *userData, FSM_EventId evtId)
{
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (erop_pdu.pdutype != RESULT_PDU && erop_pdu.pdutype != ERROR_PDU) {
	EH_problem("tr_2clInvoker06: PDU is not Result or Error");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_2clInvoker06: Called with invalid invoke pointer\n");
	return (FAIL);
    }
#endif

    if (erop_pdu.data != (DU_View)0) {

	TM_TRACE((DU_modCB, DU_MALLOC, "tr_2clInvoker06: DU_free: du=0x%lx\n",
		 erop_pdu.data));

	DU_free(erop_pdu.data);
    }

    erop_tmrCancel(invoke, erop_allTimer);

    if (erop_tmrCreate(invoke, erop_refNuTimer, invoke->refKeepTime, 0) == FAIL) {
	EH_problem("tr_2clInvoker06: erop_tmrCreate failed\n");
   	return (FAIL);	/* return error value */
    }

    return (SUCCESS);

} /* tr_2clInvoker06() */

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
tr_2clInvoker07(Void *machine, Void *userData, FSM_EventId evtId)
{
#ifdef TM_ENABLED
    static char taskN[100] = "Release Invoke: ";
#endif
    InvokeInfo *invoke;
    invoke = (InvokeInfo *)machine;

    if (FSM_evtInfo.tmrInfo.name != erop_refNuTimer) {
	EH_problem("tr_2clInvoker07: Event is not Reference Number timer");
	return (FAIL);
    }

#ifndef FAST
    if (invoke == 0) {
	EH_problem("tr_2clInvoker07: Called with invalid invoke pointer\n");
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

} /* tr_2clInvoker07() */

