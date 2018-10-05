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
 * File name: du.c
 *
 * Description: Data Unit
 *
-*/

/* 
 * Notes For Improvement:
 * #ifdef FAST and add range checking where appropriate.
 * Move size to buffInfo. If marked as sharable, deliver smaller
 * buffers from larger pools.
 */

/*
 * Author: Mohsen Banan.
 * History:
 *   
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: du.c,v 1.20 1996/11/07 02:46:34 kamran Exp $";
#endif /*}*/

#include "eh.h"
#include "estd.h"
#include "oe.h"
#include "sf.h"
#include "queue.h"
#include "du.h"
#include "tm.h"
#include "target.h"

static DU_Elem *viewFreeList;	/* List of Available Views */

#ifdef TM_ENABLED
LOCAL TM_ModuleCB *DU_modCB;
#define DU_MALLOC TM_BIT10
#endif

/*<
 * Function:
 *      DU_alloc
 * Description:
 * 	Allocate a view.
 * Arguments:
 *	pool: 	specifies which Stream Pool should the view be allocated from.
 *	nuOfBytes: The number of bytes to be allocated.
 * Returns:
 *	A DU_View is returned if one was available.
 *	The NULL pointer is returned if no view was available or
 *      nuOfBytes was invalid.
 *
>*/
PUBLIC DU_View
DU_alloc(DU_Pool *pool, Int nuOfBytes)
{
    DU_View view = (DU_View) 0;
    DU_BufInfo *bufInfo;

    TM_TRACE((DU_modCB, DU_MALLOC, 
             "+++++DU_alloc: nuOfBytes=%d    pool->count=%d  "
	     "bufFreeList=0x%lx  viewFreeList=0x%lx\n",
  	     nuOfBytes, pool->count, pool->bufFreeList, viewFreeList));

    if (nuOfBytes > pool->size) {
	EH_problem("DU_alloc: nuOfBytes greater than pool size\n");
	return((DU_View) 0);
    }

    OS_EnterAtomic();
    if ((bufInfo = pool->bufFreeList) != (DU_BufInfo *) 0) {
	if ((view = viewFreeList) != (DU_View) NULL) {
	    /* POP one off the viewFreeList */
	    viewFreeList = view->next;
	    QU_init((QU_Element *)view);
	    /* Initialize user specific fields */

	    pool->count--;
	    /* POP one of the bufFreeList */
	    pool->bufFreeList = bufInfo->un.next;

	    bufInfo->refcnt = 1;
	    bufInfo->un.pool = pool;	/* This Buffer should Know 
					 * which pool it belongs to.
					 */
	    bufInfo->end = (Byte *) bufInfo + sizeof(*bufInfo) + pool->size;

	    view->addr = bufInfo->end - nuOfBytes;
	    view->bufInfo = bufInfo;
	}
    }
    OS_LeaveAtomic();

    TM_TRACE((DU_modCB, DU_MALLOC, "          DU_alloc: return view=0x%lx\n", view));

    return ( view );
}



/*<
 * Function:
 *     	DU_free
 *
 * Description:
 *	Release a previuosly allocated view.
 *	Reference count in bufInfo is decremented.
 *	bufInfo (the buffer itself) is freed only if
 *	reference count goes to zero.
 *
 * Arguments:
 *	view: Stream that is to be freed.
 *
>*/
PUBLIC Void
DU_free(DU_View view)
{
    DU_BufInfo *bufInfo;
    DU_Pool *pool;

#ifndef FAST
    if (!view) {
	EH_problem("DU_free called with a NULL argument\n");
	return;
    }
    if (!view->bufInfo) {
	EH_problem("DU_free called with a NULL view->bufInfo\n");
	return;
    }
    if (!view->bufInfo->un.pool) {
	EH_problem("DU_free called with a NULL view->bufInfo->un.pool\n");
	return;
    }
#endif

    TM_TRACE((DU_modCB, DU_MALLOC, 
             "          DU_free:  view=0x%lx  refCnt=%d (before free)\n", 
             view, view->bufInfo->refcnt));

    OS_EnterAtomic();
    bufInfo = view->bufInfo;
/* #ifdef FAST */
    if (bufInfo->refcnt <= 0) {

#ifdef TM_ENABLED
    if (bufInfo->refcnt < 0) {
    	TM_trace(DU_modCB, DU_MALLOC, 
	     	"WARNING: DU_free: function called for DU with negative reference count: view=0x%x  refcnt=%d", view, bufInfo->refcnt);
    }
#endif
	bufInfo->refcnt = 0;
        OS_LeaveAtomic();
	return;
    }
/* #endif FAST */

    pool = bufInfo->un.pool;
    if (--bufInfo->refcnt == 0) {
	/* PUSH this bufInfo on bufFreeList */
	bufInfo->un.next = pool->bufFreeList;
	pool->bufFreeList = bufInfo;
	pool->count++;

        TM_TRACE((DU_modCB, DU_MALLOC, 
	         "-----DU_free:  view=0x%x  pool->count=%d\n", 
	         view, pool->count));
    }
    /* PUSH this view on viewFreeList */
    view->next = viewFreeList;
    viewFreeList = view;
    OS_LeaveAtomic();
}


/*<
 * Function:
 *     	DU_freeLast
 *
 * Description:
 *	Release last allocated view.
 *	Reference count in bufInfo should be 1 by this time.
 *	bufInfo (the buffer itself) is freed. 
 *
 * Arguments:
 *	view: Stream that is to be freed.
 *
 * Retrun: SUCCESS if ref count is 1, FAIL otherwise
 *
>*/
PUBLIC SuccFail
DU_freeLast(DU_View view)
{
    DU_BufInfo *bufInfo;
    Int retVal = (SUCCESS);

    TM_TRACE((DU_modCB, DU_MALLOC, 
	     "          DU_freeLast:  view=0x%lx  refCnt=%d\n", 
	     view, view->bufInfo->refcnt));

    OS_EnterAtomic();
    bufInfo = view->bufInfo;

    if (bufInfo->refcnt > 1) {

    	TM_TRACE((DU_modCB, DU_MALLOC, 
	     	"WARNING: DU_freeLast: function called for DU with "
		"reference count > 1: view=0x%lx  refcnt=%d", 
		view, bufInfo->refcnt));

	bufInfo->refcnt = 1;
	retVal = (FAIL);
    }

    DU_free(view);
    OS_LeaveAtomic();

    return retVal;
}

/*<
 * Function:
 *	DU_link
 *
 * Description:
 *	Allocate an additional Stream for an existing Stream.
 *	This allows the invoker to deal with an existing data buffer
 *	in more than one manner.
 *	reference count in bufInfo is incremented.
 *
 * Arguments:
 *	view: An existing view.
 *
 * Returns:
 *	A new view if allocation was possible.
 *	NULL if allocation failed.
 *
>*/
PUBLIC DU_View
DU_link(DU_View view)
{
    DU_View newView;
    DU_Pool *pool;

#ifndef FAST
    if (!view) {
	return NULL;
    }
#endif

    TM_TRACE((DU_modCB, DU_MALLOC, 
	     "          DU_link: start: view=0x%lx  refCnt=%d\n", 
	     view, view->bufInfo->refcnt));

    pool = view->bufInfo->un.pool;
    OS_EnterAtomic();
    if ((newView = viewFreeList) != (DU_View) 0) {
	/* POP one off the viewFreeList */
	viewFreeList = newView->next;
	QU_init((QU_Element *)newView);
	/* Initialize the user specific data */

	newView->addr    = view->addr;
	newView->bufInfo = view->bufInfo;
	newView->bufInfo->refcnt++;
    }
    OS_LeaveAtomic();

    TM_TRACE((DU_modCB, DU_MALLOC, 
	     "          DU_link: end: newView=0x%lx  refCnt=%d\n", 
	     newView, newView->bufInfo->refcnt));

    return ( newView );
}

/*<
 * Function:
 * 	DU_buildPool
 *
 * Description:
 *	Build a pool of views.
 *	Characteristics of the Pool to be created are 
 *	arguments of this function. 
 *	The Pool, the Buffers, and the Streams are all dynamically
 * 	allocated.
 *
 * Arguments:
 *	bufSize:  How big buffers of this pool are to be.
 *	nuOfBufs: Number of buffers to be allocated.
 *	nuOfViews: Number of views to be allocated.
 *
 * Returns:
 *	A pointer to Pool of views.
 *	NULL if allocation failed.
>*/
PUBLIC DU_Pool *
DU_buildPool(Int bufSize, Int nuOfBufs, Int nuOfViews)
{
    DU_View view;
    DU_BufInfo *bufInfo;
    Int i;
    DU_Pool *pool;

    static Bool virgin = TRUE;

    if ( !virgin ) {
	EH_problem("DU_buildPool: DU_buildPool is called more than one time\n");
	return 0;
    }
    virgin = FALSE;

    if (TM_OPEN(DU_modCB, "DU_") == NULL) {
        EH_problem("DU_buildPool: TM_open DU_ failed\n");
    }

    if ( (pool = (DU_Pool *) SF_memGet(sizeof(*pool))) == (DU_Pool *) 0) {
	EH_fatal("DU_buildPool: SF_memGet failed for pool allocation");
    }

    pool->count = 0;
    pool->size = bufSize;
    pool->bufFreeList = (DU_BufInfo *) 0;
    viewFreeList = (DU_Elem *) 0;
    /*
     * Allocate all the buffers and  put them in
     * bufFreeList.
     */
    for (i=0;  i < nuOfBufs; ++i) {
	bufInfo = (DU_BufInfo *) SF_memGet(sizeof(*bufInfo) + bufSize);
	if (bufInfo == (DU_BufInfo *) 0) {
	    EH_fatal("DU_buildPool: SF_memGet failed during buffer allocation");
	}
	bufInfo->refcnt = 0;

	/* PUSH this on top of bufFreeList */
	bufInfo->un.next  = pool->bufFreeList;
	pool->bufFreeList = bufInfo;

	pool->count++;
    }
    for (i=0;  i < nuOfViews; ++i) {
	view = (DU_View) SF_memGet(sizeof(*view));
	if (view == (DU_View) 0) {
	    EH_fatal("DU_buildPool: SF_memGet failed for DU view allocation");
	}
	/* PUSH this on top of viewFreeList */
	view->next = viewFreeList;
	viewFreeList = view;
    }
    return ( pool );
}

/*<
 * Function:
 *
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC Ptr
DU_vToUinfo(DU_View view)
{
    return ( (Ptr) view->uInfo.d );
}


/*<
 * Function:
 *
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC DU_View
DU_uInfoToV(Ptr userInfo)
{
    /* All of this can be reduced to a simple addition */
    static DU_Elem elem;
    Int offset;

    offset = (Char *)&elem - (Char *)elem.uInfo.d;

    return ( (DU_View) ((Char *)userInfo + offset) );
}


