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
 * File name: single.c 
 *
 * Description: 
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)main.c	1.1    Released: 8/8/87";
#endif /*}*/

#include "estd.h"
#include "getopt.h"
#include "eh.h"
#include "tm.h"
#include "pf.h"
#include "addr.h"
#include "inetaddr.h"

typedef struct G_Env {
    Char *progName;
    /* Application Specific Information */

} G_Env;

PUBLIC TM_ModDesc G_tmDesc;
PUBLIC G_Env G_env;

/* Quick temorary tracing */
#define G_tmHere()  TM_trace(G_tmDesc, TM_ENTER, "\n")

Void G_init();
Void G_exit();
Void G_usage();
Void G_sigIntr();

Int portNu;

T_SapSel remTsapSel;

N_SapAddr remNsapAddr;

struct in_addr inetAddr;


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
main(Int argc, String argv[])
{
    Int c;
    Bool badUsage; 
    
    G_env.progName = argv[0];
    TM_init();

    badUsage = FALSE;
    while ((c = getopt(argc, argv, "T:l:r:p:n:u")) != EOF) {
        switch (c) {
	case 'T':
	    TM_setUp(optarg);
	    break;

	case 'l':
	    break;

	case 'r':
	    break;

	case 'p':
	    if ( PF_getInt(optarg, &portNu, 0, 0, 10000) ) {
		EH_problem(ZPTR);
	    }
	    INET_portNuToTsapSel((MdInt) portNu, &remTsapSel);

	    break;

	case 'n':
	    * ((LgInt *) &inetAddr) = inet_addr(optarg);
	    INET_in_addrToNsapAddr(&inetAddr, &remNsapAddr);
	    
	    break;


        case 'u':
        case '?':
        default:
            badUsage = TRUE;
            break;
        } 
    }

    if (badUsage) {
	G_usage();
        G_exit(1);
    }

    G_init();
    TM_validate();

    do {
	String argument;
	static int argCount = 0;

	++argCount;
	if ( optind >= argc ) {
	    /* No arguments to start with */
	    /*
	     * inStream = stdin;
	     */
	} else {
	    /* Process Arguments */
	    argument = argv[optind];
	    TM_trace(G_tmDesc, TM_ENTER, "Argument, %s\n", argument);

	    if (!strcmp(argument, "-")) {
		/* Stdin Has been specified */
		TM_trace(G_tmDesc, TM_ENTER, "Stdin\n");
	    }
	}
    } while ( ++optind < argc );

    /* Do here what needs to be done */
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
G_init()
{
    G_tmDesc = TM_open("G_");
    if (!G_tmDesc) {
	EH_fatal("");
    }
    
    signal(SIGINT, G_sigIntr);
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
G_exit(code)
Int code;
{
    exit(code);
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
G_usage()
{
    String usage1 = "[-l lines] [-c bytes] [-n name] [file]";
    String usage2 = "[-lines] [file [name]]";
    printf("%s: Usage: %s\n", G_env.progName, usage1);
    printf("%s: Usage: %s\n", G_env.progName, usage2);
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
G_sigIntr()
{
    signal(SIGINT, G_sigIntr);
    G_exit(22);
}
