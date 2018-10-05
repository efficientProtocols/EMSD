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
#include "emsd.h"
#include "uacore.h"
#include "ua.h"


FORWARD static void
addAddr(EMSD_ORAddress * pORAddr,
	UASH_Recipient * pLocalRecip);

FORWARD static void
freeHeadingFields(UASH_MessageHeading * pHeading);



ReturnCode
ua_messageToShell(EMSD_DeliverArgument * pDeliver,
		  EMSD_Message * pIpm,
		  UA_Operation * pOperation)
{
    int				i;
    char *			pBodyText;
    char			repliedToMessageId[128];
    UASH_MessageHeading 	heading;
    UASH_Recipient * 		pLocalRecip;
    UASH_RecipientInfo *	pLocalRecipInfo;
    UASH_Extension *		pLocalExten;
    EMSD_Extension *		pEmsdExten;
    EMSD_IpmHeading *		pIpmHeading = &pIpm->heading;
    EMSD_ORAddress *		pORAddr;
    EMSD_RecipientData * 	pIpmRecipData;
    OS_Uint32			submissionTime;

    /* Initialize the structure to pass the heading to the UA Shell */
    heading.originator.style 		= UASH_RecipientStyle_Rfc822;
    heading.originator.un.pRecipRfc822 	= NULL;
    heading.sender.style 		= UASH_RecipientStyle_Rfc822;
    heading.sender.un.pRecipRfc822 	= NULL;
    QU_INIT(&heading.recipInfoQHead);
    QU_INIT(&heading.replyToQHead);
    heading.pRepliedToMessageId 	= NULL;
    heading.pSubject 			= NULL;
    heading.priority 			= UASH_Priority_Normal;
    heading.importance 			= UASH_Importance_Normal;
    heading.bAutoForwarded 		= FALSE;
    heading.pMimeVersion 		= NULL;
    heading.pMimeContentType 		= NULL;
    heading.pMimeContentId 		= NULL;
    heading.pMimeContentDescription 	= NULL;
    heading.pMimeContentTransferEncoding= NULL;
    QU_INIT(&heading.extensionQHead);

    /*
     * Convert the IPM header fields into a UASH_MessageHeading
     */

    /* Add the originator */
    addAddr(&pIpmHeading->originator, &heading.originator);

    /* Add the sender, if he exists */
    if (pIpmHeading->sender.exists)
    {
	addAddr(&pIpmHeading->sender.data, &heading.sender);
    }

    /* For each recipient... */
    for (i = pIpmHeading->recipientDataList.count - 1,
	     pIpmRecipData = &pIpmHeading->recipientDataList.data[0];

	 i >= 0;

	 --i, ++pIpmRecipData)
    {
/* Define a macro for Per-Recipient Flags to make lines a reasonable length */
#define	PRF(s)	EMSD_PerRecipFlags_##s

	/* Allocate a RecipientInfo structure for this recipient */
	if ((pLocalRecipInfo =
	     OS_alloc(sizeof(UASH_RecipientInfo))) == NULL)
	{
	    freeHeadingFields(&heading);
	    return ResourceError;
	}

	QU_INIT(pLocalRecipInfo);

	/* Add this recipient address */
	addAddr(&pIpmRecipData->recipient, &pLocalRecipInfo->recipient);

	/* What type of recipient is he? */
	if (! pIpmRecipData->perRecipFlags.exists)
	{
	    /* He's a primary recipient */
	    pLocalRecipInfo->recipientType = UASH_RecipientType_Primary;
	}
	else if (STR_bitStringGetBit(pIpmRecipData->perRecipFlags.data,
				     PRF(RecipientType_Copy)))
	{
	    /* He's a copy recipient */
	    pLocalRecipInfo->recipientType = UASH_RecipientType_Copy;
	}
	else if (STR_bitStringGetBit(pIpmRecipData->perRecipFlags.data,
				     PRF(RecipientType_BlindCopy)))
	{
	    /* He's a blind copy recipient */
	    pLocalRecipInfo->recipientType = UASH_RecipientType_BlindCopy;
	}
	else
	{
	    /* He's a primary recipient */
	    pLocalRecipInfo->recipientType = UASH_RecipientType_Primary;
	}

	/* What options were requested? */
	if (pIpmRecipData->perRecipFlags.exists)
	{
	    /* Was Receipt Notification requested? */
	    if (STR_bitStringGetBit(pIpmRecipData->perRecipFlags.data,
				    PRF(NotificationRequest_RN)))
	    {
		/* Yup. */
		pLocalRecipInfo->bReceiptNotificationRequested = TRUE;
	    }

	    /* Was Non-Receipt Notification requested? */
	    if (STR_bitStringGetBit(pIpmRecipData->perRecipFlags.data,
				    PRF(NotificationRequest_NRN)))
	    {
		/* Yup. */
		pLocalRecipInfo->bNonReceiptNotificationRequested = TRUE;
	    }

	    /* Was Message Return requested? */
	    if (STR_bitStringGetBit(pIpmRecipData->perRecipFlags.data,
				    PRF(NotificationRequest_IpmReturn)))
	    {
		/* Yup. */
		pLocalRecipInfo->bMessageReturnRequested = TRUE;
	    }

	    /* Were Non-Delivery Reports requested? */
	    if (STR_bitStringGetBit(pIpmRecipData->perRecipFlags.data,
				    PRF(ReportRequest_NonDelivery)))
	    {
		/* Yup. */
		pLocalRecipInfo->bNonDeliveryReportRequested = TRUE;
	    }

	    /* Were Delivery Reports requested? */
	    if (STR_bitStringGetBit(pIpmRecipData->perRecipFlags.data,
				    PRF(ReportRequest_Delivery)))
	    {
		/* Yup. */
		pLocalRecipInfo->bDeliveryReportRequested = TRUE;
	    }

	    /* Was a Reply requested? */
	    if (STR_bitStringGetBit(pIpmRecipData->perRecipFlags.data,
				    PRF(ReplyRequested)))
	    {
		/* Yup. */
		pLocalRecipInfo->bReplyRequested = TRUE;
	    }
	}

	/* Add this recipient to the list of recipients */
	QU_INSERT(pLocalRecipInfo, &heading.recipInfoQHead);

#undef PRF
    }

    /* For each reply-to recipient... */
    for (i = pIpmHeading->replyTo.data.count - 1,
	     pORAddr = &pIpmHeading->replyTo.data.data[0];

	 i >= 0;

	 --i, ++pORAddr)
    {
	/* Allocate a Recipient structure for this guy */
	if ((pLocalRecip =
	     OS_alloc(sizeof(UASH_Recipient))) == NULL)
	{
	    freeHeadingFields(&heading);
	    return ResourceError;
	}

	QU_INIT(pLocalRecip);

	/* Add this recipient address */
	addAddr(pORAddr, pLocalRecip);

	/* Add this recipient to the list of reply-to recipients */
	QU_INSERT(pLocalRecip, &heading.replyToQHead);
    }

    /* Add the In-reply-to message id (if it exists) */
    if (pIpmHeading->repliedToIpm.exists)
    {
	heading.pRepliedToMessageId =
	    ua_messageIdEmsdToRfc822(&pIpmHeading->repliedToIpm.data,
				    repliedToMessageId);
    }
	    
    /* Add the subject string */
    if (pIpmHeading->subject.exists)
    {
	heading.pSubject =
	    (char *) STR_stringStart(pIpmHeading->subject.data);
    }

    /* Add per-message flag header fields */
    if (pIpmHeading->perMessageFlags.exists)
    {
/* Define a macro for Per-Message Flags to make lines a reasonable length */
#define	PMF(s)	EMSD_PerMessageFlags_##s

	/* Get the message priority */
	if (STR_bitStringGetBit(pIpmHeading->perMessageFlags.data,
				PMF(Priority_Urgent)))
	{
	    heading.priority = UASH_Priority_Urgent;
	}
	else if (STR_bitStringGetBit(pIpmHeading->perMessageFlags.data,
				     PMF(Priority_NonUrgent)))
	{
	    heading.priority = UASH_Priority_NonUrgent;
	}

	/* Get the message importance */
	if (STR_bitStringGetBit(pIpmHeading->perMessageFlags.data,
				PMF(Importance_Low)))
	{
	    heading.importance = UASH_Importance_Low;
	}
	else if (STR_bitStringGetBit(pIpmHeading->perMessageFlags.data,
				     PMF(Importance_High)))
	{
	    heading.importance = UASH_Importance_High;
	}

	/* Was this message auto-forwarded? */
	if (STR_bitStringGetBit(pIpmHeading->perMessageFlags.data,
				PMF(AutoForwarded)))
	{
	    heading.bAutoForwarded = TRUE;
	}

#undef PMF
    }

    /*
     * If there's a MIME Content Type specified, then we'll
     * set the other MIME header fields.  Otherwise, we'll
     * assume that this is not a MIME encoding, and we'll take
     * the body text to be a simple text message.
     */
    if (pIpmHeading->mimeContentType.exists)
    {
	/* Add the MIME version */
	if (pIpmHeading->mimeVersion.exists)
	{
	    heading.pMimeVersion =
		(char *) STR_stringStart(pIpmHeading->mimeVersion.data);
	}
	else
	{
	    heading.pMimeVersion = "1.0";
	}

	/* Add the MIME content type */
	if (pIpmHeading->mimeContentType.exists)
	{
	    heading.pMimeContentType =
		(char *)
		STR_stringStart(pIpmHeading->mimeContentType.data);
	}

	/* Add the MIME content id */
	if (pIpmHeading->mimeContentId.exists)
	{
	    heading.pMimeContentId =
		(char *) STR_stringStart(pIpmHeading->mimeContentId.data);
	}

	/* Add the MIME content description */
	if (pIpmHeading->mimeContentDescription.exists)
	{
	    heading.pMimeContentDescription =
		(char *) STR_stringStart(pIpmHeading->
					 mimeContentDescription.data);
	}

	/* Add the MIME content transfer encoding */
	if (pIpmHeading->mimeContentTransferEncoding.exists)
	{
	    heading.pMimeContentTransferEncoding =
		(char *) STR_stringStart(pIpmHeading->
					 mimeContentTransferEncoding.
					 data);
	}
    }


    /* Add each of the extension (unrecognized) fields */
    for (i = pIpmHeading->extensions.count - 1,
	     pEmsdExten = pIpmHeading->extensions.data;
	     
	 i >= 0;
	     
	 i--, pEmsdExten++)
    {
	if ((pLocalExten = OS_alloc(sizeof(UASH_Extension))) == NULL)
	{
	    freeHeadingFields(&heading);
	    return ResourceError;
	}

	QU_INIT(pLocalExten);

	/* Get the extension label and value */
	pLocalExten->pLabel = (char *) STR_stringStart(pEmsdExten->label);
	pLocalExten->pValue = (char *) STR_stringStart(pEmsdExten->value);

	/* Add this extension to the list of extensions */
	QU_INSERT(pLocalExten, &heading.extensionQHead);
    }

    /* Get a pointer to the body text, if it exists */
    if (pIpm->body.exists)
    {
	pBodyText = (char *) STR_stringStart(pIpm->body.data.bodyText);
    }
    else
    {
	pBodyText = NULL;
    }

    /* Get this message's message id from the Deliver envelope */
    ua_messageIdEmsdToRfc822(&pDeliver->messageId,
			    pOperation->messageId);

    /* Get the submission date */
    if (pDeliver->messageId.choice == EMSD_MESSAGEID_CHOICE_RFC822)
    {
	/* See if there's one specified.  There really should be! */
	if (pDeliver->submissionTime.exists)
	{
	    /* It's in the Submission Time field */
	    submissionTime = pDeliver->submissionTime.data;
	}
	else
	{
	    submissionTime = 0;
	}
    }
    else
    {
	/* It's part of the local message id */
	submissionTime = pDeliver->messageId.un.local.submissionTime;
    }
	

    /* Give message to the user */
    if ((* ua_globals.pfMessageReceived)(pOperation->messageId,
					 submissionTime,
					 pDeliver->deliveryTime,
					 &heading,
					 pBodyText) != Success)
    {
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Could not give received message to user"));
	return Fail;
    }

    /* Free the heading fields */
    freeHeadingFields(&heading);

    return Success;
}



static void
addAddr(EMSD_ORAddress * pORAddr,
	UASH_Recipient * pLocalRecip)
{
    /* Which style of address are we converting? */
    switch(pORAddr->choice)
    {
    case EMSD_ORADDRESS_CHOICE_LOCAL:
	/*
	 * It's an EMSD Local-format address.
	 */

	TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		  "Address conversion from EMSD Local to UA Shell..."));

	/* Specify the style */
	pLocalRecip->style = UASH_RecipientStyle_Emsd;

	/* Copy the numeric address */
	pLocalRecip->un.recipEmsd.emsdAddr = pORAddr->un.local.emsdAddress;
	
	TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		  "\tEMSD Address = %03ld.%03ld",
		  pLocalRecip->un.recipEmsd.emsdAddr / 1000,
		  pLocalRecip->un.recipEmsd.emsdAddr % 1000));

	/* If there's also a name associated with it... */
	if (pORAddr->un.local.emsdName.exists)
	{
	    /* ... then point to it. */
	    pLocalRecip->un.recipEmsd.pName =
		STR_stringStart(pORAddr->un.local.emsdName.data);

	    TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		      "\tEMSD Name = %s",
		      pLocalRecip->un.recipEmsd.pName));
	}
	else
	{
	    pLocalRecip->un.recipEmsd.pName = NULL;
	}
	break;

    case EMSD_ORADDRESS_CHOICE_RFC822:
	/*
	 * It's an RFC-822 address.
	 */

	TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		  "Address conversion from RFC-822 to UA Shell..."));

	/* Specify the style */
	pLocalRecip->style = UASH_RecipientStyle_Rfc822;

	/* Point to the RFC-822 address */
	pLocalRecip->un.pRecipRfc822 = STR_stringStart(pORAddr->un.rfc822);

	TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		  "\tRFC-822 Address = %s",
		  pLocalRecip->un.pRecipRfc822));

	break;
    }
}


static void
freeHeadingFields(UASH_MessageHeading * pHeading)
{
    /* Free the elements in the recipient info queue */
    QU_FREE(&pHeading->recipInfoQHead);

    /* Free the elements in the reply-to queue */
    QU_FREE(&pHeading->replyToQHead);

    /* Free the elements in the extension queue */
    QU_FREE(&pHeading->extensionQHead);
}
