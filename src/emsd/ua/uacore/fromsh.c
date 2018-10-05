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
 *    ua_messageFromShell(UASH_MessageHeading * pLocalHeading,
 *    addAddr(UASH_Recipient * pLocalRecip,
 *    addressesDiffer(UASH_Recipient * pRecip1,
 *
 */


#include "estd.h"
#include "tm.h"
#include "strfunc.h"
#include "emsd.h"
#include "mtsua.h"
#include "uacore.h"
#include "ua.h"


FORWARD static ReturnCode
addAddr(UASH_Recipient * pLocalRecip,
	EMSD_ORAddress * pORAddr);

static OS_Boolean
addressesDiffer(UASH_Recipient * pRecip1,
		UASH_Recipient * pRecip2);



ReturnCode
ua_messageFromShell(UASH_MessageHeading * pLocalHeading,
		    char * pBodyText,
		    EMSD_Message ** ppIpm)
{
    int 			count;
    EMSD_Message * 		pIpm;
    EMSD_IpmHeading * 		pIpmHeading;
    EMSD_RecipientData * 	pIpmRecipientData;
    EMSD_ORAddress * 		pORAddress;
    EMSD_AsciiString * 		phString;
    UASH_Recipient *		pLocalRecip;
    UASH_RecipientInfo * 	pLocalRecipInfo;
    ReturnCode 			rc;
    
    /* Allocate an EMSD Message structure */
    if ((rc = mtsua_allocMessage(&pIpm)) != Success)
    {
	return rc;
    }

    /* Point to the heading */
    pIpmHeading = &pIpm->heading;
    
    /*
     * Fill in the IPM heading from the RFC-822 envelope info
     */
    
    /* Add the originator */
    if ((rc = addAddr(&pLocalHeading->originator,
		      &pIpmHeading->originator)) != Success)
    {
	mtsua_freeMessage(pIpm);
	return rc;
    }

    /* Add the sender, if it exists and differs from the originator */
    if (((pLocalHeading->sender.style == UASH_RecipientStyle_Rfc822 &&
	  pLocalHeading->sender.un.pRecipRfc822 != NULL) ||
	 pLocalHeading->sender.style == UASH_RecipientStyle_Emsd) &&
	addressesDiffer(&pLocalHeading->sender, &pLocalHeading->originator))
    {
	pIpmHeading->sender.exists = TRUE;
	if ((rc = addAddr(&pLocalHeading->sender,
			  &pIpmHeading->sender.data)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }
    else
    {
	pIpmHeading->sender.exists = FALSE;
    }
    
    /* Point to the recipient data list */
    pIpmRecipientData = &pIpmHeading->recipientDataList.data[0];
    
    /* For each recipient... */
    for (pLocalRecipInfo = QU_FIRST(&pLocalHeading->recipInfoQHead),
	     count = 0;
	 
	 ! QU_EQUAL(pLocalRecipInfo, &pLocalHeading->recipInfoQHead);
	 
	 pLocalRecipInfo = QU_NEXT(pLocalRecipInfo),
	     count++,
	     pIpmRecipientData++)
    {
/* Define a macro for Per-Recipient Flags to make lines a reasonable length */
#define	PRF(s)	EMSD_PerRecipFlags_##s

	/* Add the recipient address */
	if ((rc = addAddr(&pLocalRecipInfo->recipient,
			  &pIpmRecipientData->recipient)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}

	/* Allocate memory in which to put the bit-string data */
	if ((rc =
	     STR_alloc(8, &pIpmRecipientData->perRecipFlags.data)) != Success)
	{
	    return rc;
	}
	
	/* Initially, there are no per-recipient flags */
	pIpmRecipientData->perRecipFlags.exists = FALSE;

	/* What type of recipient is he? */
	switch(pLocalRecipInfo->recipientType)
	{
	case UASH_RecipientType_Primary:
	    /* No flag for primary recipient */
	    break;

	case UASH_RecipientType_Copy:
	    /* Set the flag indicating that this is a Cc: recipient */
	    STR_bitStringSetBit(pIpmRecipientData->perRecipFlags.data,
				PRF(RecipientType_Copy),
				TRUE);

	    /* There will now be a Per-Recipient Flags bit-string */
	    pIpmRecipientData->perRecipFlags.exists = TRUE;

	    break;

	case UASH_RecipientType_BlindCopy:
	    /* Set the flag indicating that this is a Bcc: recipient */
	    STR_bitStringSetBit(pIpmRecipientData->perRecipFlags.data,
				PRF(RecipientType_BlindCopy),
				TRUE);

	    /* There will now be a Per-Recipient Flags bit-string */
	    pIpmRecipientData->perRecipFlags.exists = TRUE;

	    break;
	}

	/* Was receipt notification requested? */
	if (pLocalRecipInfo->bReceiptNotificationRequested)
	{
	    /* Set the flag indicating that notification is requested */
	    STR_bitStringSetBit(pIpmRecipientData->perRecipFlags.data,
				PRF(NotificationRequest_RN),
				TRUE);

	    /* There will now be a Per-Recipient Flags bit-string */
	    pIpmRecipientData->perRecipFlags.exists = TRUE;
	}

	/* Was non-receipt notification requested? */
	if (pLocalRecipInfo->bNonReceiptNotificationRequested)
	{
	    /* Set the flag indicating that notification is requested */
	    STR_bitStringSetBit(pIpmRecipientData->perRecipFlags.data,
				PRF(NotificationRequest_NRN),
				TRUE);

	    /* There will now be a Per-Recipient Flags bit-string */
	    pIpmRecipientData->perRecipFlags.exists = TRUE;
	}

	/* Was Message Return requested? */
	if (pLocalRecipInfo->bMessageReturnRequested)
	{
	    /* Set the flag indicating that message return is requested */
	    STR_bitStringSetBit(pIpmRecipientData->perRecipFlags.data,
				PRF(NotificationRequest_IpmReturn),
				TRUE);

	    /* There will now be a Per-Recipient Flags bit-string */
	    pIpmRecipientData->perRecipFlags.exists = TRUE;
	}

	/* Were Non-Delivery Reports requested? */
	if (pLocalRecipInfo->bNonDeliveryReportRequested)
	{
	    /* Set the flag indicating that reports are requested */
	    STR_bitStringSetBit(pIpmRecipientData->perRecipFlags.data,
				PRF(ReportRequest_NonDelivery),
				TRUE);

	    /* There will now be a Per-Recipient Flags bit-string */
	    pIpmRecipientData->perRecipFlags.exists = TRUE;
	}

	/* Were Delivery Reports requested? */
	if (pLocalRecipInfo->bDeliveryReportRequested)
	{
	    /* Set the flag indicating that reports are requested */
	    STR_bitStringSetBit(pIpmRecipientData->perRecipFlags.data,
				PRF(ReportRequest_Delivery),
				TRUE);

	    /* There will now be a Per-Recipient Flags bit-string */
	    pIpmRecipientData->perRecipFlags.exists = TRUE;
	}

	/* Was a Reply requested? */
	if (pLocalRecipInfo->bReplyRequested)
	{
	    /* Set the flag indicating that a reply is requested */
	    STR_bitStringSetBit(pIpmRecipientData->perRecipFlags.data,
				PRF(ReplyRequested),
				TRUE);

	    /* There will now be a Per-Recipient Flags bit-string */
	    pIpmRecipientData->perRecipFlags.exists = TRUE;
	}

#undef PRF
    }
    
    /* Fill in the number of recipients we added */
    pIpmHeading->recipientDataList.count = count;
    
    /* Point to the reply-to list */
    pORAddress = &pIpmHeading->replyTo.data.data[0];
    
    /*
     * Get the reply-to recipients.  If there's only one, exclude it
     * if it's the same as the From: line.  Otherwise, add all of
     * 'em.
     */

    /* First, is there one and only one reply-to recipient? */
    pLocalRecip = QU_FIRST(&pLocalHeading->replyToQHead);
    if (! QU_EQUAL(pLocalRecip, &pLocalHeading->replyToQHead) &&
	QU_EQUAL(QU_NEXT(pLocalRecip), &pLocalHeading->replyToQHead))
    {
	/* Yup.  Assume initially, it's the same as the sender or originator */
	count = 0;

	/* Now, was there a sender specified? */
	if (pLocalHeading->sender.style != UASH_RecipientStyle_Rfc822 ||
	     pLocalHeading->sender.un.pRecipRfc822 != NULL)
	{
	    /* Yup.  Is the reply-to recipient the same as the sender? */
	    if (addressesDiffer(pLocalRecip, &pLocalHeading->sender))
	    {
		if ((rc = addAddr(pLocalRecip,
				  &pIpmHeading->replyTo.data.data[0])) !=
		    Success)
		{
		    mtsua_freeMessage(pIpm);
		    return rc;
		}

		/* We're only going to have one reply-to recipient */
		count = 1;
	    }
	}
	else
	{
	    /* No sender specified.  Compare to Originator. */
	    if (addressesDiffer(pLocalRecip, &pLocalHeading->originator))
	    {
		if ((rc = addAddr(pLocalRecip,
				  &pIpmHeading->replyTo.data.data[0])) !=
		    Success)
		{
		    mtsua_freeMessage(pIpm);
		    return rc;
		}

		/* We're only going to have one reply-to recipient */
		count = 1;
	    }
	}
    }
    else
    {
	/*
	 * There were either zero reply-to recipients, or there were
	 * some number of them, greater than one.  Add all of 'em.
	 */
	for (pLocalRecip = QU_FIRST(&pLocalHeading->replyToQHead),
		 pORAddress = &pIpmHeading->replyTo.data.data[0],
		 count = 0;
	     
	     ! QU_EQUAL(pLocalRecip, &pLocalHeading->replyToQHead);
	     
	     pLocalRecip = QU_NEXT(pLocalRecip),
		 pORAddress++,
		 count++)
	{
	    if ((rc = addAddr(pLocalRecip,
			      pORAddress)) != Success)
	    {
		mtsua_freeMessage(pIpm);
		return rc;
	    }
	}
    }
    
    /* If there were no reply-to recipients... */
    if (count == 0)
    {
        /* ... then don't include replyTo in the PDU */
        pIpmHeading->replyTo.exists = FALSE;
    }
    else
    {
        /* Make sure replyTo is added to the PDU */
        pIpmHeading->replyTo.exists = TRUE;

        /* Fill in the number of reply-to recipients we added */
        pIpmHeading->replyTo.data.count = count;
    }
    
    /* Get the replied-to message id, if it exists */
    if (pLocalHeading->pRepliedToMessageId != NULL)
    {
	/* Indicate that it exists */
	pIpmHeading->repliedToIpm.exists = TRUE;
	
	/* We're using the RFC-822 option */
	pIpmHeading->repliedToIpm.data.choice =
	    EMSD_MESSAGEID_CHOICE_RFC822;
	
	/* Point to the AsciiString handle */
	phString =
	    &pIpmHeading->repliedToIpm.data.un.rfc822;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the in-reply-to string to it */
	if ((rc =
	     STR_assignZString(phString,
			       pLocalHeading->pRepliedToMessageId)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }
    
    /* Get the subject, if it exists */
    if (pLocalHeading->pSubject != NULL)
    {
	/* Indicate that it exists */
	pIpmHeading->subject.exists = TRUE;
	
	/* Point to the AsciiString handle for the subject */
	phString = &pIpmHeading->subject.data;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the Subject string to it */
	if ((rc =
	     STR_assignZString(phString,
			       pLocalHeading->pSubject)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }
    
    /* Allocate memory in which to put the Per Message Flag bit-string data */
    if ((rc = STR_alloc(8, &pIpmHeading->perMessageFlags.data)) != Success)
    {
	mtsua_freeMessage(pIpm);
	return rc;
    }
	
/* Define a macro for Per-Recipient Flags to make lines a reasonable length */
#define	PMF(s)	EMSD_PerMessageFlags_##s

    /* Add the message priority */
    switch(pLocalHeading->priority)
    {
    case UASH_Priority_Normal:
	break;
	
    case UASH_Priority_NonUrgent:
	/* Set the flag indicating Non-Urgent Priority */
	STR_bitStringSetBit(pIpmHeading->perMessageFlags.data,
			    PMF(Priority_NonUrgent),
			    TRUE);
	break;
	
    case UASH_Priority_Urgent:
	/* Set the flag indicating Urgent Priority */
	STR_bitStringSetBit(pIpmHeading->perMessageFlags.data,
			    PMF(Priority_Urgent),
			    TRUE);
	break;
    }

    /* Add the message importance */
    switch(pLocalHeading->importance)
    {
    case UASH_Importance_Normal:
	break;
	
    case UASH_Importance_Low:
	/* Set the flag indicating Low Importance */
	STR_bitStringSetBit(pIpmHeading->perMessageFlags.data,
			    PMF(Importance_Low),
			    TRUE);
	break;

    case UASH_Importance_High:
	/* Set the flag indicating High Importance */
	STR_bitStringSetBit(pIpmHeading->perMessageFlags.data,
			    PMF(Importance_High),
			    TRUE);
	break;
    }

    /* Add the flag indicating whether the message was auto-forwarded */
    if (pLocalHeading->bAutoForwarded)
    {
	STR_bitStringSetBit(pIpmHeading->perMessageFlags.data,
			    PMF(AutoForwarded),
			    TRUE);
    }

#undef PMF

    /*
     * Fill in the MIME header fields
     */

    /* Assign the Mime Version, if it exists and isn't 1.0 */
    if (pLocalHeading->pMimeVersion != NULL &&
	strcmp(pLocalHeading->pMimeVersion, "1.0") != 0)
    {
	/* Indicate that it exists */
	pIpmHeading->mimeVersion.exists = TRUE;
	
	/* Point to the AsciiString handle */
	phString = &pIpmHeading->mimeVersion.data;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the value */
	if ((rc =
	     STR_assignZString(phString,
			       pLocalHeading->pMimeVersion)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }
    
    /* Assign the Mime Content Type if it exists */
    if (pLocalHeading->pMimeContentType != NULL)
    {
	/* Indicate that it exists */
	pIpmHeading->mimeContentType.exists = TRUE;
	
	/* Point to the AsciiString handle */
	phString = &pIpmHeading->mimeContentType.data;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the value */
	if ((rc =
	     STR_assignZString(phString,
			       pLocalHeading->pMimeContentType)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }

    /* Assign the Mime Content Id if it exists */
    if (pLocalHeading->pMimeContentId != NULL)
    {
	/* Indicate that it exists */
	pIpmHeading->mimeContentId.exists = TRUE;
	
	/* Point to the AsciiString handle */
	phString = &pIpmHeading->mimeContentId.data;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the value */
	if ((rc =
	     STR_assignZString(phString,
			       pLocalHeading->pMimeContentId)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }

    /* Assign the Mime Content Description if it exists */
    if (pLocalHeading->pMimeContentDescription != NULL)
    {
	/* Indicate that it exists */
	pIpmHeading->mimeContentDescription.exists = TRUE;
	
	/* Point to the AsciiString handle */
	phString = &pIpmHeading->mimeContentDescription.data;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the value */
	if ((rc =
	     STR_assignZString(phString,
			       pLocalHeading->pMimeContentDescription)) !=
	    Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }

    /* Assign the Mime Content Transfer Encoding if it exists */
    if (pLocalHeading->pMimeContentTransferEncoding != NULL)
    {
	/* Indicate that it exists */
	pIpmHeading->mimeContentTransferEncoding.exists = TRUE;
	
	/* Point to the AsciiString handle */
	phString = &pIpmHeading->mimeContentTransferEncoding.data;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the value */
	if ((rc =
	     STR_assignZString(phString,
			       pLocalHeading->pMimeContentTransferEncoding)) !=
	    Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }

    /*
     * Fill in the IPM body from the specified body text, if it exists
     */
    if (pBodyText != NULL)
    {
	/* Indicate that it exists */
	pIpm->body.exists = TRUE;
	
	/* Point to the AsciiString handle */
	phString = &pIpm->body.data.bodyText;
	
	/* Make sure STR_assignZString() allocates memory for us */
	*phString = NULL;
	
	/* Assign the value */
	if ((rc = STR_assignZString(phString, pBodyText)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return rc;
	}
    }
    else
    {
	/* There's no body */
	pIpm->body.exists = FALSE;
    }
    
    /* Give 'em what they came for */
    *ppIpm = pIpm;

    return Success;

} /* ua_messageFromShell() */



static ReturnCode
addAddr(UASH_Recipient * pLocalRecip,
	EMSD_ORAddress * pORAddr)
{
    void **	    phString;
    ReturnCode	    rc;

    /* Which style of address are we converting? */
    switch(pLocalRecip->style)
    {
    case UASH_RecipientStyle_Emsd:
	/*
	 * It's an EMSD Local-format address.
	 */

	TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		  "Address conversion from UA Shell to EMSD Local..."));

	/* Specify the style */
	pORAddr->choice = EMSD_ORADDRESS_CHOICE_LOCAL;

	/* Copy the numeric address */
	pORAddr->un.local.emsdAddress = pLocalRecip->un.recipEmsd.emsdAddr;
	
	TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		  "\tEMSD Address = %03ld.%03ld",
		  pLocalRecip->un.recipEmsd.emsdAddr / 1000,
		  pLocalRecip->un.recipEmsd.emsdAddr % 1000));

	/* If there's also a name associated with it... */
	if (pLocalRecip->un.recipEmsd.pName != NULL)
	{
	    /*
	     * ... then assign it to an STR string.
	     */

	    TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		      "\tEMSD Name = %s", pLocalRecip->un.recipEmsd.pName));

	    /* Indicate that it exists */
	    pORAddr->un.local.emsdName.exists = TRUE;
	
	    /* Point to the AsciiString handle */
	    phString = &pORAddr->un.local.emsdName.data;
	
	    /* Make sure STR_assignZString() allocates memory for us */
	    *phString = NULL;
	
	    /* Assign the value */
	    if ((rc = STR_assignZString(phString, pLocalRecip->un.recipEmsd.pName)) != Success)
	    {
		return Fail;
	    }
	}
	break;
	
    case UASH_RecipientStyle_Rfc822:
	/*
	 * It's an RFC-822 address.
	 */

	TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		  "Address conversion from UA Shell to RFC-822..."));

	/* Specify the style */
	pORAddr->choice = EMSD_ORADDRESS_CHOICE_RFC822;

	/* If the RFC-822 address was specified... */
	if (pLocalRecip->un.pRecipRfc822 != NULL)
	{
	    /*
	     * ... then assign it to an STR string.
	     */

	    TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		      "\tRFC-822 Address = %s",
		      pLocalRecip->un.pRecipRfc822));

	    /* Point to the AsciiString handle */
	    phString = &pORAddr->un.rfc822;
	
	    /* Make sure STR_assignZString() allocates memory for us */
	    *phString = NULL;
	
	    /* Assign the value */
	    if ((rc = STR_assignZString(phString, pLocalRecip->un.pRecipRfc822)) != Success)
	    {
		return Fail;
	    }
	}
	else
	{
	    TM_TRACE((ua_globals.hTM, UA_TRACE_ADDRESS,
		      "\tRFC-822 Address = <null>  *** ERROR ***"));
	}
	break;
    }

    return Success;

} /* addAddr() */


static OS_Boolean
addressesDiffer(UASH_Recipient * pRecip1,
		UASH_Recipient * pRecip2)
{
    /* If they're not the same style, then they differ. */
    if (pRecip1->style != pRecip2->style)
    {
	return TRUE;
    }

    /* Which style of address are they? */
    switch(pRecip1->style)
    {
    case UASH_RecipientStyle_Emsd:
	/* EMSD-format address */
	if (pRecip1->un.recipEmsd.emsdAddr !=
	    pRecip2->un.recipEmsd.emsdAddr)
	{
	    return TRUE;
	}
	break;
	
    case UASH_RecipientStyle_Rfc822:
	/* RFC-822 address */
	if ((pRecip1->un.pRecipRfc822 == NULL &&
	     pRecip2->un.pRecipRfc822 != NULL) ||
	    (pRecip1->un.pRecipRfc822 != NULL &&
	     pRecip2->un.pRecipRfc822 == NULL))
	{
	    return TRUE;
	}
	
	if (OS_strcasecmp(pRecip1->un.pRecipRfc822,
			  pRecip2->un.pRecipRfc822) != 0)
	{
	    return TRUE;
	}
	break;
    }

    /* The addresses do not differ. */
    return FALSE;

} /* addressesDiffer() */
