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
#include "tmr.h"
#include "tm.h"
#include "sch.h"
#include "getopt.h"

OS_Uint16    numberOfTimers = 10;		/* arbitrary */
OS_Uint16    millisecondsPerTick = 1000;	/* 1 second per tick */
OS_Uint32    interval = 3000;			/* 3 seconds */
OS_Boolean   bInterruptOccured = FALSE;
volatile Int OS_isrActive;
OS_Uint16    exit_event = 0;
extern int tmrIsrDiag;
extern int tmrIsrErrOccurred;

FORWARD ReturnCode
newTimerExpired(void * hTimer,
		void * hUserData1,
		void * hUserData2);

FORWARD Void
ctrlchandler(int);

Int
oldTimerExpired(void * hLocalData);


void
interruptHandler(void);


int
main(int argc, char * argv[])
{
    int		    i;
    int		    j;
    ReturnCode	    rc;
    void *	    hTimer;
    int		    c;
    OS_Boolean	    foundSomething;
    char *	    p;
    OS_Uint16	    dataSize;

    OS_allocDebugInit(NULL);
    TM_init();

    while ((c = getopt(argc, argv, "i:n:t:")) != EOF)
    {
        switch (c)
	{
	case 'i':		/* timer Interval */
	    interval = atoi(optarg);
	    break;

	case 'n':		/* Number of timers */
	    numberOfTimers = atoi(optarg);
	    break;
	    
	case 't':		/* milliseconds per Tick */
	    millisecondsPerTick = atoi(optarg);
	    break;
	    
	default:
	  usage:
	    fprintf(stderr,
		    "usage: "
		    "tmrtest [-n <number of timers>]\n");
	    return 1;
	}
    }

    argc -= optind;
    argv += optind;

    if (argc != 0)
    {
      goto usage;
    }

    signal(SIGINT, ctrlchandler);

    /* Initialize the scheduler package */
    SCH_init(4);

    /* Initialize the timer package */
    TMR_init(numberOfTimers, millisecondsPerTick);

    /* Start a bunch of new-style timers */
    for (i = 0; i < numberOfTimers / 2; i++)
    {
	if ((rc = TMR_start(interval, (void *) i, &millisecondsPerTick,
			    newTimerExpired, &hTimer)) != Success)
	{
	    fprintf(stderr, "TMR_start() returned 0x%x\n", rc);
	    return 1;
	}

    }

    /* Start (create) a bunch of old-style timers */
    for (i = 0; i < numberOfTimers / 2; i++)
    {
	hTimer = TMR_create(interval, oldTimerExpired);

	dataSize = TMR_getLocalDataSize();
	p = TMR_getData(hTimer);
	
	for (j = 0; j < dataSize - 1; j++)
	{
	    p[j] = i + '0';
	}
	p[j] = '\0';
	printf("Started traditional timer 0x%lx\n", (unsigned long) hTimer);
    }

    /* Start generating timer interrupts. */
    TMR_startClockInterrupt(interruptHandler);

    while (!exit_event && !tmrIsrErrOccurred)
    {
      SCH_block();
      SCH_run();
      foundSomething = FALSE;
      if ((rc = TMR_processQueue(&foundSomething)) != Success)
         {
            fprintf(stderr, "TMR_processQueue() returned 0x%x\n", rc);
            return 1;
         }

         if (! foundSomething)
         {
	    printf("Sleeping.\n");
            OS_pause();
         }
    }

    printf("\ntmrIsrDiag = %d\n", tmrIsrDiag);
    printf("\ntmrIsrErrOccurred = %d\n", tmrIsrErrOccurred);
    return 0;
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

ReturnCode
newTimerExpired(void * hTimer,
		void * hUserData1,
		void * hUserData2)
{
    ReturnCode	    rc;
    
    if (bInterruptOccured)
    {
	printf("Interrupt\n");
	bInterruptOccured = FALSE;
    }

    if (hUserData2 == &millisecondsPerTick)
    {
	printf("Timer %lu Expired: data valid.\n", (unsigned long) hUserData1);
    }
    else
    {
	printf("Timer Expired: ERROR in user data!\n");
	return Fail;
    }

    /* restart the timer */
    if ((rc = TMR_start(interval, hUserData1, &millisecondsPerTick,
			newTimerExpired, &hTimer)) != Success)
    {
	fprintf(stderr, "TMR_start() returned 0x%x\n", rc);
	return 1;
    }

    return Success;
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

Int
oldTimerExpired(void * hLocalData)
{
    int		    j;
    char *	    pLocalData = hLocalData;
    void *	    hTimer;
    OS_Uint16	    dataSize;
    char *	    p;

    if (bInterruptOccured)
    {
	printf("Interrupt\n");
	bInterruptOccured = FALSE;
    }

    printf("Traditional Timer 0x%lx expired.  Data=(%s)\n",
	   (unsigned long) TMR_getDesc(pLocalData), pLocalData);

    /* restart the timer */
    hTimer = TMR_create(interval, oldTimerExpired);

    dataSize = TMR_getLocalDataSize();
    p = TMR_getData(hTimer);
	
    for (j = 0; j < dataSize - 1; j++)
    {
	p[j] = *pLocalData;
    }
    p[j] = '\0';

    printf("Started traditional timer 0x%lx\n", (unsigned long) hTimer);

    return Success;
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

Void 
ctrlchandler(int sig)
{
    exit_event = 1;
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

void
interruptHandler(void)
{
    bInterruptOccured = TRUE;
}
