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

/*
 * Submission State Diagram
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
 * | State  | E: SUBMIT.req         | State  |
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
 * |        | A: Notify user of     |        |
 * |        |      submit failure   |        |
 * |        |                       |        |
 * |        |                   [D] |        |
 * |        |<---------------------<|        |
 * |        | E: FAILURE.ind        |        |
 * |        | P: More segments      |        |
 * |        | A: Respool message or |        |
 * |        |      notify user of   |        |
 * +--------+      submit failure   +--------+
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
FORWARD static FSM_State	s2InvokeResponseWait;


/* Transition Diagrams */
FORWARD static FSM_Trans	t1Idle[2];
FORWARD static FSM_Trans	t2InvokeResponseWait[7];


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
aS2S2b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aUnexpectedEvent(void * hMachine,
		 void * hUserData,
		 FSM_EventId eventId);

static ReturnCode
formatAndInvoke(UA_Operation * pOperation,
		EMSD_SubmitArgument * pEmsdSubmitArg,
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
    "UA Submit Idle"
};


static FSM_State	s2InvokeResponseWait =
{
    NULL,
    NULL,
    t2InvokeResponseWait,
    "UA Submit Invoke Response Wait"
};




/* Transition Diagrams */
static FSM_Trans	t1Idle[] =
{
    {
	UA_FsmEvent_MessageSubmitRequest,
	NULL,
	aS1S2a,
	&s2InvokeResponseWait,
	"s1s2a (Message Submit request)"
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
	UA_FsmEvent_ResultIndication,
	pS2S1a,
	aS2S1a,
	&s1Idle,
	"s2s1a (RESULT.ind, last segment)"
    },
    {
	UA_FsmEvent_ResultIndication,
	NULL,
	aS2S2a,
	&s2InvokeResponseWait,
	"s2s2a (Result.ind, more segments)"
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
 * ua_submitFsmInit()
 */
ReturnCode
ua_submitFsmInit(void)
{
    if ((ua_globals.hSubmitTransDiag =
	 FSM_TRANSDIAG_create("UA Submit Transitions", &s1Idle)) == NULL)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "FSM for UA Submit failed to initialize"));

	return Fail;
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
	      "FSM for UA Submit initialized"));

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
    UA_Operation *	pOperation = hUserData;

    if (! pOperation->bComplete)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "Predicate FAIL for S2->S1 (a): "
		  "more segments expected"));

	return FALSE;
    }

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
    OS_Uint32 			numSegments;
    OptionalSegmentInfo *	pSegmentInfo;
    void * 			hSegmentContents;
    static OS_Uint32		sequenceId = 0;
    EMSD_SubmitArgument *	pEmsdSubmitArg;
    UA_Operation *		pOperation = hUserData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS1S2a"));

    /* Submit always uses the 3-way saps */
    pOperation->sapSel = MTSUA_Rsap_Mts_Performer_3Way;
    pOperation->hSapDesc = ua_globals.sap.hInvoker3Way;
    pOperation->operationValue = EMSD_Operation_Submission_1_1;

    /* Point to the first submit argument */
    pEmsdSubmitArg = pOperation->hFirstOperationData;

    /* Point to the segmentation structure */
    pSegmentInfo = (OptionalSegmentInfo *) &pEmsdSubmitArg->segmentInfo;

    /* Get the length of the contents data */
    len = BUF_getBufferLength(pOperation->hFullPduBuf);

    /*
     * Increment the length by the maximum length of a formatted
     * Deliver Argument (which is adequate because it's longer than
     * the maximum length of a Submit Argument.
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
    if ((len > MTSUA_MAX_IPM_LEN_CONNECTIONLESS)
#if 0
	&& (len < MTSUA_MIN_IPM_LEN_CONNECTIONORIENTED)
	/*
	 * djl: 12 Nov 96
	 *
	 *   Connection Oriented mode is not yet implemented in ESROS.  When
	 *   it is implemented, we also need to modify the function
	 *   ua_issueInvoke() in file uasubr.c, because right now, it
	 *   allocates a DU buffer of the size of the INVOKE.req data, which
	 *   will, of course, be too big for those buffers.
	 */
#endif
	)
    {
	/*
	 * We need to segment it.
	 */

        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "Segmenting message of length %ld", len));

	/* Reset the parsing location to the beginning of the Contents */
	BUF_resetParse(pOperation->hFullPduBuf);

	/* Determine the sequence number for this sequence of segments */
	pOperation->sequenceId = ++sequenceId;

	/* Determine the length of the contents */
	len = BUF_getBufferLength(pOperation->hFullPduBuf);

	/* Determine the maximum contents length of each segment */
	pOperation->segmentLength = (MTSUA_MAX_IPM_LEN_CONNECTIONLESS -
				     MTSUA_MAX_FORMATTED_DELIVER_ARG_LEN);

	/* Determine the number of segments */
	numSegments = ((len / pOperation->segmentLength) +
		       ((len % pOperation->segmentLength) != 0));

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
					 pOperation->segmentLength,
					 TRUE,
					 &hSegmentContents)) != Success)
	{
	    /* Generate a FAILURE.ind. */
            TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		      "BUF_cloneBufferPortion() failed, rc=0x%04x", rc));
	    FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
	    return 0;
	}

	/* Format this Submit Argument and send it */
	if ((rc = formatAndInvoke(pOperation,
				  pEmsdSubmitArg,
				  hSegmentContents)) != Success)
	{
	    /* Generate a FAILURE.ind. */
            TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		      "formatAndInvoke() failed, rc=0x%04x", rc));
	    FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
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

	/* Format this Submit Argument and send it */
	if ((rc = formatAndInvoke(pOperation,
				  pEmsdSubmitArg,
				  pOperation->hFullPduBuf)) != Success)
	{
	    /* Generate a FAILURE.ind. */
            TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		      "formatAndInvoke() failed; rc=0x%04x", rc));
	    FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
	    return 0;
	}

	/* We've sent the complete PDU */
	pOperation->bComplete = TRUE;
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS1S2a() OK", rc));
    return 0;
}


static Int
aS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    EMSD_SubmitResult *	pSubmitResult = pOperation->hOperationData;
    VerifyData *	pVerifyData;
    VerifyElem *	pVerifyElem;
    NVM_Transaction	hTrans;
    NVM_Memory		hElem;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S1a: RESULT.ind (Success)"));

    /* Generate a message id string from the EMSD Local-format message id */
    ua_messageIdEmsdLocalToRfc822(&pSubmitResult->localMessageId,
				 ua_globals.tmpbuf);

    /* Point to the Verify Data queue data */
    pVerifyData = NVM_getMemPointer(ua_globals.hVerify, NVM_FIRST);

    /* Get the first queue element on the inactive queue */
    NVM_quNext(ua_globals.hVerify, pVerifyData->hInactive, &hTrans, &hElem);

    /* Was there anything there? */
    if (pVerifyData->hInactive == hElem)
    {
	/* No.  Remove the first element from the active queue */
	NVM_quNext(ua_globals.hVerify, pVerifyData->hActive, &hTrans, &hElem);
	NVM_quRemove(hTrans, hElem);
    }

    /* Point to the verify element */
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL, "Point to the verify element"));
    pVerifyElem = NVM_getMemPointer(hTrans, hElem);

    /* Copy the message id into the element buffer */
    strncpy(pVerifyElem->messageId,
	    ua_globals.tmpbuf, EMSDUB_MAX_MESSAGE_ID_LEN);
    ua_globals.tmpbuf[EMSDUB_MAX_MESSAGE_ID_LEN] = '\0';

    /* Insert this element onto the active queue */
    NVM_quInsert(hTrans, hElem, ua_globals.hVerify, pVerifyData->hActive);

    /* Make sure non-volatile memory has been synchronized */
    NVM_sync();

    /* Let the UA Shell know that we've successfully submitted the message. */
    (* ua_globals.pfSubmitSuccess)(pOperation, ua_globals.tmpbuf, 
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

    /* Let the UA Shell know that we couldn't submit the message */
    (* ua_globals.pfSubmitFail)(pOperation, TRUE, pOperation->pHackData);

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

    /* Let the UA Shell know that we couldn't submit the message */
    (* ua_globals.pfSubmitFail)(pOperation, FALSE, pOperation->pHackData);

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

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "as2s1d(): Transient failure"));

    /* Let the UA Shell know that we couldn't submit the message */
    (* ua_globals.pfSubmitFail)(pOperation, TRUE, pOperation->pHackData);

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
    ReturnCode 			rc;
    OptionalSegmentInfo *	pSegmentInfo;
    void * 			hSegmentContents;
    EMSD_SubmitArgument *	pEmsdSubmitArg;
    UA_Operation *		pOperation = hUserData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S2a"));

    /* Get a pointer to the submit arg */
    pEmsdSubmitArg = pOperation->hFirstOperationData;

    /* Point to the segmentation structure */
    pSegmentInfo = (OptionalSegmentInfo *) &pEmsdSubmitArg->segmentInfo;

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
    if ((rc = BUF_cloneBufferPortion(pOperation->hFullPduBuf,
				     pOperation->segmentLength,
				     TRUE,
				     &hSegmentContents)) != Success)
    {
	/* Generate a FAILURE.ind. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "BUF_cloneBufferPortion() failed; rc=0x%04x", rc));
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    /* Format this Submit Argument and send it */
    if ((rc = formatAndInvoke(pOperation,
			      pEmsdSubmitArg,
			      hSegmentContents)) != Success)
    {
	/* Generate a FAILURE.ind. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "formatAndInvoke() failed; rc=0x%04x", rc));
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    /* See if we've sent the complete PDU */
    if (pOperation->nextSegmentNumber > pOperation->lastSegmentNumber)
    {
	/* We've sent the complete PDU */
	pOperation->bComplete = TRUE;
    }

    /* clean up */
    BUF_free(hSegmentContents);

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S2a() OK", rc));
    return 0;
}


static Int
aS2S2b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode			rc;
    UA_Operation *		pOperation = hUserData;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S2b"));

    /* Issue the LS_ROS Invoke Request again */
    if ((rc = ua_issueInvoke(pOperation,
			     pOperation->hCurrentBuf)) != Success)
    {
	/* Generate a FAILURE.ind. */
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS2S2b() OK", rc));
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
formatAndInvoke(UA_Operation * pOperation,
		EMSD_SubmitArgument * pEmsdSubmitArg,
		void * hContents)
{
    ReturnCode 			rc;
    OS_Uint32			len;
    QU_Head *			pCompTree;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "FormatAndInvoke entered"));

    /*Get the ASN compile tree for a Submit Argument. */
    pCompTree = UA_getProtocolTree(MTSUA_Protocol_SubmitArg);

    /* Allocate a buffer to begin formatting the Submit Argument */
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

    /* Save the formatted IPM buffer handle */
    pEmsdSubmitArg->hContents = hContents;

    /* Format the Submit Argument. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "Formatting Submit Argument"));
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 pOperation->hCurrentBuf,
			 pEmsdSubmitArg,
			 &len)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "ASN_format() failed; rc=0x%04x", rc));
	return rc;
    }

    /* Issue the LS_ROS Invoke Request */
    if ((rc = ua_issueInvoke(pOperation,
			     pOperation->hCurrentBuf)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "ua_issueInvoke() failed; rc=0x%04x", rc));
	return rc;
    }

    /* All done. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "formatAndInvoke() OK"));
    return Success;
}
