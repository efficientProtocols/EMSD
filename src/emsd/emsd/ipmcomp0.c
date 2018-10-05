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
#include "asn.h"
#include "emsd.h"
#include "emsdlocal.h"


/*
 * Compile an EMSD Version 1.0 IPM
 */


static ReturnCode
compileHeading(unsigned char * pCStruct,
	       OS_Boolean * pExists,
	       EMSD_IpmHeading * pHeading,
	       OS_Uint8 tag,
	       QU_Head * pQ,
	       char * pMessage);

static ReturnCode
compileBody(unsigned char * pCStruct,
	    OS_Boolean * pExists,
	    EMSD_IpmBody * pBody,
	    OS_Uint8 tag,
	    QU_Head * pQ,
	    char * pMessage);

static ReturnCode
compileExtensions(unsigned char * pCStruct,
		  OS_Boolean * pExists,
		  EMSD_Extensions * pExtensions,
		  OS_Uint8 tag,
		  QU_Head * pQ,
		  char * pMessage);

static ReturnCode
compileRecipientData(unsigned char * pCStruct,
		     OS_Boolean * pExists,
		     EMSD_RecipientDataList * pRecipDataList,
		     OS_Uint8 tag,
		     QU_Head * pQ,
		     char * pMessage);

static ReturnCode
compileReplyTo(unsigned char * pCStruct,
	       OS_Boolean * pExists,
	       EMSD_ReplyTo * pReplyTo,
	       OS_Uint8 tag,
	       QU_Head * pQ,
	       char * pMessage);

/* --------------------------------------------------------------- */

ReturnCode
EMSD_compileIpm1_0(unsigned char * pCStruct,
		  OS_Boolean * pExists,
		  EMSD_Message * pMessage,
		  OS_Uint8 tag,
		  QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "IPM",
		  ("v1.0 emsd_compileIPM: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the heading. */
    RC_CALL(rc,
	    compileHeading((unsigned char *) pCStruct,
			   NULL,
			   &pMessage->heading,
			   ASN_UniversalTag_Sequence, pQ,
			   "Heading"),
	    ("v1.0 emsd_compileIpm: heading"));
    
    /* Add the body. */
    RC_CALL(rc,
	    compileBody((unsigned char *) pCStruct,
			&pMessage->body.exists,
			&pMessage->body.data,
			ASN_UniversalTag_Sequence, pQ,
			"Body"),
	    ("v1.0 emsd_compileIpm: body"));
    
    
    return Success;
}


static ReturnCode
compileHeading(unsigned char * pCStruct,
	       OS_Boolean * pExists,
	       EMSD_IpmHeading * pHeading,
	       OS_Uint8 tag,
	       QU_Head * pQ,
	       char * pMessage)
{
    ReturnCode 		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("v1.0 compileHeading: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the sender. */
    RC_CALL(rc,
	    emsd_compileORAddress1_0((unsigned char *) pCStruct,
				    &pHeading->sender.exists,
				    &pHeading->sender.data,
				    ASN_TAG_CONTEXT(0) | ASN_EXPLICIT,
				    pQ, "Sender ORAddress"),
	    ("v1.0 compileHeading: sender"));
    
    /* Add the originator. */
    RC_CALL(rc,
	    emsd_compileORAddress1_0((unsigned char *) pCStruct,
				    NULL,
				    &pHeading->originator,
				    0, pQ, "Originator ORAddress"),
	    ("v1.0 compileHeading: originator"));
    
    /* Add the sequence of recipient data */
    RC_CALL(rc,
	    compileRecipientData((unsigned char *) pCStruct,
				 NULL,
				 &pHeading->recipientDataList,
				 ASN_UniversalTag_Sequence,
				 pQ, "Recipient List"),
	    ("v1.0 compileHeading: recipient data"));
    
    /* Create a table entry for the per-message-flags */
    ASN_NEW_TABLE(pTab, ASN_Type_BitString,
		  ASN_TAG_CONTEXT(1), pQ,
		  pCStruct,
		  &pHeading->perMessageFlags.exists,
		  &pHeading->perMessageFlags.data,
		  "Per-message Flags",
		  ("v1.0 compileHeading: per message flags"));
    
    /* Add the reply-to recipient. */
    RC_CALL(rc,
	    compileReplyTo((unsigned char *) pCStruct,
			   &pHeading->replyTo.exists,
			   &pHeading->replyTo.data,
			   ASN_TAG_CONTEXT(2), pQ,
			   "Reply To"),
	    ("v1.0 compileHeading: reply to"));
    
    /* Add the replied-to IPM */
    RC_CALL(rc,
	    emsd_compileMessageId1_0((unsigned char *) pCStruct,
				    &pHeading->repliedToIpm.exists,
				    &pHeading->repliedToIpm.data,
				    0, pQ,
				    "Replied-to Message Id"),
	    ("v1.0 compileHeading: replied-to-ipm"));
    
    /* Create a table entry for the subject */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(4), pQ,
		  pCStruct,
		  &pHeading->subject.exists,
		  &pHeading->subject.data,
		  "Subject",
		  ("v1.0 compileHeading: subject"));
    
    /* Add the extension fields */
    RC_CALL(rc, compileExtensions((unsigned char *) pCStruct,
				  (OS_Boolean *) &pHeading->extensions.count,
				  &pHeading->extensions,
				  ASN_TAG_CONTEXT(5), pQ,
				  "Extensions"),
	    ("v1.0 compileHeading: extensions"));

    /* Create a table entry for the MIME version number */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(6), pQ,
		  pCStruct,
		  &pHeading->mimeVersion.exists,
		  &pHeading->mimeVersion.data,
		  "MIME Version",
		  ("v1.0 compileHeading: mime version"));
    
    /* Create a table entry for the MIME content type */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(7), pQ,
		  pCStruct,
		  &pHeading->mimeContentType.exists,
		  &pHeading->mimeContentType.data,
		  "MIME Content Type",
		  ("v1.0 compileHeading: mime content type"));
    
    /* Create a table entry for the MIME content id */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(8), pQ,
		  pCStruct,
		  &pHeading->mimeContentId.exists,
		  &pHeading->mimeContentId.data,
		  "MIME Content Id",
		  ("v1.0 compileHeading: mime content id"));
    
    /* Create a table entry for the MIME content description */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(9), pQ,
		  pCStruct,
		  &pHeading->mimeContentDescription.exists,
		  &pHeading->mimeContentDescription.data,
		  "MIME Content Description",
		  ("v1.0 compileHeading: mime content description"));
    
    /* Create a table entry for the MIME content transfer encoding */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(10), pQ,
		  pCStruct,
		  &pHeading->mimeContentTransferEncoding.exists,
		  &pHeading->mimeContentTransferEncoding.data,
		  "MIME Content Transfer Encoding",
		  ("v1.0 compileHeading: mime content transfer encoding"));
    
    return Success;
}


static ReturnCode
compileBody(unsigned char * pCStruct,
	    OS_Boolean * pExists,
	    EMSD_IpmBody * pBody,
	    OS_Uint8 tag,
	    QU_Head * pQ,
	    char * pMessage)
{
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("v1.0 compileBody: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the compression method */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_TAG_CONTEXT(0), pQ,
		  pCStruct,
		  &pBody->compressionMethod.exists,
		  &pBody->compressionMethod.data,
		  "Compression Method",
		  ("v1.0 compileBody: compression method"));
    
    /* Create a table entry for the body */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &pBody->bodyText,
		  "Body Text",
		  ("v1.0 compileBody: body text"));
    
    return Success;
}


static ReturnCode
compileExtensions(unsigned char * pCStruct,
		  OS_Boolean * pExists,
		  EMSD_Extensions * pExtensions,
		  OS_Uint8 tag,
		  QU_Head * pQ,
		  char * pMessage)
{
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for a sequence of */
    ASN_NEW_TABLE(pTab, ASN_Type_SequenceOf,
		  tag, pQ,
		  pCStruct,
		  pExists,
		  &pExtensions->count,
		  pMessage,
		  ("v1.0 compileExtensions: sequence of"));
    
    /* Keep track of the size of each element */
    pTab->elementSize = sizeof(EMSD_Extension);
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  NULL,
		  NULL,
		  NULL,
		  "Extensions",
		  ("v1.0 compileExtensions: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the label */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_APPL(0), pQ,
		  pCStruct,
		  NULL,
		  &pExtensions->data[0].label,
		  "Label",
		  ("v1.0 compileExtensions: label"));
    
    /* Create a table entry for the value */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_APPL(0), pQ,
		  pCStruct,
		  NULL,
		  &pExtensions->data[0].value,
		  "Value",
		  ("v1.0 compileExtensions: value"));
    
    return Success;
}


static ReturnCode
compileRecipientData(unsigned char * pCStruct,
		     OS_Boolean * pExists,
		     EMSD_RecipientDataList * pRecipDataList,
		     OS_Uint8 tag,
		     QU_Head * pQ,
		     char * pMessage)
{
    ReturnCode	        rc;
    ASN_TableEntry *	pTab;
    EMSD_RecipientData * pRecipData = pRecipDataList->data;
    
    /* Create a table entry for a sequence of */
    ASN_NEW_TABLE(pTab, ASN_Type_SequenceOf,
		  tag, pQ,
		  pCStruct,
		  pExists,
		  &pRecipDataList->count,
		  pMessage,
		  ("v1.0 compileRecipientData: sequence of"));
    
    /* Keep track of the size of each element */
    pTab->elementSize = sizeof(EMSD_RecipientData);
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  NULL,
		  NULL,
		  NULL,
		  "Recipient Data",
		  ("v1.0 compileRecipientData: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the recipient address */
    RC_CALL(rc,
	    emsd_compileORAddress1_0((unsigned char *) pCStruct,
				    NULL,
				    &pRecipData->recipient,
				    0, pQ,
				    "Recipient Address"),
	    ("v1.0 compileRecipientData: recipient address"));
    
    /* Create a table entry for the per-recipient flags */
    ASN_NEW_TABLE(pTab, ASN_Type_BitString,
		  ASN_UniversalTag_BitString, pQ,
		  pCStruct,
		  &pRecipData->perRecipFlags.exists,
		  &pRecipData->perRecipFlags.data,
		  "Per-recipient Flags",
		  ("v1.0 compileRecipientData: per-recipient flags"));
    
    return Success;
}



static ReturnCode
compileReplyTo(unsigned char * pCStruct,
	       OS_Boolean * pExists,
	       EMSD_ReplyTo * pReplyTo,
	       OS_Uint8 tag,
	       QU_Head * pQ,
	       char * pMessage)
{
    ReturnCode	        rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for a sequence of */
    ASN_NEW_TABLE(pTab, ASN_Type_SequenceOf,
		  tag, pQ,
		  pCStruct,
		  pExists,
		  &pReplyTo->count,
		  pMessage,
		  ("v1.0 compileRecipientData: sequence of"));
    
    /* Specify the maximum number of elements in the sequence-of */
    pTab->maxDataLength = EMSDUB_MAX_REPLY_TO;
    
    /* Keep track of the size of each element */
    pTab->elementSize = sizeof(EMSD_ORAddress);
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the recipient address */
    RC_CALL(rc,
	    emsd_compileORAddress1_0((unsigned char *) pCStruct,
				    NULL,
				    pReplyTo->data,
				    0, pQ,
				    "Recipient Address"),
	    ("v1.0 compileRecipientData: recipient address"));
    
    return Success;
}
