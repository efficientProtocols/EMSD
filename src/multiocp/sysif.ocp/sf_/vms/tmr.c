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
static char sccs[] = "@(#)tmr.c	1.2    Released: 1/31/88";
#endif /*}*/

#include "eh.h"
#include "estd.h"
#include "queue.h"
#include "tm.h"
#include "target.h"
#include "tmr.h"

typedef struct tmr_Info {
    struct tmr_Info *next;	/* next timer in linked list */
    struct tmr_Info *prev;	/* previous timer in linked list */
    LgInt time;			/* incr of time off previous entry */
    Int	(*func)();		/* user supplied timer response function */
#define DATASIZE 64
    /* Be very carefull about DATASIZE, IT must be largest 
     * data size used by any TMR service user.
     */
    Byte tmrData[DATASIZE];	/* Data to be supplied to func */
} tmr_Info;


typedef struct TmrQuHead {
    tmr_Info *first;
    tmr_Info *last;
} TmrQuHead;

/* Head of running timer queue elements */
TmrQuHead tmrQuHead = {(tmr_Info *)&tmrQuHead, (tmr_Info *)&tmrQuHead};	

/* Head of the list of available elements */
TmrQuHead availQuHead = {(tmr_Info *)&availQuHead, (tmr_Info *)&availQuHead};

PUBLIC LgUns tmr_freeCnt;		/* Free running Counter */

#define CLOCK_RATE 1000
static Int tmr_clockPeriod = CLOCK_RATE;  /* In millisecods */



/*<
 * Function:
 * Description:
 *	Initialize the timer package.
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC Void
TMR_init(nuOfTimers, clockPeriod)
Int nuOfTimers;
Int clockPeriod;
{
    Int i;
    tmr_Info *tq;

    QU_init(&tmrQuHead);
    QU_init(&availQuHead);
    for (i = 0; i < nuOfTimers; i++) {
	if ((tq = (tmr_Info *) SF_memGet(sizeof(*tq))) == (tmr_Info *) 0) {
	    EH_fatal("");
	}
	QU_init(tq);
	QU_insert(tq, &availQuHead);
    }
    tmr_freeCnt = 0;
    tmr_clockPeriod = clockPeriod;
}




/*<
 * Function:
 * Description:
 *  clock:  update timer package's idea of the current time.
 *  This routine is called on every hardware clock interrupt.
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC Void
TMR_clock()
{
    tmr_Info *tq;

    if ((tq = tmrQuHead.first) != (tmr_Info *) &tmrQuHead) {
	tq->time -= tmr_clockPeriod ;
    }
    tmr_freeCnt += tmr_clockPeriod;
}




/*<
 * Function:
 * Description:
 *  TMR_poll:  check timer queue for expired timer.
 *  This function executes the appropriate TIMER.response function
 *  for each expired timer. Expired timers contains a value less than
 *  or equal to zero in their time structure entry.
 *  This function should be called periodically at non-interrupt level
 *  to process expired timers.
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC Void
TMR_poll()
{
    tmr_Info *tq;

    while ((tq = tmrQuHead.first) != (tmr_Info *) &tmrQuHead &&
	    tq->time <= 0) {
	if (tq->next != (tmr_Info *) &tmrQuHead) {
	    tq->next->time += tq->time;
	}
	ENTER();
	QU_move(tq, &availQuHead);
	LEAVE();
	(*tq->func)(tq->tmrData);
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
>*/
PUBLIC TMR_Desc
TMR_create(time, func)
LgInt time;
Int (*func)();
{
    tmr_Info *newTimer;
    tmr_Info *tq;
    LgInt absTime;

    absTime = 0;
    for (tq = tmrQuHead.first; tq != (tmr_Info *) &tmrQuHead; tq = tq->next) {
	absTime += tq->time;
	if (time < absTime) {
	    break;
	}
    }

    if ((newTimer = availQuHead.first) == (tmr_Info *) &availQuHead) {
	EH_fatal(ZPTR);
    }
    ENTER();
    QU_move(newTimer, tq);
    if (tq == (tmr_Info *) &tmrQuHead) {
	/* So this is the only element in the queue */
	newTimer->time = time - absTime;
    } else {
	newTimer->time = time - absTime + tq->time;
	tq->time -= newTimer->time;
    }
    LEAVE();
    newTimer->func = func;

    return ( (TMR_Desc) newTimer );
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
PUBLIC Void
TMR_cancel(tmrDesc)
TMR_Desc tmrDesc;
{
    tmr_Info *tmrQuElem;

    tmrQuElem = (tmr_Info *) tmrDesc;

    if (tmrQuElem->next != (tmr_Info *) &tmrQuHead) {
	/* So this is not the only timer in the queue
	 * and therefor the next one must be adjusted.
	 */
	tmrQuElem->next->time += tmrQuElem->time;
    }
    ENTER();
    QU_move(tmrQuElem, &availQuHead);
    LEAVE();
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
TMR_getData(tmrDesc)
TMR_Desc tmrDesc;
{
    tmr_Info *tmrQuElem;

    return ( (Ptr) ((tmr_Info *) tmrDesc)->tmrData );
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
PUBLIC TMR_Desc
TMR_getDesc(tmrData)
Ptr tmrData;
{
    /* All of this can be reduced to a simple addition */
    static tmr_Info tmrQuElem;
    Int offset;

    offset = (Char *)&tmrQuElem - (Char *)tmrQuElem.tmrData;

    return ( (TMR_Desc) ((Char *)tmrData + offset) );
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
PUBLIC LgInt
TMR_diff(t1, t2)
LgUns t1;
LgUns t2;
{
    return (t1 - t2);
}
