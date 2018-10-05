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

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: tmr_intr.c,v 1.1.1.1 1995/05/17 06:20:52 mohsen Exp $";
#endif /*}*/


#include <estd.h>
#include "target.h"
#include <signal.h>
#ifndef SYSV
#include <sys/time.h>
#else
#include <time.h>
#endif

/* This file Should may be merged with  tmr.c.
 * At Least make sure CLACK_RATE is defined the same in both places.
 */
static int clockPeriod = 1000;

static Void
tmr_clock(Int unUsed)
{
#ifdef SYSV
    signal(SIGALRM, tmr_clock);
    alarm(clockPeriod/1000);
#endif
    TMR_clock();
#if 0
    printf("clock\n");
#endif
}

PUBLIC 
TMR_startClockInterrupt(period)
Int period;	/* In milliseconds */
{
    clockPeriod = period;
    signal(SIGALRM, tmr_clock);
#ifdef SYSV
    alarm(clockPeriod/1000);
#else
    {
	struct itimerval timer;

	timer.it_interval.tv_sec = timer.it_value.tv_sec = clockPeriod/1000;
	timer.it_interval.tv_usec = timer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, 0);
    }
#endif
}

PUBLIC 
TMR_stopClockInterrupt()
{
}
