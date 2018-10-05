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
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)du.c	1.2    Released: 1/13/88";
#endif /*}*/

#include "eh.h"
#include "estd.h"
#include "queue.h"
#include "du.h"
#include "target.h"

static DU_Elem *viewFreeList;	/* List of Available Views */



/*<
 * Function:
 * Description:
 *  Data Unit allocation for desperat users.
 *  If NON is available, we LOOP until one becomes available.
 *  This facility should be used only when we can rely on
 *  something else being able to free a view while we are looping.
 *
 *
>*/
PUBLIC DU_View
DU_get(pool, len)
DU_Pool *pool;
int len;
{
    DU_View view;

    while ((view = DU_alloc(pool, len)) == (DU_View) 0) {
	;
    }
    return ( view );
}



/*<
 * Function:
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
DU_alloc(pool, nuOfBytes)
DU_Pool *pool;
Int nuOfBytes;
{
    DU_View view = (DU_View) 0;
    DU_BufInfo *bufInfo;

    if (nuOfBytes > pool->size) {
	return((DU_View) 0);
    }

    ENTER();
    if ((bufInfo = pool->bufFreeList) != (DU_BufInfo *) 0) {
	if ((view = viewFreeList) != (DU_View) NULL) {
	    /* POP one off the viewFreeList */
	    viewFreeList = view->next;
	    QU_init(view);
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
    LEAVE();
    return ( view );
}



/*<
 * Function:
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
DU_free(view)
DU_View view;
{
    DU_BufInfo *bufInfo;
    DU_Pool *pool;

    ENTER();
    bufInfo = view->bufInfo;
    pool = bufInfo->un.pool;
    if (--bufInfo->refcnt == 0) {
	/* PUSH this bufInfo on bufFreeList */
	bufInfo->un.next = pool->bufFreeList;
	pool->bufFreeList = bufInfo;
	pool->count++;
    }
    /* PUSH this view on viewFreeList */
    view->next = viewFreeList;
    viewFreeList = view;
    LEAVE();
}



/*<
 * Function:
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
DU_link(view)
DU_View view;
{
    DU_View newView;
    DU_Pool *pool;

    pool = view->bufInfo->un.pool;
    ENTER();
    if ((newView = viewFreeList) != (DU_View) 0) {
	/* POP one off the viewFreeList */
	viewFreeList = newView->next;
	QU_init(newView);
	/* Initialize the user specific data */

	newView->addr = view->addr;
	newView->bufInfo = view->bufInfo;
	newView->bufInfo->refcnt++;
    }
    LEAVE();
    return ( newView );
}



/*<
 * Function:
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
DU_buildPool(bufSize, nuOfBufs, nuOfViews)
Int bufSize;
Int nuOfBufs;
Int nuOfViews;
{
    DU_View view;
    DU_BufInfo *bufInfo;
    Int i;
    DU_Pool *pool;

    pool = (DU_Pool *) SF_memGet(sizeof(*pool));
    if (pool == (DU_Pool *) 0) {
	EH_fatal(ZPTR);
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
	    EH_fatal(ZPTR);
	}
	bufInfo->refcnt = 0;

	/* PUSH this on top of bufFreeList */
	bufInfo->un.next = pool->bufFreeList;
	pool->bufFreeList = bufInfo;

	pool->count++;
    }
    for (i=0;  i < nuOfViews; ++i) {
	view = (DU_View) SF_memGet(sizeof(*view));
	if (view == (DU_View) 0) {
	    EH_fatal(ZPTR);
	}
	/* PUSH this on top of viewFreeList */
	view->next = viewFreeList;
	viewFreeList = view;
    }
    return ( pool );
}

/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC Ptr
DU_vToUinfo(view)
DU_View view;
{
    return ( (Ptr) view->uInfo.d );
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC DU_View
DU_uInfoToV(userInfo)
Ptr userInfo;
{
    /* All of this can be reduced to a simple addition */
    static DU_Elem elem;
    Int offset;

    offset = (Char *)&elem - (Char *)elem.uInfo.d;

    return ( (DU_View) ((Char *)userInfo + offset) );
}
