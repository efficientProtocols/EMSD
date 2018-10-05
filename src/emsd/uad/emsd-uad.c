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
#include "emsdmail.h"
#include "emsd822.h"
#include "strfunc.h"
#include "buf.h"
#include "tmr.h"
#include "config.h"
#include "emsd.h"
#include "emsdd.h"
#include "asn.h"
#include "tm.h"
#include "eh.h"
#include "profile.h"
#include "nnp.h"


static int
DUP_invokeIn (ESRO_Event *ep)
{
    ESRO_OperationValue  op  = ep->un.invokeInd.operationValue;
    Byte *               dp  = ep->un.invokeInd.data;
    Int                  len = ep->un.invokeInd.len;

    if (op > 31 && op < 128)
      {
	bcopy (dp + 1, dp, ESRODATASIZE - 1);
	ep->un.invokeInd.len -= 1;
	return (1);
      }
    return (0);
}

#define	EMSD_PROTOCOL_VERSION_MAJOR		1
#define	EMSD_PROTOCOL_VERSION_MINOR		1

extern char *	cpystr (char * pString);

static ReturnCode
fromRfc822ToEmsd(char * pMessageFileName,
		EMSDD_ControlFileData * pControlFileData);

static ReturnCode
fromEmsdToRfc822(char * pMessageFileName,
		EMSDD_ControlFileData * pControlFileData);

static void
nonDelToRfc822(char * pControlFileName,
	       char * pDataFileName,
	       char * pOriginator,
	       EMSDD_RecipList * pRecipList);

static void
nonDelToEmsd(char * pControlFileName,
	    char * pDataFileName,
	    char * pOriginator,
	    EMSDD_RecipList * pRecipList);

static OS_Boolean
dirFilter(const char * pName);

static ReturnCode
deliverToEmsdUser(char ** ppRecipients,
		 ENVELOPE * pRfcEnv,
		 char * pExtraHeaders,
		 BODY * pRfcBody);

static ReturnCode
getMailboxName(char * pRecipient,
	       char ** ppMailBox);


static ReturnCode
checkTerminate(void * hTimer,
	       void * hUserData1,
	       void * hUserData2);

static char *
diagnosticString(EMSD_NonDeliveryDiagnostic diagnostic);

static char *
reasonString(EMSD_NonDeliveryReason reason);

static ReturnCode
issueInvoke(EMSDD_RecipList * pRecipList,
	    void * hInvokeData);

static ReturnCode
writeEmsdPdu(QU_Head * pRecipList,
	    void * hBuf);

static ReturnCode
formatAndInvoke(void * pArg,
		void * hContents,
		void * pRecipList);

static void
esrosListener();


char *			__applicationName = "" ;
TM_ModuleCB *		emsdd_hModCB;


static char 		errbuf[1024];
static ESRO_SapDesc 	hLocalSap;
static void * 		hConfig;


/*
 * ASN "compiled" trees for the EMSD protocols.
 */
static QU_Head		compIpm            = QU_INITIALIZE(compIpm);
static QU_Head		compReportDelivery = QU_INITIALIZE(compReportDelivery);
static QU_Head		compDeliverArg     = QU_INITIALIZE(compDeliverArg);
static QU_Head		compSubmitArg      = QU_INITIALIZE(compSubmitArg);
static QU_Head		compSubmitResult   = QU_INITIALIZE(compSubmitResult);
static QU_Head		compErrorRequest   = QU_INITIALIZE(compErrorRequest);

static struct
{
    struct
    {
	char *		pNewMessageDirectory;
	char *		pQueueDirectory;
    } 		    fromRfc822ToEmsd;
    struct
    {
	char *		pNewMessageDirectory;
	char *		pQueueDirectory;
    } 		    fromEmsdToRfc822;
    QU_Head	    recipients;
    char *	    pMtsNsap;
    char *	    pEmsdHostName;
} config;

static struct ConfigParams
{
    char *	    pSectionName;
    char *	    pTypeName;
    char **	    ppValue;
} configParams[] =
  {
      {
	  "Submission",
	  "New Message Directory",
	  &config.fromRfc822ToEmsd.pNewMessageDirectory
      },
      {
	  "Submission",
	  "Queue Directory",
	  &config.fromRfc822ToEmsd.pQueueDirectory
      },
      {
	  "Delivery",
	  "New Message Directory",
	  &config.fromEmsdToRfc822.pNewMessageDirectory
      },
      {
	  "Delivery",
	  "Queue Directory",
	  &config.fromEmsdToRfc822.pQueueDirectory
      },
      {
	  "Delivery",
	  "EMSD Host Name",
	  &config.pEmsdHostName
      },
      {
	  "Communications",
	  "MTS NSAP Address",
	  &config.pMtsNsap
      }
  };

typedef struct User
{
    QU_ELEMENT;
    char *	    pEmsdAddress;
    char *	    pMailBox;
} User;

/* Queue of local users */
QU_Head		localUsers = QU_INITIALIZE(localUsers);

int
main(int argc, char *argv[])
{
    ReturnCode			rc;
    char 			buf[32];
    char *			p;
    char *			pEmsdAddress;
    char *			pMailBox;
    char *			pConfigFile = NULL;
    ESRO_RetVal			retval;
    EMSD_Message *		pEmsdIpm;
    EMSD_SubmitArgument *	pEmsdSubmitArg;
    EMSD_DeliverArgument *	pEmsdDeliverArg;
    EMSD_ReportDelivery *	pEmsdReportDelivery;
    EMSD_ErrorRequest		emsdErrorReq;
    struct ConfigParams *	pConfigParam;
    int				pid = -1;
    int				c;
    OS_Uint32 			retryTime;
    User *			pUser;
    static char			appName[128];
    
    strcpy(appName, argv[0]);
    __applicationName = appName;

    /* Initialize the trace module */
    TM_INIT();
    
    /* Initialize the OS module */
    if (OS_init() != Success)
    {
	sprintf(errbuf, "Could not initialize OS module");
	EH_fatal(errbuf);
    }

    /* Initialize the OS memory allocation debug routines */
    OS_allocDebugInit(NULL);

    /* Initialize the OS file tracking debug routines */
    OS_fileDebugInit("/tmp/etwpd.files");

    /* Scan past optional arguments to find the configuration file name */
    while ((c = getopt(argc, argv, "p:T:")) != EOF)
	;
    
    /* This argument should be the configuration file name */
    if (optind >= argc)
    {
	/* No configuration file name was given. */
	goto usage;
    }
    
    /* Save the configuration file name pointer */
    pConfigFile = argv[optind];

    /* Initialize the config module */
    if ((rc = CONFIG_init()) != Success)
    {
	sprintf(errbuf,
		"%s: Could not initialize CONFIG module\n",
		__applicationName);
	EH_fatal(errbuf);
    }

    /* Open the configuration file.  Get a handle to the configuration data */
    if ((rc = CONFIG_open(pConfigFile, &hConfig)) != Success)
    {
	sprintf(errbuf,
		"%s: Could not open configuration file (%.*s), "
		"reason 0x%04x\n",
		argv[0], (int) (sizeof(errbuf) / 2), pConfigFile, rc);
	EH_fatal(errbuf);
    }

    /* Get the default trace flags from the configuration file */
    for (c = 1, rc = Success; rc == Success; c++)
    {
	/* Create the parameter type string */
	sprintf(buf, "Module %d", c);

	/* Get the next trace parameter */
	if ((rc = CONFIG_getString(hConfig,
				   "Trace",
				   buf,
				   &p)) == Success)
	{
	    /* Issue the trace command */
	    TM_SETUP(p);
	}
    }

    optind = 1;
    while ((c = getopt(argc, argv, "p:T:")) != EOF)
    {
        switch (c)
	{
	case 'p':		/* specify process to run (0=child) */
	    pid = atoi(optarg);
	    break;

	case 'T':		/* enable trace module tracing */
	    TM_SETUP(optarg);
	    break;

	default:
	  usage:
	    sprintf(errbuf,
		    "usage: %s [-p <0|1>] [-T <TM option string>] "
		    "<configuration file name>\n",
		    argv[0]);
	    EH_fatal(errbuf);
	    break;
	}
    }

    /* Give ourselves a trace handle */
    if (TM_OPEN(emsdd_hModCB, "UA") == NULL)
    {
	EH_fatal("Could not open UA trace");
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
	    sprintf(errbuf,
		    "%s: Configuration parameter\n\t%s/%s\n"
		    "\tnot found, reason 0x%04x",
		    argv[0],
		    pConfigParam->pSectionName,
		    pConfigParam->pTypeName,
		    rc);
	    EH_fatal(errbuf);
	}

	TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT,
		  "\n    Configuration parameter\n"
		  "\t[%s]:%s =\n\t%s\n",
		  pConfigParam->pSectionName,
		  pConfigParam->pTypeName,
		  *pConfigParam->ppValue));
    }

    /* Get retry times */
    for (c = 1, rc = Success; rc == Success; c++)
    {
	/* Create the parameter type string */
	sprintf(buf, "Time %d", c);
	if ((rc = CONFIG_getNumber(hConfig,
				   "Retry Times",
				   buf,
				   &retryTime)) == Success)
	{
	    /* Add this time to the list of retries */
	    rc = emsdd_addRetryTime(retryTime);

	    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT,
		      "Retry time at %lu seconds\n", retryTime));
	}
    }

    /* Initialize the queue of local users */
    QU_INIT(&localUsers);

    /* Get the list of local users for this user agent */
    for (c = 1, rc = Success; rc == Success; c++)
    {
	/* Create the parameter type string */
	sprintf(buf, "User %d", c);
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
		if ((pUser = (OS_alloc(sizeof(User) +
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
		pUser->pEmsdAddress = (char *) (pUser + 1);

		/* Copy the recipient name */
		strcpy(pUser->pEmsdAddress, pEmsdAddress);

		/*
		 * Point to the area that's reserved for the mailbox file
		 * name.
		 */
		pUser->pMailBox = pUser->pEmsdAddress + strlen(pEmsdAddress) + 1;

		/* Copy the mailbox name */
		strcpy(pUser->pMailBox, pMailBox);

		/* Insert this recipient onto our "users" queue */
		QU_INSERT(pUser, &localUsers);
		
		TM_TRACE((emsdd_hModCB, EMSDD_TRACE_MESSAGE_FLOW,
			  "\n\tUser: %s\n\tMailbox: %s\n",
			  pUser->pEmsdAddress,
			  pUser->pMailBox));
	    }
	}
    };

    /* Save the local domain/host name since the rfc822_* functions need it */
    emsdd_setLocalDomainAndHost(config.pEmsdHostName);
    
    /*
     * ASN compile each of the EMSD protocols
     */

    /* Initialize the ASN Module */
    if ((rc = ASN_init()) != Success)
    {
	sprintf(errbuf,
		"%s: "
		"Could not initialize ASN module\n",
		argv[0]);
	EH_fatal(errbuf);
    }

    /* Allocate an EMSD Message structure */
    if ((rc = emsdd_allocMessage(&pEmsdIpm)) != Success)
    {
	sprintf(errbuf,
		"%s: "
		"Could not allocate an EMSD message structure.",
		argv[0]);
	EH_fatal(errbuf);
    }

    /* Compile an EMSD (Version 1.1) IPM Message */
    if ((rc = EMSD_compileIpm1_1((unsigned char *) pEmsdIpm,
				NULL, pEmsdIpm,
				0, &compIpm)) != Success)
    {
	emsdd_freeMessage(pEmsdIpm);
	sprintf(errbuf,
		"%s: Compiling IPM failed, reason 0x%04x\n",
		argv[0], rc);
	EH_fatal(errbuf);
    }

    /* We're done with this message structure */
    emsdd_freeMessage(pEmsdIpm);


    /* Allocate an EMSD Delivery Report structure */
    if ((rc =
	 emsdd_allocReportDelivery(&pEmsdReportDelivery)) != Success)
    {
	sprintf(errbuf,
		"%s: "
		"Could not allocate an EMSD delivery report.",
		argv[0]);
	EH_fatal(errbuf);
    }

    /* Compile an EMSD Delivery Report */
    if ((rc =
	 EMSD_compileReportDelivery1_1((unsigned char *)
				      pEmsdReportDelivery,
				      NULL, pEmsdReportDelivery,
				      0,
				      &compReportDelivery)) != Success)
    {
	emsdd_freeReportDelivery(pEmsdReportDelivery);
	sprintf(errbuf,
		"%s: Compiling delivery report failed, "
		"reason 0x%04x\n",
		argv[0], rc);
	EH_fatal(errbuf);
    }

    /* We're done with this Delivery Report structure */
    emsdd_freeReportDelivery(pEmsdReportDelivery);


    /* Allocate an EMSD Deliver Argument structure */
    if ((rc = emsdd_allocDeliverArg(&pEmsdDeliverArg)) != Success)
    {
	sprintf(errbuf,
		"%s: "
		"Could not allocate an EMSD deliver argument.",
		argv[0]);
	EH_fatal(errbuf);
    }

    /* Compile an EMSD Deliver Argument */
    if ((rc =
	 EMSD_compileDeliverArgument1_1((unsigned char *) pEmsdDeliverArg,
				       NULL, pEmsdDeliverArg,
				       0, &compDeliverArg)) != Success)
    {
	emsdd_freeDeliverArg(pEmsdDeliverArg);
	sprintf(errbuf,
		"%s: Compiling deliver arg failed, "
		"reason 0x%04x\n",
		argv[0], rc);
	EH_fatal(errbuf);
    }

    /* We're done with this Deliver Argument structure */
    emsdd_freeDeliverArg(pEmsdDeliverArg);


    /* Allocate an EMSD Submit Argument structure */
    if ((rc = emsdd_allocSubmitArg(&pEmsdSubmitArg)) != Success)
    {
	sprintf(errbuf,
		"%s: "
		"Could not allocate an EMSD deliver argument.",
		argv[0]);
	EH_fatal(errbuf);
    }

    /* Compile an EMSD Submit Argument */
    if ((rc =
	 EMSD_compileSubmitArgument1_1((unsigned char *) pEmsdSubmitArg,
				      NULL, pEmsdSubmitArg,
				      0, &compSubmitArg)) != Success)
    {
	emsdd_freeSubmitArg(pEmsdSubmitArg);
	sprintf(errbuf,
		"%s: "
		"Compiling submit arg failed, reason 0x%04x\n",
		argv[0], rc);
	EH_fatal(errbuf);
    }

    /* We're done with this submit argument structure */
    emsdd_freeSubmitArg(pEmsdSubmitArg);


    /* Compile an EMSD Error Indication */
    if ((rc =
	 EMSD_compileErrorRequest1_1((unsigned char *) &emsdErrorReq,
				    NULL, &emsdErrorReq,
				    0, &compErrorRequest)) !=
	Success)
    {
	sprintf(errbuf,
		"%s: "
		"Compiling error indication failed, "
		"reason 0x%04x\n",
		argv[0], rc);
	EH_fatal(errbuf);
    }

    /* Initialize the ESROS module */
    ESRO_init();

    /*
     * Here, we'll split up into two processes.  The child process
     * will become the "listener" on the ESROS socket.  The parent
     * process will enter the main EMSD daemon loop.
     */
    if (pid == -1)
    {
	pid = fork();
    }

    switch(pid)
    {
    case -1:		/* fork() error */
	sprintf(errbuf, "%s: Error creating ESROS listener process\n", argv[0]);
	perror("fork");
	EH_fatal(errbuf);

    case 0:		/* child process */
	/*
	 * Set our command name to distinguish it from the Spooler
	 * process.
	 */
	argv[0] = "Etwp Daemon (ESROS Performer)";
	strcpy(appName, "EMSD ESROS Performer");

	TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT,
		  "==> %s: Process ID: %lu <==\n",
		  appName,
		  (unsigned long) getpid()));

	esrosListener();	/* never returns */
	break;

    default:		/* parent process */
	/*
	 * Set our command name to distinguish it from the ESROS
	 * Listener process.
	 */
	argv[0] = "Etwp Daemon (RFC-822/EMSD Spooler)";
	strcpy(appName, "EMSD Spooler");

	TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT,
		  "==> %s: Process ID: %lu <==\n",
		  appName,
		  (unsigned long) getpid()));

	/* Set the directory filter */
	OS_dirSetFilter(dirFilter);

	/* Register our first client */
	if ((rc =
	     emsdd_registerClient(config.fromRfc822ToEmsd.pNewMessageDirectory,
				 config.fromRfc822ToEmsd.pQueueDirectory,
				 fromRfc822ToEmsd,
				 nonDelToRfc822)) != Success)
	{
	    sprintf(errbuf,
		    "%s: Could not register client 1 "
		    "with directories:\n"
		    "  New Messages    : %s\n"
		    "  Queue Directory : %s\n"
		    "(reason 0x%04x)\n",
		    argv[0],
		    config.fromRfc822ToEmsd.pNewMessageDirectory,
		    config.fromRfc822ToEmsd.pQueueDirectory,
		    rc);
#ifdef OS_TYPE_UNIX
	    kill(pid, SIGTERM);
#endif
	    EH_fatal(errbuf);
	}

	/* Register our second client */
	if ((rc =
	     emsdd_registerClient(config.fromEmsdToRfc822.pNewMessageDirectory,
				 config.fromEmsdToRfc822.pQueueDirectory,
				 fromEmsdToRfc822,
				 nonDelToEmsd)) != Success)
	{
	    sprintf(errbuf,
		    "%s: Could not register client 2 "
		    "with directories:\n"
		    "  New Messages    : %s\n"
		    "  Queue Directory : %s\n"
		    "(reason 0x%04x)\n",
		    argv[0],
		    config.fromEmsdToRfc822.pNewMessageDirectory,
		    config.fromEmsdToRfc822.pQueueDirectory,
		    rc);
	    EH_fatal(errbuf);
	}

	/* Create a timer to see if we should terminate */
	if ((rc = TMR_start(TMR_SECONDS(10),
			    NULL,
			    NULL,
			    checkTerminate,
			    NULL)) != Success)
	{
	    EH_fatal("No timer");
	}

	/* Activate an ESROS service access point */
	if ((retval =
	     ESRO_sapBind(&hLocalSap, EMSDD_Rsap_Etwpd_Invoker, ESRO_3Way)) < 0)
	{
	    sprintf(errbuf,
			  "%s: Could not activate "
			  "ESROS Service Access Point.",
			  argv[0]);
	    EH_fatal(errbuf);
	}

	/*
	 * Enter the main daemon loop.  This will exit only on error.
	 */
	if ((rc = emsdd_mainLoop()) != Success)
	{
	    sprintf(errbuf,
		    "%s: "
		    "Main loop exited on error, "
		    "reason 0x%04x\n",
		    argv[0], rc);
	    EH_fatal(errbuf);
	}

	break;
    }

    return 0;
}


static ReturnCode
fromRfc822ToEmsd(char * pMessageFileName,
		EMSDD_ControlFileData * pControlFileData)
{
    ReturnCode 			rc;
    OS_Uint32			len;
    char *			p;
    char * 			pBodyText;
    char * 			pHeaderText;
    MAILSTREAM *		pRfcMessage = NIL;
    ENVELOPE * 			pRfcEnv;
    EMSD_Message *		pEmsdIpm;
    EMSD_SubmitArgument *	pEmsdSubmitArg;
    EMSD_ContentType		contentType;
    void *			hFormattedIpm;

    /* Open the RFC-822 message */
    if ((pRfcMessage = mail_open(pRfcMessage,
				 pMessageFileName,
				 NIL,
				 config.pEmsdHostName)) == NULL)
    {
	return FAIL_RC(Fail,
		       ("fromRfc822ToEmsd: mail_open failed for %s\n",
			pMessageFileName));
    }

    /* Retrieve the RFC-822 Envelope */
    if ((pRfcEnv =
	 mail_fetchstructure(pRfcMessage, 1, NULL)) == NULL)
    {
	mail_close(pRfcMessage);
	return FAIL_RC(Fail,
		       ("fromRfc822ToEmsd: "
			"fetch structure failed for %s\n",
			pMessageFileName));
    }

    /* Get a pointer to the RFC-822 header text */
    p = mail_fetchheader(pRfcMessage, 1);

    /* Copy header, since the same buffer is returned by fetchtext */
    if ((pHeaderText = OS_alloc(strlen(p) + 1)) == NULL)
    {
	mail_close(pRfcMessage);
	return FAIL_RC(ResourceError,
		       ("fromRfc822ToEmsd: alloc header text"));
    }

    strcpy(pHeaderText, p);

    /* Get a pointer to the body text */
    pBodyText = mail_fetchtext(pRfcMessage, 1);

    /*
     * Convert the RFC-822 message to EMSD IPM format.
     *
     * CHEAT NOTE:
     *
     * We _know_ that we'll only be receiving normal IPM messages
     * from the etwp user, in this test implementation, so we
     * don't support any message type other than IPM.
     *
     */
    if ((rc =
	 emsdd_convertRfc822ToEmsdContent(pRfcEnv,
					pBodyText,
					pHeaderText,
					(void **) &pEmsdIpm,
					&contentType)) != Success)
    {
	OS_free(pHeaderText);
	mail_close(pRfcMessage);
	return FAIL_RC(rc,
		       ("fromRfc822ToEmsd: convertRfc822ToEmsdIpm"));
    }

    /* We're done with this memory */
    OS_free(pHeaderText);

    /* Allocate a buffer to begin formatting the IPM */
    if ((rc = BUF_alloc(0, &hFormattedIpm)) != Success)
    {
	emsdd_freeMessage(pEmsdIpm);
	mail_close(pRfcMessage);
	return FAIL_RC(rc,
		       ("fromRfc822ToEmsd: alloc IPM buffer"));
    }

    /* Format the IPM */
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 &compIpm,
			 hFormattedIpm,
			 pEmsdIpm,
			 &len)) != Success)
    {
	BUF_free(hFormattedIpm);
	emsdd_freeMessage(pEmsdIpm);
	mail_close(pRfcMessage);
	return FAIL_RC(rc,
		       ("fromRfc822ToEmsd: format IPM failed, "
			"reason 0x%04x", rc));
    }

    /* We're done with this structure. */
    emsdd_freeMessage(pEmsdIpm);

    /* Allocate an EMSD Submit Argument structure */
    if ((rc = emsdd_allocSubmitArg(&pEmsdSubmitArg)) != Success)
    {
	BUF_free(hFormattedIpm);
	emsdd_freeMessage(pEmsdIpm);
	mail_close(pRfcMessage);
	return FAIL_RC(rc,
		       ("fromRfc822ToEmsd: alloc submit arg"));
    }

    /* Get the point-to-point envelope fields */
    if ((rc =
	 emsdd_convertRfc822ToEmsdSubmit(pRfcEnv,
				       pBodyText,
				       pEmsdSubmitArg)) != Success)
    {
	BUF_free(hFormattedIpm);
	emsdd_freeSubmitArg(pEmsdSubmitArg);
	mail_close(pRfcMessage);
	return FAIL_RC(rc,
		       ("fromRfc822ToEmsd: convertRfc822ToEmsdSubmit"));
    }

    /*
     * Format the Submit Argument.  This may require segmenting it
     * and sending multiple Submit Arguments, each in its own
     * INVOKE.request.
     */
    if ((rc = emsdd_segment(pEmsdSubmitArg,
			   hFormattedIpm,
			   &pControlFileData->recipList,
			   &pEmsdSubmitArg->segmentInfo,
			   EMSDD_MAX_IPM_LEN_CONNECTIONLESS,
			   EMSDD_MIN_IPM_LEN_CONNECTIONORIENTED,
			   EMSDD_MAX_FORMATTED_DELIVER_ARG_LEN,
			   formatAndInvoke)) != Success)
    {
	BUF_free(hFormattedIpm);
	emsdd_freeSubmitArg(pEmsdSubmitArg);
	mail_close(pRfcMessage);
	return FAIL_RC(rc, ("fromRfc822ToEmsd: segment"));
    }

    /* Clean up */
    BUF_free(hFormattedIpm);
    emsdd_freeSubmitArg(pEmsdSubmitArg);
    mail_close(pRfcMessage);

    return Success;
}



static ReturnCode
formatAndInvoke(void * pArg,
		void * hContents,
		void * pRecipList)
{
    ReturnCode 			rc;
    EMSD_SubmitArgument *	pEmsdSubmitArg = pArg;
    void * 			hFormattedSubmitArg;
    OS_Uint32			len;

    TM_TRACE((NULL, EMSD_TRACE_DETAIL, "formatAndInvoke"));
    /* Allocate a buffer to begin formatting the Submit Argument */
    if ((rc = BUF_alloc(0, &hFormattedSubmitArg)) != Success)
    {
	return FAIL_RC(rc, ("formatAndInvoke: alloc Submit Arg buffer"));
    }

    /* Save the formatted IPM buffer handle */
    pEmsdSubmitArg->hContents = hContents;

    TM_TRACE((NULL, EMSD_TRACE_DETAIL, "...Format..."));
    /* format the Submit Argument. */
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 &compSubmitArg,
			 hFormattedSubmitArg,
			 pEmsdSubmitArg,
			 &len)) != Success)
    {
	BUF_free(hFormattedSubmitArg);
	return FAIL_RC(rc,
		       ("formatAndInvoke: format submit arg failed, "
			"reason 0x%04x", rc));
    }

    /* Issue the LS_ROS Invoke Request */
    TM_TRACE((NULL, EMSD_TRACE_DETAIL, "...Invoke..."));
    if ((rc = issueInvoke(pRecipList, hFormattedSubmitArg)) != Success)
    {
	BUF_free(hFormattedSubmitArg);
	return FAIL_RC(rc, ("formatAndInvoke: issueInvoke"));
    }

    /* Clean up */
    BUF_free(hFormattedSubmitArg);

    /* All done. */
    return Success;
}



static ReturnCode
fromEmsdToRfc822(char * pMessageFileName,
		EMSDD_ControlFileData * pControlFileData)
{
    ReturnCode 			rc;
    OS_Uint32			i;
    OS_Uint32			pduLength;
    OS_Uint16			segmentSize;
    EMSDD_Recipient *		pRecip;
    EMSD_Message *		pIpm;
    EMSD_ReportDelivery *	pReportDelivery;
    EMSD_DeliverArgument *	pDeliverArg;
    EMSD_PerRecipReportData * 	pPerRecip;
    EMSDD_RecipList * 		pRecipList;
    ENVELOPE *			pRfcEnv;
    BODY *			pRfcBody = NULL;
    char			buf[MAILTMPLEN * 8];
    char			extraHeaders[1024];
    char *			p;
    char *			pReason;
    char *			pDiagnostic;
    char *			pExplanation;
    char *			pLocalDomainAndHost;
    char **			ppRecipArray;
    char **			pp;
    FILE *			hMessageFile;
    void *			hBuf;
    STR_String			hString;

    /* Determine the size of the message file (EMSD PDU) */
    if ((rc = OS_fileSize(pMessageFileName, &pduLength)) != Success)
    {
	return FAIL_RC(rc, ("fromEmsdToRfc822: fileSize"));
    }

    /* Open the file containing the EMSD PDU */
    if ((hMessageFile = OS_fileOpen(pMessageFileName, "rb")) == NULL)
    {
	return FAIL_RC(OS_RC_FileOpen, ("fromEmsdToRfc822: file open"));
    }

    /* Obtain a buffer header */
    if ((rc = BUF_alloc(1, &hBuf)) != Success)
    {
	OS_fileClose(hMessageFile);
	return FAIL_RC(rc, ("fromEmsdToRfc822: buffer alloc"));
    }

    /*
     * Read the ASN-formatted EMSD PDU contained within the file.
     * Create a buffer chain, making sure that no single buffer
     * requires a length that won't fit in 16 bits.  (Sigh; we're
     * still desiging for a silly Pee Cee running DOS...)
     */
    for (i = pduLength; i > 0; i -= segmentSize)
    {
	/* Get the largest segment size that fits in 16 bits */
	segmentSize = (pduLength > 0xffff ? 0xffff : pduLength);

	/* Allocate a string long enough for this formatted file */
	if ((rc = STR_alloc(segmentSize, &hString)) != Success)
	{
	    OS_fileClose(hMessageFile);
	    BUF_free(hBuf);
	    
	    return FAIL_RC(rc, ("fromEmsdToRfc822: alloc string"));
	}

	/* Read the specified amount of data */
	if (OS_fileRead(STR_stringStart(hString), 1,
			segmentSize, hMessageFile) != segmentSize)
	{
	    OS_fileClose(hMessageFile);
	    STR_free(hString);
	    BUF_free(hBuf);

	    return FAIL_RC(OS_RC_FileRead, ("fromEmsdToRfc822: read"));
	}

	/* Set the new length of this string */
	if ((rc = STR_setStringLength(hString,
				      segmentSize)) != Success)
	{
	    OS_fileClose(hMessageFile);
	    STR_free(hString);
	    BUF_free(hBuf);

	    return FAIL_RC(rc, ("fromEmsdToRfc822: set string length"));
	}

	/* Attach this string to the buffer */
	if ((rc = BUF_appendChunk(hBuf, hString)) != Success)
	{
	    OS_fileClose(hMessageFile);
	    STR_free(hString);
	    BUF_free(hBuf);

	    return FAIL_RC(rc, ("fromEmsdToRfc822: buf append chunk"));
	}

	/* Remove our reference to this string.  The buffer has it now. */
	STR_free(hString);
    }

    /* We're done with this file now. */
    OS_fileClose(hMessageFile);

    /* Allocate an EMSD Deliver Argument structure */
    if ((rc = emsdd_allocDeliverArg(&pDeliverArg)) != Success)
    {
	BUF_free(hBuf);

	return FAIL_RC(rc, ("fromEmsdToRfc822: alloc deliver arg"));
    }

    /* Parse the EMSD PDU */
    if ((rc = ASN_parse(ASN_EncodingRules_Basic,
			&compDeliverArg,
			hBuf, pDeliverArg, &pduLength)) != Success)
    {
	BUF_free(hBuf);
	emsdd_freeDeliverArg(pDeliverArg);

	return FAIL_RC(rc, ("fromEmsdToRfc822: parse deliver arg"));
    }

    /* Now parse the enclosed Deliver Content */
    switch(pDeliverArg->contentType)
    {
    case EMSD_ContentType_Probe:
	/* We're done with the PDU buffers now */
	BUF_free(hBuf);
	BUF_free(pDeliverArg->hContents);
	break;
	
    case EMSD_ContentType_DeliveryReport:
	/* Allocate an EMSD Message structure */
	if ((rc =
	     emsdd_allocReportDelivery(&pReportDelivery)) != Success)
	{
	    BUF_free(hBuf);
	    BUF_free(pDeliverArg->hContents);
	    emsdd_freeDeliverArg(pDeliverArg);

	    return FAIL_RC(rc,
			   ("fromEmsdToRfc822: alloc report delivery"));
	}

	/* Get the length of the Contents PDU */
	pduLength = BUF_getBufferLength(hBuf);

	/* Parse the IPM PDU */
	if ((rc = ASN_parse(ASN_EncodingRules_Basic,
			    &compReportDelivery,
			    pDeliverArg->hContents,
			    pReportDelivery, &pduLength)) != Success)
	{
	    BUF_free(hBuf);
	    emsdd_freeDeliverArg(pDeliverArg);
	    emsdd_freeReportDelivery(pReportDelivery);

	    return FAIL_RC(rc,
			   ("fromEmsdToRfc822: parse report delivery"));
	}

	/* We're done with the PDU buffers now. */
	BUF_free(hBuf);
	BUF_free(pDeliverArg->hContents);
	pDeliverArg->hContents = NULL;

	/* Get the local domain/host name */
	emsdd_getLocalDomainAndHost(&pLocalDomainAndHost);

	/* Obtain an RFC-822 envelope pointer */
	pRfcEnv = mail_newenvelope();

	/* Extra headers are appended to the buffer */
	*extraHeaders = '\0';

	/*
	 * Convert the Report to RFC-822
	 */

	strcpy(buf, "EMSD-Mailer-Daemon");

	/* Add the originator */
	rfc822_parse_adrlist(&pRfcEnv->from,
			     buf,
			     pLocalDomainAndHost);

	/* rfc822_parse_adrlist() may have modified buf */
	strcpy(buf, "EMSD-Mailer-Daemon");

	/* Add the originator as the return_path also */
	rfc822_parse_adrlist(&pRfcEnv->return_path,
			     buf,
			     pLocalDomainAndHost);

	/* Add the recipient */
	pRecip = QU_FIRST(&pControlFileData->recipList);
	rfc822_parse_adrlist(&pRfcEnv->to,
			     pRecip->recipient,
			     pLocalDomainAndHost);

	/* Convert the Deliver parameters to RFC-822 */
	if ((rc =
	     emsdd_convertEmsdDeliverToRfc822(pDeliverArg,
					    pRfcEnv,
					    extraHeaders)) !=
	    Success)
	{
	    emsdd_freeDeliverArg(pDeliverArg);
	    emsdd_freeReportDelivery(pReportDelivery);
	    mail_free_envelope(&pRfcEnv);

	    return FAIL_RC(rc,
			   ("fromEmsdToRfc822: convert Deliver to RFC"));
	}

	/*
	 * Add the body of the message
	 */

	/* Add the header telling 'em what this is */
	sprintf(p = buf,
		"This is a Delivery/Non-Delivery report for:\n\n"
		"\tMessage Id: %.*s\n\n",
		STR_getStringLength(pReportDelivery->
				    msgIdOfReportedMsg.un.rfc822),
		STR_stringStart(pReportDelivery->
				msgIdOfReportedMsg.un.rfc822));

	p += strlen(p);

	/* Add the per-recipient data */
	for (pPerRecip = pReportDelivery->recipDataList.data,
	     i = pReportDelivery->recipDataList.count;

	     i > 0;

	     i--,
	     pPerRecip++)
	{
	    /* Is this a delivery or non-delivery report? */
	    switch(pPerRecip->report.choice)
	    {
	    case EMSD_DELIV_CHOICE_DELIVERY:
		sprintf(p,
			"Delivery To:\n"
			"\t%.*s\n"
			"\tat %s",
			STR_getStringLength(pPerRecip->
					    recipient.un.rfc822),
			STR_stringStart(pPerRecip->
					recipient.un.rfc822),
			OS_printableDateTime(&pPerRecip->
					     report.un.
					     deliveryReport.
					     messageDeliveryTime));
		p += strlen(p);
		break;

	    case EMSD_DELIV_CHOICE_NONDELIVERY:
		/* Get the non-delivery reason */
		pReason =
		    reasonString(pPerRecip->
				 report.un.nonDeliveryReport.
				 nonDeliveryReason);

		/* Get the non-delivery diagnostic (if specified) */
		if (pPerRecip->
		    report.un.nonDeliveryReport.
		    nonDeliveryDiagnostic.exists)
		{
		    pDiagnostic =
			diagnosticString(pPerRecip->
					 report.un.nonDeliveryReport.
					 nonDeliveryDiagnostic.data);
		}
		else
		{
		    pDiagnostic = NULL;
		}

		/* Get the explanation (if specified) */
		if (pPerRecip->
		    report.un.nonDeliveryReport.
		    explanation.exists)
		{
		    pExplanation = (pPerRecip->
				    report.un.nonDeliveryReport.
				    explanation.data);
		}
		else
		{
		    pExplanation = NULL;
		}

		sprintf(p,
			"Non-Delivery To:\n"
			"\t%.*s\n"
			"\tReason: %s\n",
			STR_getStringLength(pPerRecip->
					    recipient.un.rfc822),
			STR_stringStart(pPerRecip->
					recipient.un.rfc822),
			pReason);

		p += strlen(p);

		if (pDiagnostic != NULL)
		{
		    sprintf(p, "\tDiagnostic: %s\n", pDiagnostic);
		    p += strlen(p);
		}

		if (pExplanation != NULL)
		{
		    sprintf(p, "\tExplanation: %s\n", pExplanation);
		    p += strlen(p);
		}

		sprintf(p, "\n");
		break;
	    }
	}

	/* Obtain an RFC-822 body */
	pRfcBody = mail_newbody();

	/* This'll be a simple text message */
	pRfcBody->type = TYPETEXT;
	pRfcBody->contents.text = (unsigned char *) emsdd_copyString(buf);

	/* We're done with the EMSD structures */
	emsdd_freeDeliverArg(pDeliverArg);
	emsdd_freeReportDelivery(pReportDelivery);

	/* Get a pointer to the recipient list */
	pRecipList = &pControlFileData->recipList;

	/* Allocate an array of pointers to one recipient string */
	if ((ppRecipArray =
	     OS_alloc((1 + 1) * sizeof(char *))) == NULL)
	{
	    mail_free_envelope(&pRfcEnv);
	    mail_free_body(&pRfcBody);

	    return FAIL_RC(ResourceError,
			   ("fromEmsdToRfc822: recip array"));
	}

	/* Assign the one (and only) recipient */
	pp = ppRecipArray;
	*pp++ = QU_FIRST(pRecipList);

	/* Null-terminate the list of recipients */
	*pp = NULL;

	/* Append the message to the mailbox */
	if ((rc = deliverToEmsdUser(ppRecipArray,
				   pRfcEnv,
				   extraHeaders,
				   pRfcBody)) != Success)
	{
	    OS_free(ppRecipArray);
	    mail_free_envelope(&pRfcEnv);
	    mail_free_body(&pRfcBody);

	    return FAIL_RC(rc,
			   ("fromEmsdToRfc822: open deliverToEmsdUser"));
	}
	
	/* We're done with the RFC-822 message structures now. */
	OS_free(ppRecipArray);
	mail_free_envelope(&pRfcEnv);
	mail_free_body(&pRfcBody);

	break;
	
    case EMSD_ContentType_Ipm95:
	/* Allocate an EMSD Message structure */
	if ((rc = emsdd_allocMessage(&pIpm)) != Success)
	{
	    BUF_free(hBuf);
	    BUF_free(pDeliverArg->hContents);
	    emsdd_freeDeliverArg(pDeliverArg);

	    return FAIL_RC(rc, ("fromEmsdToRfc822: alloc message"));
	}

	/* Get the length of the Contents PDU */
	pduLength = BUF_getBufferLength(hBuf);

	/* Parse the IPM PDU */
	if ((rc = ASN_parse(ASN_EncodingRules_Basic,
			    &compIpm,
			    pDeliverArg->hContents,
			    pIpm, &pduLength)) != Success)
	{
	    BUF_free(hBuf);
	    BUF_free(pDeliverArg->hContents);
	    emsdd_freeDeliverArg(pDeliverArg);
	    emsdd_freeMessage(pIpm);

	    return FAIL_RC(rc, ("fromEmsdToRfc822: parse ipm"));
	}

	/* We're done with the PDU buffers now. */
	BUF_free(hBuf);
	BUF_free(pDeliverArg->hContents);
	pDeliverArg->hContents = NULL;

	/* Obtain an RFC-822 envelope pointer */
	pRfcEnv = mail_newenvelope();

	/* Extra headers are appended to the buffer */
	*extraHeaders = '\0';

	/* Convert the IPM to RFC-822 */
	if ((rc =
	     emsdd_convertEmsdIpmToRfc822(pIpm,
					pRfcEnv,
					extraHeaders)) != Success)
	{
	    emsdd_freeDeliverArg(pDeliverArg);
	    emsdd_freeMessage(pIpm);
	    mail_free_envelope(&pRfcEnv);

	    return FAIL_RC(rc, ("fromEmsdToRfc822: convert IPM to RFC"));
	}

	/* Convert the Deliver parameters to RFC-822 */
	if ((rc =
	     emsdd_convertEmsdDeliverToRfc822(pDeliverArg,
					    pRfcEnv,
					    extraHeaders)) !=
	    Success)
	{
	    emsdd_freeDeliverArg(pDeliverArg);
	    emsdd_freeMessage(pIpm);
	    mail_free_envelope(&pRfcEnv);

	    return FAIL_RC(rc,
			   ("fromEmsdToRfc822: convert Deliver to RFC"));
	}

	/* Attach the body text to the RFC-822 message */
	if (pIpm->body.exists)
	{
	    /* Allocate a body structure */
	    pRfcBody = mail_newbody();

	    /* Set the data type */
	    pRfcBody->type = TYPETEXT;

	    /* Point to our data */
	    pRfcBody->contents.text =
		emsdd_copyString(STR_stringStart(pIpm->body.data.bodyText));
	}

	/* We're done with the EMSD structures */
	emsdd_freeDeliverArg(pDeliverArg);
	emsdd_freeMessage(pIpm);

	/* Get a pointer to the recipient list */
	pRecipList = &pControlFileData->recipList;

	/* Build a list of recipients.  First, how many are there? */
	for (pRecip = QU_FIRST(pRecipList), i = 0;
	     ! QU_EQUAL(pRecip, pRecipList);
	     pRecip = QU_NEXT(pRecip))
	{
	    ++i;
	}

	/* Allocate an array of pointers to recipient strings */
	if ((ppRecipArray =
	     OS_alloc((i + 1) * sizeof(char *))) == NULL)
	{
	    mail_free_envelope(&pRfcEnv);
	    mail_free_body(&pRfcBody);

	    return FAIL_RC(ResourceError,
			   ("fromEmsdToRfc822: recip array"));
	}

	/* Create the list of recipients */
	for (pRecip = QU_FIRST(pRecipList), pp = ppRecipArray;
	     ! QU_EQUAL(pRecip, pRecipList);
	     pRecip = QU_NEXT(pRecip), ++pp)
	{
	    *pp = pRecip->recipient;
	}

	/* Null-terminate the list of recipients */
	*pp = NULL;

	/* Append the message to the mailbox */
	if ((rc = deliverToEmsdUser(ppRecipArray,
				   pRfcEnv,
				   extraHeaders,
				   pRfcBody)) != Success)
	{
	    OS_free(ppRecipArray);
	    mail_free_envelope(&pRfcEnv);
	    mail_free_body(&pRfcBody);

	    return FAIL_RC(rc,
			   ("fromEmsdToRfc822: open deliverToEmsdUser"));
	}
	
	/* We're done with the RFC-822 message structures now. */
	OS_free(ppRecipArray);
	mail_free_envelope(&pRfcEnv);
	mail_free_body(&pRfcBody);

	break;
	
    case EMSD_ContentType_VoiceMessage:
	/* We're done with the PDU buffers now */
	BUF_free(hBuf);
	BUF_free(pDeliverArg->hContents);
	emsdd_freeDeliverArg(pDeliverArg);
	break;
	
    default:
	/* We're done with the PDU buffers now */
	BUF_free(hBuf);
	BUF_free(pDeliverArg->hContents);
	emsdd_freeDeliverArg(pDeliverArg);
	break;
    }

    return Success;
}



static OS_Boolean
dirFilter(const char * pName)
{
    /* Include only files which begin with "C_" */
    if ((pName[0] == 'C' || pName[0] == 'c') && (pName[1] == '_'))
    {
	/* Include this file. */
	return 1;
    }

    return 0;
}


static void
nonDelToRfc822(char * pControlFileName,
	       char * pDataFileName,
	       char * pOriginator,
	       EMSDD_RecipList * pRecipList)
{
    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_MESSAGE_FLOW,
	      "Submission or non-delivery failure attempting to "
	      "send to MTS,\n\t control file (%s)\n",
	      pControlFileName));
}


static void
nonDelToEmsd(char * pControlFileName,
	    char * pDataFileName,
	    char * pOriginator,
	    EMSDD_RecipList * pRecipList)
{
    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_MESSAGE_FLOW,
	      "Delivery failure attempting to deliver to local user,\n\t"
	      "control file (%s)\n",
	      pControlFileName));
}


static ReturnCode
deliverToEmsdUser(char ** ppRecipients,
		 ENVELOPE * pRfcEnv,
		 char * pExtraHeaders,
		 BODY * pRfcBody)
{
    char *	    p1;
    char *	    p2;
    char *	    pMailBox;
    char	    buf[1024];
    FILE *	    hFolder;
    time_t	    currentTime;
    ReturnCode	    rc;

    for (; *ppRecipients; ppRecipients++)
    {
	/* Get the user's mailbox name */
	if ((rc = getMailboxName(*ppRecipients, &pMailBox)) != Success)
	{
	    return FAIL_RC(rc, ("deliverToEmsdUser: unknown user"));
	}

	TM_TRACE((emsdd_hModCB, EMSDD_TRACE_MESSAGE_FLOW,
		  "DELIVER to EMSD user mailbox: %s", pMailBox));

	if ((hFolder = OS_fileOpen(pMailBox, "at")) == NULL)
	{
	    return FAIL_RC(OS_RC_FileOpen,
			   ("deliverToEmsdUser: open folder"));
	}

	/* compose "From <return-path> <date>" line */
	strcpy (buf,"From ");

	if (pRfcEnv->return_path != NULL)
	{
	    rfc822_address(buf, pRfcEnv->return_path);
	}
	else if (pRfcEnv->sender != NULL)
	{
	    rfc822_address(buf, pRfcEnv->sender);
	}
	else
	{
	    rfc822_address(buf, pRfcEnv->from);
	}

	/* Add the from line and the time to the message */
	currentTime = time((time_t *) NULL);
	strcat(buf, " ");
	strcat(buf, ctime(&currentTime));

	/* Get rid of the trailing '\n'; we'll put in "\n" later */
	buf[strlen(buf) - 1] = '\0';

	/* Output the From line */
	OS_filePutString(buf, hFolder);
	OS_filePutString("\n", hFolder);

	/* build RFC-822 header */
	p1 = buf;
	*p1 = '\0';

	rfc822_header_line (&p1, "Date",
			    pRfcEnv, pRfcEnv->date);
	rfc822_address_line (&p1, "From",
			     pRfcEnv, pRfcEnv->from);
	rfc822_address_line (&p1, "Sender",
			     pRfcEnv, pRfcEnv->sender);
	rfc822_address_line (&p1, "Reply-To",
			     pRfcEnv, pRfcEnv->reply_to);
	rfc822_header_line (&p1, "Subject",
			    pRfcEnv, pRfcEnv->subject);
	rfc822_address_line (&p1, "To",
			     pRfcEnv, pRfcEnv->to);
	rfc822_address_line (&p1, "cc",
			     pRfcEnv, pRfcEnv->cc);
	rfc822_header_line (&p1, "In-Reply-To",
			    pRfcEnv, pRfcEnv->in_reply_to);
	rfc822_header_line (&p1, "Message-ID",
			    pRfcEnv, pRfcEnv->message_id);
	
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
    
	/* write the extra headers */
	OS_filePutString(pExtraHeaders, hFolder);

	/* put in the header-terminating "\n" */
	OS_filePutString("\n", hFolder);

	/* Strip carriage returns from the body */
	for (p1 = p2 = (char *) pRfcBody->contents.text; *p2 != '\0'; )
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
	OS_filePutString((char *) pRfcBody->contents.text, hFolder);

	/* write two trailing new lines to make it proper folder format */
	OS_filePutString("\n\n", hFolder);

	/* all done! */
	OS_fileClose(hFolder);
    }

    return Success;
}


static OS_Boolean
emsdAuaEqual(char * pAua1, char * pAua2)
{
    char *	    p;
    char	    aua1[256];
    char	    aua2[256];

    /*
     * Get the "relevent" part of the AUA.  Ignore white space and
     * surrounding angle brackets.  Also, ignore the domain part, if
     * it exists.
     */

    /* First, skip the comment portion of the first AUA, if it exists */
    if ((p = strchr(pAua1, '<')) != NULL)
    {
	/* Found an angle bracket denoting end of comment.  Point past it. */
	pAua1 = p + 1;
    }
    else
    {
	/* There was no comment portion.  Skip white space instead. */
	while (isspace(*pAua1))
	{
	    ++pAua1;
	}
    }

    /* Copy until we get to a '>', '@', or the null terminator */
    for (p = aua1; *pAua1 != '>' && *pAua1 != '@' && *pAua1 != '\0'; pAua1++)
    {
	*p++ = *pAua1;
    }

    /* Null terminate here. */
    *p = '\0';
    
    /* Now, skip the comment portion of the second AUA, if it exists */
    if ((p = strchr(pAua1, '<')) != NULL)
    {
	/* Found an angle bracket denoting end of comment.  Point past it. */
	pAua1 = p + 1;
    }
    else
    {
	/* There was no comment portion.  Skip white space instead. */
	while (isspace(*pAua1))
	{
	    ++pAua1;
	}
    }

    /* Copy until we get to a '>', '@', or the null terminator */
    for (p = aua2; *pAua2 != '>' && *pAua2 != '@' && *pAua2 != '\0'; pAua2++)
    {
	*p++ = *pAua2;
    }

    /* Null terminate here. */
    *p = '\0';
    
    /* Now, are the two the same? */
    return (OS_strcasecmp(aua1, aua2) == 0);
}

static ReturnCode
getMailboxName(char * pRecipient,
	       char ** ppMailBox)
{
    User *	    pUser;

    for (pUser = QU_FIRST(&localUsers);
	 ! QU_EQUAL(pUser, &localUsers);
	 pUser = QU_NEXT(pUser))
    {
	if (emsdAuaEqual(pRecipient, pUser->pEmsdAddress))
	{
	    *ppMailBox = pUser->pMailBox;
	    return Success;
	}
    }

    return Fail;
}


static OS_Boolean
isLocalUser(char * pRecipient)
{
    User *	    pUser;

    for (pUser = QU_FIRST(&localUsers);
	 ! QU_EQUAL(pUser, &localUsers);
	 pUser = QU_NEXT(pUser))
    {
	if (emsdAuaEqual(pRecipient, pUser->pEmsdAddress))
	{
	    return TRUE;
	}
    }

    return FALSE;
}


static ReturnCode
checkTerminate(void * hTimer,
	       void * hUserData1,
	       void * hUserData2)
{
    FILE *	    hStop;

    if ((hStop = OS_fileOpen("/tmp/emsd-uad.stop", "r")) != NULL)
    {
	OS_fileClose(hStop);
	OS_deleteFile("/tmp/emsd-uad.stop");
	TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT,
		  "%s: Stop-file found.  Exiting.\n",
		  __applicationName));
	exit(0);
    }

    /* Arrange to be called again to re-check for termination */
    if (TMR_start(TMR_SECONDS(10),
		  NULL,
		  NULL,
		  checkTerminate,
		  NULL) != Success)
    {
	/*
	 * If we couldn't create a timer, other bad things will
	 * happen too, since timers are required for enqueuing
	 * messages.  We'll just exit now.
	 */
	exit(1);
    }

    return Success;
}




static char *
reasonString(EMSD_NonDeliveryReason reason)
{
    static char *	reasonStrings[] =
    {
	"Transfer Failure",
	"Unable To Transfer",
	"Restricted Delivery"
    };

    return reasonStrings[reason];
}


static char *
diagnosticString(EMSD_NonDeliveryDiagnostic diagnostic)
{
    static char *	diagnosticStrings[] =
    {
	"Congestion",
	"Loop Detected",
	"Recipient Unavailable",
	"Maximum Time Expired",
	"Content Too Long",
	"Size Constraint Violated",
	"Protocol Violation",
	"Content Type Not Supported",
	"Too Many Recipients",
	"Line Too Long",
	"Unrecognized Address",
	"Recipient Unknown",
	"Recipient Refused To Accept",
	"Unable To Complete Transfer",
	"Transfer Attempts Limit Reached",
    };

    return diagnosticStrings[diagnostic];
}


static ReturnCode
issueInvoke(EMSDD_RecipList * pRecipList,
	    void * hInvokeData)
{
    char * 			p;
    unsigned char * 		pMemStart;
    unsigned char * 		pMem;
    void * 			hBuf;
    OS_Uint16 			len;
    OS_Uint32 			pduLength;
    STR_String 			hString;
    ReturnCode 			rc;
    ESRO_RetVal 		retval;
    ESRO_Event 			ev;
    ESRO_InvokeId 		invokeId;
    EMSDD_Recipient * 		pRecip;
    EMSD_ErrorRequest	 	errorReq;
    static EMSDD_NetworkAddress 	netAddr;
    static OS_Boolean		bNetAddrInitialized = FALSE;
    unsigned char               dupeId = 0;

    TM_CALL(emsdd_hModCB, EMSDD_TRACE_PDU, BUF_dump, hInvokeData, "Invoker");

    /* Allocate contiguous memory the size of our buffer contents */
    /* plus one for the dupe detection byte */
    if ((pMemStart = OS_alloc(BUF_getBufferLength(hInvokeData) + 1)) == NULL)
    {
	return FAIL_RC(ResourceError, ("issueInvoke: allocate buffer"));
    }
    pMemStart[0] = dupeId++;		/* sequence id cycles, starting at zero */

    /* For each buffer segment... */
    for (pMem = pMemStart + 1; ; pMem += len)
    {
	/* ... get the segment data, ... */
	len = 0;
	if ((rc = BUF_getChunk(hInvokeData, &len, (unsigned char **) &p)) != Success)
	{
	    if (rc == BUF_RC_BufferExhausted)
	    {
		/* Normal loop exit; no more segments. */
		break;
	    }
	    
	    OS_free(pMemStart);
	    return FAIL_RC(rc, ("issueInvoke: BUF_getChunk"));
	}

	/* ... and copy it into our contiguous memory. */
	OS_copy(pMem, (unsigned char *) p, len);
    }

    /* Get the address of the EMSD-MTS */
    if (! bNetAddrInitialized)
    {
	int		a;
	int		b;
	int		c;
	int		d;
	unsigned char  	pUserTsap[] = { 0x07, 0xd2 };

	/* Set the MTS RSAP Selector */
	netAddr.remoteRsapSelector = EMSDD_Rsap_Emsdd_Performer;

	/* Set the MTS TSAP Selector */
	netAddr.remoteTsapSelector.len = 2;
	OS_copy(netAddr.remoteTsapSelector.addr, pUserTsap, 2);

	/* Set the MTS NSAP (IP) Address from the configuration data */
	netAddr.remoteNsapAddress.len =
	    sscanf(config.pMtsNsap, "%d.%d.%d.%d", &a, &b, &c, &d);
	netAddr.remoteNsapAddress.addr[0] = a;
	netAddr.remoteNsapAddress.addr[1] = b;
	netAddr.remoteNsapAddress.addr[2] = c;
	netAddr.remoteNsapAddress.addr[3] = d;

	bNetAddrInitialized = TRUE;
    }
    
    TM_TRACE((NULL, EMSD_TRACE_DETAIL, "ESRO_invokeReq..."));
    retval = ESRO_invokeReq(&invokeId,
			    hLocalSap,
			    netAddr.remoteRsapSelector,
			    &netAddr.remoteTsapSelector,
			    &netAddr.remoteNsapAddress,
			    EMSD_Operation_Submission_1_1,
			    ESRO_EncodingBER,
			    pMem - pMemStart,
			    pMemStart);

    TM_TRACE((NULL, EMSD_TRACE_DETAIL, "...EMSD_invokeReq returned %d", rc));
    if (retval < 0)
    {
	/*
	 * The invoke failed.  We'll call this a transient
	 * error for now.
	 */
        TM_TRACE((NULL, EMSD_TRACE_DETAIL, "Invoke failed, rc = %d", rc));
	/* For each recipient... */
	for (pRecip = QU_FIRST(pRecipList);
	     ! QU_EQUAL(pRecip, pRecipList);
	     pRecip = QU_NEXT(pRecip))
	{
	    /* ... set the recipient status. */
	    pRecip->status = EMSDD_Status_TransientError;
	}
    }
    else
    {
	/*
	 * The invoke succeeded.  Now, wait for a response.
	 */
	do
	{
	    errno = 0;
		    
	    if ((retval = ESRO_getEvent(hLocalSap, &ev, TRUE)) < 0)
	    {
		if (errno != EAGAIN)
		{
		    /*
		     * This should never occur.  Just call it a
		     * transient error.
		     */
		    /* For each recipient... */
		    for (pRecip = QU_FIRST(pRecipList);
			 ! QU_EQUAL(pRecip, pRecipList);
			 pRecip = QU_NEXT(pRecip))
		    {
			/* ... set the recipient status. */
			pRecip->status = EMSDD_Status_TransientError;
		    }

		    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT,
			      "%s: ESRO_getEvent() got errno = %d\n",
			      __applicationName, errno));
		    goto done;
		}
	    }
	} while (retval < 0);
		
	/* What type of response did we get? */
	TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
		 "Response is event type %d", ev.type));
	switch(ev.type)
	{
	case ESRO_RESULTIND:
	    /* Nothing to do.  Successfully delivered. */
	    break;

	case ESRO_ERRORIND:
	    /* Get the length of the data */
	    pduLength = ev.un.errorInd.len;

	    /* Allocate a buffer for the error data */
	    if ((rc = BUF_alloc(1, &hBuf)) != Success)
	    {
		break;
	    }

	    /*
	     * Allocate a string long enough for
	     * this segment.
	     */
	    if ((rc = STR_alloc(pduLength, &hString)) != Success)
	    {
		BUF_free(hBuf);
		break;
	    }

	    /* Copy the error data to the string */
	    OS_copy(STR_stringStart(hString), ev.un.errorInd.data, pduLength);

	    /* Set the new length of this string */
	    if ((rc = STR_setStringLength(hString, pduLength)) != Success)
	    {
		STR_free(hString);
		BUF_free(hBuf);
		break;
	    }

	    /* Attach this string to the buffer */
	    if ((rc = BUF_appendChunk(hBuf, hString)) != Success)
	    {
		STR_free(hString);
		BUF_free(hBuf);
		break;
	    }

	    TM_CALL(emsdd_hModCB, EMSDD_TRACE_PDU, BUF_dump, hBuf,
		    "Invoker Error Indication");

	    /* Parse the error indication */
	    if ((rc =
		 ASN_parse(ev.un.errorInd.encodingType,
			   &compErrorRequest,
			   hBuf,
			   &errorReq,
			   &pduLength)) !=
		Success)
	    {
		break;
	    }

	    /* For each recipient... */
	    for (pRecip = QU_FIRST(pRecipList);
		 ! QU_EQUAL(pRecip, pRecipList);
		 pRecip = QU_NEXT(pRecip))
	    {
		/* ... set the recipient status. */
		pRecip->status = EMSDD_Status_TransientError;

		/* Create an error string for 'em */
#undef e
#define	e(s)	EMSD_Error_##s
		switch(errorReq.error)
		{
		case e(ProtocolVersionNotRecognized):
		    p = "504 "
			"Protocol Version Not Recognized";
		    break;

		case e(MessageIdentifierInvalid):
		    p = "504 "
			"Message Identifier Invalid";
		    break;

		case e(ProtocolViolation):
		    p = "500 "
			"Protocol Violation";
		    break;

		case e(SecurityError):
		    p = "553 "
			"Security Error";
		    break;

		case e(SubmissionControlViolated):
		    p = "552 "
			"Submission Control Violated";
		    break;

		case e(DeliveryControlViolated):
		    p = "552 "
			"Delivery Control Violated";
		    break;

		case e(ResourceError):
		    p = "552 "
			"Resource Error";
		    break;

		default:
		    p = "500 "
			"Unknown error";
		    break;
		}
#undef e

		if ((pRecip->pErrorString =
		     OS_alloc(strlen((char *) p) + 1)) != NULL)
		{
		    /* Copy the error string */
		    strcpy(pRecip->pErrorString, (char *) p);
		}
	    }

	    /* We're done with the buffer */
	    BUF_free(hBuf);

	    break;

	case ESRO_FAILUREIND:
	default:
	    /* For each recipient... */
	    for (pRecip = QU_FIRST(pRecipList);
		 ! QU_EQUAL(pRecip, pRecipList);
		 pRecip = QU_NEXT(pRecip))
	    {
		/* ... set the recipient status. */
		pRecip->status = EMSDD_Status_TransientError;
	    }
	    break;
	}
    }

  done:

    /* We're all done with the buffer memory now */
    OS_free(pMemStart);

    return Success;
}

static ReturnCode
writeEmsdPdu(QU_Head * pRecipList,
	    void * hBuf)
{
    unsigned char * 	    p;
    char 		    dataFileName[OS_MAX_FILENAME_LEN];
    char 		    tempControlFileName[OS_MAX_FILENAME_LEN];
    FILE * 		    hFile;
    OS_Uint16 		    len;
    OS_Uint32 		    retryTimeJulian;
    ReturnCode 		    rc;
    EMSDD_Recipient *	    pRecip;
    EMSDD_ControlFileData    cfData;

    /* Create a prototype for the data file name */
    sprintf(dataFileName, "%s%cD_XXXXXX",
	    config.fromEmsdToRfc822.pNewMessageDirectory,
	    *OS_DIR_PATH_SEPARATOR);

    /* Create the data file */
    if ((rc = OS_uniqueName(dataFileName)) != Success)
    {
	return FAIL_RC(rc, ("writeEmsdPdu: unique data file name"));
    }

    /* Open the data file */
    if ((hFile = OS_fileOpen(dataFileName, "wb")) == NULL)
    {
	OS_deleteFile(dataFileName);
	return FAIL_RC(rc, ("writeEmsdPdu: open data file %s",
			    dataFileName));
    }

    /* Reset the internal buffer pointers in this buffer */
    BUF_resetParse(hBuf);

    for (;;)
    {
	len = 0;
	if ((rc = BUF_getChunk(hBuf, &len, &p)) != Success)
	{
	    if (rc == BUF_RC_BufferExhausted)
	    {
		break;
	    }
	    
	    OS_fileClose(hFile);
	    OS_deleteFile(dataFileName);

	    return FAIL_RC(rc, ("writeEmsdPdu: BUF_getChunk"));
	}

	OS_fileWrite(p, 1, len, hFile);
    }

    /* All done writing the the data file. */
    OS_fileClose(hFile);

    /* Create a prototype for the TEMPORARY control file name */
    sprintf(tempControlFileName, "%s%cT_XXXXXX",
	    config.fromEmsdToRfc822.pNewMessageDirectory,
	    *OS_DIR_PATH_SEPARATOR);

    /* Create the temporary control file */
    if ((rc = OS_uniqueName(tempControlFileName)) != Success)
    {
	OS_deleteFile(dataFileName);
	return FAIL_RC(rc, ("writeEmsdPdu: unique temp control file"));
    }

    /* Get the current time, in date/struct format */
    if ((rc = OS_currentDateTime(NULL,
				 &retryTimeJulian)) != Success)
    {
	OS_deleteFile(dataFileName);
	return FAIL_RC(rc, ("writeEmsdPdu: get current date/time"));
    }

    /* Specify the control file data */
    cfData.retries = 0;
    cfData.nextRetryTime = retryTimeJulian;
    strcpy(cfData.dataFileName, dataFileName);
    *cfData.originator = '\0';	/* we dont' know the originator */


    /* Initialize the recipient list */
    QU_INIT(&cfData.recipList);

    /* Copy the existing recipient list */
    for (pRecip = QU_FIRST(pRecipList);
	 ! QU_EQUAL(pRecip, pRecipList);
	 pRecip = QU_FIRST(pRecipList))
    {
	QU_REMOVE(pRecip);
	QU_INSERT(pRecip, &cfData.recipList);
    }

    /* Write the control file */
    rc = emsdd_writeControlFile(tempControlFileName, &cfData);

    /* Put the recipient list back where it was */
    for (pRecip = QU_FIRST(&cfData.recipList);
	 ! QU_EQUAL(pRecip, &cfData.recipList);
	 pRecip = QU_FIRST(&cfData.recipList))
    {
	QU_REMOVE(pRecip);
	QU_INSERT(pRecip, pRecipList);
    }

    /* We're all done with the control file data */
    emsdd_freeControlFileData(&cfData);

    /* Did we successfully write the control file? */
    if (rc == Success)
    {
	/* Yup.  */
	return Success;
    }

    /* We had a problem.  Clean up and let 'em know */
    OS_deleteFile(dataFileName);
    OS_deleteFile(tempControlFileName);
    return FAIL_RC(rc, ("writeEmsdPdu: write control file"));
}


static void
esrosListener()
{
    int				i;
    OS_Uint32			len;
    OS_Uint32			operationValue;
    OS_Uint32			errorVal;
    ESRO_RetVal 		retval;
    ESRO_Event 			ev;
    ReturnCode 			rc = Success;
    STR_String	    		hString = NULL;
    void *	    		hBuf = NULL;
    void *	    		hFormattedArg = NULL;
    void *			hListener = NULL;
    EMSD_Message *		pIpm = NULL;
    EMSD_ReportDelivery *        pRpt = NULL;
    EMSD_DeliverArgument *	pDeliverArg = NULL;
    EMSD_RecipientData *		pAddr;
    EMSD_PerRecipReportData *	pPerRecip;
    EMSDD_Recipient *		pRecip;
    ESRO_SapDesc		hSap;
    OS_Uint32			pduLength;
    OS_Boolean			bComplete;
    char			tmp[512];
    QU_Head			recipQHead;

    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, "Listener() entered"));

    /* Initialize ourselves as a listener */
    if ((retval = emsdd_listenerInit(&hListener)) != Success)
    {
	sprintf(errbuf,
		"ESROS Listener could not initialize as a listener.\n");
	EH_fatal(errbuf);
    }

    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
 	     "Activating SAP %d", (int)EMSDD_Rsap_Etwpd_Performer));
    /* Activate our ESROS service access point */
    if ((retval = ESRO_sapBind(&hSap, EMSDD_Rsap_Etwpd_Performer, ESRO_3Way)) < 0)
    {
        TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
		 "sapBind failed: retval=%d", retval));
	sprintf(errbuf,
		"ESROS Listener could not activate "
		"Service Access Point.\n");
	EH_fatal(errbuf);
    }
    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, "sapBind OK"));

    /* Initialize our recipient queue head */
    QU_INIT(&recipQHead);

    for (;;)
    {
        TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, "Waiting for ESROS event"));
	do
	{
	    errno = 0;
		    
	    if ((retval = ESRO_getEvent(hSap, &ev, 1)) < 0)
	    {
	        TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
			 "  ESRO_getEvent returned %d", retval));
		if (errno != EAGAIN)
		{
		    /*
		     * This should never occur.
		     */
		    sprintf(errbuf,
			    "ESROS Listener got error from ESRO_getEvent: "
			    "%d\n",
			    retval);
		    EH_problem(errbuf);
		    goto freeItAll;
		}
		TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, ""));
	    }
	    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
		     "ESRO_getEvent() got event %d", ev.type));
	} while (retval < 0);
		
	TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
		 "Listener got event type %d", ev.type));
	switch(ev.type)
	{
	case ESRO_INVOKEIND:
	    TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
		     "Listener got event type %d", ev.type));
	    /* Handle possible duplicate detection byte */
	    DUP_invokeIn (&ev);

	    /* Attach the data memory to one of our string handles */
	    if ((rc = STR_attachString(ev.un.invokeInd.len,
				       ev.un.invokeInd.len,
				       ev.un.invokeInd.data,
				       FALSE,
				       &hString)) != Success)
	    {
		rc = ResourceError;
		TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
			 "STR_attachString failure for INVOKE.ind"));
		goto freeItAll;
	    }

	    /* Allocate a buffer to which we'll attach this string */
	    if ((rc = BUF_alloc(0, &hBuf)) != Success)
	    {
		TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
			 "Alloc failure INVOKE.ind"));
		goto freeItAll;
	    }

	    /* Attach the string to a buffer so we can parse it */
	    if ((rc = BUF_appendChunk(hBuf, hString)) != Success)
	    {
		TM_TRACE((emsdd_hModCB, EMSDD_TRACE_INIT, 
			 "BUF_appendChunk error for INVOKE.ind"));
		goto freeItAll;
	    }

	    TM_CALL(emsdd_hModCB, EMSDD_TRACE_PDU, BUF_dump, hBuf, "Listener");

	    /*
	     * Create our internal rendition of the operation value, which
	     * includes the protocol version number in the high-order 16 bits.
	     */
	    operationValue =
		EMSD_OP_PROTO(ev.un.invokeInd.operationValue,
			     EMSD_PROTOCOL_VERSION_MAJOR,
			     EMSD_PROTOCOL_VERSION_MINOR);

	    /* See what operation is being requested */
	    switch(operationValue)
	    {
	    case EMSD_Operation_Delivery_1_1:
		/* Allocate an EMSD Deliver Argument structure */
		if ((rc =
		     emsdd_allocDeliverArg(&pDeliverArg)) != Success)
		{
		    goto freeItAll;
		}
		
		/* Specify the maximum length to parse */
		pduLength = BUF_getBufferLength(hBuf);

		/* Parse the EMSD PDU */
		if ((rc = ASN_parse(ev.un.invokeInd.encodingType,
				    &compDeliverArg,
				    hBuf, pDeliverArg,
				    &pduLength)) != Success)
		{
		    goto freeItAll;
		}

		/*
		 * If reassembly is required, and we haven't yet
		 * received the last segment of the message, go free
		 * everything and send back the Result so we can get
		 * the next segment.  The reasssemble function
		 * maintains its own pointer to the data, so we're
		 * allowed to free ours.
		 */
		if (((rc = emsdd_reassemble(operationValue,
					   (void **) &pDeliverArg,
					   hListener,
					   EMSDD_SEGMENTATION_TIMER,
					   &bComplete)) != Success) ||
		    ! bComplete)
		{
		    goto freeItAll;
		}

		/* See what type of contents we've received */
		switch(pDeliverArg->contentType)
		{
		case EMSD_ContentType_Probe:
		    rc = UnsupportedOption;
		    goto freeItAll;
	
		case EMSD_ContentType_DeliveryReport:
		    /* Allocate an EMSD Delivery Report structure */
		    if ((rc = emsdd_allocReportDelivery(&pRpt)) != Success)
		    {
			goto freeItAll;
		    }

		    /* Get the length of the Contents PDU */
		    pduLength = BUF_getBufferLength(pDeliverArg->hContents);

		    /* Make sure we can parse the Report PDU */
		    if ((rc = ASN_parse(ASN_EncodingRules_Basic,
					&compReportDelivery,
					pDeliverArg->hContents,
					pRpt, &pduLength)) !=
			Success)
		    {
			goto freeItAll;
		    }

		    /*
		     * The message was parsable.  Build the
		     * recipient list.
		     */
		    for (i = pRpt->recipDataList.count,
			 pPerRecip = pRpt->recipDataList.data;

			 i > 0;

			 i--, pPerRecip++)
		    {
			/* What format is the address in? */
			if (pPerRecip->recipient.choice ==
			    EMSD_ORADDRESS_CHOICE_LOCAL)
			{
			    /*
			     * Create an RFC-822 address from the
			     * local-format address.
			     */
			    emsdd_createRfc822AddrFromEmsd(&pPerRecip->recipient,
							 NULL,
							 tmp,
							 sizeof(tmp));
			    len = strlen(tmp) + 1;
			}
			else
			{
			    /* RFC-822 format */
			    len = STR_getStringLength(pPerRecip->
						      recipient.
						      un.rfc822);
			    strncpy(tmp,
				    (char *)
				    STR_stringStart(pPerRecip->
						    recipient.
						    un.rfc822),
				    len);
			    tmp[len++] = '\0';
			}

			/* Allocate a Recipient queue entry */
			if ((pRecip =
			     OS_alloc(sizeof(EMSDD_Recipient) +
				      len)) ==
			    NULL)
			{
			    rc = ResourceError;
			    goto freeItAll;
			}

			/* Initialize the queue entry */
			QU_INIT(pRecip);

			/* Copy the recipient name */
			strcpy(pRecip->recipient, tmp);

			/* Use default protocol version number */
			pRecip->protocolVersionMajor = 0;
			pRecip->protocolVersionMinor = 0;

			/*
			 * Insert this recipient onto the recipient
			 * list.
			 */
			QU_INSERT(pRecip, &recipQHead);
		    }

		    /* Create a file for the PDU */
		    rc = writeEmsdPdu(&recipQHead, hBuf);

		    break;
	
		case EMSD_ContentType_Ipm95:
		    /* Allocate an EMSD Message structure */
		    if ((rc = emsdd_allocMessage(&pIpm)) != Success)
		    {
			goto freeItAll;
		    }

		    /* Get the length of the Contents PDU */
		    pduLength = BUF_getBufferLength(pDeliverArg->hContents);

		    /* Make sure we can parse the IPM PDU */
		    if ((rc = ASN_parse(ASN_EncodingRules_Basic,
					&compIpm,
					pDeliverArg->hContents,
					pIpm, &pduLength)) !=
			Success)
		    {
			goto freeItAll;
		    }

		    /*
		     * The message was parsable.  Build the
		     * recipient list.
		     */
		    for (i = pIpm->heading.recipientDataList.count,
			 pAddr = (pIpm->
				  heading.recipientDataList.data);

			 i > 0;

			 i--, pAddr++)
		    {
			/* What format is the address in? */
			if (pAddr->recipient.choice ==
			    EMSD_ORADDRESS_CHOICE_LOCAL)
			{
			    /*
			     * Create an RFC-822 address from the
			     * local-format address.
			     */
			    emsdd_createRfc822AddrFromEmsd(&pAddr->recipient,
							 NULL,
							 tmp,
							 sizeof(tmp));
			    len = strlen(tmp) + 1;

			    /* If this recipient is local... */
			    if (isLocalUser(tmp))
			    {
				/*
				 * ... we need to deliver to him.
				 */

				/* Allocate a Recipient queue entry */
				if ((pRecip =
				     OS_alloc(sizeof(EMSDD_Recipient) + len)) ==
				    NULL)
				{
				    rc = ResourceError;
				    goto freeItAll;
				}

				/* Initialize the queue entry */
				QU_INIT(pRecip);
				
				/* Copy the recipient name */
				strcpy(pRecip->recipient, tmp);

				/* Use default protocol version number */
				pRecip->protocolVersionMajor = 0;
				pRecip->protocolVersionMinor = 0;

				/*
				 * Insert this recipient onto the recipient
				 * list.
				 */
				QU_INSERT(pRecip, &recipQHead);
			    }
			}
			else
			{
			    /* RFC-822 format */
			    len = STR_getStringLength(pAddr->
						      recipient.un.rfc822);
			    strncpy(tmp,
				    (char *)
				    STR_stringStart(pAddr->
						    recipient.un.rfc822),
				    len);
			    tmp[len++] = '\0';
			}
		    }

		    if ((rc = BUF_alloc(0, &hFormattedArg)) != Success)
		    {
			goto freeItAll;
		    }

		    /* Format the Deliver Argument. */
		    if ((rc = ASN_format(ASN_EncodingRules_Basic,
					 &compDeliverArg,
					 hFormattedArg,
					 pDeliverArg,
					 &len)) != Success)
		    {
			goto freeItAll;
		    }

		    /* Create a file for the PDU */
		    rc = writeEmsdPdu(&recipQHead, hFormattedArg);

		    break;
	
		case EMSD_ContentType_VoiceMessage:
		    /* We're done with the PDU buffers now */
		    rc = UnsupportedOption;
		    goto freeItAll;
	
		default:
		    /* We're done with the PDU buffers now */
		    rc = UnsupportedOption;
		    goto freeItAll;
		}

		break;

	    default:
		rc = UnsupportedOption;
		goto freeItAll;
	    }
	    break;

	case ESRO_RESULTCNF:
	    continue;

	case ESRO_ERRORCNF:
	    rc = UnsupportedOption;
	    break;

	case ESRO_FAILUREIND:
	    rc = UnsupportedOption;
	    break;

	default:
	    rc = UnsupportedOption;
	    break;
	}

    freeItAll:
	/* If we've formatted an argument for writing to a file, free it */
	if (hFormattedArg != NULL)
	{
	    BUF_free(hFormattedArg);
	    hFormattedArg = NULL;
	}

	/* Free the recipient queue.  We're done with it. */
	QU_FREE(&recipQHead);

	/* If there's an IPM structure, free it */
	if (pIpm != NULL)
	{
	    printf ("emsdd_freeMessage(pIpm);\n");
	    pIpm = NULL;
	}

	/* If there's a Report Delivery structure, free it */
	if (pRpt != NULL)
	{
	    emsdd_freeReportDelivery(pRpt);
	    pRpt = NULL;
	}

	/* If there's a Deliver argument, free it */
	if (pDeliverArg != NULL)
	{
	    /* If there's a contents buffer, free it */
	    if (pDeliverArg->hContents != NULL)
	    {
		BUF_free(pDeliverArg->hContents);
		pDeliverArg->hContents = NULL;
	    }

	    printf("emsdd_freeDeliverArg(pDeliverArg);\n");
	    pDeliverArg = NULL;
	}

	/* If there's a buffer, free it */
	if (hBuf != NULL)
	{
	    BUF_free(hBuf);
	    hBuf = NULL;
	}

	/* If we've allocated string memory, free it */
	if (hString != NULL)
	{
	    STR_free(hString);
	    hString = NULL;
	}

	/*
	 * Did we successfully process the event?
	 */
	if (rc == Success)
	{
	    /*
	     * Successful processing of the event.
	     */

	    /* Send back a null-data result */
	    if ((retval = ESRO_resultReq(ev.un.invokeInd.invokeId,
					 ESRO_EncodingBER,
					 0, (Byte *) "")) != Success)
	    {
		/* Ok, it failed.  What do we do about it? */
			/* DO NOTHING. (Sigh) */
	    }

	    continue;
	}

	/*
	 * An error occured.  Generate an error request
	 */
	switch(rc)
	{
	case ResourceError:
	    errorVal = EMSD_Error_ResourceError;
	    break;

	default:
	    errorVal = EMSD_Error_ProtocolViolation;
	    break;
	}

	if ((retval = ESRO_errorReq(ev.un.invokeInd.invokeId,
				    ESRO_EncodingBER,
				    errorVal, 0, NULL)) != Success)
	{
	    /* Couldn't send error request for some reason */
	    continue;
	}

	if (hBuf != NULL)
	{
	    BUF_free(hBuf);
	}
    }
}
