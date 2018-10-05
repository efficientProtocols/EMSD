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


#include "uashloc.h"
#include "estd.h"
#include "tm.h"
#include "nvm.h"
#include "ua.h"
#include "target.h"


/*
 * Forward function declarations
 */
FORWARD static ReturnCode
addAddress(UASH_Recipient * pRecip,
	   Recipient * pLocRecip,
	   NVM_TransInProcess hTip);



/*
 * Registered functions which are called by UACORE
 */


ReturnCode
uash_messageReceived(char * pMessageId,
		     OS_Uint32 submissionTime,
		     OS_Uint32 deliveryTime,
		     UASH_MessageHeading * pHeading,
		     char * pBodyText)
{
    ReturnCode			rc;
    UASH_Recipient * 		pRecip;
    UASH_RecipientInfo *	pRecipInfo;
    UASH_Extension *		pExtension;
    NVM_TransInProcess		hTip;
    NVM_Transaction		hTrans;
    NVM_Memory			hMem;
    NVM_Memory			hMessage;
    Message *			pMessage;
    Recipient *			pLocRecip;
    RecipientInfo *		pLocRecipInfo;
    Extension *			pLocExten;

    /* Begin a non-volatile memory transaction */
    if ((rc = NVM_beginTrans(TransType_InBox, &hTip, &hTrans)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "NVM_beginTrans() failed, reason 0x%x", rc));
	return rc;
    }

    /* Obtain a new Message structure handle */
    if ((rc = NVM_alloc(hTip, sizeof(Message), &hMessage)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate message heading, reason 0x%x", rc));
	goto cleanUp;
    }

    /* Point to the Message structure */
    pMessage = NVM_getMemPointer(hTrans, hMessage);

    /* Add the Message Id string */
    if ((rc = NVM_allocStringCopy(hTip, pMessageId,
				  &pMessage->hMessageId)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate message id string, reason 0x%x", rc));
	goto cleanUp;
    }

    /* Add the submission time */
    pMessage->submissionTime = submissionTime;

    /* Add the delivery time */
    pMessage->deliveryTime = deliveryTime;

    /* Add the delivery status */
    pMessage->status.deliverStatus = UASH_DeliverStatus_UnAcknowledged;

    /* Initially, there's no printable message status */
    pMessage->hPrintableStatus = NVM_NULL;
    
    /* Add the originator */
    if ((rc = addAddress(&pHeading->originator,
			 &pMessage->originator,
			 hTip)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not add originator address, reason 0x%x", rc));
	goto cleanUp;
    }

    /* Add the sender */
    if ((rc = addAddress(&pHeading->sender,
			 &pMessage->sender,
			 hTip)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not add sender address, reason 0x%x", rc));
	goto cleanUp;
    }

    /* Allocate the recipient info queue head */
    if ((rc = NVM_alloc(hTip, sizeof(NVM_QuHead),
			&pMessage->hRecipInfoQHead)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate recip info queue head, "
		  "reason 0x%x", rc));
	goto cleanUp;
    }

    /* Initialize the queue */
    NVM_quInit(hTrans, pMessage->hRecipInfoQHead);

    /* For each recipient... */
    for (pRecipInfo = QU_FIRST(&pHeading->recipInfoQHead);
	 ! QU_EQUAL(pRecipInfo, &pHeading->recipInfoQHead);
	 pRecipInfo = QU_NEXT(pRecipInfo))
    {
	/* Allocate Recipient Info structure */
	if ((rc = NVM_alloc(hTip, sizeof(RecipientInfo), &hMem)) != Success)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		      "Could not allocate memory for Recipient Info, "
		      "reason 0x%x", rc));
	    goto cleanUp;
	}

	/* Point to the Recipient Info structure */
	pLocRecipInfo = NVM_getMemPointer(hTrans, hMem);

	/* Add the recipient address */
	if ((rc = addAddress(&pRecipInfo->recipient,
			     &pLocRecipInfo->recipient,
			     hTip)) != Success)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		      "Could not add recipient address, reason 0x%x", rc));
	    goto cleanUp;
	}

	/* Add the remainder of the per-recipient fields */
	pLocRecipInfo->recipientType =
	    pRecipInfo->recipientType;
	pLocRecipInfo->bReceiptNotificationRequested =
	    pRecipInfo->bReceiptNotificationRequested;
	pLocRecipInfo->bNonReceiptNotificationRequested =
	    pRecipInfo->bNonReceiptNotificationRequested;
	pLocRecipInfo->bMessageReturnRequested =
	    pRecipInfo->bMessageReturnRequested;
	pLocRecipInfo->bNonDeliveryReportRequested =
	    pRecipInfo->bNonDeliveryReportRequested;
	pLocRecipInfo->bDeliveryReportRequested =
	    pRecipInfo->bDeliveryReportRequested;
	pLocRecipInfo->bReplyRequested =
	    pRecipInfo->bReplyRequested;

	/* Initialize the recipient info queue pointers */
	NVM_quInit(hTrans, hMem);

	/* Insert this recipient onto the queue of recipients */
	NVM_quInsert(hTrans, hMem, hTrans, pMessage->hRecipInfoQHead);
    }

    /* Allocate the reply-to queue head */
    if ((rc = NVM_alloc(hTip, sizeof(NVM_QuHead),
			&pMessage->hReplyToQHead)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate reply-to queue head, "
		  "reason 0x%x", rc));
	goto cleanUp;
    }

    /* Initialize the queue */
    NVM_quInit(hTrans, pMessage->hReplyToQHead);

    /* If there were reply-to recipients specified... */
    for (pRecip = QU_FIRST(&pHeading->replyToQHead);
	 ! QU_EQUAL(pRecip, &pHeading->replyToQHead);
	 pRecip = QU_NEXT(pRecip))
    {
	/* Allocate Recipient structure */
	if ((rc = NVM_alloc(hTip, sizeof(Recipient), &hMem)) != Success)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		      "Could not allocate memory for Recipient, "
		      "reason 0x%x", rc));
	    goto cleanUp;
	}

	/* Point to the Recipient structure */
	pLocRecip = NVM_getMemPointer(hTrans, hMem);

	/* Add the recipient address */
	if ((rc = addAddress(pRecip, pLocRecip, hTip)) != Success)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		      "Could not add recipient address, reason 0x%x", rc));
	    goto cleanUp;
	}

	/* Initialize the recipient queue pointers */
	NVM_quInit(hTrans, hMem);

	/* Insert this recipient onto the reply-to recipient queue */
	NVM_quInsert(hTrans, hMem, hTrans, pMessage->hReplyToQHead);
    }
    
    /* Add the replied-to-IPM */
    if ((rc = NVM_allocStringCopy(hTip, pHeading->pRepliedToMessageId,
				  &pMessage->hRepliedToMessageId)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate replied-to message id, "
		  "reason 0x%x", rc));
	goto cleanUp;
    }

    /* Add the subject string */
    if ((rc = NVM_allocStringCopy(hTip, pHeading->pSubject,
				  &pMessage->hSubject)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate subject, reason 0x%x", rc));
	goto cleanUp;
    }

    /*
     * Add per-message flag header fields
     */

    pMessage->priority = pHeading->priority;
    pMessage->importance = pHeading->importance;
    pMessage->bAutoForwarded = pHeading->bAutoForwarded;

    /* Save the MIME fields */
    if ((rc =
	 NVM_allocStringCopy(hTip,
			     pHeading->pMimeVersion,
			     &pMessage->hMimeVersion)) != Success ||
	(rc =
	 NVM_allocStringCopy(hTip,
			     pHeading->pMimeContentType,
			     &pMessage->hMimeContentType)) != Success ||
	(rc =
	 NVM_allocStringCopy(hTip,
			     pHeading->pMimeContentId,
			     &pMessage->hMimeContentId)) != Success ||
	(rc =
	 NVM_allocStringCopy(hTip,
			     pHeading->pMimeContentDescription,
			     &pMessage->hMimeContentDescription)) != Success ||
	(rc =
	 NVM_allocStringCopy(hTip,
			     pHeading->pMimeContentTransferEncoding,
			     &pMessage->hMimeContentTransferEncoding)) !=
	Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate mime fields, reason 0x%x", rc));
	goto cleanUp;
    }

    /* Allocate the extension queue head */
    if ((rc = NVM_alloc(hTip, sizeof(NVM_QuHead),
			&pMessage->hExtensionQHead)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate extension queue head, "
		  "reason 0x%x", rc));
	goto cleanUp;
    }

    /* Initialize the queue */
    NVM_quInit(hTrans, pMessage->hExtensionQHead);

    /* Add the extension fields */
    for (pExtension = QU_FIRST(&pHeading->extensionQHead);
	 ! QU_EQUAL(pExtension, &pHeading->extensionQHead);
	 pExtension = QU_NEXT(pExtension))
    {
	/* Allocate an extension queue element */
	if ((rc = NVM_alloc(hTip, sizeof(NVM_QuHead), &hMem)) != Success)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		      "Could not allocate extension queue element, "
		      "reason 0x%x", rc));
	    goto cleanUp;
	}

	/* Point to the extension structure */
	pLocExten = NVM_getMemPointer(hTrans, hMem);

	/* Allocate the extension label */
	if ((rc = NVM_allocStringCopy(hTip, pExtension->pLabel,
				      &pLocExten->hLabel)) != Success)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		      "Could not allocate extension label, reason 0x%x", rc));
	    goto cleanUp;
	}

	/* Allocate the extension value */
	if ((rc = NVM_allocStringCopy(hTip, pExtension->pValue,
				      &pLocExten->hValue)) != Success)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		      "Could not allocate extension value, reason 0x%x", rc));
	    goto cleanUp;
	}

	/* Initialize the extension queue element */
	NVM_quInit(hTrans, hMem);

	/* Insert the extension queue element onto the extension queue */
	NVM_quInsert(hTrans, hMem, hTrans, pMessage->hExtensionQHead);
    }

    /* Allocate the body text */
    if ((rc = NVM_allocStringCopy(hTip, pBodyText,
				  &pMessage->hBody)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not allocate body text, reason 0x%x", rc));
	goto cleanUp;
    }

    /* End the non-volatile memory transaction */
    if ((rc = NVM_endTrans(hTip)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not end message transaction, reason 0x%x", rc));
	goto cleanUp;
    }

    /* Give the message to the user */
    if ((rc = uash_messageToUser(hTrans)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not give message to user, reason 0x%x", rc));
	goto cleanUp;
    }

    return Success;

  cleanUp:
    (void) NVM_abortTrans(hTip);
    return rc;
}


void
uash_modifyDeliverStatus(char * pMessageId,
			 UASH_DeliverStatus messageStatus)
{
    ReturnCode		rc;
    NVM_Transaction	hTrans;
    Message *		pMessage;
    char *		p;

    /* Find the message with the specified message id */
    for (hTrans = NVM_FIRST;
	 (rc = NVM_nextTrans(TransType_InBox, hTrans, &hTrans)) == Success;
	 )
    {
	/* Get the first allocation in this transaction: the message handle */
	pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);

	/* Get a pointer to the message id */
	p = NVM_getMemPointer(hTrans, pMessage->hMessageId);

	/* Is this the one we're looking for? */
	if (OS_strcasecmp(p, pMessageId) == 0)
	{
	    /* Yup. */
	    break;
	}
    }

    /* Did we find it? */
    if (rc != Success)
    {
	/* Nope.  Oh well. */
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Deliver Status: message id (%s) not found",
		  pMessageId));
	return;
    }

    /* Update the delivery status */
    pMessage->status.deliverStatus = messageStatus;

    /* Make sure non-volatile memory is properly synchronized */
    NVM_sync();
}


void
uash_submitSuccess(void * hMessage,
		   char * pMessageId,
		   void * hShellReference)
{
    ReturnCode		rc;
    NVM_Transaction	hTrans;
    Message *		pMessage;

    /* Find the message with the specified message id */
    for (hTrans = NVM_FIRST;
	 (rc = NVM_nextTrans(TransType_OutBox, hTrans, &hTrans)) == Success;
	 )
    {
	/* Get the first allocation in this transaction: the message handle */
	pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);

	/* Does this message have the specified message handle? */
	if (pMessage->status.hSubmission == hMessage)
	{
	    break;
	}
    }

    /* Did we find it? */
    if (rc != Success)
    {
	/* Nope.  Oh well. */
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Submission Success: message handle (0x%lx) not found",
		  hMessage));
	return;
    }

    /* We successfully submitted the message.  Free it from the outbox */
    (void) NVM_freeTrans(hTrans);
}


void
uash_submitFail(void * hMessage,
		OS_Boolean bTransient,
		void * hShellReference)
{
    unsigned long	i;
    OS_Uint32		dateTime;
    ReturnCode		rc;
    NVM_Transaction	hTrans;
    Message *		pMessage;
    char *		p;

    /* Find the message with the specified message id */
    for (hTrans = NVM_FIRST;
	 (rc = NVM_nextTrans(TransType_OutBox, hTrans, &hTrans)) == Success;
	 )
    {
	/* Get the first allocation in this transaction: the message handle */
	pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);

	/* Does this message have the specified message handle? */
	if (pMessage->status.hSubmission == hMessage)
	{
	    break;
	}
    }

    /* Did we find it? */
    if (rc != Success)
    {
	/* Nope.  Oh well. */
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Submission Failure: message handle (0x%lx) not found",
		  hMessage));
	return;
    }

    /* Indicate that submission is no longer in process */
    pMessage->status.hSubmission = NULL;

    /* Get the submission status message */
    p = NVM_getMemPointer(hTrans, pMessage->hPrintableStatus);

    /* Parse the message (if there is one) */
    if (*p == '\0')
    {
	i = 0;
    }
    else
    {
	sscanf(p, "%lu", &i);
    }

    /* Get the current date and time */
    if ((rc = OS_currentDateTime(NULL, &dateTime)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		  "Could not get current date/time to modify status message"));
    }
    else
    {
	/* Add the status message.  There's already space allocated for it. */
	sprintf(p, "%-4lu  (Last failure at: %s",
		++i, OS_printableDateTime(&dateTime));

	/* Delete new-line from date string, and add trailing parenthesis */
	p[strlen(p) - 1] = ')';
    }
	    
    NVM_sync();
}


void
uash_deliveryReport(char * pMessageId,
		    OS_Uint32 deliveryTime)
{
    printf("Delivery Report\n");
    printf("\tMessage-Id: %s\n", pMessageId);
    printf("\tDelivery-Time: %s\n", OS_printableDateTime(&deliveryTime));
}


void
uash_nonDeliveryReport(char * pMessageId,
		       UASH_NonDeliveryReason reason,
		       UASH_NonDeliveryDiagnostic diagnostic,
		       char * pExplanation)
{
    printf("Non-Delivery Report\n");

    switch(reason)
    {
    case UASH_NonDeliveryReason_TransferFailure:
	printf("\tReason: Transfer Failure\n");
	break;
	
    case UASH_NonDeliveryReason_UnableToTransfer:
	printf("\tReason: Unable To Transfer\n");
	break;
	
    case UASH_NonDeliveryReason_RestrictedDelivery:
	printf("\tReason: Restricted Delivery\n");
	break;

    default:
	printf("\tReason: INVALID REASON CODE %d\n", reason);
	break;
    }

    switch(diagnostic)
    {
    case UASH_NonDeliveryDiagnostic_Unspecified:
	printf("\tDiagnostic: Unspecified\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_Congestion:
	printf("\tDiagnostic: Congestion\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_LoopDetected:
	printf("\tDiagnostic: Loop Detected\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_RecipientUnavailable:
	printf("\tDiagnostic: Recipient Unavailable\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_MaximumTimeExpired:
	printf("\tDiagnostic: Maximum Time Expired\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_ContentTooLong:
	printf("\tDiagnostic: Content Too Long\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_SizeConstraintViolated:
	printf("\tDiagnostic: Size Constraint Violated\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_ProtocolViolation:
	printf("\tDiagnostic: Protocol Violation\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_ContentTypeNotSupported:
	printf("\tDiagnostic: Content Type Not Supported\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_TooManyRecipients:
	printf("\tDiagnostic: Too Many Recipients\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_LineTooLong:
	printf("\tDiagnostic: Line Too Long\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_UnrecognizedAddress:
	printf("\tDiagnostic: Unrecognized Address\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_RecipientUnknown:
	printf("\tDiagnostic: Recipient Unknown\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_RecipientRefusedToAccept:
	printf("\tDiagnostic: Recipient Refused To Accept\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_UnableToCompleteTransfer:
	printf("\tDiagnostic: Unable To Complete Transfer\n");
	break;
	
    case UASH_NonDeliveryDiagnostic_TransferAttemptsLimitReached:
	printf("\tDiagnostic: Transfer Attempts Limit Reached\n");
	break;
	
    default:
	printf("\tDiagnostic: INVALID DIAGNOSTIC CODE %d\n", diagnostic);
	break;
    }

    if (pExplanation != NULL)
    {
	printf("\tExplanation: %s\n", pExplanation);
    }
    else
    {
	printf("\tExplanation: <not provided>\n");
    }
}


static ReturnCode
addAddress(UASH_Recipient * pRecip,
	   Recipient * pLocRecip,
	   NVM_TransInProcess hTip)
{
    ReturnCode 	    rc;

    /* Save the recipient style */
    pLocRecip->style = pRecip->style;

    if (pRecip->style == UASH_RecipientStyle_Emsd)
    {
	/* Copy the EMSD address */
	pLocRecip->un.recipEmsd.emsdAddr = pRecip->un.recipEmsd.emsdAddr;

	/* Copy the name */
	if ((rc =
	     NVM_allocStringCopy(hTip, pRecip->un.recipEmsd.pName,
				 &pLocRecip->un.recipEmsd.hName)) != Success)
	{
	    return rc;
	}
    }
    else
    {
	/* RFC-822 format */
	if ((rc = NVM_allocStringCopy(hTip, pRecip->un.pRecipRfc822,
				      &pLocRecip->un.hRecipRfc822)) != Success)
	{
	    return rc;
	}
    }

    /* Initialize the delivery status */
    pLocRecip->status = DelStatus_Delivered;

    return Success;
}

#ifdef DELIVERY_CONTROL
void
uash_deliveryControlSuccess(void * hOperationId,
    			    EMSD_DeliveryControlResult *deliveryControlResult,
		    	    void * hShellReference)
{
    printf("Delivery Control Successful!\n");

    TM_TRACE((uash_globals.hTM, UASH_TRACE_DETAIL,
    	    "Delivery Control Successful!"
	    "Operation handle: 0x%lx",
	    (unsigned long) hOperationId));
}

void
uash_deliveryControlError(void * hOperationId,
		          OS_Boolean bTransient,
		          void *hShellReference)
{
    printf("Delivery Request received by Message Center!\n"); 

    TM_TRACE((uash_globals.hTM, UASH_TRACE_DETAIL,
    	    "Delivery Control received Error response from MTS; "
	    "Operation Handle: 0x%lx; "
	    "%s",
	    (unsigned long) hOperationId,
	    bTransient ? "Transient Error" : "No further attempt"));

/*
    if ((rc = TMR_start(TMR_SECONDS(10), 
			NULL, NULL, UA_issueDeliveryControl, NULL)) != Success)
    {
    	sprintf(buf,
	        "Could not start timer to retry delivery control operation; "
		"rc = 0x%x", rc);
	EH_fatal(buf);
    }
*/
}


void
uash_deliveryControlFail(void * hOperationId,
		         OS_Boolean bTransient,
		         void *hShellReference)
{
    printf("Delivery Control Failure: ");
    printf("%s\n", bTransient ? "Transient Error" : "Permanent Error");

    TM_TRACE((uash_globals.hTM, UASH_TRACE_DETAIL,
    	    "Delivery Control Failure; "
	    "Operation Handle: 0x%lx; "
	    "%s",
	    (unsigned long) hOperationId,
	    bTransient ? "Transient Error" : "Permanent Error"));

/*
    if ((rc = TMR_start(TMR_SECONDS(10), 
			NULL, NULL, UA_issueDeliveryControl, NULL)) != Success)
    {
    	sprintf(buf,
	        "Could not start timer to retry delivery control operation; "
		"rc = 0x%x", rc);
	EH_fatal(buf);
    }
*/
}
#endif


