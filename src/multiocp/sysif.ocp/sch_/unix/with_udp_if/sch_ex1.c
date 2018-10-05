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
 * File: sch_ex1.c
 *
 * Description: Example program fro Scheduler module: 
 *              Used with UDP module to show the single point of blocking.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sch_ex1.c,v 1.4 1999/09/30 00:05:11 mohsen Exp $";
#endif /*}*/

#include <stdio.h>
#include <string.h>

#include "estd.h"		/* Variable type definitions */
#include "getopt.h"		/* Get options from command line */
#include "g.h"			/* Global definitions specific to this prog */
#include "tm.h"			/* Trace module */
#include "inetaddr.h"		/* extern definition of addr. transl. funcs. */
#include "udp_if.h"		/* UDP Interface module */

#include "sch.h"		/* Scheduler module */

#define MAXBFSZ 1528		/* Maximum buffer size */

extern Void USER_init();
extern Void SSP_init();
extern Void SSP_sapCreate();

PUBLIC G_Env    G_env;		/* controls hard/soft reset of program */
PUBLIC DU_Pool *G_duMainPool;   /* Data Unit pool */

#define SSP_K_UdpSapSel 2002	/* Service Provider: UDP SAP Selector */

int stepwise = 0;		/* switch to controled execution of program */

Void
main(Int argc, String argv[])
{
    Int c;

    TM_init();			/* Initialize Trace Module */

    while ((c = getopt(argc, argv, "T:t:S:s")) != EOF) {
	switch ( c ) {
	case 'T':			/* Trace */
	case 't':			/* Trace */
	    TM_setUp(optarg);
	    break;
	case 'S':			/* Stepwise execution of program */
	case 's':
	    stepwise = 1;
	    break;
	case '?':
	default:
	    fprintf(stderr, "Usage: %s [-T] [-S]", argv[0]);
	    exit(1);
	}
    }

    G_duMainPool    = DU_buildPool(MAXBFSZ, 20, 40);  /* Build Data Unit pool */
    G_env.hardReset = G_env.softReset = FALSE;

    while ( !G_env.hardReset ) {
	SCH_init(K_schQuLen);
	SSP_init();
	USER_init();
	TM_validate();

	while ( !G_env.softReset ) {
	    if (stepwise == 1) {
		printf("\nIn step mode. Press ENTER:");
		getchar();
	    }

	    if (SCH_block() < 0) 
	   	fprintf(stderr, "main: Handle interrupts here");
	    
	    SCH_run();
	}

	SCH_term();
    }

    exit(0);

} /* main() */

/* 
 * USER_ module.
 */

Void
userEventPrim(T_SapSel  *remTsapSel,
 	      N_SapAddr *remNsapAddr,
	      T_SapSel  *locTsapSel,
	      N_SapAddr *locNsapAddr,
	      DU_View data)
{
    printf ("\nSSP Event Primitive, invoked through scheduler: \ndata: %s]\n", 
	    DU_data(data));
}

Void USER_init()
{
    static T_SapSel locTsapSel;

    INET_portNuToTsapSel (SSP_K_UdpSapSel, &locTsapSel);

    SSP_sapCreate((T_SapSel)locTsapSel, (Void *)userEventPrim);
}


/*
 * Some Service Provider (SSP_) Module.
 */


Void 
SSP_init()
{
    UDP_init(5);
}

Void 
SSP_sapCreate(T_SapSel locTsapSel, Void (*eventPrim)())
{
    UDP_sapUnbind (&locTsapSel);
    UDP_sapBind (&locTsapSel, (Void *)eventPrim);
}
