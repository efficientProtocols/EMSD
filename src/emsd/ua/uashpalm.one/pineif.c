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

#include "estd.h"
#include "nvm.h"
#include "emsdmail.h"
#include "emsd822.h"
#include "ua.h"
#include "uashloc.h"
#include "tm.h"

/*
 * Memory length required for a header and its data.  The extra bytes
 * are for ": " between header and data, CR/NL at end of line, and
 * optional TAB in case there are more multiple entries for this
 * header type.
 */
#define	LEN(header, h)							\
	((h) == NVM_NULL ? 0 :						\
	 strlen(header) + strlen((char *) NVM_getMemPointer(hTrans, h)) + 5)


static OS_Boolean	bInitialized = 0;

static char *		monthName[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


FORWARD static void
addRfc822Addr(Recipient * pRecip,
	      NVM_Transaction hTrans,
	      char * pHeader,
	      char * pDestBuf,
	      int * pLen);

FORWARD static ReturnCode
addEmsdAddress(NVM_TransInProcess hTip,
	      ADDRESS * pAddr,
	      Recipient * pRecip);

FORWARD static void
getPrintableRfc822Address(char * pBuf,
			  ADDRESS * pAddr);

FORWARD static long
countOctets(void * pCounter,
	    char * pString);

FORWARD static long
addOctets(void * ppData,
	  char * pString);

extern void
pine_encode_body (BODY *body);

extern long
pine_rfc822_output_body(BODY *body,
			long (*f)(void *, char *),
			void *s);

extern void *
fs_get(int size);

extern void
mm_log(char * string,
       long errflg);
#define	LOG(s)	mm_log(s, (long) 0);


/*
 * UASH_exists()
 *
 * Verify that an EMSD-1 format NVM file exists under the specified file name.
 */
int
UASH_exists(char * pNvmFileName)
{
    return TRUE;
}


/*
 * UASH_open()
 *
 * Initialize the NVM module and open the NVM file.
 */
int
UASH_open(char * pFileName,
	  int * phMailbox)
{
    char *	    p;
    ReturnCode	    rc;
    char	    tmp[OS_MAX_FILENAME_LEN];

    /* Copy the file name */
    strcpy(tmp, pFileName);

    /* The mailbox format is fileName:mailBoxName */
    if ((p = strrchr(tmp, '=')) == NIL)
    {
	LOG("UASH_open() equal not found");
	
	/* Invalid name */
	return Fail;
    }

    /* Null-terminate file name and update pointer to mailbox name */
    *p++ = '\0';

    /* Give 'em the mailbox handle */
    if (strcmp(p, "INBOX") == 0)
    {
	*phMailbox = TransType_InBox;
    }
    else if (strcmp(p, "OUTBOX") == 0)
    {
	*phMailbox = TransType_OutBox;
    }
    else
    {
	LOG("UASH_open() mailbox not INBOX or OUTBOX");

	/* Invalid mailbox name */
	return Fail;
    }

    /* If we've already been here, nothing else to do. */
    if (bInitialized)
    {
	return Success;
    }

    /* Initialize the TM module, as NVM needs it */
    TM_init();

    /* Initialize the NVM module */
    NVM_init();

    /* Open the specified NVM file name */
    if ((rc = NVM_open(tmp, 0)) != Success)
    {
	return Fail;
    }

    /* We're now initialized */
    bInitialized = TRUE;

    return Success;
}


/*
 * UASH_getMessage()
 *
 * Parse the message from NVM memory, and create an ENVELOPE and BODY
 * structure to give the the caller.
 */
int
UASH_getMessage(int hMailbox,
		long msgno,
		char ** ppHeader,
		char ** ppBody)
{
    ReturnCode			rc;
    int				len;
    Recipient * 		pRecip;
    RecipientInfo * 		pRecipInfo;
    Extension * 		pExtension;
    char *			p;
    char *			pText;
    char *			pTextStart;
    char			tmp[512];
    OS_ZuluDateTime 		zuluDateTime;
    Message *			pMessage;
    NVM_Transaction		hTrans;
    NVM_Memory			hElem;

    /* Find the message */
    for (hTrans = NVM_FIRST, rc = Success; msgno > 0 && rc == Success; msgno--)
    {
	rc = NVM_nextTrans((OS_Uint8) hMailbox, hTrans, &hTrans);
    }

    if (rc != Success)
    {
	LOG("UASH_getMessage() could not find specified message number");
	return Fail;
    }

    /* Point to the message structure */
    pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);
    
    /* Create the header text (if it was requested) */
    if (ppHeader != NULL)
    {
	/*
	 * We make two passes through the header of the message.  In
	 * the first pass, we determine how much memory to allocate
	 * for the printable rendition of the header of this message.
	 * In the second pass, after actually allocating the memory,
	 * we write the printable rendition into the allocated buffer.
	 */

	/***
	*** PASS 1 -- determine length
	***/

	/* Keep track of textual message length */
	len = 0;

	/* Add the Message Id string length */
	p = NVM_getMemPointer(hTrans, pMessage->hMessageId);

	if (pMessage->hMessageId != NVM_NULL)
	{
	    len += LEN("Message-Id", pMessage->hMessageId);
	}
	else
	{
	    len += strlen("Message-Id: <not yet determined>") + 3;
	}

	/* Add the submission time Date string */
	len += sizeof("Date: ") + 32;

	/* Add the originator */
	addRfc822Addr(&pMessage->originator, hTrans,
		      "From", tmp, &len);

	/* Add the printable submission status string */
	len += LEN("X-Submission-Attempts", pMessage->hPrintableStatus);

	/* Add the sender, if it exists.  Also, add the Return Path */
	if (pMessage->sender.style == RecipientStyle_Emsd ||
	    pMessage->sender.un.hRecipRfc822 != NVM_NULL)
	{
	    addRfc822Addr(&pMessage->sender, hTrans,
			  "Sender", tmp, &len);

	    /* Add the sender as the return_path also */
	    addRfc822Addr(&pMessage->sender, hTrans,
			  "Return-Path", tmp, &len);
	}
	else
	{
	    /* Add the originator as the return_path also */
	    addRfc822Addr(&pMessage->originator, hTrans,
			  "Return-Path", tmp, &len);
	}

	/* For each recipient... */
	for (NVM_quNext(hTrans, pMessage->hRecipInfoQHead, &hTrans, &hElem);
	     hElem != pMessage->hRecipInfoQHead;
	     NVM_quNext(hTrans, hElem, &hTrans, &hElem))
	{
	    pRecipInfo = NVM_getMemPointer(hTrans, hElem);

	    /* ... what type of recipient is he? */
	    switch(pRecipInfo->recipientType)
	    {
	    case RecipientType_Primary:
		/* He's a primary recipient */
		addRfc822Addr(&pRecipInfo->recipient, hTrans,
			      "To", tmp, &len);
		break;

	    case RecipientType_Copy:
		/* He's a copy recipient */
		addRfc822Addr(&pRecipInfo->recipient, hTrans,
			      "Cc", tmp, &len);
		break;

	    case RecipientType_BlindCopy:
		/* He's a blind copy recipient */
		addRfc822Addr(&pRecipInfo->recipient, hTrans,
			      "Bcc", tmp, &len);
		break;
	    }
	}

	/* If there were reply-to recipients specified... */
	for (NVM_quNext(hTrans, pMessage->hReplyToQHead, &hTrans, &hElem);
	     hElem != pMessage->hReplyToQHead;
	     NVM_quNext(hTrans, hElem, &hTrans, &hElem))
	{
	    pRecip = NVM_getMemPointer(hTrans, hElem);
	    addRfc822Addr(pRecip, hTrans,
			  "Reply-To", tmp, &len);
	}

	/* Add the replied-to-IPM */
	if (pMessage->hRepliedToMessageId != NVM_NULL)
	{
	    len += LEN("In-Reply-To", pMessage->hRepliedToMessageId);
	}

	/* Add the subject string */
	if (pMessage->hSubject != NVM_NULL)
	{
	    len += LEN("Subject", pMessage->hSubject);
	}

	/*
	 * Determine how much space is required for non-standard
	 * header fields
	 */
	len += (sizeof("Precedence: special-delivery") + 3 +
		sizeof("X-Importance: High") + 3 +
		sizeof("X-Auto-Forwarded: True") + 3);

	/*
	 * If there's a MIME Content Type specified, then we'll set the
	 * other MIME header fields.  Otherwise, we'll assume that this is
	 * not a MIME encoding, and we'll take the body text to be a
	 * simple text message.
	 */
	if (pMessage->hMimeContentType != NVM_NULL)
	{
	    /* Add the MIME version */
	    if (pMessage->hMimeVersion != NVM_NULL)
	    {
		len += LEN("MIME-Version", pMessage->hMimeVersion);
	    }
	    else
	    {
		len += sizeof("MIME-Version: 1.0") + 3;
	    }

	    /* Add the MIME content type */
	    len += LEN("Content-Type", pMessage->hMimeContentType);

	    /* Add the MIME content id */
	    len += LEN("Content-Id", pMessage->hMimeContentId);

	    /* Add the MIME content description */
	    len += LEN("Content-Description",
		       pMessage->hMimeContentDescription);

	    /* Add the MIME content transfer encoding */
	    len += LEN("Content-Transfer-Encoding",
		       pMessage->hMimeContentTransferEncoding);
	}

	/* Determine how much space is required for extension header fields */
	for (NVM_quNext(hTrans, pMessage->hExtensionQHead, &hTrans, &hElem);
	     hElem != pMessage->hExtensionQHead;
	     NVM_quNext(hTrans, hElem, &hTrans, &hElem))
	{
	    pExtension = NVM_getMemPointer(hTrans, hElem);
	    len += LEN(NVM_getMemPointer(hTrans, pExtension->hLabel),
		       pExtension->hValue);
	}

	/***
	*** PASS 2 -- allocate memory and copy data into said memory
	***/

	/* Allocate memory the the textual representation of the message */
	pText = pTextStart = fs_get(len + 1);

	/* Add the Message Id */
	if (pMessage->hMessageId != NVM_NULL)
	{
	    sprintf(pText, "Message-Id: %s\n",
		    (char *) NVM_getMemPointer(hTrans, pMessage->hMessageId));
	}
	else
	{
	    sprintf(pText, "Message-Id: <not yet determined>\n");
	}
	pText += strlen(pText);

	/* Add the submission time Date string */
	if ((rc = OS_julianToDateStruct(pMessage->submissionTime,
					&zuluDateTime)) != Success)
	{
	    LOG("UASH_getMessage() could not convert submission time");
	    return rc;
	}

	/* Create a properly-formatted date string */
	sprintf(pText,
		"Date: %02d %.3s %02d %02d:%02d:%02d GMT\n",
		zuluDateTime.day,
		monthName[zuluDateTime.month - 1],
		zuluDateTime.year % 100,
		zuluDateTime.hour,
		zuluDateTime.minute,
		zuluDateTime.second);
	pText += strlen(pText);

	/* Add the originator */
	addRfc822Addr(&pMessage->originator, hTrans,
		      "From", pText, &len);
	pText += strlen(pText);

	if (pMessage->hPrintableStatus != NVM_NULL)
	{
	    /* Add the printable submission status string */
	    sprintf(pText, "X-Submission-Attempts: %s\n",
		    (char *) NVM_getMemPointer(hTrans,
					       pMessage->hPrintableStatus));
	    pText += strlen(pText);
	}
	
	/* Add the sender, if it exists.  Also, add the Return Path */
	if (pMessage->sender.style == RecipientStyle_Emsd ||
	    pMessage->sender.un.hRecipRfc822 != NVM_NULL)
	{
	    addRfc822Addr(&pMessage->sender, hTrans,
			  "Sender", pText, &len);
	    pText += strlen(pText);

	    /* Add the sender as the return_path also */
	    addRfc822Addr(&pMessage->sender, hTrans,
			  "Return-Path", pText, &len);
	    pText += strlen(pText);
	}
	else
	{
	    /* Add the originator as the return_path also */
	    addRfc822Addr(&pMessage->originator, hTrans,
			  "Return-Path", pText, &len);
	    pText += strlen(pText);
	}

	/* For each recipient... */
	for (NVM_quNext(hTrans, pMessage->hRecipInfoQHead, &hTrans, &hElem);
	     hElem != pMessage->hRecipInfoQHead;
	     NVM_quNext(hTrans, hElem, &hTrans, &hElem))
	{
	    pRecipInfo = NVM_getMemPointer(hTrans, hElem);

	    /* ... what type of recipient is he? */
	    switch(pRecipInfo->recipientType)
	    {
	    case RecipientType_Primary:
		/* He's a primary recipient */
		addRfc822Addr(&pRecipInfo->recipient, hTrans,
			      "To", pText, &len);
		break;

	    case RecipientType_Copy:
		/* He's a copy recipient */
		addRfc822Addr(&pRecipInfo->recipient, hTrans,
			      "Cc", pText, &len);
		break;

	    case RecipientType_BlindCopy:
		/* He's a blind copy recipient */
		addRfc822Addr(&pRecipInfo->recipient, hTrans,
			      "Bcc", pText, &len);
		break;
	    }

	    pText += strlen(pText);
	}

	/* If there were reply-to recipients specified... */
	for (NVM_quNext(hTrans, pMessage->hReplyToQHead, &hTrans, &hElem);
	     hElem != pMessage->hReplyToQHead;
	     NVM_quNext(hTrans, hElem, &hTrans, &hElem))
	{
	    pRecip = NVM_getMemPointer(hTrans, hElem);
	    addRfc822Addr(pRecip, hTrans,
			  "Reply-To", pText, &len);
	    pText += strlen(pText);
	}

	/* Add the replied-to-IPM */
	if (pMessage->hRepliedToMessageId != NVM_NULL)
	{
	    p = NVM_getMemPointer(hTrans, pMessage->hRepliedToMessageId);
	    sprintf(pText, "In-Reply-To: %s\n", p);
	    pText += strlen(pText);
	}

	/* Add the subject string */
	if (pMessage->hSubject != NVM_NULL)
	{
	    p = NVM_getMemPointer(hTrans, pMessage->hSubject);
	    sprintf(pText, "Subject: %s\n", p);
	    pText += strlen(pText);
	}

	/* Add each of the extension (unrecognized) fields */
	for (NVM_quNext(hTrans, pMessage->hExtensionQHead, &hTrans, &hElem);
	     hElem != pMessage->hExtensionQHead;
	     NVM_quNext(hTrans, hElem, &hTrans, &hElem))
	{
	    /* Point to this extension element */
	    pExtension = NVM_getMemPointer(hTrans, hElem);

	    /* Add this extension label and value */
	    sprintf(pText, "%s: %s\n",
		    (char *) NVM_getMemPointer(hTrans, pExtension->hLabel),
		    (char *) NVM_getMemPointer(hTrans, pExtension->hValue));
	    pText += strlen(pText);
	}

	/*
	 * If there's a MIME Content Type specified, then we'll set the
	 * other MIME header fields.  Otherwise, we'll assume that this is
	 * not a MIME encoding, and we'll take the body text to be a
	 * simple text message.
	 */
	if (pMessage->hMimeContentType != NVM_NULL)
	{
	    /* Add the MIME version */
	    if (pMessage->hMimeVersion != NVM_NULL)
	    {
		sprintf(pText, "MIME-Version: %s\n",
			(char *) NVM_getMemPointer(hTrans,
						   pMessage->hMimeVersion));
	    }
	    else
	    {
		strcpy(pText, "MIME-Version: 1.0\n");
	    }

	    pText += strlen(pText);

	    /* Add the MIME content type */
	    sprintf(pText, "Content-Type: %s\n",
		    (char *) NVM_getMemPointer(hTrans,
					       pMessage->hMimeContentType));
	    pText += strlen(pText);

	    /* Add the MIME content id */
	    if (pMessage->hMimeContentId != NVM_NULL)
	    {
		sprintf(pText, "Content-Id: %s\n",
			(char *) NVM_getMemPointer(hTrans,
						   pMessage->hMimeContentId));
		pText += strlen(pText);
	    }

	    /* Add the MIME content description */
	    if (pMessage->hMimeContentDescription != NVM_NULL)
	    {
		sprintf(pText, "Content-Description: %s\n",
			(char *)
			NVM_getMemPointer(hTrans,
					  pMessage->hMimeContentDescription));
		pText += strlen(pText);
	    }

	    /* Add the MIME content transfer encoding */
	    if (pMessage->hMimeContentTransferEncoding != NVM_NULL)
	    {
		sprintf(pText,
			"Content-Transfer-Encoding: %s\n",
			(char *)
			NVM_getMemPointer(hTrans,
					  pMessage->hMimeContentTransferEncoding));
		pText += strlen(pText);
	    }
	}

	/*
	 * Add per-message flag header fields
	 */

	/* If the Priority isn't "Normal", add it. */
	switch(pMessage->priority)
	{
	case Priority_Normal:
	    /* Do nothing in this case */
	    break;

	case Priority_NonUrgent:
	    /* Add a Priority header line */
	    strcpy(pText, "Precedence: bulk\n");
	    pText += strlen(pText);
	    break;
	
	case Priority_Urgent:
	    /* Add a Priority header line */
	    strcpy(pText, "Precedence: special-delivery\n");
	    pText += strlen(pText);
	    break;
	}
    
	/* If the Importance isn't "Normal", add it. */
	switch(pMessage->importance)
	{
	case Importance_Normal:
	    /* Do nothing in this case */
	    break;
	
	case Importance_Low:
	    /* Add an Importance header line */
	    strcpy(pText, "X-Importance: Low\n");
	    pText += strlen(pText);
	    break;
	
	case Importance_High:
	    /* Add an Importance header line */
	    strcpy(pText, "X-Importance: High\n");
	    pText += strlen(pText);
	    break;
	}

	/* If the message was Auto-Forwarded, indicate such. */
	if (pMessage->bAutoForwarded)
	{
	    /* Add an Auto-Forwarded header line */
	    strcpy(pText, "X-Auto-Forwarded: True\n");
	    pText += strlen(pText);
	}
    
	/* Give 'em the header pointer */
	*ppHeader = pTextStart;
    }

    /* Make a copy of the body (if it was requested) */
    if (ppBody != NULL)
    {
	if (pMessage->hBody == NVM_NULL)
	{
	    pText = fs_get(1);
	    *pText = '\0';
	}
	else
	{
	    p = NVM_getMemPointer(hTrans, pMessage->hBody);
	    pText = fs_get(strlen(p) + 1);
	    strcpy(pText, p);
	}

	/* Give 'em the body pointer */
	*ppBody = pText;
    }

    return 0;
}


/*
 * UASH_expungeMessage()
 *
 * Expunge the specified message from the mailbox.
 */
void
UASH_expungeMessage(int hMailbox,
		    long msgno)
{
    ReturnCode		rc;
    NVM_Transaction	hTrans;

    /* Find the message */
    for (hTrans = NVM_FIRST, rc = Success; msgno > 0 && rc == Success; msgno--)
    {
	rc = NVM_nextTrans((OS_Uint8) hMailbox, hTrans, &hTrans);
    }

    if (rc != Success)
    {
	return;
    }

    (void) NVM_freeTrans(hTrans);

    /* Make sure our (possibly) copied version matches the real one */
    NVM_sync();
}


/*
 * UASH_countMessages()
 *
 * Count the number of messages in a mailbox.
 */
long
UASH_countMessages(int hMailbox)
{
    ReturnCode		rc;
    long		numMessages;
    NVM_Transaction	hTrans;

    /* Find the message */
    for (hTrans = NVM_FIRST, rc = Success, numMessages = 0;
	 rc == Success;
	 numMessages++)
    {
	rc = NVM_nextTrans((OS_Uint8) hMailbox, hTrans, &hTrans);
    }

    return numMessages - 1;
}


/*
 * UASH_getSubmissionTime()
 *
 * Return the specified message's submission time in the format:
 *
 *    dd mmm yy hh:mm:ss GMT
 */
void
UASH_getSubmissionTime(int hMailbox,
		       long msgno,
		       char * pDateBuf)
{
    ReturnCode		rc;
    Message *		pMessage;
    NVM_Transaction	hTrans;
    OS_ZuluDateTime 	zuluDateTime;

    /* Find the message */
    for (hTrans = NVM_FIRST, rc = Success; msgno > 0 && rc == Success; msgno--)
    {
	rc = NVM_nextTrans((OS_Uint8) hMailbox, hTrans, &hTrans);
    }

    if (rc != Success)
    {
	*pDateBuf = '\0';;
	return;
    }

    /* Point to the message structure */
    pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);
    
    /* Add the submission time Date string */
    if ((rc = OS_julianToDateStruct(pMessage->submissionTime,
				    &zuluDateTime)) != Success)
    {
	*pDateBuf = '\0';;
	return;
    }

    /* Create a properly-formatted date string */
    sprintf(pDateBuf,
	    "%02d %.3s %02d %02d:%02d:%02d GMT\n",
	    zuluDateTime.day,
	    monthName[zuluDateTime.month - 1],
	    zuluDateTime.year % 100,
	    zuluDateTime.hour,
	    zuluDateTime.minute,
	    zuluDateTime.second);
}


/*
 * UASH_getFlags()
 *
 * Get the flags for a message
 */
unsigned long
UASH_getFlags(int hMailbox,
	      long msgno)
{
    ReturnCode		rc;
    Message *		pMessage;
    NVM_Transaction	hTrans;

    /* Find the message */
    for (hTrans = NVM_FIRST, rc = Success; msgno > 0 && rc == Success; msgno--)
    {
	rc = NVM_nextTrans((OS_Uint8) hMailbox, hTrans, &hTrans);
    }

    if (rc != Success)
    {
	return 0;
    }

    /* Point to the message structure */
    pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);

    return pMessage->flags;
}


/*
 * UASH_setFlags()
 *
 * Set the flags for a message
 */
void
UASH_setFlags(int hMailbox,
	      long msgno,
	      unsigned long flags)
{
    ReturnCode		rc;
    Message *		pMessage;
    NVM_Transaction	hTrans;

    /* Find the message */
    for (hTrans = NVM_FIRST, rc = Success; msgno > 0 && rc == Success; msgno--)
    {
	rc = NVM_nextTrans((OS_Uint8) hMailbox, hTrans, &hTrans);
    }

    if (rc != Success)
    {
	LOG("UASH_setFlags() could not find specified msgno");
	return;
    }

    /* Point to the message structure */
    pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);


    /* Set the flags */
    pMessage->flags = flags;

    /* Make sure our (possibly) copied version matches the real one */
    NVM_sync();
}


int
UASH_submitMessage(METAENV * pRfc822Env,
		   BODY * pRfc822Body)
{
    int				bodyCount;
    char *			p;
    char *			pTmp;
    char			tmp[512];
    Message * 			pMessage;
    Recipient * 		pRecip;
    RecipientInfo * 		pRecipInfo;
    Extension *			pExtension;
    ADDRESS * 			pAddr;
    NVM_TransInProcess		hTip;
    NVM_Transaction		hTrans;
    NVM_Transaction		h;
    ReturnCode 			rc;
    PINEFIELD *			pPineField;
    PARAMETER *			pParam;
    extern const char *		tspecials;
    
    /* Begin a non-volatile memory transaction */
    if ((rc = NVM_beginTrans(TransType_OutBox, &hTip, &hTrans)) != Success)
    {
	return rc;
    }

    /* Obtain a new Message structure handle */
    if ((rc = NVM_alloc(hTip, sizeof(Message), &h)) != Success)
    {
	(void) NVM_abortTrans(hTip);
	return rc;
    }

    /* Point to the Message structure */
    pMessage = NVM_getMemPointer(hTrans, h);

    /* Message Id not specified by UA */
    pMessage->hMessageId = NVM_NULL;

    /* Submission Time / Delivery Time not specified by UA */
    pMessage->submissionTime = 0;
    pMessage->deliveryTime = 0;

    /* Allocate memory for a submission status string */
    if ((rc = NVM_alloc(hTip, UASH_SUBMISSION_STATUS_SIZE,
			&pMessage->hPrintableStatus)) != Success)
    {
	(void) NVM_abortTrans(hTip);
	return rc;
    }
    
    /* Indicate that we haven't yet attempted a submission */
    strcpy((char *) NVM_getMemPointer(hTrans, pMessage->hPrintableStatus),
	   "0");

    /*
     * Fill in the heading from the RFC-822 envelope info
     */
    
    /* Get the originator */
    if ((rc = addEmsdAddress(hTip,
			    pRfc822Env->env->from,
			    &pMessage->originator)) != Success)
    {
	(void) NVM_abortTrans(hTip);
	return rc;
    }

    /* Get the sender */
    if (pRfc822Env->env->sender != NULL)
    {
	if ((rc = addEmsdAddress(hTip,
				pRfc822Env->env->sender,
				&pMessage->sender)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}
    }
    
    /* Allocate the recipient info queue head */
    if ((rc = NVM_alloc(hTip, sizeof(NVM_QuHead),
			&pMessage->hRecipInfoQHead)) != Success)
    {
	(void) NVM_abortTrans(hTip);
	return rc;
    }

    /* Initialize the queue */
    NVM_quInit(hTrans, pMessage->hRecipInfoQHead);

    /* Get the primary (To:) recipients */
    for (pAddr = pRfc822Env->env->to; pAddr != NULL; pAddr = pAddr->next)
    {
	/* Allocate a RecipientInfo structure */
	if ((rc = NVM_alloc(hTip, sizeof(RecipientInfo), &h)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	pRecipInfo = NVM_getMemPointer(hTrans, h);

	/* Initialize the RecipientInfo structure */
	pRecipInfo->recipientType = RecipientType_Primary;
	pRecipInfo->bReceiptNotificationRequested = FALSE;
	pRecipInfo->bNonReceiptNotificationRequested = FALSE;
	pRecipInfo->bMessageReturnRequested = FALSE;
	pRecipInfo->bNonDeliveryReportRequested = TRUE;
	pRecipInfo->bDeliveryReportRequested = FALSE;
	pRecipInfo->bReplyRequested = FALSE;

	/* Get the recipient address */
	if ((rc = addEmsdAddress(hTip,
				pAddr, &pRecipInfo->recipient)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	/* Initialize the recipient info queue pointers */
	NVM_quInit(hTrans, h);

	/* Insert this recipient onto the queue of recipients */
	NVM_quInsert(hTrans, h, hTrans, pMessage->hRecipInfoQHead);
    }
    
    /* Get the copy (Cc:) recipients */
    for (pAddr = pRfc822Env->env->cc; pAddr != NULL; pAddr = pAddr->next)
    {
	/* Allocate a RecipientInfo structure */
	if ((rc = NVM_alloc(hTip, sizeof(RecipientInfo), &h)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	pRecipInfo = NVM_getMemPointer(hTrans, h);
	
	/* Initialize the RecipientInfo structure */
	pRecipInfo->recipientType = RecipientType_Copy;
	pRecipInfo->bReceiptNotificationRequested = FALSE;
	pRecipInfo->bNonReceiptNotificationRequested = FALSE;
	pRecipInfo->bMessageReturnRequested = FALSE;
	pRecipInfo->bNonDeliveryReportRequested = TRUE;
	pRecipInfo->bDeliveryReportRequested = FALSE;
	pRecipInfo->bReplyRequested = FALSE;

	/* Get the recipient address */
	if ((rc = addEmsdAddress(hTip,
				pAddr, &pRecipInfo->recipient)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	/* Initialize the recipient info queue pointers */
	NVM_quInit(hTrans, h);

	/* Insert this recipient onto the queue of recipients */
	NVM_quInsert(hTrans, h, hTrans, pMessage->hRecipInfoQHead);
    }
    
    /* Get the blind copy (Bcc:) recipients */
    for (pAddr = pRfc822Env->env->bcc; pAddr != NULL; pAddr = pAddr->next)
    {
	/* Allocate a RecipientInfo structure */
	if ((rc = NVM_alloc(hTip, sizeof(RecipientInfo), &h)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	pRecipInfo = NVM_getMemPointer(hTrans, h);

	/* Initialize the RecipientInfo structure */
	pRecipInfo->recipientType = RecipientType_BlindCopy;
	pRecipInfo->bReceiptNotificationRequested = FALSE;
	pRecipInfo->bNonReceiptNotificationRequested = FALSE;
	pRecipInfo->bMessageReturnRequested = FALSE;
	pRecipInfo->bNonDeliveryReportRequested = TRUE;
	pRecipInfo->bDeliveryReportRequested = FALSE;
	pRecipInfo->bReplyRequested = FALSE;

	/* Get the recipient address */
	if ((rc = addEmsdAddress(hTip,
				pAddr, &pRecipInfo->recipient)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	/* Initialize the recipient info queue pointers */
	NVM_quInit(hTrans, h);

	/* Insert this recipient onto the queue of recipients */
	NVM_quInsert(hTrans, h, hTrans, pMessage->hRecipInfoQHead);
    }
    
    /* Allocate the reply-to queue head */
    if ((rc = NVM_alloc(hTip, sizeof(NVM_QuHead),
			&pMessage->hReplyToQHead)) != Success)
    {
	(void) NVM_abortTrans(hTip);
	return rc;
    }

    /* Initialize the queue */
    NVM_quInit(hTrans, pMessage->hReplyToQHead);

    /* Get the reply-to recipients. */
    for (pAddr = pRfc822Env->env->reply_to; pAddr != NULL; pAddr = pAddr->next)
    {
	/* Allocate a Recipient structure */
	if ((rc = NVM_alloc(hTip, sizeof(Recipient), &h)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	pRecip = NVM_getMemPointer(hTrans, h);

	/* Get the recipient address */
	if ((rc = addEmsdAddress(hTip,
				pAddr, pRecip)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	/* Initialize the recipient info queue pointers */
	NVM_quInit(hTrans, h);

	/* Insert this recipient onto the queue of recipients */
	NVM_quInsert(hTrans, h, hTrans, pMessage->hReplyToQHead);
    }
    
    /* Get the replied-to message id */
    if ((rc = NVM_allocStringCopy(hTip,
				  pRfc822Env->env->in_reply_to,
				  &pMessage->hRepliedToMessageId)) != Success)
    {
	(void) NVM_abortTrans(hTip);
	return rc;
    }
    
    /* Get the subject */
    if ((rc = NVM_allocStringCopy(hTip,
				  pRfc822Env->env->subject,
				  &pMessage->hSubject)) != Success)
    {
	(void) NVM_abortTrans(hTip);
	return rc;
    }

    /* Initialize optional header fields */
    pMessage->priority = Priority_Normal;
    pMessage->importance = Importance_Normal;
    pMessage->hMimeVersion = NVM_NULL;
    pMessage->hMimeContentId = NVM_NULL;
    pMessage->hMimeContentType = NVM_NULL;
    pMessage->hMimeContentDescription = NVM_NULL;
    pMessage->hMimeContentTransferEncoding = NVM_NULL;

    /* Initialize flags.  This is a new message. */
    pMessage->flags = 0;
    pMessage->status.hSubmission = NULL;

    /* Allocate and Initialize the extension queue head */
    if ((rc = NVM_alloc(hTip, sizeof(NVM_QuHead),
			&pMessage->hExtensionQHead)) != Success)
    {
	(void) NVM_abortTrans(hTip);
	return rc;
    }

    /* Initialize the queue */
    NVM_quInit(hTrans, pMessage->hExtensionQHead);

    /* See if there's a "Precedence:" field that we can convert to priority */
    for (pPineField = pRfc822Env->custom;
	 pPineField != NULL && pPineField->name != NULL;
	 pPineField = pPineField->next)
    {
	if (pPineField->type == FreeText)
	{
	    /* It's not one that we care about.  Make it an extension. */
	    if ((rc = NVM_alloc(hTip,
				sizeof(Extension),
				&h)) != Success)
	    {
		(void) NVM_abortTrans(hTip);
		return rc;
	    }

	    pExtension = NVM_getMemPointer(hTrans, h);

	    if ((rc = NVM_allocStringCopy(hTip,
					  pPineField->name,
					  &pExtension->hLabel)) != Success)
	    {
		(void) NVM_abortTrans(hTip);
		return rc;
	    }

	    if ((rc = NVM_allocStringCopy(hTip,
					  pPineField->text,
					  &pExtension->hValue)) != Success)
	    {
		(void) NVM_abortTrans(hTip);
		return rc;
	    }

	    /* Initialize the extension queue pointers */
	    NVM_quInit(hTrans, h);
		
	    /* Insert this extension the queue of extensions */
	    NVM_quInsert(hTrans, h,
			 hTrans, pMessage->hExtensionQHead);
	}
    }

    /* Get the body into a format we can play with */
    pine_encode_body(pRfc822Body);

    /*
     * To save bits going over the wire, we only encode MIME fields if we're
     * getting a message with something other than a TEXT body part.
     */
    if (pRfc822Body->type != TYPETEXT)
    {
	/* Add the MIME Version */
	if ((rc =
	     NVM_allocStringCopy(hTip,
				 "1.0",
				 &pMessage->hMimeVersion)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	/* Add the Content Type */
	pTmp = tmp;
	sprintf (pTmp, "%s", body_types[pRfc822Body->type]);
	p = (pRfc822Body->subtype ?
	     pRfc822Body->subtype : rfc822_default_subtype(pRfc822Body->type));
	sprintf (pTmp += strlen(pTmp), "/%s", p);

	if ((pParam = pRfc822Body->parameter) != NULL)
	{
	    for (; pParam != NULL; pParam = pParam->next)
	    {
		sprintf (pTmp += strlen(pTmp), "; %s=", pParam->attribute);
		rfc822_cat(pTmp, pParam->value, tspecials);
	    }
	}
	else if (pRfc822Body->type == TYPETEXT)
	{
	    strcat(pTmp, "; charset=US-ASCII");
	}
	
	strcpy(pTmp += strlen(pTmp), "\015\012");

	if ((rc =
	     NVM_allocStringCopy(hTip, tmp,
				 &pMessage->hMimeContentType)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}

	/* Add the Content Transfer Encoding */
	if (pRfc822Body->encoding)	/* note: encoding 7BIT never output! */
	{
	    sprintf(tmp, "%s\015\012",
		    body_encodings[((pRfc822Body->encoding == ENCBINARY ?
				     ENCBASE64 :
				     (pRfc822Body->encoding == ENC8BIT ?
				      ENCQUOTEDPRINTABLE :
				      (pRfc822Body->encoding <= ENCMAX ?
				       pRfc822Body->encoding : ENCOTHER))))]);
	    
	    if ((rc =
		 NVM_allocStringCopy(hTip,
				     tmp,
				     &pMessage->
				     hMimeContentTransferEncoding)) != Success)
	    {
		(void) NVM_abortTrans(hTip);
		return rc;
	    }
	}

	/* Add the Content Id */
	if (pRfc822Body->id)
	{
	    sprintf(tmp, "%s\015\012", pRfc822Body->id);

	    if ((rc =
		 NVM_allocStringCopy(hTip,
				     tmp,
				     &pMessage->hMimeContentId)) != Success)
	    {
		(void) NVM_abortTrans(hTip);
		return rc;
	    }
	}
	
	/* Add the Content Description */
	if (pRfc822Body->description)
	{
	    sprintf (tmp, "%s\015\012", pRfc822Body->description);

	    if ((rc =
		 NVM_allocStringCopy(hTip,
				     tmp,
				     &pMessage->hMimeContentDescription)) !=
		Success)
	    {
		(void) NVM_abortTrans(hTip);
		return rc;
	    }
	}
    }

    /*
     * Format the body into its textual form.  We'll do this in two
     * passes for now.  This should get optimized later.
     *
     * Pass 1 counts the number of bytes required for the message.
     *
     * Pass 2 writes the data into an NVM-allocated buffer.
     *
     */

    /* Pass 1 */
    bodyCount = 0;
    pine_rfc822_output_body(pRfc822Body, countOctets, &bodyCount);

    /* Pass 2 */
    if (bodyCount > 0)
    {
	if ((rc = NVM_alloc(hTip, bodyCount + 1, &pMessage->hBody)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    return rc;
	}
	p = NVM_getMemPointer(hTrans, pMessage->hBody);
	pine_rfc822_output_body(pRfc822Body, addOctets, &p);
    }
    else
    {
	pMessage->hBody = NVM_NULL;
    }

    /* Submit this message to the UASH */
    NVM_endTrans(hTip);

    return Success;
}



static ReturnCode
addEmsdAddress(NVM_TransInProcess hTip,
	      ADDRESS * pAddr,
	      Recipient * pRecip)
{
    int		    lowOrder = 0;
    int		    highOrder = 0;
    char *	    p;
    char *	    pEnd;
    char	    tmpbuf[MAILTMPLEN];
    char	    saveCh;
    ReturnCode	    rc;
    OS_Boolean	    bGotAddr = FALSE;

    /* Null-terminate the buffer at the beginning, as address is cat'ed */
    *tmpbuf = '\0';

    /* Convert the address to printable format */
    getPrintableRfc822Address(tmpbuf, pAddr);

    /* Skip the comment portion of the address */
    if ((p = strchr(tmpbuf, '<')) == NULL)
    {
	p = tmpbuf;
	pEnd = p + strlen(p);
	saveCh = '\0';
    }
    else
    {
	++p;

	if ((pEnd = strchr(p, '>')) == NULL)
	{
	    pEnd = p + strlen(p);
	}

	/* Save character following string */
	saveCh = *pEnd;

	/* Temporarily null-terminate string */
	*pEnd = '\0';
    }

    /* See if it's an EMSD Local-format address */
    if (strncmp(p, "EMSD:", 4) == 0)
    {
        if (isdigit(*p))
	{
	    highOrder = atoi(p);
	    for (; isdigit(*p); p++)
	        ;
	    if (*p == '.' && isdigit(p[1]))
	    {
	        lowOrder = atoi(++p);
		bGotAddr = TRUE;
	    }
	}
    }

    if (bGotAddr)
    {
	/* Yup.  Create the recipient structure */
	pRecip->style = RecipientStyle_Emsd;

	/* Create the EMSD Address value */
	pRecip->un.recipEmsd.emsdAddr = (highOrder * 1000) + lowOrder;

	/* Restore modified terminator */
	*pEnd = saveCh;
    }
    else
    {
	/* It's not an EMSD Local-format address. */
	pRecip->style = RecipientStyle_Rfc822;

	/* Restore modified terminator */
	*pEnd = saveCh;

	/* Allocate space for the RFC-822 address */
	if ((rc =
	     NVM_allocStringCopy(hTip, tmpbuf,
				 &pRecip->un.hRecipRfc822)) != Success)
	{
	    return rc;
	}
    }

    return Success;
}



static void
getPrintableRfc822Address(char * pBuf,
			  ADDRESS * pAddr)
{
    const char *rspecials =  "()<>@,;:\\\"[].";
				/* full RFC-822 specials */

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


static long
countOctets(void * pCounter,
	    char * pString)
{
    *(int *) pCounter += strlen(pString);

    return 1L;
}


static long
addOctets(void * ppData,
	  char * pString)
{
    strcpy(*(char **) ppData, pString);
    *(char **) ppData += strlen(pString);

    return 1L;
}


static void
addRfc822Addr(Recipient * pRecip,
	      NVM_Transaction hTrans,
	      char * pHeader,
	      char * pDestBuf,
	      int * pLen)
{
    char *		pBuf = pDestBuf;
    static char		prevHeader[32];

    /* Is this header the same as the previous one? */
    if (strcmp(pHeader, prevHeader) == 0)
    {
	/* Yup.  Make it a continuation line. */
	*pBuf++ = ' ';
    }
    else
    {
	/* It's a new header.  Add it. */
	sprintf(pBuf, "%s: ", pHeader);
	pBuf += strlen(pBuf);

	/* Remember what this header was, for next time. */
	strcpy(prevHeader, pHeader);
    }

    if (pRecip->style == RecipientStyle_Emsd)
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
	 *         The local address value, split into two values separated by
	 *         dots.  The first value is the local address divided by
	 *         1000; the second is the local address modulo 1000.
	 *
	 *     The Domain-name portion is unused.
	 */
	sprintf(pBuf, "%.*s <%03ld.%03ld>",
		(int) (pRecip->un.recipEmsd.hName != NVM_NULL ?
		       strlen(NVM_getMemPointer(hTrans,
						pRecip->un.recipEmsd.hName)) :
		       0),
		(pRecip->un.recipEmsd.hName != NVM_NULL ?
		 (char *) NVM_getMemPointer(hTrans,
					    pRecip->un.recipEmsd.hName) :
		 ""),
		pRecip->un.recipEmsd.emsdAddr / 1000,
		pRecip->un.recipEmsd.emsdAddr % 1000);

    }
    else
    {
	/* RFC-822 format */
	strcpy(pBuf,
	       NVM_getMemPointer(hTrans, pRecip->un.hRecipRfc822));
    }

    /* Add the line terminator */
    strcat(pBuf, "\n");

    /* Update the length */
    *pLen += strlen(pDestBuf) + 3;
}
