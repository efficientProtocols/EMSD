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
 * 
 * File: stress_p.c
 * 
 * Description: ESROS Stress Tester: Performer with Callback API.
 * 
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: stress_p.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "getopt.h"
#include "eh.h"
#include "tm.h"
#include "pf.h"
#include "du.h"
#include "sch.h"
#include "tmr.h"
#include "target.h"
#include "udp_po.h"

#include "esro_cb.h"
#include "sf.h"

#include "extfuncs.h"

#ifdef MSDOS
#include <time.h>
#endif

#include "relid.h"


static long int counter = 0;		/* Opertion counter */
PUBLIC DU_Pool *G_duMainPool;		/* Data Unit main pool */

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;		/* Global module trace descriptor */
#endif

ESRO_SapSel locSapSel = 15;		/* Local SAP selector */
ESRO_SapDesc locSapDesc;		/* Local SAP descriptor */
ESRO_OperationValue operationValue;	/* Operation value */

#define SHELL_CMD_OP	2

struct ESRO_Event event;		/* ESROS event */
unsigned reportPeriod = 5000;		/* Reporting period */
unsigned delay = 5000;			/* Delay between two operations */

FILE *logFile = stdout;			/* Log file */

LgInt invCnt    = 0;	/* Invocation counter for a report period */
LgInt totInvCnt = 0;	/* Total invocation counter for the entire period of stress testing*/
LgInt invErrCnt = 0;	/* Invocation error cournter for a report period */
LgInt totInvErrCnt = 0;	/* Total invocation error cournter for the entire period of stress test */
LgInt eventCnt     = 0;	/* Event counter for a report period */
LgInt totEventCnt  = 0;	/* Event counter for the entire period of stress test */
LgInt eventErrCnt  = 0;	/* Event error counter for a report period*/
LgInt totEventErrCnt = 0;/* Event error counter for the entire period of stress test */

ESRO_FunctionalUnit funcUnit = ESRO_3Way;	/* 2-way/3-way handshake */

#ifdef OS_TYPE_UNIX
Void G_sigIntr(Int unused);
Void G_sigPipe(Int unused);
Void G_sigTerm(Int unused);
Void G_sigKill(Int unused);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
Void G_sigIntr(Int unused);
#endif

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

static 
void resultReq(ESRO_InvokeId invokeId, 
		      ESRO_UserInvokeRef userInvokeRef, 
		      String result);
static 
void errorReq(ESRO_InvokeId invokeId, 
		     ESRO_UserInvokeRef userInvokeRef, 
		     Int errorValue, String error);

#define RES_RESULT	0
#define RES_ERROR	1
#define RES_TOGGLE	2
Int resErr = RES_RESULT;

SuccFail TMR_startClockInterrupt(Int);

char *__applicationName;
char pConfigFile[512];

#ifdef UDP_PO_
extern Bool UDP_noLogSw;
extern char eropEngOutLog[512];
extern char eropEngErrLog[512];
#endif

typedef int *G_UserInvokeRef;
int uInvokeRef = 0;
G_UserInvokeRef userInvokeRef = &uInvokeRef;

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
 * Description: main function of performer.
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

    if (_wabout("ESROS Stress Performer\n"
		"\n"
		"Copyright (C) 1997-2002 Neda Communications, Inc. "
		"All rights reserved.") != 0) {
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
    while ((c = getopt(argc, argv, "g:y:h:T:l:s:c:f:o:retV")) != EOF) {
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

    while ((c = getopt(argc, argv, "g:y:h:T:l:s:f:c:o:retV")) != EOF) {
        switch (c) {
	case 'T':
	    TM_SETUP(optarg);
	    break;

#if 0
	case 'p':	/* Remote Transport Sap Selector, UDP PortNu */
	{
	    Int portNu;

	    if ( PF_getInt(optarg, &portNu, 0, 0, 10000) ) {
		EH_problem("main: Invalid Transport SAP selector (UDP port no)");
		badUsage = TRUE;
	    } else {
		INET_portNuToTsapSel((MdInt) portNu, &udpSapSel);
	    }
	}
	    break;
#endif

	case 'l':  	/* Local ESRO Sap Selector */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 13, 0, 63) ) {
		EH_problem("main (performer): ");
		badUsage = TRUE;
	    } else {
		locSapSel = gotVal;
	    }
	}
	    break;

        case 'r':
	    resErr = RES_RESULT;
	    break;

        case 'e':
	    resErr = RES_ERROR;
	    break;

        case 't':
	    resErr = RES_TOGGLE;
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

	case 'o':  	/* outputfile */
	{
	    if ( ( logFile = fopen(optarg, "a") ) == NULL ) {
		EH_problem("main: Cannot open output file\n");
		badUsage = TRUE;
		logFile = stdout;
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

        case 'y':
	    if ( strlen(optarg) > 0 ) {
		strcpy(traceFileName, optarg);
	    } else {
		EH_problem("main: Invalid Trace file name\n");
		badUsage = TRUE;
	    }

        case 'c':
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

    G_init();

    OS_init();

#ifdef UDP_PO_
    fprintf(stderr, "**** stress_p.cb.one compiled <<<WITH UDP_PO>>> *****\n");
    if (!UDP_noLogSw) {
    	UDP_PO_init (eropEngErrLog, eropEngOutLog);
	fprintf(stdout, "===> Logging activated\n");
    }
#else
    fprintf(stderr, "\n**** stress_p.cb.one main compiled <<<WITHOUT UDP_PO>>> *****\n");
#endif

    G_duMainPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS); /* build buf pool */

    /* Do here what needs to be done */

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

    fprintf(stderr, "\nStress Tester with ESROS Call-Back API\n\n");
    fprintf(stderr, "\nFunctional Unit: %d-Way handshaking\n\n", funcUnit);

    ESRO_CB_sapUnbind(locSapSel);

    if (ESRO_CB_sapBind(&locSapDesc, locSapSel, 
		  	funcUnit,
  		        invokeInd, 
	                resultInd, 
		        errorInd, resultCnf, 
			errorCnf, failureInd) < 0) {
        EH_problem("invoke: Could not activate local ESRO SAP.");
        G_exit(13);
    }

    fprintf( logFile, "\nInvocations\tInvocation Errors\tEvents\tEvent Errors");
    fprintf( logFile, "\n=============================================================================");
    TMR_create(CLOCK_PERIOD, G_log);
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
    String usage1 = "";
    String usage2 = "";
    fprintf(stderr, "%s: Usage: %s\n", __applicationName, usage1);
    fprintf(stderr, "%s: Usage: %s\n", __applicationName, usage2);
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
G_log(tmrData)
Ptr tmrData;
{
    fprintf(logFile, "\n\t%ld\t\t%ld\t\t%ld\t\t%ld", invCnt, invErrCnt, eventCnt, eventErrCnt);
    /*  fflush(logFile); */

    totInvCnt += invCnt;
    invCnt = 0;

    totInvErrCnt += invErrCnt;
    invErrCnt = 0;

    totEventCnt += eventCnt;
    eventCnt = 0;

    totEventErrCnt += eventErrCnt;
    eventErrCnt = 0;


    TMR_create(reportPeriod, G_log);
    return 0;
}


int
invokeInd (ESRO_SapDesc locSapDesc, ESRO_SapSel remESROSap, 
           T_SapSel *remTsap, N_SapAddr *remNsap, ESRO_InvokeId invokeId, 
           ESRO_OperationValue opValue, ESRO_EncodingType encodingType, 
           DU_View parameter)
{
	static toggleSw = 1;

	TM_TRACE((G_tmDesc, TM_ENTER,
		 "ProcessEvent:Got ESRO-Invoke.Indcation invokeId=0x%lx, "
		 "operationValue=%d, paramter=%s\n",
		 invokeId, opValue, DU_data(parameter)));

  	++invCnt;
        ++eventCnt;

        {
	    time_t idate;
	    char *pctime;

            time(&idate);
	    pctime = ctime(&idate);
	    *(pctime+(strlen(pctime)-1)) = '\0';
            (*userInvokeRef)++;

	    switch (resErr) {
		case RES_RESULT:
     		    resultReq(invokeId, 
			      (ESRO_UserInvokeRef) userInvokeRef, 
			      pctime);
		    break;

		case RES_ERROR:
    		    errorReq(invokeId, 
			     (ESRO_UserInvokeRef) userInvokeRef, 
			     1, pctime);
		    break;

		case RES_TOGGLE:
		    if (toggleSw) {
     		        resultReq(invokeId, 
				  (ESRO_UserInvokeRef) userInvokeRef, 
			          pctime);
		    	toggleSw = 0;
		    } else {
    		        errorReq(invokeId, 
				 (ESRO_UserInvokeRef) userInvokeRef, 
			         1, pctime);
			toggleSw = 1;
		    }
		    break;
	    }
	}

	return SUCCESS;
}

int
resultInd (ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef,
           ESRO_EncodingType encodingType, 
           DU_View parameter)
{
	return SUCCESS;
}

int
errorInd (ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef,
          ESRO_EncodingType encodingType, 
          ESRO_ErrorValue errorValue, 
          DU_View parameter)
{
	return SUCCESS;
}

int
resultCnf(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
	counter++;
	TM_TRACE((G_tmDesc, TM_ENTER,
		 "processEvent: Got ESRO-Result.Confirm invokeid=0x%lx, "
		 "userInvokeRef=%d, counter=%ld\n",
		 invokeId,
	         *((int *)userInvokeRef), 
		 counter));

	fprintf(stderr, 
		"------------------------------------------------------------------\n");
	fprintf(stderr, "Performer: Got Result Confirmation. invokeId=0x%lx, "
	   	        "userInvokeRef=%d,  Counter=%ld\n",
		        (unsigned long)invokeId, 
	                *((int *)userInvokeRef), 
			counter);
	fprintf(stderr, 
		"------------------------------------------------------------------\n");
	return SUCCESS;
}

int
errorCnf(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
	counter++;
        ++eventCnt;

	TM_TRACE((G_tmDesc, TM_ENTER,
		 "processEvent: Got ESRO-Error.Confirm invokeid=0x%lx, "
		 "userInvokeRef=%d, counter=%ld\n",
		 (unsigned long)invokeId,
	         *((int *)userInvokeRef), 
		 counter));

	fprintf(stderr, 
		"------------------------------------------------------------------\n");
	fprintf(stderr, "Performer: Got Error Confirmation. invokeId=0x%lx, "
	   	        "userInvokeRef=%d,  Counter=%ld\n",
		        (unsigned long)invokeId,
	                *((int *)userInvokeRef), 
			counter);
	fprintf(stderr, 
		"------------------------------------------------------------------\n");
	return SUCCESS;
}

int
failureInd(ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef,
	   ESRO_FailureValue failureValue)
{
	TM_TRACE((G_tmDesc, TM_ENTER,
		 "processEvent: Got ESRO-Failure.Indication invokeid=0x%lx\n",
		 invokeId));

        ++eventCnt;

	return SUCCESS;
}


/*<
 * Function:    resultReq
 *
 * Description: Result request.
 *
 * Arguments:   Result.
 *
 * Returns:     None.
 * 
>*/

static void
resultReq(ESRO_InvokeId invokeId, 
	  ESRO_UserInvokeRef userInvokeRef, 
	  String result)
{
    ESRO_RetVal gotVal;
    DU_View resultDU;
    int paramLength;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Result.Request parameter=%s\n",
	     result));

    paramLength = strlen(result) + 1;

    if ((resultDU = DU_alloc(G_duMainPool, paramLength)) == NULL) {
	EH_fatal("DU_alloc failed\n");
    }

    OS_copy(DU_data(resultDU), result, paramLength);

    if ((gotVal = ESRO_CB_resultReq(invokeId, 
				    userInvokeRef, 
				    (ESRO_EncodingType) 2, resultDU)) < 0) {
	EH_problem("resultReq: Could not Invoke");
        ++eventErrCnt;
    }

    ++eventCnt;
  
    DU_free(resultDU);
}



/*<
 * Function:    errorReq
 *
 * Description: Error request.
 *
 * Arguments:   Error.
 *
 * Returns:     None.
 *
>*/

static void
errorReq(ESRO_InvokeId invokeId, 
	 ESRO_UserInvokeRef userInvokeRef, 
	 Int errorValue, String error)
{
    ESRO_RetVal gotVal;
    DU_View errorDU;
    int paramLength;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Error.Request parameter=%s\n",
	     error));

    paramLength = strlen(error);

    if ((errorDU = DU_alloc(G_duMainPool, paramLength)) == NULL) {
	EH_fatal("DU_alloc failed\n");
    }

    OS_copy(DU_data(errorDU), error, paramLength);

    if ((gotVal = ESRO_CB_errorReq(invokeId, 
				   userInvokeRef,
				   (ESRO_EncodingType) 2, 
				   errorValue, 
				   errorDU)) < 0) {
        ++eventErrCnt;
	EH_problem("errorReq: ESRO_CB_errorReq failed\n");
    }

    ++eventCnt;
  
    DU_free(errorDU);
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
