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
 * History: copy-and-edit based on uashpine.one/uashloc.h
 *
 */


#include "estd.h"
#include "queue.h"

/*
 * Module-global Types
 */

typedef struct UASH_LocalUser
{
    QU_ELEMENT;

    char *	    pEmsdAddr;
    OS_Uint32	    emsdAddr;

    char *	    pMailBox;
} UASH_LocalUser;


/*
 * Module-global Variables
 */
typedef struct UASH_Globals
{
    /* Trace module handle */
    void * 	    hModCB;

    /* Queue of local users */
    QU_Head 	    localUserQHead;

#if defined(OS_VARIANT_WinCE)
    /* lock file parameters */
    WCHAR	szLockFile[128];
    Int		grabLockMaxRetries;
    Int		grabLockRetryInterval;

    /* icon animation */
    OS_Boolean	fVisibleHeartbeat;
    ULONG	loopCount;
    OS_Boolean	trayIconStateIsFlip;
    HANDLE	trayIconFlip;
    HANDLE	trayIconFlop;
    HANDLE	trayIconInit;
    NOTIFYICONDATA   tnid;


#endif

#define	CHECKSUBMIT_WATCHDOG TRUE

#if CHECKSUBMIT_WATCHDOG
    /* checkSubmit Watchdog */
    OS_Boolean	csWatchdogInitiated;
    OS_Uint32	csWatchdogLoopCount;
    Int		csWatchdogIOUCount;
    OS_Boolean	csWatchdogSubmissionStarted;
#endif

    /* A temporary buffer to use throughout */
    char 	    tmpbuf[4096];
} UASH_Globals;

typedef enum
{
    TransType_InBox		= 1, /* not used in this portation */
    TransType_OutBox		= 2, /* not used in this portation */
    TransType_Verify		= 3,
    TransType_DupDetection	= 4
} TransType;

typedef struct UASH_Statistics
{
OS_Uint32  successfulSubmissions; 
OS_Uint32  successfulDeliveries;
OS_Uint32  submissionAttempts; 
OS_Uint32  deliveryAttempts;
OS_Uint32  submissionFailures; 
OS_Uint32  deliveryFailures;
OS_Uint32  deliveryReports;
OS_Uint32  nonDeliveryReports;
} UASH_Statistics;

/* TM Trace Flags */
#define	UASH_TRACE_ERROR   	(1 << 0)
#define	UASH_TRACE_WARNING   	(1 << 1)
#define	UASH_TRACE_DETAIL   	(1 << 2)
#define	UASH_TRACE_ACTIVITY	(1 << 3)
#define	UASH_TRACE_INIT		(1 << 4)
#define	UASH_TRACE_VALIDATION	(1 << 5)
#define	UASH_TRACE_PREDICATE	(1 << 6)
#define	UASH_TRACE_ADDRESS	(1 << 7)
#define	UASH_TRACE_STATE  	(1 << 8)
#define	UASH_TRACE_PDU		(1 << 9)



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
		void *hShellReference);

void
uash_deliveryReport(char * pMessageId,
		    OS_Uint32 deliveryTime);

void
uash_nonDeliveryReport(char * pMessageId,
		       UASH_NonDeliveryReason reason,
		       UASH_NonDeliveryDiagnostic diagnostic,
		       char * pExplanation);

ReturnCode
uash_deliverMessage(ENVELOPE * pRfcEnv,
		    char * pExtraHeaders,
		    char * pBody,
		    QU_Head * pRecipInfoQHead);

ReturnCode
UA_issueDeliveryControl(void ** phMessage,
		 	void *hShellReference);

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

