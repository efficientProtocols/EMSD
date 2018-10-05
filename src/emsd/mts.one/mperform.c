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

#define NULL_CHECK(x,y)		((x) ? (x) : (y))
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

static char *rcs = "$Id: mperform.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";

#include "estd.h"
#include "eh.h"
#include "buf.h"
#include "tmr.h"
#include "mm.h"
#include "fsm.h"
#include "fsmtrans.h"
#include "emsdmail.h"
#include "emsd822.h"
#include "esro_cb.h"
#include "mtsua.h"
#include "mts.h"
#include "verify.h"
#include "subpro.h"
#include "msgstore.h"


/*
 * This is a temporary array to allow for testing.
 * A persistent queue, based on the NVM module, will replace this array.
 * -- fnh
 */

int                messageCount = 0;
DeliverVerifyList  deliverVerifyList[MAX_VERIFY_MESSAGES];

void
verify_setStatus (char *messageId, VState status)
{
    int   index  = 0;

    for (index=0; index < MAX_VERIFY_MESSAGES && index < messageCount; index++)
      {
	if (!strcmp (deliverVerifyList[index].messageId, messageId))
	     break;
      }
    if (index > MAX_VERIFY_MESSAGES)
      {
	index = messageCount++;
      }

    sprintf (deliverVerifyList[index].messageId, "%1.*s", MAX_VERIFY_MSGID_LEN, messageId);
    deliverVerifyList[index].status = status;

    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
	     "Status of %s is %d", messageId, status));
}

VState
verify_getStatus (char *messageId)
{
    int                      index  = 0;
    EMSD_DeliveryVerifyStatus status = EMSD_DeliveryVerifyStatus_NoReportIsSentOut;

    for (index=0; index < MAX_VERIFY_MESSAGES && index < messageCount; index++)
      {
	if (!strcmp (deliverVerifyList[index].messageId, messageId))
	  {
	     status = deliverVerifyList[index].status;
	     break;
	  }
      }
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "DeliveryVerify status of %s is %d",
		      messageId, status));
    return (status);
}

char *
mts_messageIdEmsdLocalToRfc822(EMSD_LocalMessageId * pEmsdLocalMessageId,
			      char * pBuf)
{
    sprintf(pBuf, "%08lx.%08lx@%s", pEmsdLocalMessageId->submissionTime,
	    pEmsdLocalMessageId->dailyMessageNumber, globals.config.pLocalHostName);

    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Converted EMSD Message ID to RFC-822: %s", pBuf));

    return pBuf;
}

/*
 * Performer State Diagram
 *
 * E: = Event
 * P: = Predicate
 * A: = Action
 *
 * [x] = Identifier allowing multiple possible transitions between two states.
 *
 * Transitions are identified by the Starting State number, Ending State
 * number, and Identifier.  For example, the transition from State 1 to State
 * 2, upon receiving an INVOKE.ind is "S1S2a".  Predicate functions prepend a
 * 'p' to the transition name (e.g. "pS1S2a") whereas Action functions prepend
 * an 'a' to the transition name (e.g. 'aS1S2a").
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
 * | State  | E: INVOKE.ind         | State  |
 * |   1    | P: Type==Submit,      |   2    |
 * |        |    SAP==3Way          |        |
 * |        | A: Issue RESULT.req,  |        |
 * |        |    Start seg. timer   |        |
 * |        |                       |        |
 * |   I    |                       |S       |
 * |   d    |                   [A] |u R     |
 * |   l    |<---------------------<|b e R   |
 * |   e    | E: RESULT.con         |m s e W |
 * |        | P: Last segment       |i u s a |
 * |        | A: Spool message      |t l p i |
 * |        |                       |  t o t |
 * |        |                   [B] |    n   |
 * |        |<---------------------<|    s   |
 * |        | E: RESULT.con         |    e   |
 * |        | P: More segments      |        |
 * |        | A: Concatenate segment|        |
 * |        |                       |        |
 * |        |                   [C] |        |
 * |        |<---------------------<|        |
 * |        | E: Segmentation Timer |        |
 * |        |    expired            |        |
 * |        |                       |        |
 * |        |                   [D] |        |                       +--------+
 * |        |<---------------------<|        |                       |        |
 * |        | E: FAILURE.ind        |        | [A]                   |        |
 * |        | P: More segments      |        |>--------------------->|        |
 * |        |                       |        | E: FAILURE.ind        | State  |
 * |        |                       +--------+ P: Last segment       |   3    |
 * |        |                                  A: Issue SUBMIT.verify|        |
 * |        |                                                        |        |
 * |        |                                                        |        |
 * |        |                                                    [A] |        |
 * |        |<------------------------------------------------------<|S       |
 * |        | E: RESULT.ind                                          |u V     |
 * |        | P: Response==SendMessage                               |b e R   |
 * |        | A: Spool message                                       |m r e W |
 * |        |                                                        |i i s a |
 * |        |                                                    [B] |t f p i |
 * |        |<------------------------------------------------------<|  y o t |
 * |        | E: RESULT.ind                                          |    n   |
 * |        | P: Response==DropMessage                               |    s   |
 * |        |                                                        |    e   |
 * |        |                                                    [C] |        |
 * |        |<------------------------------------------------------<|        |
 * |        | E: FAILURE.ind                                         |        |
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
 * |        |      DeliveryVerify   |        |
 * |        | A: Process request    |        |
 * |        |    Issue RESULT.req   |        |
 * |        |       or ERROR.req    |        |
 * |        |                       |n       |
 * |        |                       |o R     |
 * |        | [B]                   |n e R   |
 * |        |>--------------------->|- s e W |
 * |        | E: INVOKE.ind         |S u s a |
 * |        | P: type==             |u l p i |
 * |        |      DeliveryControl  |b t o t |
 * |        | A: Process request    |m   n   |
 * |        |    Issue RESULT.req   |i   s   |
 * |        |       or ERROR.req    |t   e   |
 * |        |                       |        |
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
FORWARD static FSM_State	s2SubmitResultResponseWait;
FORWARD static FSM_State	s3SubmitVerifyResponseWait;
FORWARD static FSM_State	s4ErrorResponseWait;
FORWARD static FSM_State	s5NotSubmitResultResponseWait;


/* Transition Diagrams */
FORWARD static FSM_Trans	t1Idle[];
FORWARD static FSM_Trans	t2SubmitResultResponseWait[];
FORWARD static FSM_Trans	t3SubmitVerifyResponseWait[];
FORWARD static FSM_Trans	t4ErrorResponseWait[];
FORWARD static FSM_Trans	t5NotSubmitResultResponseWait[];


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
pS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId);

FORWARD static OS_Boolean
pS3S1a(void * hMachine,
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
aS2S1c(void * hMachine,
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
aS3S1c(void * hMachine,
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
writeRfc822Message(char * pDataFileName,
		   ENVELOPE * pRfcEnv,
		   char * pExtraHeaders,
		   char * pBody);


FORWARD static void
writeRfc822Header(FILE * hFolder,
		  ENVELOPE * pRfcEnv,
		  char *pExtraHeaders);

FORWARD static void
writeRfc822HeaderEntry(FILE * hFolder,
		  ENVELOPE * pRfcEnv,
		  char * pFieldName,
		  void * pFieldData,
		  void (*pfAddRfcHeaderOrAddress)());

FORWARD static ReturnCode
segmentationTimerExpired(void * hTimer,
			 void * hUserData1,
			 void * hUserData2);

FORWARD ReturnCode
spoolNow(char * pControlFileName);

FORWARD char *
generateMessageId(MTS_Operation * pOperation);


/*
 * Actual Declarations
 */

/* States */
static FSM_State 	s1Idle =
{
    NULL,
    NULL,
    t1Idle,
    "MTS Performer Idle"
};


static FSM_State	s2SubmitResultResponseWait =
{
    NULL,
    NULL,
    t2SubmitResultResponseWait,
    "MTS Performer Submit Result Response Wait"
};


static FSM_State	s3SubmitVerifyResponseWait =
{
    NULL,
    NULL,
    t3SubmitVerifyResponseWait,
    "MTS Performer Submit Verify Response Wait"
};


static FSM_State	s4ErrorResponseWait =
{
    NULL,
    NULL,
    t4ErrorResponseWait,
    "MTS Performer Error Response Wait"
};


static FSM_State	s5NotSubmitResultResponseWait =
{
    NULL,
    NULL,
    t5NotSubmitResultResponseWait,
    "MTS Performer Not Submit Result Response Wait"
};


/* Transition Diagrams */
static FSM_Trans	t1Idle[] =
{ 
    {
	MTS_FsmEvent_InvokeIndication,
	pS1S2a,
	aS1S2a,
	&s2SubmitResultResponseWait,
	"s1s2a (INVOKE.ind, type=submit, sap=3Way)"
    },
    {
	MTS_FsmEvent_InvokeIndication,
	pS1S5a,
	aS1S5a,
	&s5NotSubmitResultResponseWait,
	"s1s5a (INVOKE.ind, type=delivery verify)"
    },
    {
	MTS_FsmEvent_InvokeIndication,
	pS1S5b,
	aS1S5b,
	&s5NotSubmitResultResponseWait,
	"s1s5b (INVOKE.ind, type=delivery control)"
    },
    {
	MTS_FsmEvent_InvokeIndication,
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


static FSM_Trans	t2SubmitResultResponseWait[] =
{ 
    {
	MTS_FsmEvent_ResultConfirm,
	pS2S1a,
	aS2S1a,
	&s1Idle,
	"s2s1a (RESULT.con, last segment)"
    },
    {
	MTS_FsmEvent_ResultConfirm,
	NULL,
	NULL,
	&s1Idle,
	"s2s1b (RESULT.con, more segments)"
    },
    {
	MTS_FsmEvent_FailureIndication,
	pS2S1d,
	NULL,
	&s1Idle,
	"s2s3a (FAILURE.ind, more segments)"
    },
    {
	MTS_FsmEvent_FailureIndication,
	NULL,
	aS2S3a,
	&s3SubmitVerifyResponseWait,
	"s2s3a (FAILURE.ind, last segment)"
    },
    {
	MTS_FsmEvent_SegmentationTimerExpired,
	NULL,
	aS2S1c,
	&s1Idle,
	"s2s1c (Segmentation Timer expired)"
    },
    {
	MTS_FsmEvent_InvokeIndication,
	pS2S2a,
	aS2S2a,
	&s2SubmitResultResponseWait,
	"s2s2a (INVOKE.ind, queued)"
    },
    {
	FSM_EvtDefault,
	NULL,
	aUnexpectedEvent,
	&s1Idle,
	"Unexpected Event (default catch-all)"
    }
};


static FSM_Trans	t3SubmitVerifyResponseWait[] =
{
    {
	MTS_FsmEvent_ResultIndication,
	pS3S1a,
	aS3S1a,
	&s1Idle,
	"s3s1a (RESULT.ind, response=Send Message)"
    },
    {
	MTS_FsmEvent_ResultIndication,
	NULL,
	aS3S1b,
	&s1Idle,
	"s3s1b (RESULT.ind, response=Drop Message)"
    },
    {
	MTS_FsmEvent_FailureIndication,
	NULL,
	aS3S1c,
	&s1Idle,
	"s3s1c (FAILURE.ind)"
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
	MTS_FsmEvent_ErrorConfirm,
	NULL,
	aS4S1a,
	&s1Idle,
	"s4s1a (ERROR.con)"
    },
    {
	MTS_FsmEvent_FailureIndication,
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


static FSM_Trans	t5NotSubmitResultResponseWait[] =
{
    {
	MTS_FsmEvent_ResultConfirm,
	NULL,
	aS5S1a,
	&s1Idle,
	"s5s1a (RESULT.con)"
    },
    {
	MTS_FsmEvent_ErrorConfirm,
	NULL,
	aS5S1b,
	&s1Idle,
	"s5s1b (ERROR.con)"
    },
    {
	MTS_FsmEvent_FailureIndication,
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
 * mts_performerInit()
 *
 * Initialize the Performer transition diagrams.
 */
ReturnCode
mts_performerFsmInit(void)
{
    if ((globals.hPerformerTransDiag =
	 FSM_TRANSDIAG_create("MTS Performer", &s1Idle)) == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
		 "FSM for MTS Performer failed to initialize"));
	return Fail;
    }
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "FSM for MTS Performer initialized"));
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
 * Enabling Condition:	   type=Submit and sap=3Way
 */
static OS_Boolean
pS1S2a(void *       hMachine,
       void *       hUserData,
       FSM_EventId  eventId)


{
    int				i                   = 0;
    int				len                 = 0;
    char *			p                   = NULL;
    char *			pBody               = NULL;
    char *			pControlFileName    = NULL;
    ReturnCode			rc                  = Success;
    MTS_Operation *		pOperation          = hUserData;
    SUB_Profile *		pSubPro             = NULL;
    SUB_DeviceProfile *		pProfile            = NULL;
    MTSUA_Recipient *		pRecip              = NULL;
    MTSUA_ControlFileData *	pControlFileData    = NULL;
    EMSD_RecipientData *		pAddr               = NULL;
    EMSD_SubmitArgument *	pSubmitArg          = pOperation->hOperationData;
    EMSD_Message *		pIpm                = NULL;
    EMSD_ORAddress *		pORAddr             = NULL;
    OptionalSegmentInfo *	pOptionalSegInfo    = NULL;
    OS_Uint32			pduLength           = 0;
    OS_Uint32			thisSeg             = 0;
    ENVELOPE *			pRfcEnv             = NULL;
    void *			pCompTree           = NULL;
    
    static char                 extraHeaders[2048];
    
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "S1->S2a predicate entered {%ld}", pOperation->operId));

    /* Get the device profile for this NSAP address */
    if ((pSubPro = SUB_getEntry (NULL, &pOperation->nsap)) == NULL) {
      TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "No subscriber for NSAP"));
      return FALSE;
    } else {
      pProfile = &pSubPro->deviceProfile;
    }      

    /* See if it's a Submit operation, and received on the 3-way SAP */
    if ((pOperation->operationValue != EMSD_Operation_Submission_1_1 &&
	 pOperation->operationValue != EMSD_Operation_Submission_1_0_WRONG) ||
	pOperation->hSapDesc != globals.sap.hPerformer3Way)
    {
	/* Nope. */
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
		 "Submit (3-way) operation rejected"));
	return FALSE;
    }
    
    /*
     * Determine if this operation is segmented.  First, point to the
     * segmentation structure.
     */
    
    /* Point to the submit argument structure */
    pSubmitArg = (EMSD_SubmitArgument *) pOperation->hOperationData;
    
    /* Point to the segment info and exists flag */
    pOptionalSegInfo = (OptionalSegmentInfo *) &pSubmitArg->segmentInfo;

    MM_logMessage(globals.hMMLog, globals.tmpbuf,
		  "MTS received submit operation; OperVal=0x%x, ContentType=%d%s\n",
		  pOperation->operationValue, pSubmitArg->contentType,
		   pOptionalSegInfo->exists ? "; Segmented" : "");


    /* Is this operation segmented? */
    if (pOptionalSegInfo->exists)
    {
#include "segment.c"      /* params =  pOperation */
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
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL,
		  "Ready to parse submitArgument, content type = %ld",
		  pSubmitArg->contentType));
	/* See what type of contents we've received */
	switch(pSubmitArg->contentType)
	{
	case EMSD_ContentType_Ipm95:
#include "parse_ipm.c"
	    break;
	    
	case EMSD_ContentType_Probe:
	case EMSD_ContentType_VoiceMessage:
	case EMSD_ContentType_DeliveryReport:
	default:
	    pOperation->rc = UnsupportedOption;
	    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "Unsupported operation"));
	    return FALSE;
	}
    } else {
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Additional segments required to complete message"));
    }
    
    /* This transition is acceptable. */
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "S1->S2 Transition OK"));
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
 * Enabling Condition:	   type=DeliveryVerify
 */
static OS_Boolean
pS1S5a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "S1->S5a predicate entered {%ld}", pOperation->operId));
    /* See if it's a Delivery Verify operation */
    if (pOperation->operationValue != EMSD_Operation_DeliveryVerify_1_1)
    {
	/* Nope. */
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
		 "Invalid operation: %d vs %d",
		  pOperation->operationValue, EMSD_Operation_DeliveryVerify_1_1));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "S1->S5a transition OK"));
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Delivery Verification"));
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
 * Enabling Condition:	   type=DeliveryControl
 */
static OS_Boolean
pS1S5b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "S1->S5b predicate entered (Delivery Control) {%ld}", pOperation->operId));
    /* See if it's a Delivery Control operation */
    if (pOperation->operationValue != EMSD_Operation_DeliveryControl_1_1)
    {
	/* Nope. */
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
		 "Invalid operation: %d vs %d",
		  pOperation->operationValue, EMSD_Operation_DeliveryControl_1_1));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "S1->S5b transition OK"));
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Delivery Control"));

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
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "S2->S1a predicate entered {%ld}", pOperation->operId));

    /* See if we've received all segments */
    if (! pOperation->bComplete)
    {
	/* Nope. */
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
		 "Not all segments received"));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "S2->S1a transition OK"));
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
 * Enabling Condition:	   more segments to be received
 */
static OS_Boolean
pS2S1d(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "S2->S1d transition predicated entered {%ld}", pOperation->operId));
    /* See if we've received all segments */
    if (pOperation->bComplete)
    {
	/* Yup. */
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "All segments received"));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "S2->S1d transition OK"));
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
 * Enabling Event:	   INVOKE.indication
 *
 * Enabling Condition:	   more segments to be received
 */
static OS_Boolean
pS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "S2->S2a transition predicated entered {%ld}", pOperation->operId));
    
    /* This transition is acceptable. */
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "S2->S2a transition OK"));
    return TRUE;
}


/*
 * pS3S1a()
 *
 * Predicate function.
 *
 * Current State:          S3
 * Transition State:       S1
 *
 * Enabling Event:	   RESULT.indication
 *
 * Enabling Condition:	   Submit Verify response = Send Message
 */
static OS_Boolean
pS3S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *			pOperation = hUserData;
    EMSD_SubmissionVerifyResult *	pSubmitVerifyResult;
    
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "S3->S1a predicate entered {%ld}", pOperation->operId));

    /* Point to the result structure */
    pSubmitVerifyResult = pOperation->hOperationData;
    
    /* Is the verify response "Send Message"? */
    if (pSubmitVerifyResult->submissionStatus != EMSD_SubmissionVerifyStatus_SendMessage)
    {
	/* Nope. */
        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
		 "SubmitVerify: invalid result: %ld vs %d",
		  pSubmitVerifyResult->submissionStatus, EMSD_SubmissionVerifyStatus_SendMessage));
	return FALSE;
    }
    
    /* This transition is acceptable. */
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "S3->S1a transition OK"));
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
	     "SubmitVerifyResult: Send message"));
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
    ESRO_RetVal         retval        = 0;
    MTS_Operation *     pOperation    = hUserData;
    ReturnCode          rc            = Success;
    OS_Uint16           length        = 0;
    OS_Uint32           len           = 0;
    DU_View             hView         = NULL;
    unsigned char *     pData         = NULL;
    QU_Head *           pCompTree     = NULL;
    void *              hBuf          = NULL;
    SUB_Profile *       pSubPro       = NULL;
    SUB_DeviceProfile * pProfile      = NULL;
    EMSD_SubmitResult    submitRes;

    /*
     * Create the Submit RESULT.req PDU
     */
    submitRes.localMessageId.submissionTime = pOperation->localMessageId.submissionTime;
    submitRes.localMessageId.dailyMessageNumber = pOperation->localMessageId.dailyMessageNumber;

    TM_TRACE((globals.hTM, MTS_TRACE_STATE, 
	     "aS1S2a: SubmitResult Message ID: %08lx/%ld",
	      submitRes.localMessageId.submissionTime,
	      submitRes.localMessageId.dailyMessageNumber));

    /* Get the device profile for this NSAP address */
    if ((pSubPro = SUB_getEntry (NULL, &pOperation->nsap)) == NULL) {
      TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "No subscriber for NSAP"));
      return FALSE;
    } else {
      pProfile = &pSubPro->deviceProfile;
    }      

    if ((rc = BUF_alloc(0, &hBuf)) == Success)  /* get storage for hBuf */
    {
        /* Determine which protocol tree to use. */
        pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_SubmitResult,
					  pProfile->protocolVersionMajor,
					  pProfile->protocolVersionMinor);
	/* generate a PDU */
	rc = ASN_format(ASN_EncodingRules_Basic, pCompTree, hBuf, &submitRes, &len);
	if (rc == Success)
	{
	    /*** BUF_getchuck must iterate until 'buffer exhausted' result ***/
	    if((rc = BUF_getChunk (hBuf, &length, &pData)) == Success)
	    {
	      hView = DU_alloc(G_duMainPool, BUF_getBufferLength(hBuf));
	      if (hView != NULL)
		  OS_copy (DU_data(hView), pData, length);
	    }
	}
    }
    if (hView != NULL) {
        /* Send back a RESULT.request */
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Issuing RESULT.req"));
        retval = ESRO_CB_resultReq(pOperation->invokeId, NULL, ESRO_EncodingBER, hView); /***/
    }
    else
    {
        TM_TRACE((globals.hTM, MTS_TRACE_WARNING, 
		 "Failed to send Submit Result PDU"));
	retval = Fail;
    }
    if (retval != Success)
    {
	/* Generate a FAILURE.ind. */
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
		 "Generating FAILURE.ind event"));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }

    if (hView)
        DU_free (hView);
    if (hBuf)
        BUF_free (hBuf);

    return 0;
}

static Int
aS1S4a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    OS_Uint32           err        = 0;
    
    /* Generate an error request */
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "aS1S4a: Error Request"));

    err = (pOperation->rc == ResourceError) ? EMSD_Error_ResourceError
                                            : EMSD_Error_ProtocolViolation;
    
    mts_issueErrorRequest(pOperation->invokeId, err, 0, 1, 1);

    return 0;
}

static Int
aS1S5a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode                    rc                 = Success;
    char *                        pMsgId             = NULL;
    VState                        state;
    void *			  hBuf               = NULL;
    QU_Head *			  pCompTree          = NULL;
    OS_Uint32 			  len                = 0;
    ESRO_RetVal			  retval             = Success;
    MTS_Operation *		  pOperation         = hUserData;
    SUB_Profile *       	  pSubPro            = NULL;
    SUB_DeviceProfile *  	  pProfile           = NULL;
    EMSD_DeliveryVerifyArgument *  pDeliveryVerify    = NULL;
    EMSD_DeliveryVerifyResult	  deliveryVerifyRes;

    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "aS1S5a: Delivery Verify"));

    /* Get the device profile for this NSAP address */
    if ((pSubPro = SUB_getEntry (NULL, &pOperation->nsap)) == NULL) {
      TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "No subscriber for NSAP"));
      return FALSE;
    } else {
      pProfile = &pSubPro->deviceProfile;
    }      

    /* Point to the deliver argument structure */
    pDeliveryVerify = (EMSD_DeliveryVerifyArgument *) pOperation->hOperationData;
    
    /* If the message id is of local format (it should be)... */
    if (pDeliveryVerify->messageId.choice == EMSD_MESSAGEID_CHOICE_LOCAL)
    {
	/*... generate id string from the EMSD Local-format message id */
        pMsgId = globals.tmpbuf;
	mts_messageIdEmsdLocalToRfc822(&pDeliveryVerify->messageId.un.local, pMsgId);
				      
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
		 "EMSD Message ID: %s", pMsgId));
    }
    else
    {
	/* Point to the RFC-822 message id */
	pMsgId = STR_stringStart(pDeliveryVerify->messageId.un.rfc822);
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
		 "RFC-822 Message ID: %s", pMsgId));
    }

    /* look up message by messageId */
    state = verify_getStatus (pMsgId);
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
	     "State=%d, Message ID: %s", state, pMsgId));

    /* send back deliveryVerifyResult PDU */
    {
	/* No.  Generate an RESULT.req with submission status of "Drop" */
	deliveryVerifyRes.deliveryStatus = state;
    }

    /*Get the ASN compile tree for a Submit Argument. */
    pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_DeliveryVerifyRes,
				      pProfile->protocolVersionMajor,
				      pProfile->protocolVersionMinor);

    /* Allocate a buffer to begin formatting the Delivery Verify Result */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "BUF_alloc() failed"));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }

    /* Arrange for the buffer to be freed */
    if ((rc = mts_operationAddUserFree(pOperation, BUF_free, hBuf)) != Success)
    {
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
		  "mts_operationAddUserFree() failed; rc=0x%04x", rc));
	BUF_free(pOperation->hCurrentBuf);
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }

    /* Format the Delivery Verify Argument. */
    TM_TRACE((globals.hTM, MTS_TRACE_STATE,
	      "Formatting Delivery Verify Result"));
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 hBuf,
			 &deliveryVerifyRes,
			 &len)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
		  "ASN_format() failed; rc=0x%04x", rc));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }

    /* Send back a RESULT.request */
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Issuing RESULT.req"));
    if ((retval = ESRO_CB_resultReq(pOperation->invokeId, NULL,
				    ESRO_EncodingBER, hBuf)) != SUCCESS)
    {
	/* Couldn't.  Generate a FAILURE.ind. */
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
		  "RESULT.req failed; generating FAILURE.ind event"));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }

    return 0;
}


static Int
aS1S5b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode                     rc                      = 0;
    QU_Head *			   pCompTree               = NULL;
    OS_Uint32 			   len                     = 0;
    void *			   hBuf                    = NULL;
    ESRO_RetVal			   retval                  = Success;
    MTS_Operation *	 	   pOperation              = hUserData;
    SUB_Profile *       	   pSubPro                 = NULL;
    SUB_DeviceProfile *  	   pProfile                = NULL;
    EMSD_DeliveryControlArgument *  pDeliveryControlArgPdu  = NULL;
    EMSD_DeliveryControlResult	   deliveryControlRes;


    TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "aS1S5b: Delivery Control"));

    /* Get the device profile for this NSAP address */
    if ((pSubPro = SUB_getEntry (NULL, &pOperation->nsap)) == NULL) {
      TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "No subscriber for NSAP issuing DeliveryControl"));
      return FALSE;
    } else {
      pProfile = &pSubPro->deviceProfile;
    }      

    /* Point to the deliver control argument structure */
    pDeliveryControlArgPdu = (EMSD_DeliveryControlArgument *) pOperation->hOperationData;
	    
#if WE_CARE_ABOUT_THE_DELIVERY_CONTROL_CONTENTS
    /* Get the length of the Contents PDU */
    pduLength = BUF_getBufferLength(pDeliverControlArgPdu);
    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "PDU length = %ld", pduLength));
    
    /* Determine which protocol tree to use */
    pCompTree =	mtsua_getProtocolTree(MTSUA_Protocol_DeliveryControl,
				      pProfile->protocolVersionMajor,
				      pProfile->protocolVersionMinor);
	    
    /* Parse the Delivery Control PDU */
    rc = ASN_parse(ASN_EncodingRules_Basic, pCompTree, pDeliveryControlArgPdu,
		   pDCArg, &pduLength);
    if (rc != Success)
      {
	TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "Parse of DeliveryControlArg failed [%d]", rc));
	return FALSE;
      }
#endif /* WE_CARE_... */    

    /*** check for messages for this UA (Based on NSAP) ***/
    TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "Checking delivery queue for NSAP XXX"));

#ifdef DELIVERY_CONTROL_SUPPORTED
    /* Get the ASN compile tree for a Delivery Control Result */
    pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_DeliveryControlRes,
				      pProfile->protocolVersionMajor,
				      pProfile->protocolVersionMinor);

    /* Allocate a buffer to begin formatting the Delivery Control Result */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "BUF_alloc() failed"));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
	return (Fail);
    }

    /* Arrange for the buffer to be freed */
    if ((rc = mts_operationAddUserFree(pOperation, BUF_free, hBuf)) != Success)
    {
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
		  "mts_operationAddUserFree() failed; rc=0x%04x", rc));
	BUF_free(pOperation->hCurrentBuf);
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
	return (Fail);
    }

    /* Format the Delivery Control Argument. */
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "Formatting Delivery Control Result"));
    rc = ASN_format(ASN_EncodingRules_Basic, pCompTree, hBuf, &deliveryControlRes, &len);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
		  "ASN_format(DeliveryControlResult) failed; rc=0x%04x", rc));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
    }

    /* Send back a RESULT.request */
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Issuing RESULT.req for Delivery Control"));
    retval = ESRO_CB_resultReq(pOperation->invokeId, NULL, ESRO_EncodingBER, hBuf);
    if (retval != SUCCESS)
    {
	/* Couldn't.  Generate a FAILURE.ind. */
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "RESULT.req failed; generating FAILURE.ind event"));
	FSM_generateEvent(hMachine, MTS_FsmEvent_FailureIndication);
	return (Fail);
    }
#else  /* No delivery control support */
    /* Issue a ERROR.ind to indicate we ignored the contents of the Delivey Control */
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
	      "Issuing ErrorRequest, InvokeId=%d", pOperation->invokeId));
    mts_issueErrorRequest(pOperation->invokeId, EMSD_Error_ProtocolViolation, 0,
			  pProfile->protocolVersionMajor, pProfile->protocolVersionMinor);
#endif /* DELIVERY_CONTROL_SUPPORTED */

    return (Success);
}

static Int
aS2S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode			rc1 = 0;
    ReturnCode			rc2 = 0;
    MTS_Operation *		pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "aS2S1a: Submit result"));
    /* The UA knows that we have the message.  Spool it. */
    if ((rc1 = spoolNow(pOperation->pControlFileName)) != Success ||
	(rc2 = TMR_start(TMR_SECONDS(0), pOperation->pControlFileName,
			 NULL, mts_tryRfc822Submit, NULL)) != Success)
    {
	/* Error spooling message, send non-delivery */
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
		  "aS2S1a: spool failed, creating timer [%d/%d]", rc1, rc2));
	mts_submissionNonDel(pOperation, (rc1 != Success) ? rc1 : rc2);
    }
    else
    {
	/* Don't free the control file name with the operation */
	mts_operationDelUserFree(pOperation, pOperation->pControlFileName);
    }

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
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "aS2S1c: Timer expired"));
    /* Let 'em know that we're throwing stuff away */
    (void) MM_logMessage(globals.hMMLog,
			 globals.tmpbuf,
			 "MTS segmentation timer expired, operId=%d, discarding partial message.\n",
			 pOperation->operId);
    
    
    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}

static Int
aS2S2a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
	      "aS2S2a: INVOKE.ind (queued)"));

    return 0;
}

static Int
aS2S3a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode 			   rc;
    OS_Uint32			   len;
    QU_Head *			   pCompTree;
    void *			   hBuf;
    MTS_Operation *	           pOperation         = hUserData;
    SUB_Profile *        	   pSubPro            = NULL;
    SUB_DeviceProfile *  	   pProfile           = NULL;
    EMSD_SubmissionVerifyArgument * pSubmissionVerify;

    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "aS2S3a: (Issue SUBMIT.verify)"));

    /* Get the device profile for this NSAP address */
    if ((pSubPro = SUB_getEntry (NULL, &pOperation->nsap)) == NULL) {
      TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "No subscriber for NSAP"));
      return FALSE;
    } else {
      pProfile = &pSubPro->deviceProfile;
    }      

    /* Allocate an EMSD Delivery Verify structure */
    if ((rc = mtsua_allocSubmissionVerifyArg(&pSubmissionVerify)) != Success)
    {
	TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
		  "AllocDeliveryVerify failed"));
	return 0;
    }
	    
    /* Arrange for the Submit Verify Argument to be freed later. */
    rc = mts_operationAddUserFree(pOperation,
				  ((void (*)(void *)) mtsua_freeSubmissionVerifyArg),
				  pSubmissionVerify);
    if (rc != Success)
    {
	mtsua_freeSubmissionVerifyArg(pSubmissionVerify);
	TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
		  "MTS Op AddUserFree failed"));
	return 0;
    }

    pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_SubmissionVerifyArg,
				      pProfile->protocolVersionMajor,
				      pProfile->protocolVersionMinor);

    /* Allocate a buffer to begin formatting the Submit Verify Argument */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	return rc;
    }

    /* Arrange for the buffer to be freed */
    if ((rc = mts_operationAddUserFree(pOperation, BUF_free, hBuf)) != Success)
    {
	BUF_free(pOperation->hCurrentBuf);
	return rc;
    }

    /* set the message id */
    pSubmissionVerify->messageId.choice = EMSD_MESSAGEID_CHOICE_LOCAL;
    pSubmissionVerify->messageId.un.local.submissionTime = pOperation->localMessageId.submissionTime;
    pSubmissionVerify->messageId.un.local.dailyMessageNumber = pOperation->localMessageId.dailyMessageNumber;

    /* Format the Submission Verify Argument. */
    TM_TRACE((globals.hTM, MTS_TRACE_STATE,
	      "Formatting Submission Verify Argument for %08lx/%ld",
	      pSubmissionVerify->messageId.un.local.submissionTime,
	      pSubmissionVerify->messageId.un.local.dailyMessageNumber));

    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 hBuf,
			 pSubmissionVerify,
			 &len)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE,
		  "ASN_format() failed; rc=0x%04x", rc));
	return rc;
    }

    /* override previous values in pOperation */
    /* Note that all exits from this state are to the idle state */
    /* so it is not necessary to restore the pOperation values below */
    pOperation->operationValue = EMSD_Operation_SubmissionVerify_1_1;
    pOperation->sapSel = MTSUA_Rsap_Ua_Performer_2Way;
    pOperation->hSapDesc = globals.sap.hInvoker2Way;

    /* Issue the LS_ROS Invoke Request */
    if ((rc = mts_issueInvoke(pOperation, hBuf)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE,
		  "mts_issueInvoke() failed; rc=0x%04x", rc));
	return rc;
    }

    /* All done. */
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "Sent SubmissionVerifyArg"));

    return Success;
}


static Int
aS3S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode		rc;
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "aS3S1a: Verify result"));

    /* The UA knows that we have the message.  Spool it. */
    if (spoolNow(pOperation->pControlFileName) != Success)
    {
	mts_submissionNonDel(pOperation, EMSD_Error_ResourceError);
    }
    else
    {
	/* Create a timer to process submitting this RFC-822 message */
	if ((rc = TMR_start(TMR_SECONDS(0),
			    pOperation->pControlFileName,
			    NULL,
			    mts_tryRfc822Submit,
			    NULL)) != Success)
	{
	    /* Let 'em know that we couldn't start the timer */
	    (void) MM_logMessage(globals.hMMLog,
				 globals.tmpbuf,
				 "MTS could not start timer for "
				 "control file (%s)\n",
				 pOperation->pControlFileName);
	    
	    mts_submissionNonDel(pOperation, rc);
	}
	else
	{
	    /* Don't free the control file name with the operation */
	    mts_operationDelUserFree(pOperation, pOperation->pControlFileName);
	}
    }

    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}


static Int
aS3S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode			rc;
    MTS_Operation *		pOperation = hUserData;
    MTSUA_ControlFileData	cfData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "aS3S1b: Dropping message"));

    MM_logMessage(globals.hMMLog,
		  globals.tmpbuf,
		  "Dropping message associated with control file '%s'\n",
		  pOperation->pControlFileName);
    /*
     * Remove the not-yet-spooled file
     */
    
    /* Read the control file */
    if ((rc = mtsua_readControlFile(pOperation->pControlFileName,
				    &cfData)) != Success)
    {
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "MTS could not read control file (%s)\n",
		      pOperation->pControlFileName);
    }
    else
    {
	/* Delete the data file */
	if (OS_deleteFile(cfData.dataFileName) != Success)
	{
	    MM_logMessage(globals.hMMLog,
			  globals.tmpbuf,
			  "MTS could not delete data file %s\n",
			  cfData.dataFileName);
	}
    
	/* Delete the control file */
	if (OS_deleteFile(pOperation->pControlFileName) != Success)
	{
	    MM_logMessage(globals.hMMLog,
			  globals.tmpbuf,
			  "MTS could not delete control file %s\n",
			  pOperation->pControlFileName);
	}
    }
    
    /* Free the control file data */
    mtsua_freeControlFileData(&cfData);
    
    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}


static Int
aS3S1c(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    ReturnCode			rc;
    MTS_Operation *		pOperation = hUserData;
    MTSUA_ControlFileData	cfData;
    
    /*
     * Remove the not-yet-spooled file
     */
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
	     "aS3S1c: FAILURE.ind, dropping message"));

    MM_logMessage(globals.hMMLog,
		  globals.tmpbuf,
		  "Dropping message associated with control file '%s'\n",
		  pOperation->pControlFileName);

    /* Read the control file */
    if ((rc = mtsua_readControlFile(pOperation->pControlFileName,
				    &cfData)) != Success)
    {
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "MTS could not read control file (%s)\n",
		      pOperation->pControlFileName);
    }
    else
    {
	/* Delete the data file */
	if (OS_deleteFile(cfData.dataFileName) != Success)
	{
	    MM_logMessage(globals.hMMLog,
			  globals.tmpbuf,
			  "MTS could not delete data file %s\n",
			  cfData.dataFileName);
	}
    
	/* Delete the control file */
	if (OS_deleteFile(pOperation->pControlFileName) != Success)
	{
	    MM_logMessage(globals.hMMLog,
			  globals.tmpbuf,
			  "MTS could not delete control file %s\n",
			  pOperation->pControlFileName);
	}
    }

    /* Free the control file data */
    mtsua_freeControlFileData(&cfData);
    
    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}


static Int
aS4S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS4S1a"));
    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}


static Int
aS4S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS4S1b"));
    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}


static Int
aS5S1a(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS5S1a"));
    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}


static Int
aS5S1b(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS5S1b"));
    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return 0;
}


static Int
aS5S1c(void * hMachine,
       void * hUserData,
       FSM_EventId eventId)
{
    MTS_Operation *	pOperation = hUserData;
    
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "aS5S1c"));
    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

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
writeRfc822Message(char * pDataFileName,
		   ENVELOPE * pRfcEnv,
		   char * pExtraHeaders,
		   char * pBody)
{
    char *	    p1;
    char *	    p2;
    FILE *	    hFolder;
    
    if ((hFolder = OS_fileOpen(pDataFileName, "at")) == NULL)
    {
	return OS_RC_FileOpen;
    }
    
    writeRfc822Header (hFolder, pRfcEnv, pExtraHeaders);    /* write RFC-822 header */
    
    /* Strip carriage returns from the body */
    for (p1 = p2 = pBody; *p2 != '\0'; )
    {
	if (*p2 != '\r')
	{
	    *p1++ = *p2++;
	}
	else
	{
	    ++p2;
	}
    }
    
    /* Null-terminate the CR-less string */
    *p1 = '\0';
    
    /* write the body */
    OS_filePutString(pBody, hFolder);
    
    /* write two trailing new lines to make it proper folder format */
    OS_filePutString("\n\n", hFolder);
    
    /* all done! */
    OS_fileClose(hFolder);

    unlink ("./fnh_datafile");
    link (pDataFileName, "./fnh_datafile");
    
    return Success;
}


static void
writeRfc822HeaderEntry(FILE * hFolder,
		  ENVELOPE * pRfcEnv,
		  char * pFieldName,
		  void * pFieldData,
		  void (*pfAddRfcHeaderOrAddress)())
{
    char 	    buf[1024];
    char *	    p1;
    char *	    p2;

    /* rfc822_header_line() concatenates to existing string.  Null terminate */
    *(p1 = buf) = '\0';
    
    /* Add this header line */
    (*pfAddRfcHeaderOrAddress)(&p1, pFieldName, pRfcEnv, pFieldData);
    
    /* Strip carriage returns from the header */
    for (p1 = p2 = buf; *p2 != '\0'; )
    {
	if (*p2 != '\r')
	{
	    *p1++ = *p2++;
	}
	else
	{
	    ++p2;
	}
    }
    
    /* Null-terminate the CR-less string */
    *p1 = '\0';
    
    /* write the header */
    OS_filePutString(buf, hFolder);
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "RFC-822 Header: '%s'", buf));
}


static ReturnCode
segmentationTimerExpired(void * hTimer,
			 void * hUserData1,
			 void * hUserData2)
{
    MTS_Operation * pOperation = hUserData1;

    TM_TRACE((globals.hTM, MTS_TRACE_MSG, "Segmentation timer expired"));

    /* Free the finite state machine and operation structure */
    mts_freeOperation(pOperation);

    return Success;
}

ReturnCode
spoolNow(char * pControlFileName)
{
    ReturnCode	        rc;
    char *		p;
    char		controlFileName[OS_MAX_FILENAME_LEN];
    
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "Spooling %s", pControlFileName));

    if (strlen(pControlFileName) < 8)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_STATE,
		  "Spool filename too short: '%s'", pControlFileName));
	return Fail;
    }

    /* Copy the current (unspooled) file name */
    strcpy(controlFileName, pControlFileName);
    
    /* Create a new prototype for the real control file. */
    p = controlFileName + strlen(controlFileName) - 6;
    strcpy(p, "XXXXXX");
    p = strrchr (controlFileName, '/');
    if (p)
    {
      if (*++p == 'T') {
	  *p = 'C';
      } else if (*p == 'C') {
	  TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Control filename already starts with 'C'"));
      } else {
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "*** Expected 'T', got '%c'", *p));
      }
    }
    else
    {
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "*** Control file name does not contain a '/'"));
    }

    /* Create a unique name for this file */
    if ((rc = OS_uniqueName(controlFileName)) != Success)
    {
	/* Let 'em know that we couldn't start the timer */
	(void) MM_logMessage(globals.hMMLog,
			     globals.tmpbuf,
			     "MTS could not create new name for "
			     "control file (%s)\n",
			     pControlFileName);
	
	return rc;
    }
    
    /* Rename the temporary file to the real control file name */
    if ((rc = OS_moveFile(controlFileName, pControlFileName)) != Success)
    {
	(void) MM_logMessage(globals.hMMLog,
			     globals.tmpbuf,
			     "MTS could not rename control file from (%s) to (%s)\n",
			     pControlFileName,
			     controlFileName);

	return rc;
    }
    
    /* Give 'em the new control file name.  It's the same length. */
    strcpy(pControlFileName, controlFileName);
    
    return Success;
}


char *
generateMessageId(MTS_Operation * pOperation)
{
    static OS_Uint16	uniqueness = 0;
    char		buf[512];
#if 0
    OS_ZuluDateTime	now;
#endif
    /*
     * Message ID's contain the initiating NSAP, current date/time, an
     * incrementing uniqueness value, and the local host name.
     */

    /* Get the current date/time */
    pOperation->localMessageId.submissionTime = time(NULL);
    pOperation->localMessageId.dailyMessageNumber = ++uniqueness;

    sprintf(buf, "%d.%d.%d.%d_%08lx_%08lx@EMSD.MTS.%s", 
	    pOperation->nsap.addr[0], pOperation->nsap.addr[1],
	    pOperation->nsap.addr[2], pOperation->nsap.addr[3],
	    pOperation->localMessageId.submissionTime, (OS_Uint32)uniqueness,
	    globals.config.pLocalHostName);

    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "New message ID is %0.64s", buf));

    /* Allocate memory for this string */
    return OS_allocStringCopy(buf);
}

static void
writeRfc822Header (FILE *hOut, ENVELOPE * pRfcEnv, char *pExtraHeaders)
{
    char *p1 = NULL;

    /* build RFC-822 header */
    p1 = globals.tmpbuf;
    *p1 = '\0';
    
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL,
	      "Writing RFC-822 message; extra header fields = '%s'",
	      pExtraHeaders ? pExtraHeaders : "None"));

    writeRfc822HeaderEntry(hOut, pRfcEnv, "Date", pRfcEnv->date, rfc822_header_line);
    writeRfc822HeaderEntry(hOut, pRfcEnv, "From", pRfcEnv->from, rfc822_address_line);
    writeRfc822HeaderEntry(hOut, pRfcEnv, "Sender", pRfcEnv->sender, rfc822_address_line);
    writeRfc822HeaderEntry(hOut, pRfcEnv, "Reply-To", pRfcEnv->reply_to, rfc822_address_line);
    writeRfc822HeaderEntry(hOut, pRfcEnv, "Subject", pRfcEnv->subject, rfc822_header_line);
    writeRfc822HeaderEntry(hOut, pRfcEnv, "To", pRfcEnv->to, rfc822_address_line);
    writeRfc822HeaderEntry(hOut, pRfcEnv, "Cc", pRfcEnv->cc, rfc822_address_line);
    writeRfc822HeaderEntry(hOut, pRfcEnv, "In-Reply-To", pRfcEnv->in_reply_to, rfc822_header_line);
    writeRfc822HeaderEntry(hOut, pRfcEnv, "Message-ID", pRfcEnv->message_id, rfc822_header_line);

    /* write the extra headers */
    if (pExtraHeaders && *pExtraHeaders)
        OS_filePutString(pExtraHeaders, hOut);
    
    /* put in the header-terminating "\n" */
    OS_filePutString("\n", hOut);
}
