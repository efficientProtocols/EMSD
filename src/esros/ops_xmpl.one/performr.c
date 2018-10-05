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
 * File: performer.c
 * 
 * Description: Performer (ESROS).
 * 
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: performr.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "oe.h"
#include "target.h"
#include "getopt.h"
#include "eh.h"
#include "tm.h"
#include "pf.h"
#include "sch.h"
#include "udp_po.h"
#ifdef MSDOS
#include <time.h>
#endif

#include "esro.h"
#include "extfuncs.h"

Void G_usage(void);
Void G_init(void);
Void G_exit(Int code);
Void G_sigIntr(Int unused);

extern Int udpSapSel;

#ifdef TM_ENABLED
extern TM_ModDesc G_tmDesc;
#endif

static Int perform(void);

ESRO_SapSel locSapSel = 13;
ESRO_SapDesc locSapDesc;

ESRO_InvokeId invokeId;
ESRO_EncodingType encodingType;

ESRO_OperationValue operationValue;
#define SHELL_CMD_OP	2

struct ESRO_Event event;

#define RES_RESULT	0
#define RES_ERROR	1
Int resErr = RES_RESULT;

char *__applicationName;

char pConfigFile[256] = "./erop.ini";

ESRO_FunctionalUnit funcUnit = ESRO_3Way;

typedef int *G_UserInvokeRef;
int uInvokeRef = 0;
G_UserInvokeRef userInvokeRef = &uInvokeRef;

#ifdef UDP_PO_
extern Bool UDP_noLogSw;
extern char eropEngOutLog[512];
extern char eropEngErrLog[512];
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

Int main(int argc, char **argv)
{
    Int c;
    Bool badUsage; 

    Char *copyrightNotice;
    EXTERN Char *RELID_getRelidNotice(Void);
    EXTERN Char *RELID_getRelidNotice(Void);
    EXTERN Char *RELID_getRelidNotice(Void);
    
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_QuickWin)
    /* If we ever call exit(), prompt for whether to destroy windows */
    if (_wsetexit(_WINEXITPROMPT) != 0) {
    	EH_problem("wsetexit() failed");
    } else {
    	EH_message("Success", "wsetexit()");
    }
    if (_wabout("ESROS Single Performer\n"
		"\n"
		""
		"") != 0) {
    	EH_problem("wabout() failed");
    } else {
    	EH_message("Success", "wabout()");
    }
#endif

    __applicationName = argv[0];

#ifdef TM_ENABLED
    TM_INIT();
#endif

    badUsage = FALSE;

    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:c:f:g:h:l:e:o:p:V")) != EOF) {
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

    while ((c = getopt(argc, argv, "T:c:f:g:h:l:e:o:p:V")) != EOF) {
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
		EH_problem("main (performer): ");
		badUsage = TRUE;
	    } else {
		locSapSel = gotVal;
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

	case 'p':
	    if (PF_getInt(optarg, &udpSapSel, 2002, 2000, 4000) ) {
		EH_problem("main: Invalid port number");
	    	exit(1);
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

        case 'r':
	    resErr = RES_RESULT;
	    break;

        case 'e':
	    resErr = RES_ERROR;
	    break;

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

    printf("\nFunctional Unit: %d-Way handshaking\n", funcUnit);

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);
#endif

    perform();
    
    exit (0);

} /* main() */


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
    String usage1 = "[-T G_,ffff]";
    String usage2 = "-l localEsroSapSel -p remotePortNu";
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
    ESRO_sapUnbind(locSapSel);
    G_exit(22);
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

static void resultReq(ESRO_InvokeId invokeId, String result);
static void errorReq(ESRO_InvokeId invokeId, String error);


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

    if ((gotVal = ESRO_sapBind(&locSapDesc, locSapSel, funcUnit)) < 0) {
	EH_fatal("perform: Could not activate local ESRO SAP.");
    }

    TM_TRACE((G_tmDesc, TM_ENTER, "Waiting for event\n"));
   
    while ( TRUE ) {
	while ((gotVal = ESRO_getEvent(locSapDesc, &event, FALSE)) < 0) { 
#if defined(MSDOS) && !defined(WINDOWS)
	    if (kbhit())
		return 0;
#endif
	}

	processEvent(&event);
    }
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

            idate = time(&idate);

    	    if (resErr == RES_RESULT) {
     	    	resultReq(invokeId, ctime(&idate));
	    } else {
    	    	errorReq(invokeId, ctime(&idate));
	    } 
	}
	break;

    case ESRO_RESULTCNF:

	TM_TRACE((G_tmDesc, TM_ENTER,
		 "Got ESRO-Result.Confirm invokeId=%d, userInvokeRef=%d\n",
		 p_event->un.resultCnf.invokeId,
		 *((int *)p_event->un.resultCnf.userInvokeRef)));

	printf("------------------------------------------------------\n");
	printf("Performer: Got Result Confirmation. invokeId=%d, "
	       "userInvokeRef=%d\n",
		p_event->un.resultCnf.invokeId,
		*((int *)p_event->un.resultCnf.userInvokeRef));
	printf("------------------------------------------------------\n");
	G_exit(0);
	break;

    case ESRO_ERRORCNF:

	TM_TRACE((G_tmDesc, TM_ENTER,
		 "Got ESRO-Error.Confirm invokeId=%d, userInvokeRef=%d\n",
		 p_event->un.errorCnf.invokeId,
		 *((int *)p_event->un.errorCnf.userInvokeRef)));

	printf("------------------------------------------------------\n");
	printf("Performer: Got Error Confirmation. invokeId=%d, "
	       "userInvokeRef=%d\n",
		p_event->un.errorCnf.invokeId,
		*((int *)p_event->un.errorCnf.userInvokeRef));
	printf("------------------------------------------------------\n");
	G_exit(0);
	break;
    case ESRO_FAILUREIND:
	TM_TRACE((G_tmDesc, TM_ENTER, "Performer: Got Failure Indication.\n"));
	break;
    default:
	TM_TRACE((G_tmDesc, TM_ENTER, "Performer: Invalid event type.\n"));
	EH_problem("processEvent: invalid event type");
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
resultReq(ESRO_InvokeId invokeId, String result)
{
    ESRO_RetVal gotVal;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Result.Request parameter=%s\n",
	     result));

    (*userInvokeRef)++;

    gotVal = ESRO_resultReq(invokeId, 
			    (ESRO_UserInvokeRef) userInvokeRef, 
			    (ESRO_EncodingType) 2,
			    strlen(result), result);
    
    if (gotVal < 0) {
	EH_problem("resultReq: Could not Invoke");
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
errorReq(ESRO_InvokeId invokeId, String error)
{
    ESRO_RetVal gotVal;

    (*userInvokeRef)++;

    if ((gotVal = ESRO_errorReq(invokeId, 
			   (ESRO_UserInvokeRef) userInvokeRef, 
			   (ESRO_EncodingType) 2,
			   1, strlen(error), error)) < 0) {
	EH_problem("errorReq: Could not Invoke");
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
