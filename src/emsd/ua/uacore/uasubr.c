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
 *
 * Functions:
 *   ua_issueInvoke(UA_Operation * pOperation, void * hInvokeData)
 *   ua_issueErrorRequest(ESRO_InvokeId invokeId, EMSD_Error errorVal)
 *   ua_issueDelCtrlInvoke(UA_Operation * pOperation, void * hInvokeData)
 *
 * History:
 *
 */


#include "estd.h"
#include "eh.h"
#include "queue.h"
#include "mm.h"
#include "du.h"
#include "buf.h"
#include "tm.h"
#include "tmr.h"
#include "fsm.h"
#include "esro_cb.h"
#include "mtsua.h"
#include "uacore.h"
#include "dup.h"


/*
 * UA_issueInvoke()
 *
 * Issue invoke for submit operation
 *
 * Parameters:
 *
 *   pOperation -- Operation
 *   hInvokeData -- Invoke Data
 *
 * Returns:
 *
 *   Success upon success;
 *   ResourceError if fails
 */

ReturnCode
ua_issueInvoke(UA_Operation * pOperation,
	       void * hInvokeData)
{
    char *		    p;
    unsigned char * 	    pMem;
    OS_Uint16 		    len;
    ReturnCode 		    rc = Success;
    ESRO_RetVal		    retval;
    DU_View		    hView;
    static T_SapSel	    tsap = { 2, { 7, 210 } };
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "UA issuing invoke: SapDesc=%d, SapSel=%d, OpVal=%d",
	      pOperation->hSapDesc, pOperation->sapSel,
	      EMSD_OP_VALUE(pOperation->operationValue)));
	     
    TM_CALL(ua_globals.hTM, UA_TRACE_PDU, BUF_dump, hInvokeData, "");

    /* Allocate a DU buffer for our buffer contents */
    if ((hView =
	 DU_alloc(G_duMainPool,
		  (int) BUF_getBufferLength(hInvokeData))) == NULL)
    {
	pOperation->status = MTSUA_Status_TransientError;
	return ResourceError;
    }

    /* Point to the location to copy the data */
    pMem = DU_data(hView);

    /* Prepare to retrieve the segment data */
    BUF_resetParse(hInvokeData);

    /* For each buffer segment... */
    for (;;)
    {
	/* ... get the segment data, ... */
	len = 0;
	if ((rc = BUF_getChunk(hInvokeData, &len,
			       (unsigned char **) &p)) != Success)
	{
	    if (rc == BUF_RC_BufferExhausted)
	    {
		break;		/* Normal loop exit; no more segments. */
	    }

	    DU_free(hView);
	    return rc;
	}

	/* ... and copy it into our contiguous memory. */
	OS_copy(pMem, (unsigned char *) p, len);
	pMem += len;
    }
    
    /* Add the duplication-detection octet */
    ua_addDuplicateDetection(hView, EMSD_OP_VALUE(pOperation->operationValue));

    /* Issue an ESROS Invoke Request */
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "Issuing INVOKE.req"));
    retval = ESRO_CB_invokeReq(&pOperation->invokeId,
			       NULL,
			       pOperation->hSapDesc,
			       pOperation->sapSel,
			       &tsap,
			       &ua_globals.mtsNsap,
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
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Invoke failed, transient error"));
	return Fail;
    }
    
    /* We're done with our view of this data */
    DU_free(hView);

    return Success;
}


/*
 * ua_issueErrorRequest()
 *
 * Issue error request
 *
 * Parameters:
 *
 *   invokeId -- Invoke Id
 *   errorVal -- Error valu
 *
 */

void
ua_issueErrorRequest(ESRO_InvokeId invokeId,
		     EMSD_Error errorVal)
{
    void *		hBuf = NULL;
    unsigned char *	p;
    OS_Uint16		n;
    OS_Uint32		len;
    ReturnCode		rc;
    EMSD_ErrorRequest 	errorReq;
    ESRO_RetVal	        retval;
    QU_Head *		pCompTree;
    DU_View		hView = NULL;
          
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "Issuing error request: errorVal=%d", errorVal));

    /* Generate an error request */
    errorReq.error = errorVal;
    
    /*
     * Allocate a buffer to begin formatting the Error
     * Indication.
     */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	goto cleanUp;
    }
    
    /* Determine which protocol tree to use. */
    pCompTree = UA_getProtocolTree(MTSUA_Protocol_ErrorRequest);
    
    /* Format an Error Indication */
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 pCompTree,
			 hBuf,
			 &errorReq,
			 &len)) != Success)
    {
	goto cleanUp;
    }
    
    /* Allocate a DU buffer for our buffer contents */
    if ((hView = DU_alloc(G_duMainPool,
			  (int) BUF_getBufferLength(hBuf))) == NULL)
    {
	goto cleanUp;
    }
    
    /* Reset the buffer pointers */
    BUF_resetParse(hBuf);

    /* Get a pointer to the formatted data */
    if ((rc = BUF_getChunk(hBuf, &n, &p)) != Success)
    {
	goto cleanUp;
    }
    
    /* Copy the data to the DU_View */
    OS_copy(DU_data(hView), (unsigned char *) p, n);

    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "Issuing ERROR.req"));
    retval =
	ESRO_CB_errorReq(invokeId, NULL, ESRO_EncodingBER, errorVal, hView);
    if (retval != Success)
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

} /* ua_issueErrorRequest() */



/*
 * ua_issueDelCtrlInvoke()
 *
 * Issue invoke for delivery control operation
 *
 * Parameters:
 *
 *   pOperation -- Operation
 *
 * Returns:
 *
 *   Success upon success;
 *   ResourceError if fails
 */

ReturnCode
ua_issueDelCtrlInvoke(UA_Operation * pOperation,
	       	      void * hInvokeData)
{
    char *		    p;
    unsigned char * 	    pMem;
    OS_Uint16 		    len;
    ReturnCode 		    rc = Success;
    ESRO_RetVal		    retval;
    DU_View		    hView;
    static T_SapSel	    tsap = { 2, { 7, 210 } };
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
	      "UA issuing invoke: SapDesc=%d, SapSel=%d, OpVal=%d",
	      pOperation->hSapDesc, pOperation->sapSel,
	      EMSD_OP_VALUE(pOperation->operationValue)));
	     
    TM_CALL(ua_globals.hTM, UA_TRACE_PDU, BUF_dump, hInvokeData, "");

    /* Allocate a DU buffer for our buffer contents */
    if ((hView =
	 DU_alloc(G_duMainPool,
		  (int) BUF_getBufferLength(hInvokeData))) == NULL)
    {
	pOperation->status = MTSUA_Status_TransientError;
	return ResourceError;
    }

    /* Point to the location to copy the data */
    pMem = DU_data(hView);

    /* Prepare to retrieve the segment data */
    BUF_resetParse(hInvokeData);

    /* For each buffer segment... */
    for (;;)
    {
	/* ... get the segment data, ... */
	len = 0;
	if ((rc = BUF_getChunk(hInvokeData, &len,
			       (unsigned char **) &p)) != Success)
	{
	    if (rc == BUF_RC_BufferExhausted)
	    {
		break;		/* Normal loop exit; no more segments. */
	    }

	    DU_free(hView);
	    return rc;
	}

	/* ... and copy it into our contiguous memory. */
	OS_copy(pMem, (unsigned char *) p, len);
	pMem += len;
    }
    
    /* Add the duplication-detection octet */
    ua_addDuplicateDetection(hView, EMSD_OP_VALUE(pOperation->operationValue));

    /* Issue an ESROS Invoke Request */
    TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY, "Issuing INVOKE.req"));
    retval = ESRO_CB_invokeReq(&pOperation->invokeId,
			       NULL,
			       pOperation->hSapDesc,
			       pOperation->sapSel,
			       &tsap,
			       &ua_globals.mtsNsap,
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
	TM_TRACE((ua_globals.hTM, UA_TRACE_ACTIVITY,
		  "Invoke failed, transient error"));
	return Fail;
    }
    
    /* We're done with our view of this data */
    DU_free(hView);

    return Success;

} /* ua_issueDelCtrlInvoke() */
