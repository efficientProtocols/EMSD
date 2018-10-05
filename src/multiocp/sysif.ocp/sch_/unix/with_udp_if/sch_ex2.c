/*
 *  
 *  Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
 * 
 */

/*+
 * File: sch_ex2.c
 *
 * Description: Example program fro Scheduler module: 
 *              Usage of Scheduler when code is not re-enterant.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sch_ex2.c,v 1.4 1999/09/30 00:05:13 mohsen Exp $";
#endif /*}*/

#include <stdio.h>

#include "estd.h"
#include "tm.h"
#include "getopt.h"
#include "g.h"

#include "sch.h"

Void SSP_init(), SSP_sapCreate(), SSP_actionPrim(); 
Void USER_init();

PUBLIC G_Env G_env;

Void
main(Int argc, String argv[])
{
    Int c;

    TM_init();

    while ((c = getopt(argc, argv, "T:t:")) != EOF) {
	switch ( c ) {
	case 'T':
	case 't':
	    TM_setUp(optarg);
	    break;
	case '?':
	default:
	    fprintf(stderr, "Usage: %s [-T] [-S]", argv[0]);
	    G_exit(1);
	}
    }

    G_env.hardReset = G_env.softReset = FALSE;

    while ( !G_env.hardReset ) {
	SCH_init(K_schQuLen);
	SSP_init();
	USER_init();
	TM_validate();

	while ( !G_env.softReset ) {

	    if (SCH_block() < 0) {
	   	fprintf(stderr, "main: Handle interrupts here");
	    } 
	    SCH_run();
	}

	SCH_term();
    }

    G_exit(0);

} /* main() */

Void
G_exit(Int code)
{
    exit(code);
}

/* 
 * USER_ module.
 */

Void 
userEventPrim()
{
    printf("SSP Event Primitive, invoked through the scheduler\n");
    G_env.softReset = G_env.hardReset = TRUE;
}

Void USER_init()
{
    SSP_sapCreate(userEventPrim);
    SSP_actionPrim();
}

/*
 * Some Service Provider (SSP_) Module.
 */
static Void (*sapEventPrim)();

Void 
SSP_init()
{
    /* Initialization could have been done here */
}

Void 
SSP_sapCreate(Void (*eventPrim)())
{
    sapEventPrim = eventPrim;
}

Void 
SSP_actionPrim()
{
    /* Let's say this action primitive was to result
     * into an event primitive, but since we can not rely
     * on the user context being re-enterant.
     * The event primitive is scheduled for exection
     * outside the user's context.
     */
    printf("SSP Action Primitive, invoked by USER\n");

    SCH_submit(sapEventPrim, (Ptr)0, SCH_PSEUDO_EVENT, 0); 

}
