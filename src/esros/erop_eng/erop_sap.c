/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File: sap.c
 *
 * Description: SAP processing.
 *
 * Functions:
 *   erop_sapInit(Int nuOfSaps)
 *   EROP_sapBind(EROP_SapSel sapSel, ...)
 *   EROP_sapUnbind(EROP_SapSel sapSel)
 *   erop_getSapInfo(EROP_SapSel sapSel)
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: erop_sap.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include  "target.h"   /* NOTYET */
#include  "estd.h"
#include  "eh.h"
#include  "queue.h"
#include  "seq.h"
#include  "tm.h"
#include  "esro.h"
#include  "invoke.h"
#include  "erop_cfg.h"

#include "erop_sap.h"
#include "local.h"

STATIC SapInfoSeq   sapInfoSeq;
STATIC SEQ_PoolDesc sapInfoPool;

LOCAL TM_ModDesc erop_tmDesc;

extern InvokeInfo *refToInvokerMap[NREFS + 1];
extern InvokeInfo *refToPerformerMap[NREFS + 1];
extern unsigned short refBase;
extern unsigned short newRef;	

#define EROP_SapSel ESRO_SapSel


/*<
 * Function:    erop_sapInit
 * 
 * Description: Initialize the EROP_ module.
 *	        Create a Tracing Module.
 *
 * Arguments:   Number of SAP's.
 *
 * Returns:     None.
 * 
>*/

PUBLIC Void
erop_sapInit(Int nuOfSaps)
{
    static Bool virgin = TRUE;

    if ( !virgin ) {
	return;
    }
    virgin = FALSE;

    sapInfoPool = SEQ_poolCreate(sizeof(*sapInfoSeq.first), 0);
    QU_INIT(&sapInfoSeq);
}



/*<
 * Function: EROP_sapBind
 *
 * Description: Bind an Address to a Service User.
 *
 * Arguments: SAP selector, SAP descriptor (outgoing argument), action
 *            functions.
 *
 * Returns:   0 if successful, a negative error value if unsuccessful.
 * 
>*/

PUBLIC Int
EROP_sapBind(EROP_SapDesc *sapDesc, EROP_SapSel sapSel, 
	     EROP_FunctionalUnit functionalUnit,
	     int (*invokeInd) (EROP_SapSel 	   locEROPSap,
			       EROP_SapSel 	   remEROPSap,
			       T_SapSel		   *remTsap,
			       N_SapAddr	   *remNsap,
			       EROP_InvokeDesc     invoke,
			       EROP_OperationValue operationValue,
			       EROP_EncodingType   encodingType,
			       DU_View		   parameter),
	     int (*resultInd) (EROP_InvokeDesc   invoke,
			       EROP_UserInvokeRef  userInvokeRef,
			       EROP_EncodingType encodingType,
			       DU_View		 parameter),
	     int (*errorInd) (EROP_InvokeDesc   invoke,
			      EROP_UserInvokeRef  userInvokeRef,
			      EROP_EncodingType	encodingType,
			      EROP_ErrorValue   errorValue,
			      DU_View 		parameter),
	     int (*resultCnf) (EROP_InvokeDesc  invoke,
			       EROP_UserInvokeRef  userInvokeRef),
	     int (*errorCnf) (EROP_InvokeDesc   invoke,
			      EROP_UserInvokeRef  userInvokeRef),
	     int (*failureInd) (EROP_InvokeDesc   invoke,
			        EROP_UserInvokeRef  userInvokeRef,
				EROP_FailureValue failureValue))
{
    SapInfo *sapInfo;

    /*
     * If no SAP-ID specified, we will only allow one TS user which will
     * match the null SAP-ID. 
     */
    if (sapSel == 0 ) {
	EH_problem("EROP_sapBind: Bad Sap");
	return  -3; 
    }

    if ( sapSel > ESRO_MAX_SAP_NO ) { /* no need to check min because unsigned */
	EH_problem("EROP_sapBind: Invalid Sap number\n");
    	TM_TRACE((EROP_modCB, TM_ENTER, 
		 "               Accepted range: %d to %d\n",
		 ESRO_MIN_SAP_NO, ESRO_MAX_SAP_NO));
	return  -6; 
    }

    if ( ! (sapInfo = (SapInfo *) SEQ_elemObtain(sapInfoPool)) ) {
	EH_problem("EROP_sapBind: SEQ_elemObtain failed");
	return -2;
    }

    /* If the SAP-ID is already in use, the function fails.  */
    if (erop_getSapInfo(sapSel) != (SapInfo *) 0) {
	EH_problem("EROP_sapBind: Sap in use");
	return -4; 
    }

    if (functionalUnit != ESRO_3Way  &&  functionalUnit != ESRO_2Way) {
	EH_problem("EROP_sapBind: Invalid functional unit. It should be 2-way or 3-way\n");
	return -5; 
    }

    /* So the SAP can be created */

    sapInfo->sapSel = sapSel;
    sapInfo->functionalUnit = functionalUnit;

    sapInfo->invokeInd  = invokeInd;
    sapInfo->resultInd  = resultInd;
    sapInfo->errorInd   = errorInd;
    sapInfo->resultCnf  = resultCnf;
    sapInfo->errorCnf   = errorCnf;
    sapInfo->failureInd = failureInd;

    QU_INIT(sapInfo);
    QU_INSERT(&sapInfoSeq, sapInfo);

    QU_INIT(&sapInfo->invokeSeq);

    TM_TRACE((EROP_modCB, TM_ENTER,
	    "EROP_sapBind (before return) sapSel=%d  functionalUnit=%d\n", 
	    sapSel, functionalUnit));

#ifndef NO_UPSHELL
    TM_TRACE((EROP_modCB, TM_ENTER, 
    	     "***** erop_engine compiled WITHOUT NO_UPSHELL for one process call back *****\n"));
    *sapDesc = sapInfo;
#else
    TM_TRACE((EROP_modCB, TM_ENTER, 
    	     "***** erop_engine compiled WITH NO_UPSHELL for one process call back *****\n"));
    *sapDesc = (EROP_SapDesc) (unsigned long)sapSel;
#endif

    return 0;
}



/*<
 * Function:    EROP_sapUnbind
 *
 * Description: Unbind a service user.
 *
 * Arguments:   SAP selector.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

PUBLIC SuccFail
EROP_sapUnbind(EROP_SapSel sapSel)
{
    SapInfo *sap;

    TM_TRACE((EROP_modCB, TM_ENTER,
	    "EROP_sapUnbind (Entry) sapSel=%d\n", sapSel));

    if ((sap = erop_getSapInfo(sapSel)) == (SapInfo *) 0) {
        TM_TRACE((EROP_modCB, TM_ENTER,
	         "EROP_sapUnbind: Sap not active (%d)!\n", sapSel));
	return ( FAIL ); /*	return -2;   *** err.h ***/
    }

    /*
     * We refuse to unregister a SAP-ID which is in use.
     */

    if (! QU_EQUAL(QU_FIRST(&sap->invokeSeq), &sap->invokeSeq)) {
#if 0
	EH_problem("EROP_sapUnbind: SAP has active invokations");
	return ( FAIL ); /*	return -3;   *** err.h ***/
#endif
 	Int refNu;
	InvokeInfo *invokeInfo;
	InvokeInfo *invokeInfoNext;

#if 0		/* NOTYET: The real problem is not masking interrupts at
			   queue insert time. This should be removed when
			   signal-based fix is verified */

	for (invokeInfo = sap->invokeSeq.first; 
             invokeInfo != (InvokeInfo *) &sap->invokeSeq;) {
	    if ( invokeInfo->performingNotInvoking == TRUE ) {
            	for (refNu = 0; refNu < NREFS; refNu++) {
		    if (invokeInfo == refToPerformerMap[refNu]) {
		    	refToPerformerMap[refNu] = (InvokeInfo *)0;
		    }
	    	}
	    } else {
            	for (refNu = 0; refNu < NREFS; refNu++) {
		    if (invokeInfo == refToInvokerMap[refNu]) {
			refToInvokerMap[refNu] = (InvokeInfo *)0;
		    }
	    	}
	    }
#else
	invokeInfo = sap->invokeSeq.first; 
	if ( invokeInfo->performingNotInvoking == TRUE ) {
            for (refNu = 0; refNu < NREFS; refNu++) {
	    	if (invokeInfo == refToPerformerMap[refNu]) {
		    refToPerformerMap[refNu] = (InvokeInfo *)0;
		}
	    }
	} else {
            for (refNu = 0; refNu < NREFS; refNu++) {
	    	if (invokeInfo == refToInvokerMap[refNu]) {
		    refToInvokerMap[refNu] = (InvokeInfo *)0;
		}
	    }
	}
#endif

            erop_tmrCancel(invokeInfo, erop_allTimer);

	    invokeInfoNext = invokeInfo->next;

	    erop_refFree(invokeInfo);
	    erop_freeLopsInvoke(invokeInfo); 
	    erop_invokeInfoFree(invokeInfo);
		
	    invokeInfo = invokeInfoNext;
#if 0
	}
#endif
    	QU_INIT(&sap->invokeSeq);
    }

    /* Free SAP table entry  */

    QU_REMOVE(sap);
    SEQ_elemRelease(sapInfoPool, sap);

    TM_TRACE((EROP_modCB, TM_ENTER,
	    "erop_sapUnbind (exit) sapSel=%d\n", sapSel));

    return ( SUCCESS );
}



/*<
 * Function:    erop_getSapInfo
 *
 * Description: Get SAP information. Find entry in local service user table.
 *
 * Arguments:   SAP selector.
 *
 * Returns:     Pointer to SAP information struct if successful, NULL if 
 *              unsuccessful.
 *
>*/

LOCAL SapInfo *
erop_getSapInfo(ESRO_SapSel sapSel)
{
    SapInfo *sap;

    for (sap = QU_FIRST(&sapInfoSeq); 
	 ! QU_EQUAL(sap, &sapInfoSeq);
	 sap = QU_NEXT(sap)) {

	if (!sap) {
	    EH_problem("erop_getSapInfo: sap queue pointing to zero\n")
    	    return ((SapInfo *) 0);
	}

	if (sapSel == sap->sapSel) {
	    return (sap);
	}
    }

    TM_TRACE((EROP_modCB, TM_ENTER,
	    "erop_getSapInfo: No sapInfo found for sapSel(%d)\n", sapSel));
#ifdef TM_ENABLED
    for (sap = QU_FIRST(&sapInfoSeq); 
     	 ! QU_EQUAL(sap, &sapInfoSeq);
	 sap = QU_NEXT(sap)) {
    	TM_TRACE((EROP_modCB, TM_ENTER,
                 "erop_getSapInfo: Active SAP: %d\n", sap->sapSel));
    	}
#endif

    return ((SapInfo *) 0);
}


