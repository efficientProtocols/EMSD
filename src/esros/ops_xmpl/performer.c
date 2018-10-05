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
static char rcs[] = "$Id: performer.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#ifdef OS_MALLOC_DEBUG
#include "os.h"

#undef FAIL
#endif

char *__applicationName;

#include "estd.h"
#include "getopt.h"
#include "eh.h"
#include "tm.h"
#include "pf.h"

#include "esro.h"
#include "extfuncs.h"

extern char pubQuName[];

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif

ESRO_SapSel locSapSel = 15;
ESRO_SapDesc locSapDesc;

ESRO_InvokeId invokeId;

typedef int *G_UserInvokeRef;
int uInvokeRef = 0;
G_UserInvokeRef userInvokeRef = &uInvokeRef;

ESRO_EncodingType encodingType;

ESRO_OperationValue operationValue;
#define SHELL_CMD_OP	2

struct ESRO_Event event;

/* Static functions */
static Int      perform(void);

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

    while ((c = getopt(argc, argv, "T:l:s:V")) != EOF) {
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

	case 's':
	    if (strlen(optarg) != 0)
	    	strcpy(pubQuName, optarg);
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
	TM_validate();
#endif

	perform();
    }
    
    return (0);

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
    if (! TM_OPEN(G_tmDesc, "G_")) {
	EH_problem("G_init : TM_open G_ failed");
    }
#endif
    
    signal(SIGINT, G_sigIntr);
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
    String usage1 = "-l sap [-T ...]";
    String usage2 = "-l sap";

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
G_sigIntr(Int unUsed)
{
    signal(SIGINT, G_sigIntr);
    G_exit(22);
}

static void resultReq(String result);
static void errorReq(String result, ESRO_ErrorValue errorValue);


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

    if ((gotVal = ESRO_sapBind(&locSapDesc, locSapSel, ESRO_3Way)) < 0) {
	EH_fatal("perform: Could not activate local ESRO SAP.");
    }

   
    while ( TRUE ) {
	while ((gotVal = ESRO_getEvent(locSapDesc, &event, 1)) < 0) {  /*????*/
	    EH_problem("perform: ESRO_getEvent failed");
	    return (FAIL);
	}
	if (gotVal < 0) {
	    EH_problem("perform: Bad Event");
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

	*(p_event->un.invokeInd.data + p_event->un.invokeInd.len) = '\0';

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
	    resultReq(ctime(&idate));  
	}
	break;

    case ESRO_RESULTCNF:

	TM_TRACE((G_tmDesc, TM_ENTER,
		 "Got ESRO-Result.Confirm invokeid=%d, userInvokeRef=%d\n",
		 p_event->un.resultCnf.invokeId,
		 *((int *)p_event->un.resultCnf.userInvokeRef)));

	printf("------------------------------------------------------\n");
	printf("Performer: Got Result Confirmation. invokeId=%d, userInvokeRef=%d\n",
		p_event->un.resultCnf.invokeId,
		*((int *)p_event->un.resultCnf.userInvokeRef));
	printf("------------------------------------------------------\n");
	G_exit(0);

    case ESRO_ERRORCNF:
	TM_TRACE((G_tmDesc, TM_ENTER, "Got ESRO-Error.Confirm\n"));
	break;

    case ESRO_FAILUREIND:
	TM_TRACE((G_tmDesc, TM_ENTER, "Got ESRO-Failure.Indication\n"));
	break;

    default:
	TM_TRACE((G_tmDesc, TM_ENTER, "Invalid event.\n"));
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
resultReq(String result)
{
    ESRO_RetVal gotVal;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Result.Request: parameter=%s\n",
	     result));

    (*userInvokeRef)++;
    gotVal = ESRO_resultReq(invokeId, 
			    (ESRO_UserInvokeRef)userInvokeRef, 
			    (ESRO_EncodingType) 2,
			    (unsigned char *)strlen(result), result);
    
    if (gotVal < 0) {
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
errorReq(String error, ESRO_ErrorValue errorValue)
{
    ESRO_RetVal gotVal;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Error.Request: parameter=%s\n",
	     error));

    (*userInvokeRef)++;
    if ((gotVal = ESRO_errorReq(invokeId, 
				userInvokeRef, 
				(ESRO_EncodingType) 2,
			        errorValue,
			        (unsigned char *)strlen(error), error)) < 0) {

	EH_problem("errorReq: ESRO_errorReq failed\n");
    }
}
