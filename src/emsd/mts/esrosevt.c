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

static char *rcs = "$Id: esrosevt.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";

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
#include "mts.h"
#include "subpro.h"

/*
 * Forward Declarations
 */
FORWARD static MTS_Operation *
findOperationByInvokeId(ESRO_InvokeId invokeId);

FORWARD static void
issueErrorRequest(ReturnCode rc,
		  ESRO_InvokeId invokeId,
		  OS_Uint8 protocolVersionMajor,
		  OS_Uint8 protocolVersionMinor);

FORWARD static ReturnCode
viewToBuf(DU_View hView,
	  void ** phBuf);

int
mts_invokeIndication(ESRO_SapDesc        hSapDesc,
		     ESRO_SapSel         sapSel, 
		     T_SapSel *          pRemoteTsap,
		     N_SapAddr *         pRemoteNsap,
		     ESRO_InvokeId       invokeId, 
		     ESRO_OperationValue operationValue,
		     ESRO_EncodingType   encodingType, 
		     DU_View             hView)
{
    void *		hBuf         = NULL;
    MTS_Operation *	pOperation   = NULL;
    SUB_Profile       * pSubPro      = NULL;
    ReturnCode		rc           = Success;
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "INVOKE.ind from NSAP %u.%u.%u.%u,"
	      " invokeId = 0x%x, operationValue = %d, sapSel = %d",
	      pRemoteNsap->addr[0], pRemoteNsap->addr[1],
	      pRemoteNsap->addr[2], pRemoteNsap->addr[3],
	      invokeId, operationValue, sapSel));

    /* check for duplicate */
    DUP_invokeIn (hView, operationValue, pRemoteNsap);

    /* Get the device profile for this NSAP address */
    if ((pSubPro = SUB_getEntry (NULL, pRemoteNsap)) == NULL)
    {
	TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
		  "No valid profile for NSAP %u.%u.%u.%u, rc=%d",
		  pRemoteNsap->addr[0], pRemoteNsap->addr[1],
		  pRemoteNsap->addr[2], pRemoteNsap->addr[3], rc));

	/*
	 * This is an unknown device.  Send back an Error Request with
	 * reason Security Error.
	 */
	
	/* assume 1.1 if not otherwise specified */
	TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
		  "Issuing ErrorRequest, InvokeId=%d", invokeId));
	issueErrorRequest(rc, invokeId, 1, 1);
	
	return SUCCESS;
    }
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
	      "Profile for NSAP %u.%u.%u.%u: Version=%d.%d, Subscriber=%s",
	      pSubPro->deviceProfile.nsap.addr[0], pSubPro->deviceProfile.nsap.addr[1],
	      pSubPro->deviceProfile.nsap.addr[2], pSubPro->deviceProfile.nsap.addr[3], 
	      pSubPro->deviceProfile.protocolVersionMajor,
	      pSubPro->deviceProfile.protocolVersionMinor,
	      pSubPro->subscriberId));

    
    /* Convert the DU_View into a BUF_buffer */
    if ((rc = viewToBuf(hView, &hBuf)) != Success)
    {
	/* Couldn't copy view to buffer */
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
		  "Issuing ErrorRequest, InvokeId=%d", invokeId));
	issueErrorRequest(rc, invokeId,
			  pSubPro->deviceProfile.protocolVersionMajor,
			  pSubPro->deviceProfile.protocolVersionMinor);
	return SUCCESS;
    }
    
    TM_CALL(globals.hTM, MTS_TRACE_PDU, BUF_dump, hBuf, "INVOKE.ind data");

    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
	     "Parsing Invoke Indication: opVal=%d, invokeId=%d, Version=%d.%d",
	      operationValue, invokeId, pSubPro->deviceProfile.protocolVersionMajor,
	      pSubPro->deviceProfile.protocolVersionMinor));

    /* Preprocess the request (e.g. parse the buffer, get operation pointer) */
    rc = mts_preprocessInvokeIndication(&pSubPro->deviceProfile, hBuf, operationValue,
				encodingType, invokeId, hSapDesc, sapSel, &pOperation);
    if (rc != Success)
    {
	/* Couldn't parse buffer or some other problem */
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
		  "Could not proprocess INVOKE.ind; Issuing ERROR.req, invokeId=%d, rc=%d",
		  invokeId, rc));

	issueErrorRequest(rc, invokeId, pSubPro->deviceProfile.protocolVersionMajor,
			  pSubPro->deviceProfile.protocolVersionMinor);
	BUF_free(hBuf);
	return SUCCESS;
    }
    
    /* We should be done with our copy of the buffer now. */
    BUF_free(hBuf);
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, MTS_FsmEvent_InvokeIndication);
    
    return Success;
}


int
mts_resultIndication(ESRO_InvokeId invokeId, 
		     ESRO_UserInvokeRef userInvokeRef,
		     ESRO_EncodingType encodingType, 
		     DU_View hView)
{
    MTS_Operation *	pOperation  = NULL;
    ReturnCode 	        rc          = Fail;
    void *		hBuf        = NULL;
    
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
	     "RESULT.ind, invokeId=%d", invokeId));

    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received result indication for unknown invoke id\n");
	return FAIL;
    }
    
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "RESULT.ind, OperationValue=%d",
	      pOperation->operationValue));

    /* See what operation is being requested */
    switch(pOperation->operationValue)
    {
      case EMSD_Operation_Delivery_1_0_WRONG:
      case EMSD_Operation_Delivery_1_1:
	/* No data, so nothing to do */
	break;
	
      default:
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
		  "mts_resultIndication: unrecognized operationValue: %d",
		  pOperation->operationValue));
	/* fall thru */
      case EMSD_Operation_SubmissionVerify_1_1:
      case EMSD_Operation_SubmissionControl_1_1:
	/* Convert the DU_View into a BUF_buffer */
	if ((rc = viewToBuf(hView, &hBuf)) != Success)
	  {
	    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL,
		      "Error converting resultInd data, invokeId=%d", invokeId));
	    /***	return SUCCESS; ***/
	  }
    }
    if (hBuf)
        TM_CALL(globals.hTM, MTS_TRACE_PDU, BUF_dump, hBuf, "RESULT.ind data");

    /* Preprocess the result (e.g. parse the buffer) */
    if ((rc = mts_preprocessResultIndication(pOperation, hBuf)) != Success)
    {
	/* Pretend it never happened */
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
		 "Error parsing resultInd; invokeId=%d", invokeId));
	BUF_free(hBuf);
	return SUCCESS;
    }
    
    /* We should be done with our copy of the buffer now. */
    BUF_free(hBuf);
    
    /* Generate the event to the finite state machine */
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
 	     "Generating resultInd event to FSM; invokeId=%d", invokeId));
    FSM_generateEvent(pOperation->hFsm, MTS_FsmEvent_ResultIndication);
    
    return Success;
}


int
mts_errorIndication(ESRO_InvokeId invokeId, 
		    ESRO_UserInvokeRef userInvokeRef,
		    ESRO_EncodingType encodingType,
		    ESRO_ErrorValue errorValue,
		    DU_View hView)
{
    MTS_Operation *	pOperation;
    ReturnCode		rc;
    void *		hBuf;
    
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "ERROR.ind, val=%d, invokeId=%d",
	      errorValue, invokeId));

    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received error indication for unknown invoke id\n");
	return FAIL;
    }
    
    if (hView) {
      /* Convert the DU_View into a BUF_buffer */
      if ((rc = viewToBuf(hView, &hBuf)) != Success)
	{
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
		    "Error converting errorInd, invokeId=%d", invokeId));
	}
      else
	{
    
	  TM_CALL(globals.hTM, MTS_TRACE_PDU, BUF_dump, hBuf, "ERROR.ind data");

	  /* Preprocess the error (e.g. parse the buffer) */
	  if ((rc = mts_preprocessErrorIndication(pOperation, hBuf)) != Success)
	    {
	      TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
			"Error parsing errorInd, invokeId=%d", invokeId));
	    }
    
	  /* We should be done with our copy of the buffer now. */
	  BUF_free(hBuf);
	}
    }
    TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
	      "ERROR.ind, generating FSM event: %08lx", pOperation->hFsm));

    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, MTS_FsmEvent_ErrorIndication);
    
    return Success;
}


int
mts_resultConfirm(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    MTS_Operation *	pOperation;
    
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
	     "resultConfirm, invokeId=%d", invokeId));

    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received result confirm for unknown invoke id\n");
	return Fail;
    }
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, MTS_FsmEvent_ResultConfirm);
    
    return Success;
}


int
mts_errorConfirm(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    MTS_Operation *	pOperation;
    
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
	     "errorConfirm, invokeId=%d", invokeId));

    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received error confirm for unknown invoke id\n");
	return Fail;
    }
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, MTS_FsmEvent_ErrorConfirm);
    
    return Success;
}


int
mts_failureIndication(ESRO_InvokeId invokeId,
		      ESRO_UserInvokeRef userInvokeRef,
		      ESRO_FailureValue failureValue)
{
    MTS_Operation *	pOperation;
    
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
	     "failureInd, invokeId=%d", invokeId));

    /* Associate the invoke id with an existing operation */
    if ((pOperation = findOperationByInvokeId(invokeId)) == NULL)
    {
	/* No such invoke id. */
	EH_problem("Received failure indication for unknown invoke id\n");
	return Fail;
    }
    
    /* Generate the event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, MTS_FsmEvent_FailureIndication);
    
    return Success;
}


static MTS_Operation *
findOperationByInvokeId(ESRO_InvokeId invokeId)
{
    MTS_Operation *	pOperation;
    
    /* For each operation in our active operations queue... */
    for (pOperation = QU_FIRST(&globals.activeOperations);
	 ! QU_EQUAL(pOperation, &globals.activeOperations);
	 pOperation = QU_NEXT(pOperation))
    {
	/* ... does this one have the specified invoke id? */
	if (pOperation->invokeId == invokeId)
	{
	    /* Yup. */
	    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
		     "Found invokeId=%d in activeOperations list", invokeId));
	    return pOperation;
	}
    }
    
    /*** debug info ***/
    /* For each operation in our active operations queue... */
    {
      char  debugMsg[1024];

      sprintf (debugMsg, "Failed to find invokeId = %d; head/prev/next=(%08x/%08x/%08x), Current list = {",
	       invokeId,
	       (unsigned int) &globals.activeOperations,
	       (unsigned int) QU_FIRST(&globals.activeOperations),
	       (unsigned int) QU_LAST(&globals.activeOperations));

      for (pOperation = QU_FIRST(&globals.activeOperations);
	   ! QU_EQUAL(pOperation, &globals.activeOperations);
	   pOperation = QU_NEXT(pOperation))
	{
	  sprintf (&debugMsg[strlen(debugMsg)], " %d", pOperation->invokeId);
	}
      TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "%s }", debugMsg));
    }
    /*** end of debug info ***/

#ifdef OPER_HACK
    /*** hack for test purposes *** fnh ***/
    pOperation = QU_FIRST(&globals.activeOperations);
    if (!QU_EQUAL (pOperation, &globals.activeOperations))
      {
	TM_TRACE((globals.hTM, MTS_TRACE_WARNING, 
		 "*** HACK *** returning alternate operation"));
	return (pOperation);
      }
    /*** EOH - End-Of-Hack ***/
#endif

    TM_TRACE((globals.hTM, MTS_TRACE_WARNING,
	      "findOperationByInvokeId(invokeId=%d) returning NULL", invokeId));
    return NULL;
}


static void
issueErrorRequest(ReturnCode rc,
		  ESRO_InvokeId invokeId,
		  OS_Uint8 protocolVersionMajor,
		  OS_Uint8 protocolVersionMinor)
{
    EMSD_Error	    errorVal;

    switch(rc)
    {
    case ResourceError:
	errorVal = EMSD_Error_ResourceError;
	break;
	
    case MTS_RC_DeviceProfileNotFound:
    case MTS_RC_MissingProfileAttribute:
	errorVal = EMSD_Error_SecurityError;
	break;

    case MTS_RC_UnrecognizedProtocolVersion:
	errorVal = EMSD_Error_ProtocolVersionNotRecognized;
	break;
	
    default:
	errorVal = EMSD_Error_ProtocolViolation;
	break;
    }
    
    /* Issue the error request */
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
	      "Issuing ErrorRequest, InvokeId=%d, errVal=%d", invokeId, errorVal));
    mts_issueErrorRequest(invokeId, errorVal, 0, protocolVersionMajor, protocolVersionMinor);
}


static ReturnCode
viewToBuf(DU_View hView,
	  void ** phBuf)
{
    unsigned char * p;
    STR_String 	    hStr;
    ReturnCode	    rc;

    if (hView == NULL)
      {
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "viewToBuf: NULL hView"));
	return Fail;
      }
      
    /* Get a pointer to the data in the view */
    p = DU_data(hView);

    /* Allocate an STR_String for this data */
    if ((rc =
	 STR_attachString(DU_size(hView), DU_size(hView), p, FALSE, &hStr)) !=
	Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Error in viewToBuf"));
	return rc;
    }

    /* Allocate a buffer */
    if ((rc = BUF_alloc(0, phBuf)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Error in viewToBuf"));
	STR_free(hStr);
	return rc;
    }

    /* Append the view data to the new buffer */
    if ((rc = BUF_appendChunk(*phBuf, hStr)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Error in viewToBuf"));
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
}
