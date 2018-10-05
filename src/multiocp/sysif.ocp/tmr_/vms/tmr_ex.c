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

#include "estd.h"
#include "eh.h"
#include "tmr.h"

#define CLOCK_PERIOD  54   /* Millisec. Granularity of the periodic
			    * interrupt. */
#define SAMPLE_PERIOD 200  /* millisec. Sampling Period */
#define LOAD_FACTOR 2	   /* Simulated Load */ 
LgUns pureCpu;		   /* Measure of Pure Cpu in one Sample */
LgUns idleCpu;		   /* Measure of Idle Cpu in one Sample */
Bool reset = FALSE;

Void samplePureCpu();
Void IDLE_tmrHandler(), IDLE_init(), IDLE_action();
Void LOAD_init(), LOAD_action();

main()
{
    SCH_init(22);
    TMR_init(22, CLOCK_PERIOD);
    TMR_startClockInterrupt(CLOCK_PERIOD);

    samplePureCpu();
    IDLE_init();
    LOAD_init(LOAD_FACTOR);

    while (!reset) {
    	SCH_block();	/* In Un-Hosted environemnt we never block */
	SCH_run();
    }
}

typedef enum TmrId {
    PURE,
    IDLE
} TmrId;
Bool pureSampling;

Void samplePureCpu()
{
    TMR_Desc tmrDesc;
    TmrId *tmrId;
    
    tmrDesc = TMR_create((TMR_MSecs)SAMPLE_PERIOD, IDLE_tmrHandler);
    tmrId = (TmrId *) TMR_getData(tmrDesc);
    *tmrId = PURE;
    
    pureSampling = TRUE;
    pureCpu = 0;
    
    while (pureSampling) {
	++pureCpu;
	SCH_run();	/* expired timers get a chance to run */
    }
    /* Make a note of the value of pureCpu */
    printf("pureCpu = %ld\n", pureCpu);	
}

Void IDLE_tmrHandler(tmrData)
Ptr tmrData;
{
    TMR_Desc tmrDesc;
    TmrId *tmrId;
    
    tmrId = (TmrId *)tmrData;
    if (tmrId == PURE) {
	pureSampling = FALSE;
    } else if (tmrId == IDLE) {
	/* Ratio of pureCpu/idleCpu is proportional to load */
	printf("idleCpu=%ld, Load=%f\n", idleCpu,
		 (float)pureCpu/(float)idleCpu);
	idleCpu = 0;	 
	tmrDesc = TMR_create((TMR_MSecs)SAMPLE_PERIOD, IDLE_tmrHandler);
	tmrId = (TmrId *) TMR_getData(tmrDesc);
	*tmrId = IDLE;
    } else {
	EH_oops();
    }
}	

Void IDLE_init()
{
    TMR_Desc tmrDesc;
    TmrId *tmrId;

    idleCpu = 0;	 
    tmrDesc = TMR_create((TMR_MSecs)SAMPLE_PERIOD, IDLE_tmrHandler);
    tmrId = (TmrId *) TMR_getData(tmrDesc);
    *tmrId = IDLE;
    SCH_submit(IDLE_action, (Ptr)0);
}

/* ARGSUNUSED */
Void IDLE_action(arg)
Ptr arg;
{
    ++idleCpu;
    SCH_submit(IDLE_action, (Ptr)0);
}

Int loadFactor;

Void LOAD_init(load)
Int load;
{
    loadFactor = load;
    SCH_submit(LOAD_action, (Ptr)0);
}

/* ARGSUNUSED */
Void LOAD_action()
{
    Int i;
    for (i=0; i<loadFactor; ++i) {
	/* Load Simulation */
    }        
    SCH_submit(LOAD_action, (Ptr)0);
}
