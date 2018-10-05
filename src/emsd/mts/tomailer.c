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

static char *rcs = "$Id: tomailer.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";


#include "estd.h"
#include "mm.h"
#include "mtsua.h"
#include "mts.h"
#include "msgstore.h"

extern Void   G_sigPipe(int);
extern int    errno;

ReturnCode
mts_sendMessage(MTSUA_ControlFileData * pControlFileData, FILE * hDataFile)
{		    
    int 		c             = 0;
    OS_Uint32           emsdAddr       = 0;
    OS_Uint32           rfcCount      = 0;
    OS_Uint32           emsdCount      = 0;
    char * 		p             = NULL;
    char *		pStartAddr    = NULL;
    char *		pEndAddr      = NULL;
    void * 		hSendmail     = NULL;
    MTSUA_Recipient * 	pRecip        = NULL;

    /*
     * Generate a log message.
     */
    /* Point to our temporary buffer */
    p = globals.tmpbuf;

    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "sendMessage: Id: %s, From: %s",
	      pControlFileData->messageId, pControlFileData->originator));

    /* optionally handle local messages */
    if (getenv ("MTA_LOCAL_ROUTING"))
      {
	/* handle local addresses internally */
	msg_addMessage (pControlFileData->messageId, pControlFileData->originator,
			pControlFileData->recipList, pControlFileData->dataFileName);
      }

    sprintf(p, "Submitting message to mailer:\n\tMessage Id: %s\n\tFrom: %s\n",
	    pControlFileData->messageId, pControlFileData->originator);
    
    /* Update our buffer pointer */
    p += strlen(p);

    /* For each recipient... */
    for (pRecip = QU_FIRST(&pControlFileData->recipList);
	 ! QU_EQUAL(pRecip, &pControlFileData->recipList);
	 pRecip = QU_NEXT(pRecip))
    {
        if (getenv ("MTA_LOCAL_ROUTING"))
        {
	    /* handle local addresses internally */
	    if ((emsdAddr = msg_rfc822ToEmsdAddr (pRecip->recipient)) != 0)
	    {
	        continue;
	    }
        }

	/* Make sure there's room in the buffer for this recipient address */
	if (sizeof(globals.tmpbuf) - strlen(p) < strlen(pRecip->recipient))
	{
	    /* There's not. */
	    MM_logMessage(globals.hMMLog, globals.tmpbuf,
			  "MTS found too many recipients to fit on mailer command line; "
			  "Message Id: %s\n", pControlFileData->messageId);
	    
	    return Fail;
	}

	/* Add the recipient address to the log message */
	TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Adding Recipient: '%s'",
	      pRecip->recipient));

	sprintf(p, "\tTo: %s\n", pRecip->recipient);

	/* Update the command line pointer */
	p += strlen(p);
    }
    
    /*
     * Generate the mailer command.
     */

    /* Point to our command line buffer. */
    p = globals.tmpbuf;

    /* Specify the command name to be executed */
    sprintf(p, "%s ", globals.config.pSendmailPath);

    /* Update the buffer pointer */
    p += strlen(p);

    /*
     * If we know the originator, and we're configured to do it, add a
     * "from" option.
     */
    if (*globals.config.pSendmailSpecifyFrom != '\0' &&
	*pControlFileData->originator != '\0' &&
	(pStartAddr = strchr(pControlFileData->originator, '<')) != NULL &&
	(pEndAddr = strchr(pStartAddr, '>')) != NULL)
    {
	/* Temporarily null-terminate the actual address portion */
	*pEndAddr = '\0';

	/* Update to beginning of actual address portion */
	++pStartAddr;

	/* Create the From option */
	sprintf(p, "%s%s ", globals.config.pSendmailSpecifyFrom, pStartAddr);

	/* Update the buffer pointer */
	p += strlen(p);

	/* Undo our temporary termination modification */
	*pEndAddr = '>';
    }

    /* For each recipient... */
    for (pRecip = QU_FIRST(&pControlFileData->recipList);
	 ! QU_EQUAL(pRecip, &pControlFileData->recipList);
	 pRecip = QU_NEXT(pRecip))
    {
        if (getenv ("MTA_LOCAL_ROUTING"))
        {
	    /* handle local addresses internally */
	    if ((emsdAddr = msg_rfc822ToEmsdAddr (pRecip->recipient)) != 0)
	    {
	        /* skip the RFC 822 mailer for this recipient */
		emsdCount++;
	        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
			"Local routing of recipient: '%s'", pRecip->recipient));
	        continue;
	    }
        }

        TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, 
		 "Recip: %s", pRecip->recipient));

	/* Make sure there's room in the buffer for this recipient address */
	if (sizeof(globals.tmpbuf) - strlen(p) < strlen(pRecip->recipient))
	{
	    /* There's not. */
 	    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Buffer too small"));

	    MM_logMessage(globals.hMMLog, globals.tmpbuf,
				 "MTS found too many recipients to fit on mailer command line; "
				 "Message Id: %s\n", pControlFileData->messageId);
	    return Fail;
	}

	/* Create a command line to sendmail */
	sprintf(p, "'%s' ", pRecip->recipient);

	/* Update the command line pointer */
	p += strlen(p);

	rfcCount++;
    }
    
    if (rfcCount == 0) {
      TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
		"No non-local messages to send to external mail system"));
      return (emsdCount ? Success : Fail);
    }
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY,
	      "MTS issuing Mailer Command, %d recipients: (%s)\n", rfcCount, globals.tmpbuf));
    /* Open a pipe to sendmail */
    if ((hSendmail = popen(globals.tmpbuf, "w")) == NULL) {
      /* Let 'em know that we're throwing stuff away */
      MM_logMessage(globals.hMMLog, globals.tmpbuf,
		    "MTS could not establish pipe to sendmail\n\tMessage Id: %s\n",
		    pControlFileData->messageId);
      return Fail;
    }
    
#if DUMP_MAIL_CONTENTS
    /* Display the contents */
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Dump of Data File:"));
#endif /* DUMP_MAIL_CONTENTS */

    /* Read from the data file and write to the sendmail pipe */
    while ((c = fgetc(hDataFile)) != EOF)
    {
	fputc(c, hSendmail);
#if DUMP_MAIL_CONTENTS
	putchar(c);
#endif /* DUMP_MAIL_CONTENTS */
    }

    MM_logMessage(globals.hMMLog, globals.tmpbuf,
		  "Submission Successful:\n\tMessage Id: %s\n",
		  pControlFileData->messageId);
    TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Sucessfully submitted to sendmail"));

    /* Close the pipe */
    sigset (SIGPIPE, SIG_IGN);
    sighold (SIGALRM);
    if (pclose(hSendmail))
      {
      TM_TRACE((globals.hTM, MTS_TRACE_ACTIVITY, "Close of sendmail pipe failed [%d]", errno));
      }
    sigset (SIGPIPE, G_sigPipe);
    sigrelse (SIGALRM);

    return Success;
}
