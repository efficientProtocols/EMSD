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

#include "estd.h"
#include "queue.h"
#include "eh.h"

typedef struct
{
    QU_ELEMENT;
    
    OS_Uint32	    expiryTime;
    void *	    hUserData1;
    void *	    hUserData2;
    ReturnCode	 (* pfHandler)(void * hTimer,
			       void * hUserData1,
			       void * hUserData2);
    unsigned char   localData[1];
} Timer;

static QU_Head	    timerQHead = QU_INITIALIZE(timerQHead);


/* Now, get the portation-specific functions */
#include "port.h"



static OS_Uint16    localDataSize = LOCAL_DATA_SIZE;



void
TMR_init(OS_Uint16 numberOfTimers, OS_Uint16 millisecondsPerTick)
{
    tmr_portationInit(numberOfTimers, millisecondsPerTick);
}


ReturnCode
TMR_start(OS_Uint32 milliseconds,
	    void * hUserData1,
	    void * hUserData2,
	    ReturnCode (* pfHandler)(void * hTimer,
				     void * hUserData1,
				     void * hUserData2),
	    void ** phTimer)
{
    ReturnCode 		rc;
    Timer * 		pNewTimer;
    Timer * 		pTimer;
    OS_Uint32		timeNow;

    /* Determine the current time */
    if ((rc = tmr_currentTime(&timeNow)) != Success)
    {
	return FAIL_RC(rc, ("TMR_start: get current date"));
    }
    
    /* Allocate a timer structure */
    if ((pNewTimer = tmr_obtain()) == NULL)
    {
	return FAIL_RC(ResourceError, ("TMR_start: alloc"));
    }
    
    /* Initialize the timer fields */
    QU_INIT(pNewTimer);
    pNewTimer->expiryTime = timeNow + milliseconds;
    pNewTimer->hUserData1 = hUserData1;
    pNewTimer->hUserData2 = hUserData2;
    pNewTimer->pfHandler = pfHandler;
    
    /*
     * Find the proper location in the timer queue for this timer.
     * The timer queue is time-ordered.
     */
    OS_EnterAtomic();		/* yes, it _really_ needs to be HERE! */
    for (pTimer = QU_FIRST(&timerQHead);
	 ! QU_EQUAL(pTimer, &timerQHead);
	 pTimer = QU_NEXT(pTimer))
    {
	/* Have we reached a later time? */
	if (pTimer->expiryTime > pNewTimer->expiryTime)
	{
	    /* Yes.  This is the element that we insert before */
	    QU_INSERT(pNewTimer, pTimer);
	    
	    /* We're done. */
	    break;
	}
    }
    
    /* Did we find someplace in the middle of the queue for it? */
    if (QU_EQUAL(pTimer, &timerQHead))
    {
	/* No.  Insert it at the end. */
	QU_INSERT(pNewTimer, &timerQHead);
    }
    OS_LeaveAtomic();		/* don't leave atomic section until HERE! */
    
    /* Give 'em what they came for */
    if (phTimer != NULL)
    {
	*phTimer = pNewTimer;
    }
    
    return Success;
}


void
TMR_stop(void * hTimer)
{
    Timer *		pTimer = hTimer;

    /* Remove the specified timer entry from the queue */
    OS_EnterAtomic();
    QU_REMOVE(pTimer);
    OS_LeaveAtomic();

    /* Free it. */
    tmr_dispose(pTimer);
}


ReturnCode
TMR_processQueue(OS_Boolean * pProcessedSomething)
{
    OS_Uint32		timeNow;
    ReturnCode		rc;
    Timer *		pTimer;
	
    /* Get the current time */
    if ((rc = tmr_currentTime(&timeNow)) != Success)
    {
	return FAIL_RC(rc, ("TMR_processQueue: get current time"));
    }
    
    /* While there are timer elements that need processing... */
    OS_EnterAtomic();
    for (pTimer = QU_FIRST(&timerQHead);
	 ! QU_EQUAL(pTimer, &timerQHead);
	 pTimer = QU_FIRST(&timerQHead))
    {
	/* Is it time to process this element yet? */
	if (pTimer->expiryTime <= timeNow)
	{
	    /* Yup.  Do it.  First, remove element from the queue. */
	    QU_REMOVE(pTimer);
	    OS_LeaveAtomic();
	    
	    /*
	     * Indicate that we found something to process.
	     *
	     * NOTE: This variable is not modified if nothing is
	     *       processed.  This allows the caller to find out
	     *       if any of a number of events (timers being only
	     *       one event type) have occured.
	     */
	    if (pProcessedSomething != NULL)
	    {
		*pProcessedSomething = TRUE;
	    }

	    /*
	     * Now, call the handler.  If we're emulating the original OCP TMR
	     * module, there's only one parameter.  Otherwise, there's three
	     * parameters.
	     */
	    if (pTimer->hUserData1 == &timerQHead)
	    {
		(void) (* pTimer->pfHandler)(pTimer->localData,
					     NULL, /* dummy arguments */
					     NULL);
	    }
	    else if ((rc =
		      (* pTimer->pfHandler)(pTimer,
					    pTimer->hUserData1,
					    pTimer->hUserData2)) !=
		     Success)
	    {
		/*
		 * This is bad.  The handler had an error.  We'll
		 * just return the error to the caller, and free
		 * this queue element so we don't loop here forever.
		 */
		tmr_dispose(pTimer);
		return FAIL_RC(rc, ("TMR_processQueue: call handler"));
	    }
	    
	    /* Dispose of the timer element, since we're done with it. */
	    tmr_dispose(pTimer);

	    /* Enter the atomic secion again. */
	    OS_EnterAtomic();
	}
	else
	{
	    /* No more timers to handle right now. */
	    break;
	}

	OS_LeaveAtomic();
    }
    
    return Success;
}



/*
 * The following functions provide compatibility with the original OCP module.
 */

void
TMR_poll(void)
{
    (void) TMR_processQueue(NULL);
}


void *
TMR_create(LgInt milliseconds, Int (*pfHandler) (/* ??? */))
{
    ReturnCode		rc;
    Timer *		pTimer;

    if ((rc = TMR_start((OS_Uint32) milliseconds, NULL, NULL,
			(ReturnCode (*)(void * hTimer,
					void * hUserData1,
					void * hUserData2)) pfHandler,
			(void **) &pTimer)) != Success)
    {
	EH_fatal("TMR_create: Could not start timer.\n");
    }

    /* We need to be able to figure out if this is an emulation call */
    pTimer->hUserData1 = &timerQHead;

    return pTimer;
}

void
TMR_cancel(void * hTimer)
{
    TMR_stop(hTimer);
}


void
TMR_setLocalDataSize(OS_Uint16 size)
{
    localDataSize = size;
}

OS_Uint16
TMR_getLocalDataSize(void)
{
    return localDataSize;
}


void *
TMR_getData(void * hTimer)
{
    Timer *		pTimer = hTimer;

    return pTimer->localData;
}


void *
TMR_getDesc(void * pLocalData)
{
    Timer *		pTimer;
    
    return ((void *)
	    ((unsigned char *) pLocalData -
	     ((unsigned char *) pTimer->localData -
	      (unsigned char *) pTimer)));
}


OS_Uint32
TMR_diff(OS_Uint32 time1, OS_Uint32 time2)
{
    return (time1 - time2);
}
