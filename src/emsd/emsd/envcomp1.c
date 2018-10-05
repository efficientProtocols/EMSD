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
#include "queue.h"
#include "asn.h"
#include "emsd.h"
#include "emsdlocal.h"


static ReturnCode
compileLocalMessageId(unsigned char * pCStruct,
		      OS_Boolean * pExists,
		      EMSD_LocalMessageId * pLocalMessageId,
		      OS_Uint8 tag,
		      QU_Head * pQ,
		      char * pMessage);

static ReturnCode
compileEMSDAddress(unsigned char * pCStruct,
		  OS_Boolean * pExists,
		  EMSD_EMSDAddress * pAddress,
		  OS_Uint8 tag,
		  QU_Head * pQ,
		  char * pMessage);

static ReturnCode
compileCredentialsSimple(unsigned char * pCStruct,
			 OS_Boolean * pExists,
			 EMSD_CredentialsSimple * pSimple,
			 OS_Uint8 tag,
			 QU_Head * pQ,
			 char * pMessage);

static ReturnCode
compileSecurityElement(unsigned char * pCStruct,
		       OS_Boolean * pExists,
		       EMSD_SecurityElement * pSec,
		       OS_Uint8 tag,
		       QU_Head * pQ,
		       char * pMessage);

static ReturnCode
compileSegmentData(unsigned char * pCStruct,
		   OS_Boolean * pExists,
		   EMSD_SegmentData * pSegData,
		   OS_Uint8 tag,
		   QU_Head * pQ,
		   char * pMessage);

static ReturnCode
compileSegmentInfo(unsigned char * pCStruct,
		   OS_Boolean * pExists,
		   EMSD_SegmentInfo * pSegInfo,
		   OS_Uint8 tag,
		   QU_Head * pQ,
		   char * pMessage);


/* --------------------------------------------------------------- */

ReturnCode
EMSD_compileSubmitArgument1_1(unsigned char * pCStruct,
			     OS_Boolean * pExists,
			     EMSD_SubmitArgument * pArg,
			     OS_Uint8 tag,
			     QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Submit Arg",
		  ("emsd_compileSubmitArg: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the security element */
    RC_CALL(rc,
	    compileSecurityElement((unsigned char *) pCStruct,
				   &pArg->security.exists,
				   &pArg->security.data,
				   ASN_TAG_CONTEXT(0), pQ,
				   "Security Element"),
	    ("emsd_compileSubmitArg: security"));
    
    /* Add the segment info */
    RC_CALL(rc,
	    compileSegmentInfo((unsigned char *) pCStruct,
			       &pArg->segmentInfo.exists,
			       &pArg->segmentInfo.data,
			       0, pQ,
			       "Segment Info"),
	    ("emsd_compileSubmitArg: segmentInfo"));
    
    /* Create a table entry for the content type */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pArg->contentType,
		  "Content Type",
		  ("emsd_compileSubmitArg: content type"));
    
    /* Create a table entry for the contents */
    ASN_NEW_TABLE(pTab, ASN_Type_Buffer,
		  0, pQ,
		  pCStruct,
		  NULL,
		  &pArg->hContents,
		  "Contents",
		  ("emsd_compileSubmitArgument: contents"));
    
    return Success;
}


ReturnCode
EMSD_compileSubmitResult1_1(unsigned char * pCStruct,
			   OS_Boolean * pExists,
			   EMSD_SubmitResult * pResult,
			   OS_Uint8 tag,
			   QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Submit Result",
		  ("emsd_compileSubmitResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the local message id */
    RC_CALL(rc,
	    compileLocalMessageId((unsigned char *) pCStruct,
				  NULL,
				  &pResult->localMessageId,
				  ASN_UniversalTag_Sequence, pQ,
				  "Local Message Id"),
	    ("emsd_compileSubmitResult: local message id"));
    
    return Success;
}


ReturnCode
EMSD_compileDeliveryControlArgument1_1(unsigned char * pCStruct,
				      OS_Boolean * pExists,
				      EMSD_DeliveryControlArgument *pArg,
				      OS_Uint8 tag,
				      QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Delivery Control",
		  ("emsd_compileDeliveryControlArgument: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the restrict flag */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer, ASN_TAG_CONTEXT(0), pQ,
		  pCStruct,
		  &pArg->restrict.exists,
		  &pArg->restrict.data,
		  "Restrict Flag",
		  ("emsd_compileDeliveryControlArgument: restrict"));
    
    /* Create a table entry for permissible-operations */
    ASN_NEW_TABLE(pTab, ASN_Type_BitString, ASN_TAG_CONTEXT(1), pQ,
		  pCStruct,
		  &pArg->permissibleOperations.exists,
		  &pArg->permissibleOperations.data,
		  "Permissible Operations",
		  ("emsd_compileDeliveryControlArgument: "
		   "permissible operations"));
    
    /* Create a table entry for persmission-max-content-length */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer, ASN_TAG_CONTEXT(2), pQ,
		  pCStruct,
		  &pArg->permissibleMaxContentLength.exists,
		  &pArg->permissibleMaxContentLength.data,
		  "Permissible Max Content Length",
		  ("emsd_compileDeliveryControlArgument: "
		   "permissible maximum content length"));

    /* Specify the maximum length */
    pTab->maxDataLength = EMSDUB_MAX_CONTENT_LEN;
    
    /* Create a table entry for permissible-lowest-priority */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer, ASN_TAG_CONTEXT(3), pQ,
		  pCStruct,
		  &pArg->permissibleLowestPriority.exists,
		  &pArg->permissibleLowestPriority.data,
		  "Permissible Lowest Priority",
		  ("emsd_compileDeliveryControlArgument: "
		   "permissible lowest priority"));
    
    /* Add a security element */
    RC_CALL(rc,
	    compileSecurityElement((unsigned char *) pCStruct,
				   &pArg->security.exists,
				   &pArg->security.data,
				   ASN_TAG_CONTEXT(4), pQ,
				   "Security Element"),
	    ("emsd_compileDeliveryControlArgument: security"));
    
    /* Create a table entry for user features */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString, ASN_TAG_CONTEXT(5), pQ,
		  pCStruct,
		  &pArg->userFeatures.exists,
		  &pArg->userFeatures.data,
		  "User Features",
		  ("emsd_compileDeliveryControlArgument: "
		   "user features"));
    
    return Success;
}


ReturnCode
EMSD_compileDeliveryControlResult1_1(unsigned char             * pCStruct,
				    OS_Boolean                * pExists,
				    EMSD_DeliveryControlResult * pResult,
				    OS_Uint8                    tag,
				    QU_Head                   * pQ)
{
    ASN_TableEntry *	pTab = NULL;
    ASN_TableEntry *	pTop = NULL;

    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Delivery Control Result",
		  ("emsd_compileDeliveryControlResult: sequence"));
    
    pTop = pTab;  /*** fnh ***/

    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for waiting-operations */
    ASN_NEW_TABLE(pTab, ASN_Type_BitString, ASN_TAG_CONTEXT(0), pQ,
		  pCStruct,
		  &pResult->waitingOperations.exists,
		  &pResult->waitingOperations.data,
		  "Waiting Operations",
		  ("emsd_compileDeliveryControlResult: "
		   "waiting operations"));
    
    /* Create a table entry for waiting-messages */
    ASN_NEW_TABLE(pTab, ASN_Type_BitString, ASN_TAG_CONTEXT(1), pQ,
		  pCStruct,
		  &pResult->waitingMessages.exists,
		  &pResult->waitingMessages.data,
		  "Waiting Messages",
		  ("emsd_compileDeliveryControlResult: waiting messages"));
    
    /* Create a table entry for a sequence of */
    ASN_NEW_TABLE(pTab, ASN_Type_SequenceOf,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  NULL,
		  &pResult->waitingContentTypes.count,
		  "Content Types",
		  ("emsd_compileDeliveryControlResult: content types sequence of"));
    pTab->maxDataLength = EMSDUB_NUM_CONTENT_TYPES;
    
    /* Indicate the size of each element of the sequence-of */
    pTab->elementSize = sizeof(OS_Uint32);

    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the waiting content types */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  pResult->waitingContentTypes.data,
		  "Waiting Content Types",
		  ("emsd_compileDeliveryControlResult: content type Integer"));
    
    return Success;
}


ReturnCode
EMSD_compileDeliveryVerifyArgument1_1(unsigned char * pCStruct,
				     OS_Boolean * pExists,
				     EMSD_DeliveryVerifyArgument * pArg,
				     OS_Uint8 tag,
				     QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Delivery Verify Arg",
		  ("emsd_compileDeliveryVerifyArgument: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the message id */
    RC_CALL(rc,
	    emsd_compileMessageId1_1((unsigned char *) pCStruct,
				    NULL, &pArg->messageId,
				    0, pQ,
				    "Message Id"),
	    ("emsd_compileDeliveryVerifyArgument: message id"));
    
    return Success;
}


ReturnCode
EMSD_compileDeliveryVerifyResult1_1(unsigned char * pCStruct,
				   OS_Boolean * pExists,
				   EMSD_DeliveryVerifyResult * pArg,
				   OS_Uint8 tag,
				   QU_Head * pQ)
{
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Delivery Verify Result",
		  ("emsd_compileDeliveryVerifyResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the delivery status */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer, ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pArg->deliveryStatus,
		  "Delivery Status",
		  ("emsd_compileDeliveryVerifyResult: "
		   "delivery status"));
    
    return Success;
}


ReturnCode
EMSD_compileDeliverArgument1_1(unsigned char * pCStruct,
			      OS_Boolean * pExists,
			      EMSD_DeliverArgument * pArg,
			      OS_Uint8 tag,
			      QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Deliver Arg",
		  ("emsd_compileDeliverArgument: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the message id */
    RC_CALL(rc,
	    emsd_compileMessageId1_1((unsigned char *) pCStruct,
				    NULL, &pArg->messageId,
				    0, pQ, "Message Id"),
	    ("emsd_compileDeliverArgument: message id"));
    
    /* Create a table entry for message delivery time */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pArg->deliveryTime,
		  "Message Delivery Time",
		  ("emsd_compileDeliverArgument: delivery time"));
    
    /* Create a table entry for message submission time */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_TAG_CONTEXT(0), pQ,
		  pCStruct,
		  &pArg->submissionTime.exists,
		  &pArg->submissionTime.data,
		  "Message Submission Time",
		  ("emsd_compileDeliverArgument: submission time"));
    
    /* Add a security element */
    RC_CALL(rc,
	    compileSecurityElement((unsigned char *) pCStruct,
				   &pArg->security.exists,
				   &pArg->security.data,
				   ASN_TAG_CONTEXT(1), pQ,
				   "Security Element"),
	    ("emsd_compileDeliverArgument: security"));
    
    /* Add the segment info */
    RC_CALL(rc,
	    compileSegmentInfo((unsigned char *) pCStruct,
			       &pArg->segmentInfo.exists,
			       &pArg->segmentInfo.data,
			       0, pQ,
			       "Segment Info"),
	    ("emsd_compileDeliverArgument: segmentInfo"));
    
    /* Create a table entry for the content type */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pArg->contentType,
		  "Content Type",
		  ("emsd_compileDeliverArgument: content type"));
    
    /* Create a table entry for the contents */
    ASN_NEW_TABLE(pTab, ASN_Type_Buffer,
		  0, pQ,
		  pCStruct,
		  NULL,
		  &pArg->hContents,
		  "Contents",
		  ("emsd_compileDeliverArgument: content type"));
    
    return Success;
}


ReturnCode
EMSD_compileSubmissionVerifyArgument1_1(unsigned char * pCStruct,
				       OS_Boolean * pExists,
				       EMSD_SubmissionVerifyArgument * pArg,
				       OS_Uint8 tag,
				       QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Submission Verify Arg",
		  ("emsd_compileSubmissionVerifyArgument: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the message id */
    RC_CALL(rc,
	    emsd_compileMessageId1_1((unsigned char *) pCStruct,
				    NULL, &pArg->messageId,
				    0, pQ, "Message Id"),
	    ("emsd_compileSubmissionVerifyArgument: message id"));
    
    return Success;
}


ReturnCode
EMSD_compileSubmissionVerifyResult1_1(unsigned char * pCStruct,
				     OS_Boolean * pExists,
				     EMSD_SubmissionVerifyResult * pArg,
				     OS_Uint8 tag,
				     QU_Head * pQ)
{
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "Submission Verify Result",
		  ("emsd_compileSubmissionVerifyResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the submission status */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Enumerated, pQ,
		  pCStruct,
		  NULL,
		  &pArg->submissionStatus,
		  "Submission Status",
		  ("emsd_compileSubmissionVerifyResult: "
		   "submission status"));
    
    return Success;
}


ReturnCode
emsd_compileMessageId1_1(unsigned char * pCStruct,
			OS_Boolean * pExists,
			EMSD_MessageId * pMessageId,
			OS_Uint8 tag,
			QU_Head * pQ,
			char * pMessage)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    ASN_NEW_TABLE(pTab, ASN_Type_Choice, tag, pQ,
		  pCStruct,
		  pExists,
		  &pMessageId->choice,
		  pMessage,
		  ("emsd_compileMessageId: choice"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the local message id choice. */
    RC_CALL(rc,
	    compileLocalMessageId((unsigned char *) pCStruct,
				  NULL,
				  &pMessageId->un.local,
				  ASN_TAG_APPL(4), pQ,
				  "Local Message Id"),
	    ("emsd_compileMessageId: local message id"));
    
    /* create a table entry for the rfc822 message id choice. */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_APPL(5), pQ,
		  pCStruct,
		  NULL,
		  &pMessageId->un.rfc822,
		  "RFC822 Message Id",
		  ("emsd_compileMessageId: rfc822 message id"));

    /* Specify the maximum length */
    pTab->maxDataLength = EMSDUB_MAX_MESSAGE_ID_LEN;
    
    return Success;
}


ReturnCode
emsd_compileORAddress1_1(unsigned char * pCStruct,
			OS_Boolean * pExists,
			EMSD_ORAddress * pAddress,
			OS_Uint8 tag,
			QU_Head * pQ,
			char * pMessage)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the Choice */
    ASN_NEW_TABLE(pTab, ASN_Type_Choice, tag, pQ,
		  pCStruct,
		  pExists,
		  &pAddress->choice,
		  pMessage,
		  ("emsd_compileORAddress: choice"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add a local-format address */
    RC_CALL(rc,
	    compileEMSDAddress((unsigned char *) pCStruct,
			      NULL,
			      &pAddress->un.local,
			      ASN_UniversalTag_Sequence, pQ,
			      "Local Format"),
	    ("emsd_compileORAddress: local format"));
    
    /* Create a table entry for an rfc822 user address */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString, ASN_TAG_APPL(0), pQ,
		  pCStruct,
		  NULL,
		  &pAddress->un.rfc822,
		  "RFC822 User Address",
		  ("emsd_compileORAddress: rfc822 user address"));

    /* Specify the maximum length */
    pTab->maxDataLength = EMSDUB_MAX_RFC822_NAME_LEN;
    
    return Success;
}


static ReturnCode
compileLocalMessageId(unsigned char * pCStruct,
		      OS_Boolean * pExists,
		      EMSD_LocalMessageId * pLocalMessageId,
		      OS_Uint8 tag,
		      QU_Head * pQ,
		      char * pMessage)
{
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence, tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("compileLocalMessageId: Sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the submission time */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pLocalMessageId->submissionTime,
		  "Submission Time",
		  ("compileLocalMessageId: submission time"));
    
    /* Create a table entry for the daily message number */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pLocalMessageId->dailyMessageNumber,
		  "Daily Message Number",
		  ("compileLocalMessageId: daily message number"));
    
    return Success;
}


static ReturnCode
compileEMSDAddress(unsigned char * pCStruct,
		  OS_Boolean * pExists,
		  EMSD_EMSDAddress * pAddress,
		  OS_Uint8 tag,
		  QU_Head * pQ,
		  char * pMessage)
{
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence, tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("compileEMSDAddress: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the emsd-address value */
    ASN_NEW_TABLE(pTab, ASN_Type_BcdString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &pAddress->emsdAddress,
		  "EMSD Address",
		  ("compileEMSDAddress: emsd-address"));
    
    /* Create a table entry for an emsd-name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(0), pQ,
		  pCStruct,
		  &pAddress->emsdName.exists,
		  &pAddress->emsdName.data,
		  "EMSD Name",
		  ("compileEMSDAddress: emsd-name"));

    /* Specify the maximum length */
    pTab->maxDataLength = EMSDUB_MAX_NAME_LENGTH;
    
    return Success;
}


static ReturnCode
compileCredentialsSimple(unsigned char * pCStruct,
			 OS_Boolean * pExists,
			 EMSD_CredentialsSimple * pSimple,
			 OS_Uint8 tag,
			 QU_Head * pQ,
			 char * pMessage)
{
    ASN_TableEntry *	pTab;
    ReturnCode	        rc;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence, tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("compileCredentialsSimple: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add an emsd-address */
    RC_CALL(rc,
	    compileEMSDAddress((unsigned char *) pCStruct,
			      &pSimple->emsdAddress.exists,
			      &pSimple->emsdAddress.data,
			      ASN_UniversalTag_Sequence, pQ,
			      "EMSD Address"),
	    ("compileSimple: emsd address"));
    
    /* Create a table entry for the password */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(0), pQ,
		  pCStruct,
		  &pSimple->password.exists,
		  &pSimple->password.data,
		  "Password",
		  ("compileSimple: password"));

    /* Specify the maximum length */
    pTab->maxDataLength = EMSDUB_MAX_PASSWORD_LEN;
    
    return Success;
}


static ReturnCode
compileSecurityElement(unsigned char * pCStruct,
		       OS_Boolean * pExists,
		       EMSD_SecurityElement * pSec,
		       OS_Uint8 tag,
		       QU_Head * pQ,
		       char * pMessage)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence, tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("compileSecurityElement: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add simple credentials */
    RC_CALL(rc,
	    compileCredentialsSimple((unsigned char *) pCStruct,
				     NULL,
				     &pSec->credentials.simple,
				     ASN_TAG_CONTEXT(0), pQ,
				     "Simple Credentials"),
	    ("compileSecurityElement: simple: emsd address"));
    
    /* Create a table entry for the content security check */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  &pSec->contentIntegrityCheck.exists,
		  &pSec->contentIntegrityCheck.data,
		  "Content Integrity Check",
		  ("compileSecurityElement: "
		   "content integrity check"));

    return Success;
}


static ReturnCode
compileSegmentInfo(unsigned char * pCStruct,
		   OS_Boolean * pExists,
		   EMSD_SegmentInfo * pSegInfo,
		   OS_Uint8 tag,
		   QU_Head * pQ,
		   char * pMessage)
{
    ReturnCode		rc;
    ASN_TableEntry *	pTab;

    /* Create a table entry for the Choice */
    ASN_NEW_TABLE(pTab, ASN_Type_Choice, tag, pQ,
		  pCStruct,
		  pExists,
		  &pSegInfo->choice,
		  pMessage,
		  ("emsd_compileSegmentInfo: choice"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /*
     * Since the structures are essentially the same, we use the same
     * fields for both the First Segment and the Other Segments in
     * this "union".
     */

    /* Add a First Segment */
    RC_CALL(rc,
	    compileSegmentData((unsigned char *) pCStruct,
			       NULL,
			       &pSegInfo->segmentData,
			       ASN_TAG_APPL(2), pQ,
			       "First Segment"),
	    ("emsd_compileSegmentInfo: first segment"));
    
    /* Add an Other Segment */
    RC_CALL(rc,
	    compileSegmentData((unsigned char *) pCStruct,
			       NULL,
			       &pSegInfo->segmentData,
			       ASN_TAG_APPL(3), pQ,
			       "Other Segment"),
	    ("emsd_compileSegmentInfo: other segment"));
    
    return Success;
}


static ReturnCode
compileSegmentData(unsigned char * pCStruct,
		   OS_Boolean * pExists,
		   EMSD_SegmentData * pSegData,
		   OS_Uint8 tag,
		   QU_Head * pQ,
		   char * pMessage)
{
    ASN_TableEntry *	pTab;

    /* Create a table entry for the Sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence, tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("emsd_compileSegmentData: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the sequence id this segment is part of */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pSegData->sequenceId,
		  "Part Of",
		  ("compileSegmentData: sequence id"));
    
    /* Create a table entry for the segment number or number of segments */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pSegData->data,
		  "Total Number Of Segments Or Segment Number",
		  ("compileSegmentInfo: "
		   "total number of segments or segment number"));
    
    return Success;
}
