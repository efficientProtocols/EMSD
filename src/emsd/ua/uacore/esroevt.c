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
 * Functions:
 *    ua_invokeIndication()
 *    ua_resultIndication()
 *    ua_errorIndication()
 *    ua_resultConfirm()
 *    ua_errorConfirm()
 *    ua_failureIndication()
 *    findOperationByInvokeId()
 *    issueErrorRequest()
 *    viewToBuf()
 */


#include "estd.h"
#include "du.h"
#include "addr.h"
#include "inetaddr.h"
#include "tm.h"
#include "eh.h"
#include "buf.h"
#include "fsm.h"
#include "esro_cb.h"
#include "emsd.h"
#include "mtsua.h"
#include "uacore.h"
#include "dup.h"

/*
 * Forward Declarations
 */
FORWARD static UA_Operation *
findOperationByInvokeId(ESRO_InvokeId invokeId);

FORWARD static void
issueErrorRequest(ReturnCode rc,
		  ESRO_InvokeId invokeId);

FORWARD static ReturnCode
viewToBuf(DU_View hView,
	  void ** phBuf);


int
ua_invokeIndication(ESRO_SapDesc hSapDesc,
		    ESRO_SapSel sapSel, 
		    T_SapSel * pRemoteTsap,
		    N_SapAddr * pRemoteNsap,
		    ESRO_InvokeId invokeId, 
		    ESRO_OperationValue operationValue,
		    ESRO_EncodingType encodingType, 
		    DU_View hView)
{
    void *		hBuf;
    UA_Operation *	pOperation;
    ReturnCode		rc;

    if (pRemoteTsap == NULL  ||  pRemoteNsap == NULL )
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Refusing INVOKE.ind: Zero length parameter is invalid."));
	EH_problem("Refusing Message: Invalid address!");
	issueErrorRequest(EMSD_Error_ProtocolViolation, invokeId);
	return SUCCESS;
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "\n\tINVOKE.ind from NSAP %u.%u.%u.%u\n"
	      "\tinvokeId = 0x%lx\n"
	      "\toperationValue = %d\n"
	      "\tsapSel = %u\n",
	      pRemoteNsap->addr[0], pRemoteNsap->addr[1],
	      pRemoteNsap->addr[2], pRemoteNsap->addr[3],
	      (unsigned long) invokeId, operationValue, sapSel));

    if (hView == NULL)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Refusing INVOKE.ind: Zero length parameter is invalid."));
	EH_problem("Refusing Message: Zero length parameter is invalid.");
	issueErrorRequest(EMSD_Error_ProtocolViolation, invokeId);
	return SUCCESS;
    }

    /* If we're not yet configured, they can't receive invoke indications. */
    if (ua_globals.mtsNsap.len == 0)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Refusing INVOKE.ind; MTS NSAP not yet configured."));
	issueErrorRequest(UA_RC_SecurityViolation, invokeId);
	return SUCCESS;
    }

    /* check for duplicate */
    if (ua_isDuplicate(hView, operationValue))
    {
	/*
	 * WARNING!!!
	 *
	 * At the time of implementation of this module, the only
	 * operation which requires duplicate detection by the UA is a
	 * DELIVER.request.  The RESULT for a DELIVER.request is a
	 * NULL result.
	 *
	 * If, at some future time, duplicate detection must be
	 * accomplished on operations which require data in their
	 * RESULT.request, that information will need to be maintained
	 * and provided to the invoker here.
	 */
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Duplicate Operation found.  Resending RESULT.req"));

	if (ESRO_CB_resultReq(invokeId, NULL,
			      ESRO_EncodingBER, (DU_View) NULL) != SUCCESS)
	{
	    /* Couldn't.  Let 'em know. */
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "RESULT.req for duplicate operation failed"));
	}

	/* It's a duplicate.  Just ignore it. */
	return SUCCESS;
    }

    /* Convert the DU_View into a BUF_buffer */
    if ((rc = viewToBuf(hView, &hBuf)) != Success)
    {
	/* Couldn't copy view to buffer */
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
                 "ua_invokeIndication: viewToBuf failed\n"));
	issueErrorRequest(rc, invokeId);
	return SUCCESS;
    }
    
    TM_CALL(ua_globals.hTM, UA_TRACE_PDU, BUF_dump, hBuf, "INVOKE.ind data");

    /* Preprocess the request (e.g. parse the buffer, get operation pointer) */
    if ((rc = ua_preprocessInvokeIndication(hBuf,
					    operationValue,
					    encodingType,
					    invokeId,
					    hSapDesc,
					    sapSel,
					    &pOperation)) != Success)
    {
	/* Couldn't parse buffer or some other problem */
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
                 "ua_invokeIndication: ua_preprocessInvokeIndication failed\n"));
	issueErrorRequest(rc, invokeId);
	BUF_free(hBuf);
	return SUCCESS;
    }
    
    /* We should be done with our copy of the buffer now. */
    BUF_free(hBuf);
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_InvokeIndication);
    
    return Success;

} /* ua_invokeIndication() */


int
ua_resultIndication(ESRO_InvokeId invokeId, 
		    ESRO_UserInvokeRef userInvokeRef,
		    ESRO_EncodingType encodingType, 
		    DU_View hView)
{
    UA_Operation *	pOperation;
    ReturnCode 	        rc;
    void *		hBuf;
    
    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received result indication for unknown invoke id\n");
	return FAIL;
    }
    
    if (hView == NULL)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Refusing INVOKE.ind; Zero length parameter is invalid."));
	EH_problem("Received RESULT with zero length parameter. It's invalid.");
	return SUCCESS;
    }

    /* Convert the DU_View into a BUF_buffer */
    if ((rc = viewToBuf(hView, &hBuf)) != Success)
    {
	/* Pretend it never happened */
	return SUCCESS;
    }
    
    TM_CALL(ua_globals.hTM, UA_TRACE_PDU, BUF_dump, hBuf, "RESULT.ind data");

    /* Preprocess the result (e.g. parse the buffer) */
    if ((rc = ua_preprocessResultIndication(pOperation, hBuf)) != Success)
    {
	/* Pretend it never happened */
	BUF_free(hBuf);
	return SUCCESS;
    }
    
    /* We should be done with our copy of the buffer now. */
    BUF_free(hBuf);
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_ResultIndication);
    
    return Success;

} /* ua_resultIndication() */


int
ua_errorIndication(ESRO_InvokeId invokeId, 
		   ESRO_UserInvokeRef userInvokeRef,
		   ESRO_EncodingType encodingType,
		   ESRO_ErrorValue errorValue,
		   DU_View hView)
{
    UA_Operation *	pOperation;
    ReturnCode		rc;
    void *		hBuf;
    
    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received error indication for unknown invoke id\n");
	return FAIL;
    }
    
    if (hView != NULL)
    {
    	/* Convert the DU_View into a BUF_buffer */
    	if ((rc = viewToBuf(hView, &hBuf)) != Success)
    	{
	    /* Pretend it never happened */
	    return SUCCESS;
        }

        TM_CALL(ua_globals.hTM, UA_TRACE_PDU, BUF_dump, hBuf, "ERROR.ind data");

    	/* Preprocess the error (e.g. parse the buffer) */
    	if ((rc = ua_preprocessErrorIndication(pOperation, hBuf)) != Success)
    	{
	    /* Pretend it never happened */
	    BUF_free(hBuf);
	    return SUCCESS;
    	}
    
    	/* We should be done with our copy of the buffer now. */
    	BUF_free(hBuf);
    
    } else {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Received Error response with zero length parameter. "));
    }

    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_ErrorIndication);
    
    return Success;

} /* ua_errorIndication() */


int
ua_resultConfirm(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    UA_Operation *	pOperation;
    
    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received result confirm for unknown invoke id:\n"
		   "possibly the result to a duplicate operation "
		   "(which is ok to be unknown)");
	return Fail;
    }
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_ResultConfirm);
    
    return Success;

} /* ua_resultConfirm() */


int
ua_errorConfirm(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    UA_Operation *	pOperation;
    
    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received error confirm for unknown invoke id\n");
	return Fail;
    }
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_ErrorConfirm);
    
    return Success;

} /* ua_errorConfirm() */


int
ua_failureIndication(ESRO_InvokeId invokeId,
		     ESRO_UserInvokeRef userInvokeRef,
		     ESRO_FailureValue failureValue)
{
    UA_Operation *	pOperation;
    
    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received failure indication for unknown invoke id\n");
	return Fail;
    }
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_FailureIndication);
    
    return Success;

} /* ua_failureIndication() */


static UA_Operation *
findOperationByInvokeId(ESRO_InvokeId invokeId)
{
    UA_Operation *	pOperation;
    
    /* For each operation in our active operations queue... */
    for (pOperation = QU_FIRST(&ua_globals.activeOperations);
	 ! QU_EQUAL(pOperation, &ua_globals.activeOperations);
	 pOperation = QU_NEXT(pOperation))
    {
	/* ... does this one have the specified invoke id? */
	if (pOperation->invokeId == invokeId)
	{
	    /* Yup. */
	    return pOperation;
	}
    }
    
    return NULL;

} /* findOperationByInvokeId() */

static void
issueErrorRequest(ReturnCode rc,
		  ESRO_InvokeId invokeId)
{
    EMSD_Error	    errorVal;

    switch(rc)
    {
    case ResourceError:
	errorVal = EMSD_Error_ResourceError;
	break;
	
    case UA_RC_UnrecognizedProtocolVersion:
	errorVal = EMSD_Error_ProtocolVersionNotRecognized;
	break;
	
    case UA_RC_SecurityViolation:
	errorVal = EMSD_Error_SecurityError;
	break;

    default:
	errorVal = EMSD_Error_ProtocolViolation;
	break;
    }
    
    /* Issue the error request */
    ua_issueErrorRequest(invokeId, errorVal);

} /* issueErrorRequest() */


static ReturnCode
viewToBuf(DU_View hView,
	  void ** phBuf)
{
    unsigned char * p;
    STR_String 	    hStr;
    ReturnCode	    rc;

    if (hView == NULL)
    {
	EH_problem("viewToBuf: Zero is passed as view\n");
	return Fail;
    }
    /* Get a pointer to the data in the view */
    p = DU_data(hView);

    /* Allocate an STR_String for this data */
    if ((rc =
	 STR_attachString(DU_size(hView), DU_size(hView), p, FALSE, &hStr)) !=
	Success)
    {
	return rc;
    }

    /* Allocate a buffer */
    if ((rc = BUF_alloc(0, phBuf)) != Success)
    {
	STR_free(hStr);
	return rc;
    }

    /* Append the view data to the new buffer */
    if ((rc = BUF_appendChunk(*phBuf, hStr)) != Success)
    {
	STR_free(hStr);
	BUF_free(phBuf);
	return rc;
    }

    /*
     * The reference count on the STR_string has been incremented,
     * since the buffer now has its copy of it.  We can free our copy
     * of it.
     */
    STR_free(hStr);

    return Success;

} /* viewToBuf() */


