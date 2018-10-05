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
static char sccs[] = "@(#)tm_ex.c	1.1    Released: 13 Mar 1990";
#endif /*}*/

#define TM_ENABLED

#include "estd.h"
#include "tm.h"
#include "getopt.h"

Void G_init(), SSP_init(), SSP_action();

TM_ModDesc G_tmDesc;

Int
main(Int argc, String argv[])
{
    Int c;

    G_init();

    while ((c = getopt(argc, argv, "T:t:")) != EOF) {
	switch ( c ) {
	case 'T':
	case 't':
	    TM_setUp(optarg);
	    break;
	case '?':
	default:
	    exit(1);
	}
    }

    TM_trace(G_tmDesc, TM_ENTER,
	     "G_ can be used by orphand modules\n");

    SSP_init();
    TM_validate();
    SSP_action();
    exit(0);
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

Void G_init()
{
    TM_init();
    G_tmDesc = TM_open("G_");
}

/*
 * SSP_ MODULE
 */

TM_ModDesc SSP_tmDesc;


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

Void SSP_init()
{
    SSP_tmDesc = TM_open("SSP_");
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

Void SSP_action()
{
    static Char *someData = "SOME DATA";

#ifdef TM_ENABLED
    TM_trace(SSP_tmDesc, TM_ENTER,
	     "Action, someData=%s\n", TM_prAddr(someData));
    TM_hexDump(SSP_tmDesc, TM_ENTER,
	       "someData", someData, strlen(someData)+1);
#endif

#ifdef TM_ENABLED
    if (TM_query(SSP_tmDesc, TM_BIT1)) {
	printf("%s This way we can extend TM_ capabilities\n",
	       TM_here());
    }		 
#endif
}
