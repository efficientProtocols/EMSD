/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of Neda-EMSD. An implementation of RFC-2524.
 * 
 * Neda-EMSD is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Neda-EMSD; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
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



#include "estd.h"
#include "eh.h"
#include "buf.h"
#include "tmr.h"
#include "mm.h"
#include "nvm.h"
#include "fsm.h"
#include "fsmtrans.h"
#include "mtsua.h"
#include "uacore.h"
#include "target.h"

#ifdef DELIVERY_CONTROL
/*
 * Delivery Control State Diagram
 *
 * E: = Event
 * P: = Predicate
 * A: = Action
 *
 * [x] = Identifier allowing multiple possible transitions between two states.
 *
 * Transitions are identified by the Starting State number, Ending State
 * number, and Identifier.
 *
 * If no Predicate is specified for a particular transition, then that
 * transition occurs whenever the specified Event is raised.
 *
 * If no Action is specified for a particular transition, then nothing other
 * than changing state occurs.
 *
 * The Action listed is the "primary" action; other side effects may occur
 * within the Action function.  Particularly, in many cases when returning to
 * State 1, the state machine is scheduled to be destroyed.
 *
 *
 * +--------+                       +--------+
 * |        |                       |        |
 * |        | [A]                   |        |
 * |        |>--------------------->|        |
 * | State  | E: DeliveryControl.req| State  |
 * |   1    | A: Issue              |   2    |
 * |        |    DeliveryControl.req|        |
 * |        |                       |        |
 * |        |                       |        |
 * |   I    |                   [A] | D      |
 * |   d    |<---------------------<| e R    |
 * |   l    | E: RESULT.ind         | l e W  |
 * |   e    | A: Notify User        | i s a  |
 * |        |                       | v p i  |
 * |        |                       | e o t  |
 * |        |                   [B] | r n    |
 * |        |<---------------------<| y s    |
 * |        | E: ERROR.ind          |   e    |
 * |        | P: ErrorType==        | C      |
 * |        |      TransientError   | o      |
 * |        | A: Retry later,       | n      |
 * |        |    Notify user        | t      |
 * |        |                   [C] | r      |
 * |        |<---------------------<| o      |
 * |        | E: ERROR.ind          | l      |
 * |        | P: ErrorType==        |        |
 * |        |      PermanentError   |        |
 * |        | A: Notify user of     |        |
 * |        |    delivery control   |        |
 * |        |    failure            |        |
 * |        |                       |        |
 * |        |                   [D] |        |
 * |        |<---------------------<|        |
 * |        | E: FAILURE.ind        |        |
 * |        | A: Retry later or     |        |
 * |        |    Notify user of     |        |
 * +--------+    failure            +--------+
 *
 */

/*
 * Local types
 */
typedef struct
{
    OS_Boolean 		    exists;
    EMSD_SegmentInfo 	    data;
} OptionalSegmentInfo;


/*
 * Forward Declarations
 */

/* States */
FORWARD static FSM_State	s1Idle;
FORWARD static FSM_State	s2DeliveryControlResponseWait;


/* Transition Diagrams */
FORWARD static FSM_Trans	t1Idle[2];
FORWARD static FSM_Trans	t2DeliveryControlResponseWait[6];


/* Predicates */
FORWARD static OS_Boolean
pS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS2S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);



/* Actions */
FORWARD static Int
aS1S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS2S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS2S1c(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aUnexpectedEvent(void * hMachine,
		 void * hUserData,
		 FSM_EventId eventId);

static ReturnCode
formatAndInvokeDeliveryControl(UA_Operation * pOperation,
		EMSD_DeliveryControlArgument * pEmsdDeliveryControlArg,
		void * hContents);



/*
 * Actual Declarations
 */

/* States */
static FSM_State 	s1Idle =
{
    NULL,
    NULL,
    t1Idle,
    "UA Delivery Control Idle"
};


static FSM_State	s2DeliveryControlResponseWait =
{
    NULL,
    NULL,
    t2DeliveryControlResponseWait,
    "UA Delivery Control Invoke Response Wait"
};




/* Transition Diagrams */
static FSM_Trans	t1Idle[] =
{
    {
        UA_FsmEvent_DeliveryControlRequest,
	NULL,
	aS1S2a,
	&s2DeliveryControlResponseWait,
	"s1s2a (Delivery Control Request)"
    },
    {
	FSM_EvtDefault,
	NULL,
	aUnexpectedEvent,
	&s1Idle,
	"Unexpected Event (default catch-all)"
    },
};


static FSM_Trans	t2DeliveryControlResponseWait[] =
{
    {
	UA_FsmEvent_ResultIndication,
	pS2S1a,
	aS2S1a,
	&s1Idle,
	"s2s1a (RESULT.ind)"
    },
    {
	UA_FsmEvent_ErrorIndication,
	pS2S1b,
	aS2S1b,
	&s1Idle,
	"s2s1b (ERROR.ind, errorType=Transient)"
    },
    {
	UA_FsmEvent_ErrorIndication,
	NULL,
	aS2S1c,
	&s1Idle,
	"s2s1c (ERROR.ind, errorType=Permanent)"
    },
    {
	UA_FsmEvent_FailureIndication,
	pS2S1d,
	aS2S1d,
	&s1Idle,
	"s2s1d (FAILURE.ind, max retries reached)"
    },
    {
	UA_FsmEvent_FailureIndication,
	NULL,
	aS2S2a,
	&s2DeliveryControlResponseWait,
	"s2s2a (FAILURE.ind, max retries not reached)"
    },
    {
	FSM_EvtDefault,
	NULL,
	aUnexpectedEvent,
	&s1Idle,
	"Unexpected Event (default catch-all)"
    }
};


/*
 * ua_deliveryControlFsmInit()
 */
ReturnCode
ua_deliveryControlFsmInit(void)
{
    if ((ua_globals.hDeliveryControlTransDiag =
	 FSM_TRANSDIAG_create("UA Delivery Control Transitions", &s1Idle)) 
	 == NULL)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "FSM for UA Delivery Control failed to initialize"));

	return Fail;
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
	      "FSM for UA Delivery Control initialized"));

    return Success;
}



/*
 * pS2S1a()
 *
 * Predicate function.
 *
 * Current State:          S2
 * Transition State:       S1
 *
 * Enabling Event:	   RESULT.indication
 *
 * Enabling Condition:	   Received result indication for delivery control
 */
static OS_Boolean
pS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "Predicate OK for S2->S1 (a)"));

    return TRUE;
}


/*
 * pS2S1b()
 *
 * Predicate function.
 *
 * Current State:          S2
 * Transition State:       S1
 *
 * Enabling Event:	   ERROR.indication
 *
 * Enabling Condition:	   Error type = Transient Error
 *
 */
static OS_Boolean
pS2S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	    pOperation = hUserData;
    EMSD_ErrorRequest *	    pErrorRequest = pOperation->hOperationData;

    /* The only transient error is Resource Error */
    if (pErrorRequest != NULL) {
      	if (pErrorRequest->error != EMSD_Error_ResourceError)
    	{
            TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		      "Predicate FAIL for S2->S1 (b): Error = %ld (not transient)",
		      pErrorRequest->error));
	
	    return FALSE;
    	}
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "Predicate OK for S2->S1 (b)"));

    return TRUE;
}


/*
 * pS2S1d()
 *
 * Predicate function.
 *
 * Current State:          S2
 * Transition State:       S1
 *
 * Enabling Event:	   FAILURE.indication
 *
 * Enabling Condition:	   Maximum number of FAILURE retries reached
 *
 */
static OS_Boolean
pS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *		pOperation = hUserData;

    /* See if we've reached the maximum number of retries */
    if (++pOperation->retryCount <= UA_MAX_FAILURE_RETRIES)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "Predicate OK for S2->S1 (d): Retry %u of %u",
		  pOperation->retryCount,
		  UA_MAX_FAILURE_RETRIES));

	/* Nope. */
	return FALSE;
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "Predicate FAIL for S2-S1 (d): Retry count exceeded (%u)",
	      UA_MAX_FAILURE_RETRIES));

    return TRUE;
}


static Int
aS1S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode 			rc;
    OS_Uint32 			len;
    EMSD_DeliveryControlArgument *	pEmsdDeliveryControlArg;
    UA_Operation *		pOperation = hUserData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS1S2a"));

    /* Delivery Control always uses the 2-way saps */
    pOperation->sapSel = MTSUA_Rsap_Mts_Performer_2Way;
    pOperation->hSapDesc = ua_globals.sap.hInvoker2Way;
    pOperation->operationValue = EMSD_Operation_DeliveryControl_1_1;

    /* Point to the delivery control argument */
    pEmsdDeliveryControlArg = pOperation->hFirstOperationData;

    /* Get the length of the contents data */
    len = BUF_getBufferLength(pOperation->hFullPduBuf);

    /*
     * Increment the length by the maximum length of a formatted
     * Deliver Argument (which is adequate because it's longer than
     * the maximum length of a Delivery Control Argument.
     */
    len += MTSUA_MAX_FORMATTED_DELIVER_ARG_LEN;

    /* Format this Delivery Control Argument and send it */
    if ((rc = formatAndInvokeDeliveryControl(pOperation,
			      		     pEmsdDeliveryControlArg,
			      		     pOperation->hFullPduBuf)) 
	!= Success)
    {
        /* Generate a FAILURE.ind. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		 "formatAndInvokeDeliveryControl() failed; rc=0x%04x", rc));
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
	return 0;
    }

    /* We've sent the complete PDU */
    pOperation->bComplete = TRUE;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS1S2a() OK", rc));
    return 0;
}


static Int
aS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    EMSD_DeliveryControlResult *	pDeliveryControlResult = 
			  	pOperation->hOperationData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S1a: RESULT.ind (Success)"));

    /* Let the UA Shell know that delivery control operation was successful. */
    (* ua_globals.pfDeliveryControlSuccess)(pOperation, 
					    pDeliveryControlResult,
				   	    pOperation->pHackData);

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S1a() OK"));
    return 0;
}



static Int
aS2S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *		pOperation = hUserData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S1b: Transient failure"));

    /* Let the UA Shell know that delivery control operation got error response*/
    (* ua_globals.pfDeliveryControlError)(pOperation, TRUE,
				   	  pOperation->pHackData);

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S1b() OK"));
    return 0;
}



static Int
aS2S1c(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S1c: Permament failure"));

    /* Let the UA Shell know that delivery control got error response */
    (* ua_globals.pfDeliveryControlError)(pOperation, FALSE,
				         pOperation->pHackData);

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S1c() OK"));
    return 0;
}



static Int
aS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "as2s1d(): Permanent failure"));

    /* Let the UA Shell know that delivery control operation failed */
    (* ua_globals.pfDeliveryControlFail)(pOperation, FALSE, 
				         pOperation->pHackData);

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S1d() OK"));
    return 0;
}

static Int
aS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode		rc;
    UA_Operation *	pOperation = hUserData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S2a(): Transient failure"));

    /* Issue the ESROS Invoke Request again */
    if ((rc = ua_issueDelCtrlInvoke(pOperation,
			     pOperation->hCurrentBuf)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "ua_issueDelCtrlInvoke() failed; rc=0x%04x", rc));
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    /* Let the UA Shell know that delivery control operation failed */
    (* ua_globals.pfDeliveryControlFail)(pOperation, TRUE, 
				         pOperation->pHackData);

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S2a() OK", rc));

    return 0;
}

static Int
aUnexpectedEvent(void * hMachine,
		 void * hUserData,
		 FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;

    sprintf(ua_globals.tmpbuf, "Unexpected Event: %d\n", eventId);
    EH_problem(ua_globals.tmpbuf);

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}



static ReturnCode
formatAndInvokeDeliveryControl(UA_Operation * pOperation,
		EMSD_DeliveryControlArgument * pEmsdDeliveryControlArg,
		void * hContents)
{
    ReturnCode 			rc;
    OS_Uint32			len;
    QU_Head *			pCompTree;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "FormatAndInvoke entered"));

    /*Get the ASN compile tree for a Delivery Control Argument. */
    pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliveryControlArg);

    /* Allocate a buffer to begin formatting the Delivery Control Argument */
    if ((rc = BUF_alloc(0, &pOperation->hCurrentBuf)) != Success)
    {
	return rc;
    }

    /* Arrange for the buffer to be freed */
    if ((rc = ua_operationAddUserFree(pOperation,
				      BUF_free,
				      pOperation->hCurrentBuf)) != Success)
    {
	BUF_free(pOperation->hCurrentBuf);
	return rc;
    }

    /* Format the Delivery Control Argument. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, 
	     "Formatting Delivery Control Argument"));
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 pOperation->hCurrentBuf,
			 pEmsdDeliveryControlArg,
			 &len)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "ASN_format() failed; rc=0x%04x", rc));
	return rc;
    }

    /* Issue the ESROS Invoke Request */
    if ((rc = ua_issueDelCtrlInvoke(pOperation,
			     pOperation->hCurrentBuf)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "ua_issueDelCtrlInvoke() failed; rc=0x%04x", rc));
	return rc;
    }

    /* All done. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, 
	     "formatAndInvokeDeliveryControl() OK"));
    return Success;
}
#endif
