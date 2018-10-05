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
 * Description: ESROS Stress Performer (two process, function call API)
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

#include "sch.h"
#include "esro.h"

extern char pubQuName[];			/* Public queue name */

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif

Void G_init(void);
Void G_exit(Int code);
Void G_usage(void);
Void G_sigIntr(Int unused);
SuccFail processEvent(struct ESRO_Event *p_event);

static void resultReq(ESRO_InvokeId invokeId, 
		      ESRO_UserInvokeRef userInvokeRef, 
		      String result);
static void errorReq(ESRO_InvokeId invokeId, 
		     ESRO_UserInvokeRef userInvokeRef, 
		     Int errorValue, String error);

char twirl(void);

ESRO_SapSel locSapSel = 15;		/* Local SAP selector */
ESRO_SapDesc locSapDesc;		/* Local SAP descriptor */

ESRO_InvokeId invokeId;			/* Invocation identifier */
ESRO_EncodingType encodingType;		/* Encoding type */

ESRO_OperationValue operationValue;	/* Opertion value */
#define SHELL_CMD_OP	2

struct ESRO_Event event;		/* ESROS event */

static Int      perform(void);

#define RESULT	0
#define ERROR	1
#define TOGGLE	2
Int resErr = RESULT;

ESRO_FunctionalUnit funcUnit = ESRO_3Way;	/* 2-way/3-way handshake */

typedef int *G_UserInvokeRef;			/* User invoke reference */
int uInvokeRef = 0;
G_UserInvokeRef userInvokeRef = &uInvokeRef;

Void G_sigIntr(Int unused);

#ifdef OS_TYPE_UNIX
Void G_sigPipe(Int unused);
Void G_sigTerm(Int unused);
Void G_sigKill(Int unused);
#endif

char *__applicationName;

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

    __applicationName = argv[0];

#ifdef TM_ENABLED
    TM_INIT();
#endif

    badUsage = FALSE;
    while ((c = getopt(argc, argv, "T:l:s:f:retV")) != EOF) {
        switch (c) {
	case 'T':
#ifdef TM_ENABLED
	    TM_SETUP(optarg);
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

	case 's':				/* Unix socket name (public queue) */
	    if (strlen(optarg) != 0)
	    	strcpy(pubQuName, optarg);
	    break;

        case 'r':
	    resErr = RESULT;
	    break;

        case 'e':
	    resErr = ERROR;
	    break;

        case 't':
	    resErr = TOGGLE;
	    break;

	case 'f':  	/* ESRO functional unit (2-way/3-way) */
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

	case 'V':		/* specify configuration directory */
	    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
		EH_fatal("main: Get copyright notice failed\n");
	    }
	    fprintf(stdout, "%s\n", copyrightNotice);
	    exit(0);

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

    /* Do here what needs to be done */
    {
	ESRO_init();

#ifdef TM_ENABLED
        TM_VALIDATE();
#endif

	perform();
    }
    
    exit (0);

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
#ifdef TM_ENABLED
    if (! (G_tmDesc = TM_open("G_")) ) {
	EH_problem("G_init : TM_open G_ failed");
    }
#endif
    
    signal(SIGINT,  G_sigIntr);

#ifdef OS_TYPE_UNIX
    signal(SIGPIPE, G_sigPipe);
    signal(SIGTERM, G_sigTerm);
    signal(SIGKILL, G_sigKill);
#endif

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
    String usage1 = "-l sap";
    String usage2 = "-l sap [-T ...]";
    printf("Usage: %s %s\n", __applicationName, usage1);
    printf("Usage: %s %s\n", __applicationName, usage2);
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
	while ((gotVal = ESRO_getEvent(locSapDesc, &event, 1)) < 0) {
	    if (gotVal == -2) {
	    	EH_fatal("Connection lost\n");
	    }
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
    static long int counter = 0;

    static toggleSw = 1;

    switch (p_event->type) {
    case ESRO_INVOKEIND:

	TM_TRACE((G_tmDesc, TM_ENTER,
		 "ProcessEvent:Got ESRO-Invoke.Indcation "
		 "invokeId=%d, operationValue=%d, paramter=%s\n",
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

            (*userInvokeRef)++;

	    switch (resErr) {
		case RESULT:
     		        resultReq(invokeId, 
				 (ESRO_UserInvokeRef) userInvokeRef,
				  pctime);
			break;
		case ERROR:
    		        errorReq(invokeId, 
				 (ESRO_UserInvokeRef) userInvokeRef, 
				 0, pctime);
			break;
		case TOGGLE:
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

	break;

    case ESRO_RESULTCNF:
 	counter++;	
	TM_TRACE((G_tmDesc, TM_ENTER,
		 "processEvent: Got ESRO-Result.Confirm invokeId=%d, "
	         "userInvokeRef=%d, op_counter=%ld\n",
		 p_event->un.resultCnf.invokeId,
	         *((int *)p_event->un.resultCnf.userInvokeRef), 
		 counter));

	printf("------------------------------------------------------\n");
	printf("Performer: Got Result Confirmation. invokeId=%d, "
	       "userInvokeRef=%d, op_counter=%ld\n",
	       p_event->un.resultCnf.invokeId, 
	       *((int *)p_event->un.resultCnf.userInvokeRef), 
	       counter);
	printf("------------------------------------------------------\n");

	break;

    case ESRO_ERRORCNF:
	TM_TRACE((G_tmDesc, TM_ENTER,
		 "processEvent: Got ESRO-Error.Confirm invokeid=%d, "
	         "userInvokeRef=%d, op_counter=%ld\n",
		 p_event->un.errorCnf.invokeId,
	         *((int *)p_event->un.errorCnf.userInvokeRef), 
		 counter)); 

	printf("------------------------------------------------------\n");
	printf("Performer: Got Error Confirmation. invokeId=%d, "
	       "userInvokeRef=%d, op_counter=%ld\n",
	       p_event->un.errorCnf.invokeId, 
	       *((int *)p_event->un.errorCnf.userInvokeRef), 
	       counter);
	printf("------------------------------------------------------\n");

	break;

    case ESRO_FAILUREIND:
	TM_TRACE((G_tmDesc, TM_ENTER,
		 "processEvent: Got ESRO-Failure.Indication invokeid=%d, "
	         "userInvokeRef=%d\n",
		 p_event->un.failureInd.invokeId,
	         *((int *)p_event->un.failureInd.userInvokeRef))); 

	printf("------------------------------------------------------\n");
	printf("Performer: Got Failure indication. invokeId=%d, "
	       "userInvokeRef=%d, op_counter=%ld\n",
	       p_event->un.failureInd.invokeId, 
	       *((int *)p_event->un.failureInd.userInvokeRef), 
	       counter);
	printf("------------------------------------------------------\n");

	break;

    default:
	TM_TRACE((G_tmDesc, TM_ENTER,
		 "processEvent: Invalid Event received  invokeid=%d\n",
		 p_event->un.resultCnf.invokeId));
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

    gotVal = ESRO_resultReq(invokeId, 
			    userInvokeRef, 
			    (ESRO_EncodingType) 2,
			    strlen(result), (unsigned char *)result);
    
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
errorReq(ESRO_InvokeId invokeId, 
	ESRO_UserInvokeRef userInvokeRef, 
	Int errorValue, String error)
{
    ESRO_RetVal gotVal;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Error.Request parameter=%s\n",
	     error));

    if ((gotVal = ESRO_errorReq(invokeId, 
				userInvokeRef, 
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
