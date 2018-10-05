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

static volatile char *rcs = "$Id: mts.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";

#include "estd.h"
#include "tmr.h"
#include "config.h"
#include "tm.h"
#include "eh.h"
#include "du.h"
#include "profile.h"
#include "mm.h"
#include "sch.h"
#include "fsm.h"
#include "fsmtrans.h"
#include "nnp.h"
#include "mtsua.h"
#include "mts.h"
#include "dup.h"
#include "subpro.h"
#include "msgstore.h"


/*
 * Global Variables
 *
 * All global variables for the MTS go inside of MTS_Globals unless
 * there's a really good reason for putting them elsewhere (like
 * having external requirements on the name).
 */
MTS_Globals		globals;

/* Globals with external requirements that they be named like this. */
char *			__applicationName = "";
DU_Pool *		G_duMainPool;

/*
 * Forward Declarations
 */
FORWARD static ReturnCode
runMM(void * hTimer,
      void * hUserData1,
      void * hUserData2);

ReturnCode
addRetryTime(OS_Uint32 seconds);

FORWARD static OS_Boolean
dirFilter(const char * pName);

FORWARD static ReturnCode
convertRfc822ToEmsdAua(char * pUserName,
		      OS_Uint16 len,
		      OS_Uint32 * pEmsdAddress,
		      ADDRESS * pAddr);


#ifdef OS_TYPE_UNIX
    extern Void G_sigPipe();
    extern Void G_sigTerm();
    extern Void G_sigKill();
#endif

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    extern Void G_sigIntr();
#endif

/*
 * MTS Main Line
 */
void
main(int argc, char *argv[])
{
    ReturnCode			rc		= 0;
    char			buf[32];
    char *			p		= 0;
    char 			pConfigFile[512] = "";
    void *			hTimer		= NULL;
    OS_Uint32 			retryTime	= 0;
    OS_Uint32 			lastTime	= 0;
    MTS_Host *			pHost		= NULL;
    ESRO_RetVal			retval;
    struct ConfigParams *	pConfigParam	= NULL;
    int				c		= 0;
    int                         pass 		= 0;
    char			appName[1024];
    char			dirPath[512];
    
    Char                       *copyrightNotice;
    EXTERN Char                *RELID_getRelidNotice(Void);
    EXTERN Char                *RELID_getRelidNotice(Void);
    EXTERN Char                *RELID_getRelidNotice(Void);
    
    static struct ConfigParams
    {
	char *	    pSectionName;
	char *	    pTypeName;
	char **	    ppValue;
    } configParams[] =
      {
	{
	      "Delivery",
	      "Data Directory",
	      &globals.config.deliver.pDataDir
	  },
	  {
	      "Delivery",
	      "New Message Directory",
	      &globals.config.deliver.pNewMessageDir
	  },
	  {
	      "Delivery",
	      "Spool Directory",
	      &globals.config.deliver.pSpoolDir
	  },
	  {
	      "Submission",
	      "Data Directory",
	      &globals.config.submit.pDataDir
	  },
	  {
	      "Submission",
	      "Spool Directory",
	      &globals.config.submit.pSpoolDir
	  },
	  {
	      "Submission",
	      "Sendmail Path",
	      &globals.config.pSendmailPath
	  },
	  {
	      "Submission",
	      "Sendmail Specify From",
	      &globals.config.pSendmailSpecifyFrom
	  },
	  {
	      "MTS",
	      "Local Host Name",
	      &globals.config.pLocalHostName
	  },
	  {
	      "MTS",
	      "Local NSAP Address",
	      &globals.config.pLocalNsapAddress
	  },
	  {
	      "Subscribers",
	      "Profile File Name",
	      &globals.config.pSubscriberProfileFile
	  }
      };
      
#ifdef OS_TYPE_UNIX
    signal(SIGPIPE, G_sigPipe);
    signal(SIGTERM, G_sigTerm);
    signal(SIGKILL, G_sigKill);
#endif
      
#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    signal(SIGINT, G_sigIntr);
#endif

      fprintf (stderr, "MTS built at %s on %s\n", __TIME__, __DATE__);

      /* Initialize the OS module */
      if (OS_init() != Success)
      {
	  EH_fatal("Could not initialize OS module");
      }
      
      /* Initialize the global variables */
      globals.hPerformerTransDiag = NULL;
      globals.hDeliverTransDiag = NULL;
      QU_INIT(&globals.activeOperations);
      QU_INIT(&DUP_globals.seqList);
      
      strcpy(appName, argv[0]);
      __applicationName = appName;
      
#ifdef TM_ENABLED
      /* Initialize the trace module */
      TM_INIT();
#endif
      
      /* Initialize the MSG module */
      if (msg_init() != Success)
      {
	  EH_problem("Could not initialize MSG module");
      }
 
      /* Initialize the OS memory allocation debug routines */
      OS_allocDebugInit(NULL);
      
      /* Initialize the OS file tracking debug routines */
      OS_fileDebugInit("/tmp/emsdd.files");
      
      /* Initialize Module Management module and register managable objects */
      if ((rc = mts_initModuleManagement(globals.tmpbuf)) != Success)
      {
	  EH_fatal(globals.tmpbuf);
      }
      
    /* Scan past optional arguments to find the configuration file name */
    while ((c = getopt(argc, argv, "T:c:vVp")) != EOF) 
    {
	switch(c) {
	case 'c':
	    strcpy(pConfigFile, optarg);
	    break;

	case 'v':
	case 'V':
	    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
		EH_fatal("main: Get copyright notice failed\n");
	    }
	    fprintf(stdout, "%s\n", copyrightNotice);
	    printf ("MTS built at %s on %s\nRCS ID: %s\n", 
		    __TIME__, __DATE__, rcs);
	    exit(0);
	}
    }

    if (strlen(pConfigFile) == 0) {
   	EH_problem("UA's Configuration file is missing "
		   "(-c option as command line argument)\n"); 
	      sprintf(globals.tmpbuf,
		      "usage: %s [-T <TM option string>] "
		      "-c <mts configuration file name>\n",
		      __applicationName);
	      EH_fatal(globals.tmpbuf);
    }
      
      /* Initialize the config module */
      if ((rc = CONFIG_init()) != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not initialize CONFIG module\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }

      /*
       * Open the configuration file.  Get a handle to the configuration data.
       */
      if ((rc = CONFIG_open(pConfigFile, &globals.hConfig)) != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not open configuration file (%.*s), "
		  "reason 0x%04x\n",
		  __applicationName,
		  (int) (sizeof(globals.tmpbuf) / 2), pConfigFile,
		  rc);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Get the default trace flags from the configuration file */
      for (c = 1, rc = Success; rc == Success; c++)
      {
	  /* Create the parameter type string */
	  sprintf(buf, "Module %d", c);
	  
	  /* Get the next trace parameter */
	  if ((rc = CONFIG_getString(globals.hConfig,
				     "Trace",
				     buf,
				     &p)) == Success)
	  {
	      /* Issue the trace command */
	      TM_SETUP(p);
	  }
      }
      
      optind = 1;
      while ((c = getopt(argc, argv, "T:c:vVp")) != EOF)
      {
	  switch (c)
	  {
	  case 'T':
	      TM_SETUP(optarg);
	      break;

	  case 'c':	
	  case 'v':	
	  case 'V':	
	      break;
	
	  default:
	      sprintf(globals.tmpbuf,
		      "usage: %s [-T <TM option string>] "
		      "-c <mts configuration file name>\n",
		      __applicationName);
	      EH_fatal(globals.tmpbuf);
	  }
      }

      if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
	  EH_fatal("main: Get copyright notice failed!\n");
      }
      fprintf(stdout, "%s\n", argv[0]);
      fprintf(stdout, "%s\n\n", copyrightNotice);
      
      /* Give ourselves a trace handle */
      if (TM_OPEN(globals.hTM, "MTS") == NULL)
      {
	  EH_fatal("Could not open MTS trace");
      }

      TM_TRACE((globals.hTM, MTS_TRACE_INIT, "MTA build date: %s  %s", 
		__TIME__, __DATE__));
      TM_TRACE((globals.hTM, MTS_TRACE_INIT, "MTA version: %s", rcs));
      
      /* Give ourselves a trace handle */
      if (TM_OPEN(DUP_globals.hTM, "DUP_") == NULL)
      {
	  EH_fatal("Could not open DUP_ trace");
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
	  if ((rc = CONFIG_getString(globals.hConfig,
				     pConfigParam->pSectionName,
				     pConfigParam->pTypeName,
				     pConfigParam->ppValue)) != Success)
	  {
	      sprintf(globals.tmpbuf,
		      "%s: Configuration parameter %s/%s not found, reason 0x%04x",
		      __applicationName,
		      pConfigParam->pSectionName,
		      pConfigParam->pTypeName,
		      rc);
	      EH_fatal(globals.tmpbuf);
	  }
	  
	  TM_TRACE((globals.hTM, MTS_TRACE_INIT,
		    "Configuration parameter %s/%s = %s",
		    pConfigParam->pSectionName,
		    pConfigParam->pTypeName,
		    *pConfigParam->ppValue));
      }
      
      /* Get retry times */
      for (c = 1, rc = Success; rc == Success; c++)
      {
	  /* Create the parameter type string */
	  sprintf(buf, "Time %d", c);
	  rc = CONFIG_getNumber(globals.hConfig, "Retry Times", buf, &retryTime);
	  if (rc == Success)
	  {
	      /* Add this time to the list of retries */
	      rc = addRetryTime(retryTime);
	      
	      TM_TRACE((globals.hTM, MTS_TRACE_INIT,
			"Retry time at %lu seconds\n", retryTime));
	  }
      }
      
      /* Initialize the queue of hosts in the EMSD domain */
      QU_INIT(&globals.config.emsdHosts);
      
      /* Get the names of the hosts in the EMSD domain */
      for (c = 1, rc = Success; rc == Success; c++)
      {
	  
	  sprintf(buf, "Host %d", c);	  /* Create the parameter type string */
	  if ((rc = CONFIG_getString(globals.hConfig, "EMSD Hosts", buf, &p)) 
	      == Success)
	  {
	      /* Allocate a Host structure */
	      if ((pHost = OS_alloc(sizeof(MTS_Host) + strlen(p))) == NULL)
	      {
		  sprintf(globals.tmpbuf,
			  "%s: Out of memory allocating Host structure.\n",
			  __applicationName);
		  EH_fatal(globals.tmpbuf);
	      }
	      
	      /* Initialize the queue pointers */
	      QU_INIT(pHost);
	      
	      /* Copy the hostname */
	      strcpy(pHost->pHostName, p);
	      
	      /* Insert this host onto the EMSD Hosts queue */
	      QU_INSERT(pHost, &globals.config.emsdHosts);
	      
	      TM_TRACE((globals.hTM, MTS_TRACE_INIT, 
		       "EMSD Host: %s", pHost->pHostName));
	  }
      };

#ifdef OLD_PROFILE      
      /* Open the profile */
      rc = PROFILE_open(globals.config.pSubscriberProfileFile, 
			&globals.hProfile);
      if (rc != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not open Originator/Recipient Profile %s; "
		  "reason 0x%04x",
		  __applicationName,
		  globals.config.pSubscriberProfileFile, rc);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Specify the subscriber profile file and attributes that we'll use */
      rc = mts_initDeviceProfile(globals.config.pSubscriberProfileFile);
      if (rc != Success)
      {
	  sprintf(globals.tmpbuf, "%s: Could not initialize device profile '%s', ",
		  __applicationName, globals.config.pSubscriberProfileFile);
	  EH_fatal(globals.tmpbuf);
      }
#endif /* OLD_PROFILE */

      /*** call the subscriber profile init routine ***/
      SUB_init (globals.config.pSubscriberProfileFile);

      /* Initialize the TMR module */
      TMR_init(0, NUMBER_OF_TIMERS);	/* no preallocation; 1000 ms / tick */
      
      /* Initialize the SCH module */
      SCH_init(MAX_SCH_TASKS);		/* arbitrary maximum number of tasks */
      
      /* Initialize the ASN Module */
      if ((rc = ASN_init()) != Success)
      {
	  sprintf(globals.tmpbuf, "%s: Could not initialize ASN module\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Build the ASN compile trees for all of the EMSD protocols */
      if ((rc = mtsua_initCompileTrees(globals.tmpbuf)) != Success)
      {
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Build the buffer pool that the ESRO_CB_ functions require */
      G_duMainPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS);

      /* Initialize the ESROS module */
      ESRO_CB_init(pConfigFile);
      
      /* Activate our 3-way Invoker service access point */
      if ((retval = ESRO_CB_sapBind(&globals.sap.hInvoker3Way,
				    MTSUA_Rsap_Mts_Invoker_3Way,
				    ESRO_3Way,
				    mts_invokeIndication, 
				    mts_resultIndication, 
				    mts_errorIndication,
				    mts_resultConfirm, 
				    mts_errorConfirm,
				    mts_failureIndication)) < 0)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not activate ESROS Service Access Point.",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Activate our 3-way Performer service access point */
      if ((retval = ESRO_CB_sapBind(&globals.sap.hPerformer3Way,
				    MTSUA_Rsap_Mts_Performer_3Way,
				    ESRO_3Way,
				    mts_invokeIndication, 
				    mts_resultIndication, 
				    mts_errorIndication,
				    mts_resultConfirm, 
				    mts_errorConfirm,
				    mts_failureIndication)) < 0)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not activate ESROS Service Access Point.",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Activate our 2-way Invoker service access point */
      if ((retval = ESRO_CB_sapBind(&globals.sap.hInvoker2Way,
				    MTSUA_Rsap_Mts_Invoker_2Way,
				    ESRO_2Way,
				    mts_invokeIndication, 
				    mts_resultIndication, 
				    mts_errorIndication,
				    mts_resultConfirm, 
				    mts_errorConfirm,
				    mts_failureIndication)) < 0)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not activate ESROS Service Access Point.",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Activate our 2-way Performer service access point */
      if ((retval = ESRO_CB_sapBind(&globals.sap.hPerformer2Way,
				    MTSUA_Rsap_Mts_Performer_2Way,
				    ESRO_2Way,
				    mts_invokeIndication, 
				    mts_resultIndication, 
				    mts_errorIndication,
				    mts_resultConfirm, 
				    mts_errorConfirm,
				    mts_failureIndication)) < 0)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not activate ESROS Service Access Point.",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Initialize the FSM module */
      if (FSM_init() != SUCCESS)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not initialize finite state machine module\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      FSM_TRANSDIAG_init();
      
      /* Initialize the Performer finite state machines */
      if (mts_performerFsmInit() != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not initialize Performer state machines\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }

      /* Initialize the Delivery finite state machines */
      if (mts_deliverFsmInit() != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not initialize Delivery state machines\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }

      /* Set the directory filter */
      OS_dirSetFilter(dirFilter);

      /* Open the delivery new message directory */
      if ((rc =
	   OS_dirOpen(globals.config.deliver.pNewMessageDir,
		      &globals.config.deliver.hNewMessageDir)) != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not open delivery new-message directory\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Create timers for all spooled messages to be delivered */
      if ((rc = mts_buildTimerQueue(globals.config.deliver.pSpoolDir,
				    mts_tryDelivery)) != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not build timer queue for delivery directory %s\n",
		  __applicationName,
		  globals.config.deliver.pSpoolDir);
	  EH_fatal(globals.tmpbuf);
      }

      /* make sure the spool directories are writable */
      if (access (globals.config.deliver.pNewMessageDir, W_OK)) 
      {
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
		    "Permission problem (write): '%s'",
		    globals.config.deliver.pNewMessageDir));
	  EH_fatal("No write access to delivery new-message directory\n");
      }
      if (access (globals.config.deliver.pSpoolDir, W_OK)) 
      {
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
	 	    "Permission problem (write): '%s'",
		    globals.config.deliver.pSpoolDir));
	  EH_fatal("No write access to delivery spool directory\n");
      }

      /* make sure there are control and data directories for this recipient */
      sprintf (dirPath, "%s/recipData", globals.config.deliver.pSpoolDir);
      if (mkdir (dirPath, 0777)) 
      {
	if (errno != EEXIST) {
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
		   "Cannot create delivery data directory: '%s'", dirPath));
	}
      }
      sprintf (dirPath, "%s/recipControl", globals.config.deliver.pSpoolDir);
      if (mkdir (dirPath, 0777)) 
      {
	if (errno != EEXIST) 
	{
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, 
		   "Cannot create delivery data directory: '%s'", dirPath));
	}
      }

      /* Create timers for all spooled submitted messages */
      if ((rc = mts_buildTimerQueue(globals.config.submit.pSpoolDir,
				    mts_tryRfc822Submit)) != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not build timer queue for submit directory %s\n",
		  __applicationName,
		  globals.config.submit.pSpoolDir);
	  EH_fatal(globals.tmpbuf);
      }

      /* Create timers for all spooled messages to be delivered */
      rc = mts_buildTimerQueue(globals.config.deliver.pSpoolDir, 
			       mts_tryDelivery);
      if (rc != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not build timer queue for delivery directory %s\n",
		  __applicationName,
		  globals.config.deliver.pSpoolDir);
	  EH_fatal(globals.tmpbuf);
      }

      /* Create timers for all spooled submitted messages */
      TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, 
		"Creating timers for all spooled messages"));
      rc = mts_buildTimerQueue(globals.config.submit.pSpoolDir, 
			       mts_tryRfc822Submit);
      if (rc != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not build timer queue for submit directory %s\n",
		  __applicationName,
		  globals.config.submit.pSpoolDir);
	  EH_fatal(globals.tmpbuf);
      }

      /* Create a timer to spool messages destined to devices */
      if ((rc = TMR_start(TMR_SECONDS(1),
			  NULL, NULL,
			  mts_spoolToDevice, &hTimer)) != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not start initial timer to process; "
		  "spooling to device\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Create a timer to process Module Management events */
      if ((rc = TMR_start(TMR_SECONDS(1),
			  NULL, NULL, runMM, &hTimer)) != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not start initial timer to process; "
		  "module management events\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Start clock interrupts */
      if (TMR_startClockInterrupt(TMR_SECONDS(5)) != Success)
      {
	  sprintf(globals.tmpbuf,
		  "%s: Could not start clock interrupt\n",
		  __applicationName);
	  EH_fatal(globals.tmpbuf);
      }
      
      /* Specify our RFC-822 to EMSD address translation function */
      mtsua_setLookupEmsdAddress(convertRfc822ToEmsdAua);

      /* Main Loop */
      while (SCH_block() >= 0)
      {
	  pass++;
	  SCH_run();
	  TM_TRACE((globals.hTM, MTS_TRACE_SCHED, 
		   "Scheduler ran, took %d seconds (+/- 1s)",
	           time(NULL) - lastTime));
      }
      
      sprintf(globals.tmpbuf,
	      "%s: Unexpected Scheduler error.\n",
	      __applicationName);
      EH_fatal(globals.tmpbuf);
}

static ReturnCode
runMM(void * hTimer,
      void * hUserData1,
      void * hUserData2)
{
    ReturnCode		rc;

    /* Create a timer to process Module Management events again */
    if ((rc = TMR_start(TMR_SECONDS(1),
			NULL, NULL, runMM, &hTimer)) != Success)
    {
	sprintf(globals.tmpbuf,
		"%s: Could not start initial timer to process module management events\n",
		__applicationName);
	EH_fatal(globals.tmpbuf);
    }
      
    return MM_processEvents(globals.hMMEntity, NULL);
}


ReturnCode
addRetryTime(OS_Uint32 seconds)
{
    /* See if the list is already full */
    if (globals.numRetryTimes == MTS_MAX_RETRIES)
    {
	/* It is. */
	return Fail;
    }

    globals.retryTimes[globals.numRetryTimes++] = seconds;

    return Success;
}



static OS_Boolean
dirFilter(const char * pName)
{
    /* Include only files which end in "C_xxxxxx" */
    if ((pName[0] == 'C' || pName[0] == 'c') && (pName[1] == '_'))
    {
	/* Include this file. */
	return 1;
    }

    return 0;
}



static ReturnCode
getEmsdAddrNumericValue(char * pStr, OS_Uint32 *pNum)
{
    ReturnCode          rc          = Success;
    int			highOrder   = 0;
    int			lowOrder    = 0;
    int			dummy       = 0;
    int			numFields   = 0;

    /*
     * Get the numeric value of this address.  It should be two
     * numbers, separated by a dot.
     */
    numFields = sscanf(pStr, "%d.%d.%d", &highOrder, &lowOrder, &dummy);
    switch (numFields)
    {
        case 2:
	  /* Generate the numeric address from the human-readable representation */
	  highOrder = (highOrder * 1000) + lowOrder;

        case 1:
	  *pNum = highOrder;
	  TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS,
	      "Converted RFC-822 Address to EMSD Local Address: '%s' => %d", pStr, *pNum));
          break;

        default:
	  TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, 
		    "Error converting EMSD address string to numerics: %s", pStr));
	  rc = Fail;
    }
    return (rc);
}

static ReturnCode
convertRfc822ToEmsdAua(char       *pUserName,
		      OS_Uint16   len,
		      OS_Uint32  *pEmsdAddress,
		      ADDRESS    *pAddr)
{
    ReturnCode		rc          = Success;
    char *		p           = NULL;
    char *		pEnd        = pUserName + len;
    char		endChar     = '\0';
    MTS_Host *		pHost       = NULL;
    char *              pUser       = NULL;
    SUB_Profile *       pSubPro     = NULL;

    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "Converting RFC-822 address EMSD address, host = %s", pAddr->host));

    /* Make sure the host name is one in the EMSD domain */
    for (pHost = QU_FIRST(&globals.config.emsdHosts);
	 ! QU_EQUAL(pHost, &globals.config.emsdHosts);
	 pHost = QU_NEXT(pHost))
    {
	if (OS_strcasecmp(pHost->pHostName, pAddr->host) == 0)
	{
	    break;
	}
    }
    /* If we didn't find this host, don't convert it. */
    if (QU_EQUAL(pHost, &globals.config.emsdHosts))
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, 
		 "EMSD hostname not found: %s", pAddr->host));
	return Fail;
    }

    /*
     * Make sure the recipient address is null-terminated at the
     * correct place.
     */
    endChar = *pEnd;
    *pEnd = '\0';

    /*
     * Find the user profile where the EMSD-AUA or one of the EMSD-AUA Nicknames
     * match this address
     */
    if ((pSubPro = SUB_getEntry (pUserName, NULL)) == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, 
		 "Could not find User Profile for: %s", pUserName));
	*pEnd = endChar;
	return (Fail);
    }
    TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, "Found user %s", pUserName));

    if (getEmsdAddrNumericValue (pUserName, pEmsdAddress) == Success)
    {
	return Success;
    }

    /* Get the EMSD AUA for this recipient */
/***if (CONFIG_getString(globals.hConfig, hSearch, P_NAME_EMSD_AUA, &pUser) != Success) ***/
/***if (PROFILE_getAttributeValue(globals.hProfile, hSearch, P_NAME_EMSD_AUA, &p) != Success) ***/
    strcpy (pUser, pSubPro->emsdAua);
    if (*pUser == NULL)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, 
		 "EMSD AUA for recipient not found: %s", pUser ? pUser : "NULL"));
	*pEnd = endChar;
	return (Fail);
    }
    TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, "User subscriber AUA: '%s'", pUser ? pUser : "NULL"));

    /*
     * Get the numeric value of this address.  It should be one or two
     * numbers; separated by a dot if two.
     */
    if (getEmsdAddrNumericValue (p, pEmsdAddress) == Success)
    {
	return Success;
    }
    *pEnd = endChar;

    return rc;
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
    fprintf(stderr, "Program received SIGINT signal\n");
    fprintf(stdout, "Program received SIGINT signal\n");
    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Program received SIGINT signal"));
    exit(13);
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
    char   *pText = NULL;

    fprintf(stderr, "Program received SIGPIPE signal, arg=0x%08x\n", unused);
    fprintf(stdout, "Program received SIGPIPE signal, arg=0x%08x\n", unused);
    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Program received SIGPIPE signal, arg=0x%08x", unused));

    if ((pText = getenv("MTA_SIGPIPE_ACTION")) != NULL) {
      if (!strcmp (pText, "IGNORE")) {
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Ignoring SIGPIPE signal"));
	return;
      }
      if (!strcmp (pText, "DUMP")) {
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Calling abort() on SIGPIPE signal"));
	abort ();
      }
      if (!strcmp (pText, "EXIT")) {
	TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Exiting due to SIGPIPE signal"));
	exit (14);
      }
      TM_TRACE((globals.hTM, MTS_TRACE_WARNING, "Unknown value for MTA_SIGPIPE: '%s'", pText));
    }
    /*** abort for now, exit when things with SIGPIPE settle down ***/
    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Calling abort() on SIGPIPE signal"));
    abort ();

    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Exiting due to SIGPIPE signal"));
    exit(14);
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
    fprintf(stderr, "Program received SIGTERM signal\n");
    fprintf(stdout, "Program received SIGTERM signal\n");
    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Program received SIGTERM signal"));
    exit(15);
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
    /* This is never called, as SIGKILL is never caught */
    fprintf(stderr, "Program received SIGKILL signal\n");
    fprintf(stdout, "Program received SIGKILL signal\n");
    TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Program received SIGKILL signal"));
    exit(16);
}
#endif
