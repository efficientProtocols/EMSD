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
#include "emsdmail.h"
#include "emsd822.h"
#include "ua.h"
#include "uashloc.h"
#include "tm.h"
#include "tmr.h"
#include "target.h"
#include <time.h>

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
extern  OS_Sint32  maxInactiveIntervals;
extern  OS_Sint32  inactiveIntervalsCounter;
#endif

extern UASH_Statistics statistics;

EXTERN ReturnCode
checkSubmit(void * hTimer,
	    void * hUserData1,
	    void * hUserData2);

/*
 * Forward Declarations
 */

FORWARD static void
addRfc822Addr(UASH_Recipient * pRecip,
	      ADDRESS ** ppEnvAddr);


extern void
uash_displayMessage(char * pBuf);

extern UASH_Globals uash_globals;
extern OS_Uint32 messageSubmissionAttemptInterval;

/*
 * Registered functions which are called by UACORE
 */


static char * monthName[] = { "Jan", "Feb", "Mar",
		     "Apr", "May", "Jun",
		     "Jul", "Aug", "Sep",
		     "Oct", "Nov", "Dec" };

ReturnCode
uash_messageReceived(char * pMessageId,
		     OS_Uint32 submissionTime,
		     OS_Uint32 deliveryTime,
		     UASH_MessageHeading * pHeading,
		     char * pBodyText)
{
    ReturnCode			rc;
    int				len;
    ENVELOPE * 			pRfc822Env;
    UASH_Recipient * 		pRecip;
    UASH_RecipientInfo *	pRecipInfo;
    UASH_Extension *		pExtension;
    char *			pExtraHeaders;
    char *			p;
    OS_ZuluDateTime 		zuluDateTime;

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    inactiveIntervalsCounter = 0;
#endif

    /* Obtain a new RFC-822 envelope structure */
    pRfc822Env = mail_newenvelope();

    /* Add the Message Id string */
    if ((pRfc822Env->message_id = OS_allocStringCopy(pMessageId)) == NULL)
    {
	mail_free_env(pRfc822Env);
	return ResourceError;
    }

    /* Add the submission time Date string */
    if ((pRfc822Env->date = OS_alloc(32)) == NULL)
    {
	mail_free_env(pRfc822Env);
	return ResourceError;
    }

    /* Obtain the submission time fields */
    if ((rc = OS_julianToDateStruct(submissionTime, &zuluDateTime)) != Success)
    {
	mail_free_env(pRfc822Env);
	return rc;
    }

    /* Create a properly-formatted date string */
    sprintf(pRfc822Env->date,
	    "%02d %.3s %02d %02d:%02d:%02d GMT",
	    zuluDateTime.day,
		monthName[zuluDateTime.month - 1],
	    zuluDateTime.year % 100,
	    zuluDateTime.hour,
	    zuluDateTime.minute,
	    zuluDateTime.second);

    /* Add the originator */
    addRfc822Addr(&pHeading->originator, &pRfc822Env->from);

    /* Add the sender, if it exists.  Also, add the Return Path */
    if (pHeading->sender.style == UASH_RecipientStyle_Emsd ||
	pHeading->sender.un.pRecipRfc822 != NULL)
    {
	addRfc822Addr(&pHeading->sender, &pRfc822Env->sender);

	/* Add the sender as the return_path also */
	addRfc822Addr(&pHeading->sender, &pRfc822Env->return_path);
    }
    else
    {
	/* Add the originator as the return_path also */
	addRfc822Addr(&pHeading->originator, &pRfc822Env->return_path);
    }

    /* For each recipient... */
    for (pRecipInfo = QU_FIRST(&pHeading->recipInfoQHead);
	 ! QU_EQUAL(pRecipInfo, &pHeading->recipInfoQHead);
	 pRecipInfo = QU_NEXT(pRecipInfo))
    {
	/* ... what type of recipient is he? */
	switch(pRecipInfo->recipientType)
	{
	case UASH_RecipientType_Primary:
	    /* He's a primary recipient */
	    addRfc822Addr(&pRecipInfo->recipient, &pRfc822Env->to);
	    break;
	    
	case UASH_RecipientType_Copy:
	    /* He's a copy recipient */
	    addRfc822Addr(&pRecipInfo->recipient, &pRfc822Env->cc);
	    break;
	    
	case UASH_RecipientType_BlindCopy:
	    /* He's a blind copy recipient */
	    addRfc822Addr(&pRecipInfo->recipient, &pRfc822Env->bcc);
	    break;
	}
    }

    /* If there were reply-to recipients specified... */
    for (pRecip = QU_FIRST(&pHeading->replyToQHead);
	 ! QU_EQUAL(pRecip, &pHeading->replyToQHead);
	 pRecip = QU_NEXT(pRecip))
    {
	addRfc822Addr(pRecip, &pRfc822Env->reply_to);
    }
    
    /* Add the replied-to-IPM */
    if (pHeading->pRepliedToMessageId != NULL)
    {
	if ((pRfc822Env->in_reply_to =
	     OS_allocStringCopy(pHeading->pRepliedToMessageId)) == NULL)
	{
	    mail_free_env(pRfc822Env);
	    return ResourceError;
	}
    }

    /* Add the subject string */
    if (pHeading->pSubject != NULL)
    {
	if ((pRfc822Env->subject =
	     OS_allocStringCopy(pHeading->pSubject)) == NULL)
	{
	    mail_free_env(pRfc822Env);
	    return ResourceError;
	}
    }

    /* Determine how much space is required for non-standard header fields */
#define	LEN(s)	((s) == NULL ? 0 : strlen(s) + 1)

    len = (LEN("Precedence: special-delivery\n") +
	   LEN("X-Importance: High\n") +
	   LEN("X-Auto-Forwarded: True\n") +
	   LEN("MIME-Version: \n") +
	   LEN(pHeading->pMimeVersion) +
	   LEN("Content-Type: \n") +
	   LEN(pHeading->pMimeContentType) +
	   LEN("Content-Id: \n") +
	   LEN(pHeading->pMimeContentType) +
	   LEN("Content-Description: \n") +
	   LEN(pHeading->pMimeContentDescription) +
	   LEN("Content-Transfer-Encoding: \n") +
	   LEN(pHeading->pMimeContentTransferEncoding));

    for (pExtension = QU_FIRST(&pHeading->extensionQHead);
	 ! QU_EQUAL(pExtension, &pHeading->extensionQHead);
	 pExtension = QU_NEXT(pExtension))
    {
	len += LEN(pExtension->pLabel) + LEN(pExtension->pValue);
    }

#undef LEN

    /* Allocate space for non-standard header fields */
    if ((p = pExtraHeaders = OS_alloc(len)) == NULL)
    {
	mail_free_env(pRfc822Env);
	return ResourceError;
    }

    *p = '\0';    

    /* Add each of the extension (unrecognized) fields */
    for (pExtension = QU_FIRST(&pHeading->extensionQHead);
	 ! QU_EQUAL(pExtension, &pHeading->extensionQHead);
	 pExtension = QU_NEXT(pExtension))
    {
	/* Add this extension label and value */
	sprintf(p, "%s: %s\n",
		pExtension->pLabel,
		pExtension->pValue);

	/* Update extra headers pointer */
	p += strlen(p);
    }

    /*
     * If there's a MIME Content Type specified, then we'll set the
     * other MIME header fields.  Otherwise, we'll assume that this
     * is not a MIME encoding, and we'll take the body text to be a
     * simple text message.
     */
    if (pHeading->pMimeContentType != NULL)
    {
	/* Add the MIME version */
	if (pHeading->pMimeVersion != NULL)
	{
	    sprintf(p, "MIME-Version: %s\n", pHeading->pMimeVersion);
	}
	else
	{
	    strcpy(pExtraHeaders, "MIME-Version: 1.0\n");
	}

	p += strlen(p);

	/* Add the MIME content type */
	sprintf(p, "Content-Type: %s\n", pHeading->pMimeContentType);
	p += strlen(p);

	/* Add the MIME content id */
	if (pHeading->pMimeContentId != NULL)
	{
	    sprintf(p, "Content-Id: %s\n", pHeading->pMimeContentId);
	    p += strlen(p);
	}

	/* Add the MIME content description */
	if (pHeading->pMimeContentDescription != NULL)
	{
	    sprintf(p, "Content-Description: %s\n",
		    pHeading->pMimeContentDescription);
	    p += strlen(p);
	}

	/* Add the MIME content transfer encoding */
	if (pHeading->pMimeContentTransferEncoding)
	{
	    sprintf(p,
		    "Content-Transfer-Encoding: %s\n",
		    pHeading->pMimeContentTransferEncoding);
	    p += strlen(p);
	}
    }

    /*
     * Add per-message flag header fields
     */

    /* If the Priority isn't "Normal", add it. */
    switch(pHeading->priority)
    {
    case UASH_Priority_Normal:
	/* Do nothing in this case */
	break;

    case UASH_Priority_NonUrgent:
	/* Add a Priority header line */
	strcpy(p, "Precedence: bulk\n");

	/* Update extra headers pointer */
	p += strlen(p);
	break;
	
    case UASH_Priority_Urgent:
	/* Add a Priority header line */
	strcpy(p, "Precedence: special-delivery\n");

	/* Update extra headers pointer */
	p += strlen(p);
	break;
    }
    
    /* If the Importance isn't "Normal", add it. */
    switch(pHeading->importance)
    {
    case UASH_Importance_Normal:
	/* Do nothing in this case */
	break;
	
    case UASH_Importance_Low:
	/* Add an Importance header line */
	strcpy(p, "X-Importance: Low\n");

	/* Update extra headers pointer */
	p += strlen(p);
	break;
	
    case UASH_Importance_High:
	/* Add an Importance header line */
	strcpy(p, "X-Importance: High\n");

	/* Update extra headers pointer */
	p += strlen(p);

	break;
    }

    /* If the message was Auto-Forwarded, indicate such. */
    if (pHeading->bAutoForwarded)
    {
	/* Add an Auto-Forwarded header line */
	strcpy(pExtraHeaders, "X-Auto-Forwarded: True\n");

	/* Update extra headers pointer */
	p += strlen(p);

    }

    /* Deliver the message to local users */
    if ((rc = uash_deliverMessage(pRfc822Env, pExtraHeaders, pBodyText,
				  &pHeading->recipInfoQHead)) != Success)
    {
	OS_free(pExtraHeaders);
	mail_free_env(pRfc822Env);
	return rc;
    }

    /* Free the Extra Headers buffer */
    OS_free(pExtraHeaders);

    /* Free the envelope and all of its associated data */
    mail_free_env(pRfc822Env);

    return Success;
}


void
uash_modifyDeliverStatus(char * pMessageId,
			 UASH_DeliverStatus messageStatus)
{
    char	buf[512];

    buf[0] = '\0';

    strcat(buf, "Deliver Status: ");
    
    switch(messageStatus)
    {
    case UASH_DeliverStatus_UnAcknowledged:
	strcat(buf, "Unacknowledged");
	break;
	
    case UASH_DeliverStatus_Acknowledged:
	strcat(buf, "Acknowledged");
	break;
	
    case UASH_DeliverStatus_NonDelSent:
	strcat(buf, "Non-Delivery Report Sent");
	break;
	
    case UASH_DeliverStatus_PossibleNonDelSent:
	strcat(buf, "Possible Non-Delivery Report Sent");
	break;

    default:
	strcat(buf, "<unrecognized status value>");
	break;
    }

    strcat(buf, " [Msg Id=");
    strcat(buf,  pMessageId);
    strcat(buf, "]!\n");
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    strcat(buf, "Press 'q' to exit ('p' to get messages from Message Center)!\n");
#endif

    uash_displayMessage(buf);
}


void
uash_submitSuccess(void * hMessage,
		   char * pMessageId,
		   void * hShellReference)
{
    char	buf[512];
    ReturnCode	rc;

    statistics.successfulSubmissions ++; 

    sprintf(buf, "Submission successful [Msg Id=%s]!                                                               ", pMessageId);
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    strcat(buf, "Press 'q' to exit ('p' to get messages from Message Center)!\n");
#endif

    uash_displayMessage(buf);

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
    	    "Submission Successful [MsgId:%s]; "
	    "Temporary message handle: 0x%lx",
	    pMessageId,
	    (unsigned long) hMessage));

    /* Delete the submitted file */  
    /* It is file name for now. It should be ShellReference handle */
    if ((rc = OS_deleteFile((char *)hShellReference)) != Success)
    {
    	sprintf(buf,
		"Could not delete submitted file (%s)!",
		(char *)hShellReference);
	EH_problem(buf);
    }

    if ((rc = TMR_start(TMR_SECONDS(messageSubmissionAttemptInterval), 
			NULL, NULL, checkSubmit, NULL)) != Success)
    {
    	sprintf(buf,
	        "Could not start timer to check for submissions; "
		"rc = 0x%x!", rc);
	EH_fatal(buf);
    }
	
}


void
uash_submitFail(void * hMessage,
		OS_Boolean bTransient,
		void *hShellReference)
{
    char	buf[512];
    ReturnCode	rc;

    statistics.submissionFailures ++; 

    sprintf(buf, "Submission Failed %s!", bTransient ? "(will retry)                                                        " : "                                                            ");
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    strcat(buf, "Press 'q' to exit ('p' to get messages from Message Center)!\n");
#endif

    uash_displayMessage(buf);

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
    	    "Submission Failure; "
	    "Temporary Message Handle: 0x%lx; "
	    "%s",
	    (unsigned long) hMessage,
	    bTransient ? "Transient Error" : "No more attempt"));

    if ((rc = TMR_start(TMR_SECONDS(messageSubmissionAttemptInterval), 
			NULL, NULL, checkSubmit, NULL)) != Success)
    {
    	sprintf(buf,
	        "Could not start timer to check for submissions; "
		"rc = 0x%x!", rc);
	EH_fatal(buf);
    }
	
}


void
uash_deliveryReport(char * pMessageId,
		    OS_Uint32 deliveryTime)
{
    char	buf[512];

    sprintf(buf,
    	    "Delivery Report [%s] "
	    "Delivery-Time: %s",
	    pMessageId,
	    OS_printableDateTime(&deliveryTime));

    uash_displayMessage(buf);
}


void
uash_nonDeliveryReport(char * pMessageId,
		       UASH_NonDeliveryReason reason,
		       UASH_NonDeliveryDiagnostic diagnostic,
		       char * pExplanation)
{
    char	buf[512];

    buf[0] = '\0';

    sprintf(buf, "Non-Delivery Report [%s] ", pMessageId);

    switch(reason)
    {
    case UASH_NonDeliveryReason_TransferFailure:
	strcat(buf, "Reason: Transfer Failure; ");
	break;
	
    case UASH_NonDeliveryReason_UnableToTransfer:
	strcat(buf, "Reason: Unable To Transfer; ");
	break;
	
    case UASH_NonDeliveryReason_RestrictedDelivery:
	strcat(buf, "Reason: Restricted Delivery; ");
	break;

    default:
	strcat(buf, "Reason: <unknown reason code>; ");
	break;
    }

    switch(diagnostic)
    {
    case UASH_NonDeliveryDiagnostic_Unspecified:
	strcat(buf, "Diagnostic: Unspecified; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_Congestion:
	strcat(buf, "Diagnostic: Congestion; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_LoopDetected:
	strcat(buf, "Diagnostic: Loop Detected; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_RecipientUnavailable:
	strcat(buf, "Diagnostic: Recipient Unavailable; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_MaximumTimeExpired:
	strcat(buf, "Diagnostic: Maximum Time Expired; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_ContentTooLong:
	strcat(buf, "Diagnostic: Content Too Long; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_SizeConstraintViolated:
	strcat(buf, "Diagnostic: Size Constraint Violated; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_ProtocolViolation:
	strcat(buf, "Diagnostic: Protocol Violation; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_ContentTypeNotSupported:
	strcat(buf, "Diagnostic: Content Type Not Supported; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_TooManyRecipients:
	strcat(buf, "Diagnostic: Too Many Recipients; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_LineTooLong:
	strcat(buf, "Diagnostic: Line Too Long; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_UnrecognizedAddress:
	strcat(buf, "Diagnostic: Unrecognized Address; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_RecipientUnknown:
	strcat(buf, "Diagnostic: Recipient Unknown; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_RecipientRefusedToAccept:
	strcat(buf, "Diagnostic: Recipient Refused To Accept; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_UnableToCompleteTransfer:
	strcat(buf, "Diagnostic: Unable To Complete Transfer; ");
	break;
	
    case UASH_NonDeliveryDiagnostic_TransferAttemptsLimitReached:
	strcat(buf, "Diagnostic: Transfer Attempts Limit Reached; ");
	break;
	
    default:
	strcat(buf, "Diagnostic: <unrecognized diagnostic code>; ");
	break;
    }

    if (pExplanation != NULL)
    {
	strcat(buf, "Explanation: ");
	strcat(buf, pExplanation);
    }
    else
    {
	strcat(buf, "Explanation: <not provided>");
    }

    uash_displayMessage(buf);
}


static void
addRfc822Addr(UASH_Recipient * pRecip,
	      ADDRESS ** ppEnvAddr)
{
    char	    tmp[512];

    if (pRecip->style == UASH_RecipientStyle_Emsd)
    {
	/*
	 * Create an RFC-822 address from the local-format address.
	 *
	 * The format that we use is as follows:
	 *
	 *     A personal name (if it exists), followed by a valid
	 *     address:
	 *
	 *     The User-name portion is composed of:
	 *         The local address value changed into ascii string.
	 *
	 *     The Domain-name portion is unused.
	 */
	sprintf(tmp, "%.*s <%06ld>",
		(int) (pRecip->un.recipEmsd.pName != NULL ?
		       strlen(pRecip->un.recipEmsd.pName) :
		       0),
		(pRecip->un.recipEmsd.pName != NULL ?
		 pRecip->un.recipEmsd.pName :
		 ""),
		pRecip->un.recipEmsd.emsdAddr);

	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
		      "%s",tmp));
	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
		      "whole address: size: %d %ld",
			sizeof(pRecip->un.recipEmsd.emsdAddr),
			pRecip->un.recipEmsd.emsdAddr));
    }
    else
    {
	/* RFC-822 format */
	strcpy(tmp, pRecip->un.pRecipRfc822);
    }

    /* Parse the address and add it to the list. */
    rfc822_parse_adrlist(ppEnvAddr, tmp, "");
}


#ifdef DELIVERY_CONTROL
void
uash_deliveryControlSuccess(void * hOperationId,
    			    EMSD_DeliveryControlResult *pDeliveryControlResult,
		    	    void * hShellReference)
{
    char	buf[512];

    sprintf(buf, "Delivery Request Successful!");

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    strcat(buf, "Press 'q' to exit ('p' to get messages from Message Center)!\n");
#endif

    uash_displayMessage(buf);

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
    	    "Delivery Control Successful!"
	    "Operation handle: 0x%lx",
	    (unsigned long) hOperationId));
}

void
uash_deliveryControlError(void * hOperationId,
		          OS_Boolean bTransient,
		          void *hShellReference)
{
    char	buf[512];

    sprintf(buf, "Delivery Request received answer from Message Center!");

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    strcat(buf, "Press 'q' to exit ('p' to get messages from Message Center)!\n");
#endif

    uash_displayMessage(buf);

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
    	    "Delivery Control received Error response from MTS; "
	    "Operation Handle: 0x%lx; "
	    "%s",
	    (unsigned long) hOperationId,
	    bTransient ? "Transient Error" : "No more attempt"));

/*
    if ((rc = TMR_start(TMR_SECONDS(10), 
			NULL, NULL, UA_issueDeliveryControl, NULL)) != Success)
    {
    	sprintf(buf,
	        "Could not start timer to retry delivery control operation; "
		"rc = 0x%x!", rc);
	EH_fatal(buf);
    }
*/
}

void
uash_deliveryControlFail(void * hOperationId,
		         OS_Boolean bTransient,
		         void *hShellReference)
{
    char	buf[512];

    sprintf(buf, "Delivery Request failed! "
	    "No response from Message Center %s!\n", 
	    bTransient ? "(will retry)" : "");
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    strcat(buf, "Press 'q' to exit ('p' to get messages from Message Center)!\n");
#endif

    uash_displayMessage(buf);

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
    	    "Delivery Control Failure; "
	    "Operation Handle: 0x%lx; "
	    "%s",
	    (unsigned long) hOperationId,
	    bTransient ? "Transient Error" : "No more attempt"));

/*
    if ((rc = TMR_start(TMR_SECONDS(10), 
			NULL, NULL, UA_issueDeliveryControl, NULL)) != Success)
    {
    	sprintf(buf,
	        "Could not start timer to retry delivery control operation; "
		"rc = 0x%x!", rc);
	EH_fatal(buf);
    }
*/
}
#endif

