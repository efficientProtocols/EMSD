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


#ifndef __UASH_H__
#define	__UASH_H__

#include "estd.h"
#include "addr.h"
#include "queue.h"
#include "mtsua.h"
#include "target.h"


/* Recipient Address */
typedef struct
{
    QU_ELEMENT;

    enum
    {
	UASH_RecipientStyle_Emsd,
	UASH_RecipientStyle_Rfc822
    }			style;

    union
    {
	/* EMSD address format */
	struct
	{
	    OS_Uint32		emsdAddr;
	    char *		pName;
	}		    recipEmsd;

	/* RFC-822 address format */
	char * 		    pRecipRfc822;
    } un;
} UASH_Recipient;


/* Recipient information */
typedef struct
{
    QU_ELEMENT;

    /*
     * Recipient address
     */
    UASH_Recipient	recipient;


    /*
     * Per-recipient Flags
     */

    /* Recipient Type */
    enum
    {
	UASH_RecipientType_Primary = 0,
	UASH_RecipientType_Copy,
	UASH_RecipientType_BlindCopy
    }			recipientType;

    /* Notification Types */
    OS_Boolean		bReceiptNotificationRequested;
    OS_Boolean		bNonReceiptNotificationRequested;
    OS_Boolean		bMessageReturnRequested;

    /* Report Request Types */
    OS_Boolean		bNonDeliveryReportRequested;
    OS_Boolean		bDeliveryReportRequested;

    /* Request for Reply requested */
    OS_Boolean		bReplyRequested;
} UASH_RecipientInfo;


/* Unrecognized Header Fields (Extensions) */
typedef struct
{
    QU_ELEMENT;

    /* Extension label */
    char *	    pLabel;

    /* Extension value */
    char *	    pValue;
} UASH_Extension;



/*
 * The heading of a message.
 *
 * Any character pointer fields which are unspecified will be NULL.
 * Variable-length lists (e.g. recipients, extensions) are empty queues to
 * indicate unspecified data.
 */
typedef struct UASH_MessageHeading
{
    /* Originator */
    UASH_Recipient 	originator;

    /* Sender, if different than Originator */
    UASH_Recipient 	sender;

    /* Recipient list */
    QU_Head 		recipInfoQHead;	/* Queue of RecipientInfo */

    /* Who to reply to, if different than sender or originator */
    QU_Head 		replyToQHead;	/* Queue of Recipient */

    /* Message ID of replied-to message */
    char * 		pRepliedToMessageId;

    /* Subject of the message */
    char * 		pSubject;

    /* Priority values */
    enum
    {
	UASH_Priority_Normal = 0,
	UASH_Priority_NonUrgent,
	UASH_Priority_Urgent
    } 			priority;

    /* Importance values */
    enum
    {
	UASH_Importance_Normal = 0,
	UASH_Importance_Low,
	UASH_Importance_High
    } 			importance;

    /* Indicator of whether this message was auto-forwarded */
    OS_Boolean 		bAutoForwarded;

    /* Mime version */
    char *		pMimeVersion;

    /* Mime Content Type */
    char *		pMimeContentType;

    /* Mime Content Id */
    char *		pMimeContentId;

    /* Mime Content Description */
    char *		pMimeContentDescription;

    /* Mime Content Transfer Encoding */
    char *		pMimeContentTransferEncoding;

    /* Additional header fields */
    QU_Head 		extensionQHead;	/* Queue of Extension */

} UASH_MessageHeading;



/*
 * Message Delivery Status indicators
 */
typedef enum
{
    UASH_DeliverStatus_UnAcknowledged				= 0,
    UASH_DeliverStatus_Acknowledged,
    UASH_DeliverStatus_NonDelSent,
    UASH_DeliverStatus_PossibleNonDelSent
} UASH_DeliverStatus;


/*
 * Non-Delivery indicators
 */

/* Reason codes */
typedef enum
{
    UASH_NonDeliveryReason_TransferFailure			= 0,
    UASH_NonDeliveryReason_UnableToTransfer			= 1,
    UASH_NonDeliveryReason_RestrictedDelivery			= 2
} UASH_NonDeliveryReason;


typedef enum
{
    UASH_NonDeliveryDiagnostic_Unspecified	= -1,

    UASH_NonDeliveryDiagnostic_Congestion			= 0,
    UASH_NonDeliveryDiagnostic_LoopDetected			= 1,
    UASH_NonDeliveryDiagnostic_RecipientUnavailable		= 2,
    UASH_NonDeliveryDiagnostic_MaximumTimeExpired		= 3,
    UASH_NonDeliveryDiagnostic_ContentTooLong			= 4,
    UASH_NonDeliveryDiagnostic_SizeConstraintViolated		= 5,
    UASH_NonDeliveryDiagnostic_ProtocolViolation		= 6,
    UASH_NonDeliveryDiagnostic_ContentTypeNotSupported		= 7,
    UASH_NonDeliveryDiagnostic_TooManyRecipients		= 8,
    UASH_NonDeliveryDiagnostic_LineTooLong			= 9,
    UASH_NonDeliveryDiagnostic_UnrecognizedAddress		= 10,
    UASH_NonDeliveryDiagnostic_RecipientUnknown			= 11,
    UASH_NonDeliveryDiagnostic_RecipientRefusedToAccept		= 12,
    UASH_NonDeliveryDiagnostic_UnableToCompleteTransfer		= 13,
    UASH_NonDeliveryDiagnostic_TransferAttemptsLimitReached	= 14
} UASH_NonDeliveryDiagnostic;



/*
 * UA_init()
 *
 * Initialize the UA module.
 *
 * Parameters:
 *
 *   pMtsNsap --
 *       NSAP Address of the MTS to whom messages should be submitted.
 *
 *   pPassword --
 *       Password used to validate messages being delivered.  This is a
 *       null-terminated string.  If password validation is not to be used,
 *       the parameter should be NULL.
 *
 *   bDeliveryVerifySupported --
 *       TRUE if this UA should support the Delivery Verify operation; FALSE
 *       otherwise.
 *
 *   nvmVerifyTransactionType --
 *       Transaction Type Value to use, in non-volatile memory, to identify
 *       non-volatile Submission Verify elements saved by the UA Core module.
 *       This transaction type should be one which is not in use elsewhere by
 *       the application.
 *
 *   nvmDupTransactionType --
 *       Transaction Type Value to use, in non-volatile memory, to identify
 *       non-volatile Duplicate Detection elements saved by the UA Core
 *       module.  This transaction type should be one which is not in use
 *       elsewhere by the application.
 *
 *   concurrentOperationsInbound --
 *       Number of concurrent Operations which may be active, initiated by the
 *       MTS.
 *
 *   concurrentOperationsOutbound --
 *       Number of concurrent Operations which may be active, initiated by the
 *       UA.
 *
 *   pfMessageReceived --
 *       Pointer to a function which will be called as soon as a complete
 *       message has been received by the UA, from the MTS.  The message is
 *       passed to the user prior to receiving confirmation from the MTS that
 *       the MTS knows that the message has been successfully delivered.  The
 *       message "status", at the time that the function pointed to by
 *       pfMessageReceived is called, is implicitly
 *       UA_DeliverStatus_UnAcknowledged, and it is the UA Shell's
 *       responsibility to keep track of the message state.
 *
 *       Parameters to (* pfMessageReceived)():
 *
 *         pMessageId --
 *             Message Id of this message.  When the UA has received
 *             confirmation that the MTS knows that the message has been
 *             successfully delivered, or when the UA receives a failure
 *             indication indicating that it does _not_ know (and will not
 *             find out) whether the MTS knows that it delivered the message,
 *             this same message id will be passed to the function pointed to
 *             by pfModifyDeliverStatus, so that the UA Shell can update its
 *             locally-maintained message state.
 *
 *         submissionTime -
 *             Time that this message was originally submitted, if known.  If
 *             not known, the time value will zero.  Otherwise, the time is
 *             the number of seconds since midnight, 1 January 1970.
 *
 *         deliveryTime --
 *             Time that this message was delivered to the UA.  The delivery
 *             time is sent as part of the message from the MTS, and is thus
 *             the MTS's idea of the delivery time.  The value of this
 *             parameter is the number of seconds since midnight, 1 January
 *             1970.
 *
 *         pMessageHeading --
 *             Pointer to a Message Heading structure, which contains all of
 *             the per-message fields, the recipients of the message (along
 *             with each recipient's per-recipient flags), etc.  It is the UA
 *             Shell's responsibility to copy any desired data from this
 *             structure, as the structure and all data pointed to therein may
 *             be freed upon return from the function pointed to by
 *             pfMessageReceived.
 *
 *         pBodyText --
 *             Character string containing the body text.  It is the UA
 *             Shell's responsibility to copy the body text.  The data may be
 *             freed upon return from the function pointed to by
 *             pfMessageReceived.
 *
 *   pfModifyDeliverStatus --
 *       Pointer to a function which will be called when, either the MTS has
 *       confirmed that it knows that the message is successfully delivered,
 *       or when the UA determines that it can't easily find out whether the
 *       MTS knows that the message has been successfully delivered.
 *
 *       Parameters to (* pfModifyDeliverStatus)():
 *
 *         pMessageId --
 *             Message id of this message.  This is the same id that was
 *             passed to the UA Shell, via (* pfMessageReceived)() when the
 *             message was first received.
 *
 *         messageStatus --
 *             New status of this message.  If the MTS has confirmed that it
 *             knows that the message was delivered, then this parameter will
 *             be UA_DeliverStatus_DeliverAcknowledged.  If the MTS confirmed
 *             that it knows that the message was delivered, but it didn't
 *             find out until late and had thus already sent a non-delivery
 *             report to the originator, then this parameter will be the value
 *             UA_DeliverStatus_NonDelSent.  If the UA was not able to get
 *             confirmation from the MTS regarding whether the MTS knows that
 *             the message has been delivered, this parameter will be
 *             UA_DeliverStatus_PossibleNonDelSent, indicating that the MTS
 *             may have sent a non-delivery report to the originator.
 *
 *   pfSubmitSuccess --
 *       Pointer to a function which will be called when the MTS has confirmed
 *       submission of the message.
 *
 *       Parameters to (* pfSubmitSuccess)():
 *
 *         hMessage --
 *             Handle to this message.  This is the handle that was provided
 *             to the UA Shell by UA_submitMessage().
 *
 *         pMessageId --
 *             The message id of the successfully-submitted message.  The
 *             message id was generated by the MTS, and is unique world-wide.
 *
 *   pfSubmitFail --
 *       Pointer to a function which will be called when the UA determines
 *       that it could not get confirmation from the MTS that a message has
 *       been successfully submitted.  No further attempts will be made to
 *       submit the message, and the user may assume that the message has not
 *       been further submitted by the MTS into the email world.
 *
 *       Parameters to (* pfSubmitFail)():
 *
 *         hMessage --
 *             Handle to this message.  This is the handle that was provided
 *             to the UA Shell by UA_submitMessage().
 *
 *         bTransient --
 *             TRUE if an ERROR.ind was received, specifying that the failure
 *             reason was one of a transient nature; FALSE otherwise.
 *
 *   pfDeliveryReport --
 *       Pointer to a function which will be called when a Delivery Reports,
 *       for a previously-submitted message, arrives.
 *
 *       Parameters to (* pfDeliveryReport)():
 *
 *         pMessageId --
 *             The message id of the submitted message.  This is the same
 *             message id that was provided by the callback function
 *             (* pfSubmitSuccess)()
 *
 *         deliveryTime --
 *             Time that this message was delivered to the recipient.  The
 *             value of this parameter is the number of seconds since
 *             midnight, 1 January 1970.
 *
 *   pfNonDeliveryReport --
 *       Pointer to a function which will be called when a Non-Delivery
 *       Report, for a previously-submitted message, arrives.
 *
 *       Parameters to (* pfNonDeliveryReport)():
 *
 *         pMessageId --
 *             The message id of the submitted message.  This is the same
 *             message id that was provided by the callback function
 *             (* pfSubmitSuccess)()
 *
 *         reason --
 *             Reason that the message could not be delivered
 *
 *         diagnostic --
 *             Additional (optional) diagnostic as to why the message could
 *             not be delivered.
 *
 *         pExplanation --
 *             Additional (optional) explanation as to why the message could
 *             not be delivered.
 */
ReturnCode
UA_init(N_SapAddr * pMtsNsap,
	char * pPassword,
	OS_Boolean bDeliveryVerifySupported,
	OS_Uint8 nvmVerifyTransType,
	OS_Uint8 nvmDupTransType,
	OS_Uint32 concurrentOperationsInbound,
	OS_Uint32 concurrentOperationsOutbound,
	ReturnCode (* pfMessageReceived)(char * pMessageId,
					 OS_Uint32 submissionTime,
					 OS_Uint32 deliveryTime,
					 UASH_MessageHeading * pMessageHeading,
					 char * pBodyText),
	void (* pfModifyDeliverStatus)(char * pMessageId,
				       UASH_DeliverStatus messageStatus),
	void (* pfSubmitSuccess)(void * hMessage,
				 char * pMessageId,
				 void * pHackData),
	void (* pfSubmitFail)(void * hMessage,
			      OS_Boolean bTransient,
			      void * pHackData),
	void (* pfDeliveryReport)(char * pMessageId,
				  OS_Uint32 deliveryTime),
	void (* pfNonDeliveryReport)(char * pMessageId,
				     UASH_NonDeliveryReason reason,
				     UASH_NonDeliveryDiagnostic diagnostic,
				     char * pExplanation)
#ifdef DELIVERY_CONTROL
      , void (* pfDeliveryControlSuccess)(void * hMessage,
    			    		  EMSD_DeliveryControlResult *,
		    	                  void * hShellReference),
	void (* pfDeliveryControlError)(void * hMessage,
			      	       OS_Boolean bTransient,
		    	               void * hShellReference),
	void (* pfDeliveryControlFail)(void * hMessage,
			      	       OS_Boolean bTransient,
		    	               void * hShellReference)
#endif
								);


/*
 * UA_setPassword()
 *
 * Change the password used for access to the MTS.
 *
 * Parameters:
 *
 *   pPassowrd -- New password to be used.
 *
 * Returns:
 *
 *   Success upon success;
 *   ResourceError if memory could not be allocated for the new password.
 */
ReturnCode
UA_setPassword(char * pPassword);




/*
 * UA_setMtsNsap()
 *
 * Change the MTS NSAP address
 *
 * Parameters:
 *
 *   pMtsNsap -- New MTS NSAP Address
 *
 * Returns:
 *
 *   Success upon success;
 *   ResourceError if memory could not be allocated for the new MTS NSAP.
 *
 * NOTE:
 *
 *   It is the caller's responsibility to verify that the NSAP address format
 *   and value are legal.  No parameter checking is done herein.
 */
ReturnCode
UA_setMtsNsap(N_SapAddr * pMtsNsap);




/*
 * UA_submitMessage()
 *
 * This is the interface function used by the UA Shell, to submit a message to
 * the MTS.
 *
 * Parameters:
 *
 *   pHeading --
 *       Pointer to a Heading structure, containing all of the message header
 *       information for the message being submitted.
 *
 *   pBodyText --
 *       Pointer to the text of the message.
 *
 *   phMessage --
 *       Pointer to a location to put a handle to the submitted message.  The
 *       handle is valid until one of the UA Shell's specified call back
 *       functions (* pfSubmitSuccess)() or (* pfSubmitFail)(), described in
 *       the documentation for UA_init(), is called.
 *
 *       The purpose of this handle is to reference a submitted message during
 *       the time that it is being processed by the UA and the MTS, but before
 *       a message id has been assigned to the message by the MTS.  If the
 *       submission is successful, then the message id will be provided to the
 *       UA Shell through a call to (* pfSubmitSuccess)().  If the submissions
 *       is not successful for some reason, then the UA Shell will be notified
 *       of this via a call to (* pfSubmitFail)().
 */
ReturnCode
UA_submitMessage(UASH_MessageHeading * pHeading,
		 char * pBodyText,
		 void ** phMessage,
		 void * hShellReference);

#ifdef DELIVERY_CONTROL
ReturnCode
UA_issueDeliveryControl(void ** phMessage,
		        void * hShellRefernce);
#endif

/*
 * UA_initCompileTrees()
 *
 * ASN-compile each of the EMSD protocols.
 *
 * Parameters:
 *
 *     pErrorBuf --
 *         Pointer to a buffer in which error strings are placed.
 */
ReturnCode
UA_initCompileTrees(char * pErrorBuf);

/*
 * UA_getProtocolree()
 *
 * Get a pointer to the ASN-compiled tree for the specified protocol.
 *
 * Parameters:
 *
 *     protocol --
 *         Protocol identifier for which a pointer to the ASN-compiled tree is
 *         desired.
 *
 * Returns:
 *     Queue head for desired ASN-compiled tree.
 */
QU_Head *
UA_getProtocolTree(MTSUA_ProtocolType protocol);

#endif /* __UASH_H__ */
