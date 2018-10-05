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

static volatile char *rcs = "$Id: mailer.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";

#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include "estd.h"
#include "eh.h"
#include "tm.h"
#include "config.h"
#include "profile.h"
#include "nnp.h"
#include "emsdmail.h"
#include "mtsua.h"

FORWARD static OS_Boolean
emsdAuaEqual(char * pAua1,
	    char * pAua2,
	    OS_Uint32 type);


char *	__applicationName = "";
char errbuf[200];


int
main(int argc, char * argv[])
{
    ReturnCode 			rc                          = Success;
    int 			c                           = 0;
    int				len                         = 0;
    int                         quiet                       = 0;
    FILE * 			hFile                       = NULL;
    char *			p                           = NULL;
    char *			pLocalHostName              = "localhost";
    char *			pConfigFile                 = NULL;
    char *			pSpoolDir                   = NULL;
    char *			pDataDir                    = NULL;
    void *			hConfig                     = NULL;
    void *			hProfile                    = NULL;
    void *			hSubscriber                 = NULL;
    OS_Uint32 			retryTimeJulian             = 0;
    MTSUA_Recipient *		pRecip                      = NULL;
    MAILSTREAM *		pRfcMessage                 = NULL;
    ENVELOPE *			pRfcEnv                     = NULL;
    char *			pSubscriberProfileFile      = NULL;
    MTSUA_ControlFileData	cfData;
    char 			dataFileName[OS_MAX_FILENAME_LEN];
    char 			tempControlFileName[OS_MAX_FILENAME_LEN];

    Char                       *copyrightNotice;
    EXTERN Char                *RELID_getRelidNotice(Void);
    EXTERN Char                *RELID_getRelidNotice(Void);
    EXTERN Char                *RELID_getRelidNotice(Void);

    __applicationName = argv[0];
    OS_allocDebugInit(NULL);
    umask(0);

    while ((c = getopt(argc, argv, "c:e:l:vV")) != EOF)
    {
        switch (c)
	{
	case 'v':		/* Print version and exit */
	case 'V':
	    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
		EH_fatal("main: Get copyright notice failed\n");
	    }
	    fprintf(stdout, "%s\n", copyrightNotice);
	    fprintf(stdout, "emsd-mailer built at %s on %s\nRCS ID: %s\n", __TIME__, __DATE__, rcs);
	    return (0);

	case 'q':
	    quiet++;
	    break;

	case 'c':		/* Configuration File */
	    pConfigFile = optarg;
	    break;

	case 'e':		/* set an Environment variable */
	    putenv(optarg);
	    break;
	    
	case 'l':		/* local host name */
	    pLocalHostName = optarg;
	    break;

	default:
	  usage:
	    return EX_USAGE;
	}
    }

    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
      EH_fatal("main: Get copyright notice failed\n");
    }
    fprintf(stdout, "%s\n", copyrightNotice);

    fprintf(stdout, "emsd-mailer started (built at %s on %s) [%s]\n", __TIME__, __DATE__, rcs);
    fprintf(stdout, "ConfigFile=%s\n", pConfigFile);
    fprintf(stdout, "LocalHostName=%s\n", pLocalHostName);

    argc -= optind;
    argv += optind;

    if (pConfigFile == NULL || argc != 1)
    {
	goto usage;
    }

#ifdef TM_ENABLED
    /* Initialize the trace module */
    TM_INIT();
    TM_config("/tmp/emsd-mailer.trace");
#endif

    /* Initialize the configuration module */
    if ((rc = CONFIG_init()) != Success)
    {
	return EX_SOFTWARE;
    }

    /*
     * Open the configuration file.  Get a handle to the configuration data.
     */
    if ((rc = CONFIG_open(pConfigFile, &hConfig)) != Success)
    {
	return EX_OSFILE;
    }
      
    /* Get the spool directory */
    if ((rc = CONFIG_getString(hConfig, "Delivery", "New Message Directory", &pSpoolDir)) != Success)
    {
	return EX_OSFILE;
    }

    /* Get the data directory */
    if ((rc = CONFIG_getString(hConfig, "Delivery", "Data Directory", &pDataDir)) != Success)
    {
	return EX_OSFILE;
    }

    /* Get the subscriber profile file */
    if ((rc = CONFIG_getString(hConfig, "Subscribers", "Profile File Name", &pSubscriberProfileFile)) != Success)
    {
	return EX_OSFILE;
    }
fprintf(stdout, "SubscriberProfile=%s\n", pSubscriberProfileFile);

    /* Open the profile */
    if ((rc = PROFILE_open(pSubscriberProfileFile, &hProfile)) != Success)
    {
	printf ("Cannot open profile file: %s\n", pSubscriberProfileFile);
	return EX_OSFILE;
    }
      
    /* Add an attribute which will be the first one */
    rc = PROFILE_addAttribute(hProfile, P_NAME_EMSD_REPLY_TO, P_TYPE_EMSD_REPLY_TO, emsdAuaEqual);
    if (rc != Success)
    {
	printf ("Cannot add REPLY_TO attribute to profile\n");
    }

    /* Add the attribute names and types that we care about */
    rc = PROFILE_addAttribute(hProfile, P_NAME_EMSD_AUA, P_TYPE_EMSD_AUA, emsdAuaEqual);
    if (rc != Success)
    {
	printf ("Cannot add AUA attribute to profile\n");
	return EX_OSFILE;
    }
    rc = PROFILE_addAttribute(hProfile, P_NAME_EMSD_AUA_NICKNAME,
			      P_TYPE_EMSD_AUA_NICKNAME, emsdAuaEqual);
    if (rc != Success)
    {
	printf ("Cannot add 'AUA Nickname' attribute to profile\n");
	return EX_OSFILE;
    }
    
    /* Make sure the one and only recipient is an EMSD subscriber */
    if (PROFILE_searchByType(hProfile, P_TYPE_EMSD_AUA, *argv, &hSubscriber) != Success)
    {
	fprintf (stdout, "Cannot find user '%s' in profile\n", *argv);
	/*** return EX_NOUSER; ***/   /* This lets sendmail issue a No-Such-User non-delivery */
    } else {
        fprintf(stdout, "user found=%s\n", *argv);
    }
    len = strlen(pSpoolDir);
    if (len > 1)
    {
	/* If the passed-in spool directory has a '/' at the end... */
	if ((p = strchr(OS_DIR_PATH_SEPARATOR, pSpoolDir[len - 1])) != NULL)
	{
	    /* ... get rid of it. */
	    *p = '\0';
	}
    }

    /* Create a prototype for the data file name */
    sprintf(dataFileName, "%s%cD_%08lx_XXXXXX", pDataDir, *OS_DIR_PATH_SEPARATOR, time(NULL));

    /* Create the data file */
    if ((rc = OS_uniqueName(dataFileName)) != Success)
    {
        printf ("Error creating data file: %s\n", dataFileName);
	return EX_CANTCREAT;
    }

    /* MB Hack */
    chmod(dataFileName, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));

    /* Open the data file */
    if ((hFile = OS_fileOpen(dataFileName, "wb")) == NULL)
    {
        printf ("Error opening data file: %s\n", dataFileName);
	OS_deleteFile(dataFileName);
	return EX_CANTCREAT;
    }
fprintf(stdout, "Data file=%s\n", dataFileName);

    /* Write standard input to the data file */
    while ((c = fgetc(stdin)) != EOF)
    {
	OS_filePutChar(c, hFile);
    }

    /* Close the data file */
    OS_fileClose(hFile);

    /* Open the RFC-822 message */
    if ((pRfcMessage = mail_open(NIL, dataFileName,
				 NIL, pLocalHostName)) == NULL)
    {
        printf ("Error opening RFC-822 message: %s/%s\n", pLocalHostName, dataFileName);
	(void) OS_deleteFile(dataFileName);
	return EX_DATAERR;
    }

    /* Get the message envelope information */
    if ((pRfcEnv = mail_fetchstructure(pRfcMessage, 1, NULL)) == NULL)
    {
	mail_close(pRfcMessage);
	(void) OS_deleteFile(dataFileName);
	return EX_DATAERR;
    }
    
    /* Specify the control file data */
    cfData.retries = 0;
    cfData.nextRetryTime = retryTimeJulian;
    strcpy(cfData.dataFileName, dataFileName);

    /* Fill in the message id in the control file structure */
    if (pRfcEnv->message_id != NULL)
    {
	len = sizeof(cfData.messageId);
	strncpy(cfData.messageId, pRfcEnv->message_id, len);
	cfData.messageId[len] = '\0';
    }
    else
    {
	*cfData.messageId = '\0';
    }
    
    /* Fill in the originator in the control file structure */
    if (pRfcEnv->sender != NULL)
    {
	mtsua_getPrintableRfc822Address(cfData.originator, pRfcEnv->sender);
    }
    else if (pRfcEnv->from != NULL)
    {
	mtsua_getPrintableRfc822Address(cfData.originator, pRfcEnv->from);
    }

    /* Close the RFC-822 message */
    mail_close(pRfcMessage);

    /* Initialize the recipient list queue head */
    QU_INIT(&cfData.recipList);

    /* Create a prototype for the TEMPORARY control file name */
    sprintf(tempControlFileName, "%s%cT_%08lx_%08lx_XXXXXX",
	    pSpoolDir, *OS_DIR_PATH_SEPARATOR, time (NULL), 0);

    /* Create the temporary control file */
    if ((rc = OS_uniqueName(tempControlFileName)) != Success)
    {
        printf ("Error creating temporary file: %s\n", tempControlFileName);
	OS_deleteFile(dataFileName);
	return EX_CANTCREAT;
    }

    /* Get the current time, in date/struct format */
    if ((rc = OS_currentDateTime(NULL, &retryTimeJulian)) != Success)
    {
        printf ("Error getting time and date\n");
	OS_deleteFile(dataFileName);
	return EX_OSERR;
    }

    /* Allocate space for this recipient */
    if ((pRecip = OS_alloc(sizeof(MTSUA_Recipient) + strlen(*argv))) == NULL)
    {
        printf ("Error allocating system memory\n");
	mtsua_freeControlFileData(&cfData);
	OS_deleteFile(dataFileName);
	OS_deleteFile(tempControlFileName);
	return EX_TEMPFAIL;
    }

    /* Initialize the queue pointers */
    QU_INIT(pRecip);

    /* Copy the recipient address */
    strcpy(pRecip->recipient, *argv);

    /* Insert this recipient into the queue */
    QU_INSERT(pRecip, &cfData.recipList);

    /* Write the control file */
    if ((rc = mtsua_writeControlFile(tempControlFileName, &cfData)) != Success)
    {
	/* fall through; don't exit here. */
        printf ("*** Error writing control file [%s] ***\n", strerror(errno));
    }
    else if (chmod(tempControlFileName, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)))
    {
        printf ("*** Error setting permissions on control file [%s] ***\n", strerror(errno));
    }

fprintf(stdout, "Control file=%s\n", tempControlFileName);

    /* We're all done with the control file data */
    mtsua_freeControlFileData(&cfData);

    /* Did we successfully write the control file? */
    if (rc != Success)
    {
        /* We had a problem.  Clean up and let 'em know */
        printf ("Error creating control file: %s\n", tempControlFileName);
	OS_deleteFile(dataFileName);
	OS_deleteFile(tempControlFileName);
    }
    return (rc == Success ? EX_OK : EX_CANTCREAT);
}


static OS_Boolean
emsdAuaEqual(char * pAua1,
	    char * pAua2,
	    OS_Uint32 type)
{
    OS_Boolean	    rc;
    char *	    pAt1;
    char *	    pAt2;

    /* If there's a domain part in AUA 1, strip it off for now */
    if ((pAt1 = strchr(pAua1, '@')) != NULL)
    {
	/* There is.  Null terminate at this point. */
	*pAt1 = '\0';
    }

    /* If there's a domain part in AUA 2, strip it off for now */
    if ((pAt2 = strchr(pAua2, '@')) != NULL)
    {
	/* There is.  Null terminate at this point. */
	*pAt2 = '\0';
    }

    /* Now, are the two the same? */
    rc = (OS_strcasecmp(pAua1, pAua2) == 0);

    /* Restore the atsigns */
    if (pAt1 != NULL)
    {
	*pAt1 = '@';
    }

    if (pAt2 != NULL)
    {
	*pAt2 = '@';
    }

    return rc;
}
