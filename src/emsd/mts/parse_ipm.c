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

	    /* Allocate an EMSD Message structure */
	    if ((pOperation->rc = mtsua_allocMessage(&pIpm)) != Success)
	    {
	        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "AllocMessage failed"));
		return FALSE;
	    }
	    
	    /* Arrange for the IPM to be freed later. */
	    if ((pOperation->rc =
		 mts_operationAddUserFree(pOperation,
					  (void (*)(void *)) mtsua_freeMessage,
					  pIpm)) != Success)
	    {
		mtsua_freeMessage(pIpm);
		TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "MTS Op AddUserFree failed"));
		return FALSE;
	    }
	    
	    /* Get the length of the Contents PDU */
	    pduLength = BUF_getBufferLength(pSubmitArg->hContents);
	    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
		     "PDU length = %ld", pduLength));
	    
	    /* Determine which protocol tree to use */
	    pCompTree =	mtsua_getProtocolTree(MTSUA_Protocol_Ipm,
					      pProfile->protocolVersionMajor,
					      pProfile->protocolVersionMinor);
	    
	    /* Parse the IPM PDU */
	    if ((pOperation->rc = ASN_parse(ASN_EncodingRules_Basic,
					    pCompTree,
					    pSubmitArg->hContents,
					    pIpm, &pduLength)) != Success)
	    {
	        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "Parse of IPM failed [%d]", pOperation->rc));
		(void) MM_logMessage(globals.hMMLog,
				     globals.tmpbuf,
				     "MTS could not parse SUBMIT contents.\n");
		return FALSE;
	    }
	    
	    /* Allocate a Control File Data structure */
	    pControlFileData = OS_alloc(sizeof(MTSUA_ControlFileData));
	    if (pControlFileData == NULL)
	    {
	        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "Alloc failure"));
		return FALSE;
	    }
	    
	    /* Initialize the control file data */
	    QU_INIT(&pControlFileData->recipList);
	    
	    /* Arrange for control file data elements to be freed later. */
	    if ((pOperation->rc =
		 mts_operationAddUserFree(pOperation,
					  (void (*)(void *)) mtsua_freeControlFileData,
					  pControlFileData)) !=	Success)
	    {
		TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "MTS Op AddUserFree failed"));
		return FALSE;
	    }
	    
	    /* Arrange for Control File Data structure to be freed later. */
	    /*** fnh ***/
	    if ((pOperation->rc =
		 mts_operationAddUserFree(pOperation,
					  mtsua_osFree,
					  pControlFileData)) != Success)
	    {
		OS_free(pControlFileData);
		TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "MTS Op AddUserFree failed"));
		return FALSE;
	    }
	    /***/

	    /* The message was parsable.  Build the recipient list. */
	    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
		     "Building recipient list (%d entries)",
		      pIpm->heading.recipientDataList.count));
	    pAddr = pIpm->heading.recipientDataList.data;
	    for (i = pIpm->heading.recipientDataList.count; i > 0; i--, pAddr++)
	    {
		/* What format is the address in? */
		if (pAddr->recipient.choice == EMSD_ORADDRESS_CHOICE_LOCAL)
		{
		    /*
		     * Create an RFC-822 address from the
		     * local-format address.
		     */
		    pORAddr = &pAddr->recipient;
		    sprintf(globals.tmpbuf, "%.*s <%03ld.%03ld@%s>",
		      (pORAddr->un.local.emsdName.exists ?
		        STR_getStringLength(pORAddr->un.local.emsdName.data) : 0),
		      (pORAddr->un.local.emsdName.exists ?
		        (char *)STR_stringStart(pORAddr->un.local.emsdName.data) : ""),
		      pORAddr->un.local.emsdAddress / 1000,
		      pORAddr->un.local.emsdAddress % 1000,
		      globals.config.pLocalHostName);

		    len = strlen(globals.tmpbuf) + 1;
		    TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, 
			     "Locale address: %s", globals.tmpbuf));
		}
		else
		{
		    /* RFC-822 format */
		    len = STR_getStringLength(pAddr->recipient.un.rfc822);
		    strncpy(globals.tmpbuf,
			    (char *)
			    STR_stringStart(pAddr->recipient.un.rfc822),
			    len);
		    globals.tmpbuf[len++] = '\0';
		    TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS, 
			     "RFC822 address: %s", globals.tmpbuf));
		}
		
		/* Allocate a Recipient queue entry */
		if ((pRecip = OS_alloc(sizeof(MTSUA_Recipient) + len)) == NULL)
		{
		    pOperation->rc = ResourceError;
                    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "alloc failure"));
		    return FALSE;
		}
		
		/* Initialize the queue entry */
		QU_INIT(pRecip);
		
		/* Copy the recipient name */
		strcpy(pRecip->recipient, globals.tmpbuf);
		
		/* Insert this recipient onto the recipient list. */
		TM_TRACE((globals.hTM, MTS_TRACE_ADDRESS,
			  "$$$$ Inserting To: entry, %08lx", 
			  &pControlFileData->recipList));
		QU_INSERT(pRecip, &pControlFileData->recipList);
	    }
	    
	    /* Obtain an RFC-822 envelope pointer */
	    pRfcEnv = mail_newenvelope();
	    
	    /* Arrange for the Envelope to be freed later. */
	    if ((pOperation->rc =
		 mts_operationAddUserFree(pOperation,
					  (void (*)(void *))mail_free_env,
					  pRfcEnv)) != Success)
	    {
		mail_free_env(pRfcEnv);
		TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "MTS Op AddUserFree failed"));
		return FALSE;
	    }
	    
	    /* Initially, there are no extra headers */
	    *extraHeaders = '\0';
	    
	    /* Convert the IPM to RFC-822 */
	    pOperation->rc = mtsua_convertEmsdIpmToRfc822(pIpm, pRfcEnv, extraHeaders,
							 globals.config.pLocalHostName);
	    if (pOperation->rc != Success)
	    {
                TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "EMSD to RFC822 conversion failed"));
		return FALSE;
	    }
	    
	    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "Converted to RFC-822"));

	    /* Point to the end of the extra headers */
	    p = extraHeaders + strlen(extraHeaders);
	    
	    /* Is there room for an X-EMSD-MTS line? */
	    if ((sizeof(extraHeaders) - (p - extraHeaders)) >
		(strlen("X-EMSD-MTS: ") +
		 strlen(globals.config.pLocalHostName) +
		 strlen(globals.config.pLocalNsapAddress) + 4))
	    {
		/* Yup, there's room. */
		sprintf(p, "X-EMSD-MTS: %s (%s)\n",
			globals.config.pLocalHostName,
			globals.config.pLocalNsapAddress);
	    }
	    
	    /* Generate a message id for this message */
	    pRfcEnv->message_id = generateMessageId(pOperation);

	    /* Let the operation know the message id string */
	    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
		      "Assigned EMSD ID '%08lx/%4ld'",
		      pOperation->localMessageId.submissionTime,
		      pOperation->localMessageId.dailyMessageNumber));

	    /* Fill in the message id in the control file structure */
	    if (pRfcEnv->message_id != NULL)
	    {
		len = sizeof(pControlFileData->messageId);
		strncpy(pControlFileData->messageId, pRfcEnv->message_id, len);
		pControlFileData->messageId[len] = '\0';
	    }
	    else
	    {
		*pControlFileData->messageId = '\0';
	    }

	    /* Fill in the originator in the control file structure */
	    if (pRfcEnv->sender != NULL)
	    {
		mtsua_getPrintableRfc822Address(pControlFileData->originator, pRfcEnv->sender);
	    }
	    else
	    {
		mtsua_getPrintableRfc822Address(pControlFileData->originator, pRfcEnv->from);
	    }
	    
	    /* Initially, there are no delivery attempts */
	    pControlFileData->retries = 0;

	    /* Get the time now, to be used as the next retry time */
	    if ((rc = OS_currentDateTime(NULL, &pControlFileData->nextRetryTime)) != Success)
	    {
		/* Should never occur; OS_currentDateTime() shouldn't fail. */
		pControlFileData->nextRetryTime = 0;
	    }

	    /* Get a pointer to the body text, if it exists */
	    if (pIpm->body.exists)
	    {
		pBody = (char *) STR_stringStart(pIpm->body.data.bodyText);
	    }
	    else
	    {
		pBody = NULL;
	    }
	    
	    /* Allocate memory for the control file name */
	    pControlFileName = calloc(1, strlen(globals.config.submit.pSpoolDir) + 48); /*** ****/
	    if (pControlFileName == NULL)
	    {
		return FALSE;
	    }

	    /* Arrange for control file name to be freed with the operation */
	    pOperation->rc = mts_operationAddUserFree(pOperation, mtsua_osFree,
						      pControlFileName);
	    if (pOperation->rc != Success)
	    {
		mail_free_env(pRfcEnv);
		TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "MTS Op AddUserFree failed"));
		return FALSE;
	    }

	    /* Create a template for a data file name */
	    sprintf(pControlFileData->dataFileName, "%s%cD_%08lX_%08lX_XXXXXX",
		    globals.config.submit.pDataDir, *OS_DIR_PATH_SEPARATOR,
		    pOperation->localMessageId.submissionTime,
		    pOperation->localMessageId.dailyMessageNumber);
	    
	    /* Create a unique name for the data file */
	    if ((pOperation->rc =
		 OS_uniqueName(pControlFileData->dataFileName)) != Success)
	    {
                TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "OS_uniqueName failed"));
		return FALSE;
	    }
	    
	    /* Write the message to permanent storage */
	    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
		      "Writing RFC822 message; extra headers = <%s>", 
		      extraHeaders));
	    if ((pOperation->rc =
		 writeRfc822Message(pControlFileData->dataFileName,
				    pRfcEnv, extraHeaders, pBody)) != Success)
	    {
                TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "Write of RFC822 message failed (%s)",
			  pControlFileData->dataFileName));
		return FALSE;
	    }
	    
	    /* Create a template for a temporary control file name */
	    sprintf(pControlFileName, "%s%cT_%08lX_%08lX_XXXXXX",
		    globals.config.submit.pSpoolDir, *OS_DIR_PATH_SEPARATOR,
		    pOperation->localMessageId.submissionTime,
		    pOperation->localMessageId.dailyMessageNumber);
	    
	    /* Create a unique name for the control file */
	    if ((pOperation->rc = OS_uniqueName(pControlFileName)) != Success)
	    {
                TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "OS_uniqueName failed"));
		return FALSE;
	    }
	    
	    /* Create the control file */
	    pOperation->rc = mtsua_writeControlFile(pControlFileName, pControlFileData);
	    if (pOperation->rc != Success)
	    {
                TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "Write of RFC822 control file failed (%s)",
			  pControlFileData->dataFileName));
		/* Delete the data file */
		if (OS_deleteFile(pControlFileData->dataFileName) != Success)
		{
		    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			     "OS_deleteFile(%s) failed",
			      pControlFileData->dataFileName ));
		    MM_logMessage(globals.hMMLog,
				  globals.tmpbuf,
				  "MTS could not delete data file %s\n",
				  pControlFileData->dataFileName);
		}
		return FALSE;
	    }
	    
	    /* Associate the control file with the opreation */
	    pOperation->pControlFileName = pControlFileName;
