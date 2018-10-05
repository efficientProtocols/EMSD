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
 * File name: sf_qu.c
 *
 * Description: System facilities: Queue processing.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sf_qu.c,v 1.16 1996/11/21 22:25:51 kamran Exp $";
#endif /*}*/

#include "estd.h"
#include "sf.h"
#include "os.h"
#include "queue.h"

/* Disable stack checking as these functions are called at interrupt time */
#pragma check_stack( off )


/*<
 * Function:    SF_quInsert
 *
 * Description: Insert element after head (non-preemtable), 
 *              i.e. add to the end of queue (FIFO)
 *
 *	SuccFail SF_quInsert(QU_Head *head, QU_Elem *elem);
 *		SUCCESS: (0) if element inserted  
 *		FAIL: (-1) if insertion failed
 *		SF_quInsert, inserts elem after the last element of head.
 *
 * Arguments:   Head of the queue, element to be inserted.
 *
 * Returns:	0 if successful, a negative error value otherwise.
 *
>*/

Int
SF_quInsert(QU_Head *head, QU_Element *elem)
{
    Int retval = 0;

    OS_EnterAtomic();

#ifndef FAST
    if (elem == (QU_Element *)NULL  ||  (QU_Head *)head->first == (QU_Head *)NULL)
        retval = -2;
    else
#endif
	QU_INSERT (elem, head);

    OS_LeaveAtomic();

    return retval;

} /* SF_quInsert */



/*<
 * Function:    SF_quRemove
 *
 * Description: Remove the head of the line (FIFO) (non-preemtable), 
 *              i.e. head's prev element.
 *
 *	SuccFail SF_quRemove(QU_Head *head, QU_Element **elemPtr);
 *		SUCCESS: (0) if head was not empty.
 *		FAIL: (-1) if head was empty.
 *		*elemPtr is a pointer to the obtained QU_Element,
 *		if head was not empty.
 *		SF_quRemove, removes the first element of head.
 *
 * Arguments:   Head of the queue, address of element (outgoing argument).
 *
 * Returns:	0 if successful, a negative error value otherwise.
 *
>*/


Int
SF_quRemove(QU_Head *head, QU_Element **elemPtr)
{
    Int retval = 0;

    OS_EnterAtomic();

    if (QU_EQUAL(QU_FIRST(head), head))
    	retval = -1;
#ifndef FAST
    else if (QU_FIRST(head) == (QU_Element *)NULL)
	retval = -2;
#endif
    else
	QU_remove(*elemPtr = QU_FIRST(head));

    OS_LeaveAtomic();

    return retval;

} /* SF_quRemove */



/*<
 * Function:    SF_quMove
 *
 * Description: Move a task from active queue to run queue.
 *
 * Arguments:   Head of the active and run queues, address of element 
 *
 * Returns:	0 if successful, a negative error value otherwise.
 *
>*/


Int
SF_quMove(QU_Head *head, QU_Element *elem)
{
    Int retval = 0;

    OS_EnterAtomic();

#ifndef FAST
    if (head == (QU_Head *)NULL || elem == (QU_Element *)NULL)
    	retval = -1;
    else
#endif
    {
	QU_REMOVE(elem); 		/* QU_move(elem, head); */
	QU_INSERT(elem, head);
    }

    OS_LeaveAtomic();

    return retval;

} /* SF_quMove */
