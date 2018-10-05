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
#include "config.h"
#include "tmr.h"
#include "nvm.h"
#include "sch.h"
#include "asn.h"
#include "du.h"
#include "eh.h"
#include "getopt.h"
#include "udp_if.h"
#include "udp_po.h"
#include "esro_cb.h"
#include "emsdmail.h"
#include "emsd822.h"
#include "ua.h"
#include "uashloc.h"
#include "lic.h"

#include "relid.h"

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

UASH_Statistics statistics = {0, 0, 0, 0, 0, 0, 0, 0};

/*
 * Local Types and Variables
 */

static struct
{
    char *	    pLicenseFile;
    char *	    pFromPineSpoolDir;
    char *	    pMtsNsap;
    char *	    pFrom;
    char *	    pPassword;
    char *	    pDeliveryVerifySupported;
    char *	    pNonVolatileMemoryEmulation;
    char *	    pNonVolatileMemorySize;
    char *	    pConcurrentOperationsInbound;
    char *	    pConcurrentOperationsOutbound;
#if defined(OS_TYPE_MSDOS)
    char *	    pPineMessageFormat;
#endif
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    char *	    pMaxInactiveIntervals;
#endif
    char *	    pMessageSubmissionAttemptInterval;
    char *	    pFetchMessagesOnStartup;
} config = { NULL };

static void *	    hFromPineSpoolDir = NULL;

static struct ConfigParams
{
    char *	    pSectionName;
    char *	    pTypeName;
    char **	    ppValue;
} configParams[] =
  {
      {
	  "License",
	  "License File",
	  &config.pLicenseFile
      },
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
	  "Features",
	  "Fetch Messages on Startup",
    	  &config.pFetchMessagesOnStartup
      },
#ifdef OS_TYPE_MSDOS
      {
	  "Features",
	  "Pine Message Format",
	  &config.pPineMessageFormat
      },
#endif
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
      {
	  "Features",
	  "Maximum Inactive Intervals",
	  &config.pMaxInactiveIntervals
      },
#endif
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
	  "Submission",
	  "New Message Directory",
	  &config.pFromPineSpoolDir
      },
      {
	  "Submission",
	  "Message Submission Attempt Interval",
	  &config.pMessageSubmissionAttemptInterval
      },
      {
	  "Communications",
	  "MTS NSAP Address",
	  &config.pMtsNsap
      },
      {
	  "Communications",
	  "FROM",
	  &config.pFrom
      },
      {
	  "Communications",
	  "Password",
	  &config.pPassword
      }
  };

static OS_Boolean	bTerminateRequested = FALSE;
static OS_Boolean	bNoMemDebug = FALSE;

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
 * Forward Declarations
 */

FORWARD PUBLIC ReturnCode
checkSubmit(void * hTimer,
	    void * hUserData1,
	    void * hUserData2);

FORWARD static ReturnCode
submitRfc822Message(ENVELOPE * pRfc822Env,
		    char * pRfc822Body,
		    char *pHeaderStr,
		    void *hShellReference);

FORWARD static void
getPrintableRfc822Address(char * pBuf,
			  ADDRESS * pAddr);

FORWARD static ReturnCode
addLocalAddr(ADDRESS * pAddr,
	     UASH_Recipient * pRecip,
	     char * pBuf);

FORWARD static int
getPrintableRfc822AddressLength(ADDRESS * pAddr);

FORWARD static ReturnCode
writeRfc822Message(char * pDataFileName,
		   ENVELOPE * pRfcEnv,
		   char * pExtraHeaders,
		   char * pBody);

FORWARD static void
writeRfc822Header(void * hFolder,
		  ENVELOPE * pRfcEnv,
		  char * pFieldName,
		  void * pFieldData,
		  void (*pfAddRfcHeaderOrAddress)());

FORWARD static void
freeHeadingFields(UASH_MessageHeading * pHeading);

FORWARD static ReturnCode
parseHeaderString(char *pHeaderStr,
		  UASH_MessageHeading * pHeading);

FORWARD static OS_Boolean
dirFilter(const char * pName);

FORWARD void
uash_displayMessage(char * pBuf);

FORWARD static void
terminate(int sigNum);

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
FORWARD static Void
keyboardInput(Ptr arg);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Windows)
static void
displayError(char * pMessageType,
	     char * pFileName,
	     int lineNum,
	     char * pMessage);
#endif

extern struct TimerValue {
    Int	    acknowledgment;
    Int	    roundtrip;
    Int	    retransmit;
    Int	    maxNSDULifeTime;
} timerValue; 
extern int retransmitCount;

Char *copyrightNotice;

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
OS_Sint32  maxInactiveIntervals = -1;
OS_Sint32  inactiveIntervalsCounter = 0;
#endif

OS_Uint32 messageSubmissionAttemptInterval = 10;
OS_Uint32 fetchMessagesOnStartup = TRUE;

/*
 * Main Line
 */
void
UASH_init(int argc, char * argv[])
{
    int				c;
    int				j;
    OS_Uint32 			val[32];
    char 			buf[32];
    char *			p;
    char *			pEmsdAddress;
    char *			pMailBox;
    char *			pPduLog = "ua_pdu.log";
    char *			pErrLog = "ua_err.log";
    static char 		pConfigFile[256] = "";
    void *			hConfig;
    void *			hTimer;
    OS_Uint32			nvFileSize;
    N_SapAddr			mtsNsap;
    ESRO_RetVal			retval;
    ReturnCode			rc;
    struct ConfigParams *	pConfigParam;
    UASH_LocalUser * 		pUser;

#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
    setbuf(stdout, NULL);
    __applicationName = argv[0];
#endif

    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
        uash_displayMessage(argv[0]);
	EH_fatal("main: Get copyright notice failed!\n");
    }
#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
    uash_displayMessage(copyrightNotice);
#else
    {
	char *s = copyrightNotice;
	char buf[128], *p;	/* NOTYET -- watch for buf overrun */
	for (;*s;) {
	    for (p = buf; *s != '\n';) 
	    	*p++ = *s++;
	    *p = '\0'; s++;
    	    uash_displayMessage(buf);
	}
    }
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_QuickWin)
    /* If we ever call exit(), prompt for whether to destroy windows */
    if (_wsetexit(_WINEXITPROMPT) != 0) {
    	EH_problem("wsetexit() failed");
    } 
    if (_wabout(copyrightNotice) != 0) { /* NOTYET--this fails */
    	EH_problem("wabout() failed");
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
	sprintf(uash_globals.tmpbuf, "Could not initialize OS module!");
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Initialize the OS memory allocation debug routines */
    OS_allocDebugInit(NULL);

    /* Initialize the OS file tracking debug routines */
    OS_fileDebugInit("/tmp/uashpine.fil");

#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:xc:lp:e:V")) != EOF)
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

	default:
	    break;
	}
    }
#else
    /* Get configuration file name */
    strcpy(pConfigFile, argv[1]);
#endif

    if (strlen(pConfigFile) == 0)
    {
	strcpy(pConfigFile, "ua.ini");
    }

    /* Initialize the config module */
    if ((rc = CONFIG_init()) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not initialize CONFIG module!",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Open the configuration file.  Get a handle to the configuration data */
    if ((rc = CONFIG_open(pConfigFile, &hConfig)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not open configuration file (%.*s), "
		"reason 0x%04x!",
		__applicationName,
		(int) (sizeof(uash_globals.tmpbuf) / 2), pConfigFile, rc);
	EH_fatal(uash_globals.tmpbuf);
    }

#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
    optind = 0;
    while ((c = getopt(argc, argv, "T:xc:lp:e:")) != EOF)
    {
        switch (c)
	{
	case 'x':
	    bNoMemDebug = TRUE;
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

	case 'e':		/* UDP_PCO Error log file name */
	    pErrLog = optarg;
	    break;

	default:
	    sprintf(uash_globals.tmpbuf,
		    "usage: %s [-T <TM option string>] "
		    "<configuration file name>!",
		    __applicationName);
	    EH_fatal(uash_globals.tmpbuf);
	    break;
	}
    }
#endif

    /* Give ourselves a trace handle */
    if (TM_OPEN(uash_globals.hModCB, "UASH") == NULL)
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

	TM_TRACE((uash_globals.hModCB, UASH_TRACE_INIT,
		  "\n    Configuration parameter\n"
		  "\t[%s]:%s =\n\t%s\n",
		  pConfigParam->pSectionName,
		  pConfigParam->pTypeName,
		  *pConfigParam->ppValue));
    }

#if 0
    NOTYET Because of NT Differences
    LIC_check(config.pLicenseFile);
#endif

    /* Initialize the queue of local users */
    QU_INIT(&uash_globals.localUserQHead);

    /* Get the list of local users for this user agent */
    for (j = 1, rc = Success; rc == Success; j++)
    {
	/* Create the parameter type string */
	sprintf(buf, "User %d", j);
	if ((rc = CONFIG_getString(hConfig,
				   "Local Users",
				   buf,
				   &pEmsdAddress)) == Success)
	{
	    /* Get this user's mail box */
	    if ((rc = CONFIG_getString(hConfig,
				       pEmsdAddress,
				       "MailBox",
				       &pMailBox)) == Success)
	    {
		/* Allocate a User structure for this potential recipient */
		if ((pUser = (OS_alloc(sizeof(UASH_LocalUser) +
			      strlen(pEmsdAddress) + 1 +
			      strlen(pMailBox) + 1))) == NULL)
		{
		    EH_fatal("Out of memory getting user mailbox.");
		}

		/* Initialize the queue pointers */
		QU_INIT(pUser);

		/*
		 * Point to the end of the structure where we've left
		 * room for the recipient name and mailbox.
		 */
		pUser->pEmsdAddr = (char *) (pUser + 1);
		pUser->emsdAddr  = 0;

		/* Copy the recipient address */
		strcpy(pUser->pEmsdAddr, pEmsdAddress);

		/* Determine the numeric value for this EMSD address */
		for (p = pUser->pEmsdAddr; *p ;p++)
		{
		    if (*p == '.')
		    {
			continue;
		    }
		    if (isdigit(*p))
		    {
		        pUser->emsdAddr = pUser->emsdAddr * 10 + (*p - '0');
		    }
		}

		if (pUser->emsdAddr <= 0)
		{
		    sprintf(uash_globals.tmpbuf,
			    "Illegal Local EMSD Address: %s!",
			    pUser->pEmsdAddr);
		    EH_fatal(uash_globals.tmpbuf);
		}

		/*
		 * Point to the area that's reserved for the mailbox file
		 * name.
		 */
		pUser->pMailBox = pUser->pEmsdAddr + strlen(pEmsdAddress) + 1;

		/* Copy the mailbox name */
		strcpy(pUser->pMailBox, pMailBox);

		/* Insert this recipient onto our "users" queue */
		QU_INSERT(pUser, &uash_globals.localUserQHead);
		
		TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
			  "\n\tUser: %s\n\tMailbox: %s\n",
			  pUser->pEmsdAddr,
			  pUser->pMailBox));
	    }
	}
    }

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    maxInactiveIntervals = atol(config.pMaxInactiveIntervals);
#endif
    messageSubmissionAttemptInterval = atol(config.pMessageSubmissionAttemptInterval);

    if (OS_strcasecmp(config.pFetchMessagesOnStartup, "NO") == 0  ||
        OS_strcasecmp(config.pFetchMessagesOnStartup, "FALSE") == 0)
    {
    	fetchMessagesOnStartup = FALSE;
    }

    /* Initialize the TMR module */
    TMR_init(NUMBER_OF_TIMERS, CLOCK_PERIOD);
      
    /* Initialize the SCH module, using target.h number of tasks */
    SCH_init(MAX_SCH_TASKS);
      
    /* Initialize the non-volatile memory module */
    if ((rc = NVM_init()) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize NVM module, reason 0x%x!",
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
		    "reason 0x%x!",
		    __applicationName, rc);
	    EH_fatal(uash_globals.tmpbuf);
	}
    }
    else
    {
	nvFileSize = atol(config.pNonVolatileMemorySize);
	if ((rc = NVM_open(config.pNonVolatileMemoryEmulation,
			   nvFileSize)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "%s: "
		    "Could not create non-volatile memory file, "
		    "reason 0x%x!",
		    __applicationName, rc);
	    EH_fatal(uash_globals.tmpbuf);
	}
    }

    /* Initialize the ASN Module */
    if ((rc = ASN_init()) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize ASN module!",
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
		"Could not initialize ESRO module!",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Create an NSAP Address structure from the MTS NSAP string */
    for (p = config.pMtsNsap, mtsNsap.len = 0;
         mtsNsap.len < 4 && isdigit(*p);
	 mtsNsap.len++)
    {
	val[mtsNsap.len] = atol(p);
	for (; isdigit(*p); p++)
	    ;
	if (*p == '.')
	{
	    ++p;
	}
    }

    /* Was it a legal NSAP? */
    if (mtsNsap.len < 2 || mtsNsap.len > 4)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Invalid MTS Nsap address: %s!",
		__applicationName,
		config.pMtsNsap);
	EH_fatal(uash_globals.tmpbuf);
    }

    mtsNsap.addr[0] = val[0];
    mtsNsap.addr[1] = val[1];
    mtsNsap.addr[2] = val[2];
    mtsNsap.addr[3] = val[3];

    /* Initialize the UA Core */
    if ((rc = UA_init(&mtsNsap,
		      *config.pPassword == '\0' ? NULL : config.pPassword,
		      (OS_Boolean)
		        (OS_strcasecmp(config.pDeliveryVerifySupported,
				       "TRUE") == 0),
		      (OS_Uint8) TransType_Verify,
		      (OS_Uint8) TransType_DupDetection,
		      (OS_Uint32) atol(config.pConcurrentOperationsInbound),
		      (OS_Uint32) atol(config.pConcurrentOperationsOutbound),
		      uash_messageReceived,
		      uash_modifyDeliverStatus,
		      uash_submitSuccess,
		      uash_submitFail,
		      uash_deliveryReport,
		      uash_nonDeliveryReport
#ifdef DELIVERY_CONTROL
		    , uash_deliveryControlSuccess,
		      uash_deliveryControlError,
		      uash_deliveryControlFail
#endif
						)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize UA Core!",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Set the directory filter */
    OS_dirSetFilter(dirFilter);

    /* Open the submission new message directory */
    if ((rc =
	 OS_dirOpen(config.pFromPineSpoolDir, &hFromPineSpoolDir)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not open submission new-message directory %s!", 
		__applicationName, config.pFromPineSpoolDir);
	EH_fatal(uash_globals.tmpbuf);
    }
      
    /* Create a timer to spool messages destined to devices */
    if ((rc = TMR_start(TMR_SECONDS(0),
			NULL, NULL,
			checkSubmit, &hTimer)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not start initial timer to process; "
		"submissions from Pine!",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }
      
    /* Start clock interrupts */
    if (TMR_startClockInterrupt(CLOCK_PERIOD) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not start clock interrupt!",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }
      
#ifdef OS_TYPE_UNIX
    /* If we get a SIGTERM or SIGINT, request the main loop to exit. */
    (void) signal(SIGTERM, terminate);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    signal(SIGINT, terminate);
#endif

#ifdef OS_TYPE_UNIX
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

    fprintf(stderr, "\n------------ Timer Values ------------\n\tAcknowledgement\t=%d\n\tRoundtrip\t=%d",
            timerValue.acknowledgment, timerValue.roundtrip);
    fprintf(stderr, "\n\tretransmit\t=%d\n\tmaxNSDULifeTime\t=%d",
            timerValue.retransmit, timerValue.maxNSDULifeTime);
    fprintf(stderr, "\n\tretransTimeout\t=%d\n\trwait\t\t=%d\n\tnuOfRetrans\t=%d\n\trefKeepTime\t=%d",
	    timerValue.retransmit + timerValue.roundtrip, 
	    2 * (timerValue.acknowledgment + timerValue.retransmit), 
	    retransmitCount, 
	    (2 * timerValue.maxNSDULifeTime) + 
	    retransmitCount * timerValue.retransmit);
    
    fprintf(stderr, "\n\tinactivityDelay\t=%d\n\tperfNoResponse\t=%d\n--------------------------------------\n",
	    4 * timerValue.roundtrip,
	    (2 * timerValue.maxNSDULifeTime) + 
	    retransmitCount * (timerValue.retransmit + timerValue.roundtrip));
#endif

#ifdef DELIVERY_CONTROL
    if (fetchMessagesOnStartup)
    {
    	void *			hMessage;
    	void *			hShellReference = "";

    	/* Issue Delivery Control */
    	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	         "Issue delivery Control..."));

    	if ((rc = UA_issueDeliveryControl(&hMessage,
			                  hShellReference)) != Success)
    	{
	    sprintf(uash_globals.tmpbuf, "Can not contact Message Center "
		   			 "because of a local problem!");
    	    uash_displayMessage(uash_globals.tmpbuf);

    	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	             "Delivery Control failed locally."));
    	}
    	else
    	{
	    sprintf(uash_globals.tmpbuf, "Delivery Request sent "
		   			 "(pending confirmation)!");
    	    uash_displayMessage(uash_globals.tmpbuf);

	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
		     "Delivery Control Operation started "
		     "(pending confirmation)\n"));
    	}
    }
#endif

} /* UASH_init() */



/*
 * Interface to Pine -- Deliver a message
 */

ReturnCode
uash_deliverMessage(ENVELOPE * pRfcEnv,
		    char * pExtraHeaders,
		    char * pBodyText,
		    QU_Head * pRecipInfoQHead)
{
    ReturnCode			rc;
    UASH_LocalUser *		pLocalUser;
    UASH_RecipientInfo *	pRecipInfo;
    OS_Boolean			bFoundOneEmsdUser           = FALSE;
    OS_Boolean			bFoundOneLocalEmsdUser      = FALSE;
    OS_Boolean			bMessageDeliveredToMailbox = FALSE;
    char			buf[512];

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	      "Deliver Message to user..."));

    /* For each recipient... */
    for (pRecipInfo = QU_FIRST(pRecipInfoQHead);
	 ! QU_EQUAL(pRecipInfo, pRecipInfoQHead);
	 pRecipInfo = QU_NEXT(pRecipInfo))
    {
	/* Is this recipient address in "local EMSD" format? */
	if (pRecipInfo->recipient.style != UASH_RecipientStyle_Emsd)
	{
	    /* Nope.  Therefore, he's not local. */
	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
		      "\tRecipient not local (RFC-822): %s",
		      pRecipInfo->recipient.un.pRecipRfc822));
	    continue;
	}

        TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
		 "\tIs user %06ld local?",
		 pRecipInfo->recipient.un.recipEmsd.emsdAddr));

	/* Set flag to determine if we found this recipient. */
        bFoundOneEmsdUser      = TRUE;
	bFoundOneLocalEmsdUser = FALSE;

	/* Search the Local User queue to see if he's one of ours */
	for (pLocalUser = QU_FIRST(&uash_globals.localUserQHead);
	     ! QU_EQUAL(pLocalUser, &uash_globals.localUserQHead);
	     pLocalUser = QU_NEXT(pLocalUser))
	{
	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
		      "\tComparing against local user %06ld",
		      pLocalUser->emsdAddr));

	    /* Does this local user match the recipient we're looking at? */
	    if (pLocalUser->emsdAddr !=
		pRecipInfo->recipient.un.recipEmsd.emsdAddr)
	    {
		/* Nope.  Try next local user. */
		continue;
	    }

	    /* We found one that matches. Write the message to his mailbox */
	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
		      "\tDeliver to user %06ld in mailbox %s",
		      pLocalUser->emsdAddr,
		      pLocalUser->pMailBox));
	    
	    if ((rc =
		 writeRfc822Message(pLocalUser->pMailBox,
				    pRfcEnv,
				    pExtraHeaders,
				    pBodyText)) != Success)
	    {
		TM_TRACE((uash_globals.hModCB, UASH_TRACE_ERROR,
			  "===> ERROR: Could not deliver to mailbox %s, "
			  "reason 0x%04x",
			  pLocalUser->pMailBox, rc));
	    }

	    /* Indicate that we found one */
	    bFoundOneLocalEmsdUser = TRUE;

	    /* Indicate that we delivered message to mailbox */
    	    bMessageDeliveredToMailbox = TRUE;

	    /* Display a message */
	    sprintf(buf,
	    	    "Deliver to %06ld successful [Msg Id=%s]!",
		    pLocalUser->emsdAddr,
		    pRfcEnv->message_id);

	    uash_displayMessage(buf);

	    statistics.successfulDeliveries ++;

	    /* break;    Try to deliver to multiple mailboxes */
	}

	if (! bFoundOneLocalEmsdUser)
	{
	    /* Make sure we found at least one recipient to deliver to */
	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
		      "===> Delivery to user %06ld failed; "
		      "no such local user, "
		      "in the message with id:\n\t%s",
		      pRecipInfo->recipient.un.recipEmsd.emsdAddr,
		      pRfcEnv->message_id));
	    
	    sprintf(buf,
		    "Deliver to %06ld FAILED; no such local user [Msg Id=%s]!",
		    pRecipInfo->recipient.un.recipEmsd.emsdAddr,
		    pRfcEnv->message_id);
	    
	    uash_displayMessage(buf);
	}
    }

    if (! bFoundOneEmsdUser)
    {
    	/* We have not found any EMSD recipient to deliver the message to */
	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	      	"===> No EMSD User found in recipient list!! "));
	    
	sprintf(buf, "No EMSD user (address) found in the recipient list!");
	uash_displayMessage(buf);
    }

    if (! bMessageDeliveredToMailbox)
    {
    	/* We have not found any local EMSD user to deliver the message to */
	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	      	"===> None of the Users in recipient list "
		"matched the local user(s) !! "));
	    
	sprintf(buf, "No EMSD user (address) matched the local user!");
        uash_displayMessage(buf);

	pLocalUser = QU_FIRST(&uash_globals.localUserQHead);
	    
	if ((rc = writeRfc822Message(pLocalUser->pMailBox,
				     pRfcEnv,
				     pExtraHeaders,
				     pBodyText)) != Success)
	{
	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ERROR,
		     "===> ERROR: Could not deliver to mailbox %s, "
		     "reason 0x%04x",
		     pLocalUser->pMailBox, rc));
	}

	sprintf(buf, "Message is delivered to: %s !", pLocalUser->pMailBox);
        uash_displayMessage(buf);

        return Fail;

    }

    return Success;

} /* uash_deliverMessage() */




/*
 * Local Function Declarations
 */


PUBLIC ReturnCode
checkSubmit(void * hTimer,
	    void * hUserData1,
	    void * hUserData2)
{
    ReturnCode 		rc;
    MAILSTREAM *	pRfcMessage;
    ENVELOPE *		pRfcEnv;
    char *		pHeaderText = NULL;
    char *		pBodyText;
    char * 		pFileName;
    char *		p;
    
    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	      "Checking for newly-submitted (from Pine) messages..."));

    /* See if there are messages to be handled. */
    if ((rc = OS_dirFindNext(hFromPineSpoolDir, &pFileName)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"Error searching pine spool directory; rc = 0x%x!", rc);
	EH_fatal(uash_globals.tmpbuf);
	return rc;
    }
    
    /* If there were no entries, rewind once and try again */
    if (pFileName == NULL)
    {
	if ((rc = OS_dirRewind(hFromPineSpoolDir)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "Error rewinding pine spool directory; rc = 0x%x!", rc);
	    EH_fatal(uash_globals.tmpbuf);
	}
	
	if ((rc = OS_dirFindNext(hFromPineSpoolDir, &pFileName)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "Error searching pine spool directory; rc = 0x%x!", rc);
	    EH_fatal(uash_globals.tmpbuf);
	}
    }
    
    /* Was there a message waiting for us? */
    if (pFileName == NULL)
    {
	/* Nope. Create a timer to call this function again (later) */
	if ((rc = TMR_start(TMR_SECONDS(messageSubmissionAttemptInterval), 
			    NULL, NULL, checkSubmit, NULL)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "Could not start timer to check for submissions; "
		    "rc = 0x%x!", rc);
	    EH_fatal(uash_globals.tmpbuf);
	}
	
	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
		  "No newly-submitted messages found.\n"));

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    	if (maxInactiveIntervals > 0 && ++inactiveIntervalsCounter > maxInactiveIntervals) {
	    bTerminateRequested = TRUE;
	}
#endif

	return Success;
    }
    
    /*
     * Yup.  Process it.
     */
    
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    inactiveIntervalsCounter = 0;
#endif

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	      "Found newly-submitted message file (%s)\n", pFileName));

    /* Open the RFC-822 message */
    pRfcMessage = NULL;
    if ((pRfcMessage = mail_open(pRfcMessage, pFileName, NIL, NULL)) == NULL)
    {
	sprintf(uash_globals.tmpbuf,
		"Error opening RFC-822 message (%s)!",
		pFileName);
	EH_problem(uash_globals.tmpbuf);
	rc = Fail;
	goto cleanUp;
    }

    /* Retrieve the RFC-822 Envelope */
    if ((pRfcEnv = mail_fetchstructure(pRfcMessage, 1, NULL)) == NULL)
    {
	sprintf(uash_globals.tmpbuf,
		"Error retrieving RFC-822 Envelope from message (%s)!",
		pFileName);
	EH_problem(uash_globals.tmpbuf);
	rc = Fail;
	goto cleanUp;
    }
    
    /* Get a pointer to the RFC-822 header text */
    p = mail_fetchheader(pRfcMessage, 1);

    /* Copy header, since the same buffer is returned by fetchtext */
    if ((pHeaderText = OS_allocStringCopy(p)) == NULL)
    {
	sprintf(uash_globals.tmpbuf,
		"Could not allocate memory for Header Text of message (%s)!",
		pFileName);
	EH_problem(uash_globals.tmpbuf);
	rc = Fail;
	goto cleanUp;
    }

    /* Get a pointer to the body text */
    pBodyText = mail_fetchtext(pRfcMessage, 1);
    
    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
	      "\n\n\nHeader = (%s)\n\n\n", pHeaderText));
    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
	      "Body = (%s)\n\n\n", pBodyText));

    /* Submit the message */ 	
    /* pFileName is passed temporarily. It should be ShellReference handle */
    if ((rc = submitRfc822Message(pRfcEnv, pBodyText, pHeaderText, 
				  (void *)pFileName)) 
	!= Success)
    {
	sprintf(uash_globals.tmpbuf,
		"Error submitting message to UA CORE, reason 0x%x!", rc);
	EH_problem(uash_globals.tmpbuf);
	rc = Fail;
    }

  cleanUp:

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	      "Done processing newly-submitted message (%s)\n", pFileName));

    /* Close the RFC-822 message.  We're done with it. */
    if (pRfcMessage != NULL)
    {
	mail_close(pRfcMessage);
    }

    /* We're done with the header text */
    if (pHeaderText != NULL)
    {
	OS_free(pHeaderText);
    }

    /* We're done with the Filename pointer */
    OS_free(pFileName);

    if (rc == Fail)
    {
	/* Nope. Create a timer to call this function again (later) */
        TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
	         "Schedule later polling"));

	if ((rc = TMR_start(TMR_SECONDS(messageSubmissionAttemptInterval), 
			    NULL, NULL, checkSubmit, NULL)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "Could not start timer to check for submissions; "
		    "rc = 0x%x!", rc);
	    EH_fatal(uash_globals.tmpbuf);
	}
    } 
 
    /* See 'ya */
    return Success;

} /* checkSubmit() */



static ReturnCode
submitRfc822Message(ENVELOPE * pRfc822Env,
		    char * pRfc822Body,
		    char *pHeaderStr,
		    void *hShellReference)
{
    void *			hMessage;
    UASH_MessageHeading		heading;
    UASH_Recipient *		pRecip;
    UASH_RecipientInfo * 	pRecipInfo;
    ADDRESS * 			pAddr;
    ReturnCode 			rc;
    
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

    /*
     * Fill in the heading from the RFC-822 envelope info
     */
    
    /* Get the sender */
    if (pRfc822Env->sender != NULL)
    {
	addLocalAddr(pRfc822Env->sender, &heading.sender, NULL);
    }
    
    /* Get the originator */
    addLocalAddr(pRfc822Env->from, &heading.originator, NULL);

    /* Get the primary (To:) recipients */
    for (pAddr = pRfc822Env->to; pAddr != NULL; pAddr = pAddr->next)
    {
	/* Allocate a RecipientInfo structure */
	if ((pRecipInfo =
	     OS_alloc(sizeof(UASH_RecipientInfo) +
		      getPrintableRfc822AddressLength(pAddr))) == NULL)
	{
	    freeHeadingFields(&heading);
	    return ResourceError;
	}

	/* Initialize the queue pointers */
	QU_INIT(pRecipInfo);

	/* Add this recipient to the heading */
	QU_INSERT(pRecipInfo, &heading.recipInfoQHead);

	/* Initialize the RecipientInfo structure */
	pRecipInfo->recipientType = UASH_RecipientType_Primary;
	pRecipInfo->bReceiptNotificationRequested = FALSE;
	pRecipInfo->bNonReceiptNotificationRequested = FALSE;
	pRecipInfo->bMessageReturnRequested = FALSE;
	pRecipInfo->bNonDeliveryReportRequested = TRUE;
	pRecipInfo->bDeliveryReportRequested = FALSE;
	pRecipInfo->bReplyRequested = FALSE;

	/* Get the recipient address */
	addLocalAddr(pAddr, &pRecipInfo->recipient, (char *) (pRecipInfo + 1));
    }
    
    /* Get the copy (Cc:) recipients */
    for (pAddr = pRfc822Env->cc; pAddr != NULL; pAddr = pAddr->next)
    {
	/* Allocate a RecipientInfo structure */
	if ((pRecipInfo =
	     OS_alloc(sizeof(UASH_RecipientInfo) +
		      getPrintableRfc822AddressLength(pAddr))) == NULL)
	{
	    freeHeadingFields(&heading);
	    return ResourceError;
	}

	/* Initialize the queue pointers */
	QU_INIT(pRecipInfo);

	/* Add this recipient to the heading */
	QU_INSERT(pRecipInfo, &heading.recipInfoQHead);

	/* Initialize the RecipientInfo structure */
	pRecipInfo->recipientType 		     = UASH_RecipientType_Copy;
	pRecipInfo->bReceiptNotificationRequested    = FALSE;
	pRecipInfo->bNonReceiptNotificationRequested = FALSE;
	pRecipInfo->bMessageReturnRequested          = FALSE;
	pRecipInfo->bNonDeliveryReportRequested      = TRUE;
	pRecipInfo->bDeliveryReportRequested         = FALSE;
	pRecipInfo->bReplyRequested  		     = FALSE;

	/* Get the recipient address */
	addLocalAddr(pAddr, &pRecipInfo->recipient, (char *) (pRecipInfo + 1));
    }
    
    /* Get the blind copy (Bcc:) recipients */
    for (pAddr = pRfc822Env->bcc; pAddr != NULL; pAddr = pAddr->next)
    {
	/* Allocate a RecipientInfo structure */
	if ((pRecipInfo =
	     OS_alloc(sizeof(UASH_RecipientInfo) +
		      getPrintableRfc822AddressLength(pAddr))) == NULL)
	{
	    freeHeadingFields(&heading);
	    return ResourceError;
	}

	/* Initialize the queue pointers */
	QU_INIT(pRecipInfo);

	/* Add this recipient to the heading */
	QU_INSERT(pRecipInfo, &heading.recipInfoQHead);

	/* Initialize the RecipientInfo structure */
	pRecipInfo->recipientType = UASH_RecipientType_BlindCopy;
	pRecipInfo->bReceiptNotificationRequested = FALSE;
	pRecipInfo->bNonReceiptNotificationRequested = FALSE;
	pRecipInfo->bMessageReturnRequested = FALSE;
	pRecipInfo->bNonDeliveryReportRequested = TRUE;
	pRecipInfo->bDeliveryReportRequested = FALSE;
	pRecipInfo->bReplyRequested = FALSE;

	/* Get the recipient address */
	addLocalAddr(pAddr, &pRecipInfo->recipient, (char *) (pRecipInfo + 1));
    }
    
    /* Get the reply-to recipients. */
    for (pAddr = pRfc822Env->reply_to; pAddr != NULL; pAddr = pAddr->next)
    {
	/* Allocate a Recipient structure */
	if ((pRecip =
	     OS_alloc(sizeof(UASH_Recipient) +
		      getPrintableRfc822AddressLength(pAddr))) == NULL)
	{
	    freeHeadingFields(&heading);
	    return ResourceError;
	}

	/* Initialize the queue pointers */
	QU_INIT(pRecip);

	/* Add this recipient to the heading */
	QU_INSERT(pRecip, &heading.replyToQHead);

	/* Get the recipient address */
	addLocalAddr(pAddr, pRecip, (char *) (pRecip + 1));
    }
    
    /* Get the replied-to message id */
    heading.pRepliedToMessageId = pRfc822Env->in_reply_to;
    
    /* Get the subject */
    heading.pSubject = pRfc822Env->subject;
    
    /* Get additional header fields not in the ENVELOPE */
    if ((rc = parseHeaderString(pHeaderStr, &heading)) != Success)
    {
	freeHeadingFields(&heading);
	return rc;
    }
    
    statistics.submissionAttempts ++; 

    /* Submit this message to the UA */
    if ((rc = UA_submitMessage(&heading, pRfc822Body, &hMessage, 
			       hShellReference)) != Success)
    {
	char buf[512];
	sprintf(buf, "Submission failed initially (locally), reason 0x%x!                                    ", 
		rc);
        uash_displayMessage(buf);
    }
    else
    {
	char buf[512];
	sprintf(buf, "Submission started (pending confirmation)!                                                                                       ");
        uash_displayMessage(buf);

        TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
		 "Submission started (pending confirmation)\n"
		 "\tTemporary Message Handle: 0x%lx\n",
	         (unsigned long) hMessage));
    }

    /* We're all done with the heading */
    freeHeadingFields(&heading);

    return rc;

} /* submitRfc822Message() */



static ReturnCode
addLocalAddr(ADDRESS * pAddr,
	     UASH_Recipient * pRecip,
	     char * pBuf)
{
    char *	    p;
    char *	    pEnd;
    char	    tmpbuf[MAILTMPLEN];
    char	    saveCh;

    /* If no buffer was provided, use our temporary one for now */
    if (pBuf == NULL)
    {
	pBuf = tmpbuf;
    }

    /* Null-terminate the buffer at the beginning, as address is cat'ed */
    *pBuf = '\0';

    /* Convert the address to printable format */
    getPrintableRfc822Address(pBuf, pAddr);

    /* Skip the comment portion of the address */
    if ((p = strchr(pBuf, '<')) == NULL)
    {
	p = pBuf;
	pEnd = p + strlen(p);
	saveCh = '\0';
    }
    else
    {
	++p;

	if ((pEnd = strchr(p, '>')) == NULL)
	{
	    pEnd = p + strlen(p);
	}

	/* Save character following string */
	saveCh = *pEnd;

	/* Temporarily null-terminate string */
	*pEnd = '\0';
    }

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL, 
	     "Checking to see if %s is an EMSD Addres! \n",p));

    if (strlen(p) > 4 && ((strncmp(p, "EMSD=", 4) == 0 && isdigit(p[4])) ||
        		  (strncmp(p, "\"EMSD=", 5) == 0 && isdigit(p[5]))))
    {
	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY, 
		 "EMSD Address (Ascii format): %s", p));

        p += ((*p == '"') ? 5 : 4);
	pRecip->un.recipEmsd.emsdAddr = 0;

	/* Determine the numeric value for this EMSD address */
	for (; *p ;p++)
	{
	    if (*p == '.')
	    {
		continue;
	    }
	    if (isdigit(*p))
	    {
	        pRecip->un.recipEmsd.emsdAddr = pRecip->un.recipEmsd.emsdAddr * 10 
					      + (*p - '0');
	    }
	}
   
	if (pRecip->un.recipEmsd.emsdAddr <= 0)
	{
	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY, 
		    "Illegal EMSD Address!\n"));
	}

	/* Mark it as an EMSD LOCAL address */
	pRecip->style = UASH_RecipientStyle_Emsd;

	/* There is no name */
	pRecip->un.recipEmsd.pName = NULL;

	/* Restore modified terminator */
	*pEnd = saveCh;

	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY, 
		 "EMSD Address (Integer format): %ld", 
		 pRecip->un.recipEmsd.emsdAddr));
    }
    else
    {
	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
		 "RFC 822 Address: %s", p));

	/* It's not an EMSD Local-format address. */
	pRecip->style = UASH_RecipientStyle_Rfc822;

	/* Restore modified terminator */
	*pEnd = saveCh;

	/* If we weren't provided with a buffer... */
	if (pBuf == tmpbuf)
	{
	    /* Allocate space for the RFC-822 address */
	    if ((pBuf = OS_allocStringCopy(tmpbuf)) == NULL)
	    {
		return ResourceError;
	    }
	}

	/* Point to the RFC-822 address */
	pRecip->un.pRecipRfc822 = pBuf;

        TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL, 
	         "It's an Rfc822 address: %s\n",pBuf));
    }

    return Success;

} /* addLocalAddr() */



static int
getPrintableRfc822AddressLength(ADDRESS * pAddr)
{
    char	    tmpbuf[MAILTMPLEN];

#ifndef MSDOS
    if (pAddr == NULL)
    {
	EH_problem("getPrintableRfc822AddressLength: "
		   "Null pointer passed to function as address");
	return 0;
    }
#endif

    getPrintableRfc822Address(tmpbuf, pAddr);
    return strlen(tmpbuf);
}


static void
getPrintableRfc822Address(char * pBuf,
			  ADDRESS * pAddr)
{
    const char *rspecials =  "()<>@,;:\\\"[].";
				/* full RFC-822 specials */
#ifndef MSDOS
    *pBuf = '\0';

    if (pAddr == NULL)
    {
	EH_problem("getPrintableRfc822Address: "
		   "Null pointer passed to function as address");
	return;
    }
#endif

    if (pAddr->mailbox != NULL && pAddr->host == NULL)
    {
	/* yes, write group name */
	rfc822_cat(pBuf, pAddr->mailbox, rspecials);
	strcat(pBuf, ": ");	/* write group identifier */
    }
    else
    {
	if (pAddr->host == NULL)
	{
	    /* end of group */
	    strcat(pBuf,";");
	}
	else if (pAddr->personal == NULL && pAddr->adl == NULL)
	{
	    /* simple case */
	    rfc822_address(pBuf, pAddr);
	}
	else
	{
	    /* must use phrase <route-addr> form */
	    if (pAddr->personal != NULL)
	    {
		rfc822_cat(pBuf, pAddr->personal, rspecials);
	    }

	    strcat(pBuf, " <");	/* write address delimiter */
	    rfc822_address(pBuf, pAddr); /* write address */
	    strcat(pBuf, ">");	/* closing delimiter */
	}
    }
}



static ReturnCode
writeRfc822Message(char * pDataFileName,
		   ENVELOPE * pRfcEnv,
		   char * pExtraHeaders,
		   char * pBodyText)
{
    char *	    p1;
    char *	    p2;
    FILE *	    hFolder;
    OS_Uint32	    currentTime;
    ReturnCode	    rc;
#if defined(OS_TYPE_MSDOS)
    OS_ZuluDateTime dateTime;
    long	    headerPos;
    long	    messagePos;
    long 	    messageSize;
    char	    headerBuf[64];
    static char *   months[] =
    {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

#define	OPEN_MODE	"rt+"
#define	WRITE_MODE	"wt+"
#else
#define	OPEN_MODE	"r+"
#define	WRITE_MODE	"w+"
#endif
    
    /*
     * Open the file.  Don't use APPEND mode because it prevents
     * seeking back to the header and modifying it, in Dawz mode.
     */
    if ((hFolder = OS_fileOpen(pDataFileName, OPEN_MODE)) == NULL)
    {
	if ((hFolder = OS_fileOpen(pDataFileName, WRITE_MODE)) == NULL)
	{
	    return OS_RC_FileOpen;
	}
    }
    
    /* See to the end of the file */
    fseek(hFolder, 0L, SEEK_END);

    /*
     * Compose the Message Header
     */
#if defined(OS_TYPE_MSDOS)
    if (OS_strcasecmp(config.pPineMessageFormat, "Berkeley") == 0)
#endif
    {
	/*
	 * Message Header format in Berkeley message file is:
	 *
	 *	From <return-path> <date>
	 */
	strcpy(uash_globals.tmpbuf,"From ");

	if (pRfcEnv->return_path != NULL)
	{
	    rfc822_address(uash_globals.tmpbuf, pRfcEnv->return_path);
	}
	else if (pRfcEnv->sender != NULL)
	{
	    rfc822_address(uash_globals.tmpbuf, pRfcEnv->sender);
	}
	else
	{
	    rfc822_address(uash_globals.tmpbuf, pRfcEnv->from);
	}

	/* Add the from line and the time to the message */
	if ((rc = OS_currentDateTime(NULL, &currentTime)) != Success)
	{
	    OS_fileClose(hFolder);
	    return rc;
	}
	strcat(uash_globals.tmpbuf, " ");
	strcat(uash_globals.tmpbuf, OS_printableDateTime(&currentTime));

	/* Get rid of the trailing '\n'; we'll put in "\n" later */
	uash_globals.tmpbuf[strlen(uash_globals.tmpbuf) - 1] = '\0';

	/* Output the Message Header line */
	OS_filePutString(uash_globals.tmpbuf, hFolder);
	OS_filePutString("\n", hFolder);
    }
#if defined(OS_TYPE_MSDOS)
    else if (OS_strcasecmp(config.pPineMessageFormat, "Dawz") == 0)
    {
	/*
	 * Message Header format in Berkeley message file is:
	 *
	 *	<date>,<messageSize>;<flags>
	 */

	/* Keep track of location as we'll need to put in messageSize later */
	headerPos = ftell(hFolder);

	/* Get the current date and time for the header */
	if ((rc = OS_currentDateTime(&dateTime, NULL)) != Success)
	{
	    OS_fileClose(hFolder);
	    return rc;
	}

	sprintf(headerBuf,
		"%2d-%s-%d %02d:%02d:%02d +0000,0000000000;000000000000\n",
		dateTime.day,
		months[dateTime.month - 1],
		dateTime.year,
		dateTime.hour,
		dateTime.minute,
		dateTime.second);
	
	/* Output the Message Header line */
	OS_filePutString(headerBuf, hFolder);

	/* Keep track of the file position prior to adding message */
	messagePos = ftell(hFolder);
    }
    else
    {
	return UnsupportedOption;
    }
#endif /* MSDOS */

    /* build remainder of the RFC-822 header */
    p1 = uash_globals.tmpbuf;
    *p1 = '\0';
    
    writeRfc822Header(hFolder, pRfcEnv,
		      "Date", pRfcEnv->date, rfc822_header_line);
    writeRfc822Header(hFolder, pRfcEnv,
		      "From", pRfcEnv->from, rfc822_address_line);
    writeRfc822Header(hFolder, pRfcEnv,
		      "Sender", pRfcEnv->sender, rfc822_address_line);
    writeRfc822Header(hFolder, pRfcEnv,
		      "Reply-To", pRfcEnv->reply_to, rfc822_address_line);
    writeRfc822Header(hFolder, pRfcEnv,
		      "Subject", pRfcEnv->subject, rfc822_header_line);
    writeRfc822Header(hFolder, pRfcEnv,
		      "To", pRfcEnv->to, rfc822_address_line);
    writeRfc822Header(hFolder, pRfcEnv,
		      "Cc", pRfcEnv->cc, rfc822_address_line);
    writeRfc822Header(hFolder, pRfcEnv,
		      "In-Reply-To", pRfcEnv->in_reply_to, rfc822_header_line);
    writeRfc822Header(hFolder, pRfcEnv,
		      "Message-ID", pRfcEnv->message_id, rfc822_header_line);
    
    /* write the extra headers */
    OS_filePutString(pExtraHeaders, hFolder);
    
    /* put in the header-terminating "\n" */
    OS_filePutString("\n", hFolder);
    
    /* Strip carriage returns from the body */
    for (p1 = p2 = pBodyText; *p2 != '\0'; )
    {
	if (*p2 != '\r')
	{
	    *p1++ = *p2++;
	}
	else
	{
	    ++p2;
	}
    }
    
    /* Null-terminate the CR-less string */
    *p1 = '\0';
    
    /* write the body */
    OS_filePutString(pBodyText, hFolder);
    
    /* write two trailing new lines to make it proper folder format */
    OS_filePutString("\n\n", hFolder);
    
#if defined(OS_TYPE_MSDOS)
    /* If we're writing DAWZ format, put in the file message size */
    if (OS_strcasecmp(config.pPineMessageFormat, "Dawz") == 0)
    {
	/* Determine the size of the message we added */
	messageSize = ftell(hFolder) - messagePos;

	/* Seek back to the Message Header */
	if (fseek(hFolder, headerPos, SEEK_SET) != 0)
	{
	    /* We're hosed.  Can't fix the message header. */
	    OS_fileClose(hFolder);
	    return Fail;
	}

	/* Point to the location, in the header, for the message size */
	p1 = strchr(headerBuf, ',') + 1;

	/* Write the message size into the header */
	sprintf(p1, "%010ld", messageSize);
	OS_filePutString(headerBuf, hFolder);
    }
#endif

    /* all done! */
    OS_fileClose(hFolder);
    
    return Success;
}


static void
writeRfc822Header(void * hFolder,
		  ENVELOPE * pRfcEnv,
		  char * pFieldName,
		  void * pFieldData,
		  void (*pfAddRfcHeaderOrAddress)())
{
    char 	    buf[1024];
    char *	    p1;
    char *	    p2;

    /* rfc822_header_line() concatenates to existing string.  Null terminate */
    *(p1 = buf) = '\0';
    
    /* Add this header line */
    (*pfAddRfcHeaderOrAddress)(&p1, pFieldName, pRfcEnv, pFieldData);
    
    /* Strip carriage returns from the header */
    for (p1 = p2 = buf; *p2 != '\0'; )
    {
	if (*p2 != '\r')
	{
	    *p1++ = *p2++;
	}
	else
	{
	    ++p2;
	}
    }
    
    /* Null-terminate the CR-less string */
    *p1 = '\0';
    
    /* write the header */
    OS_filePutString(buf, hFolder);
}


static void
freeHeadingFields(UASH_MessageHeading * pHeading)
{
    /* If the sender has an RFC-822 address, free it. */
    if (pHeading->sender.style == UASH_RecipientStyle_Rfc822 &&
	pHeading->sender.un.pRecipRfc822 != NULL)
    {
	OS_free(pHeading->sender.un.pRecipRfc822);
	pHeading->sender.un.pRecipRfc822 = NULL;
    }

    /* If the originator has an RFC-822 address, free it. */
    if (pHeading->originator.style == UASH_RecipientStyle_Rfc822 &&
	pHeading->originator.un.pRecipRfc822 != NULL)
    {
	OS_free(pHeading->originator.un.pRecipRfc822);
	pHeading->originator.un.pRecipRfc822 = NULL;
    }

    /* Free the elements in the recipient info queue */
    QU_FREE(&pHeading->recipInfoQHead);

    /* Free the elements in the reply-to queue */
    QU_FREE(&pHeading->replyToQHead);

    /* Free the elements in the extension queue */
    QU_FREE(&pHeading->extensionQHead);
}


static ReturnCode
parseHeaderString(char *pHeaderStr,
		  UASH_MessageHeading * pHeading)
{
    OS_Boolean 		foundIt;
    char * 		pEnd;
    char * 		p1;
    char * 		p2;
    char 		saveCh = '\0';
    UASH_Extension  *	pExtension;

    static struct IgnoreHeaderList
    {
	char           *pSearchString;
	int             length;
    } ignoreHeaderList[] =
    {
	{
	    ">From ",
	    sizeof(">From") - 1,
	},
	{
	    "BCC:",
	    sizeof("BCC:") - 1,
	},
	{
	    "CC:",
	    sizeof("CC:") - 1,
	},
	{
	    "Date:",
	    sizeof("Date:") - 1,
	},
	{
	    "From ",
	    sizeof("From ") - 1,
	},
	{
	    "From:",
	    sizeof("From:") - 1,
	},
	{
	    "In-Reply-To:",
	    sizeof("In-Reply-To:") - 1,
	},
	{
	    "Message-ID:",
	    sizeof("Message-ID:") - 1,
	},
	{
	    "Reply-To:",
	    sizeof("Reply-To:") - 1,
	},
	{
	    "Sender:",
	    sizeof("Sender:") - 1,
	},
	{
	    "Subject:",
	    sizeof("Subject:") - 1,
	},
	{
	    "To:",
	    sizeof("To:") - 1,
	},
	{
	    NULL
	}
    };
    struct IgnoreHeaderList *	pIgnoreHeaderList;

    static struct MimeHeaders
    {
	char * 			pSearchString;
	int 			length;
	char **			ppData;
    } mimeHeaders[] =
    {
	{
	    "MIME-Version:",
	    sizeof("MIME-Version:") - 1
	},
	{
	    "Content-Type:",
	    sizeof("Content-Type:") - 1
	},
	{
	    "Content-Id:",
	    sizeof("Content-Id:") - 1
	},
	{
	    "Content-Description:",
	    sizeof("Content-Description:") - 1
	},
	{
	    "Content-Transfer-Encoding:",
	    sizeof("Content-Transfer-Encoding:") - 1
	},
	{
	    NULL
	}
    };
    struct MimeHeaders *   pMimeHeaders;

    /* Strip carriage returns from the header string */
    for (p1 = p2 = pHeaderStr; *p2 != '\0'; )
    {
	if (*p2 != '\r')
	{
	    *p1++ = *p2++;
	}
	else
	{
	    ++p2;
	}
    }

    /* Initialize the AsciiString pointers in the Mime Headers */
    mimeHeaders[0].ppData = &pHeading->pMimeVersion;
    mimeHeaders[1].ppData = &pHeading->pMimeContentType;
    mimeHeaders[2].ppData = &pHeading->pMimeContentId;
    mimeHeaders[3].ppData = &pHeading->pMimeContentDescription;
    mimeHeaders[4].ppData = &pHeading->pMimeContentTransferEncoding;

    /* Null-terminate the CR-less string */
    *p1 = '\0';

    /* For each header item... */
    while (*pHeaderStr != '\0')
    {
	/* ... find the end of this item */
	if ((pEnd = strchr(pHeaderStr, '\n')) == NULL)
	{
	    pEnd = pHeaderStr + strlen(pHeaderStr);
	}

	/* If there's a space or tab following a new-line... */
	while (pEnd[0] == '\n' &&
	       (pEnd[1] == ' ' || pEnd[1] == '\t'))
	{
	    /* ... it's a continuation of the preceeding line */
	    ++pEnd;
	    if ((pEnd = strchr(pEnd, '\n')) == NULL)
	    {
		pEnd += strlen(pEnd);
	    }
	}
	
	/* Save character at end, 'cause we'll compare with '\0' later. */
	saveCh = *pEnd;

	/* See if we can find a match for this header item */
	foundIt = FALSE;
	for (pIgnoreHeaderList = ignoreHeaderList;
	     pIgnoreHeaderList->pSearchString != NULL;
	     pIgnoreHeaderList++)
	{
	    if (OS_strncasecmp(pIgnoreHeaderList->pSearchString,
			       pHeaderStr,
			       pIgnoreHeaderList->length) == 0)
	    {
		foundIt = TRUE;
		break;
	    }
	}
	
	/*
	 * If it wasn't a recognized header item in the c-client
	 * ENVELOPE...
	 */
	if (!foundIt)
	{
	    /*
	     * ... was it one of the MIME headers, which isn't in
	     * the c-client ENVELOPE?
	     */
	    foundIt = FALSE;
	    for (pMimeHeaders = mimeHeaders;
		 pMimeHeaders->pSearchString != NULL;
		 pMimeHeaders++)
	    {
		if (OS_strncasecmp(pMimeHeaders->pSearchString,
				   pHeaderStr,
				   pMimeHeaders->length) == 0)
		{
		    foundIt = TRUE;
		    break;
		}
	    }

	    if (foundIt)
	    {
		/* Skip past the header name */
		pHeaderStr += pMimeHeaders->length;
		
		/* Skip past white space */
		for (;
		     *pHeaderStr != '\0' && *pHeaderStr != '\n';
		     pHeaderStr++)
		{
		    if (*pHeaderStr != ' ' && *pHeaderStr != '\t')
		    {
			break;
		    }
		}
		
		/* Save the character at the end of this header */
		saveCh = *pEnd;

		/* Null-terminate this header field string */
		*pEnd = '\0';

		/* Point the Header to this string */
		*pMimeHeaders->ppData = pHeaderStr;
		
	    }
	    else
	    {
		/*
		 * It's unrecognized.  Put it in the list of
		 * extensions.
		 */

		/* Allocate an Extension structure */
		if ((pExtension = OS_alloc(sizeof(UASH_Extension))) == NULL)
		{
		    return ResourceError;
		}
		
		/* Initialize the queue pointers */
		QU_INIT(pExtension);

		/* Add this extension to the extension queue */
		QU_INSERT(pExtension, &pHeading->extensionQHead);

		/* Determine length of the label */
		if ((p1 = strchr(pHeaderStr, ':')) == NULL)
		{
		    /* should never occur */
		    return Fail;
		}
		
		/* Make sure label ended in current header item */
		if (p1 > pEnd)
		{
		    /* should never occur */
		    return Fail;
		}

		/* Null terminate the label */
		*p1 = '\0';
		
		/* Save this extension label */
		pExtension->pLabel = pHeaderStr;

		/* Scan past white space in the value */
		for (++p1; *p1 != '\0' && p1 < pEnd; p1++)
		{
		    if (*p1 != ' ' && *p1 != '\t')
		    {
			break;
		    }
		}
		
		saveCh = *pEnd;
		*pEnd = '\0';
		
		/* Save the extension value */
		pExtension->pValue = p1;
	    }
	}
	
	/* Point to the next header item, if there is one */
	pHeaderStr = pEnd;
	if (saveCh != '\0')
	{
	    ++pHeaderStr;
	}
    }
    
    return Success;
}



static OS_Boolean
dirFilter(const char * pName)
{
    /* Include only files which begin with "C_" */
    if ((pName[0] == 'S' || pName[0] == 's') && (pName[1] == '_'))
    {
	/* Include this file. */
	return 1;
    }

    return 0;
}


static void
terminate(int sigNum)
{
    bTerminateRequested = TRUE;

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	     "ua: Signal received\n"));
}


#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
int
main(int argc, char * argv[])
{
    UASH_init(argc, argv);

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    sprintf(uash_globals.tmpbuf,
            "Press 'q' to exit ('p' to get messages from Message Center)!");
    uash_displayMessage(uash_globals.tmpbuf);
#endif

    /* Main Loop */
    while (SCH_block() >= 0 && ! bTerminateRequested)
    {
	SCH_run();
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
	/* Allow CTRL-C actions to take place */
	kbhit();
#endif
    }
      
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
	sprintf(uash_globals.tmpbuf,
		"%s: Unexpected Scheduler error!",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }


#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    if (! bNoMemDebug)
    {
	printf("EXITING:\n");
	OS_allocPrintOutstanding();
    }

    sprintf(uash_globals.tmpbuf,
    	    "%5ld Message%ssent!\n%5ld Message%sreceived!", 
	    statistics.successfulSubmissions, 
	    (statistics.successfulSubmissions > 1) ? "s " : " ",
	    statistics.successfulDeliveries, 
	    (statistics.successfulDeliveries  > 1) ? "s " : " ");
    uash_displayMessage(uash_globals.tmpbuf);

#endif
    
    exit(0);
}


void
uash_displayMessage(char * pBuf)
{
    puts(pBuf);
}


#else	/* it's Windows */

# include "uashmenu.h"

long FAR PASCAL _export WndProc (HWND, UINT, UINT, LONG);

HWND        		hwnd;
static QU_Head		logMessageQHead = QU_INITIALIZE(logMessageQHead);
static short		messageCount = 0;
static short		maxMessages;
static short		cyClient;
static short		cyChar;

typedef struct
{
    QU_ELEMENT;
    char	message[1];
} LogMessage;


int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdParam, int nCmdShow)
{
    static char	    szAppName[] = "EMSD UA for Pine";
    MSG 	    msg;
    WNDCLASS 	    wndclass;
    int 	    i;
    char *	    p;

    if (!hPrevInstance)
    {
        wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon (0, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (0, IDC_ARROW);
	wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
/*	wndclass.hbrBackground = COLOR_BACKGROUND + 1; */
	wndclass.lpszMenuName  = "uapinew";
	wndclass.lpszClassName = szAppName;
	
	RegisterClass (&wndclass);
    }

    hwnd = CreateWindow (szAppName,         // window class name
			 "EMSD User Agent for Pine",// window caption
			 WS_OVERLAPPEDWINDOW,    // window style
/*			 WS_OVERLAPPEDWINDOW |    // window style
			 WS_VSCROLL | WS_HSCROLL,     // window style
*/
			 CW_USEDEFAULT,           // initial x position
			 CW_USEDEFAULT,           // initial y position
			 CW_USEDEFAULT,           // initial x size
			 CW_USEDEFAULT,           // initial y size
			 0,                       // parent window handle
			 0,                       // window menu handle
			 hInstance,               // program instance handle
			 NULL);			  // creation parameters

    ShowWindow (hwnd, nCmdShow);
/*
    SetScrollRange (hwnd, SB_VERT, 1, 1000, TRUE);
    SetScrollRange (hwnd, SB_HORZ, 1, 150, TRUE);
    ShowScrollBar (hwnd, SB_BOTH, TRUE);
*/
    UpdateWindow (hwnd);

    /* Arrange for EH_ messages to pop-up a message box */
    EH_displayMessages(displayError);

    /* See if there's a configuration file name specified */
    if (lpszCmdParam != NULL)
    {
	/* Scan through white space at the end of the string */
	for (p = lpszCmdParam + strlen(lpszCmdParam) - 1; isspace(*p); p--)
	    ;

	/* Null terminate in front of superfluous white space */
	*++p = '\0';
    }

    /* Initialize everything */
    UASH_init(1, &lpszCmdParam);

    while (GetMessage (&msg, 0, 0, 0))
    {
        TranslateMessage (&msg);
	DispatchMessage (&msg);

	/* Main Loop */
	if (! bTerminateRequested)
	{
	    if (SCH_block() >= 0)
	    {
	        SCH_run();
	    }
	}
    }

    /* Shutdown all sockets */
    while (UDP_terminate() != SUCCESS)
    {
    }

    if (! bTerminateRequested)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Unexpected Scheduler error!",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    return msg.wParam;
}

long FAR PASCAL _export WndProc (HWND hwnd, UINT message, UINT wParam,
				 LONG lParam)
{
    LogMessage *    pLogMessage;
    HDC 	    hdc;
    HMENU 	    hMenu;
    PAINTSTRUCT     ps;
    RECT 	    rect;
    TEXTMETRIC 	    tm;
    short 	    i;
    short 	    x;
    short 	    y;
    static short    cxChar;
    static short    cxCaps;
    static short    cxClient;
    static short    nMaxWidth;
    static int	    closeCount = 1;
    
    switch (message)
    {
    case WM_CREATE:
        hdc = GetDC (hwnd);

	GetTextMetrics (hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
	cyChar = tm.tmHeight + tm.tmExternalLeading;

	ReleaseDC (hwnd, hdc);

	nMaxWidth = 40 * cxChar + 22 * cxCaps;
	return 0;

    case WM_SIZE:
	cxClient = LOWORD (lParam);
	cyClient = HIWORD (lParam);
	maxMessages = cyClient / cyChar;

	/* If size was reduced, eliminate messages */
	for (; messageCount > maxMessages; messageCount--)
	{
	    pLogMessage = QU_FIRST(&logMessageQHead);
	    QU_REMOVE(pLogMessage);
	    OS_free(pLogMessage);
	}
	return 0;

    case WM_PAINT:
	hdc = BeginPaint (hwnd, &ps);
/*	SendMessage(hwnd, WM_ERASEBKGND, 0, 0L); */

	/* Display the messages */
	for (pLogMessage = QU_FIRST(&logMessageQHead), i = 0;
	     ! QU_EQUAL(pLogMessage, &logMessageQHead);
	     pLogMessage = QU_NEXT(pLogMessage), i++)
	{
	    x = cxChar;
	    y = cyChar * i;

/*  	    SetTextAlign(hdc, TA_UPDATECP); */
	    TextOut (hdc, x, y,
		     pLogMessage->message,
		     lstrlen (pLogMessage->message));
	}

	EndPaint (hwnd, &ps);
	return 0;

    case WM_COMMAND:
	hMenu = GetMenu(hwnd);

	switch (wParam)
	{
	case IDM_EXIT:
/*	    if (MessageBox(hwnd, "Are you sure you want to exit?",
		          "EMSD UA", MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
*/
	    SendMessage(hwnd, WM_CLOSE, 0, 0L);
	    return 0;

	case IDM_HELP:
	    MessageBox(hwnd, "Help not yet implemented.",
		       "EMSD UA",
		       MB_ICONEXCLAMATION | MB_OK);
	    return 0;

	case IDM_ABOUT:
	    MessageBox(hwnd, RELID_getCopyrightNedaAtt(),
		       "EMSD UA",
		       MB_ICONEXCLAMATION | MB_OK);
	    return 0;

	case IDM_MTSADDRESS:
	    {
		char buf[128];
	    strcpy(buf, "Message Center's IP Address: ");
	    strcat(buf, config.pMtsNsap);
	    MessageBox(hwnd, buf,
		       "EMSD UA",
		       MB_ICONEXCLAMATION | MB_OK);
	    return 0;
	    }

	case IDM_POLLINTERVAL:
	    {
		char buf[128];
	    strcpy(buf, "Message submission attempt interval: ");
	    strcat(buf, config.pMessageSubmissionAttemptInterval);
	    strcat(buf, " Seconds");
	    MessageBox(hwnd, buf,
		       "EMSD UA",
		       MB_ICONEXCLAMATION | MB_OK);
	    return 0;
	    }

	case IDM_DELIVERYCTRL:
	    MessageBox(hwnd, "Delivery Control send",
		       "EMSD UA",
		       MB_ICONEXCLAMATION | MB_OK);
	    return 0;
	}

	return 0;

    case WM_CLOSE:
	bTerminateRequested = TRUE;

	/* Close all sockets */
	/* if (UDP_terminate() != SUCCESS)
	{
	     *
	     * Socket close is pending, but not yet complete.  We're
	     * not quite ready to terminate.  Arrange to get back here
	     * again.
	     *
	    PostMessage(hwnd, WM_CLOSE, 0, 0L); 
	    return 0; 
	} */
	while (UDP_terminate() != SUCCESS)
		;

	/* Socket close successful.  Go do default WM_CLOSE processing. */

	SendMessage(hwnd, WM_DESTROY, 0, 0L); 

	return 0L;

    case WM_DESTROY:
	PostQuitMessage (0);
	/*DestroyWindow(hwnd);*/
	return 0L;
    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}



void
uash_displayMessage(char * pBuf)
{
    LogMessage *	pLogMessage;

    /* If there are no messages remaining, free the first one. */
    if (messageCount == maxMessages)
    {
	pLogMessage = QU_FIRST(&logMessageQHead);
	QU_REMOVE(pLogMessage);
	OS_free(pLogMessage);
	--messageCount;
    }

    /* Allocate a new log message structure */
    if ((pLogMessage = OS_alloc(sizeof(LogMessage) + strlen(pBuf))) == NULL)
    {
	/* nothing to do if we're out of memory */
	return;
    }

    /* We've used one of the messages */
    ++messageCount;

    /* Initialize the queue pointers */
    QU_INIT(pLogMessage);

    /* Copy the message string */
    strcpy(pLogMessage->message, pBuf);

    /* Insert it into the log message queue */
    QU_INSERT(pLogMessage, &logMessageQHead);

    /* Update the screen */
    InvalidateRect(hwnd, NULL, FALSE);
    UpdateWindow(hwnd);
}


static void
displayError(char * pMessageType,
	     char * pFileName,
	     int lineNum,
	     char * pMessage)
{
    char	    buf[1024];

    sprintf(buf, "%s: %s(%d): %s",
	    pMessageType, pFileName, lineNum, pMessage);
    MessageBox(hwnd, buf,
	       "EMSD UA",
	       MB_OK | MB_ICONINFORMATION);
}


int __cdecl printf(const char * pFormat, ...)
{
    return 0;
}

void __cdecl perror(const char * pMsg)
{
    return;
}

#endif /* Windows */

#if ! defined(OS_VARIANT_QuickWin) && ! defined(OS_VARIANT_Windows)
static Void
keyboardInput(Ptr arg)
{
    int		c;
    ReturnCode	rc;
    void *	hMessage;
    void *	hShellReference = "";

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

#ifdef DELIVERY_CONTROL
    case 'p':			/* Issue Delivery Control */
    case 'P':			/* Issue Delivery Control */
        if ((rc = UA_issueDeliveryControl(&hMessage,
    			              hShellReference)) != Success)
        {
	    sprintf(uash_globals.tmpbuf, 
	            "Can not contact MTS because of a local problem!");
    	    uash_displayMessage(uash_globals.tmpbuf);

    	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
	              "Can not contact MTS because of a local problem!"));
        }
        else
        {
	    sprintf(uash_globals.tmpbuf, 
	    	    "Delivery request sent (pending confirmation)!");
    	    uash_displayMessage(uash_globals.tmpbuf);

    	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
    		     "Delivery Control Operation started "
    		     "(pending confirmation)\n"));
        }
	break;
#endif
    
    default:
/*	(* uash_globals.pfInput)(c); */
	break;
    }

    /* If anything is typed on the keyboard, we want to know about it. */
    SCH_submit(keyboardInput, NULL, KEYBOARD_EVENT   DEBUG_STRING);

}
#endif

