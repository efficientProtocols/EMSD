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
 * File name: ps_map.c
 *
 * Description: Provider Shell: SAP and InvokeID 
 *
 * Functions:
 *   LOPS_init (void)
 *   LOPS_addInvoke(EROP_InvokeDesc EROP_invokeDesc, Int sapDesc)
 *   LOPS_remInvoke(Int invokeId)
 *   LOPS_getInvoke(Int invokeId)
 *   LOPS_getInvokeIdForEROP_InvokeDesc(EROP_InvokeDesc EROP_invokeDesc)
 *   LOPS_addSap(ESRO_SapSel inSap)
 *   LOPS_remSap(Int sapDesc)
 *   LOPS_getSap(Int sapDesc)
 *   LOPS_lookSap(ESRO_SapSel sapSel)
 *   LOPS_sapForInvokeId(Int invokeId)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: ps_map.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
 */

#include "estd.h"
#include "eh.h"
#include "tm.h"
#include "du.h"

#include "esro.h"
#include "esro_sh.h"
#include "erop.h"
#include "imq.h"
#include "sch.h"

#include "target.h"

#include "local.h"
#include "extfuncs.h"

InvokeCB psInvokeCBMap[LOPQ_INVOKE_CBS];   /*Provider Shell Invoke Control Blk*/
SapCB psSapCBMap[LOPQ_SAPS];               /*Provider Shell SAP Control Block */

#ifdef TM_ENABLED
TM_ModuleCB *LOPS_modCB;
#endif

extern PSQ_register(Int, Int, Int (*lops_userIn)(), Int (*lops_cleanSap)());
extern lops_userIn(LOPQ_Primitive *prim, Int len);
extern lops_cleanSap();
extern Void lops_freeInvoke(EROP_InvokeDesc *invoke);


/*<
 * Function:    LOPS_init
 *
 * Description: Initialize the Provider Shell
 *
 * Arguments:   None.
 *
 * Returns:     0 is successful, a negative error value if unsuccessful.
 *
>*/

PUBLIC SuccFail
LOPS_init (void)
{
    InvokeCB *indexInvokeCB;
    SapCB *indexSap;

    static Bool virgin = TRUE;
    
    if ( !virgin ) {
	return 0;
    }
    virgin = FALSE;

#ifdef TM_ENABLED
    if ( ! TM_OPEN(LOPS_modCB, "LOPS_") ) {
     	EH_problem("LOPS_init: TM_open LOPS_ failed");
    }
#endif

    PSQ_init();   	/* PSQ_ initialize IMQ_ */

    for (indexInvokeCB=psInvokeCBMap; indexInvokeCB <= ENDOF(psInvokeCBMap); 
	 ++indexInvokeCB) {
	QU_init((QU_Element *)indexInvokeCB);
	indexInvokeCB->state = INACTIVE_STATE;
	indexInvokeCB->sapCB = (SapCB *)0;
    }

    for (indexSap=psSapCBMap; indexSap <= ENDOF(psSapCBMap); ++indexSap) {
	QU_init((QU_Element *)&indexSap->invokeQuHead);
	QU_init((QU_Element *)&indexSap->evQuHead);
	indexSap->state = INACTIVE_STATE;
    }

    PSQ_register(LOPQ_A_BASE, BAD_LOPQ_ACTION, lops_userIn, lops_cleanSap);

    EROP_freeLopsInvoke(lops_freeInvoke);
    
    return ( SUCCESS );
}



/*<
 * Function:    LOPS_addInvoke.
 *
 * Description: Add Invoke ID.
 *
 * Arguments:   Invoke descriptor, SAP descriptor. 
 *
 * Returns:     Invoke ID if successful, NULL if unsuccessful.
 * 
>*/

LOCAL ESRO_InvokeId
LOPS_addInvoke(EROP_InvokeDesc EROP_invokeDesc, Int sapDesc)
{
    InvokeCB *indexInvoke;
    ESRO_InvokeId invokeId;

    for (indexInvoke=psInvokeCBMap; 		/* Find empty invoke entry */
	 indexInvoke <= ENDOF(psInvokeCBMap); 
	 ++indexInvoke) {

	if (indexInvoke->EROP_invokeDesc == (EROP_InvokeDesc) 0) {

	    indexInvoke->EROP_invokeDesc = EROP_invokeDesc;
	    indexInvoke->sapCB           = LOPS_getSap(sapDesc);

	    invokeId = indexInvoke - psInvokeCBMap;
#ifdef FUTURE
	    EROP_storeInvokeDec(EROP_invokeDesc, invokeId);
#endif

            TM_TRACE((LOPS_modCB, 0x1, 
		     "LOPS_addInvoke: Invoke added =0x%x\n", invokeId));

	    return ( invokeId );
	}
    }
    EH_problem("LOPS_addInvoke: no more space in invoke control block");
    return ( -1 );
}



/*<
 * Function:    LOPS_remInvoke.
 *
 * Description: Remove Invoke ID.
 *
 * Arguments:   Invoke ID.
 *
 * Returns:     None.
 *
>*/

LOCAL Void
LOPS_remInvoke(Int invokeId)
{
    InvokeCB *invokePtr;

    if ( invokeId < 0 || invokeId >= DIMOF(psInvokeCBMap) ) {
	EH_problem("LOPS_remInvoke: invalid invoke ID");
	return;
    }

    invokePtr = &(psInvokeCBMap[invokeId]);
    invokePtr->EROP_invokeDesc = (EROP_InvokeDesc) 0;

}


/*<
 * Function:    lops_freeInvoke
 *
 * Description: Free Invoke
 *
 * Arguments:   Invoke discriptor
 *
 * Returns:     None.
 *
>*/


LOCAL Void
lops_freeInvoke(EROP_InvokeDesc *invoke)
{
    InvokeCB *indexInvoke;

    TM_TRACE((LOPS_modCB, 0x1, 
	     "lops_freeInvoke: input paramter (invoke)=0x%x\n", invoke));

    for (indexInvoke=psInvokeCBMap; 			/* Find invoke entry */
	 indexInvoke <= ENDOF(psInvokeCBMap); 
	 ++indexInvoke) {

	if (indexInvoke->EROP_invokeDesc == (EROP_InvokeDesc) invoke) {

    	    TM_TRACE((LOPS_modCB, 0x1, 
	     	     "lops_freeInvoke:  Matched ==> Released invoke=0x%x\n", 
		     invoke));

	    indexInvoke->EROP_invokeDesc = 0;
    	    indexInvoke->state 		 = INACTIVE_STATE;
	}
    }
}


/*<
 * Function:    LOPS_getInvoke.
 *
 * Description: Get Invoke ID info.
 *
 * Arguments:   Invoke ID.
 *
 * Returns:     Pointer to Invoke ID info.
 *
>*/

LOCAL InvokeCB *
LOPS_getInvoke(Int invokeId)
{
    if ( invokeId < 0 || invokeId >= DIMOF(psInvokeCBMap) ) {
	EH_problem("LOPS_getInvoke: invalid invoke ID");
	return ( (InvokeCB *) 0);
    }
    return ( &(psInvokeCBMap[invokeId]) );	/* invoke id info */
}



/*<
 * Function:    LOPS_getInvokeIdForEROP_InvokeDesc
 *
 * Description: Get Invoke ID for a given Invoke descriptor.
 *
 * Arguments:   Invoke descriptor.
 *
 * Returns:  	A sapDesc if one does exist. -1 otherwise.
 *
 * 
>*/

LOCAL Int
LOPS_getInvokeIdForEROP_InvokeDesc(EROP_InvokeDesc EROP_invokeDesc)
{
    InvokeCB *indexInvokeCB;
    char errbuf[1024];

    for (indexInvokeCB = psInvokeCBMap; 
	 indexInvokeCB <= ENDOF(psInvokeCBMap); 
	 ++indexInvokeCB) {

	if ( indexInvokeCB->EROP_invokeDesc ==  EROP_invokeDesc ) {
	    /* Found it */
	    break;
	}
    }

    if ( indexInvokeCB > ENDOF(psInvokeCBMap) ) {
	sprintf(errbuf, 
		"LOPS_getInvokeIdForEROP_InvokeDesc: no invoke ID found\n"
		"                                    invokeDesc=0x%lx\n", 
		(unsigned long)EROP_invokeDesc);
	EH_problem(errbuf);

	return ( -1 );
    } else {

	return ( indexInvokeCB - psInvokeCBMap );
    }
}



/*<
 * Function:    LOPS_addSap
 *
 * Description:	Add SAP
 *
 * Arguments:	Sap selector, functional unit (2-way ro 3-way)
 *
 * Returns:
 *  	A nonnegative integer that can be used as a sap descriptor,
 *      or FAIL, if something went wrong.
 * 
>*/
LOCAL Int
LOPS_addSap(ESRO_SapSel inSap, ESRO_FunctionalUnit functionalUnit)
{
    SapCB *indexSap;

    for (indexSap = psSapCBMap; indexSap <= ENDOF(psSapCBMap); ++indexSap) {
	if (indexSap->sapSel == 0) {
	    break;
	}
    }
    if (indexSap <= ENDOF(psSapCBMap)) {

#ifdef FUTURE
	/* NOTYET, I don't like this. Why bother */
	if ( erop_getSap(inSap) == (erop_SapInfo *) 0 ) {
	    /* So this sap does not already exist,
	     * and we can bind it as a new SAP.
	     */
#else
	if ( TRUE ) {
#endif
	    if ( LOPS_sapBind(inSap, functionalUnit) != 0 ) {
                TM_TRACE((LOPS_modCB, 0x1, 
			 "LOPS_addSap: SAP bind failed"));
		return ( -1 );
	    }
	    indexSap->sapSel  = inSap;
	    indexSap->sapDesc = indexSap - psSapCBMap;

            TM_TRACE((LOPS_modCB, 0x1, 
                      "LOPS_addSap: SAP added: sapSel=%d  sapDesc=%d \n", 
                      indexSap->sapSel, indexSap->sapDesc));
	    /*
	     * If necassary Initialize everything else 
	     */
	    return ( indexSap - psSapCBMap );
	}
    }
    return ( -1 );
}



/*<
 * Function:    LOPS_remSap
 *
 * Description: Remove SAP
 *
 * Arguments:	Sap descriptor
 *
 * Returns:	0 if successful, a negative value otherwise.
 *
 * 
>*/

LOCAL SuccFail
LOPS_remSap(Int sapDesc)
{
    static int imqSimuCompileSw = 1;
    char errbuf[1024];

    if ( sapDesc < 0 || sapDesc >= DIMOF(psSapCBMap) ) {
	EH_problem("LOPS_remSap: invalid SAP descriptor");
	return ( FAIL );
    }
    if ( psSapCBMap[sapDesc].sapSel == 0 ) {
	sprintf(errbuf, 
		"LOPS_remSap: no SAP selector for the given SAP descriptor "
		"(it's OK)\n               sapDesc=%d\n", 
		sapDesc);
	EH_problem(errbuf);
	return ( FAIL );
    }

    if ( LOPS_sapUnbind(psSapCBMap[sapDesc].sapSel) ) {
	EH_problem("LOPS_remSap: SAP unbind failed");
	return ( FAIL );
    }

    psSapCBMap[sapDesc].sapSel = 0;

    SCH_allTaskDelete(SCH_PSEUDO_EVENT, (Void *)PSQ_procActionQu, 
		      psSapCBMap[sapDesc].primQu);

#ifndef IMQ_SIMU
    if (imqSimuCompileSw) {
        TM_TRACE((LOPS_modCB, 0x1, 
        	"\n**** provider_shell compiled <<<WITHOUT IMQ_SIMU>>> *****\n"));
	imqSimuCompileSw = 0;
    }
    IMQ_blockQuRemove(psSapCBMap[sapDesc].primQu->actionQu);
    IMQ_blockQuRemove(psSapCBMap[sapDesc].primQu->eventQu);
#else
    if (imqSimuCompileSw) {
        TM_TRACE((LOPS_modCB, 0x1, 
    		"\n**** provider_shell compiled <<<WITH IMQ_SIMU>>> *****\n"));
	imqSimuCompileSw = 0;
    }
#endif

    PSQ_primQuRemove(psSapCBMap[sapDesc].primQu);

    return ( SUCCESS );
}


/*<
 * Function:    LOPS_getSap
 *
 * Description: Get pointer to SAP info
 *
 * Arguments:   SAP descriptor (index)
 *
 * Returns:	SAP Control Block
 *
 * 
>*/
LOCAL SapCB *
LOPS_getSap(Int sapDesc)
{

    if ( sapDesc < 0 || sapDesc >= DIMOF(psSapCBMap) ) {
	EH_problem("LOPS_getSap: invalid SAP description");
	return ( (SapCB *) 0);
    }
    return ( &psSapCBMap[sapDesc] );
}


/*<
 * Function:    LOPS_lookSap
 *
 * Description: Get SAP selector and return the LOPS SAP descriptor (offset)
 *
 * Arguments:   SAP selector
 *
 * Returns:     SAP descriptor if exists, -1 otherwise.
 *
>*/

Int
LOPS_lookSap(ESRO_SapSel sapSel)
{
    SapCB *indexSap;
    char errbuf[1024];

    for (indexSap=psSapCBMap; indexSap <= ENDOF(psSapCBMap); ++indexSap) {
	if ( indexSap->sapSel ==  sapSel ) {
	    /* So, the sap is active */
	    break;
	}
    }
    if ( indexSap > ENDOF(psSapCBMap) ) {
	sprintf(errbuf, "LOPS_lookSap: SAP not active, SapSel=%d\n", sapSel);
	EH_problem(errbuf);
	return ( -1 );
    } else {
	return ( indexSap - psSapCBMap );
    }
}



/*<
 * Function:    LOPS_sapForInvokeId
 *
 * Description: Get Invoke ID and return the corresponding SAP.
 *
 * Arguments:   Invoke ID.
 *
 * Returns:     SAP descriptor.
 *
>*/

LOCAL Int
LOPS_sapForInvokeId(Int invokeId)
{
    return (psInvokeCBMap[invokeId].sapCB->sapDesc);
}


/*<
 * Function:    lops_isLopsInvokeCBFull
 *
 * Description: Is LOPS invoke control block full?
 *
 * Arguments:   None. 
 *
 * Returns:     0 if not full, -1 if full.
 * 
>*/

LOCAL SuccFail
lops_isLopsInvokeCBFull()
{
    InvokeCB *indexInvoke;

    for (indexInvoke = psInvokeCBMap; 
	 indexInvoke <= ENDOF(psInvokeCBMap); ++indexInvoke) {
	if (indexInvoke->EROP_invokeDesc == (EROP_InvokeDesc) 0) {
	    return ( SUCCESS );
	}
    }
#ifdef TM_ENABLED
    EH_problem("LOPS_isLopsInvokeCBFull: no more space in invoke control block");
#endif

    return ( FAIL );
}


