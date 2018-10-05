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
 *	This Module implements a fixed size fifo as a Circular buffer.
 *	A write to the fifo when it is full results in an error.
 *	There is a WASTED element in this implementation.
 *      One could otherwise implement it by using an extra variable.
 *	I am open to suggestion for a more efficient implementation.
 *	Is there a way of implementing a FIFO based on a circular queue,
 *	without wasting an element and not not using an extar variable?
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)fifo.c	1.1    Released: 13 Mar 1990";
#endif /*}*/


/* #includes */
#include  "estd.h"
#include  "fifo.h"
 

PUBLIC Void
FIFO_init (ctrlBlk, begin, end)
FIFO_CtrlBlk *ctrlBlk;
FIFO_Element *begin;
FIFO_Element *end;
{
    ctrlBlk->begin = begin;
    ctrlBlk->end = end;
    ctrlBlk->out = ctrlBlk->in = begin;
}



/*<
 * Function:
 * Description:
 *	Increment and adjust inPtr, if same as outPtr then full so restore.
 *	Otherwise, Insert new item in.
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC SuccFail
FIFO_put (ctrlBlk, element)
FIFO_CtrlBlk  *ctrlBlk;
FIFO_Element  element;
{
    FIFO_Element *inOrig;

    inOrig = ctrlBlk->in;
    if ( ++ctrlBlk->in > ctrlBlk->end ) {
	ctrlBlk->in = ctrlBlk->begin;
    }
    if ( ctrlBlk->in == ctrlBlk->out ) {
	/* This means the fifo is full */
	ctrlBlk->in = inOrig;
	return ( FAIL );
    } else {
	/* So there is room, to put the new element */
	*inOrig = element;
	return ( SUCCESS );
    }
}



/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC FIFO_Element *
FIFO_get( ctrlBlk )
FIFO_CtrlBlk  *ctrlBlk;
{
    FIFO_Element *retVal;

    if ( ctrlBlk->in == ctrlBlk->out ) {
	/* This means the fifo is Empty */
	return ( (FIFO_Element *) 0 );
    } else {
	/* So there is something there */
	retVal =  ctrlBlk->out;
	if ( ++(ctrlBlk->out) > ctrlBlk->end ) {
	    ctrlBlk->out = ctrlBlk->begin;
	}
    }
    return ( retVal );
}


/*<
 * Function:
 * Description:
 *	Increment and adjust inPtr, if same as outPtr then full so restore.
 *	Otherwise, Insert new item in.
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Bool
FIFO_full( ctrlBlk )
FIFO_CtrlBlk  *ctrlBlk;
{
    FIFO_Element *inOrig;
    Bool retVal;

    inOrig = ctrlBlk->in++;
    if ( ctrlBlk->in > ctrlBlk->end ) {
	ctrlBlk->in = ctrlBlk->begin;
    }
    if ( ctrlBlk->in == ctrlBlk->out ) {
	/* This means the fifo is full */
	retVal = TRUE;
    } else {
	/* So there is room, to put a new element */
	retVal = FALSE;
    }
    ctrlBlk->in = inOrig;
    return ( retVal );
}
