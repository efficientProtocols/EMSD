/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of Neda-EMSD. An implementation of RFC-2524.
 * 
 * Neda-EMSD is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Neda-EMSD; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:
 *
 */


#include "estd.h"
#include "tm.h"
#include "eh.h"
#include "buf.h"
#include "config.h"
#include "tmr.h"
#include "strfunc.h"
#include "nvm.h"
#include "sch.h"
#include "asn.h"
#include "du.h"
#include "getopt.h"
#include "udp_if.h"
#include "udp_po.h"
#include "inetaddr.h"
#include "esro_cb.h"
#include "mm.h"
#include "emsdmail.h"
#include "emsd822.h"
#include "ua.h"
#include "uashloc.h"
#include "relid.h"

#define	QUERY_HEADER	"!>>"

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

/*
 * Global Variables
 *
 * All global variables for the UASH go inside of UASH_Globals unless
 * there's a really good reason for putting them elsewhere (like
 * having external requirements on the name).
 */
UASH_Globals 	uash_globals;

/* Globals with external requirements that they be named like this. */
char *		__applicationName = "";
DU_Pool *	G_duMainPool;


/*
 * Local Types and Variables
 */
static struct
{
    char *	    pAgentPortNumber;
    char *	    pUserId;
    char *	    pDeliveryVerifySupported;
    char *	    pNonVolatileMemoryEmulation;
    char *	    pNonVolatileMemorySize;
    char *	    pConcurrentOperationsOutbound;
    char *	    pConcurrentOperationsInbound;
} config = { NULL };

static struct ConfigParams
{
    char *	    pSectionName;
    char *	    pTypeName;
    char **	    ppValue;
} configParams[] =
  {
      {
	  "Features",
	  "Delivery Verify Enabled",
	  &config.pDeliveryVerifySupported
      },
      {
	  "Features",
	  "Non-Volatile Memory Emulation",
	  &config.pNonVolatileMemoryEmulation
      },
      {
	  "Features",
	  "Non-Volatile Memory Size",
	  &config.pNonVolatileMemorySize
      },
      {
	  "Concurrent Operations",
	  "Inbound",
	  &config.pConcurrentOperationsInbound
      },
      {
	  "Concurrent Operations",
	  "Outbound",
	  &config.pConcurrentOperationsOutbound
      },
      {
	  "Local User",
	  "User Id",
	  &config.pUserId
      }
  };

static ESRO_SapDesc 	hAgentInvoker;
static ESRO_SapDesc 	hAgentPerformer;
static OS_Boolean	bInitializing = TRUE;
static OS_Boolean	bTerminateRequested = FALSE;
static OS_Boolean	bNoMemDebug = FALSE;
static OS_Boolean	bTelephoneKeyboard = FALSE;
static OS_Boolean	bNoInterrupts = FALSE;
static NVM_Transaction	hTrans;
static void *		hMmEntity;
static void *		hMmUash;
static void *		hMmPassword;
static void *		hMmMtsNsap;
static int		columns				= 24;
static int		rows				= 6;

/*
 * Forward Declarations
 */
FORWARD static void
displayMessageList(NVM_Transaction hFirst);

ReturnCode
displayMessage(NVM_Transaction hTrans,
	       int highlight);

FORWARD static Void
keyboardInput(Ptr arg);

FORWARD static void
menuInput(int c);

FORWARD static void
messageInput(int c);

FORWARD static ReturnCode
createResponse(int response);

FORWARD static int
getTelephoneKey(int c);

FORWARD static void
freeHeadingFields(UASH_MessageHeading * pHeading);

FORWARD static void
terminate(int sigNum);

static ReturnCode
initAgent(void);

int
agentInvokeIndication(ESRO_SapDesc hSapDesc,
		      ESRO_SapSel sapSel, 
		      T_SapSel * pRemoteTsap,
		      N_SapAddr * pRemoteNsap,
		      ESRO_InvokeId invokeId, 
		      ESRO_OperationValue operationValue,
		      ESRO_EncodingType encodingType, 
		      DU_View hView);

int
agentResultIndication(ESRO_InvokeId invokeId, 
		      ESRO_UserInvokeRef userInvokeRef,
		      ESRO_EncodingType encodingType, 
		      DU_View hView);

int
agentErrorIndication(ESRO_InvokeId invokeId, 
		     ESRO_UserInvokeRef userInvokeRef,
		     ESRO_EncodingType encodingType,
		     ESRO_ErrorValue errorValue,
		     DU_View hView);

int
agentResultConfirm(ESRO_InvokeId invokeId,
		   ESRO_UserInvokeRef userInvokeRef);

int
agentErrorConfirm(ESRO_InvokeId invokeId,
		  ESRO_UserInvokeRef userInvokeRef);

int
agentFailureIndication(ESRO_InvokeId invokeId,
		       ESRO_UserInvokeRef userInvokeRef,
		       ESRO_FailureValue failureValue);

static ReturnCode
agentResponse(void ** phOperation,
	      MM_RemoteOperationType operationType,
	      MM_RemoteOperationValue  operationValue,
	      void ** phBuf);

static void
agentValueChanged(void * hUserData,
		  void * hManagableObject,
		  MM_Value * pValue);


#if defined(OS_TYPE_UNIX) && defined(OS_VARIANT_SYSV)
/*
 * Something on SYSV defines strchr() in terms of index(), but doesn't
 * provide an extern declaration of index().
 */
extern char *
index(char * s, int c);
#endif


/*
 * Main Line
 */
int
main(int argc, char * argv[])
{
    int				c;
    static char 		pConfigFile[256] = "";
    char *			p;
    char *			pPassword;
    char *			pPduLog = "ua_pdu.log";
    char *			pErrLog = "ua_err.log";
    void *			hConfig;
    OS_Uint32			nvFileSize;
    ESRO_RetVal			retval;
    ReturnCode			rc;
    MM_Value			mmValue;
    N_SapAddr *			pMtsNsap;
    static N_SapAddr		dummyNsap = { 0, { 0, 0, 0, 0 } };
    struct ConfigParams *	pConfigParam;

    Char *copyrightNotice;
    
    setbuf(stdout, NULL);
    __applicationName = argv[0];

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_QuickWin)
    /* If we ever call exit(), prompt for whether to destroy windows */
    i = _wsetexit(_WINEXITPROMPT);
    if (i != 0)
    {
    	EH_problem("wsetexit() failed");
    }
    else
    {
    	EH_message("Success", "wsetexit()");
    }
#endif

    /* Initialize the trace module */
    TM_INIT();

#ifdef MSDOS
    {
	extern void erop_force_msdos_link();
	erop_force_msdos_link();
    }
#endif
    
    /* Initialize the OS module */
    if (OS_init() != Success)
    {
	sprintf(uash_globals.tmpbuf, "Could not initialize OS module");
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Initialize the OS memory allocation debug routines */
    OS_allocDebugInit(NULL);

    /* Initialize the OS file tracking debug routines */
    OS_fileDebugInit("/tmp/uashdvem.files");

    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:xc:itw:h:lp:e:V")) != EOF)
    {
        switch (c)
	{
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

    if (strlen(pConfigFile) == 0)
    {
	strcpy(pConfigFile, "ua.ini");
    }

    /* Initialize the config module */
    if ((rc = CONFIG_init()) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not initialize CONFIG module\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Open the configuration file.  Get a handle to the configuration data */
    if ((rc = CONFIG_open(pConfigFile, &hConfig)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not open configuration file (%.*s), "
		"reason 0x%04x\n",
		__applicationName,
		(int) (sizeof(uash_globals.tmpbuf) / 2), pConfigFile, rc);
	EH_fatal(uash_globals.tmpbuf);
    }

    optind = 0;
    while ((c = getopt(argc, argv, "T:xc:itw:h:lp:e:")) != EOF)
    {
        switch (c)
	{
	case 'x':		/* Disable memory debugging */
	    bNoMemDebug = TRUE;
	    break;

	case 'i':		/* Don't allow timer interrupts */
	    bNoInterrupts = TRUE;
	    break;

	case 't':		/* Switch keys to telephone style */
	    bTelephoneKeyboard = TRUE;
	    break;

	case 'w':		/* Width */
	    columns = atoi(optarg);
	    if (columns > 80)
	    {
	        EH_fatal("Maximum number of columns is 80.");
	    }
	    break;

	case 'h':		/* Height */
	    rows = atoi(optarg);
	    break;

#ifdef TM_ENABLED
	case 'T':		/* enable trace module tracing */
	    TM_SETUP(optarg);
	    break;
#endif

	case 'c':		/* Config file (already handled) */
	    break;

	case 'l':		/* Enable UDP_PCO logging */
	    UDP_noLogSw = FALSE;
	    break;

	case 'p':		/* UDP_PCO PDU log file name */
	    pPduLog = optarg;
	    break;

	case 'e':		/* UDP_PCO Error log file */
	    pErrLog = optarg;
	    break;

	default:
	    sprintf(uash_globals.tmpbuf,
		    "usage: %s -c <ua.ini> -a <esros.ini> [<options>]\n",
		    __applicationName);
	    EH_fatal(uash_globals.tmpbuf);
	    break;
	}
    }

    /* Give ourselves a trace handle */
    if (TM_OPEN(uash_globals.hTM, "UASH") == NULL)
    {
	EH_fatal("Could not open UASH trace");
    }

    /*
     * Read the configuration
     */

    /* Get each of the configuration parameter values */
    for (pConfigParam = &configParams[0];
	 pConfigParam < &configParams[sizeof(configParams) /
				     sizeof(configParams[0])];
	 pConfigParam++)
    {
	if ((rc = CONFIG_getString(hConfig,
				   pConfigParam->pSectionName,
				   pConfigParam->pTypeName,
				   pConfigParam->ppValue)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "%s: Configuration parameter\n\t%s/%s\n"
		    "\tnot found, reason 0x%04x",
		    __applicationName,
		    pConfigParam->pSectionName,
		    pConfigParam->pTypeName,
		    rc);
	    EH_fatal(uash_globals.tmpbuf);
	}

	TM_TRACE((uash_globals.hTM, UASH_TRACE_INIT,
		  "\n    Configuration parameter\n"
		  "\t[%s]:%s =\n\t%s\n",
		  pConfigParam->pSectionName,
		  pConfigParam->pTypeName,
		  *pConfigParam->ppValue));
    }

    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);

    /* Initialize the TMR module */
    TMR_init(NUMBER_OF_TIMERS, CLOCK_PERIOD);
      
    /* Initialize the SCH module */
    SCH_init(MAX_SCH_TASKS);
      
    /* Initialize the non-volatile memory module */
    if ((rc = NVM_init()) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize NVM module, reason 0x%x\n",
		__applicationName, rc);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Is there an existing Non-volatile memory file? */
    if (OS_fileSize(config.pNonVolatileMemoryEmulation,
		    &nvFileSize) == Success)
    {
	if ((rc = NVM_open(config.pNonVolatileMemoryEmulation, 0)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "%s: "
		    "Could not open existing non-volatile memory file, "
		    "reason 0x%x",
		    __applicationName, rc);
	    EH_fatal(uash_globals.tmpbuf);
	}
    }
    else
    {
	nvFileSize = atol(config.pNonVolatileMemorySize);

#if defined(OS_TYPE_MSDOS)
	if (nvFileSize > 32764)
	{
	    sprintf(uash_globals.tmpbuf,
		    "%s: "
		    "Maximum non-volatile memory size on DOS is 32764.",
		    __applicationName);
	    EH_fatal(uash_globals.tmpbuf);
	}
#endif

	if ((rc = NVM_open(config.pNonVolatileMemoryEmulation,
			   nvFileSize)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "%s: "
		    "Could not create non-volatile memory file, "
		    "reason 0x%x",
		    __applicationName, rc);
	    EH_fatal(uash_globals.tmpbuf);
	}
    }

    /* Initialize the ASN Module */
    if ((rc = ASN_init()) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize ASN module\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Build the buffer pool that the ESRO_CB_ functions require */
    G_duMainPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS);

    /* Initialize the UDP_PO module */
    UDP_PO_init(pErrLog, pPduLog);

    /* Initialize the ESROS module */
    if ((retval = ESRO_CB_init(pConfigFile)) != 0)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize ESRO module\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Initialize MM agent */
    if ((rc = initAgent()) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize MM agent\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Register this application entity instance */
    if ((rc = MM_entityInit("EMSD User Agent (device-emulator)",
			    NULL, &hMmEntity)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize UA entity with MM\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Register the UASH module with module management */
    if ((rc = MM_registerModule(hMmEntity,
				"UASH",
				&hMmUash)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not register UASH module with MM\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Create a managable object for the MTS password */
    if ((rc =
	 MM_registerManagableObject(hMmUash,
				    MM_ManagableObjectType_String,
				    "password",
				    "MTS Password",
				    MM_AccessByName_ReadWrite,
				    0,
				    agentValueChanged,
				    NULL,
				    &hMmPassword)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not register password object with module management.",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Create a managable object for the MTS NSAP */
    if ((rc =
	 MM_registerManagableObject(hMmUash,
				    MM_ManagableObjectType_String,
				    "mtsNsap",
				    "MTS NSAP Address",
				    MM_AccessByName_ReadWrite,
				    0,
				    agentValueChanged,
				    NULL,
				    &hMmMtsNsap)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not register password object with module management.",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Get the MTS password */
    if ((rc = NVM_nextTrans(TransType_Password,
			    NVM_FIRST, &hTrans)) != Success)
    {
	/* No password has yet been specified. Treat it as an empty password */
	pPassword = "";
    }
    else
    {
	/* Get a pointer to the password */
	pPassword = NVM_getMemPointer(hTrans, NVM_FIRST);
    }

    /* Specify the password value */
    mmValue.type = MM_ValueType_String;
    mmValue.un.string = pPassword;

    /* Set the Module Management object for the password */
    if ((rc = MM_setValueByHandle(hMmPassword, &mmValue)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not set Password object for module management\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Get the MTS NSAP */
    if ((rc = NVM_nextTrans(TransType_MtsNSAP,
			    NVM_FIRST, &hTrans)) != Success)
    {
	/* No MTS NSPA has yet been specified. Treat it as empty */
	pMtsNsap = &dummyNsap;
	p = "";
    }
    else
    {
	/* Get a pointer to the MTS NSAP */
	pMtsNsap = NVM_getMemPointer(hTrans, NVM_FIRST);
	sprintf(uash_globals.tmpbuf,
		"%d.%d.%d.%d",
		pMtsNsap->addr[0],
		pMtsNsap->addr[1],
		pMtsNsap->addr[2],
		pMtsNsap->addr[3]);
	p = uash_globals.tmpbuf;
    }

    /* Specify the password value */
    mmValue.type = MM_ValueType_String;
    mmValue.un.string = p;

    /* Set the Module Management object for the MTS NSAP */
    if ((rc = MM_setValueByHandle(hMmMtsNsap, &mmValue)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not set MTS NSAP object for module management\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Initialize the UA Core */
    if ((rc = UA_init(pMtsNsap,
		      *pPassword == '\0' ? NULL : pPassword, 
		      (OS_Boolean)
		      (OS_strcasecmp(config.pDeliveryVerifySupported,
				     "TRUE") == 0),
		      (OS_Uint8) TransType_Verify,
		      (OS_Uint8) TransType_DupDetection,
		      (OS_Uint32) atoi(config.pConcurrentOperationsInbound),
		      (OS_Uint32) atoi(config.pConcurrentOperationsOutbound),
		      uash_messageReceived,
		      uash_modifyDeliverStatus,
		      uash_submitSuccess,
		      uash_submitFail,
		      uash_deliveryReport,
		      uash_nonDeliveryReport
#ifdef DELIVERY_CONTROL
		    				 ,
		      uash_deliveryControlSuccess,
		      uash_deliveryControlError,
		      uash_deliveryControlFail
#endif
						)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize UA Core\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Start clock interrupts */
    if (! bNoInterrupts)
    {
	if (TMR_startClockInterrupt(CLOCK_PERIOD) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "%s: Could not start clock interrupt\n",
		    __applicationName);
	    EH_fatal(uash_globals.tmpbuf);
	}
    }
      
#ifdef OS_TYPE_UNIX
    /* If we get a SIGTERM or SIGINT, request the main loop to exit. */
    signal(SIGTERM, terminate);
#endif
    signal(SIGINT, terminate);

    if (! bNoMemDebug)
    {
	printf("\n\nAFTER INITIALIZATION:");

	OS_allocPrintOutstanding();

	printf("\n\n\nkill %-11lu\t(or CTRL-C) to exit\n",
	       (unsigned long) getpid());
	printf("kill -USR1 %-5lu\tto reprint allocation list\n",
	       (unsigned long) getpid());
	printf("kill -USR2 %-5lu\tto reset allocation list\n\n",
	       (unsigned long) getpid());

	printf("RESETTING ALLOCATION LIST NOW.\n\n\n");
	OS_allocResetPrint();
    }

    initscr();
    raw();
    noecho();
    setbuf(stdout, NULL);
    clearok(stdscr, FALSE);
    wclear(curscr);
    wrefresh(curscr);
    wprintw(stdscr, "Initializing...");
    wrefresh(stdscr);

    if ((uash_globals.pFrame = newwin(rows + 2, columns + 2, 0, 0)) == NULL)
    {
	endwin();
	printf("Could not create message frame\n");
	return 1;
    }
    clearok(uash_globals.pFrame, FALSE);

#ifdef OS_TYPE_MSDOS
    box(uash_globals.pFrame, DB_V_LIN, DB_H_LIN);
#else
    box(uash_globals.pFrame, '*', '*');
#endif
    wrefresh(uash_globals.pFrame);

    if ((uash_globals.pWin = subwin(uash_globals.pFrame,
				    rows, columns, 1, 1)) == NULL)
    {
	endwin();
	printf("Could not create message window\n");
	return 1;
    }

    if ((uash_globals.pDocFrame = newwin(10, 62,
					 rows + 3, 10)) == NULL)
    {
	endwin();
	printf("Could not create documentation frame\n");
	return 1;
    }
    clearok(uash_globals.pDocFrame, FALSE);

#ifdef OS_TYPE_MSDOS
    box(uash_globals.pDocFrame, SG_V_LIN, SG_H_LIN);
#else
    box(uash_globals.pDocFrame, '.', '.');
#endif
    wrefresh(uash_globals.pDocFrame);

    if ((uash_globals.pDocWin = subwin(uash_globals.pDocFrame,
				       8, 60, rows + 4, 11)) == NULL)
    {
	endwin();
	printf("Could not create docuemtation window\n");
	return 1;
    }

    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);

    wprintw(uash_globals.pWin, "PID = %d\n", getpid());
    wrefresh(uash_globals.pWin);
    wrefresh(uash_globals.pDocWin);

    /* Initially, we don't have a current message */
    uash_globals.hCurrentMessage = NVM_FIRST;

    /* Display the message list */
    displayMessageList(uash_globals.hCurrentMessage);

    /* We're all done initializing. */
    bInitializing = FALSE;

    /* Main Loop */
    while (SCH_block() >= 0 && ! bTerminateRequested)
    {
	SCH_run();
    }
      
    /* Terminate use of curses */
    endwin();

    /* Shutdown all sockets */
    (void) UDP_terminate();

    if (! bTerminateRequested)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Unexpected Scheduler error.\n",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);

	exit(1);
    }


    if (! bNoMemDebug)
    {
	printf("EXITING:");
	OS_allocPrintOutstanding();
    }
    
    exit(0);
      
}
ReturnCode
uash_messageToUser(NVM_Transaction hMessageTrans)
{
    /* Beep to notify the user that there's a new message */
    putchar(0x7);

    /* Display the message list */
    displayMessageList(uash_globals.hCurrentMessage);

    return Success;
}



/*
 * Local Function Declarations
 */

static void
displayMessageList(NVM_Transaction hCenter)
{
    ReturnCode		rc;
    NVM_Transaction	hTrans;
    Message * 		pMessage;
    int			i;
    int			highlight;
    char *		p;
    char		buf[128];
    char		outbuf[82];

    /* Select the appropriate input function */
    uash_globals.pfInput = menuInput;

    /* Clear the documentation window */
    wclear(uash_globals.pDocWin);
    wmove(uash_globals.pDocWin, 0, 0);

    /* Add the docuemtation for the message list */
    wstandout(uash_globals.pDocWin);
    wprintw(uash_globals.pDocWin, "RTFM:\n");
    wstandend(uash_globals.pDocWin);
    wprintw(uash_globals.pDocWin, "\t#\tDisplay active message\n");
    wprintw(uash_globals.pDocWin, "\t*\tDelete active message\n");
    wprintw(uash_globals.pDocWin, "\t%c\tMake previous message 'active'\n",
	    bTelephoneKeyboard ? '2' : '8');
    wprintw(uash_globals.pDocWin, "\t%c\tMake next message 'active'\n",
	    bTelephoneKeyboard ? '8' : '2');
    wprintw(uash_globals.pDocWin, "\t^C\tExit\n");
    wprintw(uash_globals.pDocWin, "\t^L\tRedraw the screen\n");

    /* Clear the message display */
    wclear(uash_globals.pWin);
    wmove(uash_globals.pWin, 0, 0);

    /* If we were just told to start with the first one... */
    if (hCenter == NVM_FIRST)
    {
	/* ... then get the first one. */
	if ((rc = NVM_nextTrans(TransType_InBox, hCenter, &hTrans)) != Success)
	{
	    /* No messages. */
	    wrefresh(uash_globals.pWin);
	    wrefresh(uash_globals.pDocWin);

	    /* Indicate that there are no messages */
	    uash_globals.hCurrentMessage = NVM_FIRST;

	    return;
	}

	highlight = 0;
    }
    else
    {
	/* We were given a specific starting point.  Is there a previous? */
	if ((rc = NVM_prevTrans(TransType_InBox, hCenter, &hTrans)) != Success)
	{
	    /* Nope.  The first one will be the highlighted one. */
	    hTrans = hCenter;
	    highlight = 0;
	}
	else
	{
	    /* We found a previous item, so we'll highlight the centered one */
	    highlight = 1;
	}
    }

    /* Display the next 3 InBox message From: and Subject: lines */
    for (i = 0, rc = Success;
	 i < 3 && rc == Success;
	 i++, rc = NVM_nextTrans(TransType_InBox, hTrans, &hTrans))
    {
	/* Get the handle of the message */
	pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);

	/* Create the address string */
	if (pMessage->originator.style == RecipientStyle_Rfc822)
	{
	    p = NVM_getMemPointer(hTrans,
				  pMessage->originator.un.hRecipRfc822);
	    sprintf(buf, "%.*s", columns, p);
	}
	else
	{
	    p = NVM_getMemPointer(hTrans,
				  pMessage->originator.un.recipEmsd.hName);
	    sprintf(buf, "%03ld.%03ld %.*s",
		    pMessage->originator.un.recipEmsd.emsdAddr / 1000,
		    pMessage->originator.un.recipEmsd.emsdAddr % 1000,
		    columns - 8,
		    (pMessage->originator.un.recipEmsd.hName == NVM_NULL ?
		     "" : p));
	}

	/* Move to the appropriate place on the screen */
	wmove(uash_globals.pWin, 2 * i, 0);

	/* If this is the selected message, remember it and highlight it. */
	if (i == highlight)
	{
	    /* Highlight it. */
	    wstandout(uash_globals.pWin);

	    /* Save this message handle as the current message */
	    uash_globals.hCurrentMessage = hTrans;
	}

	/* Display the originator */
	wprintw(uash_globals.pWin, "%s", buf);

	/* Display the subject */
	p = NVM_getMemPointer(hTrans, pMessage->hSubject);
	if (strncmp(p, QUERY_HEADER, strlen(QUERY_HEADER)) == 0)
	{
	    p += strlen(QUERY_HEADER);
	}

	strncpy(outbuf, p, columns - 1);
	outbuf[columns - 1] = '\0';
	wprintw(uash_globals.pWin, " %s", p);

	/* Turn off highlighting. */
	wstandend(uash_globals.pWin);
    }

    /* Refresh the screen */
    wrefresh(uash_globals.pWin);
    wrefresh(uash_globals.pDocWin);
}



ReturnCode
displayMessage(NVM_Transaction hTrans,
	       int highlight)
{
    Message * 		pMessage;
    int			i;
    char *		p;
    char *		pEnd;
    char		savedCh;
    OS_Boolean		bIsQuery;
    char		outbuf[82];

    /* If there is no current message (i.e. no messages at all), see ya. */
    if (uash_globals.hCurrentMessage == NVM_FIRST)
    {
	return Fail;
    }

    /* Clear the documentation window */
    wclear(uash_globals.pDocWin);
    wmove(uash_globals.pDocWin, 0, 0);

    /* Add the docuemtation for the message list */
    wstandout(uash_globals.pDocWin);
    wprintw(uash_globals.pDocWin, "RTFM:\n");
    wstandend(uash_globals.pDocWin);
    wprintw(uash_globals.pDocWin, "\t#\tReply with active response\n");
    wprintw(uash_globals.pDocWin, "\t*\tReturn to message list\n");
    wprintw(uash_globals.pDocWin, "\t%c\tMake previous response 'active'\n",
	    bTelephoneKeyboard ? '2' : '8');
    wprintw(uash_globals.pDocWin, "\t%c\tMake next response 'active'\n",
	    bTelephoneKeyboard ? '8' : '2');
    wprintw(uash_globals.pDocWin, "\t^C\tExit\n");
    wprintw(uash_globals.pDocWin, "\t^L\tRedraw the screen\n");

    /* Clear the message display */
    wclear(uash_globals.pWin);
    wmove(uash_globals.pWin, 0, 0);

    /* Get the handle of the message */
    pMessage = NVM_getMemPointer(hTrans, NVM_FIRST);
    
    /* Get a pointer to the subject */
    p = NVM_getMemPointer(hTrans, pMessage->hSubject);

    /* Assume it's not a query */
    bIsQuery = FALSE;

    /* Is this a query? */
    if (strncmp(p, QUERY_HEADER, strlen(QUERY_HEADER)) == 0)
    {
	/* Yup.  Re-display the query without the query header. */
	p += strlen(QUERY_HEADER);
	bIsQuery = TRUE;
    }

    /* Display the subject */
    strncpy(outbuf, p, columns);
    wprintw(uash_globals.pWin, "%s\n", p);

    /*
     * Display the first 5 lines of content
     */

    /* Point to the content */
    for (i = 0,
	     savedCh = '\n',
	     p = NVM_getMemPointer(hTrans, pMessage->hBody);
	 i < 5 && savedCh != '\0' && *p != '\0';
	 i++, p = pEnd + 1)
    {
	/* Find the end of the line */
	pEnd = strchr(p, '\n');
	if (pEnd == NULL)
	{
	    pEnd = p + strlen(p);
	}

	/* Ignore trailing blank line */
	if (p == pEnd || *p == '\0')
	{
	    if (i > 0)
	    {
		--i;
	    }
	    break;
	}

	/* Save the line terminator (either '\n' or '\0' */
	savedCh = *pEnd;
	*pEnd = '\0';

	/* Move to the beginning of the the appropriate line */
	wmove(uash_globals.pWin, i + 1, 0);

	/* Output this line */
	if (bIsQuery)
	{
	    if (i == highlight)
	    {
		wstandout(uash_globals.pWin);
		uash_globals.queryResponse = i;
	    }
	    strncpy(outbuf, p, columns - 3);
	    outbuf[columns - 3] = '\0';
	    wprintw(uash_globals.pWin, "%1d: %s", i + 1, p);
	    wstandend(uash_globals.pWin);
	}
	else
	{
	    strncpy(outbuf, p, columns);
	    outbuf[columns] = '\0';
	    wprintw(uash_globals.pWin, "%s", p);
	}

	/* Restore the line terminator */
	*pEnd = savedCh;
    }
    
    /* Save the last query response */
    if (bIsQuery)
    {
	uash_globals.numQueryResponses = i;
    }
    else
    {
	uash_globals.numQueryResponses = 0;
	uash_globals.queryResponse = -1;
    }

    /* Refresh the screen */
    wrefresh(uash_globals.pWin);
    wrefresh(uash_globals.pDocWin);

    return Success;
}


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
#endif

    c = wgetch(uash_globals.pWin);

    switch(c)
    {
    case 0x0c:			/* CTRL-L: Refresh screen */
	wmove(stdscr, LINES-1, 0);
	wclrtobot(stdscr);
	wrefresh(stdscr);
	wrefresh(curscr);
	break;

    case 0x03:			/* CTRL-C: Quit */
	bTerminateRequested = TRUE;
	break;

    default:
	(* uash_globals.pfInput)(c);
	break;
    }

    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);

}


static void
menuInput(int c)
{
    ReturnCode		    rc;
    NVM_Transaction	    hTrans = NVM_FIRST;

    if (bTelephoneKeyboard)
    {
	c = getTelephoneKey(c);
    }

    switch(c)
    {
    case '#':			/* Display selected message */
	if (uash_globals.hCurrentMessage == NVM_FIRST)
	{
	    putchar(0x7);
	    goto displayIt;
	}

	if (displayMessage(uash_globals.hCurrentMessage, 0) == Success)
	{
	    uash_globals.pfInput = messageInput;
	}
	else
	{
	    putchar(0x7);
	}
	break;

    case '*':			/* Delete selected message */
	if (uash_globals.hCurrentMessage == NVM_FIRST)
	{
	    putchar(0x7);
	    goto displayIt;
	}

	if ((rc = NVM_nextTrans(TransType_InBox,
				uash_globals.hCurrentMessage,
				&hTrans)) != Success)
	{
	    if ((rc = NVM_prevTrans(TransType_InBox,
				    uash_globals.hCurrentMessage,
				    &hTrans)) != Success)
	    {
		hTrans = NVM_FIRST;
	    }
	}

	if ((rc = NVM_freeTrans(uash_globals.hCurrentMessage)) != Success)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_ERROR,
		      "Could not free transaction 0x%lx",
		      (unsigned long) hTrans));
	    putchar(0x7);
	    hTrans = uash_globals.hCurrentMessage;
	}

      displayIt:
	displayMessageList(hTrans);
	break;

    case '8':			/* Up */
	if (uash_globals.hCurrentMessage == NVM_FIRST)
	{
	    putchar(0x7);
	    goto displayIt;
	}

	if ((rc = NVM_prevTrans(TransType_InBox,
				uash_globals.hCurrentMessage,
				&hTrans)) != Success)
	{
	    putchar(0x7);
	    hTrans = uash_globals.hCurrentMessage;
	}

	displayMessageList(hTrans);
	break;

    case '2':		/* Down */
	if (uash_globals.hCurrentMessage == NVM_FIRST)
	{
	    putchar(0x7);
	    goto displayIt;
	}

	if ((rc = NVM_nextTrans(TransType_InBox,
				uash_globals.hCurrentMessage,
				&hTrans)) != Success)
	{
	    putchar(0x7);
	    hTrans = uash_globals.hCurrentMessage;
	}

	displayMessageList(hTrans);
	break;

    default:
	putchar(0x7);
	break;
    }
}


static void
messageInput(int c)
{
    if (bTelephoneKeyboard)
    {
	c = getTelephoneKey(c);
    }

    switch(c)
    {
    case '*':			/* Go back to message list */
	displayMessageList(uash_globals.hCurrentMessage);
	break;

    case '8':			/* Up */
	if (uash_globals.queryResponse > 0)
	{
	    displayMessage(uash_globals.hCurrentMessage,
			   uash_globals.queryResponse - 1);
	}
	else
	{
	    putchar(0x7);
	}
	break;
	
    case '2':			/* Down */
	if (uash_globals.queryResponse < uash_globals.numQueryResponses - 1)
	{
	    displayMessage(uash_globals.hCurrentMessage,
			   uash_globals.queryResponse + 1);
	}
	else
	{
	    putchar(0x7);
	}
	break;

    case '#':			/* Create Response */
	(void) createResponse(uash_globals.queryResponse);
	displayMessageList(uash_globals.hCurrentMessage);
	break;

    default:
	putchar(0x7);
	break;
    }
}


static ReturnCode
createResponse(int response)
{
    ReturnCode			rc = Success;
    UASH_MessageHeading 	heading;
    UASH_Recipient * 		pRecip;
    UASH_RecipientInfo * 	pRecipInfo;
    Message *			pMessage;
    Recipient * 		pLocRecip;
    int				i;
    int				savedCh;
    char * 			p;
    char *			pEnd;
    char * 			pBodyText = NULL;
    void *			hMessage;
    unsigned long 		a;
    unsigned long 		b;

    /* Point to the message to which we're responding */
    pMessage = NVM_getMemPointer(uash_globals.hCurrentMessage, NVM_FIRST);

    /* Initialize the structure to pass the heading to the UA */
    heading.originator.style = UASH_RecipientStyle_Rfc822;
    heading.originator.un.pRecipRfc822 = NULL;
    heading.sender.style = UASH_RecipientStyle_Rfc822;
    heading.sender.un.pRecipRfc822 = NULL;
    QU_INIT(&heading.recipInfoQHead);
    QU_INIT(&heading.replyToQHead);
    heading.pRepliedToMessageId = NULL;
    heading.pSubject = NULL;
    heading.priority = UASH_Priority_Normal;
    heading.importance = UASH_Importance_Normal;
    heading.bAutoForwarded = FALSE;
    heading.pMimeVersion = NULL;
    heading.pMimeContentType = NULL;
    heading.pMimeContentId = NULL;
    heading.pMimeContentDescription = NULL;
    heading.pMimeContentTransferEncoding = NULL;
    QU_INIT(&heading.extensionQHead);

    /* Add the originator */
    heading.originator.style = UASH_RecipientStyle_Emsd;
    a = 0;
    b = 0;
    sscanf(config.pUserId, "%lu.%lu", &a, &b);
    heading.originator.un.recipEmsd.emsdAddr = (a * 1000) + b;
    heading.originator.un.recipEmsd.pName = NULL;

    /* Add the subject */
    p = NVM_getMemPointer(uash_globals.hCurrentMessage, pMessage->hSubject);
    if ((heading.pSubject = OS_allocStringCopy(p)) == NULL)
    {
	rc = ResourceError;
	goto cleanUp;
    }

    /* Are we using the sender or the originator to determine the recipient? */
    if (pMessage->sender.style == RecipientStyle_Rfc822 &&
	pMessage->sender.un.hRecipRfc822 == NVM_NULL)
    {
	/* There's no sender specified.  Use the originator */
	pLocRecip = &pMessage->originator;
    }
    else
    {
	/* There's a sender.  Use him. */
	pLocRecip = &pMessage->sender;
    }

    /* Is this an RFC-822 address that we'll need to allocate memory for? */
    if (pLocRecip->style == RecipientStyle_Rfc822 &&
	pLocRecip->un.hRecipRfc822 != NVM_NULL)
    {
	p = NVM_getMemPointer(uash_globals.hCurrentMessage,
			      pLocRecip->un.hRecipRfc822);
	i = strlen(p) + 1;
    }
    else
    {
	i = 0;
    }

    /* Allocate Recipient structure */
    if ((pRecipInfo = OS_alloc(sizeof(UASH_RecipientInfo) + i)) == NULL)
    {
	rc = ResourceError;
	goto cleanUp;
    }

    /* Point to the recipient field */
    pRecip = &pRecipInfo->recipient;

    /* What address style is he? */
    if (pLocRecip->style == RecipientStyle_Emsd)
    {
	pRecip->style = UASH_RecipientStyle_Emsd;
	pRecip->un.recipEmsd.emsdAddr = pLocRecip->un.recipEmsd.emsdAddr;

	/* Ignore the emsdName for now */
	pRecip->un.recipEmsd.pName = NULL;
    }
    else
    {
	pRecip->style = UASH_RecipientStyle_Rfc822;
	p = NVM_getMemPointer(uash_globals.hCurrentMessage,
			      pLocRecip->un.hRecipRfc822);
	pRecip->un.pRecipRfc822 = (char *) (pRecipInfo + 1);
	strcpy(pRecip->un.pRecipRfc822, p);
    }

    /* Specify the per-recipient flags */
    pRecipInfo->recipientType = UASH_RecipientType_Primary;
    pRecipInfo->bReceiptNotificationRequested = FALSE;
    pRecipInfo->bNonReceiptNotificationRequested = FALSE;
    pRecipInfo->bMessageReturnRequested = FALSE;
    pRecipInfo->bNonDeliveryReportRequested = FALSE;
    pRecipInfo->bDeliveryReportRequested = FALSE;
    pRecipInfo->bReplyRequested = FALSE;

    /* Add the recipient to the recipient queue */
    QU_INIT(pRecipInfo);
    QU_INSERT(pRecipInfo, &heading.recipInfoQHead);

    /* Add the response text */
    pBodyText =
	NVM_getMemPointer(uash_globals.hCurrentMessage, pMessage->hBody);

    /* Find the selected response */
    for (i = 0,
	     savedCh = '\n',
	     p = NVM_getMemPointer(uash_globals.hCurrentMessage,
				   pMessage->hBody),
	     pEnd = strchr(p, '\n');
	 i < response && savedCh != '\0';
	 i++,
	     p = pEnd + 1,
	     pEnd = strchr(p, '\n'))
    {
	/* Find the end of the line */
	if (pEnd == NULL)
	{
	    pEnd = p + strlen(p);
	}
	savedCh = *pEnd;
    }

    if (savedCh == '\0')
    {
	rc = Fail;
	goto cleanUp;
    }

    *pEnd = '\0';

    /* Allocate body text for the response message */
    if ((pBodyText =
	 OS_alloc(sizeof("The response to your query:\n\t") +
		  strlen(NVM_getMemPointer(uash_globals.hCurrentMessage,
					   pMessage->hSubject)) +
		  sizeof("\nis:\n\t") +
		  (pEnd - p) +
		  2)) == NULL)
    {
	rc = ResourceError;
	goto cleanUp;
    }

    sprintf(pBodyText,
	    "The response to your query:\n\t%s\nis:\n\t%s\n",
	    (char *) NVM_getMemPointer(uash_globals.hCurrentMessage,
				       pMessage->hSubject),
	    p);

    /* Restore the line terminator */
    *pEnd = savedCh;

    /* Submit this message to the UA */
    rc = UA_submitMessage(&heading, pBodyText, &hMessage, (void *)"");

  cleanUp:
    /* We're all done with the heading */
    freeHeadingFields(&heading);
    if (pBodyText != NULL)
    {
	OS_free(pBodyText);
    }

    return rc;
}


static int
getTelephoneKey(int c)
{
    /* If we're emulating a telephone keyboard, swap upper & lower rows */
    switch(c)
    {
    case '1':
	c = '7';
	break;

    case '2':
	c = '8';
	break;

    case '3':
	c = '9';
	break;

    case '7':
	c = '1';
	break;

    case '8':
	c = '2';
	break;

    case '9':
	c = '3';
	break;

    default:
	break;
    }

    return c;
}


static void
freeHeadingFields(UASH_MessageHeading * pHeading)
{
#undef	FREE
#define	FREE(p)					\
    if ((p) != NULL)				\
    {						\
	OS_free(p);				\
	p = NULL;				\
    }

    /* If the originator has an RFC-822 address, free it. */
    if (pHeading->originator.style == UASH_RecipientStyle_Rfc822)
    {
	FREE(pHeading->originator.un.pRecipRfc822);
    }

    /* If the sender has an RFC-822 address, free it. */
    if (pHeading->sender.style == UASH_RecipientStyle_Rfc822)
    {
	FREE(pHeading->sender.un.pRecipRfc822);
    }

    /* Free the elements in the recipient info queue */
    QU_FREE(&pHeading->recipInfoQHead);

    /* Free the elements in the reply-to queue */
    QU_FREE(&pHeading->replyToQHead);

    /* If there's a replied-to message id, free it. */
    FREE(pHeading->pRepliedToMessageId);

    /* If there's a Subject, free it. */
    FREE(pHeading->pSubject);

    /* If there's a Mime Version, free it. */
    FREE(pHeading->pMimeVersion);

    /* If there's a Mime Content Type, free it. */
    FREE(pHeading->pMimeContentType);

    /* If there's a Mime Content Id, free it. */
    FREE(pHeading->pMimeContentId);

    /* If there's a Mime Content Description, free it. */
    FREE(pHeading->pMimeContentDescription);

    /* If there's a Mime Content Transfer Encoding, free it. */
    FREE(pHeading->pMimeContentTransferEncoding);

    /* Free the elements in the extension queue */
    QU_FREE(&pHeading->extensionQHead);

#undef FREE
}


static void
terminate(int sigNum)
{
    bTerminateRequested = TRUE;
}


static ReturnCode
initAgent(void)
{
    ReturnCode		    rc;

    /* Activate the agent's invoker SAP */
    if (ESRO_CB_sapBind(&hAgentInvoker,
			MTSUA_Rsap_Ua_MMAgentInvoker,
			ESRO_3Way,
			agentInvokeIndication, 
			agentResultIndication, 
			agentErrorIndication,
			agentResultConfirm, 
			agentErrorConfirm,
			agentFailureIndication) < 0)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_INIT,
		  "%s: "
		  "Could not activate ESROS invoker SAP for "
		  "module management agent.",
		  __applicationName));
	return Fail;
    }
      
    /* Activate the agent's performer SAP */
    if (ESRO_CB_sapBind(&hAgentPerformer,
			MTSUA_Rsap_Ua_MMAgentPerformer,
			ESRO_3Way,
			agentInvokeIndication, 
			agentResultIndication, 
			agentErrorIndication,
			agentResultConfirm, 
			agentErrorConfirm,
			agentFailureIndication) < 0)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_INIT,
		  "%s: "
		  "Could not activate ESROS performer SAP "
		  "for module management agent.",
		  __applicationName));
	return Fail;
    }
      
    /* Initialize the MM agent code */
    if ((rc = MM_agentInit(agentResponse)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_INIT,
		  "Could not initialize MM agent, reason 0x%lx\n",
		  (unsigned long) rc));
	return Fail;
    }

    return Success;
}



int
agentInvokeIndication(ESRO_SapDesc hSapDesc,
		      ESRO_SapSel sapSel, 
		      T_SapSel * pRemoteTsap,
		      N_SapAddr * pRemoteNsap,
		      ESRO_InvokeId invokeId, 
		      ESRO_OperationValue operationValue,
		      ESRO_EncodingType encodingType, 
		      DU_View hView)
{
    STR_String		hString = NULL;
    void *		hRequestBuf = NULL;
    ReturnCode		rc;

    
    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
	      "\n\tAgent INVOKE.ind from NSAP %u.%u.%u.%u\n"
	      "\tinvokeId = 0x%lx\n"
	      "\toperationValue = %d\n"
	      "\tsapSel = %u\n",
	      pRemoteNsap->addr[0], pRemoteNsap->addr[1],
	      pRemoteNsap->addr[2], pRemoteNsap->addr[3],
	      (unsigned long) invokeId, operationValue, sapSel));

    /* Attach the data memory to one of our string handles */
    if ((rc =
	 STR_attachString(DU_size(hView), DU_size(hView),
			  DU_data(hView), FALSE, &hString)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
		  "Agent could not attach DU string to STR string, "
		  "reason 0x%lx\n",
		  (unsigned long) rc));
	goto cleanUp;
    }
	
    /* Allocate a buffer to which we'll attach this string */
    if ((rc = BUF_alloc(0, &hRequestBuf)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
		  "Agent could not allocate buffer for attached string, "
		  "reason 0x%lx\n",
		  (unsigned long) rc));
	goto cleanUp;
    }
	
    /* Attach the string to a buffer so we can parse it */
    if ((rc = BUF_appendChunk(hRequestBuf, hString)) != Success)
    {
	TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
		  "Agent could not append STR string to buffer, "
		  "reason 0x%lx\n",
		  (unsigned long) rc));
	goto cleanUp;
    }

    /* Process the event */
    MM_agentProcessEvent(MM_RemOpType_Request,
			 operationValue,
			 (void **) &invokeId,
			 hRequestBuf);

  cleanUp:

    if (hString != NULL)
    {
	STR_free(hString);
    }
	
    if (hRequestBuf != NULL)
    {
	BUF_free(hRequestBuf);
    }

    return SUCCESS;
}


int
agentResultIndication(ESRO_InvokeId invokeId, 
		      ESRO_UserInvokeRef userInvokeRef,
		      ESRO_EncodingType encodingType, 
		      DU_View hView)
{
    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
	      "Agent received RESULT.ind for invokeId = 0x%lx\n",
	      (unsigned long) invokeId));

    return 0;
}


int
agentErrorIndication(ESRO_InvokeId invokeId, 
		     ESRO_UserInvokeRef userInvokeRef,
		     ESRO_EncodingType encodingType,
		     ESRO_ErrorValue errorValue,
		     DU_View hView)
{
    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
	      "Agent received ERROR.ind for invokeId = 0x%lx\n",
	      (unsigned long) invokeId));
    return 0;
}


int
agentResultConfirm(ESRO_InvokeId invokeId,
		   ESRO_UserInvokeRef userInvokeRef)
{
    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
	      "Agent received RESULT.con for invokeId = 0x%lx\n",
	      (unsigned long) invokeId));
    return 0;
}


int
agentErrorConfirm(ESRO_InvokeId invokeId,
		  ESRO_UserInvokeRef userInvokeRef)
{
    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
	      "Agent received ERROR.con for invokeId = 0x%lx\n",
	      (unsigned long) invokeId));
    return 0;
}


int
agentFailureIndication(ESRO_InvokeId invokeId,
		       ESRO_UserInvokeRef userInvokeRef,
		       ESRO_FailureValue failureValue)
{
    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
	      "Agent received FAILURE.ind for invokeId = 0x%lx\n",
	      (unsigned long) invokeId));
    return 0;
}


static ReturnCode
agentResponse(void ** phOperation,
	      MM_RemoteOperationType operationType,
	      MM_RemoteOperationValue  operationValue,
	      void ** phBuf)
{
    int				i;
    char *			pMem;
    char *			p;
    DU_View			hView = NULL;
    OS_Uint8			x;
    ESRO_InvokeId		invokeId = *(ESRO_InvokeId **) phOperation;
    OS_Uint16			chunkLen;
    OS_Uint16			errorVal;
    ReturnCode			rc;

    switch(operationType)
    {
    case MM_RemOpType_Request:
	/* should never get here */
	break;
	
    case MM_RemOpType_Result:
	/* If there's result data... */
	if (phBuf != NULL && *phBuf != NULL)
	{
	    /* Allocate a DU View for our buffer contents */
	    if ((hView =
		 DU_alloc(G_duMainPool,
			  (Int) BUF_getBufferLength(*phBuf))) == NULL)
	    {
		TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
			  "Agent could not allocate DU for result\n"));
		return ResourceError;
	    }
	
	    /* For each buffer segment... */
	    for (pMem = DU_data(hView); ; pMem += chunkLen)
	    {
		/* ... get the segment data, ... */
		chunkLen = 0;
		if ((rc = BUF_getChunk(*phBuf,
				       &chunkLen,
				       (unsigned char **) &p)) != Success)
		{
		    if (rc == BUF_RC_BufferExhausted)
		    {
			/* Normal loop exit; no more segments. */
			break;
		    }
		
		    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
			      "Agent could not add result data result\n"));
		    DU_free(hView);
		    return rc;
		}
	    
		/* ... and copy it into our contiguous memory. */
		OS_copy(pMem, (unsigned char *) p, chunkLen);
	    }
	}

	/* Send the result */
	if (ESRO_CB_resultReq(invokeId, NULL,
			      ESRO_EncodingBER, hView) != SUCCESS)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
		      "Agent error sending RESULT.req to manager\n"));
	}
	break;
	
    case MM_RemOpType_Event:
	/* no event implemented yet */
	break;
	
    case MM_RemOpType_Error:
	/* Retrieve the Error Code from the buffer */
	for (errorVal = 0, i=0; i<2; i++)
	{
	    if ((rc = BUF_getOctet(*phBuf, &x)) != Success)
	    {
		TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
			  "Agent could not parse error value for "
			  "ERROR.req\n"));
		return Success;
	    }

	    errorVal = (errorVal << 8) | x;
	}

	/* Send the error */
	if (ESRO_CB_errorReq(invokeId, NULL,
			     ESRO_EncodingBER, errorVal, NULL) != SUCCESS)
	{
	    TM_TRACE((uash_globals.hTM, UASH_TRACE_AGENT,
		      "Agent error sending ERROR.req to manager\n"));
	}
	break;
	
    case MM_RemOpType_Failure:
	/* nothing we can do about this */
	break;

    case MM_RemOpType_User:
    case MM_RemOpType_Reserved:
	/* nothing to do here */
	break;
    }

    if (hView != NULL)
    {
	DU_free(hView);
    }

    return Success;
}


static void
agentValueChanged(void * hUserData,
		  void * hManagableObject,
		  MM_Value * pValue)
{
    int				a;
    int				b;
    int				c;
    int				d;
    char *			pMsg = NULL;
    ReturnCode 			rc;
    ReturnCode 			rcOld;
    NVM_Memory			hMem;
    NVM_Transaction 		hOld;
    NVM_Transaction 		hNew;
    NVM_TransInProcess		hTip;
    N_SapAddr *			pMtsNsap;

    /* If initialization is still in process, don't do anything */
    if (bInitializing)
    {
	return;
    }

    if (hManagableObject == hMmPassword)
    {
	/* Get the old MTS password, if one exists. */
	rcOld =
	    NVM_nextTrans(TransType_Password, NVM_FIRST, &hOld);

	/* Begin a new transaction to add the new password */
	if ((rc = NVM_beginTrans(TransType_Password, &hTip, &hNew)) != Success)
	{
	    goto failPassword;
	}

	/* Allocate NVM for the new password */
	if ((rc =
	     NVM_allocStringCopy(hTip, pValue->un.string, &hMem)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    goto failPassword;
	}

	/* End the transaction */
	if ((rc = NVM_endTrans(hTip)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    goto failPassword;
	}

	/* Specify the new password to the UA Core */
	if ((rc = UA_setPassword(pValue->un.string)) != Success)
	{
	    /* It failed.  Free the new password */
	    (void) NVM_freeTrans(hNew);

	    goto failPassword;
	}
	else
	{
	    /* Free the old password (if there was one) */
	    if (rcOld == Success)
	    {
		if ((rc = NVM_freeTrans(hOld)) != Success)
		{
		    (void) NVM_freeTrans(hNew);
		    goto failPassword;
		}
	    }
	}

	pMsg = "MTS Password has changed to (%s)\n";

      failPassword:
	if (pMsg == NULL)
	{
	    pMsg = "MTS Password change to (%s) FAILED!\n";
	}

    }
    else if (hManagableObject == hMmMtsNsap)
    {
	/* Get the old MTS NSAP, if one exists. */
	rcOld = NVM_nextTrans(TransType_MtsNSAP, NVM_FIRST, &hOld);

	/* Begin a new transaction to add the new MTS NSAP */
	if ((rc = NVM_beginTrans(TransType_MtsNSAP, &hTip, &hNew)) != Success)
	{
	    goto failMtsNsap;
	}

	/* Allocate NVM for the new MTS NSAP */
	if ((rc = NVM_alloc(hTip, sizeof(*pMtsNsap), &hMem)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    goto failMtsNsap;
	}

	/* End the transaction */
	if ((rc = NVM_endTrans(hTip)) != Success)
	{
	    (void) NVM_abortTrans(hTip);
	    goto failMtsNsap;
	}

	/* Get a pointer to the MTS NSAP Address in NVM */
	pMtsNsap = NVM_getMemPointer(hNew, hMem);
	
	/* Create an NSAP Address structure from the MTS NSAP string */
	if ((pMtsNsap->len = sscanf(pValue->un.string,
				    "%d.%d.%d.%d%s",
				    &a, &b, &c, &d,
				    uash_globals.tmpbuf)) != 4)
	{
	    /* It failed.  Free the new MTS NSAP */
	    (void) NVM_freeTrans(hNew);

	    goto failMtsNsap;
	}

	pMtsNsap->addr[0] = a;
	pMtsNsap->addr[1] = b;
	pMtsNsap->addr[2] = c;
	pMtsNsap->addr[3] = d;

	/* Specify the new MTS NSAP to the UA Core */
	if ((rc = UA_setMtsNsap(pMtsNsap)) != Success)
	{
	    /* It failed.  Free the new MTS NSAP */
	    (void) NVM_freeTrans(hNew);

	    goto failMtsNsap;
	}
	else
	{
	    /* Free the old MTS NSAP (if there was one) */
	    if (rcOld == Success)
	    {
		if ((rc = NVM_freeTrans(hOld)) != Success)
		{
		    (void) NVM_freeTrans(hNew);
		    goto failMtsNsap;
		}
	    }
	}

	pMsg = "MTS NSAP has changed to (%s)\n";

      failMtsNsap:
	if (pMsg == NULL)
	{
	    pMsg = "MTS NSAP change to (%s) FAILED!\n";
	}
    }
	

    /* Let 'em know the result. */
    wmove(stdscr, LINES-1, 0);
    wprintw(stdscr, pMsg, pValue->un.string);
    wrefresh(stdscr);
}
