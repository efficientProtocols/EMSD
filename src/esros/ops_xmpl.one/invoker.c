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
 * File: invoker.c
 * 
 * Description: Invoker (invokes one operation and ends)
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
#include "oe.h"
#include "target.h"
#include "pf.h"
#include "du.h"
#include "eh.h"
#include "getopt.h"
#include "tm.h"
#include "addr.h"
#include "inetaddr.h"
#include "sch.h"
#include "udp_po.h"

#include "esro.h"

#include "extfuncs.h"

Void G_usage(void);
Void G_init(void);
Void G_exit(Int code);
Void G_sigIntr(Int unused);

#ifdef TM_ENABLED
extern TM_ModDesc G_tmDesc;
#endif

ESRO_SapSel locSapSel = 12;
ESRO_SapDesc locSapDesc;

ESRO_InvokeId invokeId;
ESRO_UserInvokeRef userInvokeRef = NULL;
ESRO_EncodingType encodingType;

ESRO_OperationValue operationValue;
#define SHELL_CMD_OP	2

ESRO_SapSel remEsroSapSel = 13;
Int portNuGlob = 2002;
T_SapSel  remTsapSel  = {2, {0x07, 0xD2}      }; /* UDP Port Number (2002) */
N_SapAddr remNsapAddr = {4, {198, 62, 92, 10} }; /* Remote IP Address */

ESRO_FunctionalUnit funcUnit = ESRO_3Way;

struct ESRO_Event event;

static OS_Boolean	bTerminateRequested = FALSE;

char *__applicationName;

char pConfigFile[256] = "./erop.ini";

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


/*<
 * Function:    main()
 *
 * Description: main function of invoker.
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
    if (_wabout("ESROS Single Invoker\n"
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

    INET_portNuToTsapSel((MdInt) portNuGlob, &remTsapSel);

    badUsage = FALSE;

    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:c:f:g:h:l:r:p:n:u:e:o:V")) != EOF) {
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

    while ((c = getopt(argc, argv, "T:c:f:g:h:l:r:p:n:u:e:o:V")) != EOF) {
        switch (c) {
	case 'T':

#ifdef TM_ENABLED
	    TM_SETUP(optarg);
#endif
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

    while ((c = getopt(argc, argv, "T:c:u")) != EOF) {
        switch (c) {
	case 'T':
#ifdef TM_ENABLED
	    TM_SETUP(optarg);
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

    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);

    G_init();

    OS_init();

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);
#endif

    /* Application Specific Code Goes Here */
    {
	static void invoke(void);	

	printf("\nRemote Network Address: %d.%d.%d.%d\n", remNsapAddr.addr[0],
		remNsapAddr.addr[1], remNsapAddr.addr[2], remNsapAddr.addr[3]);
	printf("\nFunctional Unit: %d-Way handshaking\n", funcUnit);

	invoke();
    }
    exit (SUCCESS);

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


/*<
 * Function:    invoke()
 * 
 * Description: Invoke an operation.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

static void
invoke(void)
{
    ESRO_RetVal gotVal;
    String invokeParameter = "date";

    ESRO_sapUnbind(locSapSel);

    if ((gotVal = ESRO_sapBind(&locSapDesc, locSapSel, funcUnit)) < 0) {
	EH_problem("invoke: Could not activate local ESRO SAP.");
	G_exit(13);
    }

    TM_TRACE((G_tmDesc, TM_ENTER, 
	     "invoker: Issue InvokeReq: remEsroSapSel=%d, parameter=%s\n",
	     remEsroSapSel, invokeParameter));

    if ( (gotVal = ESRO_invokeReq(&invokeId, 
				  userInvokeRef,
				  locSapDesc, remEsroSapSel,
			 	  &remTsapSel, &remNsapAddr, 
			 	  (ESRO_OperationValue) SHELL_CMD_OP, 
			 	  (ESRO_EncodingType) 2,  /* ASCII encoding */
			 	  4, invokeParameter)) < 0 ) {
	EH_problem("invoke: Could not Invoke");
    }
  
    while ( TRUE ) {

	TM_TRACE((G_tmDesc, TM_ENTER, "invoke: Waiting for Event\n"));

	while ((gotVal = ESRO_getEvent(locSapDesc, &event, FALSE)) < 0) { 
	}

	processEvent(&event);

	G_exit(0);
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
    case ESRO_RESULTIND:

	*(p_event->un.resultInd.data + p_event->un.resultInd.len) = '\0';

	TM_TRACE((G_tmDesc, TM_ENTER,
		 "processEvent(Invoker): resultInd invokeId=%d, paramter=%s\n",
		 p_event->un.resultInd.invokeId,
		 p_event->un.resultInd.data));

	printf("-------------------------------------------------------------------\n");
	printf("Invoker: Got Result Indication: invokeId=%d, paramter=%s\n",
		 p_event->un.resultInd.invokeId,
		 p_event->un.resultInd.data);
	printf("-------------------------------------------------------------------\n");
	break;
    case ESRO_ERRORIND:

        TM_TRACE((G_tmDesc, TM_ENTER, 
		 "processEvent: ErrorInd invokeId=%d, parameter=%s\n",
	         p_event->un.errorInd.invokeId,
                 p_event->un.errorInd.data));

	printf("Invoker: Got Error Indication: invokeId=%d, parameter=%s\n", 
	       p_event->un.errorInd.invokeId,
               p_event->un.errorInd.data);
	break;
    case ESRO_FAILUREIND:

        TM_TRACE((G_tmDesc, TM_ENTER, "processEvent: FailureInd  Code=%d\n",
               p_event->un.failureInd.failureValue));

	printf("Invoker: Operation Failed: Failure Code %d\n", 
               p_event->un.failureInd.failureValue);
	break;
    default:

        TM_TRACE((G_tmDesc, TM_ENTER, "processEvent: Invalid primitive\n"));

	EH_problem("processEvent(Invoker): Invalid primitive");
	printf("Invoker: Invalid primitive");
	return (FAIL);
    }
    return (SUCCESS);
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
