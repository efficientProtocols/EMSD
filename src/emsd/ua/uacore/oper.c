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
 *    ua_allocOperation()
 *    ua_freeOperation()
 *    ua_operationAddUserFree()
 *    ua_operationDelUserFree()
 */


#include "estd.h"
#include "queue.h"
#include "fsm.h"
#include "mtsua.h"
#include "eh.h"
#include "uacore.h"


/*
 * ua_allocOperation()
 */
ReturnCode
ua_allocOperation(UA_Operation ** ppOperation,
		  OS_Uint8 * pPool,
		  void *hShellReference)
{
    int			i;
    UA_Operation *	pOperation;
    
    /* Make sure we can still allocate from the specified pool */
    if (*pPool == 0)
    {
	/* Nope.  Pool exhausted. */
	TM_TRACE((ua_globals.hTM, UA_TRACE_WARNING,
		  "Allocate Operation failed, pool exhausted"));
	return ResourceError;
    }

    /* Allocate a new operation */
    if ((pOperation = OS_alloc(sizeof(UA_Operation))) == NULL)
    {
	EH_problem("ua_allocOperation: OS_alloc failed\n");
	return ResourceError;
    }
    
    /* Initialize the structure fields */
    QU_INIT(pOperation);
    pOperation->encodingType 	= ASN_EncodingRules_Basic;
    pOperation->hFsm 		= NULL;
    pOperation->hSapDesc 	= 0;
    pOperation->rc 		= Success;
    pOperation->bComplete 	= FALSE;
    pOperation->hOperationData 	= NULL;
    pOperation->hFirstOperationData = NULL;
    pOperation->hFullPduBuf 	= NULL;

    for (i=0; i<DIMOF(pOperation->hSegmentBuf); i++)
    {
	pOperation->hSegmentBuf[i] = NULL;
    }

    pOperation->expectedSegmentMask 	= 0x0;
    pOperation->receivedSegmentMask 	= 0x0;
    pOperation->sequenceId 		= 0;
    pOperation->lastSegmentNumber 	= 0;
    pOperation->lastSegmentNumber 	= 0;
    pOperation->segmentLength 		= 0;
    pOperation->hSegmentationTimer 	= NULL;
    QU_INIT(&pOperation->pendingQHead);
    pOperation->hCurrentBuf 		= NULL;
    pOperation->retryCount 		= 0;
    pOperation->status 			= MTSUA_Status_Success;
    QU_INIT(&pOperation->userFreeQHead);
    pOperation->pPool 			= pPool;

    pOperation->hShellReference = hShellReference; /* should be done this way */
    strcpy ((char *)pOperation->pHackData, (char *)hShellReference); /* hack */
    
    /* Insert this operation onto the queue of active operations */
    QU_INSERT(pOperation, &ua_globals.activeOperations);

    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Allocate Operation 0x%lx", pOperation));

    /* Give 'em what they came for */
    *ppOperation = pOperation;
    
    /* Decrement the pool count */
    --*pOperation->pPool;

    return Success;

} /* ua_allocOperation() */


/*
 * ua_freeOperation()
 */
void
ua_freeOperation(UA_Operation * pOperation)
{
    UA_OperationFree *	pOpFree;
    UA_Pending *	pPending;
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Free Operation 0x%lx...", pOperation));

    /* Remove the operation from the Active Operations queue */
    QU_REMOVE(pOperation);
    
    /* For each operation structure in the pending queue... */
    for (pPending = QU_FIRST(&pOperation->pendingQHead);
	 ! QU_EQUAL(pPending, &pOperation->pendingQHead);
	 pPending = QU_FIRST(&pOperation->pendingQHead))
    {
	/* ... remove it from the queue, ... */
	QU_REMOVE(pPending);

	/* ... and free it. */
	OS_free(pPending);
    }

    /* For each operation-specific free function... */
    for (pOpFree = QU_FIRST(&pOperation->userFreeQHead);
	 ! QU_EQUAL(pOpFree, &pOperation->userFreeQHead);
	 pOpFree = QU_FIRST(&pOperation->userFreeQHead))
    {
	/* ... remove it from the queue, ... */
	QU_REMOVE(pOpFree);
	
	TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
		  "Operation User Data Free 0x%lx: calling (* 0x%lx)(0x%lx)",
		  pOpFree, pOpFree->pfFree, pOpFree->hUserData));

	/* ... call it, ... */
	(* pOpFree->pfFree)(pOpFree->hUserData);
	
	/* ... and free it. */
	OS_free(pOpFree);
    }
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "... Done with Free Operation 0x%lx", pOperation));

    /* There's now one more operation available in this operation pool */
    ++*pOperation->pPool;

    /* Free the operation */
    OS_free(pOperation);

} /* ua_freeOperation() */



/*
 * ua_operationAddUserFree()
 */
ReturnCode
ua_operationAddUserFree(UA_Operation * pOperation,
			void (* pfFree)(void * hUserData),
			void * hUserData)
{
    UA_OperationFree *	pOpFree;
    
    /* Allocate an Operation Free structure */
    if ((pOpFree = OS_alloc(sizeof(UA_OperationFree))) == NULL)
    {
	return ResourceError;
    }
    
    /* Initialize the queue pointers */
    QU_INIT(pOpFree);

    /* Initialize per parameters */
    pOpFree->pfFree = pfFree;
    pOpFree->hUserData = hUserData;
    
    /* Add this user request to the queue */
    QU_INSERT(pOpFree, &pOperation->userFreeQHead);
    
    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Add Operation User Data Free for operation 0x%lx:\n"
	      "\t0x%lx to call (* 0x%lx)(0x%lx)",
	      pOperation, pOpFree, pOpFree->pfFree, pOpFree->hUserData));

    return Success;

} /* ua_operationAddUserFree() */


/*
 * ua_operationDelUserFree()
 */
void
ua_operationDelUserFree(UA_Operation * pOperation,
			void * hUserData)
{
    UA_OperationFree *	pOpFree;
    
    for (pOpFree = QU_FIRST(&pOperation->userFreeQHead);
	 ! QU_EQUAL(pOpFree, &pOperation->userFreeQHead);
	 pOpFree = QU_NEXT(pOpFree))
    {
	/* Is this the one we're looking for? */
	if (pOpFree->hUserData == hUserData)
	{
	    /* Yup.  Remove it from the queue */
	    QU_REMOVE(pOpFree);
	    
	    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
		      "Delete Operation User Data Free for operation 0x%lx:\n"
		      "\t0x%lx to call (* 0x%lx)(0x%lx)",
		      pOperation, pOpFree,
		      pOpFree->pfFree, pOpFree->hUserData));

	    /* Free the queue element.  Don't call the function. */
	    OS_free(pOpFree);
	    
	    /* We found it already.  No need to stick around. */
	    return;
	}
    }

    TM_TRACE((ua_globals.hTM, UA_TRACE_DETAIL,
	      "Delete Operation User Data Free for operation 0x%lx:\n"
	      "\t0x%lx NOT FOUND",
	      pOperation, pOpFree));

} /* ua_operationDelUserFree() */
