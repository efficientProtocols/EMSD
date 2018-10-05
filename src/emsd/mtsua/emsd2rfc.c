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


#include "emsdmail.h"
#include "emsd822.h"
#undef ERROR
#include "estd.h"
#include "strfunc.h"
#include "emsd.h"
#include "mtsua.h"
#include "tm.h"

static char 	tmp[MAILTMPLEN];


extern char *	cpystr (char * pString);

static void
addRfc822Addr(EMSD_ORAddress * pORAddr,
	      ADDRESS ** ppEnvAddr,
	      char * pLocalHostName);

static char *
getRfc822MessageId(EMSD_MessageId * pMsgId,
		   char * pLocalHostName);

static char *
copyCreateRfc822String(void * pSrc,
		       OS_Uint16 len);

static char *
copyString(char *pString);

void
createRfc822MessageIdFromEmsd(EMSD_MessageId * pMsgId,
			     char * pLocalHostName,
			     char * pBuf);



ReturnCode
mtsua_convertEmsdIpmToRfc822(EMSD_Message * pIpm,
			    ENVELOPE * pRfc822Env,
			    char * pExtraHeaders,
			    char * pLocalHostName)
{
    int			i;
    STR_String		hStr;
    EMSD_ORAddress *	pORAddr;
    EMSD_RecipientData *	pRecipData;
    EMSD_Extension *	pExtension;
    EMSD_IpmHeading *	pHeading             = &pIpm->heading;
#ifdef TM_ENABLED
    char *              pStartExtraHeaders   = pExtraHeaders;
#endif

    TM_TRACE ((NULL, 1, "Converting EMSD IPM to RFC-822"));

    /* Append to the Extra Headers string */
    if (pExtraHeaders != NULL)
    {
	pExtraHeaders += strlen(pExtraHeaders);
    }

    /* Add the sender */
    if (pHeading->sender.exists)
    {
	addRfc822Addr(&pHeading->sender.data, &pRfc822Env->sender,
		      pLocalHostName);
    }

    /* Add the originator */
    addRfc822Addr(&pHeading->originator, &pRfc822Env->from,
		  pLocalHostName);

    /* Add the originator as the return_path also */
    addRfc822Addr(&pHeading->originator, &pRfc822Env->return_path,
		  pLocalHostName);

    /* For each recipient... */
    printf ("\n$$$$ There are %d recipients\n", (int)pHeading->recipientDataList.count);
    pRecipData = &pHeading->recipientDataList.data[0];
    for (i = pHeading->recipientDataList.count - 1; i >= 0; --i, ++pRecipData)
    {
	/* ... what type of recipient is he? */
	if (! pRecipData->perRecipFlags.exists)
	{
	    /* He's a primary recipient */
	    addRfc822Addr(&pRecipData->recipient, &pRfc822Env->to,
			  pLocalHostName);
	}
	else if (STR_bitStringGetBit(pRecipData->perRecipFlags.data,
				     EMSD_PerRecipFlags_RecipientType_Copy))
	{
	    /* He's a copy recipient */
	    addRfc822Addr(&pRecipData->recipient, &pRfc822Env->cc,
			  pLocalHostName);
	}
	else if (STR_bitStringGetBit(pRecipData->perRecipFlags.data,
				     EMSD_PerRecipFlags_RecipientType_BlindCopy))
	{
	    /* He's a blind copy recipient */
	    addRfc822Addr(&pRecipData->recipient, &pRfc822Env->bcc,
			  pLocalHostName);
	}
	else
	{
	    /* He's a primary recipient */
	    addRfc822Addr(&pRecipData->recipient, &pRfc822Env->to,
			  pLocalHostName);
	}
    }

    /* If there were reply-to recipients specified... */
    if (pHeading->replyTo.exists)
    {
        pORAddr = &pHeading->replyTo.data.data[0];
        /* ... then add the reply-to recipients */
        for (i = pHeading->replyTo.data.count - 1; i >= 0; --i)
	{
	  addRfc822Addr(pORAddr, &pRfc822Env->reply_to,	pLocalHostName);
	  ++pORAddr;
	}
    }
    
    /* Add the replied-to-IPM */
    if (pHeading->repliedToIpm.exists)
    {
	pRfc822Env->in_reply_to =
	    getRfc822MessageId(&pHeading->repliedToIpm.data,
			       pLocalHostName);
    }

    /* Add the subject string */
    if (pHeading->subject.exists)
    {
	hStr = pHeading->subject.data;
	pRfc822Env->subject =
	    copyCreateRfc822String(STR_stringStart(hStr),
				   STR_getStringLength(hStr));
    }

    /* If we've not been asked for the extra headers as well... */
    if (pExtraHeaders == NULL)
    {
	/* ... then we're done here. */
	return Success;
    }

    /* Add per-message flag header fields */
    if (pHeading->perMessageFlags.exists)
    {
	if (STR_bitStringGetBit(pHeading->perMessageFlags.data,
				EMSD_PerMessageFlags_Priority_Urgent))
	{
	    strcpy(pExtraHeaders, "X-Priority: Urgent\n");
	}
	else if (STR_bitStringGetBit(pHeading->perMessageFlags.data,
				     EMSD_PerMessageFlags_Priority_NonUrgent))
	{
	    strcpy(pExtraHeaders, "X-Priority: Non-Urgent\n");
	}

	pExtraHeaders += strlen(pExtraHeaders);

	if (STR_bitStringGetBit(pHeading->perMessageFlags.data,
				EMSD_PerMessageFlags_Importance_Low))
	{
	    strcpy(pExtraHeaders, "X-Importance: Low\n");
	}
	else if (STR_bitStringGetBit(pHeading->perMessageFlags.data,
				     EMSD_PerMessageFlags_Importance_Low))
	{
	    strcpy(pExtraHeaders, "X-Importance: High\n");
	}

	pExtraHeaders += strlen(pExtraHeaders);

	if (STR_bitStringGetBit(pHeading->perMessageFlags.data,
				EMSD_PerMessageFlags_AutoForwarded))
	{
	    strcpy(pExtraHeaders, "X-Auto-Forwarded: True\n");
	}

	pExtraHeaders += strlen(pExtraHeaders);
    }

    /* Add each of the extension (unrecognized) fields */
    for (i = pHeading->extensions.count - 1,
	 pExtension = pHeading->extensions.data;
	     
	 i >= 0;
	     
	 i--, pExtension++)
    {
	sprintf(pExtraHeaders, "%.*s: %.*s\n",
		STR_getStringLength(pExtension->label),
		STR_stringStart(pExtension->label),
		STR_getStringLength(pExtension->value),
		STR_stringStart(pExtension->value));
	pExtraHeaders += strlen(pExtraHeaders);
    }

    /*
     * If there's a MIME Content Type specified, then we'll set the
     * other MIME header fields.  Otherwise, we'll assume that this
     * is not a MIME encoding, and we'll take the body text to be a
     * simple text message.
     */
    TM_TRACE ((NULL, 1, "MIME Content Type field %s",
	       pHeading->mimeContentType.exists ? "exists" : "does not exist"));
    if (pHeading->mimeContentType.exists)
    {
	/* Add the MIME version */
	if (pHeading->mimeVersion.exists)
	{
	    sprintf(pExtraHeaders, "MIME-Version: %.*s\n",
		    STR_getStringLength(pHeading->mimeVersion.data),
		    STR_stringStart(pHeading->mimeVersion.data));

	}
	else
	{
	    TM_TRACE ((NULL, 1, "MIME-Version defaults to '1.0'"));
	    strcpy(pExtraHeaders, "MIME-Version: 1.0\n");
	}
	pExtraHeaders += strlen(pExtraHeaders);

	/* Add the MIME content type */
	if (pHeading->mimeContentType.exists)
	{
	    sprintf(pExtraHeaders, "Content-Type: %.*s\n",
		    STR_getStringLength(pHeading->mimeContentType.data),
		    STR_stringStart(pHeading->mimeContentType.data));
	    pExtraHeaders += strlen(pExtraHeaders);
	}
	else
	{
	    TM_TRACE ((NULL, 1, "No MIME Content-Type found"));
	}

	/* Add the MIME content id */
	if (pHeading->mimeContentId.exists)
	{
	    sprintf(pExtraHeaders, "Content-Id: %.*s\n",
		    STR_getStringLength(pHeading->mimeContentId.data),
		    STR_stringStart(pHeading->mimeContentId.data));
	    pExtraHeaders += strlen(pExtraHeaders);
	}
	else
	{
	    TM_TRACE ((NULL, 1, "No MIME Content-Id found"));
	}

	/* Add the MIME content description */
	if (pHeading->mimeContentDescription.exists)
	{
	    sprintf(pExtraHeaders, "Content-Description: %.*s\n",
		    STR_getStringLength(pHeading->mimeContentDescription.data),
		    STR_stringStart(pHeading->mimeContentDescription.data));
	    pExtraHeaders += strlen(pExtraHeaders);
	}
	else
	{
	    TM_TRACE ((NULL, 1, "No MIME Content-Description found"));
	}

	/* Add the MIME content transfer encoding */
	if (pHeading->mimeContentTransferEncoding.exists)
	{
	    sprintf(pExtraHeaders,
		    "Content-Transfer-Encoding: %.*s\n",
		    STR_getStringLength(pHeading->mimeContentTransferEncoding.data),
		    STR_stringStart(pHeading->mimeContentTransferEncoding.data));
	    pExtraHeaders += strlen(pExtraHeaders);
	}
	else
	{
	    TM_TRACE ((NULL, 1, "No MIME Content-Transfer-Encoding found"));
	}
    }

    TM_TRACE ((NULL, 1, "Converted EMSD IPM to RFC-822: Extra headers = '%s'",
	       pStartExtraHeaders ? pStartExtraHeaders : "NULL"));
    return Success;
}


ReturnCode
mtsua_convertEmsdDeliverToRfc822(EMSD_DeliverArgument * pDeliver,
				ENVELOPE * pRfc822Env,
				char * pExtraHeaders,
				char * pLocalHostName)
{
    /* Append to the Extra Headers string */
    pExtraHeaders += strlen(pExtraHeaders);

    /* Get the message id */
    pRfc822Env->message_id =
	getRfc822MessageId(&pDeliver->messageId, pLocalHostName);

    /* Get the submission date */
    if (pDeliver->messageId.choice == EMSD_MESSAGEID_CHOICE_RFC822)
    {
	/* See if there's one specified.  There really should be! */
	if (pDeliver->submissionTime.exists)
	{
	    /* It's in the Submission Time field */
	    mtsua_getRfc822Date(pDeliver->submissionTime.data, tmp);

	    pRfc822Env->date = copyString(tmp);
	}
    }
    else
    {
	/* It's part of the local message id */
	mtsua_getRfc822Date(pDeliver->
			    messageId.un.local.submissionTime,
			    tmp);

	pRfc822Env->date = copyString(tmp);
    }
	
    /* Get the delivery date */
    mtsua_getRfc822Date(pDeliver->deliveryTime, tmp);
    sprintf(pExtraHeaders, "X-Delivery-Date: %s\n", tmp);

    return Success;
}


char *
mtsua_getRfc822Date(OS_Uint32 julianDate,
		    char * pBuf)
{
    OS_ZuluDateTime dateStruct;
    static char *   months[] =
    {
	"",
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    /* Convert the date to structure format */
    (void) OS_julianToDateStruct(julianDate, &dateStruct);

    sprintf(pBuf, "%02d %s %02d %02d:%02d:%02d GMT",
	    dateStruct.day,
	    months[dateStruct.month],
	    dateStruct.year - 1900,
	    dateStruct.hour,
	    dateStruct.minute,
	    dateStruct.second);

    return pBuf;
}


static void
addRfc822Addr(EMSD_ORAddress   * pORAddr,
	      ADDRESS        ** ppEnvAddr,
	      char            * pLocalHostName)
{
    char	    tmpBuf[512];

    /*
     * Create an RFC-822 address from the local-format address.
     */
    mtsua_createRfc822AddrFromEmsd(pORAddr, pLocalHostName, tmpBuf, sizeof(tmpBuf));

    /* Parse the address and add it to the list. */
    rfc822_parse_adrlist(ppEnvAddr, tmpBuf, pLocalHostName);

    if (*ppEnvAddr == NULL || (*ppEnvAddr)->mailbox == NULL)
        printf ("%s, %d: *** No mailbox field in RFC 822 message ***\n", __FILE__, __LINE__);
    printf ("%s, %d: Parsed address = '%.64s'\n", __FILE__, __LINE__, tmpBuf);
}


static char *
getRfc822MessageId(EMSD_MessageId * pMsgId,
		   char * pLocalHostName)
{
    STR_String		hStr;

    if (pMsgId->choice == EMSD_MESSAGEID_CHOICE_LOCAL)
    {
	createRfc822MessageIdFromEmsd(pMsgId, pLocalHostName, tmp);
	return copyString(tmp);
    }
    else
    {
	hStr = pMsgId->un.rfc822;
	return copyCreateRfc822String(STR_stringStart(hStr),
				      STR_getStringLength(hStr));
    }
}


static char *
copyCreateRfc822String(void * pSrc,
		       OS_Uint16 len)
{
    char *		pDest;

    if (len == STR_ZSTRING)
    {
	len = strlen((char *) pSrc);
    }
    
    if (pSrc != NULL)
    {
	/* make sure argument specified */
	pDest = (char *) OS_alloc (1 + len);
	strncpy(pDest, (char *) pSrc, len);
	pDest[len] = '\0';

	return (pDest);
    }
    else
    {
	return NULL;
    }
}


/* Copy string to free storage
 * Accepts: source string
 * Returns: free storage copy of string
 */

static char *
copyString(char *pString)
{
    char * 	    p;

    if (pString != NULL)
    {
	p = OS_alloc(strlen(pString) + 1);
	strcpy(p, pString);
	return p;
    }
    else
    {
	return NULL;
    }
}


void
createRfc822MessageIdFromEmsd(EMSD_MessageId * pMsgId,
			     char * pLocalHostName,
			     char * pBuf)
{
    /*
     * Create an RFC-822 message id from the local identifier.
     *
     * The format that we use is as follows:
     *
     *     The User-name portion is composed of three components:
     *         The string "EMSD"
     *         The local Message Id's julian submission time
     *         The local Message Id's daily message number
     *
     *     The Domain-name portion is composed of one component:
     *         The local host name (sub-domain)
     */
    sprintf(pBuf, "<EMSD.%08lx.%08lx@%s>",
	    pMsgId->un.local.submissionTime,
	    pMsgId->un.local.dailyMessageNumber,
	    pLocalHostName);
}


