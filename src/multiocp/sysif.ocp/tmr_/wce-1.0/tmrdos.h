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

#include <time.h>
#include <dos.h>

#include "estd.h"
#include "queue.h"
#include "eh.h"
#include "sch.h"
#include "tmr.h"
#include "tm.h"

#include "pf.h"

 
#define PC_TIMER_INT 0x1c
#define PC_CLOCK_FREQ_HZ 18.2
#define PC_CLOCK_PERIOD_MSEC ((int)((1.0/PC_CLOCK_FREQ_HZ)*1000))

EXTERN PUBLIC Int SCH_submit();

/* --- Globals --- */

PUBLIC int isrErr;      /* diagnostic */

/* --- ISR Stack --- */

#define ISR_STACK_SIZE 1024
STATIC unsigned isrSaveSS;
STATIC unsigned isrSaveSP;
STATIC Byte isrStack[ISR_STACK_SIZE];
STATIC unsigned isrStackTop;

/* --- Misc local variables --- */

PUBLIC int clockPeriod;
PUBLIC void(__cdecl __interrupt __far *tmr_old_isr)();
extern PUBLIC void __interrupt __far tmr_clock_isr();
PUBLIC int tmr_countdown;
STATIC virgin = TRUE;

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

/* Disable stack checks for this interrupt service routine */
#pragma check_stack (off)

static void
tmr_clock()
{
    /* Keep our own time-of-day clock even though we're not using it. */
    tmr_timeOfDay += tmr_millisecondsPerTick;


    /* Call the user-specified function, if there is one. */
    if (tmr_pfTimerHandler != NULL)
    {
	if (! OS_atomicOperationInProgress)
	{
	    (* tmr_pfTimerHandler)();
	}
    }
}
/* Reset stack checking back to the default */
#pragma check_stack ()


/*
 * TMR_startClockInterruptPlus()
 *
 * Start a periodic interrupt.  In some portations, this interrupt may update
 * the tmr_timeOfDay variable.  Otherwise, it may be used simply as a way to
 * call a function periodically (e.g. to schedule polling of the timer queue).
 */
void
TMR_startClockInterruptPlus(void (* pfHandler)(void))
{
    Void TMR_stopClockInterrupt(void);
    static virgin = TRUE;

    tmr_pfTimerHandler = pfHandler;

    if ( virgin == TRUE )
    {
        virgin = FALSE;

 /*       isrStackTop = (unsigned)(&isrStack) + ISR_STACK_SIZE - 1; */

        tmr_millisecondsPerTick = (tmr_millisecondsPerTick
				   < PC_CLOCK_PERIOD_MSEC)
			           ? PC_CLOCK_PERIOD_MSEC 
				   : tmr_millisecondsPerTick;

        tmr_countdown = clockPeriod = tmr_millisecondsPerTick/
				      PC_CLOCK_PERIOD_MSEC;

        tmr_old_isr = _dos_getvect(PC_TIMER_INT);
        atexit(TMR_stopClockInterrupt);
        _dos_setvect(PC_TIMER_INT, tmr_clock_isr);
        return;
    }
    else
    {
        return;
    }
}

/* Disable stack checks for this interrupt service routine */
#pragma check_stack (off)

static void
scheduleTimer(void)
{
#ifdef TM_ENABLED
# define	TIMER_TASK	, NULL
#else
# define	TIMER_TASK
#endif

    SCH_submit((Void *)TMR_poll, (Ptr)0,
    	       (SCH_Event)SCH_PSEUDO_EVENT TIMER_TASK);
}
/* Reset stack checking back to the default */
#pragma check_stack ()

/*
 * TMR_startClockInterrupt()
 *
 * Start a periodic interrupt.  In some portations, this interrupt may update
 * the tmr_timeOfDay variable.  Otherwise, it may be used simply as a way to
 * call a function periodically (e.g. to schedule polling of the timer queue).
 * 
 * BACKWARDS COMPATIBILITY!!!
 */
SuccFail
TMR_startClockInterrupt(Int period)
{
    tmr_millisecondsPerTick = period;

    TMR_startClockInterruptPlus(scheduleTimer);
    
    return (SUCCESS);
}


void
TMR_stopClockInterrupt(void)
{
    _dos_setvect(PC_TIMER_INT,tmr_old_isr);
}



/*<
 * Function:    tmr_clock_isr
 *
 * Description: Periodic timer interrupt service routine.
 *
 * Arguments:   None. 
 *
 * Returns:     None. 
 *
 * Caution:     Be sure not to call any functions from here that result in 
 *		a C compiler generated stack check (Microsoft C). This 
 *		includes certain library functions like fprintf(), etc.
 *
 * Notes:       All registers, except SS, are stacked by the Microsoft C 
 *              library code prior to entry to this routine. Also, DS is
 *              initialized to the local _DATA segment address.
 * 
>*/

/* Disable stack checks for this interrupt service routine */
#pragma check_stack (off)

void __far __interrupt tmr_clock_isr()
{
    if (--tmr_countdown <= 0 )
    {
	/* Only reset countdown if we actually call user-provided handler */
	if (! OS_atomicOperationInProgress)
	{
	    tmr_countdown = clockPeriod;    /* re-init the timer */
	}

	OS_isrActive = TRUE;
	tmr_clock();
	OS_isrActive = FALSE;
    }

    /* Call the next interrupt handler in the chain */
    _chain_intr( tmr_old_isr );
}

/* Reset stack checking back to the default */
#pragma check_stack ()
