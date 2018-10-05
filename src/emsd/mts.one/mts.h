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


#ifndef __MTS_H__
#define	__MTS_H__

#include "estd.h"
#include "queue.h"
#include "du.h"
#include "esro_cb.h"
#include "emsd.h"
#include "mtsua.h"
#include "subpro.h"
#include "msgstore.h"

/* Maximum number of times to retry an INVOKE.req, receiving FAILURE.ind */
#define	MTS_MAX_FAILURE_RETRIES			2

/* Time to wait for ALL segments of a segmented message to arrive */
#define	MTS_SEGMENTATION_TIMER			120

/* Maximum number of message send retries that can be run-time configured */
#define	MTS_MAX_RETRIES				50

/* TM Trace Flags */
#define	MTS_TRACE_ERROR   	(1 << 0)
#define	MTS_TRACE_WARNING   	(1 << 1)
#define	MTS_TRACE_DETAIL   	(1 << 2)
#define	MTS_TRACE_ACTIVITY	(1 << 3)
#define	MTS_TRACE_INIT		(1 << 4)
#define	MTS_TRACE_VALIDATION	(1 << 5)
#define	MTS_TRACE_PREDICATE	(1 << 6)
#define	MTS_TRACE_ADDRESS	(1 << 7)
#define	MTS_TRACE_STATE  	(1 << 8)
#define	MTS_TRACE_PDU		(1 << 9)
#define	MTS_TRACE_PROFILE	(1 << 10)
#define	MTS_TRACE_SCHED		(1 << 11)
#define	MTS_TRACE_POLL		(1 << 12)
#define	MTS_TRACE_TOOMUCH	(1 << 13)
#define	MTS_TRACE_MSG		(1 << 15)

#ifndef NULLOK
#define NULLOK(x)		((x) ? (x) : "NULL")
#endif

typedef struct Globals
{
    void *	    hMMEntity;		/* Module Management entity handle */
    void *	    hMMModule;		/* Module Management module handle */
    void *	    hMMScreenAlert;	/* Module Management alert handle */
    void *	    hTM;		/* Trace Module handle */
    void *	    hConfig;		/* Configuration handle */
    void *	    hProfile;		/* O/R Profile handle */
    void *	    hMMLog;		/* Managable object: Log */

    struct
    {
	ESRO_SapDesc 	hInvoker3Way;
	ESRO_SapDesc 	hPerformer3Way;
	ESRO_SapDesc 	hInvoker2Way;
	ESRO_SapDesc 	hPerformer2Way;
    } 		    sap;

    struct			/* data read from configuration */
    {
	struct
	{
	    char * 	    pDataDir;
	    char * 	    pSpoolDir;
	    char * 	    pNewMessageDir;
	    void * 	    hNewMessageDir;
	} 		deliver;
	struct
	{
	    char * 	    pDataDir;
	    char * 	    pSpoolDir;
	} 		submit;
	char * 		pLocalHostName;
	char * 		pLocalNsapAddress;
	char * 		pSubscriberProfileFile;
	char * 		pSendmailPath;
	char * 		pSendmailSpecifyFrom;
	QU_Head 	emsdHosts;
    } 		    config;

    /* Daily Message Number fields */
    OS_Uint32	    nextMessageNumber;
    OS_ZuluDateTime dateLastAssignedMessageNumber;

    void * 	    hPerformerTransDiag;
    void *	    hDeliverTransDiag;
    QU_Head	    activeOperations;

    /*
     * Retry times.  Each number represents the number of seconds to
     * wait for the next retry.  I.e., the first value in the array
     * represents the number of seconds to wait before the first retry.
     * If that fails, then the next number in the array represents the
     * number of seconds to wait before retrying again, etc.  When all
     * retry times have been used up, a non-delivery report will be sent
     * to the message originator/sender to indicate that the message
     * could not be delivered.
     */
    OS_Uint32 	    retryTimes[MTS_MAX_RETRIES];
    OS_Uint16 	    numRetryTimes;

    char	    tmpbuf[4096];
} MTS_Globals;

extern MTS_Globals	globals;
extern DU_Pool *	G_duMainPool;


#ifdef OLD_PROFILE
typedef struct MTS_DeviceProfile
{
    void *		hProfile;
    OS_Uint8		protocolVersionMajor;
    OS_Uint8		protocolVersionMinor;
    char *		pNsap;
    N_SapAddr		nsap;
    char *		pEmsdAua;
    char *		pEmsdUaPassword;
    char *		pDeviceType;
    char *		pDeliverMode;
    char *		pSerialMode;
} MTS_DeviceProfile;
#endif

typedef struct MTS_Host
{
    QU_ELEMENT;

    char	    pHostName[1];
} MTS_Host;



typedef struct MTS_OperationFree
{
    QU_ELEMENT;

    void 	     (* pfFree)(void * hUserData);
    void *		hUserData;
} MTS_OperationFree;


/*
 * Each operation (or possibly, set of operations) gets one of these.
 */
typedef struct MTS_Operation
{
    QU_ELEMENT;

    /* Unique operation identifier */
    OS_Uint32		        operId;

    /* message ID */
    EMSD_LocalMessageId          localMessageId;

    N_SapAddr                   nsap;

    struct DQ_ControlData *     pControlData;

    /* Operation information */
    ESRO_OperationValue 	operationValue;
    ESRO_EncodingType 		encodingType;
    ESRO_InvokeId		invokeId;

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

    /* Retry fields */
    void *			hCurrentBuf;
    OS_Uint16			retryCount;

    /* Spool maintenance */
    char * 			pControlFileName;
    MTSUA_ControlFileData *	pControlFileData;

    /* Status of message being delivered */
    MTSUA_Status 		status;

    /* Operation-specific free function and parameter */
    QU_Head 			userFreeQHead;
} MTS_Operation;


enum
{
    MTS_FsmEvent_InvokeIndication,
    MTS_FsmEvent_ResultIndication,
    MTS_FsmEvent_ResultConfirm,
    MTS_FsmEvent_ErrorIndication,
    MTS_FsmEvent_ErrorConfirm,
    MTS_FsmEvent_FailureIndication,
    MTS_FsmEvent_MessageDeliverRequest,
    MTS_FsmEvent_SegmentationTimerExpired,
} MTS_FsmEvent;


enum
{
    ModId_Mts					= MODID_GETID(1)
};

enum MTS_ReturnCode
{
    MTS_RC_DeviceProfileNotFound		= (1 | ModId_Mts),
    MTS_RC_MissingProfileAttribute		= (2 | ModId_Mts),
    MTS_RC_IllegalProfileAttribute		= (3 | ModId_Mts),
    MTS_RC_UnrecognizedProtocolVersion		= (4 | ModId_Mts),
    MTS_RC_MaxRetries				= (5 | ModId_Mts),
};


/*
 * External definitions of module-local functions
 */

ReturnCode
mts_deliverFsmInit(void);

#ifdef OLD_PROFILE
ReturnCode
mts_initDeviceProfile(char * pSubscriberProfileFile);

ReturnCode
mts_getDeviceProfileByNsap(N_SapAddr * pRemoteNsap,
			   MTS_DeviceProfile ** ppProfile);

ReturnCode
mts_getDeviceProfileByAua(char * pAua,
			  MTS_DeviceProfile ** ppProfile);

#endif  /* OLD_PROFILE */

int
mts_invokeIndication(ESRO_SapDesc hSapDesc,
		     ESRO_SapSel sapSel, 
		     T_SapSel * pRemoteTsap,
		     N_SapAddr * pRemoteNsap,
		     ESRO_InvokeId invokeId, 
		     ESRO_OperationValue operationValue,
		     ESRO_EncodingType encodingType, 
		     DU_View hView);

int
mts_resultIndication(ESRO_InvokeId invokeId, 
		     ESRO_UserInvokeRef userInvokeRef,
		     ESRO_EncodingType encodingType, 
		     DU_View hView);

int
mts_errorIndication(ESRO_InvokeId invokeId, 
		    ESRO_UserInvokeRef userInvokeRef,
		    ESRO_EncodingType encodingType,
		    ESRO_ErrorValue errorValue,
		    DU_View hView);

int
mts_resultConfirm(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef);

int
mts_errorConfirm(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef);

int
mts_failureIndication(ESRO_InvokeId invokeId,
		      ESRO_UserInvokeRef userInvokeRef,
		      ESRO_FailureValue failureValue);

ReturnCode
mts_initModuleManagement(char * pErrorBuf);

void
mts_deliveryNonDel(struct DQ_ControlData * pMsg,
		   OS_Uint32 error);

void
mts_submissionNonDel(MTS_Operation * pOperation,
		     OS_Uint32 error);

void
mts_rfc822SubmitNonDel(MTSUA_ControlFileData * pControlFileData,
		       ReturnCode rc);

ReturnCode
mts_performerFsmInit(void);

ReturnCode
mts_preprocessInvokeIndication(SUB_DeviceProfile * pProfile,
			       void * hBuf,
			       ESRO_OperationValue opValue,
			       ESRO_EncodingType encodingType,
			       ESRO_InvokeId invokeId,
			       ESRO_SapDesc hSapDesc,
			       ESRO_SapSel sapSel, 
			       MTS_Operation ** ppOperation);

ReturnCode
mts_preprocessResultIndication(MTS_Operation * pOperation,
			       void * hBuf);

ReturnCode
mts_preprocessErrorIndication(MTS_Operation * pOperation,
			      void * hBuf);

ReturnCode
mts_preprocessDeliverRequest(MTS_Operation * pOperation);

ReturnCode
mts_issueInvoke(MTS_Operation * pOperation,
		void * hInvokeData);

void
mts_issueErrorRequest(ESRO_InvokeId invokeId,
		      EMSD_Error     errorVal,
		      OS_Uint8      errorParam,
		      OS_Uint8      protocolVersionMajor,
		      OS_Uint8      protocolVersionMinor);

ReturnCode
mts_buildTimerQueue(char * pDirName,
		    ReturnCode (* pfTryMessageSend)(void * hTimer,
						    void * hUserData1,
						    void * hUserData2));

ReturnCode
mts_spoolToDevice(void * hTimer,
		  void * hUserData1,
		  void * hUserData2);

ReturnCode
mts_tryRfc822Submit(void * hTimer,
		    void * hUserData1,
		    void * hUserData2);

ReturnCode
mts_allocOperation(MTS_Operation ** ppOperation);

void
mts_freeOperation(MTS_Operation * pOperation);

ReturnCode
mts_operationAddUserFree(MTS_Operation * pOperation,
			 void (* pfFree)(void * hUserData),
			 void * hUserData);

void
mts_operationDelUserFree(MTS_Operation * pOperation,
			 void * hUserData);

ReturnCode
mts_tryDelivery(void * hTimer,
		void * hUserData1,
		void * hUserData2);

ReturnCode
mts_sendMessage(MTSUA_ControlFileData * pControlFileData,
		FILE * hDataFile);

ReturnCode
mts_convertRfc822ToEmsdContent(ENVELOPE * pRfc822Env,
			      char * pRfc822Body,
			      char *pHeaderStr,
			      void ** ppContent,
			      EMSD_ContentType * pContentType);

ReturnCode
mts_convertRfc822ToEmsdSubmit(ENVELOPE * pRfc822Env,
			     char * pRfc822Body,
			     EMSD_SubmitArgument * pSubmit);

ReturnCode
mts_convertRfc822ToEmsdDeliver(ENVELOPE * pRfc822Env,
			      char * pRfc822Body,
			      EMSD_DeliverArgument * pDeliver,
			      EMSD_ContentType contentType);



#endif /* __MTS_H__ */
