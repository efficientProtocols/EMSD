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
 * This simple program demonstrates the use of timer facilities
 * for approximate measurement of CPU idle time in an unhosted
 * environemnt.
 */


#include <stdio.h>
#include <signal.h>
#include <conio.h>

#include "estd.h"
#include "eh.h"
#include "tmr.h"
#include "getopt.h"
#include "tm.h"
#include "sch.h"

#define CLOCK_PERIOD  1100 /* millisec. Granularity of the timer tick is 54.94msec 
			      for DOS */
#define SAMPLE_PERIOD 2000 /* millisec. Sampling Period */
#define LOAD_FACTOR 10	   /* Simulated Load */ 

typedef enum TmrId {
    PURE,
    IDLE
} TmrId;

typedef struct G_Env {
    Char *progName;
    /* Application Specific Information */
} G_Env;

volatile Int OS_isrActive;

LgUns pureCpu;		   /* Measure of Pure Cpu in one Sample */
LgUns idleCpu;		   /* Measure of Idle Cpu in one Sample */
Bool reset = FALSE;
LgUns samplePeriod = SAMPLE_PERIOD;
Int loadFactor = LOAD_FACTOR;
Int clockPeriod = CLOCK_PERIOD;
Bool pureSampling;
PUBLIC G_Env G_env;

Void samplePureCpu();
Void IDLE_tmrHandler(), IDLE_init(), IDLE_action();
Void LOAD_init(), LOAD_action();
Void G_exit();


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
main(int argc, char **argv)
{
    Int c;
    Bool badUsage; 
    
    G_env.progName = argv[0];
    TM_init();

    badUsage = FALSE;
    while ((c = getopt(argc, argv, "T:s:l:h:c:")) != EOF) {
        switch (c) {
	case 'T':
	    TM_setUp(optarg);
	    break;

	case 's':
	    samplePeriod = atol(optarg);
	    break;

	case 'l':
	    loadFactor = atoi(optarg);
	    break;

	case 'c':
	    clockPeriod = atoi(optarg);
	    break;

        case 'h':
        default:
            badUsage = TRUE;
            break;
        } 
    }

    if (badUsage) {
	G_usage();
        G_exit(1);
    }

    signal(SIGINT, G_exit); /* ctrl-c handler */

    printf("\nclock period = %d milliseconds", clockPeriod);
    printf("\nsample period = %d milliseconds", samplePeriod);
    printf("\nload factor = %d\n", loadFactor);

    SCH_init(/*22*/4);
    TMR_init(/*22*/10, clockPeriod);
    TMR_startClockInterrupt(clockPeriod);

    TM_validate();
    samplePureCpu();
    IDLE_init();
    LOAD_init(loadFactor);

    while (!kbhit()) {
    	SCH_block();	/* In Un-Hosted environemnt we never block */
	SCH_run();
    }

    G_exit(0);
}



/*<
 * Function:    G_exit
 *
 * Description: Exit.
 *
 * Arguments:   Exit code.
 *
 * Returns:     None.
 *
>*/

Void
G_exit(Int code)
{
  exit(code);
}


/*<
 * Function:    G_usage
 *
 * Description: Usage.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

//Void
G_usage(void)
{
    String usage1 = "[-T module_name,trace_mask] [-s sample_period] [-l load_factor] [-c clock_period]";
    String usage2 = "";
    printf("\n%s: Usage: %s\n", G_env.progName, usage1);
/*    printf("%s: Usage: %s\n", G_env.progName, usage2); */
}



/*<
 * Function:	samplePureCpu
 *
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/

Void samplePureCpu()
{
    TMR_Desc tmrDesc;
    TmrId *tmrId;
    static int n;

    tmrDesc = TMR_create((LgInt)samplePeriod,IDLE_tmrHandler);
    tmrId = (TmrId *) TMR_getData(tmrDesc);
    *tmrId = PURE;
    pureSampling = TRUE;
    pureCpu = 0;
    
    while (pureSampling) {
	++pureCpu;
	SCH_block(); 
	SCH_run();	/* expired timers get a chance to run */
    }

    /* Make a note of the value of pureCpu */
    printf("%d: pureCpu = %ld\n", n++, pureCpu);	
}

/*<
 * Function:	IDLE_tmrHandler
 *
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/

Void 
IDLE_tmrHandler(Ptr tmrData)
{
    TMR_Desc tmrDesc;
    TmrId *tmrId;
    static int n;

    if (kbhit())
	G_exit(0);
    
    tmrId = (TmrId *)tmrData;
    if (*tmrId == PURE) {
	pureSampling = FALSE;
	n = 0;
    } else if (*tmrId == IDLE) {
	/* Ratio of pureCpu/idleCpu is proportional to load */

	if ( idleCpu != 0 )
	    printf("%d: idleCpu=%ld, Load=%f\n", n++, idleCpu,
		 (float)pureCpu/(float)idleCpu);
	else
	    printf("Oops! idleCpu = %ld\n", idleCpu);

	idleCpu = 0;	 
	tmrDesc = TMR_create((LgInt)samplePeriod, IDLE_tmrHandler);
	tmrId = (TmrId *) TMR_getData(tmrDesc);
	*tmrId = IDLE;
    } else {
	EH_oops();
    }
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

Void IDLE_init()
{
    TMR_Desc tmrDesc;
    TmrId *tmrId;

    idleCpu = 0;	 
    tmrDesc = TMR_create((LgInt)samplePeriod, IDLE_tmrHandler);
    tmrId = (TmrId *) TMR_getData(tmrDesc);
    *tmrId = IDLE;
#ifdef TM_ENABLED
    SCH_submit((Void *)IDLE_action,(Ptr)0,SCH_PSEUDO_EVENT,"in IDLE_init()");
#else
    SCH_submit(IDLE_action,(Ptr)0,SCH_PSEUDO_EVENT);
#endif
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
IDLE_action(Ptr arg)
{
    ++idleCpu;
#ifdef TM_ENABLED
    SCH_submit(IDLE_action,(Ptr)0,SCH_PSEUDO_EVENT,"in IDLE_action()");
#else
    SCH_submit(IDLE_action,(Ptr)0,SCH_PSEUDO_EVENT);
#endif
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

Void LOAD_init(Int load)
{
    loadFactor = load;
#ifdef TM_ENABLED
    SCH_submit(LOAD_action,(Ptr)0,SCH_PSEUDO_EVENT,"in LOAD_init()");
#else
    SCH_submit(LOAD_action,(Ptr)0,SCH_PSEUDO_EVENT);
#endif
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
LOAD_action()
{
    Int i;
    for (i=0; i<loadFactor; ++i) {
	/* Load Simulation */
    }        
#ifdef TM_ENABLED
    SCH_submit(LOAD_action,(Ptr)0,SCH_PSEUDO_EVENT,"in LOAD_action()");
#else
    SCH_submit(LOAD_action,(Ptr)0,SCH_PSEUDO_EVENT);
#endif
}



