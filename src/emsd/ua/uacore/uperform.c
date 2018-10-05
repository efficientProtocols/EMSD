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
#include "emsdmail.h"
#include "emsd822.h"
#include "esro_cb.h"
#include "emsd.h"
#include "mtsua.h"
#include "uacore.h"

/*
 * UA Performer State Diagram
 *
 * E: = Event
 * P: = Predicate
 * A: = Action
 *
 * [x] = Identifier allowing multiple possible transitions between two states.
 *
 *
 *
 * +--------+                       +--------+
 * |        |                       |        |
 * |        | [A]                   |        |
 * |        |>--------------------->|        |
 * | State  | E: INVOKE.ind         | State  | [A]
 * |   1    | P: Type==Deliver,     |   2    |>--------------------------+
 * |        | A: Issue RESULT.req,  |        |                           |
 * |        |    Start seg. timer   |        |                           |
 * |        |                       |        |                           |
 * |   I    |                       |D       |<--------------------------+
 * |   d    |                   [A] |e R     | E: INVOKE.ind
 * |   l    |<---------------------<|l e R   | A: Enqueue pending buffer
 * |   e    | E: RESULT.con         |i s e W |                          
 * |        | P: Last segment       |v u s a |                          
 * |        | A: Deliver message    |e l p i |                          
 * |        |    to user            |r t o t |                          
 * |        |                       |    n   |
 * |        |                   [B] |    s   |
 * |        |<---------------------<|    e   |
 * |        | E: RESULT.con         |        |
 * |        | P: More segments      |        |
 * |        | A: Concatenate segment|        |
 * |        |    Issue INVOKE.ind   |        |
 * |        |     with 1st pending  |        |
 * |        |     buffer            |        |
 * |        |                       |        |
 * |        |                   [C] |        |
 * |        |<---------------------<|        |
 * |        | E: Segmentation Timer |        |
 * |        |    expired            |        |
 * |        |                       |        |
 * |        |                   [D] |        |
 * |        |<---------------------<|        |
 * |        | E: FAILURE.ind        |        |
 * |        | P: (SAP == 2Way or    |        |
 * |        |     DeliveryVerify    |        |
 * |        |      not supported) or|        |
 * |        |    More segments      |        |
 * |        |      pending          |        |
 * |        |                       |        |
 * |        |                   [E] |        |
 * |        |<---------------------<|        |                                 
 * |        | E: FAILURE.ind        |        |                                 
 * |        | P: (SAP == 2Way or    |        |                                 
 * |        |     DeliveryVerify    |        |                       +--------+
 * |        |     not supported) and|        |                       |        |
 * |        |    All segments rcvd  |        | [A]                   |        |
 * |        | A: Deliver message    |        |>--------------------->|        |
 * |        |    to user            |        | E: FAILURE.ind        | State  |
 * |        |                       +--------+ P: Sap == 3Way and    |   3    |
 * |        |                                     DeliveryVerify     |        |
 * |        |                                      supported and     |        |
 * |        |                                     All segments rcvd  |        |
 * |        |                                  A: Issue              |        |
 * |        |                                      DELIVERY.verify   |        |
 * |        |                                                        |        |
 * |        |                                                    [A] |        |
 * |        |<------------------------------------------------------<|D       |
 * |        | E: RESULT.ind                                          |e V     |
 * |        | A: Tag message per Verify response,                    |l e R   |
 * |        |    Deliver message to user                             |i r e W |
 * |        |                                                        |v i s a |
 * |        |                                                    [B] |e f p i |
 * |        |<------------------------------------------------------<|r y o t |
 * |        | E: FAILURE.ind                                         |y   n   |
 * |        | A: Tag message: possible non-delivery sent             |    s   |
 * |        |    Deliver message to user                             |    e   |
 * |        |                                                        |        |
 * |        |                                                        +--------+
 * |        |                                                                  
 * |        |                                                        
 * |        |                                                        
 * |        |                                                        
 * |        |                       +--------+
 * |        |                       |        |
 * |        | [A]                   |        |
 * |        |>--------------------->|        |
 * |        | E: INVOKE.ind         | State  |
 * |        | P: default predicate  |   4    |
 * |        | A: Issue ERROR.req    |        |
 * |        |                       |        |
 * |        |                       |        |
 * |        |                       |        |
 * |        |                       |E       |
 * |        |                   [A] |r  R    |
 * |        |<---------------------<|r  e  W |
 * |        | E: ERROR.con          |o  s  a |
 * |        |                       |r  p  i |
 * |        |                       |   o  t |
 * |        |                       |   n    |
 * |        |                   [B] |   s    |
 * |        |<---------------------<|   e    |
 * |        | E: FAILURE.ind        |        |  
 * |        |                       |        |
 * |        |                       |        |
 * |        |                       +--------+
 * |        |                                 
 * |        |                                 
 * |        |                                 
 * |        |                                 
 * |        |                       +--------+
 * |        |                       |        |
 * |        | [A]                   |        |
 * |        |>--------------------->|        |
 * |        | E: INVOKE.ind         | State  |
 * |        | P: type==             |   5    |
 * |        |     SubmissionVerify  |        |
 * |        | A: Process request    |        |
 * |        |    Issue RESULT.req   |        |
 * |        |       or ERROR.req    |        |
 * |        |                       |n       |
 * |        |                       |o R     |
 * |        | [B]                   |n e R   |
 * |        |>--------------------->|- s e W |
 * |        | E: INVOKE.ind         |D u s a |
 * |        | P: type==             |e l p i |
 * |        |     SubmissionControl |l t o t |
 * |        | A: Process request    |i   n   |
 * |        |    Issue RESULT.req   |v   s   |
 * |        |       or ERROR.req    |e   e   |
 * |        |                       |r       |
 * |        |                   [A] |        |
 * |        |<---------------------<|        |
 * |        | E. RESULT.con         |        |
 * |        |                       |        |
 * |        |                   [B] |        |
 * |        |<---------------------<|        |
 * |        | E: ERROR.con          |        |
 * |        |                       |        |
 * |        |                   [C] |        |
 * |        |<---------------------<|        |
 * |        | E: FAILURE.ind        |        |
 * |        |                       +--------+
 * |        |                      
 * +--------+                      
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
FORWARD static FSM_State	s2DeliverResultResponseWait;
FORWARD static FSM_State	s3DeliverVerifyResponseWait;
FORWARD static FSM_State	s4ErrorResponseWait;
FORWARD static FSM_State	s5NotDeliverResultResponseWait;


/* Transition Diagrams */
FORWARD static FSM_Trans	t1Idle[5];
FORWARD static FSM_Trans	t2DeliverResultResponseWait[8];
FORWARD static FSM_Trans	t3DeliverVerifyResponseWait[3];
FORWARD static FSM_Trans	t4ErrorResponseWait[3];
FORWARD static FSM_Trans	t5NotDeliverResultResponseWait[4];


/* Predicates */
FORWARD static OS_Boolean
pS1S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS1S5a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS1S5b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS2S1e(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);


/* Actions */
FORWARD static Int
aS1S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS1S4a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS1S5a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS1S5b(void * hMachine,
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
aS2S1e(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS2S3a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS3S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS3S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS4S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS4S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS5S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS5S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aS5S1c(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static Int
aUnexpectedEvent(void * hMachine,
		 void * hUserData,
		 FSM_EventId eventId);

FORWARD static ReturnCode
segmentationTimerExpired(void * hTimer,
			 void * hUserData1,
			 void * hUserData2);

static ReturnCode
formatAndInvoke(UA_Operation * pOperation,
		EMSD_DeliveryVerifyArgument * pDeliveryVerifyArg);



/*
 * Actual Declarations
 */

/* States */
static FSM_State 	s1Idle =
{
    NULL,
    NULL,
    t1Idle,
    "UA Performer Idle"
};


static FSM_State	s2DeliverResultResponseWait =
{
    NULL,
    NULL,
    t2DeliverResultResponseWait,
    "UA Performer Deliver Result Response Wait"
};


static FSM_State	s3DeliverVerifyResponseWait =
{
    NULL,
    NULL,
    t3DeliverVerifyResponseWait,
    "UA Performer Deliver Verify Response Wait"
};


static FSM_State	s4ErrorResponseWait =
{
    NULL,
    NULL,
    t4ErrorResponseWait,
    "UA Performer Error Response Wait"
};


static FSM_State	s5NotDeliverResultResponseWait =
{
    NULL,
    NULL,
    t5NotDeliverResultResponseWait,
    "UA Performer Not Deliver Result Response Wait"
};


/* Transition Diagrams */
static FSM_Trans	t1Idle[] =
{ 
    {
	UA_FsmEvent_InvokeIndication,
	pS1S2a,
	aS1S2a,
	&s2DeliverResultResponseWait,
	"s1s2a (INVOKE.ind, type=deliver)"
    },
    {
	UA_FsmEvent_InvokeIndication,
	pS1S5a,
	aS1S5a,
	&s5NotDeliverResultResponseWait,
	"s1s5a (INVOKE.ind, type=submission verify)"
    },
    {
	UA_FsmEvent_InvokeIndication,
	pS1S5b,
	aS1S5b,
	&s5NotDeliverResultResponseWait,
	"s1s5b (INVOKE.ind, type=submission control)"
    },
    {
	UA_FsmEvent_InvokeIndication,
	NULL,			/* default predicate */
	aS1S4a,
	&s4ErrorResponseWait,
	"s1s4a (INVOKE.ind, default)"
    },
    {
	FSM_EvtDefault,
	NULL,
	aUnexpectedEvent,
	&s1Idle,
	"Unexpected Event (default catch-all)"
    },
};


static FSM_Trans	t2DeliverResultResponseWait[] =
{ 
    {
	UA_FsmEvent_ResultConfirm,
	pS2S1a,
	aS2S1a,
	&s1Idle,
	"s2s1a (RESULT.con, last segment)"
    },
    {
	UA_FsmEvent_ResultConfirm,
	NULL,
	aS2S1b,
	&s1Idle,
	"s2s1b (RESULT.con, more segments)"
    },
    {
	UA_FsmEvent_FailureIndication,
	pS2S1e,
	aS2S1e,
	&s1Idle,
	"s2s1e (FAILURE.ind, "
	"(sap=2way or delivery verify unsupported) and no more segments)"
    },
    {
	UA_FsmEvent_FailureIndication,
	pS2S1d,
	aS2S1d,
	&s1Idle,
	"s2s1d (FAILURE.ind, "
	"sap=2way or delivery verify unsupported or more segments)"
    },
    {
	UA_FsmEvent_FailureIndication,
	NULL,
	aS2S3a,
	&s3DeliverVerifyResponseWait,
	"s2s3a (FAILURE.ind, last segment)"
    },
    {
	UA_FsmEvent_InvokeIndication,
	pS2S2a,
	aS2S2a,
	&s2DeliverResultResponseWait,
	"s2s2a (INVOKE.ind)"
    },
    {
	UA_FsmEvent_SegmentationTimerExpired,
	NULL,
	aS2S1c,
	&s1Idle,
	"s2s1c (Segmentation Timer expired)"
    },
    {
	FSM_EvtDefault,
	NULL,
	aUnexpectedEvent,
	&s1Idle,
	"Unexpected Event (default catch-all)"
    }
};


static FSM_Trans	t3DeliverVerifyResponseWait[] =
{
    {
	UA_FsmEvent_ResultIndication,
	NULL,
	aS3S1a,
	&s1Idle,
	"s3s1a (RESULT.ind)"
    },
    {
	UA_FsmEvent_FailureIndication,
	NULL,
	aS3S1b,
	&s1Idle,
	"s3s1b (FAILURE.ind)"
    },
    {
	FSM_EvtDefault,
	NULL,
	aUnexpectedEvent,
	&s1Idle,
	"Unexpected Event (default catch-all)"
    }
};


static FSM_Trans	t4ErrorResponseWait[] =
{
    {
	UA_FsmEvent_ErrorConfirm,
	NULL,
	aS4S1a,
	&s1Idle,
	"s4s1a (ERROR.con)"
    },
    {
	UA_FsmEvent_FailureIndication,
	NULL,
	aS4S1b,
	&s1Idle,
	"s4s1b (FAILURE.ind)"
    },
    {
	FSM_EvtDefault,
	NULL,
	aUnexpectedEvent,
	&s1Idle,
	"Unexpected Event (default catch-all)"
    }
};


static FSM_Trans	t5NotDeliverResultResponseWait[] =
{
    {
	UA_FsmEvent_ResultConfirm,
	NULL,
	aS5S1a,
	&s1Idle,
	"s5s1a (RESULT.con)"
    },
    {
	UA_FsmEvent_ErrorConfirm,
	NULL,
	aS5S1b,
	&s1Idle,
	"s5s1b (ERROR.con)"
    },
    {
	UA_FsmEvent_FailureIndication,
	NULL,
	aS5S1c,
	&s1Idle,
	"s5s1c (FAILURE.ind)"
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
 * ua_performerFsmInit()
 *
 * Initialize the Performer transition diagrams.
 */
ReturnCode
ua_performerFsmInit(void)
{
    if ((ua_globals.hPerformerTransDiag =
	 FSM_TRANSDIAG_create("UA Performer", &s1Idle)) == NULL)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "FSM for UA Performer failed to initialize"));
	return Fail;
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
	      "FSM for UA Performer initialized"));

    return Success;
}


/*
 * pS1S2a()
 *
 * Predicate function.
 *
 * Current State:          S1
 * Transition State:       S2
 *
 * Enabling Event:	   INVOKE.indication
 *
 * Enabling Condition:	   type=Deliver
 */
static OS_Boolean
pS1S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    int				i;
    UA_Operation *		pOperation = hUserData;
    UA_Pending *		pPending;
    EMSD_DeliverArgument *	pDeliverArg = pOperation->hOperationData;
    EMSD_Message *		pIpm;
    OptionalSegmentInfo *	pOptionalSegInfo;
    OS_Uint32			pduLength;
    OS_Uint32			thisSeg;
    void *			pCompTree;
    
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "S1->S2a predicate entered"));

    /* Get the first operation from the queue of pending operations */
    if (QU_EQUAL(pPending = QU_FIRST(&pOperation->pendingQHead),
		 &pOperation->pendingQHead))
    {
	/* Nothing to do (should never occur) */
	TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "No operation found on pending operations queue"));
	return FALSE;
    }

    /* Remove the pending operation from the queue */
    QU_REMOVE(pPending);

    /* Copy the operation data into the operation structure */
    pOperation->operationValue = pPending->operationValue;
    pOperation->encodingType   = pPending->encodingType;
    pOperation->invokeId       = pPending->invokeId;
    pOperation->hSapDesc       = pPending->hSapDesc;
    pOperation->sapSel         = pPending->sapSel;
    pOperation->hOperationData = pPending->hOperationData;
    
    /* We're all done with this pending operation structure */
    OS_free(pPending);

    /* See if it's a Deliver operation */
    if (pOperation->operationValue != EMSD_Operation_Delivery_1_1)
    {
	/* Nope. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "Deliver operation rejected, incorrect operValue=%d",
		  pOperation->operationValue));
	return FALSE;
    }
    
    /* Point to the deliver argument structure */
    pDeliverArg = (EMSD_DeliverArgument *) pOperation->hOperationData;
    

    /*
     * Determine if this operation is segmented.
     */

    /* Point to the segment info and exists flag */
    pOptionalSegInfo = (OptionalSegmentInfo *) &pDeliverArg->segmentInfo;
    
    /* Is this operation segmented? */
    if (pOptionalSegInfo->exists)
    {
	/* Is this the first segment? */
	if (pOptionalSegInfo->data.choice == EMSD_SEGMENTINFO_CHOICE_FIRST)
	{
	    /* Handle the special case of one segment, sent segmented */
	    if (pOptionalSegInfo->data.segmentData.numSegments == 1)
	    {
	        TM_TRACE((ua_globals.hTM, UA_TRACE_WARNING,
			  "Single segment sent segmented"));
		pOperation->bComplete = TRUE;
	    }
	    else
	    {
		/* Save number of the last segment in this operation */
		pOperation->lastSegmentNumber =
		    pOptionalSegInfo->data.segmentData.numSegments - 1;
		
		/* This implementation won't support more than 31 segments */
		if (pOperation->lastSegmentNumber > 31)
		{
		    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			      "Deliver operation has too many segments (%ld)",
			      pOperation->lastSegmentNumber));
		    fprintf(stdout, "Message to large. More than 31 segments "
				    "is not supported\n");
		    EH_violation("More than 31 segments not supported\n");
		    return FALSE;
		}

#if defined(OS_TYPE_MSDOS) && !defined(OS_VARIANT_WinCE)
		/* To be fixed! Right now it has a problem */
		if (pOperation->lastSegmentNumber > 15)
		{
		    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			      "Deliver operation has too many segments (%ld)",
			      pOperation->lastSegmentNumber));
		    fprintf(stdout, "Message to large. More than 15 segments "
				    "is not supported\n");
		    EH_violation("More than 15 segments not supported\n");
		    return FALSE;
		}
#endif

		/* Save the sequence id for this segment sequence */
		pOperation->sequenceId =
		    pOptionalSegInfo->data.segmentData.sequenceId;
		
		/* Save the first-segment operation structure */
		pOperation->hFirstOperationData = pDeliverArg;
		
		/* Copy buffer data (since it'll get freed when we return) */
		if ((pOperation->rc =
		     BUF_copy(pDeliverArg->hContents,
			      &pOperation->hSegmentBuf[0])) != Success)
		{
		    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			      "BUF_copy failed"));
		    return FALSE;
		}
		
		/* Arrange for this segment to be freed with the operation */
		if ((pOperation->rc =
		     ua_operationAddUserFree(pOperation,
					     BUF_free,
					     pOperation->
					     hSegmentBuf[0])) != Success)
		{
		    BUF_free(pOperation->hSegmentBuf[0]);
                    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			      "UA Op AddUserFree failed"));
		    return FALSE;
		}

		/*
		 * Initialize the segment mask indicating what segment
		 * numbers we expect.
		 *
		 * ASSUMPTION: host uses 2's complement arithmetic
		 */
		pOperation->expectedSegmentMask =
		    ((1 << (pOperation->lastSegmentNumber + 1)) - 1);

		/*
		 * Initialize the segment mask indicating that we
		 * received segment zero.
		 */
		pOperation->receivedSegmentMask = (1 << 0);

		/* Start the segmentation timer */
		if ((pOperation->rc =
		     TMR_start(TMR_SECONDS(UA_SEGMENTATION_TIMER),
			       pOperation,
			       NULL,
			       segmentationTimerExpired,
			       &pOperation->hSegmentationTimer)) != Success)
		{
		    BUF_free(pOperation->hSegmentBuf[0]);
		    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			      "Timer start failed, rc=%d", pOperation->rc));
		    return FALSE;
		}
	    }
	}
	else
	{
	    /*
	     * It's not a First Segment
	     */
	    
	    /* Make sure it's a legal segment number */
	    thisSeg = pOptionalSegInfo->data.segmentData.thisSegmentNumber;

	    if (thisSeg > 31 ||
		(1 << thisSeg) > pOperation->expectedSegmentMask)
	    {
		EH_violation("received segment number too large\n");
                TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			  "Segment greater than max expected, rejected"));
		return FALSE;
	    }

	    /* Have we received this segment number already? */
	    if ((pOperation->receivedSegmentMask & (1 << thisSeg)) != 0)
	    {
		EH_violation("duplicate segment received\n");
                TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			  "Duplicate segment rejected"));
		return FALSE;
	    }

	    /* Copy the buffer data (since it'll get freed when we return) */
	    if ((pOperation->rc =
		 BUF_copy(pDeliverArg->hContents,
			  &pOperation->hSegmentBuf[thisSeg])) != Success)
	    {
                TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			  "BUF_copy failed"));
		return FALSE;
	    }
	    
	    /* Arrange for this segment to be freed with the operation */
	    if ((pOperation->rc =
		 ua_operationAddUserFree(pOperation,
					 BUF_free,
					 pOperation->hSegmentBuf[thisSeg])) !=
		Success)
	    {
		BUF_free(pOperation->hSegmentBuf[thisSeg]);
                TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			  "UA Op AddFreeUser failed"));
		return FALSE;
	    }

	    /* Set the mask indicating that we received this segment */
	    pOperation->receivedSegmentMask |= (1 << thisSeg);

	    /* Have we received all of the segments? */
	    if (pOperation->expectedSegmentMask ==
		pOperation->receivedSegmentMask)
	    {
		/* Yup.  Append the segment buffers to the first one. */
		pOperation->hFullPduBuf = pOperation->hSegmentBuf[0];
		for (i=1; i<=pOperation->lastSegmentNumber; i++)
		{
		    if ((pOperation->rc =
			 BUF_appendBuffer(pOperation->hFullPduBuf,
					  pOperation->hSegmentBuf[i])) !=
			Success)
		    {
		        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
				  "BUF_appendBuf failed"));
			return FALSE;
		    }
		}
	    
		/*
		 * Copy our saved Deliver argument into this last one.
		 */
		
		/* Give 'em the original Deliver Argument */
		pDeliverArg = pOperation->hFirstOperationData;
		
		/* Give 'em the content buffer */
		pDeliverArg->hContents = pOperation->hFullPduBuf;
		
		/* Make sure we don't try to free this later */
		pOperation->hFullPduBuf = NULL;
		
		/* Reset the buffer pointers to the beginning */
		BUF_resetParse(pDeliverArg->hContents);
		
		/* Cancel the segmentation timer */
		TMR_stop(pOperation->hSegmentationTimer);
		pOperation->hSegmentationTimer = NULL;
		
		/* Indicate that we're ready to parse the contents */
		pOperation->bComplete = TRUE;
	    }
	}
    }
    else
    {
	/* Indicate that we're ready to parse the contents */
	pOperation->bComplete = TRUE;
    }
    
    /*
     * If the operation is completed (i.e., we've received all of the
     * segments), parse the contents and create an RFC-822 message
     * file.
     */
    if (pOperation->bComplete)
    {
	/* See what type of contents we've received */
	switch(pDeliverArg->contentType)
	{
	case EMSD_ContentType_Ipm95:
	    /* Allocate an EMSD Message structure */
	    if ((pOperation->rc = mtsua_allocMessage(&pIpm)) != Success)
	    {
	        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			  "AllocMessage failed"));
		return FALSE;
	    }
	    
	    /* Arrange for the IPM to be freed later. */
	    if ((pOperation->rc =
		 ua_operationAddUserFree(pOperation,
					 (void (*)(void *)) mtsua_freeMessage,
					 pIpm)) != Success)
	    {
		mtsua_freeMessage(pIpm);
		TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			  "UA Op AddUserFree failed"));
		return FALSE;
	    }
	    
	    /* Get the length of the Contents PDU */
	    pduLength = BUF_getBufferLength(pDeliverArg->hContents);
	    
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_Ipm);
	    
	    /* Parse the IPM PDU */
	    if ((pOperation->rc = ASN_parse(ASN_EncodingRules_Basic,
					    pCompTree,
					    pDeliverArg->hContents,
					    pIpm, &pduLength)) != Success)
	    {
	        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			  "Parse of IPM failed [%d]", pOperation->rc));
		return FALSE;
	    }
	    
	    /* Deliver the message to the UA Shell */
	    if ((pOperation->rc =
		 ua_messageToShell(pDeliverArg, pIpm, pOperation)) != Success)
	    {
		TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
			  "Could not deliver message to user"));
		return FALSE;
	    }
	    break;
	    
	case EMSD_ContentType_Probe:
	case EMSD_ContentType_VoiceMessage:
	case EMSD_ContentType_DeliveryReport:
	default:
	    pOperation->rc = UnsupportedOption;
	    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		      "Unsupported operation"));
	    return FALSE;
	}
    }
    
    /* This transition is acceptable. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "S1->S2a Transition OK"));
    return TRUE;
}


/*
 * pS1S5a()
 *
 * Predicate function.
 *
 * Current State:          S1
 * Transition State:       S5
 *
 * Enabling Event:	   INVOKE.indication
 *
 * Enabling Condition:	   type=SubmissionVerify
 */
static OS_Boolean
pS1S5a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "S1->S5a predicate entered"));

    /* See if it's a Submission Verify operation */
    if (pOperation->operationValue != EMSD_Operation_SubmissionVerify_1_1)
    {
	/* Nope. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "Invalid operation: %d vs %d",
		  pOperation->operationValue,
		  EMSD_Operation_SubmissionVerify_1_1));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "S1->S5a transition OK"));
    return TRUE;
}


/*
 * pS1S5b()
 *
 * Predicate function.
 *
 * Current State:          S1
 * Transition State:       S5
 *
 * Enabling Event:	   INVOKE.indication
 *
 * Enabling Condition:	   type=SubmissionControl
 */
static OS_Boolean
pS1S5b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "S1->S5b predicate entered"));

    /* See if it's a Submission Control operation */
    if (pOperation->operationValue != EMSD_Operation_SubmissionControl_1_1)
    {
	/* Nope. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "Invalid operation: %d vs %d",
		  pOperation->operationValue,
		  EMSD_Operation_SubmissionControl_1_1));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "S1->S5b transition OK"));
    return TRUE;
}


/*
 * pS2S1a()
 *
 * Predicate function.
 *
 * Current State:          S2
 * Transition State:       S1
 *
 * Enabling Event:	   RESULT.confirm
 *
 * Enabling Condition:	   last segment received
 */
static OS_Boolean
pS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "S2->S1a predicate entered"));

    /* See if we've received all segments */
    if (! pOperation->bComplete)
    {
	/* Nope. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "Not all segments received"));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "S2->S1a transition OK"));
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
 * Enabling Event:         FAILURE.indication
 *
 * Enabling Condition:     more segments to be received or
 *                         sap == 2Way or
 *                         Delivery Verify not supported
 */
static OS_Boolean
pS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "S2->S1d transition predicated entered"));

    /*
     * See if there are no more segments are expected, and sap!=2Way,
     * and Delivery Verify is supported.
     */
    if (pOperation->bComplete &&
	pOperation->hSapDesc != ua_globals.sap.hPerformer2Way &&
	ua_globals.bDeliveryVerifySupported)
    {
	/* Yup. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "Transition rejected"));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "S2->S1d transition OK"));
    return TRUE;
}


/*
 * pS2S1e()
 *
 * Predicate function.
 *
 * Current State:          S2
 * Transition State:       S1
 *
 * Enabling Event:         FAILURE.indication
 *
 * Enabling Condition:     no more segments to be received and
 *                         (sap == 2Way or
 *                          Delivery Verify not supported)
 */
static OS_Boolean
pS2S1e(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "S2->S1e transition predicated entered"));

    /*
     * See if there are more segments are expected, or
     * (sap!=2Way and Delivery Verify is supported).
     */
    if (pOperation->bComplete ||
	(pOperation->hSapDesc != ua_globals.sap.hPerformer2Way &&
	 ua_globals.bDeliveryVerifySupported))
    {
	/* Yup. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "Transition rejected"));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "S2->S1e transition OK"));
    return TRUE;
}


/*
 * pS2S2a()
 *
 * Predicate function.
 *
 * Current State:          S2
 * Transition State:       S2
 *
 * Enabling Event:         INVOKE.indication
 *
 * Enabling Condition:     Delivery
 */
static OS_Boolean
pS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
/*    UA_Operation *	pOperation = hUserData; */
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
	      "S2->S2a transition predicated entered"));

    /* This transition is acceptable. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE, "S2->S2a transition OK ****"));
    return TRUE;
}


/*
 * Actions
 */

static Int
aS1S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ESRO_RetVal             retval        = SUCCESS;
    UA_Operation *          pOperation    = hUserData;
    EMSD_DeliverArgument *   pDeliverArg   = pOperation->hOperationData;        
    char *		    pPassword;


    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "aS1S2a: DELIVER.result"));

    /*
     * Validate the password
     */
    /* Was a password specified?  If not, ignore security parameter. */
    if (ua_globals.pPassword && strcmp(ua_globals.pPassword, "None") != 0)
    {
	/* Make sure we were given a security parameter */
	if (! pDeliverArg->security.data.credentials.simple.password.exists)
	{
	    /* Mandatory security parameter missing. */
	    pOperation->rc = UA_RC_SecurityViolation;

	    TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		      "Deliver operation rejected: "
		      "Security Violation (missing password, wanted '%s')",
		      ua_globals.pPassword));

/***	    return FALSE;    *** fnh ***/
	}


	if (pDeliverArg->security.data.credentials.simple.password.data !=
	    NULL)
	{
	    /* Point to the password in the deliver operation */
	    pPassword =
		STR_stringStart(pDeliverArg->security.data.
				credentials.simple.password.data);

	    /* Does it match our local password? */
	    if (strcmp(ua_globals.pPassword, pPassword) != 0)
	    {
		/* Nope. */
		pOperation->rc = UA_RC_SecurityViolation;
		
		TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
			  "Deliver operation rejected: "
			  "Security Violation (Incorrect password: got '%s', wanted '%s')",
			  pPassword, ua_globals.pPassword));

/***	        return FALSE; *** fnh ***/
	    }
	}
	else
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		      "Deliver operation warning: "
		      "Security Violation (Null password) "
		      "[Delivering anyway]"));
	}
    }

    /* Send back a RESULT.request */
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "Issuing RESULT.req"));
    if ((retval = ESRO_CB_resultReq(pOperation->invokeId, NULL,
				    ESRO_EncodingBER, (DU_View) NULL)) != SUCCESS)
    {
	/* Generate a FAILURE.ind. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Generating FAILURE.ind event"));
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    /* Save the invoke id of the operation being acknowledged. */
    pOperation->unacknowledgedInvokeId = pOperation->invokeId;

    return 0;
}


static Int
aS1S4a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    EMSD_Error	        errorVal;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "aS1S4a: Error Request"));

    /* What type of error occured? */
    switch(pOperation->rc)
    {
    case ResourceError:
	errorVal = EMSD_Error_ResourceError;
	break;

    case UA_RC_SecurityViolation:
	errorVal = EMSD_Error_SecurityError;
	break;

    default:
	errorVal = EMSD_Error_ProtocolViolation;
	break;
    }

    /* Generate an error request */
    ua_issueErrorRequest(pOperation->invokeId, errorVal);

    return 0;
}


static Int
aS1S5a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    char *				p;
    void *				hBuf;
    QU_Head *				pCompTree;
    OS_Uint32 				len;
    ReturnCode				rc;
    ESRO_RetVal				retval = SUCCESS;
    UA_Operation *			pOperation = hUserData;
    EMSD_SubmissionVerifyArgument *	pSubmitVerify;
    EMSD_SubmissionVerifyResult		submitVerifyRes;
    VerifyData * 			pVerifyData;
    VerifyElem * 			pVerifyElem;
    NVM_Transaction 			hTrans;
    NVM_Memory 				hElem;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "aS1S5a: Submission Verify indication"));

    /* Point to the deliver argument structure */
    pSubmitVerify =
	(EMSD_SubmissionVerifyArgument *) pOperation->hOperationData;
    
    /* If the message id is of local format (it should be)... */
    if (pSubmitVerify->messageId.choice == EMSD_MESSAGEID_CHOICE_LOCAL)
    {
	/*
	 * ... generate a message id string from the EMSD Local-format
	 * message id
	 */
	ua_messageIdEmsdLocalToRfc822(&pSubmitVerify->messageId.un.local,
				     p = ua_globals.tmpbuf);
    }
    else
    {
	/* Point to the RFC-822 message id */
	p = STR_stringStart(pSubmitVerify->messageId.un.rfc822);
    }

    /* Search for this message id from recently submitted messages */
    pVerifyData = NVM_getMemPointer(ua_globals.hVerify, NVM_FIRST);

    for (NVM_quNext(ua_globals.hVerify, pVerifyData->hActive, &hTrans, &hElem);
	 hElem != pVerifyData->hActive;
	 NVM_quNext(hTrans, hElem, &hTrans, &hElem))
    {
	/* Point to the verify element */
	pVerifyElem = NVM_getMemPointer(hTrans, hElem);

	/* Is this the one we're looking for? */
	if (OS_strcasecmp(p, pVerifyElem->messageId) == 0)
	{
	    /* Yup. */
	    break;
	}
    }

    /* Did we find it? */
    if (hElem != pVerifyData->hActive)
    {
	/* Yes.  Generate a RESULT.req with submission status of "Send" */
	submitVerifyRes.submissionStatus =
	    EMSD_SubmissionVerifyStatus_SendMessage;
    }
    else
    {
	/* No.  Generate an RESULT.req with submission status of "Drop" */
	submitVerifyRes.submissionStatus =
	    EMSD_SubmissionVerifyStatus_DropMessage;
    }

    /*Get the ASN compile tree for a Submit Argument. */
    pCompTree = UA_getProtocolTree(MTSUA_Protocol_SubmissionVerifyRes);

    /* Allocate a buffer to begin formatting the Submit Argument */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR, "BUF_alloc() failed"));
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    /* Arrange for the buffer to be freed */
    if ((rc = ua_operationAddUserFree(pOperation, BUF_free, hBuf)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree() failed; rc=0x%04x", rc));
	BUF_free(pOperation->hCurrentBuf);
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    /* Format the Delivery Verify Argument. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
	      "Formatting Submission Verify Result"));
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 hBuf,
			 &submitVerifyRes,
			 &len)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ASN_format() failed; rc=0x%04x", rc));
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    /* Send back a RESULT.request */
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "invoking RESULT.req"));
    if ((retval = ESRO_CB_resultReq(pOperation->invokeId, NULL,
				    ESRO_EncodingBER, hBuf)) != SUCCESS)
    {
	/* Couldn't.  Generate a FAILURE.ind. */
        TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "RESULT.req failed; generating FAILURE.ind event"));
	FSM_generateEvent(hMachine, UA_FsmEvent_FailureIndication);
    }

    return 0;
}


static Int
aS1S5b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "aS1S5b: Submission Control not implmented"));
    printf("\n\naS1S5b (Submission Control) not yet implemented\n\n");

    return 0;
}


static Int
aS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *		pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "aS2S1a: Deliver result, no more segments"));
    
    /* The MTS knows that we have the message.  Let our user know. */
    (*ua_globals.pfModifyDeliverStatus)(pOperation->messageId,
					UASH_DeliverStatus_Acknowledged);

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}


static Int
aS2S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *		pOperation = hUserData;
    UA_Pending *		pPending;

    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "aS2S1b: Deliver result, more segments"));
    
    /* If there's anything on the Pending queue... */
    if (! QU_EQUAL(pPending = QU_FIRST(&pOperation->pendingQHead),
		   &pOperation->pendingQHead))
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "aS2S1b Generating INVOKE.ind to FSM for queued data."));

	/* ... generate a new event to the finite state machine. */
	FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_InvokeIndication);
    }

    return 0;
}


static Int
aS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *		pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "aS2S1d: FAILURE.ind"));
    
    /*
     * If there's no segmentation timer running (which will expire and
     * free the operation), free the finite state machine and
     * operation structure now.
     */
    if (pOperation->hSegmentationTimer == NULL)
    {
	ua_freeOperation(pOperation);
    }

    return 0;
}


static Int
aS2S1e(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "aS2S1e: (FAILURE.ind; sap=2Way or "
	      "DeliveryVerify not supported) and all segments received"));
    
    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}


static Int
aS2S1c(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "aS2S1c: Timer expired"));
    
    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}


static Int
aS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "aS2S2a: INVOKE.ind (queued)"));

    return 0;
}


static Int
aS2S3a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode				rc;
    UA_Operation *			pOperation = hUserData;
    EMSD_DeliveryVerifyArgument *	pDeliveryVerify;

    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "aS2S3a: Issue DELIVER.verify"));

    /* Allocate an EMSD Delivery Verify structure */
    if ((pOperation->rc =
	 mtsua_allocDeliveryVerifyArg(&pDeliveryVerify)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "AllocDeliveryVerify failed"));
	return 0;
    }
	    
    /* Arrange for the Delivery Verify Argument to be freed later. */
    if ((pOperation->rc =
	 ua_operationAddUserFree(pOperation,
				 ((void (*)(void *))
				  mtsua_freeDeliveryVerifyArg),
				 pDeliveryVerify)) != Success)
    {
	mtsua_freeDeliveryVerifyArg(pDeliveryVerify);
	TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "UA Op AddUserFree failed"));
	return 0;
    }

    /* Generate the Message Id for the Delivery Verify Argument */
    if ((rc =
	 ua_messageIdRfc822ToEmsdLocal(pOperation->messageId,
				      &pDeliveryVerify->messageId)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_PREDICATE,
		  "Conversion of RFC-822 message id failed, reason 0x%x", rc));
	return 0;
    }

    if ((rc = formatAndInvoke(pOperation, pDeliveryVerify)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Could not format and/or issue invoke for "
		  "Delivery Verify operation, reason 0x%x", rc));
	return 0;
    }

    return 0;
}


static Int
aS3S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *		pOperation = hUserData;
    EMSD_DeliveryVerifyResult *	pDeliveryVerifyRes;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "aS3S1a: Verify result"));

    /* Point to the Delivery Verify Response */
    pDeliveryVerifyRes = pOperation->hOperationData;

    if (pDeliveryVerifyRes->deliveryStatus ==
	EMSD_DeliveryVerifyStatus_NonDeliveryReportIsSentOut)
    {
	/*
	 * The MTS has sent a non-delivery report, but our user has
	 * the message already.  Let the user know this.
	 */
	(*ua_globals.pfModifyDeliverStatus)(pOperation->messageId,
					    UASH_DeliverStatus_NonDelSent);
    }
    else
    {
	/*
	 * The MTS hasn't sent anything abnormal.  Just indicate to
	 * the user that the MTS knows that it's successfully
	 * delivered the message.
	 */
	(*ua_globals.pfModifyDeliverStatus)(pOperation->messageId,
					    UASH_DeliverStatus_Acknowledged);
    }

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}


static Int
aS3S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *		pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "aS3S1b: Dropping message"));
    
    /*
     * We were not able to confirm whether the MTS knows that the
     * message has been successfully delivered.  The MTS may have sent
     * out a non-delivery report.  Let our user know this.
     */
    (*ua_globals.pfModifyDeliverStatus)(pOperation->messageId,
					UASH_DeliverStatus_PossibleNonDelSent);

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}




static Int
aS4S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS4S1a"));
    
    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}


static Int
aS4S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS4S1b"));
    
    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}


static Int
aS5S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS5S1a"));
    
    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}


static Int
aS5S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS5S1b"));
    
    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return 0;
}


static Int
aS5S1c(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    UA_Operation *	pOperation = hUserData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "aS5S1c"));
    
    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

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
segmentationTimerExpired(void * hTimer,
			 void * hUserData1,
			 void * hUserData2)
{
    UA_Operation * pOperation = hUserData1;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "Segmentation timer expired"));

    /* Free the finite state machine and operation structure */
    ua_freeOperation(pOperation);

    return Success;
}


static ReturnCode
formatAndInvoke(UA_Operation * pOperation,
		EMSD_DeliveryVerifyArgument * pDeliveryVerifyArg)
{
    ReturnCode 			rc;
    OS_Uint32			len;
    QU_Head *			pCompTree;
    void *			hBuf;

    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
	      "FormatAndInvoke (delivery verify)  entered"));

    /*Get the ASN compile tree for a Submit Argument. */
    pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliveryVerifyArg);

    /* Allocate a buffer to begin formatting the Submit Argument */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	return rc;
    }

    /* Arrange for the buffer to be freed */
    if ((rc = ua_operationAddUserFree(pOperation, BUF_free, hBuf)) != Success)
    {
	BUF_free(pOperation->hCurrentBuf);
	return rc;
    }

    /* Format the Delivery Verify Argument. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
	      "Formatting Delivery Verify Argument"));
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 hBuf,
			 pDeliveryVerifyArg,
			 &len)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "ASN_format() failed; rc=0x%04x", rc));
	return rc;
    }

    /* Issue the LS_ROS Invoke Request */
    if ((rc = ua_issueInvoke(pOperation, hBuf)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_STATE,
		  "ua_issueInvoke() failed; rc=0x%04x", rc));
	return rc;
    }

    /* All done. */
    TM_TRACE((ua_globals.hTM, UA_TRACE_STATE, "formatAndInvoke() OK"));
    return Success;
}
