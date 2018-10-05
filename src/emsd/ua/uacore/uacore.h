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


#ifndef __UACORE_H__
#define	__UACORE_H__

#include "estd.h"
#include "queue.h"
#include "du.h"
#include "nvm.h"
#include "esro_cb.h"
#include "emsd.h"
#include "mtsua.h"
#include "ua.h"


/* EMSD Protocol Version supported by this UA implementation (1.1) */
#define	UA_PROTOCOL_VERSION_MAJOR		1
#define	UA_PROTOCOL_VERSION_MINOR		1

/* Maximum number of times to retry an INVOKE.req, receiving FAILURE.ind */
#define	UA_MAX_FAILURE_RETRIES			0

/* Time to wait for ALL segments of a segmented message to arrive */
#define	UA_SEGMENTATION_TIMER			60

/* Maximum number of message send retries that can be run-time configured */
#define	UA_MAX_RETRIES				50

/* Maximum number of Non-volatile Queue elements */
#define	UA_NVQ_MAX_NUM_ELEMENTS			5

/* Maximum size of one Non-volatile Queue element (max size of message id) */
#define	UA_NVQ_MAX_ELEMENT_SIZE			1024

/* TM Trace Flags */
#define	UA_TRACE_ERROR   	(1 << 0)
#define	UA_TRACE_WARNING   	(1 << 1)
#define	UA_TRACE_DETAIL   	(1 << 2)
#define	UA_TRACE_ACTIVITY	(1 << 3)
#define	UA_TRACE_INIT		(1 << 4)
#define	UA_TRACE_VALIDATION	(1 << 5)
#define	UA_TRACE_PREDICATE	(1 << 6)
#define	UA_TRACE_ADDRESS	(1 << 7)
#define	UA_TRACE_STATE  	(1 << 8)
#define	UA_TRACE_PDU		(1 << 9)


typedef struct VerifyData
{
    NVM_Transaction hActive;
    NVM_Transaction hInactive;
} VerifyData;

typedef struct VerifyElem
{
    NVM_QUELEMENT;

    char	    messageId[EMSDUB_MAX_MESSAGE_ID_LEN + 1];
} VerifyElem;

typedef struct DupData
{
    OS_Uint8	    nextInvokeInstanceId;
    OS_Uint8	    duplicate[32];		 /* 32 * 8 = 256 bits */
} DupData;

typedef struct Globals
{
    void *	    hTM;		/* Trace Module handle */
    NVM_Transaction hVerify;		/* Submission Verify data */
    NVM_Transaction hDupData;		/* Duplicate Detection data */

    N_SapAddr 	    mtsNsap;		/* MTS NSAP Address */
    char *	    pPassword;		/* User's password */

    struct
    {
	ESRO_SapDesc 	hInvoker3Way;
	ESRO_SapDesc 	hPerformer3Way;
	ESRO_SapDesc 	hInvoker2Way;
	ESRO_SapDesc 	hPerformer2Way;
    } 		    sap;

    void * 	    hPerformerTransDiag;
    void *	    hSubmitTransDiag;
    void *	    hDeliveryControlTransDiag;
    QU_Head	    activeOperations;

    OS_Boolean	    bDeliveryVerifySupported;

    OS_Uint8	    concurrentOperationsInbound;
    OS_Uint8	    concurrentOperationsOutbound;

    ReturnCode	    (* pfMessageReceived)(char * pMessageId,
					  OS_Uint32 submissionTime,
					  OS_Uint32 deliveryTime,
					  UASH_MessageHeading * pHeading,
					  char * pBodyText);

    void	    (* pfModifyDeliverStatus)(char * pMessageId,
					      UASH_DeliverStatus status);

    void	    (* pfSubmitSuccess)(void * hMessage,
					char * pMessageId,
					void * pHackData);

    void	    (* pfSubmitFail)(void * hMessage,
				     OS_Boolean bTransient,
				     void * pHackData);

    void	    (* pfDeliveryReport)(char * pMessageId,
					 OS_Uint32 deliveryTime);

    void	    (* pfNonDeliveryReport)(char * pMessageId,
					    UASH_NonDeliveryReason reason,
					    UASH_NonDeliveryDiagnostic diag,
					    char * pExplanation);
    
    void	    (* pfDeliveryControlSuccess)(void * hMessage,
    			                         EMSD_DeliveryControlResult *
					         pDeliveryControlResult,
		    	    		  	 void * hShellReference);

    void	    (* pfDeliveryControlError)(void * hMessage,
				              OS_Boolean bTransient,
		    	    		      void * hShellReference);

    void	    (* pfDeliveryControlFail)(void * hMessage,
				              OS_Boolean bTransient,
		    	    		      void * hShellReference);

    char	    tmpbuf[4096];
} UA_Globals;

extern UA_Globals	ua_globals;
extern DU_Pool *	G_duMainPool;


typedef struct UA_OperationFree
{
    QU_ELEMENT;

    void 	     (* pfFree)(void * hUserData);
    void *		hUserData;
} UA_OperationFree;


/*
 * Each operation (or possibly, set of operations) gets one of these.
 */
typedef struct UA_Operation
{
    QU_ELEMENT;

    /* Operation information */
    ESRO_OperationValue 	operationValue;
    ESRO_EncodingType 		encodingType;
    ESRO_InvokeId		invokeId;
    ESRO_InvokeId		unacknowledgedInvokeId;

    /* Finite state machine associated with this operation */
    void *			hFsm;

    /* SAP in use */
    ESRO_SapDesc 		hSapDesc;
    ESRO_SapSel			sapSel;
			    
    /* Keep track of errors during processing */
    ReturnCode 			rc;

    /* Segmentation and Reassembly fields */
    OS_Boolean 			bComplete;
    void * 			hOperationData;
    void * 			hFirstOperationData;
    void * 			hFullPduBuf;
    void * 			hSegmentBuf[32];
    OS_Uint32			expectedSegmentMask;
    OS_Uint32			receivedSegmentMask;
    OS_Uint32 			sequenceId;
    OS_Uint32 			lastSegmentNumber;
    OS_Uint32 			nextSegmentNumber;
    OS_Uint32 			segmentLength;
    void * 			hSegmentationTimer;
    QU_Head			pendingQHead;

    /* Retry fields */
    void *			hCurrentBuf;
    OS_Uint16			retryCount;

    /* Status of message being delivered */
    MTSUA_Status 		status;

    /* Message Id of delivered message */
    char			messageId[128];

    /* Operation-specific free function and parameter */
    QU_Head 			userFreeQHead;

    /* Pointer to pool from which the operation was allocated */
    OS_Uint8 *			pPool;
    void * 			hShellReference;
    char * 			pHackData[512];
} UA_Operation;


typedef struct
{
    QU_ELEMENT;

    /* Operation information */
    ESRO_OperationValue 	operationValue;
    ESRO_EncodingType 		encodingType;
    ESRO_InvokeId		invokeId;

    /* SAP in use */
    ESRO_SapDesc 		hSapDesc;
    ESRO_SapSel			sapSel;
			    
    /* Segmentation and Reassembly fields */
    void *			hOperationData;
} UA_Pending;


enum
{
    UA_FsmEvent_InvokeIndication,
    UA_FsmEvent_ResultIndication,
    UA_FsmEvent_ResultConfirm,
    UA_FsmEvent_ErrorIndication,
    UA_FsmEvent_ErrorConfirm,
    UA_FsmEvent_FailureIndication,
    UA_FsmEvent_MessageSubmitRequest,
    UA_FsmEvent_SegmentationTimerExpired,
    UA_FsmEvent_DeliveryControlRequest
} UA_FsmEvent;


enum
{
    ModId_Ua					= MODID_GETID(1)
};


enum UA_ReturnCode
{
    UA_RC_UnrecognizedProtocolVersion		= (1 | ModId_Ua),
    UA_RC_MaxRetries				= (2 | ModId_Ua),
    UA_RC_SecurityViolation			= (3 | ModId_Ua)
};


/*
 * External definitions of module-local functions
 */

ReturnCode
ua_submitFsmInit(void);

int
ua_invokeIndication(ESRO_SapDesc hSapDesc,
		    ESRO_SapSel sapSel, 
		    T_SapSel * pRemoteTsap,
		    N_SapAddr * pRemoteNsap,
		    ESRO_InvokeId invokeId, 
		    ESRO_OperationValue operationValue,
		    ESRO_EncodingType encodingType, 
		    DU_View hView);

int
ua_resultIndication(ESRO_InvokeId invokeId, 
		    ESRO_UserInvokeRef userInvokeRef,
		    ESRO_EncodingType encodingType, 
		    DU_View hView);

int
ua_errorIndication(ESRO_InvokeId invokeId, 
		   ESRO_UserInvokeRef userInvokeRef,
		   ESRO_EncodingType encodingType,
		   ESRO_ErrorValue errorValue,
		   DU_View hView);

int
ua_resultConfirm(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef);

int
ua_errorConfirm(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef);

int
ua_failureIndication(ESRO_InvokeId invokeId,
		     ESRO_UserInvokeRef userInvokeRef,
		     ESRO_FailureValue failureValue);

ReturnCode
ua_initModuleManagement(char * pErrorBuf);

ReturnCode
ua_performerFsmInit(void);

ReturnCode
ua_deliveryControlFsmInit(void);

ReturnCode
ua_preprocessInvokeIndication(void * hBuf,
			      ESRO_OperationValue opValue,
			      ESRO_EncodingType encodingType,
			      ESRO_InvokeId invokeId,
			      ESRO_SapDesc hSapDesc,
			      ESRO_SapSel sapSel, 
			      UA_Operation ** ppOperation);

ReturnCode
ua_preprocessResultIndication(UA_Operation * pOperation,
			      void * hBuf);

ReturnCode
ua_preprocessErrorIndication(UA_Operation * pOperation,
			     void * hBuf);

ReturnCode
ua_preprocessSubmitRequest(UA_Operation * pOperation,
			   UASH_MessageHeading * pHeading,
			   char * pBodyText);

ReturnCode
ua_preprocessDeliveryControlRequest(UA_Operation * pOperation);

ReturnCode
ua_issueInvoke(UA_Operation * pOperation,
	       void * hInvokeData);

ReturnCode
ua_issueDelCtrlInvoke(UA_Operation * pOperation,
	       	      void * hInvokeData);

void
ua_issueErrorRequest(ESRO_InvokeId invokeId,
		     EMSD_Error errorVal);

ReturnCode
ua_allocOperation(UA_Operation ** ppOperation,
		  OS_Uint8 * pPool,
		  void *hShellReference);

void
ua_freeOperation(UA_Operation * pOperation);

ReturnCode
ua_operationAddUserFree(UA_Operation * pOperation,
			void (* pfFree)(void * hUserData),
			void * hUserData);

void
ua_operationDelUserFree(UA_Operation * pOperation,
			void * hUserData);

void
ua_messageIdEmsdLocalToRfc822(EMSD_LocalMessageId * pLocalMessageId,
			     char * pBuf);

char *
ua_messageIdEmsdToRfc822(EMSD_MessageId * pEmsdMessageId,
			char * pBuf);

ReturnCode
ua_messageIdRfc822ToEmsdLocal(char * pRfc822MessageId,
			     EMSD_MessageId * pEmsdMessageId);

ReturnCode
ua_messageToShell(EMSD_DeliverArgument * pDeliver,
		  EMSD_Message * pIpm,
		  UA_Operation * pOperation);

ReturnCode
ua_messageFromShell(UASH_MessageHeading * pLocalHeading,
		    char * pBodyText,
		    EMSD_Message ** ppIpm);

OS_Boolean
ua_isDuplicate(DU_View hView,
	       ESRO_OperationValue operationValue);

void
ua_addDuplicateDetection(DU_View hView,
			 ESRO_OperationValue operationValue);

#endif /* __UACORE_H__ */
