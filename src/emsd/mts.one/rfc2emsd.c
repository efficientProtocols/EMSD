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

static char *rcs = "$Id: rfc2emsd.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";


#include "estd.h"
#include "tm.h"
#include "emsdmail.h"
#include "strfunc.h"
#include "emsd.h"
#include "emsd822.h"
#include "mtsua.h"
#include "mts.h"
#include "rc.h"

static ReturnCode
convertRfc822ToEmsdIpm(ENVELOPE * pRfc822Env,
		      char * pRfc822Body,
		      char *pHeaderStr,
		      void ** ppContent);

static ReturnCode
convertRfc822ToEmsdReport(ENVELOPE * pRfc822Env,
			 char * pRfc822Body,
			 char *pHeaderStr,
			 void ** ppContent);

static ReturnCode
getRfc822Address(ADDRESS * pAddr,
		 EMSD_ORAddress * pORAddress);

static OS_Boolean
rfc822AddressesDiffer(ADDRESS * p1, ADDRESS * p2);

static ReturnCode
parseHeaderString(char *pHeaderStr,
		  EMSD_IpmHeading * pHeading);

static int
daysInMonth(int month, int year);

static ReturnCode
parseRfc822Date(char * pDateStr,
		OS_Uint32 * pJulianDate);


ReturnCode
createMessageId(char * pRfc822MessageId);

void
getPrintableRfc822Address(char * pBuf,
			  ADDRESS * pAddr);




ReturnCode
mts_convertRfc822ToEmsdContent(ENVELOPE * pRfc822Env,
			      char * pRfc822Body,
			      char *pHeaderStr,
			      void ** ppContent,
			      EMSD_ContentType * pContentType)
{
    char *			p;

    /* See if there's a special EMSD header item */
    if ((p = OS_findSubstring(pHeaderStr, "X-EMSD-Message-Type:")) != NULL)
    {
	/* There is.  See what type of special message it is */
	p += sizeof("X-EMSD-Message-Type:") - 1;

	/* Scan past white space */
	while (*p == ' ' || *p == '\t')
	{
	    ++p;
	}

	/* Compare the type */
	if (strncmp(p, "Report\r\n", sizeof("Report\r\n") - 1) == 0)
	{
	    /* It's a Delivery/Non-Delivery Report */
	    *pContentType = EMSD_ContentType_DeliveryReport;
	    
	    return convertRfc822ToEmsdReport(pRfc822Env, pRfc822Body, pHeaderStr, ppContent);
	}
	else
	{
	    /* Unknown type.  Treat it as a normal IPM */
	    *pContentType = EMSD_ContentType_Ipm95;

	    return convertRfc822ToEmsdIpm(pRfc822Env,
					 pRfc822Body,
					 pHeaderStr,
					 ppContent);
	}
    }
    else
    {
	/* No special header.  It's a normal IPM. */
	*pContentType = EMSD_ContentType_Ipm95;

	return convertRfc822ToEmsdIpm(pRfc822Env,
				     pRfc822Body,
				     pHeaderStr,
				     ppContent);
    }
}



ReturnCode
mts_convertRfc822ToEmsdSubmit(ENVELOPE * pRfc822Env,
			     char * pRfc822Body,
			     EMSD_SubmitArgument * pSubmit)
{
    /* We're creating an IPM */
    pSubmit->contentType = EMSD_ContentType_Ipm95;

    return Success;
}



ReturnCode
mts_convertRfc822ToEmsdDeliver(ENVELOPE * pRfc822Env,
			      char * pRfc822Body,
			      EMSD_DeliverArgument * pDeliver,
			      EMSD_ContentType contentType)
{
    ReturnCode		rc;
    char		buf[128];
    char *		p;
    EMSD_AsciiString *	phString;

    /* We're using an RFC-822 message id */
    pDeliver->messageId.choice = EMSD_MESSAGEID_CHOICE_RFC822;
    
    /* Point to the AsciiString for the message id */
    phString = &pDeliver->messageId.un.rfc822;
    
    /* Make sure STR_assignZString() allocates memory for us */
    *phString = NULL;
    
    /* Assign the message id string to it */
    if (pRfc822Env->message_id == NULL)
    {
	/* Create a message id for this message */
	if ((rc = createMessageId(buf)) != Success)
	{
	    mtsua_freeDeliverArg(pDeliver);
	    return FAIL_RC(rc, ("Creating message id"));
	}

	p = buf;
    }
    else
    {
	p = pRfc822Env->message_id;
    }
    
    if ((rc =
	 STR_assignZString(phString, p)) != Success)
    {
	mtsua_freeDeliverArg(pDeliver);
	return FAIL_RC(rc, ("Assigning message id"));
    }

    /* Determine the submission time.  Convert it to a Julian date */
    if (pRfc822Env->date != NULL && *pRfc822Env->date != '\0')
    {
	pDeliver->submissionTime.exists = TRUE;
	if ((rc =
	     parseRfc822Date(pRfc822Env->date,
			     &pDeliver->submissionTime.data)) !=
	    Success)
	{
	    mtsua_freeDeliverArg(pDeliver);
	    return FAIL_RC(rc, ("Assigning submission time"));
	}
    }
    
    /* The delivery time is NOW. */
    if ((rc =
	 OS_currentDateTime(NULL,
			    &pDeliver->deliveryTime)) != Success)
    {
	mtsua_freeDeliverArg(pDeliver);
	return FAIL_RC(rc, ("Assigning delivery time"));
    }
    
    /* Specify the content type */
    pDeliver->contentType = contentType;

    return Success;
}


static ReturnCode
convertRfc822ToEmsdIpm(ENVELOPE * pRfc822Env,
		      char * pRfc822Body,
		      char *pHeaderStr,
		      void ** ppContent)
{
    int		        i;
    char *	        p;
    ASN_Count 		count;
    EMSD_Message *	pIpm;
    EMSD_IpmHeading * 	pIpmHeading;
    EMSD_IpmBody * 	pIpmBody;
    EMSD_RecipientData * pRecipientData;
    EMSD_ORAddress * 	pORAddress;
    EMSD_AsciiString * 	phString;
    ADDRESS *		pAddr;
    ReturnCode	        rc;
    
    /* Allocate an EMSD Message structure */
    if ((rc = mtsua_allocMessage(&pIpm)) != Success)
    {
	return FAIL_RC(rc, ("fromRfc822ToEmsd: alloc IPM structure"));
    }

    /* Point to the heading */
    pIpmHeading = &pIpm->heading;
    
    /*
     * Fill in the IPM heading from the RFC-822 envelope info
     */
    
    /* Get the sender */
    if (pRfc822Env->sender != NULL &&
	rfc822AddressesDiffer(pRfc822Env->sender, pRfc822Env->from))
    {
	pIpmHeading->sender.exists = TRUE;

	RC_CALL(rc,
		getRfc822Address(pRfc822Env->sender,
				 &pIpmHeading->sender.data),
		("Filling IPM heading with Sender"));
    }
    else
    {
	pIpmHeading->sender.exists = FALSE;
    }
    
    /* Point to the originator */
    pORAddress = &pIpmHeading->originator;

    /* Get the originator address */
    RC_CALL(rc,
	    getRfc822Address(pRfc822Env->from, pORAddress),
	    ("Filling IPM heading with Originator"));
    
    /* Point to the recipient data list */
    pRecipientData = &pIpmHeading->recipientDataList.data[0];
    
    /* Get the primary (To:) recipients */
    for (pAddr = pRfc822Env->to, count = 0;
	 pAddr != NULL;
	 pAddr = pAddr->next, ++count, ++pRecipientData)
    {
	/* There will be no Per-Recipient Flags bit-string */
	pRecipientData->perRecipFlags.exists = FALSE;

	/* Get the recipient address */
	RC_CALL(rc,
		getRfc822Address(pAddr, &pRecipientData->recipient),
		("Filling IPM heading with primary recipient"));
    }
    
    /* Get the copy (Cc:) recipients */
    for (pAddr = pRfc822Env->cc;
	 pAddr != NULL;
	 pAddr = pAddr->next, ++count, ++pRecipientData)
    {
	/* Get the recipient address */
	RC_CALL(rc,
		getRfc822Address(pAddr, &pRecipientData->recipient),
		("Filling IPM heading with copy recipient"));
	
	/* There will now be a Per-Recipient Flags bit-string */
	pRecipientData->perRecipFlags.exists = TRUE;

	/* Allocate memory in which to put the bit-string data */
	RC_CALL(rc,
		STR_alloc(8,
			  &pRecipientData->perRecipFlags.data),
		("alloc of bit string"));
	
	/* Initialize the bit string */
	for (i = 0,
	     p = (char *)
	     STR_stringStart(pRecipientData->perRecipFlags.data);

	     i < 8;

	     i++, p++)
	{
	    *p = FALSE;
	}

	/* Set the bit-string length */
	STR_setStringLength(pRecipientData->perRecipFlags.data, 8);

	/* Default to Non-Delivery Report */
	STR_stringStart(pRecipientData->perRecipFlags.data)
	    [EMSD_PerRecipFlags_ReportRequest_NonDelivery] = TRUE;

	/* Set the flag indicating that this is a Cc: recipient */
	STR_stringStart(pRecipientData->perRecipFlags.data)
	    [EMSD_PerRecipFlags_RecipientType_Copy] = TRUE;
    }
    
    /* Get the blind copy (Bcc:) recipients */
    for (pAddr = pRfc822Env->bcc;
	 pAddr != NULL;
	 pAddr = pAddr->next, ++count, ++pRecipientData)
    {
	/* Get the recipient address */
	RC_CALL(rc,
		getRfc822Address(pAddr, &pRecipientData->recipient),
		("Filling IPM heading with blind copy recipient"));
	
	/* There will now be a Per-Recipient Flags bit-string */
	pRecipientData->perRecipFlags.exists = TRUE;

	/* Allocate memory in which to put the bit-string data */
	RC_CALL(rc,
		STR_alloc(8,
			  &pRecipientData->perRecipFlags.data),
		("alloc of bit string"));

	/* Initialize the bit string */
	for (i = 0, p = (char *) STR_stringStart(pRecipientData->perRecipFlags.data);
	     i < 8;
	     i++, p++)
	{
	    *p = FALSE;
	}

	/* Set the bit-string length */
	STR_setStringLength(pRecipientData->perRecipFlags.data, 8);

	/* Default to Non-Delivery Report */
	STR_stringStart(pRecipientData->perRecipFlags.data)
	    [EMSD_PerRecipFlags_ReportRequest_NonDelivery] = TRUE;

	/* Set the flag indicating that this is a Bcc: recipient */
	STR_stringStart(pRecipientData->perRecipFlags.data)
	    [EMSD_PerRecipFlags_RecipientType_BlindCopy] = TRUE;
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
    if ((pAddr = pRfc822Env->reply_to) != NULL && pAddr->next == NULL)
    {
	if (pRfc822Env->sender != NULL &&
	    rfc822AddressesDiffer(pAddr, pRfc822Env->from))
	{
	    RC_CALL(rc,
		    getRfc822Address(pAddr, pORAddress),
		    ("Filling IPM heading with reply-to (from) recipient"));
	    count = 1;
	}
	else
	{
	    count = 0;
	}
    }
    else
    {
	for (count = 0;
	     pAddr != NULL;
	     pAddr = pAddr->next, ++count, ++pORAddress)
	{
	    pORAddress->choice = EMSD_ORADDRESS_CHOICE_RFC822;
	    
	    RC_CALL(rc,
		    getRfc822Address(pAddr, pORAddress),
		    ("Filling IPM heading with reply-to recipient"));
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
    if (pRfc822Env->in_reply_to != NULL)
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
			       pRfc822Env->in_reply_to)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return FAIL_RC(rc,
			   ("Fillin in IPM heading with "
			    "replied-to IPM"));
	}
    }
    
    /* Get the subject, if it exists */
    if (pRfc822Env->subject != NULL)
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
			       pRfc822Env->subject)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return FAIL_RC(rc,
			   ("Filling in IPM heading with subject"));
	}
    }
    
    /* Get additional header fields not in the ENVELOPE */
    if ((rc =
	 parseHeaderString(pHeaderStr, pIpmHeading)) != Success)
    {
	mtsua_freeMessage(pIpm);
	return FAIL_RC(rc, ("parseHeaderString"));
    }
    
    /*
     * Fill in the IPM body from the RFC-822 body, if it exists
     */
    if (pRfc822Body != NULL)
    {
	/* There's a body.  Let 'em know it's there. */
	pIpm->body.exists = TRUE;
	pIpmBody = &pIpm->body.data;
	
	/* Allocate an AsciiString and attach the body to it */
	if ((rc = STR_attachZString(pRfc822Body,
				    FALSE,
				    &pIpmBody->bodyText)) != Success)
	{
	    mtsua_freeMessage(pIpm);
	    return FAIL_RC(rc, ("Attaching body text"));
	}
    }
    else
    {
	/* There's no body */
	pIpm->body.exists = FALSE;
    }
    
    /* Give 'em what they came for */
    *ppContent = pIpm;

    return Success;
}



static ReturnCode
convertRfc822ToEmsdReport(ENVELOPE * pRfc822Env,
			 char * pRfc822Body,
			 char *pHeaderStr,
			 void ** ppContent)
{
    int				reason;
    int				diagnostic;
    char *			pHeader;
    char *			pValue;
    char *			p;
    EMSD_AsciiString * 		phString;
    EMSD_PerRecipReportData * 	pPerRecipData;
    EMSD_ReportDelivery *	pReport;
    ReturnCode 			rc;
    
    /* Allocate a report delivery structure */
    if ((rc = mtsua_allocReportDelivery(&pReport)) != Success)
    {
	return FAIL_RC(rc,
		       ("convertRfc822ToEmsdReport: "
			"alloc report delivery"));
    }

    /* Point to the non-deliverable recipient data list */
    pPerRecipData = &pReport->recipDataList.data[0];
    
    /* Scan extra headers to determine the Intended Recipients. */
    for (pHeader = OS_findStringToken(pHeaderStr, ":");
	 pHeader != NULL;
	 pHeader = OS_findStringToken(NULL, ":"))
    {
	/* Scan past white space in the token */
	while (*pHeader == ' ' ||
	       *pHeader == '\t' ||
	       *pHeader == '\r' ||
	       *pHeader == '\n')
	{
	    ++pHeader;
	}

	/* Get the value for this header element */
	if ((pValue = OS_findStringToken(NULL, "\n")) == NULL)
	{
	    /* No value; nothing for us to do (shouldn't happen) */
	    break;
	}

	/* Scan past white space in the token */
	while (*pValue == ' ' ||
	       *pValue == '\t' ||
	       *pValue == '\r' ||
	       *pValue == '\n')
	{
	    ++pValue;
	}

	/* Is this an Intended Recipient header line? */
	if (OS_strcasecmp(pHeader, "X-EMSD-Intended-Recipient") == 0)
	{
	    /*
	     * Yup.  Find non-delivery reason and diagnostic codes
	     */

	    /* First, scan from end of value for '(' */
	    for (p = pValue + strlen(pValue) - 1;
		 p > pValue && *p != '(';
		 p--)
	    {
		/* do nothing; just scan. */
	    }

	    /* See if this is a Delivery or Non-Delivery report */
	    if (strncmp(p, "(DELIVERED", strlen("(DELIVERED")) == 0)
	    {
		/* It's a Delivery report. */
		pPerRecipData->report.choice =
		    EMSD_DELIV_CHOICE_DELIVERY;
	    }
	    else
	    {
		/* It's a Non-Delivery report. */
		pPerRecipData->report.choice =
		    EMSD_DELIV_CHOICE_NONDELIVERY;

		/* We'll be specifying a reason and diagnostic */
		pPerRecipData->report.un.nonDeliveryReport.nonDeliveryDiagnostic.exists = TRUE;
	    
		/*
		 * Did we find a well-formed reason and diagnostic?
		 */
		if (*p != '(' ||
		    sscanf(p + 1, "%d/%d", &reason, &diagnostic) != 2)
		{
		    /*
		     * No, it's an improperly formed
		     * reason/diagnostic
		     */
		    reason = EMSD_NonDeliveryReason_TransferFailure;

		    diagnostic =
			EMSD_NonDeliveryDiagnostic_UnableToCompleteTransfer;
		}

		/* Set the reason and diagnostic appropriately */
		pPerRecipData->report.un.nonDeliveryReport.nonDeliveryReason = reason;
		pPerRecipData->report.un.nonDeliveryReport.nonDeliveryDiagnostic.data = diagnostic;

		/* Scan past white space at the end of the string */
		for (--p;
		     p > pValue && (*p == ' ' || *p == '\t');
		     p--)
		{
		    /* Nothing to do.  Just scanning. */
		}

		/* Null-terminate the string here */
		if (p == pValue && (*p == ' ' || *p == '\t'))
		{
		    *p = '\0';
		}
		else
		{
		    *++p = '\0';
		}

		/* It's going to be an RFC-822 address */
		pPerRecipData->recipient.choice =
		    EMSD_ORADDRESS_CHOICE_RFC822;

		/* Point to the AsciiString handle */
		phString = &pPerRecipData->recipient.un.rfc822;
		
		/* Make sure STR_assignZString() allocates for us */
		*phString = NULL;
	
		/* Get the recipient address */
		if ((rc =
		     STR_assignZString(phString, pValue)) != Success)
		{
		    mtsua_freeReportDelivery(pReport);
		    return FAIL_RC(rc, ("convertRfc822ToEmsdReport: assign recipient address"));
		}
	    }
	    
	    /* Move to next recipient data */
	    ++pPerRecipData;
	}
	else if (OS_strcasecmp(pHeader, "X-EMSD-Original-Message-Id") == 0)
	{
	    /*
	     * Assign the message id of the reported message to the
	     * report
	     */

	    /* Point to the AsciiString handle */
	    phString = &pReport->msgIdOfReportedMsg.un.rfc822;
	    
	    /* Make sure STR_assignZString() allocates for us */
	    *phString = NULL;
	
	    if ((rc =
		 STR_assignZString(phString, pValue)) != Success)
	    {
		mtsua_freeReportDelivery(pReport);
		return FAIL_RC(rc,
			       ("convertRfc822ToEmsdReport: "
				"assign message id"));
	    }

	    /* Scan past white space at the end of the string */
	    for (p = pValue + strlen(pValue) - 1;
		 (p > pValue &&
		  (*p == ' ' || *p == '\t' || *p == '\r'));
		 p--)
	    {
		/* Nothing to do.  Just scanning. */
	    }

	    /* Null-terminate the string here */
	    if (p == pValue &&
		(*p == ' ' || *p == '\t' || *p == '\r'))
	    {
		*p = '\0';
	    }
	    else
	    {
		*++p = '\0';
	    }

	    /* Specify that we copied an RFC-822 message id */
	    pReport->msgIdOfReportedMsg.choice =
		EMSD_MESSAGEID_CHOICE_RFC822;
	}
    }

    /* Fill in the number of original recipients we added */
    pReport->recipDataList.count =
	pPerRecipData - &pReport->recipDataList.data[0];

    /* Give 'em what they came for */
    *ppContent = pReport;

    return Success;
}



static ReturnCode
getRfc822Address(ADDRESS * pAddr,
		 EMSD_ORAddress * pORAddress)
{
    ReturnCode      rc;
    char            buf[MAILTMPLEN];
    
    /* Initialize our temporary buffer since it gets cat'ed to */
    *buf = '\0';
    
    /* Convert the address to printable format */
    getPrintableRfc822Address(buf, pAddr);
    
    if (mtsua_getEmsdAddrFromRfc822(buf, &pORAddress->un.local.emsdAddress, pAddr) == Success)
    {
	/* We found an address we could convert to EMSD Local format */
        printf ("--- LOCAL EMSD ADDR ---\n");
	pORAddress->choice = EMSD_ORADDRESS_CHOICE_LOCAL;
    }
    else
    {
        printf ("--- REMOTE EMSD ADDR ---\n");
	/* It's not an EMSD Local address.  Use RFC-822 format */
	pORAddress->choice = EMSD_ORADDRESS_CHOICE_RFC822;

	/* Make sure STR_assignZString() allocates memory for us */
	pORAddress->un.rfc822 = NULL;
    
	/* Assign the printable address to this new string */
	if ((rc = STR_assignZString(&pORAddress->un.rfc822,
				    buf)) != Success)
	{
	    return FAIL_RC(rc, ("Could not assign printable address to String "));
	}
    }
    return Success;
}


static OS_Boolean
rfc822AddressesDiffer(ADDRESS * p1, ADDRESS * p2)
{
#undef INVALIDATE
#define	INVALIDATE(x1, x2)			\
    if (((x1 == NULL) ^ (x2 == NULL)) ||	\
	((x1 != NULL) && strcmp(x1, x2) != 0))	\
	return TRUE;

    INVALIDATE(p1->personal, p2->personal);
    INVALIDATE(p1->adl, p2->adl);
    INVALIDATE(p1->mailbox, p2->mailbox);
    INVALIDATE(p1->host, p2->host);

    return FALSE;
#undef INVALIDATE
}


static ReturnCode
parseHeaderString(char *pHeaderStr,
		  EMSD_IpmHeading * pHeading)
{
    ReturnCode      rc;
    ASN_Count       i;
    OS_Boolean      foundIt;
    char *          pEnd;
    char *          p1;
    char *	    p2;
    char            saveCh;
    EMSD_Extension  *pExtension;

    static struct IgnoreHeaderList
    {
	char           *pSearchString;
	int             length;
    } ignoreHeaderList[] =
    {
	{
	    ">From ",
	    sizeof(">From") - 1,
	},
	{
	    "BCC:",
	    sizeof("BCC:") - 1,
	},
	{
	    "CC:",
	    sizeof("CC:") - 1,
	},
	{
	    "Date:",
	    sizeof("Date:") - 1,
	},
	{
	    "From ",
	    sizeof("From ") - 1,
	},
	{
	    "From:",
	    sizeof("From:") - 1,
	},
	{
	    "In-Reply-To:",
	    sizeof("In-Reply-To:") - 1,
	},
	{
	    "Message-ID:",
	    sizeof("Message-ID:") - 1,
	},
	{
	    "Reply-To:",
	    sizeof("Reply-To:") - 1,
	},
	{
	    "Sender:",
	    sizeof("Sender:") - 1,
	},
	{
	    "Subject:",
	    sizeof("Subject:") - 1,
	},
	{
	    "To:",
	    sizeof("To:") - 1,
	},
	{
	    NULL
	}
    };
    struct IgnoreHeaderList *	pIgnoreHeaderList;

    static struct MimeHeaders
    {
	char * 			pSearchString;
	int 			length;
	OS_Boolean *		pExists;
	EMSD_AsciiString	*	phString;
    } mimeHeaders[] =
    {
	{
	    "Content-Description:",
	    sizeof("Content-Description:") - 1
	},
	{
	    "Content-Id:",
	    sizeof("Content-Id:") - 1
	},
	{
	    "Content-Transfer-Encoding:",
	    sizeof("Content-Transfer-Encoding:") - 1
	},
	{
	    "Content-Type:",
	    sizeof("Content-Type:") - 1
	},
	{
	    "MIME-Version:",
	    sizeof("MIME-Version:") - 1
	},
	{
	    NULL
	}
    };
    struct MimeHeaders *   pMimeHeaders;

    /* Initialize the AsciiString pointers in the Mime Headers */
    mimeHeaders[0].phString = &pHeading->mimeContentDescription.data;
    mimeHeaders[0].pExists = &pHeading->mimeContentDescription.exists;
    mimeHeaders[1].phString = &pHeading->mimeContentId.data;
    mimeHeaders[1].pExists = &pHeading->mimeContentId.exists;
    mimeHeaders[2].phString = &pHeading->mimeContentTransferEncoding.data;
    mimeHeaders[2].pExists = &pHeading->mimeContentTransferEncoding.exists;
    mimeHeaders[3].phString = &pHeading->mimeContentType.data;
    mimeHeaders[3].pExists = &pHeading->mimeContentType.exists;
    mimeHeaders[4].phString = &pHeading->mimeVersion.data;
    mimeHeaders[4].pExists = &pHeading->mimeVersion.exists;
    
    /* Strip carriage returns from the header string */
    for (p1 = p2 = pHeaderStr; *p2 != '\0'; )
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

    /* For each header item... */
    while (*pHeaderStr != '\0')
    {
	/* ... find the end of this item */
	if ((pEnd = strchr(pHeaderStr, '\n')) == NULL)
	{
	    pEnd = pHeaderStr + strlen(pHeaderStr);
	}

	/* If there's a space or tab following a new-line... */
	while (pEnd[0] == '\n' &&
	       (pEnd[1] == ' ' || pEnd[1] == '\t'))
	{
	    /* ... it's a continuation of the preceeding line */
	    ++pEnd;
	    if ((pEnd = strchr(pEnd, '\n')) == NULL)
	    {
		pEnd += strlen(pEnd);
	    }
	}
	
	/* See if we can find a match for this header item */
	foundIt = FALSE;
	for (pIgnoreHeaderList = ignoreHeaderList;
	     pIgnoreHeaderList->pSearchString != NULL;
	     pIgnoreHeaderList++)
	{
	    if (OS_strncasecmp(pIgnoreHeaderList->pSearchString,
			       pHeaderStr,
			       pIgnoreHeaderList->length) == 0)
	    {
		foundIt = TRUE;
		break;
	    }
	}
	
	/*
	 * If it wasn't a recognized header item in the c-client
	 * ENVELOPE...
	 */
	if (!foundIt)
	{
	    /*
	     * ... was it one of the MIME headers, which isn't in
	     * the c-client ENVELOPE?
	     */
	    foundIt = FALSE;
	    for (pMimeHeaders = mimeHeaders;
		 pMimeHeaders->pSearchString != NULL;
		 pMimeHeaders++)
	    {
		if (OS_strncasecmp(pMimeHeaders->pSearchString,
				   pHeaderStr,
				   pMimeHeaders->length) == 0)
		{
		    foundIt = TRUE;
		    break;
		}
	    }

	    if (foundIt)
	    {
		/* Specify that we've found this field */
		*pMimeHeaders->pExists = TRUE;

		/* Skip past the header name */
		pHeaderStr += pMimeHeaders->length;
		
		/* Skip past white space */
		for (;
		     *pHeaderStr != '\0' && *pHeaderStr != '\n';
		     pHeaderStr++)
		{
		    if (*pHeaderStr != ' ' && *pHeaderStr != '\t')
		    {
			break;
		    }
		}
		
		/* Make sure STR_assignZString() allocates for us */
		*pMimeHeaders->phString = NULL;
		
		saveCh = *pEnd;
		*pEnd = '\0';
		
		/* Assign the in-reply-to string to it */
		if ((rc = STR_assignZString(pMimeHeaders->phString,
					    pHeaderStr)) != Success)
		{
		    return FAIL_RC(rc,
				   ("Filling in IPM heading with %s",
				    pMimeHeaders->pSearchString));
		}
		
		*pEnd = saveCh;
	    }
	    else
	    {
		/*
		 * It's unrecognized.  Put it in the list of
		 * extensions.
		 */
		i = pHeading->extensions.count;
		
		
		/* Point to the next extension element */
		pExtension = &pHeading->extensions.data[i];
		
		/* Determine length of the label */
		if ((p1 = strchr(pHeaderStr, ':')) == NULL)
		{
		    /* should never occur */
		    break;
		}
		
		/* Make sure label ended in current header item */
		if (p1 > pEnd)
		{
		    /* should never occur */
		    break;
		}

		/* Null terminate the label */
		saveCh = *p1;
		*p1 = '\0';
		
		/* Make sure STR_assignZString() allocates for us */
		pExtension->label = NULL;
		
		/* Copy the label */
		if ((rc = STR_assignZString(&pExtension->label,
					    pHeaderStr)) != Success)
		{
		    return FAIL_RC(ResourceError,
				   ("Assigning extension label"));
		}
		
		/* Restore the character at the end */
		*p1 = saveCh;

		/* Scan past white space in the value */
		for (++p1; *p1 != '\0' && p1 < pEnd; p1++)
		{
		    if (*p1 != ' ' && *p1 != '\t')
		    {
			break;
		    }
		}
		
		saveCh = *pEnd;
		*pEnd = '\0';
		
		/* Make sure STR_assignZString() allocates for us */
		pExtension->value = NULL;
		
		/* Copy the value */
		if ((rc = STR_assignZString(&pExtension->value,
					    p1)) != Success)
		{
		    STR_free(pExtension->label);
		    return FAIL_RC(ResourceError,
				   ("Assigning extension value"));
		}
		
		/* Restore the character at the end */
		*pEnd = saveCh;
		
		/* increment the count of extensions */
		++pHeading->extensions.count;
	    }
	}
	
	/* Point to the next header item, if there is one */
	pHeaderStr = pEnd;
	if (*pHeaderStr != '\0')
	{
	    ++pHeaderStr;
	}
    }
    
    return Success;
}


static int
daysInMonth(int month, int year)
{
    static int d_i_m[] =
    {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    if(month == 2) 
    {
	return(((year % 4) == 0 && (year % 100) != 0) ? 29 : 28);
    }
    
    return(d_i_m[month]);
}


/*
 * parseRfc822Date()
 *
 * Parse date in or near RFC-822 format into a julian date.
 *
 * This is a modified version of PINE's parse_date() function.
 *
 * Args:
 *
 *         pDateStr --
 *                  The input string to parse
 *
 *         pJulianDate --
 *                 Pointer to a julian date to place the result in
 *
 * Returns nothing
 *
 * The following date fomrats are accepted:
 *   WKDAY DD MM YY HH:MM:SS ZZ
 *   DD MM YY HH:MM:SS ZZ
 *   WKDAY DD MM HH:MM:SS YY ZZ
 *   DD MM HH:MM:SS YY ZZ
 *   DD MM WKDAY HH:MM:SS YY ZZ
 *   DD MM WKDAY YY MM HH:MM:SS ZZ
 *
 * All leading, intervening and trailing spaces tabs and commas are
 * ignored.  The prefered formats are the first or second ones.  If
 * a field is unparsable it's value is left as -1.
 */
static ReturnCode
parseRfc822Date(char * pDateStr, OS_Uint32 * pJulianDate)
{
    char * 		p;
    char ** 		i;
    char * 		q;
    char 		n;
    int 		month;
    int			weekDay;
    int			hoursOffGmt;
    int			minutesOffGmt;
    int 		x;
    ReturnCode		rc;
    OS_ZuluDateTime	dateStruct;
    
    static char *   xdays[] =
    {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
    };
    
    static char *   xmonths[] =
    {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	NULL
    };
    
    /*
     * Structure containing all knowledge of symbolic time zones.
     * To add support for a given time zone, add it here, but make
     * sure the zone name is in upper case.
     */
    static struct
    {
	char * 		zone;
	short           len;
	short           hour_offset;
	short           min_offset;
    } known_zones[] =
    {
	{
	    "PST", 3, -8, 0
	}
	    ,			/* Pacific Standard */
	{
	    "PDT", 3, -7, 0
	}
	    ,			/* Pacific Daylight */
	{
	    "MST", 3, -7, 0
	}
	    ,			/* Mountain Standard */
	{
	    "MDT", 3, -6, 0
	}
	    ,			/* Mountain Daylight */
	{
	    "CST", 3, -6, 0
	}
	    ,			/* Central Standard */
	{
	    "CDT", 3, -5, 0
	}
	    ,			/* Central Daylight */
	{
	    "EST", 3, -5, 0
	}
	    ,			/* Eastern Standard */
	{
	    "EDT", 3, -4, 0
	}
	    ,			/* Eastern Daylight */
	{
	    "JST", 3, 9, 0
	}
	    ,			/* Japan Standard */
	{
	    "GMT", 3, 0, 0
	}
	    ,			/* Universal Time */
	{
	    "UT", 2, 0, 0
	}
	    ,			/* Universal Time */
#ifdef	IST_MEANS_ISREAL
	{
	    "IST", 3, 2, 0
	}
	    ,			/* Israel Standard */
#else
#ifdef	IST_MEANS_INDIA
	{
	    "IST", 3, 5, 30
	}
	    ,			/* India Standard */
#endif
#endif
	{
	    NULL, 0, 0
	}
	    ,
    };
    
    /* If we got a null string, return the empty structure */
    if (pDateStr == NULL)
    {
	return Fail;
    }
    
    /* Initialize the date structure */
    dateStruct.second = -1;
    dateStruct.minute = -1;
    dateStruct.hour = -1;
    dateStruct.day = -1;
    dateStruct.month = -1;
    dateStruct.year = -1;
    
    /* Initialize some locals */
    weekDay = 0;
    hoursOffGmt = 0;
    minutesOffGmt = 0;
    
    /* Point to the string to be parsed */
    p = pDateStr;
    
    /* Scan past white space */
    while (*p && isspace(*p))
    {
	p++;
    }
    
    /* Start with month, weekday or day ? */
    for (i = xdays; *i != NULL; i++)
    {
	/* Match first 3 letters */
	if (OS_strncasecmp(p, *i, 3) == 0)
	{
	    break;
	}
    }
    
    if (*i != NULL)
    {
	/* Started with week day */
	weekDay = i - xdays;
	while (*p && !isspace(*p) && *p != ',')
	{
	    p++;
	}
	while (*p && (isspace(*p) || *p == ','))
	{
	    p++;
	}
    }
    
    if (isdigit(*p))
    {
	dateStruct.day = atoi(p);
	while (*p && isdigit(*p))
	{
	    p++;
	}
	while (*p && (*p == '-' || *p == ',' || isspace(*p)))
	{
	    p++;
	}
    }
    
    for (month = 1; month <= 12; month++)
    {
	if (OS_strncasecmp(p, xmonths[month - 1], 3) == 0)
	{
	    break;
	}
    }
    
    if (month < 13)
    {
	dateStruct.month = month;
	
    }
    
    /* Move over month, (or whatever is there) */
    while (*p && !isspace(*p) && *p != ',' && *p != '-')
    {
	p++;
    }
    
    while (*p && (isspace(*p) || *p == ',' || *p == '-'))
    {
	p++;
    }
    
    /* Check again for day */
    if (isdigit(*p) && dateStruct.day == -1)
    {
	dateStruct.day = atoi(p);
	
	while (*p && isdigit(*p))
	{
	    p++;
	}
	
	while (*p && (*p == '-' || *p == ',' || isspace(*p)))
	{
	    p++;
	}
    }
    
    /* Check for time */
    for (q = p; *q && isdigit(*q); q++)
	;
    
    if (*q == ':')
    {
	/* It's the time (out of place) */
	dateStruct.hour = atoi(p);
	
	while (*p && *p != ':' && !isspace(*p))
	{
	    p++;
	}
	
	if (*p == ':')
	{
	    p++;
	    dateStruct.minute = atoi(p);
	    
	    while (*p && *p != ':' && !isspace(*p))
	    {
		p++;
	    }
	    
	    if (*p == ':')
	    {
		dateStruct.second = atoi(p);
		
		while (*p && !isspace(*p))
		{
		    p++;
		}
	    }
	}
	
	while (*p && isspace(*p))
	{
	    p++;
	}
    }
    /*
     * Get the year 0-49 is 2000-2049; 50-100 is 1950-1999 and
     * 101-9999 is 101-9999
     */
    if (isdigit(*p))
    {
	dateStruct.year = atoi(p);
	
	if (dateStruct.year < 50)
	{
	    dateStruct.year += 2000;
	}
	else if (dateStruct.year < 100)
	{
	    dateStruct.year += 1900;
	}
	
	while (*p && isdigit(*p))
	{
	    p++;
	}
	
	while (*p && (*p == '-' || *p == ',' || isspace(*p)))
	{
	    p++;
	}
    }
    else
    {
	/* Something wierd, skip it and try to resynch */
	while (*p && !isspace(*p) && *p != ',' && *p != '-')
	{
	    p++;
	}
	
	while (*p && (isspace(*p) || *p == ',' || *p == '-'))
	{
	    p++;
	}
    }
    
    /* Now get hours minutes, seconds and ignore tenths */
    for (q = p; *q && isdigit(*q); q++)
	;
    
    if (*q == ':' && dateStruct.hour == -1)
    {
	dateStruct.hour = atoi(p);
	
	while (*p && *p != ':' && !isspace(*p))
	{
	    p++;
	}
	
	if (*p == ':')
	{
	    p++;
	    dateStruct.minute = atoi(p);
	    
	    while (*p && *p != ':' && !isspace(*p))
	    {
		p++;
	    }
	    
	    if (*p == ':')
	    {
		p++;
		dateStruct.second = atoi(p);
		
		while (*p && !isspace(*p))
		{
		    p++;
		}
	    }
	}
    }
    
    while (*p && isspace(*p))
    {
	p++;
    }
    
    /* The time zone */
    hoursOffGmt = 0;
    minutesOffGmt = 0;
    
    if (*p)
    {
	if ((*p == '+' || *p == '-') &&
	    isdigit(p[1]) && isdigit(p[2]) &&
	    isdigit(p[3]) && isdigit(p[4]) && !isdigit(p[5]))
	{
	    char            tmp[3];
	    
	    minutesOffGmt = hoursOffGmt = (*p == '+' ? 1 : -1);
	    p++;
	    tmp[0] = *p++;
	    tmp[1] = *p++;
	    tmp[2] = '\0';
	    hoursOffGmt *= atoi(tmp);
	    tmp[0] = *p++;
	    tmp[1] = *p++;
	    tmp[2] = '\0';
	    minutesOffGmt *= atoi(tmp);
	}
	else
	{
	    for (n = 0; known_zones[(int) n].zone; n++)
	    {
		if (OS_strncasecmp(p,
				   known_zones[(int) n].zone,
				   known_zones[(int) n].len) == 0)
		{
		    hoursOffGmt = (int) known_zones[(int) n].hour_offset;
		    minutesOffGmt = (int) known_zones[(int) n].min_offset;
		    break;
		}
	    }
	}
    }
    
    /*
     * Now, convert the date to local time
     */
    if (hoursOffGmt == 0 && minutesOffGmt == 0)
    {
	/* Nothing to do */
	goto convertToJulian;
    }
    
    x = dateStruct.hour + (-1 * hoursOffGmt);
    if (x >= 0 && x < 24)
    {
	dateStruct.hour = x;
	goto convertToJulian;
    }
    
    if (x < 0)
    {
	dateStruct.hour += 24;
	
	/* Back one day */
	if (dateStruct.day > 2)
	{
	    dateStruct.day--;
	    goto convertToJulian;
	}
	
	/* Back one month */
	if (dateStruct.month > 2)
	{
	    dateStruct.month--;
	    dateStruct.day = daysInMonth(dateStruct.month,
					   dateStruct.year);
	    goto convertToJulian;
	}
	
	/* Back one year */
	dateStruct.year--;
	dateStruct.month = 12;
	dateStruct.day = 31;
    }
    else
    {
	/* Forward one day */
	dateStruct.hour -= 24;
	if (dateStruct.day != daysInMonth(dateStruct.month, dateStruct.year))
	{
	    dateStruct.day++;
	    goto convertToJulian;
	}
	
	/* Forward one month */
	if (dateStruct.month < 12)
	{
	    dateStruct.month++;
	    dateStruct.day = 1;
	    goto convertToJulian;
	}
	
	/* Forward a year */
	dateStruct.year++;
	dateStruct.month = 1;
	dateStruct.day = 1;
    }
    
 convertToJulian:

    if ((rc = OS_dateStructToJulian(&dateStruct, pJulianDate)) != Success)
    {
        printf ("parseRfc822Date: failed to convert to Julian [%d]\n", rc);   /*** fnh ***/
	return rc;
    }

    /* correct for GMT time difference */
    *pJulianDate -= timezone;

    printf ("parseRfc822Date: Current date (local) vs RFC-822 date =  %ld - %ld = %ld]\n",
	    time(NULL), *pJulianDate, time(NULL) - *pJulianDate);   /*** fnh ***/
    return Success;
}


ReturnCode
createMessageId(char * pRfc822MessageId)
{
    ReturnCode			rc;
    OS_Uint32			submissionTime;
    OS_ZuluDateTime		dateTime;

    /* Get the current time, for the submission time */
    if ((rc = OS_currentDateTime(&dateTime, &submissionTime)) != Success)
    {
	return rc;
    }

    /* See if it's time to reset the daily message number */
    if (dateTime.year != globals.dateLastAssignedMessageNumber.year ||
	dateTime.month != globals.dateLastAssignedMessageNumber.month ||
	dateTime.day != globals.dateLastAssignedMessageNumber.day)
    {
	/* It's a new day.  Store the current date, ... */
	globals.dateLastAssignedMessageNumber = dateTime;

	/* ... and reset the daily message number */
	globals.nextMessageNumber = 0;
    }

    /*
     * Create the message id for 'em, from the local identifier.
     *
     * The format that we use is as follows:
     *
     *     The User-name portion is composed of three
     *     components:
     *         The string "EMSD"
     *         The local Message Id's julian submission time
     *         The local Message Id's daily message number
     *
     *     The Domain-name portion is composed of one component:
     *         The local host name (sub-domain)
     */
    sprintf(pRfc822MessageId,
	    "<EMSD.%08lx.%08lx@%s>",
	    submissionTime,
	    globals.nextMessageNumber,
	    globals.config.pLocalHostName);

    /* Increment the next message number, for next time */
    ++globals.nextMessageNumber;

    return Success;
}


void
getPrintableRfc822Address(char * pBuf,
			  ADDRESS * pAddr)
{
    const char *rspecials =  "()<>@,;:\\\"[].";	  /* full RFC-822 specials */

    if (pAddr->mailbox != NULL && pAddr->host == NULL)
    {
	/* yes, write group name */
	rfc822_cat(pBuf, pAddr->mailbox, rspecials);
	strcat(pBuf, ": ");	/* write group identifier */
    }
    else
    {
	if (pAddr->host == NULL)
	{
	    /* end of group */
	    strcat(pBuf,";");
	}
	else if (pAddr->personal == NULL && pAddr->adl == NULL)
	{
	    /* simple case */
	    rfc822_address(pBuf, pAddr);
	}
	else
	{
	    /* must use phrase <route-addr> form */
	    if (pAddr->personal != NULL)
	    {
		rfc822_cat(pBuf, pAddr->personal, rspecials);
	    }

	    strcat(pBuf, " <");	/* write address delimiter */
	    rfc822_address(pBuf, pAddr); /* write address */
	    strcat(pBuf, ">");	/* closing delimiter */
	}
    }
}


