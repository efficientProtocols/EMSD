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

#ifdef SCCS_VER	/*{*/
static char sccs[] = "%W%    Released: %G%";
#endif /*}*/

#include <stdio.h>
#include "estd.h"
#include "tm.h"
#include "getopt.h"
#include "g.h"

Void SSP_init(), SSP_sapCreate(), SSP_actionPrim();
Void USER_init();

PUBLIC G_Env G_env;

main(argc, argv)
Int argc;
String argv[];
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
	    G_exit(1);
	}
    }

    G_env.hardReset = FALSE;
    G_env.softReset = FALSE;

    while ( !G_env.hardReset ) {
	SCH_init(K_schQuLen);
	SSP_init();
	USER_init();
	TM_validate();

	while ( !G_env.softReset ) {
	    SCH_block();
	    SCH_run();
	}

	SCH_term();
    }

    G_exit(1);
}

G_exit(code)
Int code;
{
    exit(code);
}

/* 
 * USER_ module.
 */

Void userEventPrim()
{
    printf("SSP Event Primitive, invoked through the scheduler\n");
    G_env.softReset = TRUE;
    G_env.hardReset = TRUE;
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

Void SSP_init()
{
    /* Initialization could have been done here */
}

Void SSP_sapCreate(eventPrim)
Void (*eventPrim)();
{
    sapEventPrim = eventPrim;
}

Void SSP_actionPrim()
{
    /* Let's say this action primitive was to result
     * into an event primitive, but since we can not rely
     * on the user context being re-enterant.
     * The event primitive is scheduled for exection
     * outside the user's context.
     */
    printf("SSP Action Primitive, invoked by USER\n");
    SCH_submit(sapEventPrim, (Ptr)0); 
}
