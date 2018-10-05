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
 * File: invoksch.c
 * 
 * Description: Invoker with ESROS Call back API (with scheduler)
 *
 * Functions:
 *    G_exit(Int code);
 *    G_usage(void);
 *    G_sigIntr(Int unused);
 *    G_heartBeat(void);
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
static char rcs[] = "$Id: invoker.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
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
#include "udp_po.h"

#include "target.h"
#include "esro_cb.h"

#include "extfuncs.h"

#include "relid.h"

PUBLIC DU_Pool *G_duMainPool;

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif

ESRO_SapSel locSapSel = 12;
ESRO_SapDesc locSapDesc;

ESRO_InvokeId invokeId;

typedef int *G_UserInvokeRef;
int uInvokeRef = 0;
G_UserInvokeRef userInvokeRef = &uInvokeRef;

ESRO_EncodingType encodingType;

ESRO_OperationValue operationValue;
#define SHELL_CMD_OP	2

ESRO_SapSel remEsroSapSel = 13;
T_SapSel  remTsapSel  = {2, {0x07, 0xD2}      }; /* UDP Port Number (2002) */
N_SapAddr remNsapAddr = {4, {198, 62, 92, 14} }; /* Remote IP Address */

ESRO_FunctionalUnit funcUnit = ESRO_3Way;

struct ESRO_Event event;

char pConfigFile[512];

char *__applicationName;

extern G_heartBeat(void);
extern resultInd ();
extern errorInd ();
extern resultCnf();
extern errorCnf();
extern failureInd();

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
 * Description: main function of invoksch.
 *
 * Arguments:   argc, argv.
 *
 * Returns:     None.
 *
>*/

Int main(int argc, char **argv)
{
    Int c;
    Bool badUsage = FALSE;

    Char *copyrightNotice;
    
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_QuickWin)
    /* If we ever call exit(), prompt for whether to destroy windows */
    if (_wsetexit(_WINEXITPROMPT) != 0) {
    	EH_problem("wsetexit() failed");
    } else {
    	EH_message("Success", "wsetexit()");
    }
    if (_wabout("ESROS Single Invoker\n"
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
    while ((c = getopt(argc, argv, "T:l:c:r:f:p:n:g:h:uV")) != EOF) {
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

    optind = 0;			/* reset command line processing by getopt */

    while ((c = getopt(argc, argv, "T:l:c:r:f:p:n:g:h:u")) != EOF) {
        switch (c) {
	case 'T':
	    (void) TM_SETUP(optarg);
	    break;
	case 'l':  	/* Local ESRO Sap Selector */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 12, 0, 63) ) {
		EH_problem("main: ");
		badUsage = TRUE;
	    } else {
		locSapSel = gotVal;
	    }
	}
	    break;

	case 'r':	/* Remore ESRO Sap Selector */
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

	case 'p':	/* Remore Transport Sap Selector, UDP PortNu */
	{
	    Int portNu;

	    if ( PF_getInt(optarg, &portNu, 0, 0, 10000) ) {
		EH_problem("main: ");
		badUsage = TRUE;
	    } else {
		INET_portNuToTsapSel((MdInt) portNu, &remTsapSel);
	    }
	}
	    break;

	case 'n':	/* Remore NSAP Address, NSAP Address */
	{
	    struct in_addr inetAddr;

	    * ((LgInt *) &inetAddr) = inet_addr(optarg);
	    INET_in_addrToNsapAddr(&inetAddr, &remNsapAddr);
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

    while ((c = getopt(argc, argv, "T:u")) != EOF) {
        switch (c) {
	case 'T':
	    (void) TM_SETUP(optarg);
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

    /* Application Specific Code Goes Here */
    {
        String invokeParameter = "date";
        DU_View invokeDU;
        int paramLength;

	printf("\nRemote Network Address: %d.%d.%d.%d\n", remNsapAddr.addr[0],
		remNsapAddr.addr[1], remNsapAddr.addr[2], remNsapAddr.addr[3]);
	printf("\nFunctional Unit: %d-Way handshaking\n", funcUnit);

	/* Initialize All relevant modules */
	(void) TM_VALIDATE();


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

        TM_TRACE((G_tmDesc, TM_ENTER, 
	         "invoker: Issue InvokeReq: remSapSel=%d, "
		 "locSapDesc=%d, param=%s\n",
	         remEsroSapSel, locSapDesc, invokeParameter));

	paramLength = strlen(invokeParameter);
        invokeDU = DU_alloc(G_duMainPool, paramLength);
	OS_copy(DU_data(invokeDU), invokeParameter, paramLength);

        (*userInvokeRef) ++;
        if (ESRO_CB_invokeReq(&invokeId, 
    			      (ESRO_UserInvokeRef) userInvokeRef,
			      locSapDesc, 
		 	      remEsroSapSel,
			      &remTsapSel, &remNsapAddr, 
			      (ESRO_OperationValue) SHELL_CMD_OP, 
			      (ESRO_EncodingType) 2,  /* ASCII encoding */
			      invokeDU) < 0) {
	    EH_problem("invoke: Could not Invoke");
	    G_exit(11);
        }

	DU_free(invokeDU);

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);
#endif

        while (!bTerminateRequested) {
	    if (G_heartBeat()) {
    	        G_exit(13);
	    }
        }


        ESRO_CB_sapUnbind(locSapSel);

    }
    return (SUCCESS);

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
    String usage1 = "[-T G_,ffff]";
    String usage2 = "-l localEsroSapSel -r remoteEsroSapSel -p remotePortNu -n remoteIPAdde";
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
/*
    ESRO_sapUnbind(locSapSel);	    commented out to test ill-behaved invokers
*/ 
#ifndef WINDOWS
    signal(SIGINT, G_sigIntr);
#endif
    G_exit(22);
}


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
    if (SCH_block() < 0) {
        EH_fatal("G_heartBeat: SCH_block returned negative value");
	return (FAIL);
    }

    SCH_run();

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    /* Allow CTRL-C actions to take place */
    kbhit();
#endif

    return (SUCCESS);
}


/*<
 * Function:    resultInd
 *
 * Description: Result indication (call back function)
 *
 * Arguments:   Invoke Id, encoding type, parameter.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

int
resultInd (ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef,
           ESRO_EncodingType encodingType, 
           DU_View parameter)
{
    unsigned char *duData;
    int duSize;

    if (parameter) {
    	duData = DU_data(parameter);
    	duSize = DU_size(parameter);

    } else {
	(char *)duData = "No parameter";
    	duSize = strlen("No parameter");
    }

    *(duData+duSize) = '\0';

    printf("-----------------------------------------------------------------------------\n");
    printf("Invoker: Got Result Indication: invokeId=0x%lx "
	   "userInvokeRef=%d, paramter=%s\n",
	   (unsigned long)invokeId, *((int *)userInvokeRef), duData);
    printf("-----------------------------------------------------------------------------\n");

    bTerminateRequested = TRUE;

    return 1;
}


/*<
 * Function:    errorInd
 *
 * Description: Error indication (call back function)
 *
 * Arguments:   Invoke Id, encoding type, error value, parameter.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

int
errorInd (ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef,
           ESRO_EncodingType encodingType, 
           ESRO_ErrorValue errorValue, 
           DU_View parameter)
{
    unsigned char *duData;

    if (parameter) {
    	duData = DU_data(parameter);

    } else {
	(char *)duData = "No parameter";
    }

    printf("-----------------------------------------------------------------------------\n");
    printf("Invoker: Got Error Indication: invokeId=0x%lx, "
	   "userInvokeRef=%d, paramter=%s\n",
	   (unsigned long)invokeId, *((int *)userInvokeRef), duData);

    printf("-----------------------------------------------------------------------------\n");

    bTerminateRequested = TRUE;

    return 1;
}


/*<
 * Function:    invokeInd
 *
 * Description: Invoke indication (call back function)
 *
 * Arguments:   Sap descriptor, remote sap selector, transport sap, network sap, 
 *		invoke id, operation value, encoding type, parameter
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

int
invokeInd (ESRO_SapDesc locSapDesc, ESRO_SapSel remESROSap, 
           T_SapSel *remTsap, N_SapAddr *remNsap, ESRO_InvokeId invokeId, 
           ESRO_OperationValue opValue, ESRO_EncodingType encodingType, 
           DU_View parameter)
{
    printf("\nInvoker: invoke Indication N/A\n");
    return 1;
}


/*<
 * Function:    resultCnf
 *
 * Description: Result Confirmation (call back function)
 *
 * Arguments:   Invoke id.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

int
resultCnf(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    printf("\nInvoker: resultCnf N/A\n");
    return 1;
}


/*<
 * Function:    errorCnf
 *
 * Description: Error Confirmation (call back function)
 *
 * Arguments:   Invoke id.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

int
errorCnf(ESRO_InvokeId invokeId, ESRO_UserInvokeRef userInvokeRef)
{
    printf("\nInvoker: errorCnf N/A\n");
    return 1;
}


/*<
 * Function:    failureInd
 *
 * Description: Failure indication (call back function)
 *
 * Arguments:   Invoke Id, failure value.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

int
failureInd(ESRO_InvokeId invokeId, 
	   ESRO_UserInvokeRef userInvokeRef, 
	   ESRO_FailureValue failureValue)
{
    printf("\nInvoker: Failure Indication: InvokeId=0x%lx \n", 
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
/*	bTerminateRequested = TRUE;
	break; */
	G_exit(0);

    default:
	break;
    }

    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);

}
#endif
