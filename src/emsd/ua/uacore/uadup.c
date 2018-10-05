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


#include "estd.h"
#include "du.h"
#include "tm.h"
#include "eh.h"
#include "buf.h"
#include "uacore.h"
#include "esro_cb.h"


/*
 * ua_isDuplicate()
 *
 * Determine if an operation is a duplicate of a previous operation.
 * We only look at operations which have operation values of 32 or
 * greater.
 *
 * Parameters:
 *
 *     hView --
 *         DU View containing the incoming PDU.
 *
 *     operationValue --
 *         The operation value of the current operation.
 *
 * Returns:
 *
 *     TRUE if the operation is a duplicate; FALSE otherwise.
 *
 *     Note that the operation is, by definition, NOT a duplicate, if
 *     the operation value is less than 32.
 *
 * Side Effects:
 *
 *     The duplication detection octet is stripped from the beginning
 *     of the buffer.
 */
OS_Boolean
ua_isDuplicate(DU_View hView,
	       ESRO_OperationValue operationValue)
{
    DupData *	    pDupData;
    unsigned char * pDuplicate;
    unsigned char   operationInstanceId;

    /* Point to the Duplication Detection data structure */
    pDupData = NVM_getMemPointer(ua_globals.hDupData, NVM_FIRST);

    /* Is the operation value one which we need to concern ourselves with? */
    if (operationValue < 32)
    {
	/* Nope.  There's no duplicate detection octet on this operation. */
	return FALSE;
    }

    /* Get the instance id */
    operationInstanceId = *(DU_data(hView));

    /* Strip the operation instance id off of the beginning of the PDU */
    DU_strip(hView, 1);

    /*
     * We select a bit in the bit mask corresponding to a particular
     * operation instance id by first determining which octet of the
     * mask it's in, and then selecting a particular bit.
     */
    pDuplicate = pDupData->duplicate + (operationInstanceId / 8);
    operationInstanceId = 1 << (operationInstanceId % 8);

    /* Is this PDU a duplicate? */
    if ((*pDuplicate & operationInstanceId) != 0)
    {
#ifdef NOTYET		/* return TRUE once MTS supports dup detection */
	return TRUE;
#else
	return FALSE;
#endif
    }

    /* Set the bit for this operation to avoid later duplicates */
    *pDuplicate |= operationInstanceId;

    /*
     * Find the bit for the operation which is half way through the
     * window ((operationInstanceId + 128) % 128).  Since there are 8
     * bits per octet and 32 octets to cover the entire range of 256
     * instance id's, we can just move forward or backward (whichever
     * applies) by 16 bytes and clear the bit at the same bit offset.
     */
    if (pDuplicate >= pDupData->duplicate + 16)
    {
	pDuplicate -= 16;
    }
    else
    {
	pDuplicate += 16;
    }

    /* Clear the bit half way around the window so it can be reused */
    *pDuplicate &= ~operationInstanceId;

    /* Make sure our copy of the Duplicate Detection data is permanent */
    NVM_sync();

    return FALSE;
}


/*
 * ua_addDuplicateDetection()
 *
 * Add an octet at the beginning of an Invoke PDU to allow duplicate
 * invokations to be detected.  The duplicate detection octet is only
 * added if the operation value is at least 32.
 *
 * NOTE:
 *
 *     The current EMSD specification says that SUBMIT.requests are to
 *     use duplicate detection.  This is really unnecessary, as these
 *     requests are sent on the 3-way SAP, meaning that duplicates
 *     will never occur.  For this reason, this function simply
 *     generates a new duplicate detection octet for each PDU, and
 *     does not attempt to keep track of each message sent.
 *
 * Parameters:
 *
 *     hView --
 *         Buffer containing the Invoke PDU to be sent
 *
 *     operationValue --
 *         Operation value for this operation
 *
 * Returns:
 *
 *     Nothing
 */
void
ua_addDuplicateDetection(DU_View hView,
			 ESRO_OperationValue operationValue)
{
    static unsigned char  operationInstanceId = 0;

    if (operationValue < 32)
    {
	/* Don't add operation instance id for these operation values */
	return;
    }
    
    /* Make room at the beginning of the buffer for the dup detection octet */
    DU_prepend(hView, 1);

    /* Add the duplication detection octet */
    *DU_data(hView) = operationInstanceId++;
}
