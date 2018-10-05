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
 * Description: Stress Performer (ESROS).
 * 
-*/


#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: stress_p.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "target.h"
#include "estd.h"
#include "getopt.h"
#include "eh.h"
#include "tm.h"
#include "pf.h"
#include "udp_po.h"
#include "sch.h"

#include "esro.h"
#include "extfuncs.h"

extern char pubQuName[];		/* Public Queue name */

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif

ESRO_SapSel locSapSel = 15;
ESRO_SapDesc locSapDesc;

ESRO_InvokeId invokeId;
ESRO_EncodingType encodingType;

ESRO_OperationValue operationValue;
#define SHELL_CMD_OP	2

struct ESRO_Event event;

/* Static functions */
static Int      perform(void);

#define RES_RESULT	0
#define RES_ERROR	1
#define RES_TOGGLE	2
Int resErr = RES_RESULT;

ESRO_FunctionalUnit funcUnit = ESRO_3Way;

char pConfigFile[512];

char *__applicationName;

#ifdef UDP_PO_
extern Bool UDP_noLogSw;
extern char eropEngOutLog[512];
extern char eropEngErrLog[512];
#endif

char traceFileName[256] = "";

#ifdef OS_TYPE_UNIX
Void G_sigIntr(Int unused);
Void G_sigPipe(Int unused);
Void G_sigTerm(Int unused);
Void G_sigKill(Int unused);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
Void G_sigIntr(Int unused);
#endif


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
    Bool badUsage; 
    
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
    if (_wabout("ESROS Stress Performer\n"
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
    while ((c = getopt(argc, argv, "T:y:g:h:c:l:s:f:retV")) != EOF) {
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

    while ((c = getopt(argc, argv, "T:y:g:h:c:l:s:f:ret")) != EOF) {
        switch (c) {
	case 'T':
#ifdef TM_ENABLED
	    TM_setUp(optarg);
#endif
	    break;

	case 'l':  	/* Local ESRO Sap Selector */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 13, 0, 63) ) {
		EH_problem("main (performer): PF_getInt call failed for command line arg 'l'");
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

	case 's':
	    if (strlen(optarg) != 0)
	    	strcpy(pubQuName, optarg);
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

    if ( ! (copyrightNotice = RELID_getCopyrightNedaAtt()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);

#ifdef OS_TYPE_UNIX
    signal(SIGINT,  G_sigIntr);
    signal(SIGPIPE, G_sigPipe);
    signal(SIGTERM, G_sigTerm);
    signal(SIGKILL, G_sigKill);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    signal(SIGINT, G_sigIntr);
#endif

    G_init();

    OS_init();

#ifdef TM_ENABLED
    TM_config(traceFileName);
    TM_VALIDATE();
#endif

    /* Do here what needs to be done */
    {
    	fprintf(stderr, "\nStress Tester with ESROS 'getEvent' based API (one process)\n");
    	fprintf(stderr, "\nFunctional Unit: %d-Way handshaking\n", funcUnit);

	perform();
    }
    
    exit (0);

} /* main() */


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


static void resultReq(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef, String result);
static void errorReq(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef, Int errorValue, String error);


/*<
 * Function:    perform()
 * 
 * Description: Perform an operation.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

static Int
perform(void)
{
    ESRO_RetVal gotVal;

    ESRO_sapUnbind(locSapSel);

    if ((gotVal = ESRO_sapBind(&locSapDesc, locSapSel, funcUnit) ) < 0 ) {
	EH_fatal("perform: Could not activate local ESRO SAP");
    }
   
    while ( TRUE ) {
	TM_TRACE((G_tmDesc, TM_ENTER, "perform: Waiting for an event\n"));

	while ((gotVal = ESRO_getEvent(locSapDesc, &event, 1)) < 0) {
	    if (gotVal == -2) {
	    	EH_fatal("Connection lost\n");
	    }
	}

    	processEvent(&event);
    }

} /* perform() */


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

    static toggleSw = 1;

    switch (p_event->type) {
    case ESRO_INVOKEIND:

	TM_TRACE((G_tmDesc, TM_ENTER,
		 "Got ESRO-Invoke.Indcation invokeId=%d, "
		 "operationValue=%d, paramter=%s\n",
		 p_event->un.invokeInd.invokeId,
		 p_event->un.invokeInd.operationValue,
		 p_event->un.invokeInd.data));
	/* Here is where you perform what you should do
	 * and then call resultReq or errorReq.
	 */
        invokeId = p_event->un.invokeInd.invokeId;

        {
	    time_t idate;
	    char *pctime;

            time(&idate);
	    pctime = ctime(&idate);
	    *(pctime+(strlen(pctime)-1)) = '\0';

	    switch (resErr) {
		case RES_RESULT:
     		        resultReq(invokeId, NULL, pctime);
			break;
		case RES_ERROR:
    		        errorReq(invokeId, NULL, 1, pctime);
			break;
		case RES_TOGGLE:
			if (toggleSw) {
     		       	    resultReq(invokeId, NULL, pctime);
			    toggleSw = 0;
			} else {
    		       	    errorReq(invokeId, NULL, 1, pctime);
			    toggleSw = 1;
			}
			break;
	    }
	}
	break;

    case ESRO_RESULTCNF:
	TM_TRACE((G_tmDesc, TM_ENTER,
		 "Got ESRO-Result.Confirm invokeid=%d\n",
		 p_event->un.resultCnf.invokeId));

	printf("%s", separator);
	printf("Performer: Got Result Confirmation. invokeId=%d, "
	       "op_counter=%ld\n",
		p_event->un.resultCnf.invokeId, ++op_counter);
	printf("%s", separator);
	break;

    case ESRO_ERRORCNF:
	printf("%s", separator);
	printf("Performer: Got Error Confirmation. invokeId=%d, "
	       "op_counter=%ld\n",
		p_event->un.errorCnf.invokeId, ++op_counter);
	printf("%s", separator);
	break;

    case ESRO_FAILUREIND:
    	TM_TRACE((G_tmDesc, TM_ENTER, "Operation failed\n"));
	break;

    default:
    	TM_TRACE((G_tmDesc, TM_ENTER, "Invalid event\n"));
	EH_problem("processEvent: invalid event");
	break;
    }
    return (SUCCESS);
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

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Result.Request parameter=%s\n",
	     result));

    if ((gotVal = ESRO_resultReq(invokeId, NULL, (ESRO_EncodingType) 2,
	 		         strlen(result), result)) < 0) {
    
	EH_problem("resultReq: ESRO_resultReq function failed\n");
    }
  
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
	 Int errorValue, 
	 String error)
{
    ESRO_RetVal gotVal;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Error.Request parameter=%s\n",
	     error));

    if ((gotVal = ESRO_errorReq(invokeId, 
				NULL,
				(ESRO_EncodingType) 2, 
				errorValue, 
			        strlen(error), error)) < 0) {

	EH_problem("errorReq: ESRO_errorReq failed\n");
    }
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
