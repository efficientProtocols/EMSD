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
 *    ua_preprocessInvokeIndication()
 *    ua_preprocessResultIndication()
 *    ua_preprocessErrorIndication()
 *    ua_preprocessSubmitRequest()
 *    ua_preprocessDeliveryControlRequest()
 *    findOperationBySequenceId()
 */


#include "estd.h"
#include "buf.h"
#include "mm.h"
#include "fsm.h"
#include "fsmtrans.h"
#include "esro_cb.h"
#include "emsd.h"
#include "mtsua.h"
#include "uacore.h"
#include "target.h"


FORWARD static UA_Operation *
findOperationBySequenceId(OS_Uint32 sequenceId);



ReturnCode
ua_preprocessInvokeIndication(void * hBuf,
			      ESRO_OperationValue opValue,
			      ESRO_EncodingType encodingType,
			      ESRO_InvokeId invokeId,
			      ESRO_SapDesc hSapDesc,
			      ESRO_SapSel sapSel, 
			      UA_Operation ** ppOperation)
{
    ReturnCode		    rc = Success;
    OS_Uint32 		    operationValue;
    OS_Uint32		    pduLength;
    void *		    hOperationData = NULL;
    EMSD_DeliverArgument *   pDeliverArg;
    UA_Operation *	    pOperation = NULL;
    UA_Pending *	    pPending;
    QU_Head *		    pCompTree;
    void *		    hExtraBuf = NULL;
    void		 (* pfFree)(void * p) = NULL;
    void		 (* pfExtraFree)(void * p) = NULL;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Preprocess Invoke Indication..."));

    /*
     * Create our internal rendition of the operation value, which
     * includes the protocol version number in the high-order 16 bits.
     */
    operationValue =
	EMSD_OP_PROTO(opValue,
		     UA_PROTOCOL_VERSION_MAJOR,
		     UA_PROTOCOL_VERSION_MINOR);
    
    /* See what operation is being requested */
    switch(operationValue)
    {
    case EMSD_Operation_Delivery_1_1:
	TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
		  "Invoke Indication contains Deliver operation"));

	/* Allocate an EMSD Deliver Argument structure */
	if ((rc =
	     mtsua_allocDeliverArg((EMSD_DeliverArgument **)
				   &hOperationData)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "mtsua_allocDeliverArg() failed, reason 0x%x", rc));
	    return rc;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeDeliverArg;
	
	/* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliverArg);
	
	/* Parse the EMSD PDU */
	if ((rc = ASN_parse(encodingType,
			    pCompTree,
			    hBuf, hOperationData,
			    &pduLength)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ASN_parse() failed, reason 0x%x", rc));
	    goto cleanUp;
	}
	
	/* Look at the segmentation structure of this Deliver Arg */
	pDeliverArg = hOperationData;
	
	/* We have something extra that'll need to be freed */
	hExtraBuf = pDeliverArg->hContents;
	pfExtraFree = BUF_free;

	/* Is this a part of a segmented message? */
	if ((pDeliverArg->segmentInfo.exists) &&
	    (pDeliverArg->segmentInfo.data.choice ==
	     EMSD_SEGMENTINFO_CHOICE_OTHER))
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		     "Segment A%ld",
		     pDeliverArg->segmentInfo.data.segmentData.sequenceId));

	    /* Yup.  See if we can find an operation to associate it with */
	    if ((pOperation =
		 findOperationBySequenceId(pDeliverArg->segmentInfo.data.
					   segmentData.sequenceId)) ==
		NULL)
	    {
		TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
			  "Could not find operation for sequence id %lu",
			  pDeliverArg->segmentInfo.data.
			  segmentData.sequenceId));
		BUF_free(pDeliverArg->hContents);
		goto cleanUp;
	    }
	}
	
	break;
	
    case EMSD_Operation_SubmissionVerify_1_1:
	TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
		  "Invoke Indication contains Submission Verify operation"));

	/* Allocate an EMSD Submission Verify Argument structure */
	if ((rc =
	     mtsua_allocSubmissionVerifyArg((EMSD_SubmissionVerifyArgument ** )
					    &hOperationData)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "mtsua_allocSubmissionVerifyArg() failed, reason 0x%x",
		      rc));
	    goto cleanUp;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeSubmissionVerifyArg;
	
	/* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = UA_getProtocolTree(MTSUA_Protocol_SubmissionVerifyArg);
	
	/* Parse the EMSD PDU */
	if ((rc = ASN_parse(encodingType,
			    pCompTree,
			    hBuf, hOperationData,
			    &pduLength)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ASN_parse() failed, reason 0x%x", rc));

	    goto cleanUp;
	}
	
	break;
	
    case EMSD_Operation_SubmissionControl_1_1:
    default:
	TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
		  "Invoke Indication contains unsupported operation %ld",
		  operationValue));

	return Fail;
    }
    
    /* If we didn't find an operation to associate with... */
    if (pOperation == NULL)
    {
	/* ... then allocate a new one */
	if ((rc =
	     ua_allocOperation(&pOperation,
			       &ua_globals.concurrentOperationsInbound,
			       (void *)"")) !=
	    Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ua_allocOperation() failed, reason 0x%x", rc));

	    goto cleanUp;
	}
	
	/* Create a new finite state machine */
	if ((pOperation->hFsm = FSM_createMachine(pOperation)) == NULL)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "FSM_createMachine() failed."));

	    ua_freeOperation(pOperation);
	    goto cleanUp;
	}
	
	/* Arrange for finite state machine to be freed */
	if ((rc = ua_operationAddUserFree(pOperation,
					  (void (*)(void *)) FSM_deleteMachine,
					  pOperation->hFsm)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ua_operationAddUserFree() failed, reason 0x%x", rc));

	    FSM_deleteMachine(pOperation->hFsm);
	    ua_freeOperation(pOperation);
	    return rc;
	}
    
	/* Load the transition diagram into this machine */
	FSM_TRANSDIAG_load(pOperation->hFsm, ua_globals.hPerformerTransDiag);
	
	/* Initialize this state machine */
	FSM_TRANSDIAG_resetMachine(pOperation->hFsm);
    }
    
    /* Allocate a Pending Operation structure */
    if ((pPending = OS_alloc(sizeof(UA_Pending))) == NULL)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "Could not allocate memory for Pending Operation\n"));
	ua_freeOperation(pOperation);
	goto cleanUp;
    }

    /* Initialize the queue pointers */
    QU_INIT(pPending);

    /* Set operation-specific values */
    pPending->operationValue = operationValue;
    pPending->encodingType   = encodingType;
    pPending->invokeId 	     = invokeId;
    pPending->hSapDesc       = hSapDesc;
    pPending->sapSel         = sapSel;
    pPending->hOperationData = hOperationData;
    
    /* Put this pending operation on the queue of pending operations */
    QU_INSERT(pPending, &pOperation->pendingQHead);

    /* Arrange for the operation data to be freed with the operation */
    if ((pOperation->rc =
	 ua_operationAddUserFree(pOperation,
				 pfFree, hOperationData)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree() failed, reason 0x%x", rc));

	ua_freeOperation(pOperation);
	goto cleanUp;
    }
    
    /* If there's something extra to be freed... */
    if (hExtraBuf != NULL)
    {
	/* ... then arrange for it to be freed. */
	if ((pOperation->rc =
	     ua_operationAddUserFree(pOperation,
				     pfExtraFree, hExtraBuf)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ua_operationAddUserFree() failed, reason 0x%x", rc));
	    
	    goto cleanUp;
	}
    }
    
    /* Give 'em what they came for */
    *ppOperation = pOperation;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "UA Invoke Indication preprocessed successfully"));

    return Success;
    
  cleanUp:
    if (hOperationData != NULL)
    {
	(* pfFree)(hOperationData);
    }
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "UA Invoke Indication preprocess failed"));

    return Fail;

} /* ua_preprocessInvokeIndication() */


ReturnCode
ua_preprocessResultIndication(UA_Operation * pOperation,
			      void * hBuf)
{
    ReturnCode 			rc;
    OS_Uint32			pduLength;
    void *			hOperationData = NULL;
    QU_Head *			pCompTree;
    void		     (* pfFree)(void * p) = NULL;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Preprocess Result Indication..."));
        
    /* See what operation is being requested */
    switch(pOperation->operationValue)
    {
    case EMSD_Operation_Submission_1_0_WRONG:
    case EMSD_Operation_Submission_1_1:
	TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
		  "Result Indication contains Submit operation"));

	/* Allocate an EMSD Submit Result structure */
	if ((rc =
	     mtsua_allocSubmitRes((EMSD_SubmitResult ** ) &hOperationData)) !=
	    Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "mtsua_allocSubmitRes() failed, reason 0x%x", rc));

	    return rc;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeSubmitRes;
	
	/* Arrange for the operation data to be freed with the operation */
	if ((rc = ua_operationAddUserFree(pOperation,
					  pfFree,
					  hOperationData)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ua_operationAddUserFree() failed, reason 0x%x", rc));

	    goto cleanUp;
	}
	
	/* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = UA_getProtocolTree(MTSUA_Protocol_SubmitResult);
	
	/* Parse the EMSD PDU */
	if ((rc = ASN_parse(pOperation->encodingType,
			    pCompTree,
			    hBuf, hOperationData,
			    &pduLength)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ASN_parse() failed, reason 0x%x", rc));

	    goto cleanUp;
	}
	
	break;
	
    case EMSD_Operation_DeliveryVerify_1_1:
	TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
		  "Result Indication contains DeliveryVerify operation"));

	/* Allocate an EMSD Delivery Verify Result structure */
	if ((rc =
	     mtsua_allocDeliveryVerifyRes((EMSD_DeliveryVerifyResult ** )
					  &hOperationData)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "mtsua_allocDeliveryVerifyRes() failed, reason 0x%x",
		      rc));

	    return rc;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeDeliveryVerifyRes;
	
	/* Arrange for the operation data to be freed with the operation */
	if ((rc = ua_operationAddUserFree(pOperation,
					  pfFree,
					  hOperationData)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ua_operationAddUserFree() failed, reason 0x%x", rc));

	    goto cleanUp;
	}
	
	/* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliveryVerifyRes);
	
	/* Parse the EMSD PDU */
	if ((rc = ASN_parse(pOperation->encodingType,
			    pCompTree,
			    hBuf, hOperationData,
			    &pduLength)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "ASN_parse() failed, reason 0x%x", rc));

	    goto cleanUp;
	}
	
	break;
	
    case EMSD_Operation_DeliveryControl_1_1:
	break;

    default:
	TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
		  "Result Indication contains unknown operation %lu",
		  pOperation->operationValue));

	return Fail;
    }
    
    /* Give 'em the current operation data */
    pOperation->hOperationData = hOperationData;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "UA Result Indication preprocessed successfully"));

    return Success;
    
  cleanUp:
    if (hOperationData != NULL)
    {
	(* pfFree)(hOperationData);
    }
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "UA Result Indication preprocess failed"));

    return Fail;

} /* ua_preprocessResultIndication() */


ReturnCode
ua_preprocessErrorIndication(UA_Operation * pOperation,
			     void * hBuf)
{
    ReturnCode 			rc;
    OS_Uint32			pduLength;
    void *			hOperationData = NULL;
    QU_Head *			pCompTree;
    void		     (* pfFree)(void * p) = NULL;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Preprocess Error Indication..."));
        
#if defined(OS_VARIANT_WinCE)
    /* hack to allow acknowledgement of delivery control
     * request (see documentation of the two extern variables
     * for more info) */
    {
	extern OS_Boolean	g_delCtrlWaitingMtsAck;
	extern OS_Boolean	g_delCtrlMtsAckRecvd;

	if ( g_delCtrlWaitingMtsAck == TRUE )
	    g_delCtrlMtsAckRecvd = TRUE;
    }
#endif

    /* Allocate an EMSD Error Request structure */
    if ((rc = mtsua_allocErrorRequest((EMSD_ErrorRequest ** )
				      &hOperationData)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "mtsua_allocErrorRequest() failed, reason 0x%x", rc));

	return rc;
    }
    
    /* Point to the function to free this structure */
    pfFree =(void (*)(void *))  mtsua_freeErrorRequest;
    
    /* Arrange for the operation data to be freed with the operation */
    if ((rc = ua_operationAddUserFree(pOperation,
				      pfFree, hOperationData)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree() failed, reason 0x%x", rc));

	goto cleanUp;
    }
    
    /* Specify the maximum parse length */
    pduLength = BUF_getBufferLength(hBuf);
    
    /* Determine which protocol tree to use */
    pCompTree = UA_getProtocolTree(MTSUA_Protocol_ErrorRequest);
    
    /* Parse the EMSD PDU */
    if ((rc = ASN_parse(pOperation->encodingType,
			pCompTree,
			hBuf, hOperationData,
			&pduLength)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ASN_parse() failed, reason 0x%x", rc));

	goto cleanUp;
    }
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "UA Error Indication preprocessed successfully"));

    /* Give 'em the current operation data */
    pOperation->hOperationData = hOperationData;

    return Success;
    
  cleanUp:
    if (hOperationData != NULL)
    {
	(* pfFree)(hOperationData);
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "UA Error Indication preprocess failed"));

    return Fail;

} /* ua_preprocessErrorIndication() */


ReturnCode
ua_preprocessSubmitRequest(UA_Operation * pOperation,
			   UASH_MessageHeading * pHeading,
			   char * pBodyText)
{
    ReturnCode 			rc;
    OS_Uint32			len;
    EMSD_SubmitArgument *	pEmsdSubmitArg;
    QU_Head *			pCompTree;
    EMSD_Message *		pIpm;
    EMSD_AsciiString * 		phString;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Preprocess Submit Request..."));
        
    /* Convert the message to EMSD IPM format */
    if ((rc = ua_messageFromShell(pHeading, pBodyText, &pIpm)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_messageFromShell() failed, reason 0x%x", rc));

	return rc;
    }
    
    /* Arrange for the message contents to be freed */
    if ((rc = ua_operationAddUserFree(pOperation,
				      (void (*)(void *)) mtsua_freeMessage,
				       pIpm)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree(pIpm) failed, reason 0x%x", rc));

	mtsua_freeMessage(pIpm);
	return rc;
    }
    
    /* Allocate a buffer to begin formatting the message */
    if ((rc = BUF_alloc(0, &pOperation->hFullPduBuf)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "BUF_alloc() failed, reason 0x%x", rc));

	return rc;
    }
    
    /* Arrange for the segment buffer to be freed */
    if ((rc = ua_operationAddUserFree(pOperation,
				      BUF_free,
				      pOperation->hFullPduBuf)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree(hFullPduBuf) failed, reason 0x%x",
		  rc));

	BUF_free(pOperation->hFullPduBuf);
	return rc;
    }
    
    /* Determine the ASN compile tree to use for this PDU */
    pCompTree = UA_getProtocolTree(MTSUA_Protocol_Ipm);
    
    /* Format the message */
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 pOperation->hFullPduBuf,
			 pIpm,
			 &len)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ASN_format() failed, reason 0x%x", rc));

	return rc;
    }
    
    /* Allocate an EMSD Submit Argument structure */
    if ((rc = mtsua_allocSubmitArg(&pEmsdSubmitArg)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "mtsua_allocSubmitArg() failed, reason 0x%x", rc));

	return rc;
    }
    
    /* Arrange for the Submit Argument to be freed later. */
    if ((rc =
	 ua_operationAddUserFree(pOperation,
				 (void (*)(void *)) mtsua_freeSubmitArg,
				 pEmsdSubmitArg)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree(pEmsdSubmitArg) failed, reason 0x%x",
		  rc));

	mtsua_freeSubmitArg(pEmsdSubmitArg);
	return rc;
    }
    
    /* Get the point-to-point envelope fields */
    pEmsdSubmitArg->contentType = EMSD_ContentType_Ipm95;
    pEmsdSubmitArg->hContents = pIpm;
    
    /* Was a password specified? */
    if (ua_globals.pPassword != NULL)
    {
	/* Yup.  Add a security element. */
	pEmsdSubmitArg->security.exists = TRUE;

	/* No address required on submission.  It should use originator. */
	pEmsdSubmitArg->security.data.credentials.simple.emsdAddress.exists =
	    FALSE;

	/* Add the password */
	pEmsdSubmitArg->security.data.credentials.simple.password.exists = TRUE;

	/* Point to the AsciiString handle */
	phString =
	    &pEmsdSubmitArg->security.data.credentials.simple.password.data;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the in-reply-to string to it */
	if ((rc = STR_assignZString(phString,
				    ua_globals.pPassword)) != Success)
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		      "Could not assign password string for submission,"
		      "reason 0x%x", rc));
	    return rc;
	}
    }

    /* Save the Submit Argument */
    pOperation->hFirstOperationData = pEmsdSubmitArg;
    
    /* Create a new finite state machine */
    if ((pOperation->hFsm = FSM_createMachine(pOperation)) == NULL)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "FSM_createMachine() failed, reason 0x%x", rc));

	return ResourceError;
    }
    
    /* Arrange for finite state machine to be freed */
    if ((rc = ua_operationAddUserFree(pOperation,
				      (void (*)(void *)) FSM_deleteMachine,
				      pOperation->hFsm)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree(hFsm) failed, reason 0x%x", rc));

	FSM_deleteMachine(pOperation->hFsm);
	return rc;
    }
    
    /* Load the transition diagram into this machine */
    FSM_TRANSDIAG_load(pOperation->hFsm, ua_globals.hSubmitTransDiag);
    
    /* Initialize this state machine */
    FSM_TRANSDIAG_resetMachine(pOperation->hFsm);
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "UA Submit Request preprocessed successfully"));

    return Success;

} /* ua_preprocessSubmitRequest() */


#ifdef DELIVERY_CONTROL
ReturnCode
ua_preprocessDeliveryControlRequest(UA_Operation * pOperation)
{
    ReturnCode 			rc;
    OS_Uint32			len;
    EMSD_DeliveryControlArgument *	pEmsdDeliveryControlArg;
    QU_Head *			pCompTree;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Preprocess Delivery Control Request..."));

    /* Allocate an EMSD Delivery Control Argument structure */
    if ((rc = mtsua_allocDeliveryControlArg(&pEmsdDeliveryControlArg)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "mtsua_allocDeliveryControlArg() failed, reason 0x%x", rc));

	return rc;
    }
    

    pEmsdDeliveryControlArg->restrict.exists = FALSE;
    pEmsdDeliveryControlArg->restrict.data = (OS_Uint32)0;
    pEmsdDeliveryControlArg->permissibleOperations.exists = FALSE;
    pEmsdDeliveryControlArg->permissibleOperations.data = (ASN_BitString)"";
    pEmsdDeliveryControlArg->permissibleMaxContentLength.exists = FALSE;
    pEmsdDeliveryControlArg->permissibleMaxContentLength.data = (OS_Uint32)0;
    pEmsdDeliveryControlArg->permissibleLowestPriority.exists = FALSE;
    pEmsdDeliveryControlArg->permissibleLowestPriority.data = (OS_Uint32)0;
    pEmsdDeliveryControlArg->security.exists = FALSE;
/*    pEmsdDeliveryControlArg->security.data = ; */
    pEmsdDeliveryControlArg->userFeatures.exists = FALSE;
    pEmsdDeliveryControlArg->userFeatures.data = (STR_String)"";
/*    pEmsdDeliveryControlArg->userFeatures.data = pOperation->;*/

    /* Allocate a buffer to begin formatting the operation */
    if ((rc = BUF_alloc(0, &pOperation->hFullPduBuf)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "BUF_alloc() failed, reason 0x%x", rc));

	return rc;
    }
    
    /* Determine the ASN compile tree to use for this PDU */
    pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliveryControlArg);
    
    /* Format the message */
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 pOperation->hFullPduBuf,
    			 pEmsdDeliveryControlArg,
			 &len)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ASN_format() failed, reason 0x%x", rc));

	return rc;
    }
    
    /* Arrange for the Delivery Control Argument to be freed later. */
    if ((rc =
	 ua_operationAddUserFree(pOperation,
				 (void (*)(void *)) mtsua_freeDeliveryControlArg,
				 pEmsdDeliveryControlArg)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree(pEmsdDeliveryControlArg) failed, "
		  "reason 0x%x",
		  rc));

	mtsua_freeDeliveryControlArg(pEmsdDeliveryControlArg);
	return rc;
    }
    
    /* Save the Delivery Control Argument */
    pOperation->hFirstOperationData = pEmsdDeliveryControlArg;

    /* Create a new finite state machine */
    if ((pOperation->hFsm = FSM_createMachine(pOperation)) == NULL)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "FSM_createMachine() failed, reason 0x%x", rc));

	return ResourceError;
    }
    
    /* Arrange for finite state machine to be freed */
    if ((rc = ua_operationAddUserFree(pOperation,
				      (void (*)(void *)) FSM_deleteMachine,
				      pOperation->hFsm)) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ERROR,
		  "ua_operationAddUserFree(hFsm) failed, reason 0x%x", rc));

	FSM_deleteMachine(pOperation->hFsm);
	return rc;
    }
    
    /* Load the transition diagram into this machine */
    FSM_TRANSDIAG_load(pOperation->hFsm, ua_globals.hDeliveryControlTransDiag);
    
    /* Initialize this state machine */
    FSM_TRANSDIAG_resetMachine(pOperation->hFsm);
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "UA Delivery COntrol Request preprocessed successfully"));
    
    return Success;
    
}
#endif

static UA_Operation *
findOperationBySequenceId(OS_Uint32 sequenceId)
{
    UA_Operation *	pOperation;
    
    /* For each operation in our active operations queue... */
    for (pOperation = QU_FIRST(&ua_globals.activeOperations);
	 ! QU_EQUAL(pOperation, &ua_globals.activeOperations);
	 pOperation = QU_NEXT(pOperation))
    {
	/* ... does this one have the specified sequence id? */
	if (pOperation->sequenceId == sequenceId)
	{
	    /* Yup. */
	    return pOperation;
	}
    }
    
    return NULL;

} /* findOperationBySequenceId() */


