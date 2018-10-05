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
#include "tm.h"
#include "strfunc.h"
#include "emsd.h"
#include "mtsua.h"
#include "eh.h"


ReturnCode
mtsua_allocMessage(EMSD_Message ** ppMessage)
{
    EMSD_Message    *pMessage;
    
    /* Allocate a message structure */
    if ((pMessage = OS_alloc(sizeof(EMSD_Message))) == NULL)
    {
	EH_problem("mtsua_allocMessage: OS_alloc failed\n");
	return FAIL_RC(ResourceError, ("Allocating EMSD_Message"));
    }
    
    /* Initialize the structure */
    OS_memSet(pMessage, '\0', sizeof(EMSD_Message));
    
    /* Give 'em what they came for */
    *ppMessage = pMessage;
    
    return Success;
}


void
mtsua_freeMessage(EMSD_Message * pMessage)
{
    int             i;
    
    if (pMessage->heading.sender.exists)
    {
	if (pMessage->heading.sender.data.choice ==
	    EMSD_ORADDRESS_CHOICE_RFC822)
	{
	    STR_free(pMessage->heading.sender.data.un.rfc822);
	}
    }
    
    if (pMessage->heading.originator.choice ==
	EMSD_ORADDRESS_CHOICE_RFC822)
    {
	STR_free(pMessage->heading.originator.un.rfc822);
    }
    
    for (i = pMessage->heading.recipientDataList.count - 1;
	 i >= 0;
	 i--)
    {
	if (pMessage->heading.recipientDataList.data[i].recipient.choice ==
	    EMSD_ORADDRESS_CHOICE_RFC822)
	{
	    STR_free(pMessage->
		     heading.recipientDataList.data[i].
		     recipient.un.rfc822);
	}
	if (pMessage->heading.recipientDataList.data[i].perRecipFlags.data !=
	    NULL)
	{
	    STR_free(pMessage->
		     heading.recipientDataList.data[i].perRecipFlags.data);
	}
    }
    
    if (pMessage->heading.perMessageFlags.data != NULL)
    {
	STR_free(pMessage->heading.perMessageFlags.data);
    }
    
    if (pMessage->heading.replyTo.exists)
    {
        for (i = pMessage->heading.replyTo.data.count - 1;
	     i >= 0;
	     i--)
	{
	  if (pMessage->heading.replyTo.data.data[i].choice ==
	      EMSD_ORADDRESS_CHOICE_RFC822)
	    {
	      STR_free(pMessage->heading.replyTo.data.data[i].un.rfc822);
	    }
	}
    }
    
    if (pMessage->heading.repliedToIpm.exists &&
	(pMessage->heading.repliedToIpm.data.choice ==
	 EMSD_MESSAGEID_CHOICE_RFC822))
    {
	STR_free(pMessage->heading.repliedToIpm.data.un.rfc822);
    }
    
    if (pMessage->heading.subject.exists)
    {
	STR_free(pMessage->heading.subject.data);
    }
    
    for (i = pMessage->heading.extensions.count - 1;
	 i >= 0;
	 i--)
    {
	STR_free(pMessage->heading.extensions.data[i].label);
	STR_free(pMessage->heading.extensions.data[i].value);
    }
    
    if (pMessage->heading.mimeContentType.exists)
    {
	STR_free(pMessage->heading.mimeContentType.data);
    }
    
    if (pMessage->heading.mimeVersion.exists)
    {
	STR_free(pMessage->heading.mimeVersion.data);
    }
    
    if (pMessage->body.exists)
    {
	STR_free(pMessage->body.data.bodyText);
    }

    OS_free(pMessage);
}


ReturnCode
mtsua_allocSubmitArg(EMSD_SubmitArgument ** ppSubmit)
{
    EMSD_SubmitArgument *	pSubmit;

    /* Allocate a submit structure */
    if ((pSubmit = OS_alloc(sizeof(EMSD_SubmitArgument))) == NULL)
    {
	EH_problem("mtsua_allocSubmitArg: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_SubmitArgument"));
    }

    /* Initialize the structure */
    OS_memSet(pSubmit, '\0', sizeof(EMSD_SubmitArgument));

    /* Give 'em what they came for */
    *ppSubmit = pSubmit;

    return Success;
}


void
mtsua_freeSubmitArg(EMSD_SubmitArgument * pSubmit)
{
    OS_free(pSubmit);
}


ReturnCode
mtsua_allocSubmitRes(EMSD_SubmitResult ** ppSubmitResult)
{
    EMSD_SubmitResult *	    pSubmitResult;

    /* Allocate a submit result structure */
    if ((pSubmitResult = OS_alloc(sizeof(EMSD_SubmitResult))) == NULL)
    {
	EH_problem("mtsua_allocSubmitRes: OS_alloc failed\n");
	return FAIL_RC(ResourceError, ("Allocating EMSD_SubmitResult"));
    }

    /* Initialize the structure */
    OS_memSet(pSubmitResult, '\0', sizeof(EMSD_SubmitResult));

    /* Give 'em what they came for */
    *ppSubmitResult = pSubmitResult;

    return Success;
}


void
mtsua_freeSubmitRes(EMSD_SubmitResult * pSubmitResult)
{
    OS_free(pSubmitResult);
}


ReturnCode
mtsua_allocDeliverArg(EMSD_DeliverArgument ** ppDeliver)
{
    EMSD_DeliverArgument *	pDeliver;
    
    /* Allocate a deliver structure */
    if ((pDeliver = OS_alloc(sizeof(EMSD_DeliverArgument))) == NULL)
    {
	EH_problem("mtsua_allocDeliverArg: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_DeliverArgument"));
    }
    
    /* Initialize the structure */
    OS_memSet(pDeliver, '\0', sizeof(EMSD_DeliverArgument));
    
    /* Give 'em what they came for */
    *ppDeliver = pDeliver;
    
    return Success;
}


void
mtsua_freeDeliverArg(EMSD_DeliverArgument * pDeliver)
{
    switch (pDeliver->messageId.choice) {

	case EMSD_MESSAGEID_CHOICE_LOCAL:
	    break;

	case EMSD_MESSAGEID_CHOICE_RFC822:
    	    if (pDeliver->messageId.un.rfc822 != NULL) {
    	    	STR_free(pDeliver->messageId.un.rfc822);
	    }
	    break;

	default:
	    break;
    }

    OS_free(pDeliver);
}


ReturnCode
mtsua_allocDeliveryControlArg(EMSD_DeliveryControlArgument ** ppDeliveryControl)
{
    EMSD_DeliveryControlArgument *	pDeliveryControl;

    /* Allocate a deliver structure */
    if ((pDeliveryControl =
	 OS_alloc(sizeof(EMSD_DeliveryControlArgument))) == NULL)
    {
	EH_problem("mtsua_allocDeliverControlArg: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_DeliveryControlArgument"));
    }
    
    /* Initialize the structure */
    OS_memSet(pDeliveryControl, '\0', sizeof(EMSD_DeliveryControlArgument));

    /* Give 'em what they came for */
    *ppDeliveryControl = pDeliveryControl;

    return Success;
}


void
mtsua_freeDeliveryControlArg(EMSD_DeliveryControlArgument * pDeliveryControl)
{
    OS_free(pDeliveryControl);
}


ReturnCode
mtsua_allocDeliveryControlRes(EMSD_DeliveryControlResult ** ppDeliveryControl)
{
    EMSD_DeliveryControlResult *	pDeliveryControl;

    /* Allocate a delivery control result structure */
    if ((pDeliveryControl =
	 OS_alloc(sizeof(EMSD_DeliveryControlResult))) == NULL)
    {
	EH_problem("mtsua_allocDeliveryControlRes: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_DeliveryControlResult"));
    }
    
    /* Initialize the structure */
    OS_memSet(pDeliveryControl, '\0', sizeof(EMSD_DeliveryControlResult));

    /* Give 'em what they came for */
    *ppDeliveryControl = pDeliveryControl;

    return Success;
}


void
mtsua_freeDeliveryControlRes(EMSD_DeliveryControlResult * pDeliveryControl)
{
    OS_free(pDeliveryControl);
}


ReturnCode
mtsua_allocReportDelivery(EMSD_ReportDelivery ** ppReportDelivery)
{
    EMSD_ReportDelivery *	pReportDelivery;

    /* Allocate a deliver structure */
    if ((pReportDelivery =
	 OS_alloc(sizeof(EMSD_ReportDelivery))) == NULL)
    {
	EH_problem("mtsua_allocReportDelivery: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_ReportDelivery"));
    }
    
    /* Initialize the structure */
    OS_memSet(pReportDelivery, '\0', sizeof(EMSD_ReportDelivery));

    /* Give 'em what they came for */
    *ppReportDelivery = pReportDelivery;

    return Success;
}


void
mtsua_freeReportDelivery(EMSD_ReportDelivery * pReportDelivery)
{
    OS_free(pReportDelivery);
}


ReturnCode
mtsua_allocSubmissionVerifyArg(EMSD_SubmissionVerifyArgument ** ppSubmitVerify)
{
    EMSD_SubmissionVerifyArgument *	pSubmitVerify;

    /* Allocate a submission verify argument structure */
    if ((pSubmitVerify =
	 OS_alloc(sizeof(EMSD_SubmissionVerifyArgument))) == NULL)
    {
	EH_problem("mtsua_allocSubmitssionVerifyArg: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_SubmissionVerifyArgument"));
    }
    
    /* Initialize the structure */
    OS_memSet(pSubmitVerify, '\0', sizeof(EMSD_SubmissionVerifyArgument));

    /* Give 'em what they came for */
    *ppSubmitVerify = pSubmitVerify;

    return Success;
}


void
mtsua_freeSubmissionVerifyArg(EMSD_SubmissionVerifyArgument * pSubmitVerify)
{
    OS_free(pSubmitVerify);
}


ReturnCode
mtsua_allocSubmissionVerifyRes(EMSD_SubmissionVerifyResult ** ppSubmitVerify)
{
    EMSD_SubmissionVerifyResult *	pSubmitVerify;

    /* Allocate a submission verify result structure */
    if ((pSubmitVerify =
	 OS_alloc(sizeof(EMSD_SubmissionVerifyResult))) == NULL)
    {
	EH_problem("mtsua_allocSubmissionVerifyRes: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_SubmissionVerifyResult"));
    }
    
    /* Initialize the structure */
    OS_memSet(pSubmitVerify, '\0', sizeof(EMSD_SubmissionVerifyResult));

    /* Give 'em what they came for */
    *ppSubmitVerify = pSubmitVerify;

    return Success;
}


void
mtsua_freeSubmissionVerifyRes(EMSD_SubmissionVerifyResult * pSubmitVerify)
{
    OS_free(pSubmitVerify);
}


ReturnCode
mtsua_allocDeliveryVerifyArg(EMSD_DeliveryVerifyArgument ** ppDeliveryVerify)
{
    EMSD_DeliveryVerifyArgument *	pDeliveryVerify;

    /* Allocate a Delivery verify argument structure */
    if ((pDeliveryVerify =
	 OS_alloc(sizeof(EMSD_DeliveryVerifyArgument))) == NULL)
    {
	EH_problem("mtsua_allocDeliveryVerifyArg: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_DeliveryVerifyArgument"));
    }
    
    /* Initialize the structure */
    OS_memSet(pDeliveryVerify, '\0', sizeof(EMSD_DeliveryVerifyArgument));

    /* Give 'em what they came for */
    *ppDeliveryVerify = pDeliveryVerify;

    return Success;
}


void
mtsua_freeDeliveryVerifyArg(EMSD_DeliveryVerifyArgument * pDeliveryVerify)
{
    OS_free(pDeliveryVerify);
}


ReturnCode
mtsua_allocDeliveryVerifyRes(EMSD_DeliveryVerifyResult ** ppDeliveryVerify)
{
    EMSD_DeliveryVerifyResult *	pDeliveryVerify;

    /* Allocate a Delivery verify result structure */
    if ((pDeliveryVerify =
	 OS_alloc(sizeof(EMSD_DeliveryVerifyResult))) == NULL)
    {
	EH_problem("mtsua_allocDeliveryVerifyRes: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_DeliveryVerifyResult"));
    }
    
    /* Initialize the structure */
    OS_memSet(pDeliveryVerify, '\0', sizeof(EMSD_DeliveryVerifyResult));

    /* Give 'em what they came for */
    *ppDeliveryVerify = pDeliveryVerify;

    return Success;
}


void
mtsua_freeDeliveryVerifyRes(EMSD_DeliveryVerifyResult * pDeliveryVerify)
{
    OS_free(pDeliveryVerify);
}


ReturnCode
mtsua_allocErrorRequest(EMSD_ErrorRequest ** ppErrorRequest)
{
    EMSD_ErrorRequest *	pErrorRequest;

    /* Allocate an Error Request structure */
    if ((pErrorRequest =
	 OS_alloc(sizeof(EMSD_ErrorRequest))) == NULL)
    {
	EH_problem("mtsua_allocErrorRequest: OS_alloc failed\n");
	return FAIL_RC(ResourceError,
		       ("Allocating EMSD_ErrorRequest"));
    }
    
    /* Initialize the structure */
    OS_memSet(pErrorRequest, '\0', sizeof(EMSD_ErrorRequest));

    /* Give 'em what they came for */
    *ppErrorRequest = pErrorRequest;

    return Success;
}


void
mtsua_freeErrorRequest(EMSD_ErrorRequest * pErrorRequest)
{
    OS_free(pErrorRequest);
}


