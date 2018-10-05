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
 * File: keep.c
 *
 * Function:
 *   erop_pduSeqInit(PduSeq *p)    
 *   erop_relPdu(InvokeInfo *invokeInfo, PduSeq *pduSeq)
 *   erop_relAllPdu(InvokeInfo *invokeInfo)
 *   erop_pduRetrieve(InvokeInfo *invokeInfo, PduSeq *pduSeq)
 *   erop_pduKeep(InvokeInfo *invokeInfo, PduSeq *pduSeq, DU_View du)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: keep.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "local.h"
#include "extfuncs.h"


/*<
 * Function:    erop_pduInit
 *
 * Description: Initialize PDU queue. 
 *
 * Arguments:   Pointer to DU view.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

SuccFail
erop_pduSeqInit(PduSeq *p)    
{
    if (p) {
       QU_INIT(p);            
       return (SUCCESS);
    }
    return (FAIL);
}


/*<
 * Function:    erop_relPdu
 *
 * Description: The erop_relPdu deletes pdu data in the allocated buffers.
 *
 * Arguments:   Invoke information, PDU.
 *
 * Returns:     0 on successful completion, -1 on unsuccessful completion.
 *
>*/

SuccFail
erop_relPdu(InvokeInfo *invokeInfo, PduSeq *pduSeq)
{
    DU_View sdu;
#if 0	/* NOTYET */
    DU_View nextsdu;
#endif

#ifndef FAST
    if (invokeInfo == 0  ||  pduSeq == 0  ||  pduSeq->first == 0) {	
	EH_problem("erop_relPdu: called with invalid argument\n");
	return (FAIL);
    }
#endif

#if 0		/* NOTYET: The real problem is not masking interrupts at
			   queue insert time. This should be removed when
			   signal-based fix is verified */

    for (sdu = QU_FIRST(pduSeq); 
	 ! QU_EQUAL(sdu, pduSeq); sdu = nextsdu) {
	nextsdu = QU_NEXT(sdu);
	if (sdu == nextsdu) {
	    EH_problem("erop_relPdu: DU already released!\n");
	    return (FAIL);
	}
	QU_REMOVE(sdu);
#ifdef FUTURE
	_timeStamp = _duTimeGet(sdu);	
#endif
	TM_TRACE((DU_modCB, DU_MALLOC, "erop_relPdu: DU_free: sdu=0x%lx\n",
		 sdu));

	DU_free(sdu);
    }
#else
	sdu = QU_FIRST(pduSeq);
        if (! QU_EQUAL(sdu, (DU_View)pduSeq)) {
	    QU_REMOVE(sdu);
       	    if (sdu) {
	    	TM_TRACE((DU_modCB, DU_MALLOC, 
			 "erop_relPdu: DU_free: sdu=0x%lx\n", sdu));
	    	DU_free(sdu);
	    }
	}
#endif
    return (SUCCESS);
}



/*<
 * Function:    erop_relAllPdu
 *
 * Description: Deletes all data in all allocated buffers.
 *
 * Arguments:   Invoke information.
 *
 * Returns:     0 on successful completion, -1 on unsuccessful completion.
 *
>*/

SuccFail
erop_relAllPdu(InvokeInfo *invokeInfo)
{
    Int retVal = (SUCCESS);

#ifndef FAST
    if (invokeInfo == 0) {	
	EH_problem("erop_relAllPdu: called with invalid argument\n");
	return (FAIL);
    }
#endif

    if (erop_relPdu(invokeInfo, &invokeInfo->invokePduSeq) == FAIL) {
	EH_problem("erop_relAllPdu: erop_relPdu failed\n");
	retVal = FAIL;
    }
    if (erop_relPdu(invokeInfo, &invokeInfo->resultPduSeq) == FAIL) {
	EH_problem("erop_relAllPdu: erop_relPdu failed\n");
	retVal = FAIL;
    }

    return retVal;
}


/*<
 * Function:    erop_pduRetrieve
 *
 * Description:
 *   Right now with just one PDU type to be retrieved, the SEQ_ always only
 *   has one entry. If there was to be more, then the PUD type or some
 *   other indication must be passed to the function.
 *
 * Arguments: Invoke info, PDU seq.
 *
 * Returns:   Pointer to Data Unit view.
 *
>*/

DU_View 
erop_pduRetrieve(InvokeInfo *invokeInfo, PduSeq *pduSeq)
{
    DU_View sdu, du;

    for (sdu = QU_FIRST(pduSeq); ! QU_EQUAL(sdu, pduSeq); sdu = QU_NEXT(sdu)) {
	du = DU_link(sdu);
	return (du);
    }
    return ((DU_View) NULL);
}


/*<
 * Function:    erop_pduKeep
 *
 * Description: Store PDU (for future retrieval and checking against the future
 *              PDUs for duplication).
 *
 * Arguments:   Invoke info, pdu.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
>*/

Void
erop_pduKeep(InvokeInfo *invokeInfo, PduSeq *pduSeq, DU_View du)
{
    QU_INSERT(du, pduSeq);

#ifdef FUTURE
    if (erop_duTimeSet(du, TMR_getFreeCnt()) == FAIL) {
    	return (FAIL);
    } else {
    	return (SUCCESS);
    }
#endif
}

