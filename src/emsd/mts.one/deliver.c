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

/*
 * 
 * History:
 *
 */

static char *rcs = "$Id: deliver.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";

#include "estd.h"
#include "eh.h"
#include "buf.h"
#include "tmr.h"
#include "mm.h"
#include "fsm.h"
#include "fsmtrans.h"
#include "mtsua.h"
#include "mts.h"

/*
 * Deliver State Diagram
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
 * | State  | E: DELIVER.req        | State  |
 * |   1    | A: Issue INVOKE.req   |   2    |
 * |        |    for 1st segment    |        |
 * |        |                       |        |
 * |        |                       |        | [A]                  
 * |        |                       |        |>--------------------+
 * |        |                       |        | E: RESULT.ind       |
 * |        |                       |        | P: More segments    |
 * |        |                       |        | A: Issue INVOKE.req |
 * |        |                       |        |    for next segment |
 * |        |                       |        |<--------------------+
 * |        |                       |        |                      
 * |        |                       |        | [B]                     
 * |        |                       |        |>--------------------+
 * |        |                       |        | E: FAILURE.ind      |
 * |        |                       |        | P: Not max retries  |
 * |        |                       |        | A: Issue INVOKE.req |
 * |        |                       |        |    for same segment |
 * |        |                       |        |<--------------------+
 * |        |                       |        |
 * |        |                       |        |
 * |   I    |                   [A] | I      |
 * |   d    |<---------------------<| n R    |
 * |   l    | E: RESULT.ind         | v e W  |
 * |   e    | P: Last segment       | o s a  |
 * |        |                       | k p i  |
 * |        |                       | e o t  |
 * |        |                   [B] |   n    |
 * |        |<---------------------<|   s    |
 * |        | E: ERROR.ind          |   e    |
 * |        | P: ErrorType==        |        |
 * |        |      TransientError   |        |
 * |        | A: Respool message    |        |
 * |        |                       |        |
 * |        |                   [C] |        |
 * |        |<---------------------<|        |
 * |        | E: ERROR.ind          |        |
 * |        | P: ErrorType==        |        |
 * |        |      PermanentError   |        |
 * |        | A: Send non-delivery  |        |
 * |        |    report             |        |
 * |        |                       |        |
 * |        |                   [D] |        |
 * |        |<---------------------<|        |
 * |        | E: FAILURE.ind        |        |
 * |        | P: More segments      |        |
 * |        | A: Respool message    |        |
 * +--------+                       +--------+
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
FORWARD static FSM_State	s2InvokeResponseWait;
FORWARD static FSM_State	s3DeliveryVerifyResponseWait;


/* Transition Diagrams */
FORWARD static FSM_Trans	t1Idle[];
FORWARD static FSM_Trans	t2InvokeResponseWait[];
FORWARD static FSM_Trans	t3DeliveryVerifyResponseWait[];


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

FORWARD static OS_Boolean
pS2S3a(void * hMachine,
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
aS2S2b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aUnexpectedEvent(void * hMachine,
		 void * hUserData,
		 FSM_EventId eventId);

static ReturnCode
formatAndInvoke(MTS_Operation * pOperation,
		EMSD_DeliverArgument * pEmsdDeliverArg,
		void * hContents);

static void
deliveryRespool(MTS_Operation * pOperation);



/*
 * Actual Declarations
 */

/* States */
static FSM_State 	s1Idle =
{
    NULL,
    NULL,
    t1Idle,
    "MTS Deliver Idle"
};


static FSM_State	s2InvokeResponseWait =
{
    NULL,
    NULL,
    t2InvokeResponseWait,
    "MTS Deliver Invoke Response Wait"
};




/* Transition Diagrams */
static FSM_Trans	t1Idle[] =
{
    {
	MTS_FsmEvent_MessageDeliverRequest,
	NULL,
	aS1S2a,
	&s2InvokeResponseWait,
	"s1s2a (Message Deliver request)"
    },
    {
	FSM_EvtDefault,
	NULL,
	aUnexpectedEvent,
	&s1Idle,
	"Unexpected Event (default catch-all)"
    },
};


static FSM_Trans	t2InvokeResponseWait[] =
{
    {
	MTS_FsmEvent_ResultIndication,
	pS2S1a,
	aS2S1a,
	&s1Idle,
	"s2s1a (RESULT.ind, last segment)"
    },
    {
	MTS_FsmEvent_ResultIndication,
	NULL,
	aS2S2a,
	&s2InvokeResponseWait,
	"s2s2a (Result.ind, more segments)"
    },
    {
	MTS_FsmEvent_ErrorIndication,
	pS2S1b,
	aS2S1b,
	&s1Idle,
	"s2s1b (ERROR.ind, errorType=Transient)"
    },
    {
	MTS_FsmEvent_ErrorIndication,
	NULL,
	aS2S1c,
	&s1Idle,
	"s2s1c (ERROR.ind, errorType=Permanent)"
    },
    {
	MTS_FsmEvent_FailureIndication,
	pS2S1d,
	aS2S1d,
	&s1Idle,
	"s2s1d (FAILURE.ind, max retries reached)"
    },
    {
	MTS_FsmEvent_FailureIndication,
	NULL,
	aS2S2b,
	&s2InvokeResponseWait,
	"s2s2b (FAILURE.ind, max retries not reached)"
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
 * mts_deliverFsmInit()
 */
ReturnCode
mts_deliverFsmInit(void)
{
    if ((globals.hDeliverTransDiag =
	 FSM_TRANSDIAG_create("MTS Deliver Transitions", &s1Idle)) == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
		 "FSM for MTS Deliver failed to initialize"));
	return Fail;
    }
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "FSM for MTS Deliver initialized"));
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
 * Enabling Condition:	   Received last segment
 */
static OS_Boolean
pS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;

    if (! pOperation->bComplete)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
		  "Incomplete operation, wait for more segments {%ld}", pOperation->operId));
	return FALSE;
    }

    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "Predicate OK for S1->S2 (a) {%ld}", pOperation->operId));
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
    MTS_Operation *	    pOperation = hUserData;
    EMSD_ErrorRequest *	    pErrorRequest = pOperation->hOperationData;

    /* The only transient error is Resource Error */
    if (pErrorRequest == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
		  "Predicate failed, pErrorRequest is NULL  *** fix this ***"));
	return FALSE;
    }

    /* The only transient error is Resource Error */
    if (pErrorRequest->error != EMSD_Error_ResourceError)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
		  "Predicate failed, ErrorRequest = %ld", pErrorRequest->error));
	return FALSE;
    }

    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "Predicate OK for S1->S2 (a) {%ld}", pOperation->operId));
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
    MTS_Operation *		pOperation = hUserData;

    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "pS2S1d: Retries: %d of %d {%ld}",
	     pOperation->retryCount, MTS_MAX_FAILURE_RETRIES, pOperation->operId));
    /* See if we've reached the maximum number of retries */
    if (++pOperation->retryCount < MTS_MAX_FAILURE_RETRIES)
    {

	/* Nope. */
	return FALSE;
    }
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
	      "pS2S1d: Retry count exceeded: %d of %d {%ld}",
	      pOperation->retryCount, MTS_MAX_FAILURE_RETRIES, pOperation->operId));

    return TRUE;
}


static Int
aS1S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode 			rc;
    OS_Uint32 			len;
    OS_Uint32 			segmentLen;
    OS_Uint32 			numSegments;
    OptionalSegmentInfo *	pSegmentInfo;
    void * 			hSegmentContents;
    static OS_Uint32		sequenceId = 0;
    EMSD_DeliverArgument *	pEmsdDeliverArg;
    MTS_Operation *		pOperation = hUserData;
    SUB_DeviceProfile *         pDev = &pOperation->pControlData->pRecipQueue->deviceProfile;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS1S2a Deliver Request"));

    /*
     * Determine the operation id and sap to use.
     */
    if (pDev->protocolVersionMajor == 1 && pDev->protocolVersionMinor == 0)
    {
	/* Version 1.0. */
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "EMSD v1.0 3-Way"));
	pOperation->sapSel = MTSUA_Rsap_Ua_Performer_3Way;
	pOperation->operationValue = EMSD_Operation_Delivery_1_0_WRONG;
	pOperation->hSapDesc = globals.sap.hInvoker3Way;
    }
    else if (pDev->emsdDeliveryMode == DM_Efficient)
    {
	/* "Efficient" mode specified. */
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "EMSD v1.1 2-Way"));
	pOperation->sapSel = MTSUA_Rsap_Ua_Performer_2Way;
	pOperation->operationValue = EMSD_Operation_Delivery_1_1;
	pOperation->hSapDesc = globals.sap.hInvoker2Way;
    }
    else
    {
	/* Deliver mode specified, but not Efficient.  Assume "Complete". */
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "EMSD v1.1 3-Way"));
	pOperation->sapSel = MTSUA_Rsap_Ua_Performer_3Way;
	pOperation->operationValue = EMSD_Operation_Delivery_1_1;
	pOperation->hSapDesc = globals.sap.hInvoker3Way;
    }
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
	     "Delivery: SapSel=%d, opVal=%d, sapDesc=%d",
	      pOperation->sapSel, pOperation->operationValue,	
	      pOperation->hSapDesc));

    /* Point to the first deliver argument */
    pEmsdDeliverArg = pOperation->hFirstOperationData;

    /* Point to the segmentation structure */
    pSegmentInfo = (OptionalSegmentInfo *) &pEmsdDeliverArg->segmentInfo;

    /* Get the length of the contents data */
    len = BUF_getBufferLength(pOperation->hFullPduBuf);

    /*
     * Increment the length by the maximum length of a formatted
     * Submit or Deliver Argument.
     */
    len += MTSUA_MAX_FORMATTED_DELIVER_ARG_LEN;

    /*
     * Check the length against two values:
     *
     *   Is it short enough to fit in a single Connection-less message?
     *   Is it long enough to require a Connection-oriented stream?
     *
     * If neither is true, we'll split it up into multiple segments.
     */
    if (len >= MTSUA_MIN_IPM_LEN_CONNECTIONORIENTED)
    {
        /* ESROS does not yet do connectionless, issue a warning */
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
		  "Large message, requires C-O stream [%ld bytes]", len));
    }

    if ((len > MTSUA_MAX_IPM_LEN_CONNECTIONLESS) &&
	(len < MTSUA_MIN_IPM_LEN_CONNECTIONORIENTED))
    {
	/*
	 * We need to segment it.
	 */

        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "=== Segmenting message of length %d", len));

	/* Reset the parsing location to the beginning of the Contents */
	BUF_resetParse(pOperation->hFullPduBuf);

	/* Determine the sequence number for this sequence of segments */
	pOperation->sequenceId = ++sequenceId;

	/* Determine the length of the contents */
	len = BUF_getBufferLength(pOperation->hFullPduBuf);

	/* Determine the maximum contents length of each segment */
	pOperation->segmentLength = segmentLen =
	  MTSUA_MAX_IPM_LEN_CONNECTIONLESS - MTSUA_MAX_FORMATTED_DELIVER_ARG_LEN;

	/* Determine the number of segments */
	numSegments = ((len / segmentLen) + (len % segmentLen ? 1 : 0));

	if (numSegments > MTSUA_MAX_SEGMENTS)
	{
	    TM_TRACE((globals.hTM, MTS_TRACE_STATE,
		      "=== Too many segments in message [%d]", numSegments));
	    FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
	    return Fail;
	}

        TM_TRACE((globals.hTM, MTS_TRACE_STATE, 
		 "=== Segmenting IPM into %ld parts of size %ld",
		  numSegments, segmentLen));
	/* Last segment number if total number - 1 (zero relative) */
	pOperation->lastSegmentNumber = numSegments - 1;

	/* The next segment number will be number 1 since this one is 0 */
	pOperation->nextSegmentNumber = 1;

	/* Specify that we're segmenting */
	pSegmentInfo->exists = TRUE;

	/* Set the sequnce id */
	pSegmentInfo->data.segmentData.sequenceId = sequenceId;

	/* Indicate that this is the first segment */
	pSegmentInfo->data.choice = EMSD_SEGMENTINFO_CHOICE_FIRST;

	/* Fill in the total number of segments */
	pSegmentInfo->data.segmentData.numSegments = numSegments;

	/*
	 * Clone the next segment from the original contents buffer.
	 * The buffer pointers will be updated automagically so that
	 * the next segment action will get the next block of data.
	 */
	if ((rc = BUF_cloneBufferPortion(pOperation->hFullPduBuf,
					 segmentLen,
					 TRUE,
					 &hSegmentContents)) != Success)
	{
	    /* Generate a FAILURE.ind. */
            TM_TRACE((globals.hTM, MTS_TRACE_STATE, "BUF_cloneBufferPortion (%d)", rc));
	    FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
	    return 0;
	}

	/* Format this Deliver Argument and send it */
	if ((rc = formatAndInvoke(pOperation, pEmsdDeliverArg, hSegmentContents)) != Success)
	{
            TM_TRACE((globals.hTM, MTS_TRACE_STATE, 
		     "formatAndInvoke returned %d", rc));
	    /* Generate a FAILURE.ind. */
	    FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
	    return 0;
	}

	/* We're done with this segment buffer */
	BUF_free(hSegmentContents);
    }
    else
    {
	/*
	 * It's quite small or quite large.  We'll just send the whole
	 * contents, so it'll go as a single Connectionless message or
	 * a Connection-oriented stream will be created (at ESROS's'
	 * whim).
	 */

	/* Last segment number is 0 (zero relative) */
	pOperation->lastSegmentNumber = 0;

	/* The next segment number would be number 1 since this one is 0 */
	pOperation->nextSegmentNumber = 1;

	/* Format this Deliver Argument and send it */
	if ((rc = formatAndInvoke(pOperation, pEmsdDeliverArg, pOperation->hFullPduBuf)) != Success)
	{
	    /* Generate a FAILURE.ind. */
            TM_TRACE((globals.hTM, MTS_TRACE_STATE, 
		     "formatAndInvoke returned %d", rc));
	    FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
	    return 0;
	}

	/* We've sent the complete PDU */
	pOperation->bComplete = TRUE;
    }

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "formatAndInvoke returned %d", rc));
    return 0;
}


static Int
aS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode               rc                = Success;
    MTS_Operation *	     pOperation        = hUserData;
    MTSUA_ControlFileData *  pControlFileData  = NULL;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS2S1a: Free FSM"));

    if (pOperation == NULL)
      {
	TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS2S1a: NULL user data"));
	return 0;
      }
    pControlFileData = pOperation->pControlFileData;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS2S1a: Control file name: %s",
	      pOperation->pControlFileName ? pOperation->pControlFileName : 
	      "NULL"));

    msg_updateMessageStatus (pOperation->pControlData, MQ_SUCCEDED);

    mtsua_deleteControlAndDataFiles(pOperation->pControlFileName,
				    pOperation->pControlFileData);

    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    /* Create a timer to try delivering the message (very soon) */
    if ((rc = TMR_start(500, "NoName", NULL, mts_tryDelivery, NULL)) != Success)
    {
	sprintf(globals.tmpbuf, "Failed to schedule delivery; rc = 0x%x", rc);
	EH_problem(globals.tmpbuf);
    }

    return 0;
}



static Int
aS2S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *		pOperation = hUserData;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS2S1b: Respooling message"));

    /* Respool this message */
    deliveryRespool(pOperation);

    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}



static Int
aS2S1c(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    EMSD_ErrorRequest *  pErrorRequest = pOperation->hOperationData;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, 
	     "aS2S1c: Create Non-Delivery report"));

    /* The only transient error is Resource Error */
    if (pErrorRequest == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
		  "pErrorRequest is NULL, using local value  *** fix this ***"));
    }

    /* Create a non-delivery report */
    mts_deliveryNonDel(pOperation->pControlData, pErrorRequest ? pErrorRequest->error : 0);

    /* Delete the message from the delivery queue */
    TM_TRACE((globals.hTM, MTS_TRACE_MSG, "Deleting %lu's MID '%s', delivery failed",
	      pOperation->pControlData->pRecipQueue->recipient,
	      pOperation->pControlData->messageId));

    msg_deleteMessage (pOperation->pControlData);

    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}



static Int
aS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS2S1d: Respool message, Free FSM"));

    /* Respool this message */
    deliveryRespool(pOperation);

    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}



static Int
aS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode 			rc;
    OptionalSegmentInfo *	pSegmentInfo;
    void * 			hSegmentContents;
    EMSD_DeliverArgument *	pEmsdDeliverArg;
    MTS_Operation *		pOperation = hUserData;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS2S2a"));

    /* Get a pointer to the deliver arg */
    pEmsdDeliverArg = pOperation->hFirstOperationData;

    /* Point to the segmentation structure */
    pSegmentInfo = (OptionalSegmentInfo *) &pEmsdDeliverArg->segmentInfo;

    /* Specify that we're segmenting */
    pSegmentInfo->exists = TRUE;

    /* Set the sequnce id */
    pSegmentInfo->data.segmentData.sequenceId = pOperation->sequenceId;

    /* It's not the first segment.  Specify so. */
    pSegmentInfo->data.choice = EMSD_SEGMENTINFO_CHOICE_OTHER;

    /* Specify the current segment number */
    pSegmentInfo->data.segmentData.thisSegmentNumber =
	pOperation->nextSegmentNumber++;

    /*
     * Clone the next segment from the original contents buffer.  The
     * buffer pointers will be updated automagically so that the next
     * segment action will get the next block of data.
     */
    if ((rc = BUF_cloneBufferPortion(pOperation->hFullPduBuf, pOperation->segmentLength,
				     TRUE, &hSegmentContents)) != Success)
    {
	/* Generate a FAILURE.ind. */
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, 
		 "BUF_cloneBufferPortion returned %d", rc));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }

    /* Format this Deliver Argument and send it */
    if ((rc = formatAndInvoke(pOperation, pEmsdDeliverArg, hSegmentContents)) != Success)
    {
	/* Generate a FAILURE.ind. */
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "formatAndInvoke return %d", rc));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }


    /* clean up */
    BUF_free(hSegmentContents);

    /* indicate completed status after last segment */
    if (pOperation->nextSegmentNumber > pOperation->lastSegmentNumber)
      {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, 
		 "Last segment sent: next=%ld, last=%ld",
		  pOperation->nextSegmentNumber, pOperation->lastSegmentNumber));
	pOperation->bComplete = TRUE;
      }

    return 0;
}


static Int
aS2S2b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode			rc;
    MTS_Operation *		pOperation = hUserData;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS2S2b"));

    /* Issue the LS_ROS Invoke Request again */
    if ((rc = mts_issueInvoke(pOperation, pOperation->hCurrentBuf)) != Success)
    {
	/* Generate a FAILURE.ind. */
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }

    return 0;
}



static Int
aUnexpectedEvent(void * hMachine,
		 void * hUserData,
		 FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;

    sprintf(globals.tmpbuf, "Unexpected Event: %d\n", eventId);
    EH_problem(globals.tmpbuf);

    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}



static ReturnCode
formatAndInvoke(MTS_Operation * pOperation,
		EMSD_DeliverArgument * pEmsdDeliverArg,
		void * hContents)
{
    ReturnCode 			rc;
    OS_Uint32			len;
    QU_Head *			pCompTree;
    SUB_DeviceProfile *         pDev = &pOperation->pControlData->pRecipQueue->deviceProfile;

    /*
     * All of these recipients use the same protocol version.  Get the
     * ASN compile tree for a Deliver Argument.
     */
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "formatAndInvoke(0x%lx) {%ld}",
	      pOperation->invokeId, pOperation->operId));

    pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_DeliverArg, pDev->protocolVersionMajor,
				      pDev->protocolVersionMinor);
    if (pCompTree == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "formatAndInvoke failed; invalid ASN.1 tree for DeliveryArg"));
	return Fail;
    }

    /* Allocate a buffer to begin formatting the Deliver Argument */
    if ((rc = BUF_alloc(0, &pOperation->hCurrentBuf)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "formatAndInvoke failed; alloc error"));
	return rc;
    }

    /* Arrange for the buffer to be freed */
    rc = mts_operationAddUserFree(pOperation, BUF_free, pOperation->hCurrentBuf);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "formatAndInvoke failed; freelist error"));
	BUF_free(pOperation->hCurrentBuf);
	return rc;
    }

    /* Save the formatted IPM buffer handle */
    pEmsdDeliverArg->hContents = hContents;

    /* Format the Deliver Argument. */
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "Formatting Deliver Argument"));
    rc = ASN_format(ASN_EncodingRules_Basic, pCompTree, pOperation->hCurrentBuf,
		    pEmsdDeliverArg, &len);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "ASN_format returned %d", rc));
	return rc;
    }

    /* Issue the LS_ROS Invoke Request */
    if ((rc = mts_issueInvoke(pOperation, pOperation->hCurrentBuf)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE, "mts_issueInvoke(%d) returned 0x%lx {%ld}",
		  pOperation->invokeId, rc, pOperation->operId));
	return rc;
    }

    /* All done. */
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "formatAndInvoke successful, invokeId=0x%lx {%ld}",
	      pOperation->invokeId, pOperation->operId));
    return Success;
}

static void
deliveryRespool(MTS_Operation * pOperation)
{
    struct DQ_RecipQueue *      pRecip              = NULL;
    OS_Uint32                   delay               = 0;
    OS_Uint32                   retryIndex          = 0;
    time_t                      now                 = 0;

    now = time(NULL);
    pRecip = pOperation->pControlData->pRecipQueue;

    if (globals.numRetryTimes > pRecip->failureCount) {
      retryIndex = pRecip->failureCount;
    } else {
      retryIndex = globals.numRetryTimes - 1;
    }

    /* A delivery attempt failed.  Reschedule the delivery for a later time. */
    pOperation->pControlData->lastAttempt = now;
    pOperation->pControlData->failureCount++;
    pOperation->pControlData->status = MQ_FAILED;

    delay = globals.retryTimes[retryIndex];
    if (delay < (3 * 60) || delay > (4 * 60 * 60)) {
      delay = 10 * 60;
    }

    pRecip->lastAttempt = now;
    pRecip->nextAttempt = now + delay;
    pRecip->status = RQ_READY;

    TM_TRACE((globals.hTM, MTS_TRACE_MSG, "Retry delivery to %lu in %lu seconds {%ld}",
	      pRecip->recipient, delay, pOperation->operId));
}
