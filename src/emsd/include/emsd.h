/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of Neda-EMSD. An implementation of RFC-2524.
 *
 *Neda-EMSD is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with Neda-EMSD; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
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

#ifndef __EMSD_H__
#define	__EMSD_H__

#include "estd.h"
#include "strfunc.h"
#include "asn.h"

/*
 *  Trace flag definitions
 */
#define	EMSD_TRACE_ERROR   	(1 << 0)
#define	EMSD_TRACE_WARNING   	(1 << 1)
#define	EMSD_TRACE_ACTIVITY	(1 << 2)
#define	EMSD_TRACE_DETAIL   	(1 << 3)
#define	EMSD_TRACE_INIT		(1 << 4)
#define	EMSD_TRACE_VALIDATION	(1 << 5)
#define	EMSD_TRACE_PREDICATE	(1 << 6)
#define	EMSD_TRACE_STATE  	(1 << 8)
#define	EMSD_TRACE_ADDRESS	(1 << 7)
#define	EMSD_TRACE_PDU		(1 << 9)

/*
 * Upper Bounds
 */

/* Maximum number of recipients in a message */
#define	EMSDUB_MAX_RECIPIENTS		256

/* Maximum number of reply-to recipients in a message */
#define	EMSDUB_MAX_REPLY_TO		256

/* Maximum length of the SUBJECT field */
#define	EMSDUB_MAX_SUBJECT_LEN		127

/* Maximum length of a password */
#define	EMSDUB_MAX_PASSWORD_LEN		16

/* Maximum length of the Contents of a message */
#define	EMSDUB_MAX_CONTENT_LEN		65535

/* Maximum number of content types */
#define	EMSDUB_NUM_CONTENT_TYPES		128

/* Maximum length of a Message ID */
#define	EMSDUB_MAX_MESSAGE_ID_LEN	127

/* Maximum number of RFC-822 Extension (unrecognized key-word) fields */
#define	EMSDUB_MAX_HEADER_EXTENSIONS	64

/* Maximum length of an EMSD Name */
#define	EMSDUB_MAX_NAME_LENGTH		64

/* Maximum length of an RFC-822 recipient address */
#define	EMSDUB_MAX_RFC822_NAME_LEN	127

/* Maximum length of a MIME version number */
#define	EMSDUB_MAX_MIME_VERSION		8

/* Maximum length of a MIME content type */
#define	EMSDUB_MAX_MIME_CONTENT_TYPE_LEN	127

/* Maximum length of a MIME content id */
#define	EMSDUB_MAX_MIME_CONTENT_ID_LEN	127

/* Maximum length of a MIME content description */
#define	EMSDUB_MAX_MIME_CONTENT_DESC_LEN	127

/* Maximum length of a MIME content transfer encoding */
#define	EMSDUB_MAX_MIME_CONTENT_ENC_LEN	127





/* --------------------------------------------------------------- */

/*
 * EMSD Point-to-point Envelope
 */


typedef STR_String		EMSD_AsciiString;

typedef struct EMSD_LocalMessageId
{
    OS_Uint32		    submissionTime;
    OS_Uint32		    dailyMessageNumber;
} EMSD_LocalMessageId;

typedef struct EMSD_MessageId
{
    ASN_ChoiceSelector 	    choice;
#define	EMSD_MESSAGEID_CHOICE_LOCAL	0
#define	EMSD_MESSAGEID_CHOICE_RFC822	1

    union
    {
	EMSD_LocalMessageId 	local;		      /* choice 0 */
	EMSD_AsciiString 	rfc822;		      /* choice 1 */
    } un;
} EMSD_MessageId;

typedef struct EMSD_EMSDAddress
{
    OS_Uint32 		    emsdAddress;
    struct
    {
	OS_Boolean 		exists;
	EMSD_AsciiString		data;
    } emsdName;
} EMSD_EMSDAddress; 


typedef struct EMSD_ORAddress
{
    ASN_ChoiceSelector 	    choice;
#define	EMSD_ORADDRESS_CHOICE_LOCAL	0
#define	EMSD_ORADDRESS_CHOICE_RFC822	1

    union
    {
	EMSD_EMSDAddress 		local;		      /* choice 0 */
	EMSD_AsciiString 	rfc822;		      /* choice 1 */
    } un;
} EMSD_ORAddress;



typedef enum
{
    EMSD_ContentType_Reserved			= 0,
    EMSD_ContentType_Probe			= 1,
    EMSD_ContentType_DeliveryReport		= 2,
    EMSD_ContentType_Ipm95			= 32,
    EMSD_ContentType_VoiceMessage		= 33
} EMSD_ContentTypeBuiltIn;

/* EMSD_ContentType is usually one of the BuiltIn types, but may vary */
typedef OS_Uint32	EMSD_ContentType;

typedef struct EMSD_CredentialsSimple
{
    struct
    {
	OS_Boolean 		exists;
	EMSD_EMSDAddress 		data;
    } emsdAddress;

    struct
    {
	OS_Boolean 		exists;
	STR_String	 	data;
    } password;
} EMSD_CredentialsSimple;

typedef struct EMSD_SecurityElement
{
    ASN_ChoiceSelector 	    choice;
#define	EMSD_SECURITY_CHOICE_SIMPLE	0

    union
    {
	EMSD_CredentialsSimple 	simple;
    } credentials;

    struct
    {
	OS_Boolean 		exists;
	OS_Uint32 		data;
    } contentIntegrityCheck;
} EMSD_SecurityElement;


typedef struct EMSD_SegmentData
{
    OS_Uint32	 	    sequenceId;
    OS_Uint32		    data;
#define	thisSegmentNumber	data
#define	numSegments		data
} EMSD_SegmentData;


typedef struct EMSD_SegmentInfo
{
    ASN_ChoiceSelector	    choice;
#define EMSD_SEGMENTINFO_CHOICE_FIRST	0
#define EMSD_SEGMENTINFO_CHOICE_OTHER	1
    
    EMSD_SegmentData 	    segmentData;
} EMSD_SegmentInfo;


typedef enum
{
    EMSD_Restrict_Update				= 1,
    EMSD_Restrict_Remove				= 2
} EMSD_Restrict;

/*
 * In order to handle different Operation values for different versions of the
 * EMSD spec, we encode the protocol version number in the high-order 16 bits
 * of the Operation value.  Prior to issuing an ESROS request, we mask off the
 * high-order 16 bits.
 *
 * The minor version number may have its high-order bit set to indicate that a
 * bug was introduced in the protocol which generated the incorrect operation
 * value.
 */
#define	EMSD_OP_VALUE(operation_major_minor)	\
						(operation_major_minor & 0xff)

#define	EMSD_OP_PROTO(operation, major, minor)				\
						((major << 24) |	\
						 (minor << 16) |	\
						 (operation))

#undef BUG
#define	BUG(n)					((n) | 0x80)

typedef enum
{
    /* Version 1.0 operation values (actually wrong, but released) */
    EMSD_Operation_Submission_1_0_WRONG		= EMSD_OP_PROTO(0, 1, 0),
    EMSD_Operation_Delivery_1_0_WRONG		= EMSD_OP_PROTO(1, 1, 0),
    EMSD_Operation_DeliveryControl_1_0_WRONG	= EMSD_OP_PROTO(2, 1, 0),
    EMSD_Operation_SubmissionControl_1_0_WRONG	= EMSD_OP_PROTO(4, 1, 0),
    EMSD_Operation_GetConfiguration_1_0_WRONG	= EMSD_OP_PROTO(7, 1, 0),
    EMSD_Operation_SetConfiguration_1_0_WRONG	= EMSD_OP_PROTO(8, 1, 0),

    /* not used.  These are the correct Version 1.0 operation values ... */
    EMSD_Operation_Submission_1_0		= EMSD_OP_PROTO(1, 1, BUG(0)),
    EMSD_Operation_Delivery_1_0			= EMSD_OP_PROTO(3, 1, BUG(0)),
    EMSD_Operation_DeliveryControl_1_0		= EMSD_OP_PROTO(2, 1, BUG(0)),
    EMSD_Operation_SubmissionControl_1_0		= EMSD_OP_PROTO(4, 1, BUG(0)),
    EMSD_Operation_GetConfiguration_1_0		= EMSD_OP_PROTO(7, 1, BUG(0)),
    EMSD_Operation_SetConfiguration_1_0		= EMSD_OP_PROTO(8, 1, BUG(0)),
    /* ... not used */

    /* Version 1.1 operation values */
    EMSD_Operation_Submission_1_1		= EMSD_OP_PROTO(33, 1, 1),
    EMSD_Operation_Delivery_1_1			= EMSD_OP_PROTO(35, 1, 1),
    EMSD_Operation_DeliveryControl_1_1		= EMSD_OP_PROTO(2, 1, 1),
    EMSD_Operation_SubmissionControl_1_1		= EMSD_OP_PROTO(4, 1, 1),
    EMSD_Operation_DeliveryVerify_1_1		= EMSD_OP_PROTO(5, 1, 1),
    EMSD_Operation_SubmissionVerify_1_1		= EMSD_OP_PROTO(6, 1, 1),
    EMSD_Operation_GetConfiguration_1_1		= EMSD_OP_PROTO(7, 1, 1),
    EMSD_Operation_SetConfiguration_1_1		= EMSD_OP_PROTO(8, 1, 1)
} EMSD_Operations;

#undef BUG

typedef enum
{
    EMSD_Waiting_LongContent			= 2,
    EMSD_Waiting_LowPriority			= 3
} EMSD_WaitingMessages;

typedef enum
{
    EMSD_Priority_NonUrgent			= 0,
    EMSD_Priority_Normal				= 1,
    EMSD_Priority_Urgent				= 2
} EMSD_Priority;


typedef enum
{
    EMSD_Error_ProtocolVersionNotRecognized	= 1,
    EMSD_Error_SubmissionControlViolated		= 2,
    EMSD_Error_MessageIdentifierInvalid		= 3,
    EMSD_Error_SecurityError			= 4,
    EMSD_Error_DeliveryControlViolated		= 5,
    EMSD_Error_ResourceError			= 6,
    EMSD_Error_ProtocolViolation			= 7
} EMSD_Error;


typedef struct EMSD_ErrorRequest
{
    OS_Uint32		    error;
} EMSD_ErrorRequest;


typedef struct EMSD_SubmitArgument
{
    struct
    {
	OS_Boolean 		exists;
	EMSD_SecurityElement 	data;
    } security;

    struct
    {
	OS_Boolean 		exists;
	EMSD_SegmentInfo 	data;
    } segmentInfo;

    EMSD_ContentType 	    contentType;

    void *		    hContents;	/* Buffer pointer */
} EMSD_SubmitArgument;

typedef struct EMSD_SubmitResult
{
    /* localMessageId also contains the submission time */
    EMSD_LocalMessageId 	    localMessageId;
} EMSD_SubmitResult;

typedef struct EMSD_DeliveryControlArgument
{
    struct
    {
	OS_Boolean 		exists;
	OS_Uint32 		data; /* one of EMSD_Restrict values;
					 default: Update */
    } restrict;

    struct
    {
	OS_Boolean 		exists;
	ASN_BitString		data; /* EMSD_Operations bits */
    } permissibleOperations;

    struct
    {
	OS_Boolean 		exists;
	OS_Uint32 		data;
    } permissibleMaxContentLength;

    struct
    {
	OS_Boolean 		exists;
	OS_Uint32 		data; /* one of EMSD_Priority values */
    } permissibleLowestPriority;

    struct
    {
	OS_Boolean 		exists;
	EMSD_SecurityElement 	data;
    } security;

    struct
    {
	OS_Boolean 		exists;
	STR_String	 	data;
    } userFeatures;
} EMSD_DeliveryControlArgument;



typedef struct EMSD_DeliveryControlResult
{
    struct
    {
	OS_Boolean 		exists;
	ASN_BitString		data; /* EMSD_Operations bits */
				      /* default: { 0 } */
    } waitingOperations;

    struct
    {
	OS_Boolean 		exists;
	ASN_BitString		data; /* EMSD_WaitingMessages bits */
				      /* default: { 0 } */
    } waitingMessages;

    struct
    {
	ASN_Count 		count;
	OS_Uint8 * 		data[EMSDUB_NUM_CONTENT_TYPES];
    } waitingContentTypes;
} EMSD_DeliveryControlResult;


typedef struct EMSD_DeliverArgument
{
    EMSD_MessageId 	    messageId;

    /*
     * If messageId is a LocalMessageId, use submissionTime from it,
     * rather than from here
     */
    struct
    {
	OS_Boolean 		exists;
	OS_Uint32	 	data;
    } submissionTime;

    OS_Uint32	 	    deliveryTime;

    struct
    {
	OS_Boolean 		exists;
	EMSD_SecurityElement 	data;
    } security;

    struct
    {
	OS_Boolean 		exists;
	EMSD_SegmentInfo 	data;
    } segmentInfo;

    EMSD_ContentType 	    contentType;

    void *		    hContents;	/* Buffer pointer */
} EMSD_DeliverArgument;


typedef struct EMSD_DeliveryVerifyArgument
{
    EMSD_MessageId 	    messageId;
} EMSD_DeliveryVerifyArgument;


typedef enum
{
    EMSD_DeliveryVerifyStatus_NoReportIsSentOut		= 1,
    EMSD_DeliveryVerifyStatus_DeliveryReportIsSentOut	= 2,
    EMSD_DeliveryVerifyStatus_NonDeliveryReportIsSentOut	= 3
} EMSD_DeliveryVerifyStatus;


typedef struct EMSD_DeliveryVerifyResult
{
    /* deliveryStatus is one of EMSD_DeliveryVerifyStatus values */
    OS_Uint32			deliveryStatus;
} EMSD_DeliveryVerifyResult;


typedef struct EMSD_SubmissionVerifyArgument
{
    EMSD_MessageId		messageId;
} EMSD_SubmissionVerifyArgument;


typedef enum
{
    EMSD_SubmissionVerifyStatus_SendMessage		= 1,
    EMSD_SubmissionVerifyStatus_DropMessage		= 2
} EMSD_SubmissionVerifyStatus;


typedef struct EMSD_SubmissionVerifyResult
{
    /* submissionStatus is one of EMSD_SubmissionVerifyStatus values */
    OS_Uint32			submissionStatus;
} EMSD_SubmissionVerifyResult;



/* --------------------------------------------------------------- */

/*
 * EMSD Interpersonal Message
 */

typedef enum
{
    EMSD_PerMessageFlags_Priority_NonUrgent		= 0,
    EMSD_PerMessageFlags_Priority_Urgent			= 1,

    EMSD_PerMessageFlags_Importance_Low			= 2,
    EMSD_PerMessageFlags_Importance_High			= 3,

    EMSD_PerMessageFlags_AutoForwarded			= 4
} EMSD_PerMessageFlags;


typedef enum
{
    EMSD_PerRecipFlags_RecipientType_Copy		= 0,
    EMSD_PerRecipFlags_RecipientType_BlindCopy		= 1,

    EMSD_PerRecipFlags_NotificationRequest_RN		= 2,
    EMSD_PerRecipFlags_NotificationRequest_NRN		= 3,
    EMSD_PerRecipFlags_NotificationRequest_IpmReturn	= 4,

    EMSD_PerRecipFlags_ReportRequest_NonDelivery		= 5,
    EMSD_PerRecipFlags_ReportRequest_Delivery		= 6,

    EMSD_PerRecipFlags_ReplyRequested			= 7
} EMSD_PerRecipFlags;

typedef struct EMSD_RecipientData
{
    EMSD_ORAddress		recipient;

    /* Bit flags, selected from EMSD_PerRecipFlags */
    struct
    {
	OS_Boolean		    exists;
	ASN_BitString		    data; /* default: NonDelivery */
    } perRecipFlags;
} EMSD_RecipientData;

typedef struct EMSD_RecipientDataList
{
    ASN_Count			count;
    EMSD_RecipientData 		data[EMSDUB_MAX_RECIPIENTS];
} EMSD_RecipientDataList;

typedef struct EMSD_ReplyTo
{
    ASN_Count 			count;
    EMSD_ORAddress 		data[EMSDUB_MAX_REPLY_TO];
} EMSD_ReplyTo;

typedef struct EMSD_Extension
{
    EMSD_AsciiString 		label;
    EMSD_AsciiString 		value;
} EMSD_Extension;

typedef struct EMSD_Extensions
{
    ASN_Count 		     count;
    EMSD_Extension	     data[EMSDUB_MAX_HEADER_EXTENSIONS];
} EMSD_Extensions;


typedef enum
{
    EMSD_CompressionMethod_None				= 0,
    EMSD_CompressionMethod_LempelZiv			= 1
} EMSD_CompressionMethod;


typedef struct
{
    struct
    {
	OS_Boolean 			exists;
	EMSD_ORAddress 			data;
    } sender;

    EMSD_ORAddress 		    originator;

    EMSD_RecipientDataList	    recipientDataList;

    /* Bit flags, selected from EMSD_PerMessageFlags */
    struct
    {
	OS_Boolean 			exists;
	ASN_BitString			data; /* default: { 0 } */
    } perMessageFlags;

    struct
    {
	OS_Boolean			exists;
        EMSD_ReplyTo		        data;
    } replyTo;

    struct
    {
	OS_Boolean			exists;
	EMSD_MessageId 			data;
    } repliedToIpm;

    struct
    {
	OS_Boolean			exists;
	EMSD_AsciiString 		data;
    } subject;
	
    EMSD_Extensions		     extensions;

    struct
    {
	OS_Boolean			exists;
	EMSD_AsciiString 		data;	    
    } mimeVersion;

    struct
    {
	OS_Boolean			exists;
	EMSD_AsciiString 		data;	    
    } mimeContentType;

    struct
    {
	OS_Boolean			exists;
	EMSD_AsciiString			data;
    } mimeContentId;

    struct
    {
	OS_Boolean			exists;
	EMSD_AsciiString			data;
    } mimeContentDescription;

    struct
    {
	OS_Boolean			exists;
	EMSD_AsciiString			data;
    } mimeContentTransferEncoding;
} EMSD_IpmHeading;

typedef struct EMSD_IpmBody
{
    struct
    {
	OS_Boolean			exists;
	OS_Uint32			data; /* one of EMSD_CompressionMethod
						 values */
    } compressionMethod;

    EMSD_AsciiString 		    bodyText;
} EMSD_IpmBody;


typedef struct EMSD_Message
{
    EMSD_IpmHeading		    heading;
    struct
    {
	OS_Boolean			exists;
	EMSD_IpmBody			data;
    } body;
} EMSD_Message;

/* --------------------------------------------------------------- */

/*
 * EMSD Delivery/Non-delivery Report
 */

typedef enum
{
    EMSD_NonDeliveryReason_TransferFailure			= 0,
    EMSD_NonDeliveryReason_UnableToTransfer			= 1,
    EMSD_NonDeliveryReason_RestrictedDelivery			= 2
} EMSD_NonDeliveryReason;


typedef enum
{
    EMSD_NonDeliveryDiagnostic_Congestion			= 0,
    EMSD_NonDeliveryDiagnostic_LoopDetected			= 1,
    EMSD_NonDeliveryDiagnostic_RecipientUnavailable		= 2,
    EMSD_NonDeliveryDiagnostic_MaximumTimeExpired		= 3,
    EMSD_NonDeliveryDiagnostic_ContentTooLong			= 4,
    EMSD_NonDeliveryDiagnostic_SizeConstraintViolated		= 5,
    EMSD_NonDeliveryDiagnostic_ProtocolViolation			= 6,
    EMSD_NonDeliveryDiagnostic_ContentTypeNotSupported		= 7,
    EMSD_NonDeliveryDiagnostic_TooManyRecipients			= 8,
    EMSD_NonDeliveryDiagnostic_LineTooLong			= 9,
    EMSD_NonDeliveryDiagnostic_UnrecognizedAddress		= 10,
    EMSD_NonDeliveryDiagnostic_RecipientUnknown			= 11,
    EMSD_NonDeliveryDiagnostic_RecipientRefusedToAccept		= 12,
    EMSD_NonDeliveryDiagnostic_UnableToCompleteTransfer		= 13,
    EMSD_NonDeliveryDiagnostic_TransferAttemptsLimitReached	= 14
} EMSD_NonDeliveryDiagnostic;

typedef struct EMSD_DeliveryReport
{
    OS_Uint32	 		    messageDeliveryTime;
} EMSD_DeliveryReport;

typedef struct EMSD_NonDeliveryReport
{
    OS_Uint32		 	    nonDeliveryReason; /* one of
							  EMSD_NonDeliveryReason
							  values */
    
    struct
    {
	OS_Boolean			exists;
	OS_Uint32			data; /* one of
						 EMSD_NonDeliveryDiagnostic
						 values */
    } nonDeliveryDiagnostic;
    
    struct
    {
	OS_Boolean			exists;
	EMSD_AsciiString 		data;
    } explanation;
} EMSD_NonDeliveryReport;

typedef struct EMSD_PerRecipReportData
{
    EMSD_ORAddress 		    recipient;
				    
    struct
    {
	ASN_ChoiceSelector		choice;
#define	EMSD_DELIV_CHOICE_DELIVERY	0
#define	EMSD_DELIV_CHOICE_NONDELIVERY	1

	union
	{
						   /* choice 0 */
	    EMSD_DeliveryReport 		    deliveryReport;
	    
						   /* choice 1 */
	    EMSD_NonDeliveryReport 	    nonDeliveryReport;
	} un;
    } report;
} EMSD_PerRecipReportData;

typedef struct EMSD_PerRecipReportDataList
{
    ASN_Count 			    count;
    EMSD_PerRecipReportData 	    data[EMSDUB_MAX_RECIPIENTS];
} EMSD_PerRecipReportDataList;

typedef struct EMSD_ReportDelivery
{
    EMSD_MessageId		    msgIdOfReportedMsg;

    EMSD_PerRecipReportDataList	    recipDataList;

    struct
    {
	OS_Boolean			exists;
	
	EMSD_ContentType 		contentType;

	struct
	{
	    OS_Boolean 			    exists;
	    void * 			    data;	/* Buffer pointer */
	} returnedContents;
    } returnedContent;
} EMSD_ReportDelivery;

/* --------------------------------------------------------------- */

/*
 * EMSD Probe
 */

typedef struct EMSD_RecipientList
{
    ASN_Count			count;
    EMSD_ORAddress		data[EMSDUB_MAX_RECIPIENTS];
} EMSD_RecipientList;

typedef struct EMSD_ProbeSubmission
{
    EMSD_ORAddress 		sender;

    EMSD_RecipientList		recipients;

    struct
    {
	OS_Boolean 		    exists;
	EMSD_ContentType 	    data;
    } contentType;

    struct
    {
	OS_Boolean 		    exists;
	OS_Uint32 		    data;
    } contentLength;
} EMSD_ProbeSubmission;



ReturnCode
EMSD_compileIpm1_0(unsigned char * pCStruct,
		  OS_Boolean * pExists,
		  EMSD_Message * pMessage,
		  OS_Uint8 tag,
		  QU_Head * pQ);

ReturnCode
EMSD_compileIpm1_1(unsigned char * pCStruct,
		  OS_Boolean * pExists,
		  EMSD_Message * pMessage,
		  OS_Uint8 tag,
		  QU_Head * pQ);

ReturnCode
EMSD_compileSubmitArgument1_0(unsigned char * pCStruct,
			     OS_Boolean * pExists,
			     EMSD_SubmitArgument * pArg,
			     OS_Uint8 tag,
			     QU_Head * pQ);

ReturnCode
EMSD_compileSubmitArgument1_1(unsigned char * pCStruct,
			     OS_Boolean * pExists,
			     EMSD_SubmitArgument * pArg,
			     OS_Uint8 tag,
			     QU_Head * pQ);

ReturnCode
EMSD_compileSubmitResult1_0(unsigned char * pCStruct,
			   OS_Boolean * pExists,
			   EMSD_SubmitResult * pResult,
			   OS_Uint8 tag,
			   QU_Head * pQ);

ReturnCode
EMSD_compileSubmitResult1_1(unsigned char * pCStruct,
			   OS_Boolean * pExists,
			   EMSD_SubmitResult * pResult,
			   OS_Uint8 tag,
			   QU_Head * pQ);

ReturnCode
EMSD_compileDeliveryControlArgument1_0(unsigned char * pCStruct,
				      OS_Boolean * pExists,
				      EMSD_DeliveryControlArgument *pArg,
				      OS_Uint8 tag,
				      QU_Head * pQ);

ReturnCode
EMSD_compileDeliveryControlArgument1_1(unsigned char * pCStruct,
				      OS_Boolean * pExists,
				      EMSD_DeliveryControlArgument *pArg,
				      OS_Uint8 tag,
				      QU_Head * pQ);

ReturnCode
EMSD_compileDeliveryControlResult1_0(unsigned char * pCStruct,
				    OS_Boolean * pExists,
				    EMSD_DeliveryControlResult * pResult,
				    OS_Uint8 tag,
				    QU_Head * pQ);

ReturnCode
EMSD_compileDeliveryControlResult1_1(unsigned char * pCStruct,
				    OS_Boolean * pExists,
				    EMSD_DeliveryControlResult * pResult,
				    OS_Uint8 tag,
				    QU_Head * pQ);

ReturnCode
EMSD_compileDeliveryVerifyArgument1_1(unsigned char * pCStruct,
				     OS_Boolean * pExists,
				     EMSD_DeliveryVerifyArgument * pArg,
				     OS_Uint8 tag,
				     QU_Head * pQ);

ReturnCode
EMSD_compileDeliveryVerifyResult1_1(unsigned char * pCStruct,
				   OS_Boolean * pExists,
				   EMSD_DeliveryVerifyResult * pArg,
				   OS_Uint8 tag,
				   QU_Head * pQ);

ReturnCode
EMSD_compileSubmissionVerifyArgument1_1(unsigned char * pCStruct,
				       OS_Boolean * pExists,
				       EMSD_SubmissionVerifyArgument * pArg,
				       OS_Uint8 tag,
				       QU_Head * pQ);

ReturnCode
EMSD_compileSubmissionVerifyResult1_1(unsigned char * pCStruct,
				     OS_Boolean * pExists,
				     EMSD_SubmissionVerifyResult * pArg,
				     OS_Uint8 tag,
				     QU_Head * pQ);

ReturnCode
EMSD_compileDeliverArgument1_0(unsigned char * pCStruct,
			      OS_Boolean * pExists,
			      EMSD_DeliverArgument * pArg,
			      OS_Uint8 tag,
			      QU_Head * pQ);

ReturnCode
EMSD_compileDeliverArgument1_1(unsigned char * pCStruct,
			      OS_Boolean * pExists,
			      EMSD_DeliverArgument * pArg,
			      OS_Uint8 tag,
			      QU_Head * pQ);

ReturnCode
EMSD_compileProbe1_0(unsigned char * pCStruct,
		    OS_Boolean * pExists,
		    EMSD_ProbeSubmission * pProbe,
		    OS_Uint8 tag,
		    QU_Head * pQ);

ReturnCode
EMSD_compileProbe1_1(unsigned char * pCStruct,
		    OS_Boolean * pExists,
		    EMSD_ProbeSubmission * pProbe,
		    OS_Uint8 tag,
		    QU_Head * pQ);

ReturnCode
EMSD_compileReportDelivery1_0(unsigned char * pCStruct,
			     OS_Boolean * pExists,
			     EMSD_ReportDelivery * pReport,
			     OS_Uint8 tag,
			     QU_Head * pQ);

ReturnCode
EMSD_compileReportDelivery1_1(unsigned char * pCStruct,
			     OS_Boolean * pExists,
			     EMSD_ReportDelivery * pReport,
			     OS_Uint8 tag,
			     QU_Head * pQ);

ReturnCode
EMSD_compileErrorRequest1_0(unsigned char * pCStruct,
			   OS_Boolean * pExists,
			   EMSD_ErrorRequest * pErrorReq,
			   OS_Uint8 tag,
			   QU_Head * pQ);

ReturnCode
EMSD_compileErrorRequest1_1(unsigned char * pCStruct,
			   OS_Boolean * pExists,
			   EMSD_ErrorRequest * pErrorReq,
			   OS_Uint8 tag,
			   QU_Head * pQ);

#endif /* __EMSD_H__ */
