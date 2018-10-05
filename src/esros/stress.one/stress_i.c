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
 * File: stress_i.c
 * 
 * Description: ESROS stress test invoker.
 * 
 * Requirements:
 *  Continuously generate invocations
 *  Invoke serially, wait for one to complete before going to the next
 *  Log statistics, operations per period
 *  Reporting period, delay between operations is run time configurable
 * 
-*/


#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: stress_i.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "pf.h"
#include "eh.h"
#include "getopt.h"
#include "tm.h"
#include "addr.h"
#include "inetaddr.h"
#include "esro.h"
#include "extfuncs.h"
#include "target.h"
#include "tmr.h"
#include "sch.h"
#include "udp_po.h"

#define SHELL_CMD_OP	2

#define TMR_MSecs	int	

extern char pubQuName[];

PUBLIC TM_ModDesc G_tmDesc;

/* Quick temporary tracing */

Void G_init(void);
Void G_exit(Int code);
Void G_usage(void);
Int SSP_init(void);
char twirl(void);

#ifdef OS_TYPE_UNIX
Void G_sigIntr(Int unused);
Void G_sigPipe(Int unused);
Void G_sigTerm(Int unused);
Void G_sigKill(Int unused);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
Void G_sigIntr(Int unused);
#endif

ESRO_SapSel locSapSel = 14;			/* Local SAP selector */
ESRO_SapDesc locSapDesc;			/* Local SAP descriptor */
ESRO_InvokeId invokeId;				/* Invocation id */
ESRO_EncodingType encodingType;			/* Encoding type */
ESRO_OperationValue operationValue;		/* Operation value */
ESRO_SapSel remEsroSapSel = 15;			/* Remote ESRO SAP selector */
T_SapSel  remTsapSel  = {2, {0x07, 0xD2}      }; /* UDP Port Number */
N_SapAddr remNsapAddr = {4, {192, 168, 0, 10} }; /* Remote IP Address */
struct ESRO_Event event;			/* ESROS event */

unsigned reportPeriod 	= 5000;			/* Report period */
unsigned delay 		= 5000;			/* Delay between two ops */
unsigned maxInvocations = -1;			/* Max number of invocations */

FILE *logFile = stdout;
int verbose = FALSE;

LgInt invCnt    = 0;	/* Invocation counter for a report period */
LgInt totInvCnt = 0;	/* Total invocation counter for the entire period of stress testing*/
LgInt invErrCnt = 0;	/* Invocation error cournter for a report period */
LgInt totInvErrCnt = 0;	/* Total invocation error cournter for the entire 
		 	   period of stress test */
LgInt invFailCnt   = 0;	  /* Invocation failure counter for a report period */
LgInt totInvFailCnt= 0;	  /* Total invocation failure counter for the entire 
			     period of stress test */
LgInt eventCnt     = 0;	/* Event counter for a report period */
LgInt totEventCnt  = 0;	/* Event counter for the entire period of stress test */
LgInt eventErrCnt  = 0;	/* Event error counter for a report period*/
LgInt totEventErrCnt = 0;/* Event error counter for the entire period of stress test */

ESRO_FunctionalUnit funcUnit = ESRO_3Way;
Bool waitForCompletion = TRUE;

char pConfigFile[512];

char *__applicationName = "stress_i";

#ifdef UDP_PO_
extern Bool UDP_noLogSw;
extern char eropEngOutLog[512];
extern char eropEngErrLog[512];
#endif

char traceFileName[256] = "";

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
# define	KEYBOARD_EVENT		SCH_PSEUDO_EVENT
#else
# define	KEYBOARD_EVENT		(0)
#endif

#ifdef TM_ENABLED
# define DEBUG_STRING	, "keyboard input"
#else
# define DEBUG_STRING
#endif

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
FORWARD static Void
keyboardInput(Ptr arg);
#endif

static OS_Boolean	bTerminateRequested = FALSE;


/*<
 * Function:    main()
 *
 * Description: main function of stress invoker.
 *
 * Arguments:   argc, argv.
 *
 * Returns:     None.
 *
>*/

Void
main(int argc, char **argv)
{
    Int c;
    Bool badUsage; 
    char tmpbuf[512];
    
    Char *copyrightNotice;
    EXTERN Char *RELID_getCopyrightNeda(Void);
    EXTERN Char *RELID_getCopyrightAtt(Void);
    EXTERN Char *RELID_getCopyrightNedaAtt(Void);

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_QuickWin)
    /* If we ever call exit(), prompt for whether to destroy windows */
    if (_wsetexit(_WINEXITPROMPT) != 0) {
    	EH_problem("wsetexit() failed");
    } else {
    	EH_message("Success", "wsetexit()");
    }
    if (_wabout("ESROS Stress Invoker\n"
		"\n"
		"\n"
		"\n") != 0) {
    	EH_problem("wabout() failed");
    } else {
    	EH_message("Success", "wabout()");
    }
#endif

    __applicationName = argv[0];

#ifdef TM_ENABLED
    TM_init();
#endif

    badUsage = FALSE;

    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:y:l:g:h:c:r:s:p:n:m:u:t:d:o:vwV")) != EOF) {
        switch (c) {
	case 'c':
	    strcpy(pConfigFile, optarg);
	    break;

	case 'V':		/* specify configuration directory */
	    if ( ! (copyrightNotice = RELID_getCopyrightNedaAtt()) ) {
		EH_fatal("main: Get copyright notice failed\n");
	    }
	    fprintf(stdout, "%s\n", copyrightNotice);
	    exit(0);
	}
    }

    if (strlen(pConfigFile) == 0) {
	strcpy(pConfigFile, "./erop.ini");
    }

    optind = 0;

    while ((c = getopt(argc, argv, "T:y:l:g:h:c:r:s:p:n:m:u:t:d:o:vw")) != EOF) {
        switch (c) {
	case 'T':
#ifdef TM_ENABLED
	    TM_setUp(optarg);
#endif
	    break;

	case 'l':  	/* Local ESRO Sap Selector */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 12, 0, 63) ) {
		EH_problem("main: PF_getInt call failed for command line arg 'l'");
		badUsage = TRUE;
	    } else {
		locSapSel = gotVal;
	    }
	}
	    break;

	case 'r':	/* Remote ESRO Sap Selector */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 13, 0, 63) ) {
		EH_problem("main: PF_getInt call failed for command line arg 'r'");
		badUsage = TRUE;
	    } else {
		remEsroSapSel = gotVal;
	    }
	}
	    break;

	case 'p':	/* Remote Transport Sap Selector, UDP PortNu */
	{
	    Int portNu;

	    if ( PF_getInt(optarg, &portNu, 0, 0, 10000) ) {
		EH_problem("main: PF_getInt call failed for command line arg 'p'");
		badUsage = TRUE;
	    } else {
		INET_portNuToTsapSel((MdInt) portNu, &remTsapSel);
	    }
	}
	    break;

	case 'n':	/* Remote NSAP Address, NSAP Address */
	{
	    struct in_addr inetAddr;

	    * ((LgInt *) &inetAddr) = inet_addr(optarg);
	    INET_in_addrToNsapAddr(&inetAddr, &remNsapAddr);
	}
	    break;

	case 't':  	/* statistic reporting period */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 5000, CLOCK_PERIOD, 30000) ) {
		EH_problem("main: PF_getInt call failed for command line arg 't'");
		badUsage = TRUE;
	    } else {
	         reportPeriod = gotVal;
	    }
	}
	    break;

	case 'd':  	/* inter-invocation delay */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 5000, CLOCK_PERIOD, 20000) ) {
		EH_problem("main: PF_getInt call failed for command line arg 'd'");
		badUsage = TRUE;
	    } else {
		delay = gotVal;
	    }
	}
	    break;

	case 'm':	/* Max number of invocations */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 20, 1, 32767) ) {
		EH_problem("main: Invalid max number of invocations");
		badUsage = TRUE;
	    } else {
		maxInvocations = gotVal;
	    }
	}
	    break;

	case 'o':  	/* outputfile */
	{
	    if ( ( logFile = fopen(optarg, "a") ) == NULL ) {
		EH_problem("main: PF_getInt call failed for command line arg 'o'");
		badUsage = TRUE;
		logFile = stdout;
	    }
	}
	    break;

	case 'f':  	/* ESRO functional unit */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 12, 0, 63) ) {
		EH_problem("main: ");
		badUsage = TRUE;
	    } else {
		funcUnit = gotVal;
	    }
	}
	    break;

	case 's':
	    if (strlen(optarg) != 0)
	    	strcpy(pubQuName, optarg);
	    break;

	case 'v':
	    verbose = TRUE;
	    break;

	case 'g':  	/* PDU log output file */
	{
#ifdef UDP_PO_
	    if ( strlen(optarg) > 0 ) {
		strcpy(eropEngOutLog, optarg);
		UDP_noLogSw = 0;
	    } else {
		EH_problem("main: Cannot open PDU log output file\n");
		badUsage = TRUE;
	    }
#endif
	}
	    break;

	case 'h':  	/* PDU log error file */
	{
#ifdef UDP_PO_
	    if ( strlen(optarg) > 0 ) {
		strcpy(eropEngErrLog, optarg);
		UDP_noLogSw = 0;
	    } else {
		EH_problem("main: Cannot open PDU log error file\n");
		badUsage = TRUE;
	    }
#endif
	}
	    break;

        case 'c':
	    break;

        case 'w':
	    waitForCompletion = FALSE;
	    break;

        case 'y':
	    if ( strlen(optarg) > 0 ) {
		strcpy(traceFileName, optarg);
	    } else {
		EH_problem("main: Invalid Trace file name\n");
		badUsage = TRUE;
	    }

        case 'u':
        case '?':
        default:
            badUsage = TRUE;
            break;
        } 
    }

    while ((c = getopt(argc, argv, "T:u")) != EOF) {
        switch (c) {
	case 'T':
#ifdef TM_ENABLED
	    TM_setUp(optarg);
#endif
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

    if ( ! (copyrightNotice = RELID_getCopyrightNedaAtt()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);

    fprintf(logFile, "\nRemote Network Address: %d.%d.%d.%d\n", 
	    remNsapAddr.addr[0], remNsapAddr.addr[1],
	    remNsapAddr.addr[2], remNsapAddr.addr[3]);

    G_init();

    OS_init();

    SSP_init();

#ifdef UDP_PO_
    TM_TRACE((G_tmDesc, TM_ENTER, 
    	     "**** stress_i.cb.one compiled <<<WITH UDP_PO>>> *****\n"));
    if (!UDP_noLogSw) {
    	UDP_PO_init (eropEngErrLog, eropEngOutLog);
        TM_TRACE((G_tmDesc, TM_ENTER, "===> Logging activated\n"));
    }
#else
    TM_TRACE((G_tmDesc, TM_ENTER, 
    	    "**** stress_i.cb.one main compiled <<<WITHOUT UDP_PO>>> *****\n"));
#endif

#ifdef TM_ENABLED
    TM_config(traceFileName);
    TM_VALIDATE();
#endif

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);
#endif

    while (SCH_block() >= 0 && ! bTerminateRequested) {
       SCH_run();

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
	/* Allow CTRL-C actions to take place */
	kbhit();
#endif
       fflush(logFile);
    }
      
    /* Shutdown all sockets */
    while (UDP_terminate() != SUCCESS)
    {
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_QuickWin)
	/* Allow winsock to complete shutdown of sockets */
	_wyield();
#endif
    }

    if (! bTerminateRequested)
    {
	sprintf(tmpbuf, "%s: Unexpected Scheduler error.\n", __applicationName);
	EH_fatal(tmpbuf);
    }

    fprintf(logFile, "-------------------------------------------------------------------------------------\n");
    fprintf(logFile, "\n\t%ld\t\t%ld\t\t%ld\t\t%ld\t\t%ld", 
	    totInvCnt, totInvErrCnt, totInvFailCnt, totEventCnt, totEventErrCnt);

    exit (SUCCESS);

} /* main() */


/*<
 * Function:    SSP_init
 *
 * Description: Initialize.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

Int
SSP_init(void)
{
    int G_invoke(Ptr);
    int G_log(Ptr);
    ESRO_RetVal gotVal;

#ifdef OS_TYPE_UNIX
    signal(SIGINT,  G_sigIntr);
    signal(SIGPIPE, G_sigPipe);
    signal(SIGTERM, G_sigTerm);
    signal(SIGKILL, G_sigKill);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    signal(SIGINT, G_sigIntr);
#endif

    ESRO_sapUnbind(locSapSel);

    if ((gotVal = ESRO_sapBind(&locSapDesc, locSapSel, funcUnit)) < 0) {
	EH_fatal("invoke: Could not activate local ESRO SAP");
    }

    fprintf(logFile,"\nReport Period = %d msec, Delay = %d msec\n",reportPeriod,delay);
    fprintf( logFile, "\nInvocations\tInvocation Errors\tInvocation Failures\tEvents\tEvent Errors");
    fprintf( logFile, "\n=============================================================================");

    TMR_create(delay, G_invoke);
    TMR_create(reportPeriod, G_log);

    return 0;
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

Void
G_usage(void)
{
    String usage1 = "[-T G_,ffff] [-l localEsroSapSel] [-r remoteEsroSapSel] [-p remotePortNu]";
    String usage2 = "[-n remoteIPAddr] [-t reportPeriod] [-d delay] [-o logFile]";
    printf("%s: Usage: %s\n", __applicationName, usage1);
    printf("%s: Usage: %s\n", __applicationName, usage2);
}


/*<
 * Function:    G_sigIntr
 *
 * Description: Signal processing.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

Void
G_sigIntr(Int unused)
{
    fprintf(stdout, "\n\nProgram received SIGINT signal");
    fprintf(stderr, "\n\nProgram received SIGINT signal");
    TM_TRACE((G_tmDesc, TM_ENTER,
             "\n\nProgram received SIGINT signal"));

    G_exit(13);
}

#ifdef OS_TYPE_UNIX

/*<
 * Function:    G_sigPipe
 *
 * Description: Signal processing.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

Void
G_sigPipe(Int unused)
{
    fprintf(stdout, "\n\nProgram received SIGPIPE signal");
    fprintf(stderr, "\n\nProgram received SIGPIPE signal");
    TM_TRACE((G_tmDesc, TM_ENTER,
             "\n\nProgram received SIGPIPE signal"));

    G_exit(14);
}


/*<
 * Function:    G_sigTerm
 *
 * Description: Signal processing.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

Void
G_sigTerm(Int unused)
{
    fprintf(stdout, "\n\nProgram received SIGTERM signal");
    fprintf(stderr, "\n\nProgram received SIGTERM signal");
    TM_TRACE((G_tmDesc, TM_ENTER,
             "\n\nProgram received SIGTERM signal"));

    G_exit(15);
}



/*<
 * Function:    G_sigKill
 *
 * Description: Signal processing.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

Void
G_sigKill(Int unused)
{
    fprintf(stdout, "\n\nProgram received SIGKILL signal");
    fprintf(stderr, "\n\nProgram received SIGKILL signal");
    TM_TRACE((G_tmDesc, TM_ENTER,
             "\n\nProgram received SIGKILL signal"));

    G_exit(16);
}
#endif



/*<
 * Function:    G_log()
 * 
 * Description: Log statistics, reset counters.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

int
G_log(Ptr tmrData)
{
    fprintf(logFile, "\n\t%ld\t\t%ld\t\t%ld\t\t%ld\t\t%ld", 
	    invCnt, invErrCnt, invFailCnt, eventCnt, eventErrCnt);

    /*  fflush(logFile); */

    totInvCnt += invCnt;
    invCnt = 0;

    totInvErrCnt += invErrCnt;
    invErrCnt = 0;

    totInvFailCnt += invFailCnt;
    invFailCnt = 0;

    totEventCnt += eventCnt;
    eventCnt = 0;

    totEventErrCnt += eventErrCnt;
    eventErrCnt = 0;

    TMR_create((TMR_MSecs)reportPeriod, G_log);
    return 0;
}


/*<
 * Function:    G_invoke()
 * 
 * Description: Invoke an operation, wait for the response.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

int
G_invoke(Ptr tmrData)
{
    String invokeParameter = "date";
    Int gotVal;

    if (maxInvocations != -1) {
    	if (++totInvCnt > maxInvocations) {
	    totInvCnt--;
    	    bTerminateRequested = TRUE;
            return 0;
    	}
    }

    TM_TRACE((G_tmDesc, TM_ENTER, 
	     "invoker: Issue InvokeReq: remEsroSapSel=%d, parameter=%s\n",
	     remEsroSapSel, invokeParameter));

    ++invCnt;
    if ((gotVal = ESRO_invokeReq(&invokeId, NULL, locSapDesc, remEsroSapSel,
			 &remTsapSel, &remNsapAddr, 
			 (ESRO_OperationValue) SHELL_CMD_OP, 
			 (ESRO_EncodingType) 2,  /* ASCII encoding */
			 4, invokeParameter)) < 0) {
	EH_problem("invoke: Could not Invoke");
	++invErrCnt;
    	TMR_create(delay, G_invoke);
	return -1;
    }

    ++invCnt;

    if (!waitForCompletion) {
    	TMR_create(delay, G_invoke);
    }

    TM_TRACE((G_tmDesc, TM_ENTER, "invoke: Waiting for an event\n"));

    while ((gotVal = ESRO_getEvent(locSapDesc, &event, FALSE)) < 0) { 
	if (gotVal == -2) {
	    EH_fatal("Connection lost\n");
	}
    }
    ++eventCnt;

    processEvent(&event);

    return 0;
}


/*<
 * Function:    processEvent
 *
 * Description: Process event.
 *
 * Arguments:   Event.
 *
 * Returns:     0 on successful completion, -1 on unsuccessful completion.
 *
>*/

SuccFail
processEvent(struct ESRO_Event *p_event)
{
    static long int op_counter = 0;

    static char separator[] = {"----------------------------------------------------------------------------------\n"};

    switch (p_event->type) {
    case ESRO_RESULTIND:
	 p_event->un.resultInd.data[p_event->un.resultInd.len] = '\0';

	 TM_TRACE((G_tmDesc, TM_ENTER,
	 	 "processEvent(Invoker): resultInd invokeId=%d, "
	 	 "paramter=%s\n",
		 p_event->un.resultInd.invokeId,
	 	 p_event->un.resultInd.data));

	 if (verbose) {
	     printf("%s",separator);
	     printf("Invoker: Got Result Indication: invokeId=%d, "
		    "paramter=%s, op_counter=%ld\n",
		    p_event->un.resultInd.invokeId,
		    p_event->un.resultInd.data, ++op_counter);
	    printf("%s",separator);
	} else {
	    printf("\r%c",twirl());
	}
	
        if (waitForCompletion) {
    	   TMR_create(delay, G_invoke);
        }

	break;

    case ESRO_ERRORIND:
         ++invErrCnt;

	 p_event->un.errorInd.data[p_event->un.errorInd.len] = '\0';

         TM_TRACE((G_tmDesc, TM_ENTER, 
	 	 "processEvent: ErrorInd invokeId=%d, parameter=%s\n",
	       	 p_event->un.errorInd.invokeId,
               	 p_event->un.errorInd.data));

	 if (verbose) {
	     printf("%s",separator);
	     printf("Invoker: Got Error Indication: invokeId=%d, "
		    "parameter=%s, op_counter=%ld\n", 
	       	    p_event->un.errorInd.invokeId,
                    p_event->un.errorInd.data, ++op_counter);
	     printf("%s",separator);
	} else {
	    printf("\r%c",twirl());
	}

        if (waitForCompletion) {
    	   TMR_create(delay, G_invoke);
        }

	break;

    case ESRO_FAILUREIND:

        ++invFailCnt;

        TM_TRACE((G_tmDesc, TM_ENTER, "processEvent: FailureInd  Code=%d\n",
                 p_event->un.failureInd.failureValue));

	if (verbose) {
	    printf("%s",separator);
	    printf("Invoker: Operation Failed: Failure Code %d\n", 
         	   p_event->un.failureInd.failureValue);
	    printf("%s",separator);
	} else {
	    printf("\r%c",twirl());
	}

        if (waitForCompletion) {
    	   TMR_create(delay, G_invoke);
        }

	break;

    default:

        TM_TRACE((G_tmDesc, TM_ENTER, "processEvent: Invalid primitive\n"));

	EH_problem("processEvent(Invoker): Invalid primitive");
	++eventErrCnt;

	return (FAIL);
    }

    if (waitForCompletion) {
    	TMR_create(delay, G_invoke);
    }

    return (SUCCESS);
}


/*<
 * Function:    twirl
 *
 * Description: 
 *
 * Arguments:   None.
 *
 * Returns:     
 *
>*/

char twirl(void)
{
static int counter = 0;
static char thing[8] = {'|','/','-','\\','|','/','-','\\'};
return thing[counter++ % 8];
}

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
static Void
keyboardInput(Ptr arg)
{
    int			c;

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    if (! _kbhit())
    {
	/* If anything is typed on the keyboard, we want to know about it. */
        SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);

        return;
    }
    c = _getch();
#else
    c = getchar();
#endif


    switch(c)
    {
    case 0x0c:			/* CTRL-L: Refresh screen */
	break;

    case 0x03:			/* CTRL-C: Quit */
	break;

    case 'q':			/* q: Quit */
    case 'Q':			/* q: Quit */
	bTerminateRequested = TRUE;
	break;

    default:
	break;
    }

    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);

}
#endif
