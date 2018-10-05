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
 * File: perfsch.c
 * 
 * Description: Performer with call back ESROS API (with scheduler)
 *
 * Functions:
 *    G_exit(Int code);
 *    G_usage(void);
 *    G_sigIntr(Int unused);
 *    resultInd ();
 *    errorInd ();
 *    invokeInd ();
 *    resultCnf();
 *    errorCnf();
 *    failureInd();
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
#include "pf.h"
#include "eh.h"
#include "du.h"
#include "getopt.h"
#include "tm.h"
#include "addr.h"
#include "inetaddr.h"
#include "sch.h"
#include "sf.h"
#include "udp_po.h"
#ifdef MSDOS
#include <time.h>
#endif

#include "target.h"
#include "esro_cb.h"

#include "extfuncs.h"

#include "relid.h"

PUBLIC DU_Pool *G_duMainPool;

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif

ESRO_SapSel locSapSel = 15;
ESRO_SapDesc locSapDesc;

ESRO_InvokeId invokeId;
ESRO_EncodingType encodingType;

ESRO_OperationValue operationValue;
#define SHELL_CMD_OP	2

ESRO_FunctionalUnit funcUnit = ESRO_3Way;

struct ESRO_Event event;

static OS_Boolean	bTerminateRequested = FALSE;

#define RES_RESULT	0
#define RES_ERROR	1
Int resErr = RES_RESULT;

char pConfigFile[512];

char *__applicationName;

extern resultInd ();
extern errorInd ();
#if 0  /*** use full prototypes or none ***/
extern invokeInd ();
#endif /*** 0 ***/
extern resultCnf();
extern errorCnf();
extern failureInd();

#ifdef UDP_PO_
extern Bool UDP_noLogSw;
extern char eropEngOutLog[512];
extern char eropEngErrLog[512];
#endif

typedef int *G_UserInvokeRef;
int uInvokeRef = 0;
G_UserInvokeRef userInvokeRef = &uInvokeRef;
 
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

Int
main(int argc, char **argv)
{
    Int c;
    Bool badUsage = FALSE;
    char tmpbuf[512];

    Char *copyrightNotice;

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_QuickWin)
    /* If we ever call exit(), prompt for whether to destroy windows */
    if (_wsetexit(_WINEXITPROMPT) != 0) {
    	EH_problem("wsetexit() failed");
    } else {
    	EH_message("Success", "wsetexit()");
    }
    if (_wabout("ESROS Single Performer\n"
		"Copyright (C) 1995-2001 Neda Communications, Inc. "
		"All rights reserved.") != 0) {
    	EH_problem("wabout() failed");
    } else {
    	EH_message("Success", "wabout()");
    }
#endif

    
    __applicationName = argv[0];

    TM_INIT();

    /* Get configuration file name */
    pConfigFile[0] = '\0';

    while ((c = getopt(argc, argv, "T:l:f:g:h:c:reV")) != EOF) {
	switch ( c ) {
	case 'c':		/* specify configuration directory */
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

    fprintf(stderr, "Config file name: %s\n", pConfigFile);

    optind = 0;			/* reset command line processing by getopt */

    while ((c = getopt(argc, argv, "T:l:f:g:h:c:reV")) != EOF) {
        switch (c) {
	case 'T':
	    (void) TM_SETUP(optarg);
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

    (void) TM_VALIDATE();

    printf("\nFunctional Unit: %d-Way handshaking\n", funcUnit);

    ESRO_CB_sapUnbind(locSapSel);

    if (ESRO_CB_sapBind(&locSapDesc, locSapSel, 
		  	funcUnit,
			invokeIndication, 
	        	resultInd, 
			errorInd, resultCnf, 
			errorCnf, failureInd) < 0) {
	EH_fatal("perform: Could not activate local ESRO SAP.");
    }

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
    }
      
    ESRO_CB_sapUnbind(locSapSel);

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

    return (0);

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
    String usage1 = "-l sap";
    String usage2 = "-l sap [-T ...]";
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
#ifndef WINDOWS
    signal(SIGINT, G_sigIntr);
#endif
    G_exit(22);
}

static void resultReq(ESRO_InvokeId invokeId, String result);
static void errorReq(ESRO_InvokeId invokeId, String error);

int
invokeIndication (ESRO_SapDesc locSapDesc, ESRO_SapSel remESROSap, 
           T_SapSel *remTsap, N_SapAddr *remNsap, ESRO_InvokeId invokeId, 
           ESRO_OperationValue opValue, ESRO_EncodingType encodingType, 
           DU_View parameter)
{
	TM_TRACE((G_tmDesc, TM_ENTER,
		 "Got ESRO-Invoke.Indcation invokeId=0x%lx, "
		 "operationValue=%d, paramter=%s\n",
		 invokeId,
		 opValue,
		 DU_data(parameter)));

	/* Here is where you perform what you should do
	 * and then call resultReq or errorReq.
	 */
        {
	    time_t idate;

            idate = time(&idate);

	    if (resErr == RES_RESULT) {
    		resultReq(invokeId, ctime(&idate));
	    } else {
    		errorReq(invokeId, ctime(&idate));
	    }
	}

        return 1;
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
    DU_View resultDU;
    int paramLength;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Result.Request parameter=%s\n",
	     result));

    paramLength = strlen(result);
    resultDU = DU_alloc(G_duMainPool, paramLength);
    OS_copy(DU_data(resultDU), result, paramLength);

    (*userInvokeRef)++;

     if ((gotVal = ESRO_CB_resultReq(invokeId, 
			             (ESRO_UserInvokeRef) userInvokeRef, 
				     (ESRO_EncodingType) 2, 
				     resultDU)) < 0) { 
	EH_problem("resultReq: resultReq failed\n");
	fprintf(stderr, "           invokeId=0x%lx\n",
		(unsigned long)invokeId);	
    }

    DU_free(resultDU);

    if (funcUnit == ESRO_2Way) {
	bTerminateRequested = TRUE;
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
    DU_View errorDU;
    int paramLength;

    TM_TRACE((G_tmDesc, TM_ENTER, "Issuing ESRO-Error.Request parameter=%s\n",
	     error));

    paramLength = strlen(error);
    errorDU = DU_alloc(G_duMainPool, paramLength);
    OS_copy(DU_data(errorDU), error, paramLength);

    (*userInvokeRef)++;

    if ((gotVal = ESRO_CB_errorReq(invokeId, 
			           (ESRO_UserInvokeRef) userInvokeRef, 
				   (ESRO_EncodingType) 2, 
				   0, errorDU)) < 0) {
	EH_problem("errorReq: Could not Invoke");
    }

    DU_free(errorDU);

    if (funcUnit == ESRO_2Way) {
	bTerminateRequested = TRUE;
    }
  
}

int
resultInd (ESRO_InvokeId invokeId, 
           ESRO_EncodingType encodingType, 
           DU_View parameter)
{
    unsigned char *duData;

    duData = DU_data(parameter);

	printf("-------------------------------------------------------------------\n");
	printf("Performer: Got Result Indication: invokeId=0x%lx, "
	       "paramter=%s\n",
		(unsigned long)invokeId, duData);
	printf("-------------------------------------------------------------------\n");
    return 1;
}


int
errorInd (ESRO_InvokeId invokeId, 
           ESRO_EncodingType encodingType, 
           ESRO_ErrorValue errorValue, 
           DU_View parameter)
{
    unsigned char *duData;

    duData = DU_data(parameter);

	printf("-------------------------------------------------------------------\n");
	printf("Performer: NA: Got Error Indication: invokeId=0x%lx, "
	       "paramter=%s\n",
	       (unsigned long)invokeId, duData);

	printf("-------------------------------------------------------------------\n");

    bTerminateRequested = TRUE;

    return 1;
}

int
resultCnf(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    TM_TRACE((G_tmDesc, TM_ENTER,
	      "Got ESRO-Result.Confirm invokeid=0x%lx, userInvokeRef=%d\n",
	      (unsigned long)invokeId, 
	      *((int *)userInvokeRef) ));

    printf("------------------------------------------------------\n");
    printf("Performer: Got Result Confirmation. invokeId=0x%lx, "
	   "userInvokeRef=%d\n",
	   (unsigned long)invokeId, *((int *)userInvokeRef) );
    printf("------------------------------------------------------\n");

    bTerminateRequested = TRUE;

    return 1;
}

int
errorCnf(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    TM_TRACE((G_tmDesc, TM_ENTER,
	      "Got ESRO-Error.Confirm invokeid=0x%lx, userInvokeRef=%d\n",
	      (unsigned long)invokeId, *((int *)userInvokeRef)));

    printf("------------------------------------------------------\n");
    printf("Performer: Got Error Confirmation. invokeId=0x%lx\n"
	   "userInvokeRef=%d\n",
	   (unsigned long)invokeId, *((int *)userInvokeRef) );
    printf("------------------------------------------------------\n");

    bTerminateRequested = TRUE;

    return 1;
}

int
failureInd(ESRO_InvokeId invokeId, ESRO_FailureValue failureValue)
{
    printf("\nPerformer: Failure Indication: InvokeId=0x%lx \n",
	   (unsigned long)invokeId);
    return 1;
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
