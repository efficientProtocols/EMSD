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
 * Functions:
 *    UA_init()
 *    UA_setPassword()
 *    UA_setMtsNsap()
 *    UA_submitMessage()
 *    UA_invokeDeliveryControl()
 */


#include "estd.h"
#include "tmr.h"
#include "config.h"
#include "tm.h"
#include "eh.h"
#include "du.h"
#include "mm.h"
#include "nvm.h"
#include "sch.h"
#include "fsm.h"
#include "fsmtrans.h"
#include "nnp.h"
#include "esro_cb.h"
#include "emsd.h"
#include "mtsua.h"
#include "ua.h"
#include "uacore.h"
#include "target.h"

/*
 * Global Variables
 *
 * All global variables for the UA go inside of UA_Globals unless
 * there's a really good reason for putting them elsewhere (like
 * having external requirements on the name).
 */
UA_Globals		ua_globals;



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
								)
{
    int				i;
    ReturnCode			rc;
    ESRO_RetVal			retval;
    VerifyData *		pVerifyData;
    DupData *			pDupData;
    NVM_TransInProcess		hTip;
    NVM_Memory			hElem;

    /* Initialize the global variables */
    ua_globals.pfMessageReceived        = pfMessageReceived;
    ua_globals.pfModifyDeliverStatus    = pfModifyDeliverStatus;
    ua_globals.pfSubmitSuccess          = pfSubmitSuccess;
    ua_globals.pfSubmitFail             = pfSubmitFail;
    ua_globals.pfDeliveryReport         = pfDeliveryReport;
    ua_globals.pfNonDeliveryReport      = pfNonDeliveryReport;
#ifdef DELIVERY_CONTROL
    ua_globals.pfDeliveryControlSuccess = pfDeliveryControlSuccess;
    ua_globals.pfDeliveryControlError   = pfDeliveryControlError;
    ua_globals.pfDeliveryControlFail    = pfDeliveryControlFail;
#endif

    UA_setMtsNsap(pMtsNsap);       /* ua_globals.mtsNsap = *pMtsNsap; */

    if (concurrentOperationsInbound > 127 ||
	concurrentOperationsOutbound > 127)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: "
		"Maximum number of concurrent operations is 127\n",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }

    /* Save the numbers of concurrent operations */
    ua_globals.concurrentOperationsInbound =
	(OS_Uint8) concurrentOperationsInbound;

    ua_globals.concurrentOperationsOutbound =
	(OS_Uint8) concurrentOperationsOutbound;

    /* Save the user's password */
    if (pPassword == NULL)
    {
	ua_globals.pPassword = NULL;
    }
    else
    {
	if ((ua_globals.pPassword = OS_allocStringCopy(pPassword)) == NULL)
	{
	    sprintf(ua_globals.tmpbuf,
		    "%s: Could not allocate memory for password (%s)",
		    __applicationName,
		    pPassword);
	    EH_fatal(ua_globals.tmpbuf);
	}
    }

    ua_globals.hPerformerTransDiag = NULL;
    ua_globals.hSubmitTransDiag = NULL;
    QU_INIT(&ua_globals.activeOperations);
      
    /* Give ourselves a trace handle */
    if (TM_OPEN(ua_globals.hTM, "UA") == NULL)
    {
	EH_fatal("Could not open UA trace");
    }
      
    /* See if there's already a non-volatile queue for submission verify ops */
    rc = NVM_nextTrans(nvmVerifyTransType, NVM_FIRST, &ua_globals.hVerify);
    if (rc == Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, 
		 "Got existing non-volatile queue for submit verify"));
    }
    else
    {
	/*
	 * There's not.  Create one.
	 */
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, 
		 "Creating non-volatile queue for submit verify"));

	/* Begin a non-volatile memory transaction */
	if ((rc = NVM_beginTrans(nvmVerifyTransType,
				 &hTip, &ua_globals.hVerify)) != Success)
	{
	    EH_fatal("Could not create non-volatile queue (beginTrans)");
	}

	/* Allocate a Submission Verify structure */
	if ((rc = NVM_alloc(hTip, sizeof(VerifyData), &hElem)) != Success)
	{
	    EH_fatal("Could not create non-volatile queue (alloc verify)");
	}
	

	/* Point to the Submission Verify structure */
	pVerifyData = NVM_getMemPointer(ua_globals.hVerify, hElem);

	/* Allocate a queue head for active verify elements. */
	if ((rc = NVM_alloc(hTip, sizeof(NVM_QuElement),
			    &pVerifyData->hActive)) != Success)
	{
	    EH_fatal("Could not create non-volatile queue (alloc active)");
	}

	/* Initialize the queue head */
	NVM_quInit(ua_globals.hVerify, pVerifyData->hActive);

	/* Allocate a queue head for inactive verify elements. */
	if ((rc = NVM_alloc(hTip, sizeof(NVM_QuElement),
			    &pVerifyData->hInactive)) != Success)
	{
	    EH_fatal("Could not create non-volatile queue (alloc inactive)");
	}

	/* Initialize the queue head */
	NVM_quInit(ua_globals.hVerify, pVerifyData->hInactive);
	
	/* Pre-allocate five data elements and put 'em on the inactive queue */
	for (i = 0; i < 5; i++)
	{
	    if ((rc = NVM_alloc(hTip, sizeof(VerifyElem), &hElem)) != Success)
	    {
		EH_fatal("Could not create non-volatile queue (alloc elem)");
	    }

	    /* Initialize the queue element */
	    NVM_quInit(ua_globals.hVerify, hElem);

	    /* Insert the queue element onto the inactive queue */
	    NVM_quInsert(ua_globals.hVerify, hElem,
			 ua_globals.hVerify, pVerifyData->hInactive);
	}

	/* End the non-volatile memory transaction */
	if ((rc = NVM_endTrans(hTip)) != Success)
	{
	    EH_fatal("Could not create non-volatile queue (endTrans)");
	}
    }

    /* See if there's already a non-volatile queue for duplication detection */
    rc = NVM_nextTrans(nvmDupTransType, NVM_FIRST, &ua_globals.hDupData);
    if (rc == Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, 
		 "Got existing non-volatile queue for duplicate detection"));
    }
    else
    {
	/*
	 * There's not.  Create one.
	 */
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, 
		 "Creating non-volatile queue for duplicate detection"));

	/* Begin a non-volatile memory transaction */
	if ((rc = NVM_beginTrans(nvmDupTransType,
				 &hTip, &ua_globals.hDupData)) != Success)
	{
	    EH_fatal("Could not create non-volatile dup data (beginTrans)");
	}

	/* Allocate a Duplication Data structure */
	if ((rc = NVM_alloc(hTip, sizeof(DupData), &hElem)) != Success)
	{
	    EH_fatal("Could not create non-volatile dup data (alloc verify)");
	}
	
	/* Point to the Duplication Detection data structure */
	pDupData = NVM_getMemPointer(ua_globals.hDupData, hElem);

	/* Start off with an invoke instance id of zero */
	pDupData->nextInvokeInstanceId = 0;

	/* Initialize all duplicate flags to "free" */
	OS_memSet(pDupData->duplicate, '\0', sizeof(pDupData->duplicate));

	/* End the non-volatile memory transaction */
	if ((rc = NVM_endTrans(hTip)) != Success)
	{
	    EH_fatal("Could not create non-volatile dup data (endTrans)");
	}
    }

    /* Build the ASN compile trees for all of the EMSD protocols */
    if ((rc = UA_initCompileTrees(ua_globals.tmpbuf)) != Success)
    {
	EH_fatal(ua_globals.tmpbuf);
    }
      
    /* Activate our 3-way Invoker service access point */
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "SAP Bind: Invoker, 3-way"));
    if ((retval = ESRO_CB_sapBind(&ua_globals.sap.hInvoker3Way,
				  MTSUA_Rsap_Ua_Invoker_3Way,
				  ESRO_3Way,
				  ua_invokeIndication, 
				  ua_resultIndication, 
				  ua_errorIndication,
				  ua_resultConfirm, 
				  ua_errorConfirm,
				  ua_failureIndication)) < 0)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: Could not activate ESROS Service Access Point.",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }
      
    /* Activate our 3-way Performer service access point */
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "SAP Bind: Performer, 3-way"));
    if ((retval = ESRO_CB_sapBind(&ua_globals.sap.hPerformer3Way,
				  MTSUA_Rsap_Ua_Performer_3Way,
				  ESRO_3Way,
				  ua_invokeIndication, 
				  ua_resultIndication, 
				  ua_errorIndication,
				  ua_resultConfirm, 
				  ua_errorConfirm,
				  ua_failureIndication)) < 0)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: Could not activate ESROS Service Access Point.",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }
      
    /* Activate our 2-way Invoker service access point */
    if ((retval = ESRO_CB_sapBind(&ua_globals.sap.hInvoker2Way,
				  MTSUA_Rsap_Ua_Invoker_2Way,
				  ESRO_2Way,
				  ua_invokeIndication, 
				  ua_resultIndication, 
				  ua_errorIndication,
				  ua_resultConfirm, 
				  ua_errorConfirm,
				  ua_failureIndication)) < 0)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: Could not activate ESROS Service Access Point.",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }
    
    /* Activate our 2-way Performer service access point */
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "SAP Bind: Performer, 2-way"));
    if ((retval = ESRO_CB_sapBind(&ua_globals.sap.hPerformer2Way,
				  MTSUA_Rsap_Ua_Performer_2Way,
				  ESRO_2Way,
				  ua_invokeIndication, 
				  ua_resultIndication, 
				  ua_errorIndication,
				  ua_resultConfirm, 
				  ua_errorConfirm,
				  ua_failureIndication)) < 0)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: Could not activate ESROS Service Access Point.",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }
    
    /* Initialize the FSM module */
    if (FSM_init() != SUCCESS)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: Could not initialize finite state machine module\n",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }
      
    FSM_TRANSDIAG_init();
      
    /* Initialize the Performer finite state machines */
    if (ua_performerFsmInit() != Success)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: Could not initialize Performer state machines\n",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }

    /* Initialize the Submission finite state machines */
    if (ua_submitFsmInit() != Success)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: Could not initialize Submission state machines\n",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }

#ifdef DELIVERY_CONTROL
    /* Initialize the Delivery Control finite state machines */
    if (ua_deliveryControlFsmInit() != Success)
    {
	sprintf(ua_globals.tmpbuf,
		"%s: Could not initialize Delivery Control state machines\n",
		__applicationName);
	EH_fatal(ua_globals.tmpbuf);
    }
#endif

    return Success;
}




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
UA_setPassword(char * pPassword)
{
    char *	    pOldPassword;

    /* If there's already a password specified, save it. */
    pOldPassword = ua_globals.pPassword;

    /* Save the user's new password */
    if (pPassword == NULL)
    {
	ua_globals.pPassword = NULL;
    }
    else
    {
	/* Allocate space to save the new password */
	if ((ua_globals.pPassword = OS_allocStringCopy(pPassword)) == NULL)
	{
	    ua_globals.pPassword = pOldPassword;
	    return ResourceError;
	}
    }

    /* Free the old password, if there was one. */
    if (pOldPassword != NULL)
    {
	OS_free(pOldPassword);
    }

    return Success;
}




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
UA_setMtsNsap(N_SapAddr * pMtsNsap)
{
    ua_globals.mtsNsap = *pMtsNsap;
    ua_globals.mtsNsap.len = 4;

    return Success;
}




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
		 void *hShellReference)
{
    ReturnCode			rc;
    UA_Operation *		pOperation;
    
    /* If we're not yet configured, they can't submit. */
    if (ua_globals.mtsNsap.len == 0)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, 
		 "Attempt to submit message with no MTS NSAP configured."));
	return UA_RC_SecurityViolation;
    }

    /* Allocate an operation structure */
    if ((rc = ua_allocOperation(&pOperation,
				&ua_globals.concurrentOperationsOutbound,
				hShellReference)) !=
	Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, 
		 "UA_submitMessage: ua_allocOperation failed\n"));
	EH_problem("UA_submitMessage: ua_allocOperation failed\n");
	return rc;
    }
	    
    /* Preprocess the submission */
    if ((rc = ua_preprocessSubmitRequest(pOperation,
					 pHeading, pBodyText)) != Success)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, 
		 "UA_submitMessage: ua_preprocessSubmitRequest failed\n"));
	ua_freeOperation(pOperation);
	return rc;
    }
	    
    /* Generate an event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_MessageSubmitRequest);
	    
    /* Give 'em what they came for */
    *phMessage = pOperation;

    return Success;
}



#ifdef DELIVERY_CONTROL
ReturnCode
UA_issueDeliveryControl(void ** phMessage,
		 	void *hShellReference)
{
    ReturnCode			rc;
    UA_Operation *		pOperation;
    
    /* If we're not yet configured, they can't submit. */
    if (ua_globals.mtsNsap.len == 0)
    {
        TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, 
		 "Attempt to submit message with no MTS NSAP configured."));
	return UA_RC_SecurityViolation;
    }

    /* Allocate an operation structure */
    if ((rc = ua_allocOperation(&pOperation,
				&ua_globals.concurrentOperationsOutbound,
				hShellReference)) !=
	Success)
    {
	return rc;
    }
	    
    /* Preprocess the Delivery Control */
    if ((rc = ua_preprocessDeliveryControlRequest(pOperation)) != Success)
    {
	ua_freeOperation(pOperation);
	return rc;
    }
	    
    /* Generate an event to the finite state machine */
    FSM_generateEvent(pOperation->hFsm, UA_FsmEvent_DeliveryControlRequest);
	    
    /* Give 'em what they came for */
    *phMessage = pOperation;

    return Success;
}
#endif
