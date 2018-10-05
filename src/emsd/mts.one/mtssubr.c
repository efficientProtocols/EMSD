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

static char *rcs = "$Id: mtssubr.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";


#include "estd.h"
#include "eh.h"
#include "queue.h"
#include "mm.h"
#include "du.h"
#include "buf.h"
#include "tm.h"
#include "tmr.h"
#include "fsm.h"
#include "mtsua.h"
#include "mts.h"
#include "dup.h"
#include "subpro.h"
#include "msgstore.h"


ReturnCode
mts_issueInvoke(MTS_Operation * pOperation, void * hInvokeData)
{
    char *		    p;
    char                    msg[256];
    unsigned char * 	    pMem;
    OS_Uint16 		    len;
    ReturnCode 		    rc              = Success;
    ESRO_RetVal		    retval;
    DU_View		    hView;
    N_SapAddr *             pNsap;
    static T_SapSel	    tsap            = { 2, { 7, 210 } };
    

    if (pOperation->pControlData) {
      TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Invoke using Delivery NSAP structure"));
      pNsap = &pOperation->pControlData->pRecipQueue->deviceProfile.nsap;
    } else {
      TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Invoke using Submission NSAP structure"));
      pNsap = &pOperation->nsap;
    }

    sprintf (msg, "MTS issuing invoke: SapDesc=%d, SapSel=%d, NSAP[%d]=%d.%d.%d.%d, operId=%ld, OpVal=%d",
	     pOperation->hSapDesc, pOperation->sapSel, pNsap->len,
	     pNsap->addr[0], pNsap->addr[1], pNsap->addr[2], pNsap->addr[3], 
	     pOperation->operId, EMSD_OP_VALUE(pOperation->operationValue));

    TM_CALL(globals.hTM, MTS_TRACE_PDU, BUF_dump, hInvokeData, msg);

    /* Allocate a DU buffer for our buffer contents */
    hView = DU_alloc(G_duMainPool, BUF_getBufferLength(hInvokeData));
    if (hView == NULL)
    {
	pOperation->status = MTSUA_Status_TransientError;
	return ResourceError;
    }

    pMem = DU_data(hView);
    BUF_resetParse(hInvokeData);

    /* For each buffer segment... */
    for ( ; ; )
    {
	/* ... get the segment data, ... */
	len = 0;
	if ((rc = BUF_getChunk(hInvokeData, &len, (unsigned char **) &p)) != Success)
	{
	    if (rc == BUF_RC_BufferExhausted)
	    {
		break;		/* Normal loop exit; no more segments. */
	    }
	    DU_free(hView);
	    return rc;
	}
	/* ... and copy it into our contiguous memory. */
	TM_TRACE((globals.hTM, MTS_TRACE_TOOMUCH, "Copying %d bytes to DU_data()", len));
	OS_copy(pMem, (unsigned char *) p, len);
	pMem += len;
    }
    
    DUP_invokeOut(hView, EMSD_OP_VALUE(pOperation->operationValue), pNsap); /*** fnh ***/

    retval = ESRO_CB_invokeReq(&pOperation->invokeId,
			       NULL,
			       pOperation->hSapDesc,
			       pOperation->sapSel,
			       &tsap,
			       pNsap,
			       EMSD_OP_VALUE(pOperation->operationValue),
			       pOperation->encodingType,
			       hView);
    if (retval != SUCCESS)
    {
	/*
	 * The invoke failed.  We'll call this a transient
	 * error for now.
	 */
	pOperation->status = MTSUA_Status_TransientError;
	DU_free(hView);
	TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, "Invoke failed, transient error"));
	return Fail;
    }

    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "InvokeId=%d, NSAP=%d.%d.%d.%d  {%ld}",
	      pOperation->invokeId,
	      pNsap->addr[0], pNsap->addr[1], pNsap->addr[2], pNsap->addr[3],
	      pOperation->operId));
#ifdef BUG    
    /* Insert the operation onto the queue of active operations */
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
	     "Inserting activeOperation %08lx (%d) {%ld}",
	      pOperation, pOperation->invokeId, pOperation->operId));
    QU_INSERT(pOperation, &globals.activeOperations);
#endif

    /* We're done with our view of this data */
    DU_free(hView);

    return Success;
}


void
mts_issueErrorRequest(ESRO_InvokeId invokeId,
		      EMSD_Error errorVal,
		      OS_Uint8  errorParam,
		      OS_Uint8  protocolVersionMajor,
		      OS_Uint8  protocolVersionMinor)
{
    void *		hBuf          = NULL;
    unsigned char *	p             = NULL;
    OS_Uint16		n             = 0;
    OS_Uint32		len           = 0;
    ReturnCode		rc            = Success;
    EMSD_ErrorRequest 	errorReq;
    ESRO_RetVal	        retval        = 0;
    QU_Head *		pCompTree     = NULL;
    DU_View		hView         = NULL;
          
    TM_TRACE((globals.hTM, MTS_TRACE_MSG, "Issuing error request %d", errorVal));

    /* Generate an error request */
    
    if (errorVal == EMSD_Error_SecurityError || getenv("MTA_FORCE_ERROR_PARM")) {
      errorReq.error = errorParam;
      /*
       * Allocate a buffer to begin formatting the Error
       * Indication.
       */
      if ((rc = BUF_alloc(0, &hBuf)) != Success)
	{
	  goto cleanUp;
	}
      /* Determine which protocol tree to use. */
      pCompTree = mtsua_getProtocolTree(MTSUA_Protocol_ErrorRequest,
					protocolVersionMajor, protocolVersionMinor);
      /* Format an Error Indication */
      rc = ASN_format(ASN_EncodingRules_Basic, pCompTree, hBuf, &errorReq, &len);
      if (rc != Success)
	{
	  goto cleanUp;
	}
      /* Allocate a DU buffer for our buffer contents */
      if ((hView = DU_alloc(G_duMainPool, BUF_getBufferLength(hBuf))) == NULL)
	{
	  goto cleanUp;
	}
      /* Get a pointer to the formatted data */
      BUF_resetParse(hBuf);
      if ((rc = BUF_getChunk(hBuf, &n, &p)) != Success)
	{
	  goto cleanUp;
	}
      /* Copy the data to the DU_View */
      OS_copy(DU_data(hView), (unsigned char *) p, n);
    }

    /* invoke the ERROR.ind */
    retval = ESRO_CB_errorReq(invokeId, NULL, ESRO_EncodingBER, errorVal, hView);
    if (retval!= Success)
    {
	goto cleanUp;
    }
    
  cleanUp:
    /* We're done with our view of the data now. */
    if (hView != NULL)
    {
	DU_free(hView);
    }

    /* We're all done with the buffer now */
    if (hBuf != NULL)
    {
	BUF_free(hBuf);
    }
}


ReturnCode
mts_buildTimerQueue(char * pDirName,
		    ReturnCode (* pfTryMessageSend)(void * hTimer,
						    void * hUserData1,
						    void * hUserData2))
{
    char * 			pFileName;
    OS_Uint32 			timeNow;
    OS_Uint32 			timeToNextRetry;
    ReturnCode 			rc;
    OS_Dir 	    		hDir;
    MTSUA_ControlFileData	cfData;
    
    /* Open the specified directory */
    if ((rc = OS_dirOpen(pDirName, &hDir)) != Success)
    {
	return rc;
    }
    
    /* Get the current date and time. */
    if ((rc = OS_currentDateTime(NULL, &timeNow)) != Success)
    {
	return rc;
    }
    
    /* Forever loop: normal exit is at "No More Files". */
    for (;;)
    {
	/* Get the next file name */
	if ((rc = OS_dirFindNext(hDir, &pFileName)) != Success)
	{
	    return rc;
	}
	
	/* If there aren't any more files here... */
	if (pFileName == NULL)
	{
	    /*
	     * ... then we have a successful return.
	     *
	     * NORMAL LOOP EXIT IS HERE.
	     */
	    return Success;
	}
	
	/* Read the control file */
	if ((rc = mtsua_readControlFile(pFileName, &cfData)) != Success)
	{
	    sprintf(globals.tmpbuf, "rc = 0x%x", rc);
	    EH_problem(globals.tmpbuf);
	    return rc;
	}
	
	/* Determine the time to next retry */
	if (cfData.nextRetryTime > timeNow)
	{
	    /* Set next retry time. */
	    timeToNextRetry = cfData.nextRetryTime - timeNow;
	}
	else
	{
	    /* Tell timer to expire immediately. */
	    timeToNextRetry = 0;
	}
	
	/* We're all done with this control file data */
	mtsua_freeControlFileData(&cfData);
	
	/* Create a timer to retry at the appropriate time */
	if ((rc = TMR_start(TMR_SECONDS(1),
			    pFileName,
			    NULL,
			    pfTryMessageSend,
			    NULL)) != Success)
	{
	    sprintf(globals.tmpbuf, "rc = 0x%x", rc);
	    EH_problem(globals.tmpbuf);
	    return rc;
	}
    }

    /* Never gets here */
    TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, "Never gets here"));
}


ReturnCode
mts_spoolToDevice(void * hTimer, void * hUserData1, void * hUserData2)
{
    ReturnCode 			rc                   = 0;
    OS_Uint32                   count                = 0;
    char * 			pControlFileName     = NULL;
    char * 			pNewControlFileName  = NULL;
    MTSUA_ControlFileData	cfData;

    /* Create a timer to call this function again (later) */
    if ((rc = TMR_start(TMR_SECONDS(5), NULL, NULL, mts_spoolToDevice, NULL)) != Success)
	{
	    sprintf(globals.tmpbuf, "mts_spoolToDevice could not reschedule itself; rc = 0x%x", rc);
	    EH_fatal(globals.tmpbuf);
	    return Fail;
	}

    /* See if there are messages to be handled. */
    rc = OS_dirRewind(globals.config.deliver.hNewMessageDir);
    if (rc != Success)
      {
	sprintf(globals.tmpbuf, "mts_spoolToDevice could not rewind directory file; rc = 0x%x", rc);
	EH_fatal(globals.tmpbuf);
	return Fail;
      }
    for (count = 0; count < 100; count++) {
	rc = OS_dirFindNext(globals.config.deliver.hNewMessageDir, &pControlFileName);
	if (rc != Success) {
		sprintf(globals.tmpbuf, "mts_spoolToDevice got error reading directory; rc = 0x%x", rc);
		EH_fatal(globals.tmpbuf);
		break;
	}
    
	/* Exit loop when there are no more control files */
	if (pControlFileName == NULL) {
		break;
	}
        /* Read the control file */
	rc = mtsua_readControlFile(pControlFileName, &cfData);
	if (rc != Success) {
		sprintf(globals.tmpbuf, "MTS could not read control file (%s); rc = 0x%x\n",
			pControlFileName, rc);
		break;
	}

	TM_TRACE((globals.hTM, MTS_TRACE_MSG, "Adding %s to delivery queue", pControlFileName));
    
        /*** new queue goes here ***/
	rc = msg_addMessage (cfData.messageId, cfData.originator, cfData.recipList, cfData.dataFileName);
	if (rc != Success) {
	    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Cannot add message to delivery queue <%s>, '%s'",
		      cfData.messageId, pControlFileName));
	}

	/* Delete control file */
	OS_deleteFile(pControlFileName);
    
	/* Free allocated resources */
	OS_free(pControlFileName);    

	/* We're done with this control file data */
	mtsua_freeControlFileData(&cfData);
    }
    
    /* Create a timer to try delivering the message (very soon) */
    if ((rc = TMR_start(500, pNewControlFileName, NULL, mts_tryDelivery, NULL)) != Success)
    {
	sprintf(globals.tmpbuf, "mts_spoolToDevice failed to schedule delivery; rc = 0x%x", rc);
	EH_problem(globals.tmpbuf);
    }
    
    return rc;
}


ReturnCode
mts_tryRfc822Submit(void * hTimer,
		    void * hUserData1,
		    void * hUserData2)
{
    char *			pControlFileName = hUserData1;
    FILE *			hDataFile        = NULL;
    ReturnCode			rc               = Success;
    MTSUA_ControlFileData	cfData;

    /* Initialize the control file data recipient queue */
    QU_INIT(&cfData.recipList);

    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "mts_tryRfc822Submit"));

    /* Read the control file */
    if ((rc = mtsua_readControlFile(pControlFileName, &cfData)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Cannot read control file"));
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "MTS could not read control file %s\n",
		      pControlFileName);
	
	rc = Success;
	goto cleanUp;
    }

    /* Open the data file */
    if ((hDataFile = OS_fileOpen(cfData.dataFileName, "rb")) == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Cannot open data file"));
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "MTS could not open data file %s\n",
		      cfData.dataFileName);
	
	mts_rfc822SubmitNonDel(&cfData, OS_RC_FileOpen);
	rc = Success;
	goto cleanUp;
    }

    /* Send the message */
    if ((rc = mts_sendMessage(&cfData, hDataFile)) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Cannot send message to mailer"));
	MM_logMessage(globals.hMMLog, globals.tmpbuf,
		      "MTS could not send MID %s from control file %s\n",
		      cfData.messageId, pControlFileName);

	/* Arrange to delete files unless we start timer */
	rc = Success;

	/* Have we used up our allocation of retries? */
	if (++cfData.retries >= globals.numRetryTimes)
	{
            TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Sending Non-Delivery"));
	    mts_rfc822SubmitNonDel(&cfData, MTS_RC_MaxRetries);
	    goto cleanUp;
	}

	/* Rewrite the control file, with new retry count */
	TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
		 "Rewriting control file for retry"));
	if ((rc = mtsua_writeControlFile(pControlFileName, &cfData)) != Success)
	{
	    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
		     "Error rewriting control file"));
	    mts_rfc822SubmitNonDel(&cfData, rc);
	    goto cleanUp;
	}

	/* Create a timer to try again later */
	TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
		  "Will retry in %ldms", globals.retryTimes[cfData.retries]));
	if ((rc = TMR_start(TMR_SECONDS(globals.retryTimes[cfData.retries])+500,
			    pControlFileName,
			    NULL,
			    mts_tryRfc822Submit,
			    NULL)) != Success)
	{
	    OS_free(pControlFileName);
	    mts_rfc822SubmitNonDel(&cfData, rc);
	    goto cleanUp;
	}

	/* Don't delete files.  We started a timer to process them again. */
	rc = Fail;
    }

  cleanUp:

    /* Close the data file */
    if (hDataFile != NULL)
    {
	OS_fileClose(hDataFile);
    }

    if (rc != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL,
		  "sendMessage failed: not cleaning up control and data files"));
    }
    {
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
		 "Cleaning up control and data files"));
	/* Delete the data file */
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
		 "Deleting '%s'", cfData.dataFileName));
	if (OS_deleteFile(cfData.dataFileName) != Success)
	{
	    MM_logMessage(globals.hMMLog,
			  globals.tmpbuf,
			  "MTS could not delete data file %s\n",
			  cfData.dataFileName);
	}
	
	/* Delete the control file */
        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
		 "Deleting '%s'", pControlFileName));
	if (OS_deleteFile(pControlFileName) != Success)
	{
	    MM_logMessage(globals.hMMLog,
			  globals.tmpbuf,
			  "MTS could not delete control file %s\n",
			  pControlFileName);
	}
	
	/* Free the control file name */
	OS_free(pControlFileName);
    }

    /*  Free the control file data */
    mtsua_freeControlFileData(&cfData);

    /* Timer function always returns Success */
    return Success;
}
