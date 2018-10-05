/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
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
static char rcs[] = "$Id: main.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "tm.h"
#include "eh.h"
#include "addr.h"
#include "du.h"
#include "target.h"
#include "pf.h"
#include "tmr.h"
#include "getopt.h"
#include "udp_po.h"
#include "erop.h"
#include "sch.h"
#include "extfuncs.h"

#ifdef OS_MALLOC_DEBUG
#include "os.h"
#endif

char *__applicationName = "esros";

#ifdef UDP_PO_
extern Bool UDP_noLogSw;				/* Logging Control */
static char eropEngOutLog[256] = {"erop_out.log"}; 	/* Out Log File */
static char eropEngErrLog[256] = {"erop_err.log"}; 	/* Err Log File */
#endif

PUBLIC DU_Pool *G_duMainPool;		/* Data Unit main pool */
char *pConfigFile;

Int clockPeriod = CLOCK_PERIOD;

extern Int udpSapSel;		/* UDP SAP Selector */
extern Int nuOfSaps;		/* Max number of SAPs */
extern Int nuOfInvokes;		/* Max number of concurrent Invocations */
extern Int invokePduSize;	/* Max Invoke PDU Size */

extern char pubQuName[];	/* Public Queue Name (Unix Socket) */

extern Int      G_heartBeat(void);
extern SuccFail G_init(void);
extern Void 	G_exit(Int unUsed);
extern Void 	G_sigInt();
extern Void 	G_sigHup();
extern Void 	G_sigQuit();
extern Void 	G_sigPipe();
 
extern int getConf();

#ifdef OS_MALLOC_DEBUG
static char debugFileName[256] = "esros.dbg";
static char allocFileName[256] = "esrosAlloc.log";
extern FILE *hMemDebug;
#endif

static char *usage = "usage: %s [-T] [-p] [-c] [-l] [-o filename -e filename]\n       p=PortNu T=Tracing, u=clock unit, l=Enable logging, o=out log, e=err log\n";

extern struct TimerValue {
    Int	    acknowledgment;
    Int	    roundtrip;
    Int	    retransmit;
    Int	    maxNSDULifeTime;
} timerValue; 

extern int retransmitCount;		/* Retransmission Counter */
extern Bool dropResultConfirm;		/* For testing purposes */

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;		/* Global Module Trace descriptor */
#endif


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
    EXTERN Char *RELID_getRelidNotice(Void);
    EXTERN Char *RELID_getRelidNotice(Void);
    EXTERN Char *RELID_getRelidNotice(Void);

#ifdef OS_MALLOC_DEBUG
    if (OS_init() != (SUCCESS))
    {
	EH_fatal("esros: Initialization of OS module failed\n");
    }
    OS_allocDebugInit(allocFileName);
    OS_fileDebugInit(debugFileName); 

    TM_TRACE((G_tmDesc, TM_ENTER, 
    	     "esros: ===> Malloc debuging activated\n"));
    
#endif

#ifdef TM_ENABLED
    TM_INIT();
#endif

    G_duMainPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS); /* build buf pool */

    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:t:u:c:s:p:o:a:e:d:lxV")) != EOF) {
	switch ( c ) {
	case 'c':		/* specify configuration directory */
	    pConfigFile = optarg;
	    break;

	case 'V':		/* specify configuration directory */
	    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
		EH_fatal("main: Get copyright notice failed\n");
	    }
	    fprintf(stdout, "%s\n", copyrightNotice);
	    exit(0);
	}
    }

    if (pConfigFile == NULL) {
	pConfigFile = "./erop.ini";
    }

    getConf(pConfigFile);	/* get configuration parameters from file */

    optind = 0;			/* reset command line processing by getopt */

    while ((c = getopt(argc, argv, "T:t:u:c:s:p:o:a:e:d:lx")) != EOF) {
	switch ( c ) {

	case 'T':			/* Trace */
#ifdef TM_ENABLED
	    TM_SETUP(optarg);
#endif
	    break;

	case 'u':			/* Clock Period */
	    if (PF_getInt(optarg, &clockPeriod, 1000, 1000, 1000000) ) 
		EH_problem("main: Invalid clock period");
	    break;

	case 'p':			/* Port Number */
	    if (PF_getInt(optarg, &udpSapSel, 2002, 2000, 4000) ) {
		EH_problem("main: Invalid port number");
	    	exit(1);
	    }
	    break;

	case 's':				/* Unix Socket */
	    if (strlen(optarg) != 0)
	    	strcpy(pubQuName, optarg);
	    break;

#ifdef OS_MALLOC_DEBUG
	case 'd':				/* Debug File Name */
	    if (strlen(optarg) != 0)
	    	strcpy(debugFileName, optarg);
	    break;

	case 'a':				/* Mem Alloc file name */
	    if (strlen(optarg) != 0)
	    	strcpy(allocFileName, optarg);
	    break;
#endif

#ifdef UDP_PO_
	case 'l':				/* Log Control: (de)activate*/
	    UDP_noLogSw = 0;
	    break;

	case 'o':				/* Out Log file */
	    UDP_noLogSw = 0;
	    if (strlen(optarg) != 0)
	        strcpy(eropEngOutLog, optarg);
	    break;

	case 'e':				/* Err Log file */
	    UDP_noLogSw = 0;
	    if (strlen(optarg) != 0)
	        strcpy(eropEngErrLog, optarg);
	    break;
#endif
	case 'x':			/* Drop result.cnf (for testing) */
	    dropResultConfirm = TRUE;
	    break;

	case 'c':			/* Config file */
	    break;

	case '?':
	default :
	    sprintf(errbuf, usage, argv[0]);
	    EH_fatal(errbuf);
	}
    }

    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);

    TM_TRACE((G_tmDesc, TM_ENTER, "Port number=%d\n", udpSapSel));
    TM_TRACE((G_tmDesc, TM_ENTER, "Unix socket=%s\n", pubQuName));

    /* Don't delete: Compile date comes here */

    {
    	time_t t;
    	t = time(&t);
        TM_TRACE((G_tmDesc, TM_ENTER, "\nESROS started on: %s\n", ctime(&t)));
    }

    TM_TRACE((G_tmDesc, TM_ENTER, 
     	     "\nEngine Parameter:\n------------ Timer Values ------------\n"
	     "\tAcknowledgement\t=%d\n\tRoundtrip\t=%d"
    	     "\n\tretransmit\t=%d\n\tmaxNSDULifeTime\t=%d"
    	     "\n\tretransTimeout\t=%d\n\trwait\t\t=%d\n\tnuOfRetrans\t=%d\n"
	     "\trefKeepTime\t=%d"
    	     "\n\tinactivityDelay\t=%d\n\tperfNoResponse\t=%d\n"
	     "--------------------------------------\n",
             timerValue.acknowledgment, timerValue.roundtrip,
             timerValue.retransmit, timerValue.maxNSDULifeTime,
       	     timerValue.retransmit + timerValue.roundtrip, 
    	     2 * (timerValue.acknowledgment + timerValue.retransmit), 
             retransmitCount, 
             (2 * timerValue.maxNSDULifeTime) + 
             retransmitCount * timerValue.retransmit,
    	     4 * timerValue.roundtrip,
    	     (2 * timerValue.maxNSDULifeTime) + 
	     retransmitCount * (timerValue.retransmit + timerValue.roundtrip)));

    G_init();

    OS_init();

#ifdef UDP_PO_
    TM_TRACE((G_tmDesc, TM_ENTER, 
    	     "**** esros main compiled <<<WITH UDP_PO>>> *****\n"));
    if (!UDP_noLogSw) {
    	UDP_PO_init(eropEngErrLog, eropEngOutLog);
    }
#else
    TM_TRACE((G_tmDesc, TM_ENTER, 
    	     "**** esros main compiled <<<WITHOUT UDP_PO>>> *****\n"));
#endif

    SCH_init(MAX_SCH_TASKS);			/* Scheduler init */

    UDP_init(MAX_SAPS);				/* UDP init */
						/* ESROS Engine init */
    if (EROP_init(udpSapSel, nuOfSaps, nuOfInvokes, invokePduSize) == FAIL) {
 	sprintf(errbuf, "%s: Engine initialization (EROP_init) failed\n", 
		argv[0]);		/* improve: give error value */
 	EH_fatal(errbuf);
    }

    PSQ_init();		/* Provider-Shell Queue initialization */

    LOPS_init();	/* ESROS Primitive Queue initialization*/

    /* NMPS_init(); NOTYET -  */	/* Network Mgr. Primitive Queue */
    
    TMR_init(NUMBER_OF_TIMERS, CLOCK_PERIOD);

#ifdef TM_ENABLED
    TM_VALIDATE();
#endif

#ifndef FAST
    if (clockPeriod <= 0) {
	sprintf(errbuf, "%s: Invalid clock period: %d\n", 
		argv[0], clockPeriod);
	EH_fatal(errbuf);
    }
#endif

    TMR_startClockInterrupt(clockPeriod);

    while (TRUE) {
	if (G_heartBeat()) {
    	    break;
	}
    }

    G_exit(0);

} /* main() */



/*<
 * Function:    G_heartBeat
 *
 * Description: Heart Beat for the stack.
 *
 * Arguments:   None.
 *
 * Returns:     0 on successful completion, -1 on unsuccessful completion.
 *
>*/

SuccFail
G_heartBeat(void)
{
    if (SCH_block() < 0) {			/* Scheduler block */
        EH_fatal("main: SCH_block returned negative value");
	return (FAIL);
    }

    SCH_run();					/* Scheduler run */

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
	/* Allow CTRL-C actions to take place */
	kbhit();
#endif

    return (SUCCESS);
}


/*<
 * Function:    G_exit
 *
 * Description: Global exit.
 *
 * Arguments:   None.
 *
 * Returns:     None. 
 *
>*/

Void
G_exit(Int unUsed)
{
    TMR_stopClockInterrupt();
    unlink(pubQuName);
    exit(1);
}


/*<
 * Function:    G_init
 *
 * Description: Global initialization.
 *
 * Arguments:   None.
 *
 * Returns:     0 on successful completion, -1 on unsuccessful completion.
 *
>*/

SuccFail
G_init(void)
{
#ifdef TM_ENABLED
    if (!(G_tmDesc = TM_open("G_"))) {
	EH_problem("G_init: TM_open G_ failed");
    }
#endif

    signal(SIGINT,  G_sigInt);	/* Interrupt */
    signal(SIGHUP,  G_sigHup);	/* Hangup    */
    signal(SIGQUIT, G_sigQuit);	/* Quit      */
    signal(SIGPIPE, G_sigPipe);	/* Pipe      */
    return (SUCCESS);
}



/*<
 * Function:    G_sigXXXX
 *
 * Description: Signal handling routines
 *
 * Arguments:   None.
 *
 * Returns:     Exits.
 *
>*/

Void G_sigInt()
{
    fprintf(stdout, "\n\nProgram received SIGINT signal\n");
    G_exit(11);
}

Void G_sigHup()
{
    fprintf(stdout, "\n\nProgram received SIGHUP signal\n");
    G_exit(11);
}

Void G_sigQuit()
{
    fprintf(stdout, "\n\nProgram received SIGQUIT signal\n");
    G_exit(11);
}

Void G_sigPipe()
{
    fprintf(stdout, "\n\nProgram received SIGPIPE signal\n");
    G_exit(11);
}
