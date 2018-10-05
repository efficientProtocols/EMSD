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

/*+
 * File:	seq.c
 *
 * Description: Sequence Processing.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: seq.c,v 1.9 1996/11/07 02:46:22 kamran Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "sf.h"
#include "queue.h"
#include "seq.h"


typedef struct SeqPoolInfo {
    QU_Head freeList;
    Ptr bufFirst;
    Ptr bufLast;
    Int sizeOfElem;
    Int nuOfElems;
} SeqPoolInfo;

/*<
 * Function:	poolCreate
 *
 * Description: Create pool.
 *
 * Arguments:	Size of pool element, number of elements.
 *
 * Returns:	Pool descriptor.
 *
>*/
SEQ_PoolDesc
SEQ_poolCreate(Int sizeOfElem, Int nuOfElems)
{
    SeqPoolInfo *seqPoolInfo;
    QU_Element *quElem;

    seqPoolInfo = (SeqPoolInfo *)SF_memObtain(sizeof(*seqPoolInfo));
    if (!seqPoolInfo) {
	EH_fatal("SEQ_poolCreate: SF_memObtain failed for pool creation");
    }

    seqPoolInfo->nuOfElems = nuOfElems;

    if (!nuOfElems) {
	/* then Dynamic memory allocation is to be used. */
	seqPoolInfo->sizeOfElem = sizeOfElem;
	return (SEQ_PoolDesc) seqPoolInfo;
    }
	
    /*
     * Create a Pool and inserts them in the free List
     */
    seqPoolInfo->bufFirst = SF_memObtain(sizeOfElem * nuOfElems);
    if (!seqPoolInfo->bufFirst) {
	EH_fatal("SEQ_poolCreate: SF_memObtain failed for pool elements\n");
    }
    seqPoolInfo->bufLast = (Ptr)
	    ((Byte *)seqPoolInfo->bufFirst + (sizeOfElem * (nuOfElems - 1)));

    QU_INIT(&seqPoolInfo->freeList);
    for (quElem =(QU_Element *)seqPoolInfo->bufFirst;
	     quElem <= (QU_Element *) seqPoolInfo->bufLast;
	     quElem = (QU_Element *) ((Byte *)quElem + sizeOfElem)) {
	QU_INIT(quElem);
	QU_INSERT(&seqPoolInfo->freeList, quElem);	
    }

    return (SEQ_PoolDesc) seqPoolInfo;
}

/*<
 * Function:	poolFree
 *
 * Description: Free pool
 *
 * Arguments:	Pool descriptor.
 *
 * Returns:	None.
 *
>*/
Void
SEQ_poolFree(SEQ_PoolDesc poolDesc)
{
    SeqPoolInfo *poolInfo;

    poolInfo = (SeqPoolInfo *)poolDesc;

    if (poolInfo->nuOfElems) {
	SF_memRelease(poolInfo->bufFirst);
    }
    SF_memRelease(poolInfo);
}


/*<
 * Function:	elemObtain.
 *
 * Description:	Obtain an element from pool.
 *
 * Arguments:	Pool descriptor.
 *
 * Returns:	Pointer to element.
 *
>*/
Ptr
SEQ_elemObtain(SEQ_PoolDesc poolDesc)
{
    SeqPoolInfo *poolInfo;
    QU_Element *retVal;

    poolInfo = (SeqPoolInfo *)poolDesc;
    
    /* Dynamic Memory Allocation */
    if (!poolInfo->nuOfElems) {
	retVal = (QU_Element *) SF_memObtain(poolInfo->sizeOfElem);
	if (!retVal) {
	    EH_fatal("SEQ_elemObtain: SF_memObtain failed for elem mem alloc");
	}
	QU_INIT(retVal);
	return ((Ptr) retVal );
    }

    /* Static Memory Allocation */	
    if (QU_EQUAL(QU_FIRST(&poolInfo->freeList), &poolInfo->freeList)) {
	EH_problem("SEQ_elemObtain: No Element in free list");
	return ((Ptr)0);
    }
    retVal = QU_FIRST(&poolInfo->freeList);
    QU_REMOVE((QU_Element *)retVal);
   
    return ((Ptr) retVal);
}


/*<
 * Function:	SEQ_elemRelease
 *
 * Description:	Release element.
 *
 * Arguments:	Pool descriptor, element to be released.
 *
 * Returns:	None.
 *
>*/
Void
SEQ_elemRelease(SEQ_PoolDesc poolDesc, Ptr quElem)
{
    SeqPoolInfo *poolInfo;

    poolInfo = (SeqPoolInfo *)poolDesc;

    /* Dynamic Memory Allocation */
    if (!poolInfo->nuOfElems) {
	SF_memRelease(quElem);
	return;
    } 

    QU_INIT(quElem);	/* For compatibility with SF_qu */
    QU_INSERT(&((SeqPoolInfo *)poolDesc)->freeList, quElem);
}
 
