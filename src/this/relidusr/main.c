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
 * File name: main.c
 *
 * Description: Provider "main" program, and other G_ module functions.
 *
 * Functions:
 *   Int main(int argc, char **argv)
 *   G_heartBeat(void)
 *   G_exit(Int unUsed)
 *   G_init(void)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: main.c,v 1.5 2002/11/04 20:11:05 mohsen Exp $";
#endif /*}*/

#if defined(OS_VARIANT_WinCE)
#include "os.h"
#endif

#include "estd.h"
#include "tm.h"
/* #include "eh.h" */
#include "getopt.h"
#include "relid.h"
#include "cpr.h"

static char *usage = "usage: %s [-V]\n";

#define TM_ENABLED
#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;		/* Global Module Trace descriptor */
#endif

#if defined(OS_TYPE_UNIX) || defined(OS_VARIANT_Dos) || defined(OS_VARIANT_QuickWin) || defined(WINDOWS)

/*<
 * Function:    main()
 *
 * Description: Provider main program.
 *
 * Arguments:   argc, argv.
 *
 * Returns:     0 on successful execution, other than 0 on unsuccessful.
 * 
>*/

void
main(int argc, char **argv)
{
    int c;
    extern char *optarg;
    extern int optind;
    char errbuf[1024];
    Char *copyrightNotice;
    String relidNotice;



    optind = 0;			/* reset command line processing by getopt */

    while ((c = getopt(argc, argv, "V")) != EOF) {
	switch ( c ) {

	case 'V':
	    /* Verify that the copyright notice is authentic and print it */
	    {
		if ( ! (copyrightNotice = CPR_getCopyrightNeda()) ) {
		  /*  EH_problem("main: get copyright failed"); */
		    exit(1);
		}
		fprintf(stdout, "%s\n", copyrightNotice);
		exit(0);
	    }
	    break;

	default :
	    sprintf(errbuf, usage, argv[0]);
	    /* EH_fatal(errbuf); */
	}
    }

    /* 
     * the first thing a program does print out the version number.
     */

    if ( ! (relidNotice = RELID_getRelidNotice()) ) {
      /* EH_problem("main: get relid failed"); */
	exit(1);
    }
	
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", relidNotice);
   

    /* 
     * the first thing a program does--check copyright and 
     * print out the version number.
     */

    if ( ! (copyrightNotice = CPR_getCopyrightNotice()) ) {
      /* EH_problem("main: get copyright failed"); */
	exit(1);
    }
	
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);
    

    /*
     * now go ahead and do what it's supposed to do
     */

    fprintf(stdout, "This program has two purposes:\n\n");
    fprintf(stdout, "   1.  It prints the copyright notice to show the state of librelid.a\n");
    fprintf(stdout, "   2.  It demonstrates the various copyrights in librelid.a\n\n");

    fprintf(stdout, "The Neda copyright\n");
    fprintf(stdout, "------------------\n\n");
    fprintf(stdout, "%s\n", CPR_getCopyrightNeda());

}
#elif defined(OS_VARIANT_WinCE)
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int CmdShow)
{
    String copyrightNotice;
    String relidNotice;

#if 0
    TM_INIT();
    OS_init();
    TM_SETUP("OS_,ffff");
    TM_VALIDATE();
#endif

    /* 
     * the first thing a program does is print out the release notice
     */

    if ( ! (relidNotice = RELID_getRelidNotice()) ) {
	exit(1);
    }
    fprintf(stdout, "%s\n\n", relidNotice);

    /* 
     * the next thing a program does is print out the copyright notice
     */

    if ( ! (copyrightNotice = CPR_getCopyrightNotice()) ) {
	exit(1);
    }
    fprintf(stdout, "%s\n\n", copyrightNotice);

    /*
     * now go ahead and do what it's supposed to do
     */

    fprintf(stdout, "This program has two purposes:\n\n");
    fprintf(stdout, "   1.  It prints the copyright notice to show the state of librelid.a\n");
    fprintf(stdout, "   2.  It demonstrates the various copyrights in librelid.a\n\n");

    fprintf(stdout, "The Neda copyright\n");
    fprintf(stdout, "------------------\n\n");
    fprintf(stdout, "%s\n", CPR_getCopyrightNeda());

    return(0);			/* 0 since haven't entered message loop */

}
#endif /* OS_VARIANT_WinCE */

