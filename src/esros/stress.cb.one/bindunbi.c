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
 * File: bindunbi.c
 * 
 * Description: Stress tester for binding and unbinding SAPs.
 * 
 * Requirements:
 *  Continuously binds and unbinds SAPs
-*/

/*
 * Author: Mohsen Banan
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: bindunbi.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "pf.h"
#include "eh.h"
#include "getopt.h"
#include "tm.h"
#include "addr.h"
#include "inetaddr.h"
#include "extfuncs.h"
#include "tmr.h"
#include "sch.h"

#include "target.h"
#include "esro_cb.h"

PUBLIC DU_Pool *G_duMainPool;

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif

Void G_init(void);
Void G_exit(Int code);
Void G_usage(void);

#ifdef OS_TYPE_UNIX
Void G_sigPipe(Int unused);
Void G_sigTerm(Int unused);
Void G_sigKill(Int unused);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
Void G_sigIntr(Int unused);
#endif

ESRO_SapSel locSapSel = 14;
ESRO_SapDesc locSapDesc;
ESRO_InvokeId invokeId;
ESRO_EncodingType encodingType;
ESRO_OperationValue operationValue;
ESRO_SapSel remEsroSapSel = 15;
T_SapSel  remTsapSel  = {2, {0x07, 0xD2}      }; /* UDP Port Number (2002) */
N_SapAddr remNsapAddr = {4, {198, 62, 92, 10} }; /* Remote IP Address */
struct ESRO_Event event;
Bool reset = FALSE;
int delay = 0;

int
invokeIndication (ESRO_SapDesc locSapDesc, ESRO_SapSel remESROSap, 
           T_SapSel *remTsap, N_SapAddr *remNsap, ESRO_InvokeId invokeId, 
           ESRO_OperationValue opValue, ESRO_EncodingType encodingType, 
           DU_View parameter);
int
resultInd (ESRO_InvokeId invokeId, 
           ESRO_EncodingType encodingType, 
           DU_View parameter);
int
errorInd (ESRO_InvokeId invokeId, 
          ESRO_EncodingType encodingType, 
          ESRO_ErrorValue errorValue, 
          DU_View parameter);
int resultCnf(ESRO_InvokeId invokeId);
int errorCnf(ESRO_InvokeId invokeId);
int failureInd(ESRO_InvokeId invokeId, ESRO_FailureValue failureValue);

Int maxNuOfBindUnbind = 50; 

char *__applicationName;

char pConfigFile[512];


/*<
 * Function:    main()
 *
 * Description: main function 
 *
 * Arguments:   argc, argv.
 *
 * Returns:     None.
 *
>*/

Void 
main(int argc, char **argv)
{
    Int i;
    Int c;
    Bool badUsage; 
    
#ifdef MSDOS
    {
    extern void erop_force_msdos_link(void);
    erop_force_msdos_link();
    }
#endif
    __applicationName = argv[0];

#ifdef TM_ENABLED
    TM_INIT();
#endif

    if (strlen(pConfigFile) == 0) {
	strcpy(pConfigFile, "./eropini");
    }

    badUsage = FALSE;
    while ((c = getopt(argc, argv, "T:m:l:r:s:p:n:u:t:d:o:")) != EOF) {
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

	case 'd':  	/* inter-bind/unbind delay */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 0, 1, 200) ) {
		EH_problem("main: Invalid inter-bind/unbind delay");
		badUsage = TRUE;
	    } else {
		delay = gotVal;
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

	case 'm':  	/* max number of sap bind/unbind */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 50, 2, 20000) ) {
		EH_problem("main: Invalid max-of-bind-unbind");
		badUsage = TRUE;
	    } else {
		maxNuOfBindUnbind = gotVal;
	    }
	}
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

    G_init();

    G_duMainPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS); /* build buf pool */

    ESRO_CB_sapUnbind(locSapSel);

    printf ("\nSap Bind/Unbind (max: %d):\n\n", maxNuOfBindUnbind);
    for (i = 1; i <= maxNuOfBindUnbind; i++) {

	printf ("%5d", i);

#ifndef WINDOWS
	sleep(delay); 
#endif
    	if (ESRO_CB_sapBind(&locSapDesc, locSapSel, 
			    ESRO_3Way,
  		            invokeIndication, 
	                    resultInd, 
		            errorInd, resultCnf, 
			    errorCnf, failureInd) < 0) {
            EH_problem("invoke: Could not activate local ESRO SAP.");
            G_exit(13);
    	}

#ifndef WINDOWS
	sleep(delay); 
#endif
    	ESRO_CB_sapUnbind(locSapSel);
    }

    printf ("\n\nSap Bind/Unbind completed. Number of SapBind/Unbind: %d):\n", i-1);

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
    if ( ! TM_OPEN(G_tmDesc, "G_") ) {
	EH_problem("G_init: TM_open G_ failed\n");
    }
#endif
    
#ifdef OS_TYPE_UNIX
    signal(SIGPIPE, G_sigPipe);
    signal(SIGTERM, G_sigTerm);
    signal(SIGKILL, G_sigKill);
#endif
    
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    signal(SIGINT, G_sigIntr);
#endif


    ESRO_CB_init(pConfigFile);
    TMR_init(NUMBER_OF_TIMERS, CLOCK_PERIOD);
    TM_VALIDATE();
    TMR_startClockInterrupt(CLOCK_PERIOD);
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
    printf("\n\nProgram exited with exit code: %d\n", code);
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
    String usage2 = "[-m max-no-of-sap-bind-unbind] [-d inter-Sap-Bind-Unbind-delay] ";
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

int
invokeIndication (ESRO_SapDesc locSapDesc, ESRO_SapSel remESROSap, 
           T_SapSel *remTsap, N_SapAddr *remNsap, ESRO_InvokeId invokeId, 
           ESRO_OperationValue opValue, ESRO_EncodingType encodingType, 
           DU_View parameter)
{
    printf("\nInvoke Indication N/A\n");
    return 0;
}


int
resultInd (ESRO_InvokeId invokeId, 
           ESRO_EncodingType encodingType, 
           DU_View parameter)
{
    printf("\nResult Indication N/A\n");
    return 0;
}


int
errorInd (ESRO_InvokeId invokeId, 
          ESRO_EncodingType encodingType, 
          ESRO_ErrorValue errorValue, 
          DU_View parameter)
{
    char *duData;

    if (parameter) {
    	duData = DU_data(parameter);

    } else {
	duData = "No parameter";
    }

    printf("\nError Indication N/A\n");
    return 0;
}


int
resultCnf(ESRO_InvokeId invokeId)
{
    printf("\nResult Confirm\n");
    return 0;
}

int
errorCnf(ESRO_InvokeId invokeId)
{
    printf("\nError Confirm\n");
    return 0;
}


int
failureInd(ESRO_InvokeId invokeId, ESRO_FailureValue failureValue)
{
	printf("Failure Indication: Operation Failed: Failure Code %d\n", 
               failureValue);
	return 0;
}

