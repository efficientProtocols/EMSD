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
 * File: main.c
 *
 * Description: main function of ESROS Scenario Interpreter 
 * 		1 process, socket simulation(UPQ_SIMU module). 
 * 
-*/

/*
 * Author: Mohsen Banan. Tweaked by Mark Mc
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: main.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/


#include "estd.h"
#include "getopt.h"
#include "eh.h"
#include "tm.h"
#include "pf.h"
#include "addr.h"
#include "inetaddr.h"
#include "udp_po.h"
#include "sch.h"

#include "target.h"
#include "esro.h"

#include "extfuncs.h"

Void G_usage(void);
Void G_init(void);
Void G_exit(Int code);
Void G_sigIntr(Int unused);

#ifdef UDP_PO_
extern Bool UDP_noLogSw;

extern char eropEngOutLog[256]; 
extern char eropEngErrLog[256]; 
extern PUBLIC SuccFail UDP_PO_init(String errFile, String logFile);
#endif

#ifdef TM_ENABLED
extern TM_ModDesc G_tmDesc;
#endif

extern ESRO_SapSel locSapSel;
ESRO_SapDesc locSapDesc;

ESRO_InvokeId invokeId;
ESRO_EncodingType encodingType;

ESRO_OperationValue operationValue;
#define SHELL_CMD_OP	2

extern ESRO_SapSel remEsroSapSel;
extern T_SapSel remTsapSel;
extern N_SapAddr remNsapAddr;

struct ESRO_Event event;

int processEvent (struct ESRO_Event *p_event);

#ifndef MSDOS
extern int yydebug;
#endif

char *__applicationName;

char pConfigFile[512];

extern Int udpSapSel;
extern Int nuOfSaps;
extern Int nuOfInvokes;
extern Int invokePduSize;

extern struct TimerValue {
    Int	    acknowledgment;
    Int	    roundtrip;
    Int	    retransmit;
    Int	    maxNSDULifeTime;
} timerValue; 
extern int retransmitCount;

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

void
main(int argc, char **argv)
{
    Int c;
    Bool badUsage; 
    
    Char *copyrightNotice;
    EXTERN Char *RELID_getCopyrightNeda(Void);
    EXTERN Char *RELID_getCopyrightAtt(Void);
    EXTERN Char *RELID_getCopyrightNedaAtt(Void);

    __applicationName = argv[0];

#ifdef TM_ENABLED
    TM_init();
#endif

#ifndef MSDOS
    yydebug = 1;
#endif

    badUsage = FALSE;

    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:c:l:d:r:s:p:n:o:e:uV")) != EOF) {
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

    while ((c = getopt(argc, argv, "T:c:l:d:r:s:p:n:o:e:u")) != EOF) {
        switch (c) {
	case 'T':
#ifdef TM_ENABLED
	    TM_setUp(optarg);
#endif
	    break;

	case 'd':		/* directory path where scenario files live */
		{
		extern char *directory;
		directory = strdup(optarg);
		break;
		}

	case 'l':  	/* Local ESRO Sap Selector */
	{
	    Int gotVal;

	    if ( PF_getInt(optarg, &gotVal, 12, 0, 63) ) {
		EH_problem("tester main: bad local sap selector in -l option");
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
		EH_problem("tester main: bad remote sap selector in -r option");
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
		EH_problem("tester main: bad port number -p option");
		badUsage = TRUE;
	    } else {
		INET_portNuToTsapSel((MdInt) portNu, &remTsapSel);
	    }
	}
	    break;

	case 'n':	/* Remote NSAP Address, NSAP Address */
	{
	    struct in_addr inetAddr;

	    fprintf(stderr, "\nRemote address=%s\n", optarg);

	    * ((LgInt *) &inetAddr) = inet_addr(optarg);
	    INET_in_addrToNsapAddr(&inetAddr, &remNsapAddr);
	}
	    break;

#ifdef UDP_PO_
	case 'o':		/* Output log file */
	    UDP_noLogSw = 0;
	    if (strlen(optarg) != 0)
	        strcpy(eropEngOutLog, optarg);
	    break;

	case 'e':		/* Error log file */
	    UDP_noLogSw = 0;
	    if (strlen(optarg) != 0)
	        strcpy(eropEngErrLog, optarg);
	    break;
#endif

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
	    TM_setUp(optarg);
#endif
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

    if ( ! (copyrightNotice = RELID_getCopyrightNedaAtt()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);

    G_init();

    OS_init();

#ifdef UDP_PO_
    TM_TRACE((G_tmDesc, TM_ENTER, 
    	     "**** stress_i.cb.one compiled <<<WITH UDP_PO>>> *****\n"));
    if (!UDP_noLogSw) {
    	UDP_PO_init (eropEngErrLog, eropEngOutLog);
        TM_TRACE((G_tmDesc, TM_ENTER, "===> Logging activated\n"));
    }
#else
    TM_TRACE((G_tmDesc, TM_ENTER, 
    	    "**** stress_i.cb.one main compiled <<<WITHOUT UDP_PO>>> *****\n"));
#endif

    TM_TRACE((G_tmDesc, TM_ENTER, "\nPort number=%d\n", udpSapSel));

    {
    	time_t t;
    	t = time(&t);
        TM_TRACE((G_tmDesc, TM_ENTER,
    		 "\nESROS Scenario Interpreter started on: %s\n", ctime(&t)));
    }

    TM_TRACE((G_tmDesc, TM_ENTER,
    	     "\n--- Timer Values ---\n\tAcknowledgement\t=%d\n\tRoundtrip\t=%d",
             timerValue.acknowledgment, timerValue.roundtrip));
    TM_TRACE((G_tmDesc, TM_ENTER,
    	     "\n\tretransmit\t=%d\n\tmaxNSDULifeTime\t=%d\n",
             timerValue.retransmit, timerValue.maxNSDULifeTime));
    TM_TRACE((G_tmDesc, TM_ENTER,
    	     "\n\tretransTimeout\t=%d\n\trwait\t\t=%d\n\t"
	     "nuOfRetrans\t=%d\n\trefKeepTime\t=%d",
       	     timerValue.retransmit + timerValue.roundtrip, 
    	     2 * (timerValue.acknowledgment + timerValue.retransmit), 
             retransmitCount, 
             (2 * timerValue.maxNSDULifeTime) + 
             retransmitCount * timerValue.retransmit));

    TM_TRACE((G_tmDesc, TM_ENTER,
    	     "\n\tinactivityDelay\t=%d\n\tperfNoResponse\t=%d\n"
	     "--------------------\n",
    	     4 * timerValue.roundtrip,
    	     (2 * timerValue.maxNSDULifeTime) + 
	     retransmitCount * (timerValue.retransmit + timerValue.roundtrip)));

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);
#endif

#ifdef TM_ENABLED
	TM_validate();
#endif

    /* Application Specific Code Goes Here */
    {
	static void invoke(void);	

	TSI_parse();
    }
    exit(0);
}

Void
G_exit(Int code)
{
    fprintf (stderr, "G_exit: exit code=%d\n", code);
    exit(code);
}

Void
G_usage(void)
{
    String usage1 = "[-T G_,ffff]";
    String usage2 = "-l localEsroSapSel -r remoteEsroSapSel -p remotePortNu -n remoteIPAdrr -o outLogFile -e errLogFile -T ESRO_,ffff -T IMQ_,ffff";

    printf("%s: Usage: %s\n", __applicationName, usage1);
    printf("%s: Usage: %s\n", __applicationName, usage2);
}

Void
G_sigIntr(Int unused)
{
    G_exit(22);
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
