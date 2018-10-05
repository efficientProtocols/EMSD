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

/* External system functions not declared in os.h but required by this portation */
unsigned alarm(unsigned sec);

/*
 * This variable contains the number of milliseconds between clock ticks.
 */
static OS_Uint16       tmr_millisecondsPerTick = 1000;

/*
 * Here, we have the time-of-day counter.  We'll use it to know how many ticks
 * have occured since we started up.  We really don't need it for this
 * portation.
 */
static OS_Uint32       tmr_timeOfDay = 0;

/*
 * Maintain a pointer to a function that we will call whenever a clock tick
 * occurs.
 */
static void	    (* tmr_pfTimerHandler)(void) = NULL;

/*
 * We're going to set our own signal handler for the ALARM signal.  Keep track
 * of the old one, so if they stop the timer, we can set it back.
 */
static void	    (* tmr_pfOldAlarmSignal)(int) = SIG_DFL;

/*
 * Size of the Local Data associated with each timer structure.
 */
#define	LOCAL_DATA_SIZE		64



/*
 * tmr_portationInit()
 *
 * Any initialization required for the TMR portation may be done in this
 * function.  If none is required, a null define may be used.  This function
 * may be used, for example, to allocate or otherwise obtain all timer
 * structures at init time, placing them on a queue to later be "obtain"ed.
 *
 * This portation doesn't pre-allocate timers, and doesn't use a clock tick so
 * we don't need for this function to do anything.  If you had a clock tick,
 * you could implement this function something like this:
 *
 *         #define tmr_portationInit(numberOfTimers, millisecondsPerTick)  \
 *                         tmr_millisecondsPerTick = millisecondsPerTick;
 */
void
tmr_portationInit(OS_Uint16 numberOfTimers,
		  OS_Uint16 millisecondsPerTick);
#define	tmr_portationInit(numberOfTimers, millisecondsPerTick)


/*
 * tmr_obtain()
 *
 * Obtain a timer structure.  This may be dynamically allocated, taken from a
 * queue of timers set up at portation initialization time, etc.
 *
 * Returns NULL if no timer structures are available; otherwise returns a
 * pointer to the timer structure.
 */
void *
tmr_obtain(void);
#define	tmr_obtain()			OS_alloc(sizeof(Timer) + localDataSize)

/*
 * tmr_dispose()
 *
 * Dispose of a timer structure, previously obtained from tmr_obtain().  This
 * function should reverse the effects of tmr_obtain().  If the timer
 * structure was dynamically allocated, then it should be freed here.  If it
 * was taken from an "available" queue, it should here be put back on the
 * "available" queue.
 *
 * Nothing is returned by this function.
 */
void
tmr_dispose(Timer * pTimer);
#define	tmr_dispose(pTimer)		OS_free(pTimer)

/*
 * Obtain the TMR package's idea of the current time.  This time need not
 * reflect reality.  The time "units" is milliseconds.
 *
 * This function allows calling an operating-system provided function to get
 * the "real" current time.  It also allows a non-hosted environment (or
 * minimally hosted, like DOS) to have a clock tick update a variable every
 * millisecond (or as often as practicable), with this function returning that
 * variable.
 *
 * This function should return "Success" if the time was obtained.  Otherwise,
 * any one of the valid ReturnCode values may be returned.  If the returned
 * value is "Success", the OS_Uint32 location pointed to by pTimeNow should be
 * updated with the current time.
 */
ReturnCode
tmr_currentTime(OS_Uint32 * pTimeNow)
{
    ReturnCode		rc;

    /* Get the system's idea of the current time */
    if ((rc = OS_currentDateTime(NULL, pTimeNow)) != Success)
    {
	return rc;
    }

    /* Now, convert it to milliseconds */
    *pTimeNow *= 1000;

    return Success;
}
	

/*
 * tmr_clock()
 *
 * In environments that require an interrupt-driven clock tick to be generated
 * to drive the TMR module, this function should be called upon the clock
 * tick.
 *
 */
static void
tmr_clock()
{
    /* Make sure we get another tick next interval. */
    alarm(tmr_millisecondsPerTick / 1000);
    signal(SIGALRM, tmr_clock);

    /* Keep our own time-of-day clock even though we're not using it. */
    tmr_timeOfDay += tmr_millisecondsPerTick;

    /* Call the user-specified function, if there is one. */
    if (tmr_pfTimerHandler != NULL)
    {
	(* tmr_pfTimerHandler)();
    }
}


/*
 * TMR_startClockInterrupt()
 *
 * Start a periodic interrupt.  In some portations, this interrupt may update
 * the tmr_timeOfDay variable.  Otherwise, it may be used simply as a way to
 * call a function periodically (e.g. to schedule polling of the timer queue).
 */
void
TMR_startClockInterrupt(void (* pfHandler)(void))
{
    tmr_pfTimerHandler = pfHandler;

    /* Catch alarm signals */
    tmr_pfOldAlarmSignal = signal(SIGALRM, (void (*)(int)) tmr_clock);
    alarm((tmr_millisecondsPerTick / 1000) + 1);
}


void
TMR_stopClockInterrupt(void)
{
    alarm(0);
    signal(SIGALRM, tmr_pfOldAlarmSignal);
}
