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

static char *rcs = "$Id: mpreproc.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";


#include "estd.h"
#include "buf.h"
#include "mm.h"
#include "fsm.h"
#include "fsmtrans.h"
#include "emsd.h"
#include "mtsua.h"
#include "mts.h"
#include "subpro.h"
#include "msgstore.h"



FORWARD static MTS_Operation *
findOperationBySequenceId(char * pNsap,
			  OS_Uint32 sequenceId);



ReturnCode
mts_preprocessInvokeIndication(SUB_DeviceProfile    * pProfile,
			       void                 * hBuf,
			       ESRO_OperationValue    opValue,
			       ESRO_EncodingType      encodingType,
			       ESRO_InvokeId          invokeId,
			       ESRO_SapDesc           hSapDesc,
			       ESRO_SapSel            sapSel, 
			       MTS_Operation       ** ppOperation)
{
    ReturnCode		    rc                 = Success;
    OS_Uint32 		    operationValue     = 0;
    OS_Uint32		    pduLength          = 0;
    void *		    hOperationData     = NULL;
    EMSD_SubmitArgument *    pSubmitArg         = NULL;
    MTS_Operation *	    pOperation         = NULL;
    QU_Head *		    pCompTree          = NULL;
    void		 (* pfFree)(void * p)  = NULL;
    char                    nsapString[32];
    
    /*
     * Create our internal rendition of the operation value, which
     * includes the protocol version number in the high-order 16 bits.
     */
    operationValue = EMSD_OP_PROTO(opValue, pProfile->protocolVersionMajor,
				  pProfile->protocolVersionMinor);
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
	     "Preprocessing invokeId=%d, operVal=%u", invokeId, operationValue));

    
    sprintf (nsapString, "%d.%d.%d.%d", pProfile->nsap.addr[0], pProfile->nsap.addr[1],
	     pProfile->nsap.addr[2], pProfile->nsap.addr[3]);

    /*** reschedule deliveries to this NSAP ***/
    TM_TRACE((globals.hTM, MTS_TRACE_WARNING, 
	     "Rescheduling deliveries to NSAP [%s]", nsapString));
    msg_deliverNow (&pProfile->nsap);

    /* See what operation is being requested */
    switch(operationValue)
    {
    case EMSD_Operation_Submission_1_1:
	/* Allocate an EMSD Submit Argument structure */
	rc = mtsua_allocSubmitArg((EMSD_SubmitArgument **) &hOperationData);
	if (rc != Success)
	{
	    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Cannot allocate Submit Argument"));
	    return rc;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeSubmitArg;
	
	/* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_SubmitArg,
					  pProfile->protocolVersionMajor,
					  pProfile->protocolVersionMinor);
	if (pCompTree == NULL) {
	    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Cannot get parse tree for SubmitArg; v%d.%d",
		       pProfile->protocolVersionMajor, pProfile->protocolVersionMinor));
	    goto cleanUp;
	}	  
	
	/* Parse the EMSD PDU */
	rc = ASN_parse(encodingType, pCompTree, hBuf, hOperationData, &pduLength);
	if (rc != Success)
	{
	    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Cannot parse Submit invoke"));
	    MM_logMessage(globals.hMMLog, globals.tmpbuf,
			  "MTS Received SUBMIT invoke from NSAP [%s]; Parse error", nsapString);

	    goto cleanUp;
	}
	
	/* Look at the segmentation structure of this Submit Arg */
	pSubmitArg = hOperationData;
	
	/* Is this a part of a segmented message? */
	if ((pSubmitArg->segmentInfo.exists) &&
	    (pSubmitArg->segmentInfo.data.choice == EMSD_SEGMENTINFO_CHOICE_OTHER))
	{

	    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Segment %ld, NSAP %s",
		      pSubmitArg->segmentInfo.data.segmentData.sequenceId, nsapString));
		      
	    /* Yup.  See if we can find an operation to associate it with */
	    pOperation =
	      findOperationBySequenceId(nsapString, pSubmitArg->segmentInfo.data.segmentData.sequenceId);
	    if (pOperation == NULL)
	    {
	        TM_TRACE((globals.hTM, MTS_TRACE_ERROR,
			  "findOperationBySequenceId(%s) failed, Segment seqId = %ld",
			  nsapString,
			  pSubmitArg->segmentInfo.data.segmentData.sequenceId));
		MM_logMessage(globals.hMMLog, globals.tmpbuf,
			      "MTS Received unassociable segment from NSAP [%s];\n",
			      nsapString);
		goto cleanUp;
		
	    }
	}
	TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Parsed Invocation"));
	
	break;
	
    case EMSD_Operation_DeliveryControl_1_1:
	/* Allocate an EMSD Delivery Control Argument structure */
	rc = mtsua_allocDeliveryControlArg((EMSD_DeliveryControlArgument ** ) &hOperationData);
	if (rc != Success)
	{
	    TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "Error allocating for DeliveryControl PDU"));
	    return rc;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeDeliveryControlArg;
	
	/* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_DeliveryControlArg,
					  pProfile->protocolVersionMajor,
					  pProfile->protocolVersionMinor);
	
	/* Parse the EMSD PDU */
	rc = ASN_parse(encodingType, pCompTree, hBuf, hOperationData, &pduLength);
        if (rc != Success)
	{
	    TM_TRACE((globals.hTM, MTS_TRACE_WARNING,
		      "Error parsing DeliveryControl PDU [%s]", nsapString));
	    MM_logMessage(globals.hMMLog, globals.tmpbuf,
			  "MTS Received DELIVERY CONTROL from NSAP [%s]; Parse error",
			  nsapString);
	    goto cleanUp;
	}
	
	break;
	
    case EMSD_Operation_DeliveryVerify_1_1:
	/* Allocate an EMSD Delivery Verify Argument structure */
	rc = mtsua_allocDeliveryVerifyArg((EMSD_DeliveryVerifyArgument ** ) &hOperationData);
	if (rc != Success)
	{
	    goto cleanUp;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeDeliveryVerifyArg;
	
	/* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_DeliveryVerifyArg,
					  pProfile->protocolVersionMajor,
					  pProfile->protocolVersionMinor);
	
	/* Parse the EMSD PDU */
	if ((rc = ASN_parse(encodingType,
			    pCompTree,
			    hBuf, hOperationData,
			    &pduLength)) != Success)
	{
	    MM_logMessage(globals.hMMLog, globals.tmpbuf,
			  "MTS Received DELIVERY VERIFY indication from NSAP [%s]; Parse error",
			  nsapString);
	    goto cleanUp;
	}
	
	break;
	
    default:
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Unknown operation value = %lX", operationValue));
	MM_logMessage(globals.hMMLog, globals.tmpbuf,
		      "MTS Received unexpected operation value 0x%lx from NSAP [%s]\n",
		      operationValue, nsapString);
	return Fail;
    }    
    /* If we didn't find an operation to associate with... */
    if (pOperation == NULL)
    {
	/* ... then allocate a new one */
	if ((rc = mts_allocOperation(&pOperation)) != Success)
	{
            TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "Operation allocation error"));
	    goto cleanUp;
	}
	
	/* Initialize the new operation */
/***	pOperation->pProfile = pProfile; ***/
	memcpy (&pOperation->nsap, &pProfile->nsap, sizeof pOperation->nsap);
	
	/* Create a new finite state machine */
	if ((pOperation->hFsm = FSM_createMachine(pOperation)) == NULL)
	{
            TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "FSM creation error"));
	    mts_freeOperation(pOperation);
	    goto cleanUp;
	}
	/* Arrange for finite state machine to be freed */
	rc = mts_operationAddUserFree(pOperation, (void (*)(void *))FSM_deleteMachine, pOperation->hFsm);
	if (rc != Success)
	{
	    TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "Freelist error"));
	    mts_freeOperation(pOperation);
	    FSM_deleteMachine(pOperation->hFsm);
	    return rc;
	}
	
	/* Load the transition diagram into this machine */
	FSM_TRANSDIAG_load(pOperation->hFsm, globals.hPerformerTransDiag);
	
	/* Initialize this state machine */
	FSM_TRANSDIAG_resetMachine(pOperation->hFsm);
    }
    
    /* Set operation-specific values */
    pOperation->operationValue = operationValue;
    pOperation->encodingType = encodingType;
    pOperation->invokeId = invokeId;
    pOperation->hSapDesc = hSapDesc;
    pOperation->sapSel = sapSel;

    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
	     "invokeId=%d assigned to Operation {%ld}",
	     pOperation->invokeId, pOperation->operId));

    /* Arrange for the operation data to be freed with the operation */
    pOperation->rc = mts_operationAddUserFree(pOperation, pfFree, hOperationData);
    if (pOperation->rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "Freelist error"));
	mts_freeOperation(pOperation);
	goto cleanUp;
    }
    
    /* Give 'em the current operation data */
    pOperation->hOperationData = hOperationData;
    
    /* Give 'em what they came for */
    *ppOperation = pOperation;
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Successful preprocessing of invocation"));
    return Success;
    
  cleanUp:
    if (hOperationData != NULL)
    {
	(* pfFree)(hOperationData);
    }
    TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "Preprocessing of invocation failed"));

    return Fail;
}


ReturnCode
mts_preprocessResultIndication(MTS_Operation * pOperation,
			       void * hBuf)
{
    ReturnCode 			rc                = Success;
    OS_Uint32			pduLength         = 0;
    SUB_Profile *		pSubPro           = NULL;
    SUB_DeviceProfile *		pProfile          = NULL;
    void *			hOperationData    = NULL;
    QU_Head *			pCompTree         = NULL;
    void		     (* pfFree)(void * p) = NULL;
    
    /* Get the device profile for this NSAP address */
    if ((pSubPro = SUB_getEntry (NULL, &pOperation->nsap)) == NULL) {
      TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "No subscriber for NSAP"));
      return FALSE;
    } else {
      pProfile = &pSubPro->deviceProfile;
    }      

    /* See what operation is being requested */
    switch(pOperation->operationValue)
    {
    case EMSD_Operation_Delivery_1_0_WRONG:
    case EMSD_Operation_Delivery_1_1:
	/* No data, so nothing to do */
	break;
	
    case EMSD_Operation_SubmissionVerify_1_1:
	/* Allocate an EMSD Submit Verify Result structure */
	if ((rc =
	     mtsua_allocSubmissionVerifyRes((EMSD_SubmissionVerifyResult ** )
					    &hOperationData)) != Success)
	{
	    return rc;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeSubmissionVerifyRes;
	
	/* Arrange for the operation data to be freed with the operation */
	if ((rc = mts_operationAddUserFree(pOperation,
					   pfFree,
					   hOperationData)) != Success)
	{
	    goto cleanUp;
	}
	
	/* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_SubmissionVerifyRes,
					  pProfile->protocolVersionMajor,
					  pProfile->protocolVersionMinor);
	
	/* Parse the EMSD PDU */
	if ((rc = ASN_parse(pOperation->encodingType,
			    pCompTree,
			    hBuf, hOperationData,
			    &pduLength)) != Success)
	{
	    MM_logMessage(globals.hMMLog, globals.tmpbuf,
			  "MTS Received SUBMIT result; Parse error; operId={%d}\n",
			  pOperation->operId);
	    goto cleanUp;
	}
	
	break;
	
    case EMSD_Operation_SubmissionControl_1_1:
    default:
	MM_logMessage(globals.hMMLog, globals.tmpbuf,
		      "MTS Received unexpected operation value %d\n",
		      pOperation->operationValue);
	return Fail;
    }
    
    /* Give 'em the current operation data */
    pOperation->hOperationData = hOperationData;
    
    return Success;
    
  cleanUp:
    if (hOperationData != NULL)
    {
	(* pfFree)(hOperationData);
    }
    
    return Fail;
}


ReturnCode
mts_preprocessErrorIndication(MTS_Operation * pOperation,
			      void * hBuf)
{
    ReturnCode 			rc                = Success;
    OS_Uint32			pduLength         = 0;
    SUB_Profile *	        pSubPro           = NULL;
    SUB_DeviceProfile *	        pProfile          = NULL;
    void *			hOperationData    = NULL;
    QU_Head *			pCompTree         = NULL;
    void		     (* pfFree)(void * p) = NULL;
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Preprocessing ERROR.ind"));

    /* Get the device profile for this NSAP address */
    if ((pSubPro = SUB_getEntry (NULL, &pOperation->nsap)) == NULL) {
      TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "No subscriber for NSAP"));
      return FALSE;
    } else {
      pProfile = &pSubPro->deviceProfile;
    }      

    /* Allocate an EMSD Error Request structure */
    rc = mtsua_allocErrorRequest((EMSD_ErrorRequest ** ) &hOperationData);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Could not allocate ErrorRequest"));
	return rc;
    }
    
    /* Point to the function to free this structure */
    pfFree =(void (*)(void *))  mtsua_freeErrorRequest;
    
    /* Arrange for the operation data to be freed with the operation */
    rc = mts_operationAddUserFree(pOperation, pfFree, hOperationData);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
		 "AddUserFree error for ErrorRequest"));
	goto cleanUp;
    }
    
    /* Specify the maximum parse length */
    pduLength = BUF_getBufferLength(hBuf);
    
    /* Determine which protocol tree to use */
    pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_ErrorRequest,
				      pProfile->protocolVersionMajor,
				      pProfile->protocolVersionMinor);
    
    /* Parse the EMSD PDU */
    rc = ASN_parse(pOperation->encodingType, pCompTree, hBuf, hOperationData, &pduLength);
    if (rc != Success)
    {
	MM_logMessage(globals.hMMLog, globals.tmpbuf,
		      "MTS Received ERROR indication; Parse error [%d]\n", rc);
	goto cleanUp;
    }
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
	      "ErrorRequest: hOperationData=%08lx (should not be NULL)", hOperationData));
    
    return Success;
    
  cleanUp:
    if (hOperationData != NULL)
    {
	(* pfFree)(hOperationData);
    }

    return Fail;
}


ReturnCode
mts_preprocessDeliverRequest(MTS_Operation * pOperation)
{
    ReturnCode 			rc                   = 0;
    OS_Uint32			len                  = 0;
    char *			ptr                  = NULL;
    char * 			pBodyText            = NULL;
    char * 			pHeaderText          = NULL;
    MAILSTREAM *		pRfcMessage          = NIL;
    ENVELOPE * 			pRfcEnv              = NULL;
    EMSD_ContentType		contentType;
    EMSD_DeliverArgument *	pEmsdDeliverArg       = NULL;
    QU_Head *			pCompTree            = NULL;
    void *			pContent             = NULL;
    void		        (* pfFree)(void * p) = NULL;
    struct DQ_RecipQueue *      pRecipQueue          = NULL;
    SUB_DeviceProfile *         pDev                 = NULL;


    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Preproccessing delivery request {%d}", pOperation->operId));

    pRecipQueue = pOperation->pControlData->pRecipQueue;
    pDev = &pRecipQueue->deviceProfile;

    if (pDev->protocolVersionMajor < 1) {
      TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Protocol version not set, using v1.1"));
      pDev->protocolVersionMajor = 1;
      pDev->protocolVersionMinor = 1;
    }     
    /* Open the RFC-822 message */
    pRfcMessage = mail_open(pRfcMessage, pOperation->pControlData->dataFileName,
			    NIL, globals.config.pLocalHostName);
    if (pRfcMessage == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error processing data file '%s'",
		  pOperation->pControlData->dataFileName));
	return Fail;
    }
    
    /*
     * Arrange for the message to be closed when we're done delivering.
     *
     * PROGRAMMER NOTE:
     *
     *         It would appear that closing the rfc-822 mail message
     *         could be done in this function.  However, the body text
     *         will be "attached" with STR_stringAttach() to the
     *         buffer.  If you close the message here, then the memory
     *         containing the body text will be freed, and the invoke
     *         will fail due to invalid pointers.
     */
    rc = mts_operationAddUserFree(pOperation, (void (*)(void *)) mail_close, pRfcMessage);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error adding RFC content to free queue"));
	mail_close(pRfcMessage);
	return rc;
    }
    
    /* Retrieve the RFC-822 Envelope */
    if ((pRfcEnv = mail_fetchstructure(pRfcMessage, 1, NULL)) == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error fetching RFC envelope"));
	return Fail;
    }
    
    /* Get a pointer to the RFC-822 header text */
    ptr = mail_fetchheader(pRfcMessage, 1);
    
    /* Copy header, since the same buffer is returned by fetchtext */
    if ((pHeaderText = OS_allocStringCopy(ptr)) == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error allocating RFC header"));
	return ResourceError;
    }
    
    /* Arrange for the header text to be freed */
    if ((rc = mts_operationAddUserFree(pOperation, mtsua_osFree, pHeaderText)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error adding RFC header to free-list"));
	OS_free(pHeaderText);
	return rc;
    }
    
    /* Get a pointer to the body text */
    pBodyText = mail_fetchtext(pRfcMessage, 1);
    
    /* Convert the RFC-822 message to EMSD IPM format */
    rc = mts_convertRfc822ToEmsdContent(pRfcEnv, pBodyText, pHeaderText, &pContent, &contentType);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error converting RFC message to IPM"));
	return rc;
    }
    
    /* Dispose of content types that we don't deal with yet. */
    switch(contentType)
    {
    case EMSD_ContentType_DeliveryReport:
	pfFree = (void (*)(void *)) mtsua_freeReportDelivery;
	pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_ReportDelivery,
					  pDev->protocolVersionMajor,
					  pDev->protocolVersionMinor);
	if (pCompTree == NULL) {
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Failed to get ASN.1 tree for DeliveryReport (v%d.%d)",
		    pDev->protocolVersionMajor,
		    pDev->protocolVersionMinor));
	}
	break;
    case EMSD_ContentType_Ipm95:
	pfFree = (void (*)(void *)) mtsua_freeMessage;
	pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_Ipm,
					  pDev->protocolVersionMajor,
					  pDev->protocolVersionMinor);
	if (pCompTree == NULL) {
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Failed to get ASN.1 tree for IPM-95 (v%d.%d)",
		    pDev->protocolVersionMajor,
		    pDev->protocolVersionMinor));
	}
	break;
	
    case EMSD_ContentType_Probe:
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "ContentType 'Probe' is not supported"));
	return UnsupportedOption;

    case EMSD_ContentType_VoiceMessage:
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "ContentType 'VoiceMessage' is not supported"));
	return UnsupportedOption;

    default:
	/* We don't support the other types yet */
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Unrecognized content type = %d", contentType));
	return UnsupportedOption;
    }
    if (pCompTree == NULL) {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "No ASN.1 compile tree assigned, delivery failed"));
	return ResourceError;
    }
    
    /* Arrange for the message contents to be freed */
    if ((rc = mts_operationAddUserFree(pOperation, pfFree, pContent)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error adding RFC content to free queue"));
	(* pfFree)(pContent);
	return rc;
    }
    
    /* Allocate a buffer to begin formatting the message */
    if ((rc = BUF_alloc(0, &pOperation->hFullPduBuf)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error allocating buffer"));
	return rc;
    }
    
    /* Arrange for the segment buffer to be freed */
    if ((rc = mts_operationAddUserFree(pOperation, BUF_free, pOperation->hFullPduBuf)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error adding segment burrer to free-list"));
	BUF_free(pOperation->hFullPduBuf);
	return rc;
    }
    
    /* Format the message */
    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Formating ASN.1 PDU for delivery argument"));
    rc = ASN_format(ASN_EncodingRules_Basic, pCompTree, pOperation->hFullPduBuf, pContent, &len);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "ASN.1 encoding error, delivery failed"));
	return rc;
    }
    
    /* Allocate an EMSD Deliver Argument structure */
    if ((rc = mtsua_allocDeliverArg(&pEmsdDeliverArg)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Allocation erroe"));
	return rc;
    }
    
    /* Arrange for the Deliver Argument to be freed later. */
    rc = mts_operationAddUserFree(pOperation, (void(*)(void *)) mtsua_freeDeliverArg, pEmsdDeliverArg);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error adding Delivery Argument (ASN.1) to free queue"));
	mtsua_freeDeliverArg(pEmsdDeliverArg);
	return rc;
    }
    
    /* Get the point-to-point envelope fields */
    if ((rc = mts_convertRfc822ToEmsdDeliver(pRfcEnv, pBodyText, pEmsdDeliverArg, contentType)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error convertinging RFC fields"));
	return rc;
    }
    
    /* Save the deliver arg */
    pOperation->hFirstOperationData = pEmsdDeliverArg;
    
    /* Create a new finite state machine */
    if ((pOperation->hFsm = FSM_createMachine(pOperation)) == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error assigning a state machine"));
/***	mts_freeOperation(pOperation); ***/
	return ResourceError;
    }
    
    /* Arrange for finite state machine to be freed */
    rc = mts_operationAddUserFree(pOperation, (void (*)(void *)) FSM_deleteMachine, pOperation->hFsm);
    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error adding FSM to free queue"));
	FSM_deleteMachine(pOperation->hFsm);
/***	mts_freeOperation(pOperation); ***/
	return rc;
    }
    
    /* Load the transition diagram into this machine */
    FSM_TRANSDIAG_load(pOperation->hFsm, globals.hDeliverTransDiag);
    
    /* Initialize this state machine */
    FSM_TRANSDIAG_resetMachine(pOperation->hFsm);
    
    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Delivery pre-processing successful"));
    return Success;
}


static MTS_Operation *
findOperationBySequenceId(char * pNsap, OS_Uint32 sequenceId)
{
    MTS_Operation *	        pOperation;
    char                        nsapString[64];
    struct DQ_RecipQueue *      pRecipQueue          = NULL;

    /* For each operation in our active operations queue... */
    for (pOperation = QU_FIRST(&globals.activeOperations);
	 ! QU_EQUAL(pOperation, &globals.activeOperations);
	 pOperation = QU_NEXT(pOperation))
    {
	sprintf (nsapString, "%d.%d.%d.%d", pOperation->nsap.addr[0], pOperation->nsap.addr[1],
		 pOperation->nsap.addr[2], pOperation->nsap.addr[3]);
    
	/* ... does this one have the specified sequence id and NSAP? */
	if (pOperation->sequenceId == sequenceId && strcmp(nsapString, pNsap) == 0)
	{
	    /* Yup. */
	  TM_TRACE((globals.hTM, MTS_TRACE_DETAIL,
		    "Found operation with sequence ID = %lu", sequenceId));
	    return pOperation;
	}
    }
    TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "No operation with sequence ID = %lu", sequenceId));

    return NULL;
}


