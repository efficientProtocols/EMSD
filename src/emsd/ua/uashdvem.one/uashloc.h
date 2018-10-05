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


#include "estd.h"
#include "nvm.h"
#include "ua.h"

/*
 * Module-global Types
 */

/*
 * Module-global Variables
 */
typedef struct UASH_Globals
{
    /* Trace module handle */
    void * 	    hTM;

    /* A temporary buffer to use throughout */
    char 	    tmpbuf[4096];

    /* Window pointers, for curses */
    WINDOW *	    pFrame;
    WINDOW *	    pWin;

    WINDOW *	    pDocFrame;
    WINDOW *	    pDocWin;

    /* User Input function */
    void	 (* pfInput)(int c);

    /* Transaction identifier of currently displayed message */
    NVM_Transaction hCurrentMessage;

    /* Number of possible responses to a query */
    int		    numQueryResponses;

    /* Highlighed query response */
    int		    queryResponse;
} UASH_Globals;

extern UASH_Globals	uash_globals;

typedef enum
{
    TransType_InBox		= 1,
    TransType_OutBox		= 2,
    TransType_Verify		= 3,
    TransType_DupDetection	= 4,
    TransType_Password		= 5,
    TransType_MtsNSAP		= 6
} TransType;

typedef enum
{
    DelStatus_Unknown		 = -2,
    DelStatus_Delivered		 = -1,
    DelStatus_TransferFailure    = UASH_NonDeliveryReason_TransferFailure,
    DelStatus_UnableToTransfer   = UASH_NonDeliveryReason_UnableToTransfer,
    DelStatus_RestrictedDelivery = UASH_NonDeliveryReason_RestrictedDelivery
} DeliveryStatus;


/* Recipient Address */
typedef struct
{
    NVM_QUELEMENT;

    enum
    {
	RecipientStyle_Emsd		= UASH_RecipientStyle_Emsd,
	RecipientStyle_Rfc822		= UASH_RecipientStyle_Rfc822
    } 				style;

    union
    {
	/* EMSD address format */
	struct
	{
	    OS_Uint32 			emsdAddr;
	    NVM_Memory 			hName;
	} 			    recipEmsd;

	/* RFC-822 address format */
	NVM_Memory 		    hRecipRfc822;
    } un;

    DeliveryStatus 		status;
    UASH_NonDeliveryDiagnostic 	nonDeliveryDiagnostic;
    NVM_Memory			hNolDeliveryExplanation;
} Recipient;


/* Recipient information */
typedef struct
{
    NVM_QUELEMENT;

    /*
     * Recipient address
     */
    Recipient 	    recipient;


    /*
     * Per-recipient Flags
     */

    /* Recipient Type */
    enum
    {
	RecipientType_Primary		= UASH_RecipientType_Primary,
	RecipientType_Copy		= UASH_RecipientType_Copy,
	RecipientType_BlindCopy		= UASH_RecipientType_BlindCopy
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
} RecipientInfo;


/* Unrecognized Header Fields (Extensions) */
typedef struct
{
    NVM_QUELEMENT;

    /* Extension label */
    NVM_Memory	    hLabel;

    /* Extension value */
    NVM_Memory	    hValue;
} Extension;


/*
 * The heading of a message.
 *
 * Any character pointer fields (NVM_Memory) which are unspecified
 * will be NVM_NULL.  Variable-length lists (e.g. recipients,
 * extensions) are empty queues to indicate unspecified data.
 */
typedef struct Message
{
    /* Message Id */
    NVM_Memory		hMessageId;

    /* Submission Time */
    OS_Uint32		submissionTime;

    /* Delivery Time */
    OS_Uint32		deliveryTime;

    union
    {
	/* Delivery Status */
	UASH_DeliverStatus  deliverStatus;

	/* Submission Status: NULL if acknowledged */
	void *		    hSubmission;
    } 			status;

    /* Flags (for User Interface use) */
    unsigned long	flags;

    /* Printable message status */
    NVM_Memory		hPrintableStatus;

    /* Originator */
    Recipient		originator;

    /* Sender, if different than Originator */
    Recipient		sender;

    /* Recipient list */
    NVM_Memory 		hRecipInfoQHead;	/* Queue of RecipientInfo */

    /* Who to reply to, if different than sender or originator */
    NVM_Memory 		hReplyToQHead;		/* Queue of Recipient */

    /* Message ID of replied-to message */
    NVM_Memory 		hRepliedToMessageId;

    /* Subject of the message */
    NVM_Memory 		hSubject;

    /* Priority values */
    enum
    {
	Priority_Normal			= UASH_Priority_Normal,
	Priority_NonUrgent		= UASH_Priority_NonUrgent,
	Priority_Urgent			= UASH_Priority_Urgent
    } 			priority;

    /* Importance values */
    enum
    {
	Importance_Normal		= UASH_Importance_Normal,
	Importance_Low			= UASH_Importance_Low,
	Importance_High			= UASH_Importance_High
    } 			importance;

    /* Indicator of whether this message was auto-forwarded */
    OS_Boolean 		bAutoForwarded;

    /* Mime version */
    NVM_Memory		hMimeVersion;

    /* Mime Content Type */
    NVM_Memory		hMimeContentType;

    /* Mime Content Id */
    NVM_Memory		hMimeContentId;

    /* Mime Content Description */
    NVM_Memory		hMimeContentDescription;

    /* Mime Content Transfer Encoding */
    NVM_Memory		hMimeContentTransferEncoding;

    /* Additional header fields */
    NVM_Memory 		hExtensionQHead;	/* Queue of Extension */

    /* The body text */
    NVM_Memory		hBody;
} Message;



/* TM Trace Flags */
#define	UASH_TRACE_ERROR   	(1 << 0)
#define	UASH_TRACE_WARNING   	(1 << 1)
#define	UASH_TRACE_DETAIL   	(1 << 2)
#define	UASH_TRACE_ACTIVITY	(1 << 3)
#define	UASH_TRACE_AGENT	(1 << 4)
#define	UASH_TRACE_INIT		(1 << 5)
#define	UASH_TRACE_VALIDATION	(1 << 6)
#define	UASH_TRACE_PREDICATE	(1 << 7)
#define	UASH_TRACE_ADDRESS	(1 << 8)
#define	UASH_TRACE_STATE  	(1 << 9)
#define	UASH_TRACE_PDU		(1 << 10)



/*
 * External Function Declarations
 */

ReturnCode
uash_messageReceived(char * pMessageId,
		     OS_Uint32 submissionTime,
		     OS_Uint32 deliveryTime,
		     UASH_MessageHeading * pHeading,
		     char * pBodyText);

void
uash_modifyDeliverStatus(char * pMessageId,
			 UASH_DeliverStatus messageStatus);

void
uash_submitSuccess(void * hMessage,
		   char * pMessageId,
		   void * hShellReference);

void
uash_submitFail(void * hMessage,
		OS_Boolean bTransient,
		void * hShellReference);

void
uash_deliveryReport(char * pMessageId,
		    OS_Uint32 deliveryTime);

void
uash_nonDeliveryReport(char * pMessageId,
		       UASH_NonDeliveryReason reason,
		       UASH_NonDeliveryDiagnostic diagnostic,
		       char * pExplanation);

ReturnCode
uash_messageToUser(NVM_Transaction hMessageTrans);

void
uash_deliveryControlSuccess(void * hOperationId,
    			    EMSD_DeliveryControlResult *pDeliveryControlResult,
		    	    void * hShellReference);

void
uash_deliveryControlError(void * hOperationId,
		          OS_Boolean bTransient,
		          void *hShellReference);

void
uash_deliveryControlFail(void * hOperationId,
		         OS_Boolean bTransient,
		         void *hShellReference);

