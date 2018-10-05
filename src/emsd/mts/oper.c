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

static char *rcs = "$Id: oper.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";


#include "estd.h"
#include "queue.h"
#include "fsm.h"
#include "mtsua.h"
#include "mts.h"


/*
 * mts_allocOperation()
 */
ReturnCode
mts_allocOperation(MTS_Operation ** ppOperation)
{
    int			 i;
    MTS_Operation *	 pOperation;

    static unsigned int  serialNumber = 1;
    
    if ((pOperation = OS_alloc(sizeof(MTS_Operation))) == NULL)
    {
	return ResourceError;
    }
    
        /* Initialize the structure fields */
    QU_INIT(pOperation);
    pOperation->operId              = serialNumber++;
    pOperation->invokeId            = -1;
    pOperation->encodingType        = ASN_EncodingRules_Basic;
    pOperation->hFsm                = NULL;
    pOperation->hSapDesc            = NULL;
    pOperation->rc                  = Success;
    pOperation->bComplete           = FALSE;
    pOperation->hOperationData      = NULL;
    pOperation->hFirstOperationData = NULL;
    pOperation->hFullPduBuf         = NULL;
    pOperation->pControlData        = NULL;

    for (i = 0; i < DIMOF(pOperation->hSegmentBuf); i++)
    {
	pOperation->hSegmentBuf[i] = NULL;
    }

    pOperation->expectedSegmentMask = 0x0;
    pOperation->receivedSegmentMask = 0x0;
    pOperation->sequenceId          = 0;
    pOperation->lastSegmentNumber   = 0;
    pOperation->nextSegmentNumber   = 0;
    pOperation->segmentLength       = 0;
    pOperation->hSegmentationTimer  = NULL;
    pOperation->hCurrentBuf         = NULL;
    pOperation->retryCount          = 0;
    pOperation->pControlFileName    = NULL;
    pOperation->pControlFileData    = NULL;
    pOperation->status              = MTSUA_Status_Success;

    QU_INIT(&pOperation->userFreeQHead);
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Creating Operation {%ld}",
	      pOperation->operId));

    /* Insert this operation onto the queue of active operations */
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
	     "Inserting activeOperation %08lx (%d) {%ld}",
	      pOperation, pOperation->invokeId, pOperation->operId));

    QU_INSERT(pOperation, &globals.activeOperations);

    /* Give 'em what they came for */
    *ppOperation = pOperation;
    
    return Success;
}


/*
 * mts_freeOperation()
 */
void
mts_freeOperation(MTS_Operation * pOperation)
{
    MTS_OperationFree *	pOpFree;
    char                fnh[2];

    fnh[0] = fnh[1];	/* caused purify to log this call */
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
	     "Freeing Operation %08x: invokeId = %d {%ld}",
	      pOperation, pOperation->invokeId, pOperation->operId));

    /* Remove the operation from the Active Operations queue */
    QU_REMOVE(pOperation);
    
    /* Free the finite state machine */
/*  (void) FSM_deleteMachine(pOperation->hFsm); *** fnh ***/
    
    /* For each operation-specific free function... */
    for (pOpFree = QU_FIRST(&pOperation->userFreeQHead);
	 ! QU_EQUAL(pOpFree, &pOperation->userFreeQHead);
	 pOpFree = QU_FIRST(&pOperation->userFreeQHead))
    {
	/* ... remove it from the queue, ... */
	QU_REMOVE(pOpFree);
	
	/* ... call it, ... */
	(* pOpFree->pfFree)(pOpFree->hUserData);
	
	/* ... and free it. */
	OS_free(pOpFree);
    }
    
    /* Free the operation */
    OS_free(pOperation);
}

/*
 * mts_operationAddUserFree()
 */
ReturnCode
mts_operationAddUserFree(MTS_Operation * pOperation,
			 void (* pfFree)(void * hUserData),
			 void * hUserData)
{
    MTS_OperationFree *	pOpFree;
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
	      "mts_operationAddUserFree: invokeId = %d, hUserData = %08lx {%ld}",
	      pOperation->invokeId, hUserData, pOperation->operId));

    /* Allocate an Operation Free structure */
    if ((pOpFree = OS_alloc(sizeof(MTS_OperationFree))) == NULL)
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
    
    return Success;
}


/*
 * mts_operationDelUserFree()
 */
void
mts_operationDelUserFree(MTS_Operation * pOperation,
			 void * hUserData)
{
    MTS_OperationFree *	pOpFree;
    
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
	      "mts_operationDelUserFree: invokeId = %d, hUserData = %08lx {%ld}",
	      pOperation->invokeId, hUserData, pOperation->operId));

    for (pOpFree = QU_FIRST(&pOperation->userFreeQHead);
	 ! QU_EQUAL(pOpFree, &pOperation->userFreeQHead);
	 pOpFree = QU_NEXT(pOpFree))
    {
	/* Is this the one we're looking for? */
	if (pOpFree->hUserData == hUserData)
	{
	    /* Yup.  Remove it from the queue */
	    QU_REMOVE(pOpFree);
	    
	    /* Free the queue element.  Don't call the function. */
	    OS_free(pOpFree);
	    
	    /* We found it already.  No need to stick around. */
	    return;
	}
    }
}
