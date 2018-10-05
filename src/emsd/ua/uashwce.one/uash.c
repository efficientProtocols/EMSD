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
 * History: copy-and-edit based on uashpine.one/uash.c
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

#if defined(OS_VARIANT_WinCE)
#include "resrc1.h"
#include "resource.h"
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

#if defined(OS_VARIANT_WinCE)

/* the following two booleans are part of a hack required to provide
 * notification on MTS's sucessful receipt of a delivery control
 * request.  This is done by the MTS issues an err [sic].  However, on
 * our end uash_deliveryControlError() never gets called because the
 * err that the MTS returns in reply to the delivery control req fails
 * in preprocessing (dies in ASN_parse() of the PDU).  So we use the
 * following pair of booleans to control */
OS_Boolean	g_delCtrlWaitingMtsAck;
OS_Boolean	g_delCtrlMtsAckRecvd;

#endif /* OS_VARIANT_WinCE */

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
    char *	    pLockFile;
    char *	    pGrabLockMaxRetries;
    char *	    pGrabLockRetryInterval;
#if defined(OS_TYPE_MSDOS) && !defined(OS_VARIANT_WinCE)
    char *	    pPineMessageFormat;
#endif
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    char *	    pMaxInactiveIntervals;
#endif
    char *	    pMessageSubmissionAttemptInterval;
    char *	    pFetchMessagesOnStartup;
    char *	    pVisibleHeartbeat;
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
      {
	  "Features",
	  "Visible Heartbeat",
    	  &config.pVisibleHeartbeat
      },
#if defined(OS_TYPE_MSDOS) && !defined(OS_VARIANT_WinCE)
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
	  "Internals",
	  "Lock File",
	  &config.pLockFile
      },
      {
	  "Internals",
	  "Grab Lock Max Retries",
	  &config.pGrabLockMaxRetries
      },
      {
	  "Internals",
	  "Grab Lock Retry Interval",
	  &config.pGrabLockRetryInterval
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

static char	szAppName[80];
static WCHAR	wszAppName[80];
static WCHAR	wszAppTitle[80];

#if defined(OS_VARIANT_WinCE) || defined(OS_VARIANT_Windows)
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

static Char 	*copyrightNotice;
static WCHAR	*wCopyrightNotice;

EXTERN Char *RELID_getCopyrightNeda(Void);
EXTERN Char *RELID_getCopyrightAtt(Void);
EXTERN Char *RELID_getCopyrightNedaAtt(Void);

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

#if defined(OS_VARIANT_WinCE)
    __applicationName = szAppName;
#elif ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
    __applicationName = argv[0];
    setbuf(stdout, NULL);
#endif

    if ( ! (copyrightNotice = RELID_getCopyrightNedaAtt()) ) {
        uash_displayMessage(__applicationName);
	EH_fatal("main: Get copyright notice failed!\n");
    }

#if defined(OS_VARIANT_WinCE)
    /* also make a UNICODE version of the notice */
    wCopyrightNotice = OS_alloc(sizeof(WCHAR)*strlen(copyrightNotice));
    if ( wCopyrightNotice == NULL ) {
	EH_fatal("out of memory.");
    }
    wsprintf(wCopyrightNotice, TEXT("%S"), copyrightNotice);
    uash_displayMessage(copyrightNotice);
#elif defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Windows)
    {
	/* dislay one line at a time */
	char *s = copyrightNotice;
	char buf[128], *p;
	for (;*s;) {
	    for (p = buf; *s != '\n';)
	    	*p++ = *s++;
	    *p = '\0'; s++;
    	    uash_displayMessage(buf);
	}
    }
#else
    uash_displayMessage(copyrightNotice);
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_QuickWin)
    /* If we ever call exit(), prompt for whether to destroy windows */
    if (_wsetexit(_WINEXITPROMPT) != 0) {
    	EH_problem("wsetexit() failed");
    }
    if (_wabout(copyrightNotice) != 0) {
    	EH_problem("wabout() failed");
    }
#endif

#if defined(OS_VARIANT_WinCE)
    /* *** NOTYET */
#else
    /* Initialize the OS memory allocation debug routines */
    OS_allocDebugInit(NULL);

    /* Initialize the OS file tracking debug routines */
    OS_fileDebugInit("/tmp/uashpine.fil");
#endif


#if defined(OS_VARIANT_WinCE)
    /* *** NOTYET -- no getopt() */
#elif ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
    /* Get configuration file name */
    while ((c = getopt(argc, argv, "T:xc:lp:e:V")) != EOF)
    {
        switch (c)
	{
	case 'c':
	    strcpy(pConfigFile, optarg);
	    break;

	case 'V':		/* specify configuration directory */
	    if ( ! (copyrightNotice = RELID_getCopyrightNedaAtt()) ) {
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

    if (strlen(pConfigFile) == 0) {

#if defined(OS_VARIANT_WinCE)
#define EMSD_DEFAULT_UASH_INI "\\EMSD\\config\\ua.ini"
#else
#define EMSD_DEFAULT_UASH_INI "ua.ini"
#endif

	strcpy(pConfigFile, EMSD_DEFAULT_UASH_INI);
    }


    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering CONFIG_init()"));

    /* Initialize the config module */
    if ((rc = CONFIG_init()) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not initialize CONFIG module!",
		__applicationName);
	EH_fatal(uash_globals.tmpbuf);
    }

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering CONFIG_open()"));

    /* Open the configuration file.  Get a handle to the configuration data */
    if ((rc = CONFIG_open(pConfigFile, &hConfig)) != Success) {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not open configuration file (%s), "
		"reason 0x%04x!",
		__applicationName, pConfigFile, rc);
	EH_fatal(uash_globals.tmpbuf);
    }

#if defined(OS_VARIANT_WinCE)
    /* *** NOTYET */
#elif ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
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
		  "\t[%s]:%s = <%s>",
		  pConfigParam->pSectionName,
		  pConfigParam->pTypeName,
		  *pConfigParam->ppValue));
    }

#if defined(OS_VARIANT_WinCE)	/* *** NOTYET */
#else
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

    /* parse lock file parameters from config */
    wsprintf(uash_globals.szLockFile, TEXT("%S"), config.pLockFile);
    uash_globals.grabLockMaxRetries = atol(config.pGrabLockMaxRetries);
    uash_globals.grabLockRetryInterval = atol(config.pGrabLockRetryInterval);


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

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering NVM_init()"));

    /* Initialize the non-volatile memory module */
    if ((rc = NVM_init()) != Success) {
	sprintf(uash_globals.tmpbuf,
		"%s: "
		"Could not initialize NVM module, reason 0x%x!",
		__applicationName, rc);
	EH_fatal(uash_globals.tmpbuf);
    }

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering OS_fileSize()"));

    /* Is there an existing Non-volatile memory file? */
    if (OS_fileSize(config.pNonVolatileMemoryEmulation,
		    &nvFileSize) == Success)
    {

	TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering NVM_open() of exisiting file"));

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
	TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering NVM_open() of new file"));

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

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering ASN_init()"));


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

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering UDP_PO_init()"));

    /* Initialize the UDP_PO module */
    UDP_PO_init(pErrLog, pPduLog);

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering ESRO_CB_init()"));

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

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering UA_init()"));

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

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "entering OS_dirOpen()"));

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
			NULL, NULL, checkSubmit, &hTimer)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"%s: Could not start initial timer to process "
		"submissions from mail user.",
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
    	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY, "Issue delivery Control..."));

    	if ((rc = UA_issueDeliveryControl(&hMessage, hShellReference)) != Success)
    	{
	    sprintf(uash_globals.tmpbuf, "Can not contact Message Center "
		   			 "because of a local problem!");
    	    uash_displayMessage(uash_globals.tmpbuf);

    	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	             "Delivery Control failed locally."));
    	}
    	else
    	{
	    g_delCtrlWaitingMtsAck = TRUE;
	    g_delCtrlMtsAckRecvd = FALSE;

	    uash_displayMessage(" "); /* getting around displaying log messages under commandbar bug */
	    sprintf(uash_globals.tmpbuf, "Triggering message deliveries...");
    	    uash_displayMessage(uash_globals.tmpbuf);

#if defined(OS_VARIANT_WinCE)
	    sndPlaySound(TEXT("question"), SND_ASYNC | SND_ALIAS );
#endif
	    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
		     "Delivery Control Operation started (pending confirmation)."));
    	}
    }
#endif

#if CHECKSUBMIT_WATCHDOG
    uash_globals.csWatchdogInitiated = FALSE;
    uash_globals.csWatchdogLoopCount = 0;
    uash_globals.csWatchdogIOUCount = 0;
    uash_globals.csWatchdogSubmissionStarted = FALSE;
#endif

#if defined(OS_VARIANT_WinCE)
	if ( OS_strcasecmp(config.pVisibleHeartbeat, "TRUE") == 0 )
	    uash_globals.fVisibleHeartbeat = TRUE;
	else
	    uash_globals.fVisibleHeartbeat = FALSE;
#endif /* OS_VARIANT_WinCE */

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
    ReturnCode		rc;
    UASH_LocalUser	*pLocalUser;
    UASH_RecipientInfo	*pRecipInfo;
    OS_Boolean		bFoundOneEmsdUser           = FALSE;
    OS_Boolean		bFoundOneLocalEmsdUser      = FALSE;
    OS_Boolean		bMessageDeliveredToMailbox = FALSE;
    char		buf[512];
    HANDLE		hLockFile;
    int			retries 	= uash_globals.grabLockMaxRetries;
    int			retryInterval 	= uash_globals.grabLockRetryInterval;

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

	    /* grab the lock */
	    while ( INVALID_HANDLE_VALUE
		    == (hLockFile = CreateFile(uash_globals.szLockFile,
					       GENERIC_READ,
					       0,	/* no sharing */
					       NULL,
					       OPEN_ALWAYS,
					       FILE_ATTRIBUTE_NORMAL,
					       NULL)) ) {
		if ( retries-- > 0 ) {
		    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Could not lock shared resource--retrying..."));
		    /* MessageBeep(MB_ICONASTERISK); */
		    Sleep((DWORD)uash_globals.grabLockRetryInterval);
		}
		else {
		    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Could not lock shared resource--giving up!"));
		    return FALSE;
		}
	    }
	    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Lock acquired."));


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

	    /* release the lock */
	    CloseHandle(hLockFile);

	    /* Indicate that we found one */
	    bFoundOneLocalEmsdUser = TRUE;

	    /* Indicate that we delivered message to mailbox */
    	    bMessageDeliveredToMailbox = TRUE;

	    /* Display a message */
	    sprintf(buf,
	    	    "%06ld is a local user; delivery successful [Msg Id=%s]!",
		    pLocalUser->emsdAddr,
		    pRfcEnv->message_id);

	    uash_displayMessage(buf);

	    /* provide audible queue */
	    sndPlaySound(TEXT("startup"), SND_ASYNC | SND_ALIAS );

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
		    "%06ld is not a local user [Msg Id=%s].",
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

      sprintf(buf, "Inbound EMSD message has no local EMSD user(s) as its recipient(s)!");
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
    HANDLE		hLockFile;
    int			retries = uash_globals.grabLockMaxRetries;

#if CHECKSUBMIT_WATCHDOG

    /*
     * If this is a late TMR_start() scheduled checkSubmit
     * who was already covered for by a prior manual invocation,
     * then schedule another one, but do nothing this time around
     */
    if ( uash_globals.csWatchdogInitiated == FALSE ) {

	/*
	 * a TMR_start()-initiated call to checkSubmit...
	 */

	/* if this function is called while a submission is already in
	 * progress we always defer it since concurrent submissions are
	 * not allowed
	 */

	if ( uash_globals.csWatchdogSubmissionStarted ) {
	    if ((rc = TMR_start(TMR_SECONDS(messageSubmissionAttemptInterval),
				NULL, NULL, checkSubmit, NULL)) != Success) {
		sprintf(uash_globals.tmpbuf,
			"Could not start timer to check for submissions; "
			"rc = 0x%x!", rc);
		EH_fatal(uash_globals.tmpbuf);
	    }
	    return Success;
	}

	/* if still in catch up mode ... */
	if ( uash_globals.csWatchdogIOUCount > 0 ) { /* yup, still catching up */

	    /* catch up by one */
	    uash_globals.csWatchdogIOUCount--;

	    /* no more IOUs--finally caught up! */
	    if ( uash_globals.csWatchdogIOUCount == 0 )
		TM_TRACE((uash_globals.hModCB, TM_DEBUG, "caught up!!!!"));

	    /* schedule the timer */
	    if ((rc = TMR_start(TMR_SECONDS(1),
				NULL, NULL, checkSubmit, NULL)) != Success) {
		sprintf(uash_globals.tmpbuf,
			"Could not start timer to check for submissions; "
			"rc = 0x%x!", rc);
		EH_fatal(uash_globals.tmpbuf);
	    }
	    return Success;
	}

	/* this is a normal, TMR_start()-initiated invocation
	 * of checkSubmit(),
	 */
    }

    uash_globals.csWatchdogLoopCount = 0;

#endif /* CHECKSUBMIT_WATCHDOG */

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	      "Checking for newly-submitted messages from 'Inbox' application..."));

    /* grab the lock */
    while ( INVALID_HANDLE_VALUE
	    == (hLockFile = CreateFile(uash_globals.szLockFile,
				       GENERIC_READ,
				       0,	/* no sharing */
				       NULL,
				       OPEN_ALWAYS,
				       FILE_ATTRIBUTE_NORMAL,
				       NULL)) ) {
	if ( retries-- > 0 ) {
	    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Could not lock shared resource--retrying..."));
	    /* MessageBeep(MB_ICONASTERISK); */
	    Sleep((DWORD)uash_globals.grabLockRetryInterval);
	}
	else {
	    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Could not lock shared resource--giving up!"));
	    return FALSE;
	}
    }
    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Lock acquired."));

    /* See if there are messages to be handled. */
    if ( (rc = OS_dirFindNext(hFromPineSpoolDir, &pFileName)) != Success)
    {
	sprintf(uash_globals.tmpbuf,
		"Error searching pine spool directory; rc = 0x%x!", rc);
	/* release the lock */
	CloseHandle(hLockFile);
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
	    /* release the lock */
	    CloseHandle(hLockFile);
	    EH_fatal(uash_globals.tmpbuf);
	}

	if ((rc = OS_dirFindNext(hFromPineSpoolDir, &pFileName)) != Success)
	{
	    sprintf(uash_globals.tmpbuf,
		    "Error searching pine spool directory; rc = 0x%x!", rc);
	    /* release the lock */
	    CloseHandle(hLockFile);
	    EH_fatal(uash_globals.tmpbuf);
	}
    }

    /* release the lock */
    CloseHandle(hLockFile);

    /* Was there a message waiting for us? */
    if (pFileName == NULL)
    {
#if CHECKSUBMIT_WATCHDOG
	if ( uash_globals.csWatchdogInitiated == TRUE ) {
	    TM_TRACE((uash_globals.hModCB, TM_DEBUG,
		      "checkSubmit() watchdog initiated, skipping TMR_start() call."));
	}
	else {
	    /* Nope. Create a timer to call this function again (later) */
	    if ((rc = TMR_start(TMR_SECONDS(messageSubmissionAttemptInterval),
				NULL, NULL, checkSubmit, NULL)) != Success)
		{
		    sprintf(uash_globals.tmpbuf,
			    "Could not start timer to check for submissions; "
			    "rc = 0x%x!", rc);
		    EH_fatal(uash_globals.tmpbuf);
		}
	}
#else  /* CHECKSUBMIT_WATCHDOG */

	/* Nope. Create a timer to call this function again (later) */
	if ((rc = TMR_start(TMR_SECONDS(messageSubmissionAttemptInterval),
			    NULL, NULL, checkSubmit, NULL)) != Success)
	    {
		sprintf(uash_globals.tmpbuf,
			"Could not start timer to check for submissions; "
			"rc = 0x%x!", rc);
		EH_fatal(uash_globals.tmpbuf);
	    }

#endif /* CHECKSUBMIT_WATCHDOG */

	TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
		  "No newly-submitted messages found."));

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    	if (maxInactiveIntervals > 0 && ++inactiveIntervalsCounter > maxInactiveIntervals) {
	    bTerminateRequested = TRUE;
	}
#endif /* OS_TYPE_MSDOS && OS_VARIANT_Dos */

	return Success;
    }

    /*
     * Yup.  Process it.
     */

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    inactiveIntervalsCounter = 0;
#endif

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
	      "Found newly-submitted message file (%s)", pFileName));

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

    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL, "Header = (%s)", pHeaderText));
    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL, "Body = (%s)", pBodyText));

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
#if CHECKSUBMIT_WATCHDOG
	if ( uash_globals.csWatchdogInitiated == TRUE ) {
	    TM_TRACE((uash_globals.hModCB, TM_DEBUG,
		      "checkSubmit() watchdog initiated, skipping TMR_start() call."));
	}
	else {

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
#else /* CHECKSUBMIT_WATCHDOG */

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
#endif /* CHECKSUBMIT_WATCHDOG */
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

    TM_TRACE((uash_globals.hModCB, TM_ENTER, "submitRfc822Message() called."));

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
	sprintf(buf, "Submission failed initially (locally), reason 0x%x!",
		rc);
        uash_displayMessage(buf);
    }
    else
    {
	char buf[512];

#if CHECKSUBMIT_WATCHDOG
	uash_globals.csWatchdogSubmissionStarted = TRUE;
#endif /* CHECKSUBMIT_WATCHDOG */

#if defined(OS_VARIANT_WinCE)
	sndPlaySound(TEXT("question"), SND_ASYNC | SND_ALIAS );
#endif

	sprintf(buf, "Submission started (pending confirmation)!");
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
    int		    count;
    int		    subspan;
    char	    *pFromFormPos;
    char	    *pBodyTextMunged;

#define	OPEN_MODE	"r+"
#define	WRITE_MODE	"w+"

    /*
     * Open the file.  Don't use APPEND mode because it prevents
     * seeking back to the header and modifying it, in Dawz mode.
     */
    if ((hFolder = OS_fileOpen(pDataFileName, OPEN_MODE)) == NULL) {

      TM_TRACE((uash_globals.hModCB, TM_DEBUG, "writeRfc822Message(): OS_fileOpen() in OPEN_MODE returned NULL."));
      if ((hFolder = OS_fileOpen(pDataFileName, WRITE_MODE)) == NULL) {

	TM_TRACE((uash_globals.hModCB, TM_DEBUG, "writeRfc822Message(): OS_fileOpen() in WRITE_MODE returned NULL."));
	return OS_RC_FileOpen;
      }
    }
    
    /* See to the end of the file */
    fseek(hFolder, 0L, SEEK_END);

    /*
     * Compose the Message Header
     */

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


    /* build remainder of the RFC-822 header */
    p1 = uash_globals.tmpbuf;
    *p1 = '\0';

    printf("******* Date written to mbox is: <%s>", pRfcEnv->date);
    printf("******* From written to mbox is: <%s>", pRfcEnv->from);


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

    /*
     * protect against "^From " with "^>From "
     */
    {

#define PROTECT_FROM_FORM "\nFrom "
#define PROTECTED_FROM_FORM "\n>From "

	/* first count instances of "^From " in body */
	count = 0;
	pBodyTextMunged = NULL;
	p1 = pBodyText;
	while ((p2 = strstr(p1, PROTECT_FROM_FORM)) != NULL) {
	    count++;
	    p1 = p2 + strlen(PROTECT_FROM_FORM);
	}

	if ( count ) {

	    /* allocate space for munged body text */
	    pBodyTextMunged = OS_alloc(strlen(pBodyText)+count+1);
	    if ( NULL == pBodyTextMunged ) {
		EH_fatal("Out of memory.");
		return Fail;	/* should never reach here */
	    }

	    /* now protect the ^From line(s) in body */
	    for (p1 = pBodyText, p2 = pBodyTextMunged; count > 0; count--) {
		pFromFormPos = strstr(p1, PROTECT_FROM_FORM);
		subspan = pFromFormPos - p1;
		strncpy(p2, p1, subspan);
		strcat(p2+subspan, PROTECTED_FROM_FORM);
		p2 += subspan + strlen(PROTECTED_FROM_FORM);
		p1 += subspan + strlen(PROTECT_FROM_FORM);
	    }

	    /* copy the final fragment of p1 */
	    strcat(p2, p1);

	}
    }

    /* write the body */
    if ( pBodyTextMunged ) {
	OS_filePutString(pBodyTextMunged, hFolder);
	OS_free(pBodyTextMunged);
    }
    else {
	OS_filePutString(pBodyText, hFolder);
    }

    /* write two trailing new lines to make it proper folder format */
    OS_filePutString("\n\n", hFolder);

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



/* look for files that start with '[Ss]_' */
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


#if defined(OS_VARIANT_WinCE)

#include "resource.h"

LRESULT CALLBACK WndProc(HWND, UINT, UINT, LONG);
BOOL InitApplication(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

/* the following array contains all buttons of bitmap IDB_STD_SMALL_COLOR */
static TBBUTTON tbSTDButton[] = {
	{0,              0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0},
	{STD_FIND,       IDM_DELIVERYCTRL, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,              0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0},
	{STD_PROPERTIES, IDM_UASHCONFIG, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,              0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}
};


HINSTANCE	hInst;		/* Local copy of hInstance */
HWND        	hwndMain;	/* Result of CreateWindow -- handle to main window */
HWND		hwndCB;		/* CommandBar */
HMENU		hMenuCB;	/* CommandBar menu */

static QU_Head	logMessageQHead = QU_INITIALIZE(logMessageQHead);
static short	messageCount = 0;
static short	maxMessages;
static short	cyClient;
static short	cyChar;

typedef struct
{
    QU_ELEMENT;
    char	message[1];
} LogMessage;


int WINAPI
WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR     lpCmdLine,
	int        nCmdShow)
{
    MSG 	msg;

    /* Initialize the trace module */
    TM_INIT();

    /* Initialize the OS module */
    if (OS_init() != Success)
    {
	sprintf(uash_globals.tmpbuf, "Could not initialize OS module!");
	EH_fatal(uash_globals.tmpbuf);
    }

    /* Give ourselves a trace handle */
    if (TM_OPEN(uash_globals.hModCB, "UASH") == NULL)
    {
	EH_fatal("Could not open UASH trace");
    }

    TM_VALIDATE();

    /* Windows initialization */
    if ( hPrevInstance == 0 ) {
	if ( InitApplication(hInstance) == FALSE ) { return(FALSE);
	}
    }

    if ( InitInstance(hInstance, nCmdShow) == FALSE ) {
	return(FALSE);
    }

#if defined(OS_VARIANT_WinCE)
    /* *** NOTYET */
#else
    /* Arrange for EH_ messages to pop-up a message box */
    EH_displayMessages(displayError);

    /* See if there's a configuration file name specified */
    if (lpCmdLine != NULL)
    {
	/* Scan through white space at the end of the string */
	for (p = lpCmdLine + strlen(lpCmdLine) - 1; isspace(*p); p--)
	    ;

	/* Null terminate in front of superfluous white space */
	*++p = '\0';
    }

#endif

    /* Initialize everything */
    UASH_init(1, &lpCmdLine);

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Entering GetMessage loop"));
    uash_displayMessage("EMSD UA Shell for Windows CE initialized.");

    /* Main Loop */
    while (GetMessage (&msg, 0, 0, 0)) {

	HANDLE	hTickFile;

	TM_TRACE((uash_globals.hModCB, TM_DEBUG, "In GetMessage loop"));
	uash_globals.loopCount++;

	if (( g_delCtrlWaitingMtsAck == TRUE )
	    && ( g_delCtrlMtsAckRecvd == TRUE )) {
	    uash_displayMessage("Message delivery request acknowledged.");
#if defined(OS_VARIANT_WinCE)
	    sndPlaySound(TEXT("astersk"), SND_ASYNC | SND_ALIAS );
#endif /* OS_VARIANT_WinCE */
	    g_delCtrlWaitingMtsAck = FALSE;
	}


#if CHECKSUBMIT_WATCHDOG
	/* we don't check for new submissions if there is already
	 * a submission started (pending confirmation), so count
	 * only progress if no submission is currently underway */

#if 0
	TM_TRACE((uash_globals.hModCB, TM_DEBUG,
		  "Watchdog: sub started=<%d>; iou=<%d>; csw loop count=<%d>; csw Initated=<%d>",
		  uash_globals.csWatchdogSubmissionStarted,
		  uash_globals.csWatchdogIOUCount,
		  uash_globals.csWatchdogLoopCount,
		  uash_globals.csWatchdogInitiated
		  ));
#endif

	if ( uash_globals.csWatchdogSubmissionStarted == FALSE ) {
	    uash_globals.csWatchdogLoopCount++;

	    if ( uash_globals.csWatchdogLoopCount > 12 ) {
		TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Periodic checkSubmit() is late so invoking 'manually'."));
		uash_globals.csWatchdogIOUCount++;
		uash_globals.csWatchdogInitiated = TRUE;
		checkSubmit((void *)NULL, (void *)NULL, (void *)NULL);
		uash_globals.csWatchdogInitiated = FALSE;
	    }
	}
#endif /* CHECKSUBMIT_WATCHDOG */

	TranslateMessage (&msg);
	DispatchMessage (&msg);

	if ((uash_globals.loopCount % 4) == 0 ) {
	    /* every fourth time through this loop */

	    /* heartbeat - touch file \EMSD\uashwce_one.tick */
	    hTickFile = CreateFile(TEXT("\\EMSD\\uashwce_one.tick"),
				   GENERIC_WRITE,
				   FILE_SHARE_READ,
				   NULL,
				   CREATE_ALWAYS,
				   FILE_ATTRIBUTE_NORMAL,
				   NULL);
	    if ( hTickFile == INVALID_HANDLE_VALUE ) {
		TM_TRACE((uash_globals.hModCB, TM_DEBUG, "Failed creating tick file."));
	    }
	    else {
		static CHAR buf[] = "Used for GetFileTime() only.";
		DWORD	bytesWritten;

		/* access the tick file */
		WriteFile(hTickFile, buf, sizeof(buf), &bytesWritten, NULL);
		CloseHandle(hTickFile);
	    }

	    /* heartbeat - animate icon in system tray */
	    if ( uash_globals.fVisibleHeartbeat == TRUE ) {
		if ( uash_globals.trayIconStateIsFlip ) {
		    CloseHandle(uash_globals.trayIconFlop);
		    uash_globals.trayIconFlop = LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON4), IMAGE_ICON, 16, 16, 0);
		    uash_globals.tnid.hIcon = uash_globals.trayIconFlop;
		    uash_globals.trayIconStateIsFlip = FALSE;
		}
		else {
		    CloseHandle(uash_globals.trayIconFlip);
		    uash_globals.trayIconFlip = LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON5), IMAGE_ICON, 16, 16, 0);
		    uash_globals.tnid.hIcon = uash_globals.trayIconFlip;
		    uash_globals.trayIconStateIsFlip = TRUE;
		}

		Shell_NotifyIcon(NIM_MODIFY, &uash_globals.tnid);
	    }
	}

	if (! bTerminateRequested) {
	    if (SCH_block() >= 0) {
		SCH_run();
	    }
	}
    }

    TM_TRACE((uash_globals.hModCB, TM_DEBUG, "left GetMessage loop"));

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


/***************************************************************************

		BOOL InitApplication ( HINSTANCE hInstance )

****************************************************************************/
BOOL InitApplication ( HINSTANCE hInstance )
{
    WNDCLASSW  wc;
    BOOL f;

    LoadString(hInstance, IDS_APPNAME, wszAppName, sizeof(wszAppName)/sizeof(TCHAR));
    LoadString(hInstance, IDS_TITLE, wszAppTitle, sizeof(wszAppTitle)/sizeof(TCHAR));
    wcstombs(szAppName, wszAppName, strlen(szAppName));

    wc.style =  0 ;
    wc.lpfnWndProc = (WNDPROC) WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = wszAppName;

    f = (RegisterClass(&wc));
    return f;
}

/*************************************************************************************

		BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )

  ************************************************************************************/

BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
    hInst = hInstance;
    hwndMain = CreateWindow(wszAppName,
			    wszAppTitle,
			    WS_VISIBLE,
			    0,
			    0,
			    CW_USEDEFAULT,
			    CW_USEDEFAULT,
			    NULL, NULL, hInstance, NULL);

    if ( hwndMain == 0 )    /* Check CreateWindow return values for validity */
	{ return (FALSE); }
    if (IsWindow(hwndMain) != TRUE)
	{ return (FALSE); }

    ShowWindow(hwndMain, nCmdShow);
    ShowWindow(hwndMain, SW_SHOWNORMAL);
    UpdateWindow(hwndMain);
    ShowWindow(hwndMain, SW_HIDE);

    /* add icon to tray area */
    {
	BOOL		res;

	uash_globals.tnid.cbSize 	= sizeof(NOTIFYICONDATA);
	uash_globals.tnid.hWnd	= hwndMain;
	uash_globals.tnid.uID	= 42;	/* pick a number... */
	uash_globals.tnid.uFlags	= NIF_ICON | NIF_MESSAGE;
	uash_globals.tnid.uCallbackMessage	= UASH_MSG_NOTIFYICON;

	/* set tray icon */
	uash_globals.trayIconInit = LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 16, 16, 0);
	uash_globals.trayIconFlip = LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON4), IMAGE_ICON, 16, 16, 0);
	uash_globals.trayIconFlop = LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON5), IMAGE_ICON, 16, 16, 0);
	uash_globals.trayIconStateIsFlip = TRUE;
	uash_globals.tnid.hIcon 	= uash_globals.trayIconInit;

	res = Shell_NotifyIcon(NIM_ADD, &uash_globals.tnid);
    }

    return(TRUE);                  /* Window handle hWnd is valid */
}


/*************************************************************************************

  ConfigDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)

 ************************************************************************************/
BOOL CALLBACK
UashConfigDlgProc(HWND	hDlg,
		  UINT	message,
		  WPARAM	wParam,
		  LPARAM	lParam)
{
    WORD wNotifyCode, wID;
    HWND hwndCtl;
    TCHAR szBuf[50];
    wNotifyCode = HIWORD(wParam); /* notification code */
    wID = LOWORD(wParam);         /* item, control, or accelerator identifier */
    hwndCtl = (HWND) lParam;      /* handle of control */

    switch(message)
	{
	case WM_COMMAND:
	    {
		switch(wID)
		    {
		    case IDOK:
		    case IDCANCEL:
			EndDialog(hDlg, TRUE);
			break;
		    default:
			return(FALSE);
		    }
		break;
	    }
	case WM_INITDIALOG:
	    {
		wsprintf(szBuf, TEXT("%S"), config.pFrom);
		SetDlgItemText(hDlg,IDC_SUBSID, szBuf);
		wsprintf(szBuf, TEXT("%S"), config.pMtsNsap);
		SetDlgItemText(hDlg,IDC_MTSADDR, szBuf);
		wsprintf(szBuf, TEXT("%S"), config.pMessageSubmissionAttemptInterval);
		SetDlgItemText(hDlg,IDC_POLLINGINTERVAL, szBuf);
		wsprintf(szBuf, TEXT("%S"), config.pNonVolatileMemorySize);
		SetDlgItemText(hDlg,IDC_NVMSIZE, szBuf);

		break;
	    }
	default:
	    return(FALSE);
	}
    return (TRUE);
}


/*************************************************************************************

  WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)

 ************************************************************************************/

LRESULT CALLBACK
WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LogMessage *    	pLogMessage;
    HDC 	    	hdc;
    PAINTSTRUCT		ps;
    TEXTMETRIC 	    	tm;
    short		i;
    short		x;
    short		y;
    static short    	cxChar;
    static short    	cxCaps;
    static short    	cxClient;
    static short    	nMaxWidth;
    static int	    	closeCount = 1;

    switch (message) {

    case WM_CREATE:
	{
	    hwndCB = CommandBar_Create(hInst, hwnd, 1);
	    /* Add bitmap IDB_STD_SMALL_COLOR to Commandbar (15 buttons) */
	    CommandBar_AddBitmap(hwndCB, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 15, 16, 16);
	    CommandBar_InsertMenubar(hwndCB, hInst, IDM_MAIN_MENU, 0);
	    /* Add buttons in tbSTDButton to Commandbar */
	    CommandBar_AddButtons(hwndCB, sizeof(tbSTDButton)/sizeof(TBBUTTON), tbSTDButton);
	    /* Add help and OK buttons */
	    CommandBar_AddAdornments(hwndCB, CMDBAR_HELP | CMDBAR_OK , 0);
	    /* Get menu handle */
	    hMenuCB=CommandBar_GetMenu(hwndCB,0);
	}

    /* create a periodic timer that sends WM_TIMER so that the
     * GetMessage loop will run periodically.  This is *not* the timer
     * that is created by TMR_startClockInterrupt() (that uses a
     * TimerProc rather than the WM_TIMER because we want timely
     * processing of the timer tick which we cannot guarantee with
     * WM_TIMER messages as the latter depends on what other WM_
     * messages are in the queue.
     */

#define TIMER_ID_FORCE_GET_MESSAGE		42 /* pick a number ... */
#define TIMER_ID_FORCE_GET_MESSAGE_PERIOD	500 /* milliseconds */

    if ( SetTimer((HWND) hwnd,
		  (UINT) TIMER_ID_FORCE_GET_MESSAGE,
		  (UINT) TIMER_ID_FORCE_GET_MESSAGE_PERIOD,
		  (TIMERPROC) NULL) == 0 ) {
	EH_fatal("Failed setting WM_TIMER for GetMessage loop.");
    }

    hdc = GetDC (hwnd);

    GetTextMetrics (hdc, &tm);
    cxChar = tm.tmAveCharWidth;
    cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
    cyChar = tm.tmHeight + tm.tmExternalLeading;

    ReleaseDC (hwnd, hdc);

    nMaxWidth = 40 * cxChar + 22 * cxCaps;
    return 0L;

    case WM_HELP:
	MessageBox(hwnd,
		   TEXT("Support Resources: http://www.neda.com, info@neda.com"),
		   TEXT("EMSD UA Shell for Windows CE"),
		   MB_OK);
	return 0L;
    case WM_SIZE:

	cxClient = LOWORD (lParam);
	cyClient = HIWORD (lParam);
	maxMessages = cyClient / cyChar;

	TM_TRACE((uash_globals.hModCB, TM_DEBUG, "WM_SIZE: maxMessages is %d", maxMessages));

	/* If size was reduced, eliminate messages */
	for (; messageCount > maxMessages; messageCount--)
	    {
		pLogMessage = QU_FIRST(&logMessageQHead);
		QU_REMOVE(pLogMessage);
		OS_free(pLogMessage);
	    }
	return 0L;

    case WM_TIMER:

	/* all this message does is to make sure that GetMessage
	 * returns periodically so that SCH_run() in the main loop can
	 * run.
	 */

	return 0L;
	break;

    case WM_COPYDATA:

	TM_TRACE((uash_globals.hModCB, TM_DEBUG, "WM_COPYDATA: message received"));

	return TRUE;		/* TRUE ==> handled */

    case WM_PAINT:
	hdc = BeginPaint(hwnd, &ps);

	/* Display the messages */
	for (pLogMessage = QU_FIRST(&logMessageQHead), i = 0;
	     ! QU_EQUAL(pLogMessage, &logMessageQHead);
	     pLogMessage = QU_NEXT(pLogMessage), i++) {

	    WCHAR	*wLine;
	    int		len = strlen(pLogMessage->message);

	    x = cxChar;
	    y = cyChar * i;

	    wLine = (WCHAR *)malloc(sizeof(WCHAR) * (len + 1));
	    wsprintf(wLine, TEXT("%S"), pLogMessage->message);
	    ExtTextOut(hdc, x, y, 0, 0, wLine, len, NULL);
	    free(wLine);
	}

	EndPaint(hwnd, &ps);
	return 0L;

    case UASH_MSG_NOTIFYICON:
	{
	    ShowWindow(hwndMain, SW_SHOW);
	    UpdateWindow(hwndMain);
	    return 0L;
	}
    case WM_COMMAND:

	switch (wParam) {

	case IDOK:
	    ShowWindow(hwndMain, SW_HIDE);
	    return 0L;

	case IDM_EXIT:
	    SendMessage(hwnd, WM_CLOSE, 0, 0L);
	    return 0L;

	case IDM_HELP:
	    SendMessage(hwnd, WM_HELP, 0, 0L);
	    return 0L;

	case IDM_ABOUT:
#if defined(OS_VARIANT_WinCE)
	    MessageBox(hwnd, wCopyrightNotice,
		       TEXT("EMSD UA Shell for Windows CE"),
		       MB_OK);
#else
	    MessageBox(hwnd, RELID_getCopyrightNedaAtt(),
		       "EMSD UA",
		       MB_OK);
#endif
	    return 0L;

	case IDM_UASHCONFIG:
	    {
		DialogBox(hInst, MAKEINTRESOURCE(IDD_UASHCONFIG), hwnd,
			  (WNDPROC)UashConfigDlgProc);
		return 0L;
	    }

	case IDM_DELIVERYCTRL:
	    {
		void		*hMessage;
		void		*hShellReference = "";
		ReturnCode	rc;

		if ((rc = UA_issueDeliveryControl(&hMessage, hShellReference)) != Success) {
		    sprintf(uash_globals.tmpbuf, "Can not contact Message Center because of a local problem!");
		    uash_displayMessage(uash_globals.tmpbuf);

		    TM_TRACE((uash_globals.hModCB, UASH_TRACE_ACTIVITY,
			      "Delivery Control failed locally."));
		}
		else {

		    g_delCtrlWaitingMtsAck = TRUE;
		    g_delCtrlMtsAckRecvd = FALSE;

		    sprintf(uash_globals.tmpbuf, "Triggering message deliveries...");
#if defined(OS_VARIANT_WinCE)
		    sndPlaySound(TEXT("question"), SND_ASYNC | SND_ALIAS );
#endif
		    uash_displayMessage(uash_globals.tmpbuf);

		    TM_TRACE((uash_globals.hModCB, UASH_TRACE_DETAIL,
			      "Delivery Control Operation started (pending confirmation)."));
		}

	    }
	return 0L;
	}

    case WM_CLOSE:

	Shell_NotifyIcon(NIM_DELETE, &uash_globals.tnid);
	KillTimer(hwnd, TIMER_ID_FORCE_GET_MESSAGE);
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
	   return 0L;
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


#define DISPLAYMSG_MAXLEN	255
#define TIMESTAMP_STRLEN 	15
void
uash_displayMessage(char * pBuf)
{
    SYSTEMTIME	tm;
    static Bool	firstTime = TRUE;
    LogMessage	*pLogMessage;
    char	tmpBuf[DISPLAYMSG_MAXLEN+1];
    WCHAR	wcTimestampBuf[TIMESTAMP_STRLEN+1];
    char	timestampBuf[TIMESTAMP_STRLEN+1];
    int		idx;

    TM_TRACE((uash_globals.hModCB, TM_ENTER, "uash_displayMessage() entered."));

    /* timestamp */
    GetLocalTime(&tm);
    wsprintf(wcTimestampBuf, TEXT("[%02d%02d%02d%02d%02d%02d] "),	/* strlen must be == TIMESTAMP_STRLEN */
	     tm.wYear % 100, tm.wMonth, tm.wDay,
	     tm.wHour, tm.wMinute, tm.wSecond);
    wcstombs(timestampBuf, wcTimestampBuf, sizeof(timestampBuf));

    /* fill with spaces and null terminate */
    for (idx = 0; idx < DISPLAYMSG_MAXLEN; idx++) tmpBuf[idx] = ' ';
    tmpBuf[DISPLAYMSG_MAXLEN] = '\0';

    /* copy and maybe truncate string */
    strncpy(tmpBuf, timestampBuf, TIMESTAMP_STRLEN);
    if ((strlen(pBuf) + TIMESTAMP_STRLEN) < DISPLAYMSG_MAXLEN ) {
	strncpy(tmpBuf+TIMESTAMP_STRLEN, pBuf, strlen(pBuf));
	TM_TRACE((uash_globals.hModCB, TM_DEBUG, "uash_displayMessage() line: <%s>.", tmpBuf));
    }
    else {
	strncpy(tmpBuf+TIMESTAMP_STRLEN, pBuf, DISPLAYMSG_MAXLEN - TIMESTAMP_STRLEN);
	TM_TRACE((uash_globals.hModCB, TM_DEBUG, "uash_displayMessage() line: <%s>.", tmpBuf));
    }

    /* If there are no messages remaining, free the first one. */
    if (messageCount == maxMessages) {
	pLogMessage = QU_FIRST(&logMessageQHead);
	QU_REMOVE(pLogMessage);
	OS_free(pLogMessage);
	--messageCount;
    }

    /* Allocate a new log message structure */
    if ((pLogMessage = OS_alloc(sizeof(LogMessage) + strlen(tmpBuf))) == NULL) {
	/* nothing to do if we're out of memory */
	return;
    }

    /* We've used one of the messages */
    ++messageCount;

    /* Initialize the queue pointers */
    QU_INIT(pLogMessage);

    /* Copy the message string */
    strcpy(pLogMessage->message, tmpBuf);

    /* Insert it into the log message queue */
    QU_INSERT(pLogMessage, &logMessageQHead);

    /* Update the screen */
    InvalidateRect(hwndMain, NULL, FALSE);
    UpdateWindow(hwndMain);
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
    MessageBox(hwndMain, buf,
	       "EMSD UA",
	       MB_OK | MB_ICONINFORMATION);
}


#endif /* OS_VARIANT_WinCE */
