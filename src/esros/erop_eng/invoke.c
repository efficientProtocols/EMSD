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
 * File: invoke.c
 *
 * Description: Reference number and invoke ID initialization and handling.
 *
 * Functions:
 *    erop_invokeInit(Int nuOfInvokes)
 *    erop_invokeInfoCreate(SapInfo *sap)
 *    erop_invokeInfoFree(InvokeInfo *invokeInfo)
 *    erop_invokeInfoInit(InvokeInfo *invoke)
 *    erop_refInit(Int)
 *    erop_refNew(InvokeInfo *invoke)
 *    erop_refKeep(Int refNu, InvokeInfo *invoke)
 *    erop_refToPerformer(short unsigned int refNu)
 *    erop_refToInvoker(short unsigned int refNu)
 *    erop_refFree(InvokeInfo *invoke)
 *    erop_freeLopsInvoke(InvokeInfo *invoke)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: invoke.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "eh.h"
#include "estd.h"
#include "queue.h"
#include "seq.h"
#include "tm.h"
#include "sf.h"
#include "sap.h"

#include "erop_sap.h"
#include "invoke.h"
#include "eropfsm.h"
#include "fsm.h"
#include "nm.h"
#include "layernm.h"

#include "target.h"

#include "extfuncs.h"
#include "local.h"

STATIC InvokeInfoSeq invokeInfoSeq;
STATIC SEQ_PoolDesc invokeInfoPool;

unsigned int nSapAddrOffset;	/* Offset of NSAP address from refMap (refNuMap)*/
    				/* &invokeInfo.refMap - &invokeInfo.remNsapAddr */
unsigned int invokeInfoOffset;  /* Offset of start of invokeInfo from refMap */
    				/* &invokeInfo.refMap - &invokeInfo */

Int totalRefNumbers = 0;

QU_Head *refToInvokerMap;
QU_Head *refToPerformerMap;

/*
 *  Map for quick translations between an Invoke-Ref-Number and
 *  the correspondent InvokeInfo structure.
 *
 *  refbase contains the first local reference number which is in use.
 *  This number is incremented by NREFS each time the ESROS layer
 *  is initialized, which will guarantee that we will not reuse the
 *  same reference number too soon.
 */

/* NOTYET -- All signed and unsigned comparisions should be reviewed */

Int refBase 	       = 0;		/* Reference Number Base */
unsigned short newRef  = 0;		/* New Reference Number  */


/*<
 * Function:    erop_invokeInit
 *
 * Description: Create and initialize invoke ID pool and initialize
 *
 * Arguments:   Number of invoke IDs
 *
 * Returns:     None.
 *
>*/

LOCAL Void
erop_invokeInit(Int nuOfInvokes)
{
    static Bool virgin = TRUE;
    InvokeInfo invokeInfo;

    if ( !virgin ) {
	return;
    }
    virgin = FALSE;

    if ((invokeInfoPool = SEQ_poolCreate(sizeof(*invokeInfoSeq.first), 0)) 
	== NULL) {
	return;
    }

    QU_INIT(&invokeInfoSeq);

    /* Calulate offsets from the head of the reference number mapping queue */

    nSapAddrOffset   =  (unsigned int)&invokeInfo.refMap - 
			(unsigned int)&invokeInfo.remNsapAddr;
    invokeInfoOffset =  (unsigned int)&invokeInfo.refMap - 
			(unsigned int)&invokeInfo;

} /* erop_invokeInit() */



/*<
 * Function:    erop_invokeInfoCreate
 *
 * Description: Create an invokation information entity.
 *
 * Arguments:   SAP.
 *
 * Returns:     Pointer to invoke information struct.
 *
>*/

LOCAL InvokeInfo *
erop_invokeInfoCreate(SapInfo *sap)
{
    InvokeInfo *invokeInfo;

    if ( ! (invokeInfo = (InvokeInfo *) SEQ_elemObtain(invokeInfoPool)) ) {
	/* NM_incCounter(NM_ESROS, &erop_noInvokeInfosCounter, 1); */
	EH_problem("erop_invokeInfoCreate: SEQ_elemObtain failed\n");
	return ((InvokeInfo *) 0);
    }

    QU_MOVE(invokeInfo, &sap->invokeSeq);

    invokeInfo->locSap = sap;

    erop_invokeInfoInit(invokeInfo);

    return (invokeInfo);

} /* erop_invokeInfoCreate() */



/*<
 * Function:    erop_invokeInfoFree
 *
 * Description: Release and invoke information entity.
 *
 * Arguments:   Pointer to invoke info struct.
 *
 * Returns:     None.
 *              If input parameter is a NULL pointer, no action is performed.
 *              If input parameter is an invalid pointer, undefined results.
 *
>*/

LOCAL Void
erop_invokeInfoFree(InvokeInfo *invokeInfo)
{
    DU_View du;
#if 0 /* NOTYET */
    DU_View duNext;
#endif
    PduSeq *pduSeq;

    if (invokeInfo == (InvokeInfo *)0) {
	return;
    }

/*    SF_memRelease(invokeInfo->curState);  */
 
#if 0		/* NOTYET: The real problem is not masking interrupts at
			   queue insert time. This should be removed when
			   signal-based fix is verified */

   for (pduSeq = &invokeInfo->invokePduSeq, 
	du = (DU_View)QU_FIRST(pduSeq); 
        ! QU_EQUAL(du, (DU_View)pduSeq);
	du = duNext) {
	duNext = QU_NEXT(du);
	QU_REMOVE(du);

	if (du) {
	    TM_TRACE((DU_modCB, DU_MALLOC, 
                     "erop_invokeInfoFree: DU_free: du=0x%lx\n", du));
  	    DU_free (du);
	}
   }

   for (pduSeq = &invokeInfo->resultPduSeq, 
	du = (DU_View)QU_FIRST(pduSeq); 
        ! QU_EQUAL(du, (DU_View)pduSeq);
	du = duNext) {

	duNext = QU_NEXT(du);
	QU_REMOVE(du);

	if (du) {
	    TM_TRACE((DU_modCB, DU_MALLOC, 
                 "erop_invokeInfoFree: DU_free: du=0x%lx\n", du));
  	    DU_free (du);
	}
   }
#else
   pduSeq = &invokeInfo->invokePduSeq;
   du = (DU_View)QU_FIRST(pduSeq); 
   if (! QU_EQUAL(du, (DU_View)pduSeq)) {
	QU_REMOVE(du);
	if (du) {
	    TM_TRACE((DU_modCB, DU_MALLOC, 
                     "erop_invokeInfoFree: DU_free: du=0x%lx\n", du));
  	    DU_free (du);
	}
   }

   pduSeq = &invokeInfo->resultPduSeq;
   du = (DU_View)QU_FIRST(pduSeq); 
   if (! QU_EQUAL(du, (DU_View)pduSeq)) {
   	QU_REMOVE(du);
       	if (du) {
	    TM_TRACE((DU_modCB, DU_MALLOC, 
                "erop_invokeInfoFree: DU_free: du=0x%lx\n", du));
      	    DU_free (du);
 	}
   }
#endif

    QU_remove((QU_Element *)invokeInfo);
    SEQ_elemRelease(invokeInfoPool, invokeInfo);
}


/*<
 * Function:    erop_invokeInfoInit
 *
 * Description: This procedure initializes an InvokeInfo before it is used.
 *
 * Arguments:   Pointer to invoke info.
 *
 * Returns:     None.
 *
>*/

LOCAL Void
erop_invokeInfoInit(InvokeInfo *invoke)
{
    invoke->remNsapAddr.len = 0;	/* reset remote NSAP address length */
    invoke->remTsapSel.len  = 0;	/* reset remote TSAP selector length */

    QU_init((QU_Element *)&invoke->tmrQu);      /* Initialize Timer Queue */
    QU_init((QU_Element *)&invoke->refMap);     /* Initialize reference no map */

    erop_pduSeqInit(&invoke->invokePduSeq);  /* Initialize invoke PDU sequence */
    erop_pduSeqInit(&invoke->resultPduSeq);  /* Initialize result PDU sequence */
}

/*<
 * Function:    erop_refInit
 *
 * Description: Initialize reference number tables.
 *
 * Arguments:   Total number of reference numbers.
 *
 * Returns:     0 if successfule, a negative error value otherwise.
 *
>*/

LOCAL Int
erop_refInit(Int totalRefs)
{
    Int i;
    static Bool virgin = TRUE;
#ifdef TIME_AVAILABLE
    OS_ZuluDateTime *pDateTime;
    OS_Uint32 *pJulianDate;
#endif

    if (!virgin) {
    	TM_TRACE((EROP_modCB, TM_ENTER, 
		 "WARNING: erop_refInit: Function is called more than once\n"));
	return -2;
    }

    virgin = FALSE;

    if (totalRefs <= 0) {
	EH_problem("erop_refInit: Invalid Total Reference Numbers\n");
	return FAIL;
    }
    
    totalRefNumbers = totalRefs;

    refBase = -totalRefNumbers;
#ifdef TIME_AVAILABLE
    newRef = OS_timeMinSec();
    newRef = newRef % 900;
    newRef = newRef / 4;
#else
    newRef  =  totalRefNumbers;	
#endif

    if ((refToInvokerMap = 
	(QU_Head *)SF_memGet((totalRefNumbers + 1)*sizeof(QU_Head))) == 0) {
	EH_problem("erop_refInit: Memory allocation failed for refToInvokerMap\n");
	return FAIL;
    } 
    if ((refToPerformerMap = 
	(QU_Head *)SF_memGet((totalRefNumbers + 1)*sizeof(QU_Head))) == 0) {
	EH_problem("erop_refInit: Memory allocation failed for refToPerformerMap\n");
	return FAIL;
    } 

    for (i = 0; i <= totalRefNumbers; i++) {
	QU_INIT(&refToInvokerMap[i]);
	QU_INIT(&refToPerformerMap[i]);
    }
    refBase += totalRefNumbers;

    return (SUCCESS);

} /* erop_refInit() */


/*<
 * Function:    erop_refNew
 *
 * Description: Return an unused reference number
 *
 * Arguments:   Invoke info.
 *
 * Returns:     Reference number (a positive number) if successful, a negative
 *              error number if unsuccessful.
 *
>*/

LOCAL short 
erop_refNew(InvokeInfo *invoke)
{
    Int i;
    QU_Element *elem;

    /* RefNew only applies to the invoker */
    if ( invoke->performingNotInvoking ) {
	EH_problem("erop_refNew: called by performer\n");
        return (FAIL);
    }

    for (i = 0; i < totalRefNumbers; i++) { 
	if (++newRef >= totalRefNumbers) {
	    newRef = 0;
	}
       	for (elem = QU_FIRST(&refToInvokerMap[newRef]); 
	     ! QU_EQUAL(elem, &refToInvokerMap[newRef]); elem = QU_NEXT(elem) ) {
	    if (N_sapAddrCmp(&invoke->remNsapAddr, erop_getNsapAddr(elem)) == 0) {
	    	break;
	    }
	}
	if (QU_EQUAL(elem, &refToInvokerMap[newRef])) {
            QU_INSERT(&refToInvokerMap[newRef], &invoke->refMap); 
	    return (newRef + refBase);
        }
    }

    EH_problem("erop_refNew: Invoker Out of Reference numbers\n");
    return -1;    /*** err.h  ***/

} /* erop_refNew() */


/*<
 * Function:    Associate the given invocation with the given reference number.
 *		and register the ref number as used.
 *
 * Description: This primitive returns a reference number to be used in
 *              identifying a connection. ????
 *
 * Arguments:   Reference number, pointer to invoke structure.
 *
 * Returns:	0 is successful, -1 otherwise
 *
>*/

LOCAL SuccFail
erop_refKeep(Int refNu, InvokeInfo *invoke)
{
    QU_Element *elem;

    /* RefKeep only applies to the performer */
    if ( invoke->performingNotInvoking == FALSE ) {
	EH_problem("erop_refKeep: called by performer\n");
	return ( FAIL );
    }

    if (refNu >= refBase && refNu < (refBase + totalRefNumbers)) {
       	for (elem = QU_FIRST(&refToPerformerMap[refNu]); 
	     ! QU_EQUAL(elem, &refToPerformerMap[refNu]); elem = QU_NEXT(elem) ) {
	    if (N_sapAddrCmp(&invoke->remNsapAddr, erop_getNsapAddr(elem)) == 0) {
 		EH_problem("erop_refKeep: Reference number already in use\n");
		return ( FAIL );
	    }
	}
       	QU_INSERT(&refToPerformerMap[refNu], &invoke->refMap); 
	return (SUCCESS);

    } else {
	EH_problem("erop_refKeep: invalid reference number\n");
    	TM_TRACE((EROP_modCB, TM_ENTER, 
		 "erop_refKeep: Invalid REF NO=%d\n", refNu));
	return ( FAIL );
    }

} /* erop_refKeep() */



/*<
 * Function:    erop_refToPerformer
 *
 * Description: Get a reference number and return the corresponding invoke ID.
 *
 * Arguments:   Reference number.
 *
 * Returns:     Inovke info for valid reference number, 0 otherwise.
 *
>*/

LOCAL InvokeInfo *
erop_refToPerformer(short unsigned int refNu, N_SapAddr *nSapAddr)
{
    QU_Element *elem;
    /* only applies to the performer */

    if (refNu >= refBase && refNu < (refBase + totalRefNumbers)) {
       	for (elem = QU_FIRST(&refToPerformerMap[refNu]); 
	     ! QU_EQUAL(elem, &refToPerformerMap[refNu]); elem = QU_NEXT(elem) ) {
	    if (N_sapAddrCmp(erop_getNsapAddr(elem), nSapAddr) == 0) {
		return (InvokeInfo *)erop_getInvokeInfoAddr(elem);
	    }
	}
 	EH_problem("erop_refKeep: No operation can be associated with the given reference number\n");
	return (InvokeInfo *)0;

    } else {
	EH_problem("erop_refKeep: invalid reference number\n");
	return (InvokeInfo *)0;
    }

} /* erop_refToPerformer() */


/*<
 * Function:    erop_refToInvoker
 *
 * Description: Get reference number and return the corresponding invoke info.
 *
 * Arguments:   Reference number.
 *
 * Returns:     Inovke info for valid reference number, 0 otherwise.
 *
>*/

LOCAL InvokeInfo *
erop_refToInvoker(short unsigned int refNu, N_SapAddr *nSapAddr)
{
    QU_Element *elem;
    /* only applies to the invoker */

    if (refNu >= refBase && refNu < (refBase + totalRefNumbers)) {
       	for (elem = QU_FIRST(&refToInvokerMap[refNu]); 
	     ! QU_EQUAL(elem, &refToInvokerMap[refNu]); elem = QU_NEXT(elem) ) {
	    if (N_sapAddrCmp(erop_getNsapAddr(elem), nSapAddr) == 0) {
		return (InvokeInfo *)erop_getInvokeInfoAddr(elem);
	    }
	}
 	EH_problem("erop_refToInvoker: No operation can be associated with the given reference number\n");
	return (InvokeInfo *)0;

    } else {
	EH_problem("refToInvoker: invalid reference number\n");
	return (InvokeInfo *)0;
    }

} /* erop_refToInvoker() */


/*<
 * Function:    erop_refFree
 *
 * Description: Release reference number.
 *
 * Arguments:   Invoke info.
 *
 * Returns:     None.
 *
>*/

LOCAL Void
erop_refFree(InvokeInfo *invoke)
{
   QU_REMOVE(&invoke->refMap);
   QU_INIT(&invoke->refMap);
}


/*<
 * Function:    EROP_freeLopsInvoke
 *
 * Description: Set call back function to release LOPS layer entities 
 *              when an invocation is done in EROP layer.
 *
 * Arguments:   Call back function.
 *
 * Returns:     None.
 *
>*/

Void
EROP_freeLopsInvoke(Void (*lops_freeInvoke)(InvokeInfo *))
{
   erop_freeLopsInvoke = lops_freeInvoke;
}


/*<
 * Function:    erop_relInvoke
 *
 * Description: Release invoke structure
 *
 * Arguments:   Pointer to invoke structure
 *
 * Returns:     None.
 *
>*/

Void
erop_relInvoke(InvokeInfo *invoke)
{
   erop_refFree(invoke); 		/* Free reference number */
   erop_freeLopsInvoke(invoke); 	/* Free LOPS entries     */
   erop_invokeInfoFree(invoke); 	/* Free invoke entries   */
}

