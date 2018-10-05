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
 * Description: ESROS Stress Tester: invoker with callback API.
 * 
 * Requirements:
 *  Continuously generate invocations
 *  Invoke serially, wait for one to complete before going to the next
 *  Log statistics, operations per period
 *  Reporting period, delay between operations is run time configurable
 * 
-*/

/*
 * Author: Mohsen Banan, Hugh Shane
 * History:
 *
 */

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
#include "extfuncs.h"
#include "target.h"
#include "tmr.h"
#include "sch.h"
#include "udp_po.h"

#include "esro_cb.h"

#include "relid.h"

#define SHELL_CMD_OP	2

static long int counter = 0;		/* Operation Counter */
PUBLIC DU_Pool *G_duMainPool;		/* Data Unit main pool */

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;		/* Global module trace handler */
#endif

/* Quick temporary tracing */

Void G_init(void);
Void G_exit(Int code);
Void G_usage(void);
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

ESRO_SapSel locSapSel = 14;			 /* Local SAP selector */
ESRO_SapDesc locSapDesc;			 /* Local SAP descriptor */
ESRO_InvokeId invokeId;				 /* Invocation Identifier */
ESRO_EncodingType encodingType;			 /* Encoding type */
ESRO_OperationValue operationValue;		 /* Operation value */
ESRO_SapSel remEsroSapSel = 15;			 /* Remote SAP selector */
T_SapSel  remTsapSel  = {2, {0x07, 0xD2}      }; /* UDP Port Number (2002)*/
N_SapAddr remNsapAddr = {4, {198, 62, 92, 10} }; /* Remote IP Address */
struct ESRO_Event event;			 /* ESROS event */

unsigned reportPeriod   = 20000;		 /* Report period */
unsigned delay          = 5000;			 /* Delay period */
unsigned long int maxInvocations = -1;	 	 /* Max number of invocations */

FILE *logFile = stdout;				 /* Log file */
int verbose = FALSE;				 /* Verbose switch */

LgInt invCnt    = 0;	  /* Invocation counter for a report period */
LgInt totInvCnt = 0;	  /* Total invocation counter for the entire 
			     period of stress testing*/
LgInt invErrCnt    = 0;	  /* Invocation error counter for a report period */
LgInt totInvErrCnt = 0;	  /* Total invocation error counter for the entire 
			     period of stress test */
LgInt invFailCnt   = 0;	  /* Invocation failure counter for a report period */
LgInt totInvFailCnt= 0;	  /* Total invocation failure counter for the entire 
			     period of stress test */
LgInt eventCnt     = 0;	  /* Event counter for a report period */
LgInt totEventCnt  = 0;	  /* Event counter for the entire period 
			     of stress test */
LgInt eventErrCnt    = 0; /* Event error counter for a report period*/
LgInt totEventErrCnt = 0; /* Event error counter for the entire period 
			    of stress test */

ESRO_FunctionalUnit funcUnit = ESRO_3Way;	/* 2-way/3-way handshake */
Bool waitForCompletion = TRUE;			/* Wait for completion of op */
Bool immediateInvoke = FALSE;
Bool justInvoke      = FALSE;

int
invokeInd (ESRO_SapDesc locSapDesc, ESRO_SapSel remESROSap, 
           T_SapSel *remTsap, N_SapAddr *remNsap, ESRO_InvokeId invokeId, 
           ESRO_OperationValue opValue, ESRO_EncodingType encodingType, 
           DU_View parameter);
int
resultInd (ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef,
           ESRO_EncodingType encodingType, 
           DU_View parameter);
int
errorInd (ESRO_InvokeId invokeId, 
	  ESRO_UserInvokeRef userInvokeRef,
          ESRO_EncodingType encodingType, 
          ESRO_ErrorValue errorValue, 
          DU_View parameter);
int 
resultCnf(ESRO_InvokeId invokeId,
	      ESRO_UserInvokeRef userInvokeRef);
int 
errorCnf(ESRO_InvokeId invokeId,
	     ESRO_UserInvokeRef userInvokeRef);
int 
failureInd(ESRO_InvokeId invokeId, 
	       ESRO_UserInvokeRef userInvokeRef,
	       ESRO_FailureValue failureValue);

char pConfigFile[512];			/* Configuration file */

char *__applicationName;		/* Application name */

#ifdef UDP_PO_
extern Bool UDP_noLogSw;		/* Switch to activate logging */
extern char eropEngOutLog[512];		/* ESROS engine output log file */
extern char eropEngErrLog[512];		/* ESROS engine error  log file */
#endif

typedef int *G_UserInvokeRef;		/* User invoke reference id */
int uInvokeRef = 0;
G_UserInvokeRef userInvokeRef = &uInvokeRef;

char traceFileName[256] = "";		/* Trace file name */
 
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
    Bool badUsage = FALSE;
    char tmpbuf[512];

    Char *copyrightNotice;


#ifdef MSDOS
    {
    extern void erop_force_msdos_link(void);
    erop_force_msdos_link();
    }
#endif

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
    TM_INIT();
#endif

    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:l:r:s:p:c:m:n:u:t:d:o:f:g:h:z:vwxyV")) 
	   != EOF) {
        switch (c) {
	case 'c':
	    strcpy(pConfigFile, optarg);
	    break;

	case 'V':		/* specify configuration directory */
	    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
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

    while ((c = getopt(argc, argv, "T:l:r:s:p:c:m:n:u:t:d:o:f:g:h:z:vwxyV")) 
	   != EOF) {
        switch (c) {
	case 'T':
	    TM_SETUP(optarg);
	    break;

	case 'l':  	/* Local ESRO Sap Selector */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 12, 0, 63) ) {
		EH_problem("main: Invalid local SAP selector");
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
		EH_problem("main: ");
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
		EH_problem("main: Invalid Transport SAP selector (UDP port no)");
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

	case 'v':
	    verbose = TRUE;
	    break;

	case 't':  	/* statistic reporting period */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 5000, CLOCK_PERIOD, 30000) ) {
		EH_problem("main: Invalid sttistic reporting period");
		badUsage = TRUE;
	    } else {
	         reportPeriod = gotVal;
	    }
	}
	    break;

	case 'd':  	/* inter-invocation delay */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, CLOCK_PERIOD, 100, 20000) ) {
		EH_problem("main: Invalid inter-invocation delay");
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
		EH_problem("main: Cannot open output file\n");
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

        case 'x':
	    immediateInvoke = TRUE;
	    break;

        case 'y':
	    justInvoke = TRUE;
	    break;

        case 'z':
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
	    TM_SETUP(optarg);
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

    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);

    fprintf(logFile, "\nRemote Network Address: %d.%d.%d.%d\n", 
	    remNsapAddr.addr[0], remNsapAddr.addr[1],
	    remNsapAddr.addr[2], remNsapAddr.addr[3]);

    G_init();

    OS_init();

#ifdef UDP_PO_
    fprintf(stderr, "**** stress_i.cb.one compiled <<<WITH UDP_PO>>> *****\n");
    if (!UDP_noLogSw) {
    	UDP_PO_init (eropEngErrLog, eropEngOutLog);
	fprintf(stdout, "===> Logging activated\n");
    }
#else
    fprintf(stderr, "**** stress_i.cb.one main compiled <<<WITHOUT UDP_PO>>> *****\n");
#endif


    G_duMainPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS); /* build buf pool */

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
 * Function:    G_init
 *
 * Description: Initialize.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

Void
G_init(void)
{
    int G_invoke(Ptr);
    int G_log(Ptr);

#ifdef TM_ENABLED
    if ( ! TM_OPEN (G_tmDesc, "G_") ) {
	EH_problem("G_init: TM_open G_ failed\n");
    }
#endif
    
#ifdef OS_TYPE_UNIX
    signal(SIGINT,  G_sigIntr);
    signal(SIGPIPE, G_sigPipe);
    signal(SIGTERM, G_sigTerm);
    signal(SIGKILL, G_sigKill);
#endif
    
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    signal(SIGINT, G_sigIntr);
#endif

    SCH_init(MAX_SCH_TASKS);

    TMR_init(NUMBER_OF_TIMERS, CLOCK_PERIOD);

    ESRO_CB_init(pConfigFile);

    TMR_startClockInterrupt(CLOCK_PERIOD);

    fprintf(stderr, "\nStress Tester with ESROS Call-Back API in one process\n");
    fprintf(stderr, "\nFunctional Unit: %d-Way handshaking\n", funcUnit);

    ESRO_CB_sapUnbind(locSapSel);

    if (ESRO_CB_sapBind(&locSapDesc, locSapSel, 
		  	funcUnit,
  		        invokeInd, 
	                resultInd, 
		        errorInd, resultCnf, 
			errorCnf, failureInd) < 0) {
        EH_fatal("invoke: Could not activate local ESRO SAP.");
    }

    fprintf(logFile,"\nReport Period = %d msec, Delay = %d msec\n",
	    reportPeriod, delay);
    fprintf( logFile, "\nInvocations\tInvocation Errors\tInvocation Failures\tEvents\tEvent Errors");
    fprintf( logFile, "\n=============================================================================");

    TMR_create(delay, G_invoke);
    TMR_create(reportPeriod, G_log);
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
    String usage1 = "[-T G_,ffff] [-l localEsroSapSel] [-r remoteEsroSapSel] [-p remotePortNu] -xwyz";
    String usage2 = "[-n remoteIPAddr] [-t reportPeriod] [-d delay] [-o logFile] -xwyz";
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

/*    totInvCnt += invCnt; */
    invCnt = 0;

    totInvErrCnt += invErrCnt;
    invErrCnt = 0;

    totInvFailCnt += invErrCnt;
    invFailCnt = 0;

    totEventCnt += eventCnt;
    eventCnt = 0;

    totEventErrCnt += eventErrCnt;
    eventErrCnt = 0;

    TMR_create(reportPeriod, G_log);
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
    DU_View invokeDU;
    int paramLength;
#ifdef TM_ENABLED
    static char taskN[100] = "G_invoke: ";
#endif

    do {

    	if (maxInvocations != -1) {
    	    if (++totInvCnt > maxInvocations) {
	    	totInvCnt--;
    	    	bTerminateRequested = TRUE;
            	return 0;
    	    }
    	}

    	paramLength = strlen(invokeParameter);
    	if ((invokeDU = DU_alloc(G_duMainPool, paramLength)) == NULL) {
	    EH_problem("DU_alloc failed\n");
    	    TMR_create(delay, G_invoke);
	    return -1;
        }

    	OS_copy(DU_data(invokeDU), invokeParameter, paramLength);

    	(*userInvokeRef) ++;

        TM_TRACE((G_tmDesc, TM_ENTER, 
	         "invoker: Issue InvokeReq: remEsroSapSel=%d, parameter=%s\n",
	         remEsroSapSel, invokeParameter));

    	if ((gotVal = ESRO_CB_invokeReq(&invokeId, 
    			            (ESRO_UserInvokeRef) userInvokeRef,
				    locSapDesc, remEsroSapSel,
			            &remTsapSel, &remNsapAddr, 
			            (ESRO_OperationValue) SHELL_CMD_OP, 
			 	    (ESRO_EncodingType) 2,  /* ASCII encoding */
			 	    invokeDU)) < 0) {

	    EH_problem("invoke: Could not Invoke\n");
	    ++invErrCnt;
	    DU_free(invokeDU);

	    if (justInvoke == TRUE) {
	    	break;
	    }
            TMR_create(delay, G_invoke);
	    return -1;
        }

    	++invCnt;
	++totInvCnt;

    	DU_free(invokeDU);

    } while (justInvoke);

    if (!waitForCompletion  && !justInvoke) {
    	TMR_create(delay, G_invoke);
    }

    if (justInvoke) {
#ifdef TM_ENABLED
    	SCH_submit ((Void *)G_invoke, 
		    NULL,
		    SCH_PSEUDO_EVENT, 
		    (String) strcat(taskN, TM_here()));
#else
    	SCH_submit ((Void *)G_invoke, 
		    NULL,
		    SCH_PSEUDO_EVENT);
#endif
    }

    return 0;
}

int
invokeInd (ESRO_SapDesc locSapDesc, ESRO_SapSel remESROSap, 
           T_SapSel *remTsap, N_SapAddr *remNsap, ESRO_InvokeId invokeId, 
           ESRO_OperationValue opValue, ESRO_EncodingType encodingType, 
           DU_View parameter)
{
    printf("\nInvoker: invoke Indication N/A\n");
    return 1;
}

static char separator[] = {"----------------------------------------------------------------------------------\n"};

int
resultInd (ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef,
           ESRO_EncodingType encodingType, 
           DU_View parameter)
{
        ++eventCnt;

 	if (verbose) {
	    char param[2048];
	    strncpy(param, (char *)DU_data(parameter), DU_size(parameter));
  	    param[DU_size(parameter)] = '\0';

	    printf("%s",separator);
	    printf("Invoker: Got Result Indication: invokeId=0x%lx, "
		   "userInvokeRef=%d, paramter=%s, Counter=%ld\n",
	           (unsigned long)invokeId,
	           *((int *)userInvokeRef),
	           param, ++counter);
            printf("%s",separator);
	 } else {
	     printf("\r%c",twirl());
	 }

        if (waitForCompletion) {
    	    TMR_create(delay, G_invoke);
        }
	return 0;
}


int
errorInd (ESRO_InvokeId invokeId, 
	  ESRO_UserInvokeRef userInvokeRef,
          ESRO_EncodingType encodingType, 
          ESRO_ErrorValue errorValue, 
          DU_View parameter)
{
    unsigned char *duData;

    ++eventCnt;

    if (parameter) {
    	duData = DU_data(parameter);
	*(duData+DU_size(parameter)) = '\0';

    } else {
	duData = "No parameter";
    }

    if (verbose) {
        printf("%s",separator);

	printf("Invoker: Got Error Indication: invokeId=0x%lx, "
	       "errorValue=%d, parameter=%s, counter=%ld\n", 
	       (unsigned long)invokeId, errorValue, duData, ++counter);
        printf("%s",separator);
    } else {
     	printf("\r%c",twirl());
    }

    if (waitForCompletion) {
      	TMR_create(delay, G_invoke);
    }
    return 0;
}


int
resultCnf(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    fprintf(stderr, "\nInvoker: resultCnf N/A\n");
    return 1;
}

int
errorCnf(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    fprintf(stderr, "\nInvoker: errorCnf N/A\n");
    return 1;
}


int
failureInd(ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef,
	   ESRO_FailureValue failureValue)
{
    ++eventCnt;
    ++invFailCnt;

    if (verbose) {
        printf("%s",separator);
	fprintf(stderr, "Invoker: Operation Failed: Failure Code %d\n", 
               failureValue);
        printf("%s",separator);
    } else {
     	printf("\r%c",twirl());
    }

    if (waitForCompletion) {
     	TMR_create(delay, G_invoke);
    }

    return 0;
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
static int cnt = 0;
static char thing[8] = {'|','/','-','\\','|','/','-','\\'};
return thing[cnt++ % 8];
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
