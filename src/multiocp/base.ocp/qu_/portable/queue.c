/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*
 * Author: Derrell Lipman
 * History:
 *
 */

#include "queue.h"
#include "eh.h"

/*
 * Disable stack checking as these functions are called at interrupt time
 * by SF_QUINSERT()
 */
#pragma check_stack(off)

/*
 * Function:	QU_init()
 *
 * Description: Initialize a queue element, for later insertion onto a queue.
 */
void
QU_init(QU_Element * pQElement)
{
    pQElement->pNext = pQElement->pPrev = pQElement;
}


/*
 * Function:	QU_insert()
 *
 * Description: Insert element pointed to by pInsertThisElement before 
 * 		the element pointed to by pBeforeThisElement.  
 *		pInsertThisElement is assumed to be a stand-alone element; 
 *		no attempt is made to maintain its previous next/prev
 * 		pointers.
 *
 * Parameters:
 *
 * Returns:
 *
 */
void
#ifdef DEBUG_QU
QU_insert(QU_Element * pInsertThisElement, QU_Element * pBeforeThisElement, char *fileName, int lineNo)
#else
QU_insert(QU_Element * pInsertThisElement, QU_Element * pBeforeThisElement)
#endif
{
    QU_Element *	    pTempElement;

#ifndef FAST
    if ((pInsertThisElement->pPrev == pBeforeThisElement)
     || (pInsertThisElement->pNext == pBeforeThisElement)
     || (pBeforeThisElement->pPrev == pInsertThisElement)
     || (pBeforeThisElement->pNext == pInsertThisElement))
    {
#ifdef DEBUG_QU
	{
        extern FILE *eh_hFd;
	EH_problem ("Improper queue insertion, element already in queue");
	fprintf(eh_hFd, ", %s, %d\n", fileName, lineNo);
	}
#else
	EH_problem ("Improper queue insertion, element already in queue\n");
#endif
	return;
    }

#endif /* FAST */

    pBeforeThisElement->pPrev->pNext = pInsertThisElement;
    pTempElement = pInsertThisElement->pPrev;
    pTempElement->pNext = pBeforeThisElement;
    pInsertThisElement->pPrev = pBeforeThisElement->pPrev;
    pBeforeThisElement->pPrev = pTempElement;
}


/*
 * Function:	QU_remove()
 * 
 * Description: Remove the element pointed to by pRemoveMe from whatever 
 *		queue it's on, and re-initialize this element for later 
 *		re-insertion onto a queue.
 * Parameters:
 *
 * Returns:
 *
 */
void
QU_remove(QU_Element * pRemoveMe)
{
    /* remove myself from the list */
    pRemoveMe->pNext->pPrev = pRemoveMe->pPrev;
    pRemoveMe->pPrev->pNext = pRemoveMe->pNext;

    /* re-initialize myself */
    pRemoveMe->pNext = pRemoveMe->pPrev = pRemoveMe;
}
