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

static char *rcs = "$Id: ctrlfile.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";


#include "estd.h"
#include "mtsua.h"
#include "eh.h"
#include "mm.h"



ReturnCode
mtsua_readControlFile(char * pControlFileName,
		      MTSUA_ControlFileData * pControlFileData)
{
    char * 		p;
    char * 		pStart;
    FILE * 		hControlFile;
    MTSUA_Recipient *	pRecip;
    char	        tmpbuf[1024];
    
#define	SKIP(s1, s2)							\
    {									\
	for (pStart = s1, p = s2;					\
	     *pStart != '\0' && *p != '\0' && *pStart == *p;		\
	     pStart++, p++)						\
	{								\
	    /* empty body */						\
	}								\
									\
	if (*p != '\0')							\
	{								\
	    OS_fileClose(hControlFile);					\
	    return Fail;						\
	}								\
    }

    /* Open the control file */
    if ((hControlFile = OS_fileOpen(pControlFileName, "rt")) == NULL)
    {
	return OS_RC_FileOpen;
    }
    
    /* Read the data file name */
    if (OS_fileGetString(tmpbuf, sizeof(tmpbuf), hControlFile) == NULL)
    {
	/* We should have been able to read it! */
	OS_fileClose(hControlFile);
	return OS_RC_FileRead;
    }
    
    /* Skip past the header */
    SKIP(tmpbuf, "Data File Name: ");
    
    /* If we got a trailing new-line... */
    p = pStart + strlen(pStart);
    
    if (p > tmpbuf && p[-1] == '\n')
    {
	/* ... strip it off. */
	*--p = '\0';
    }
    
    /* Copy the data file name */
    strcpy(pControlFileData->dataFileName, pStart);
    
    /* Get the originator */
    if (OS_fileGetString(tmpbuf, sizeof(tmpbuf), hControlFile) == NULL)
    {
	/* We should have been able to read it! */
	OS_fileClose(hControlFile);
	return OS_RC_FileRead;
    }
    
    /* Skip past the header */
    SKIP(tmpbuf, "Originator: ");
    
    /* If we got a trailing new-line... */
    p = pStart + strlen(pStart);
    
    if (p > tmpbuf && p[-1] == '\n')
    {
	/* ... strip it off. */
	*--p = '\0';
    }
    
    /* Copy the originator name */
    strcpy(pControlFileData->originator, pStart);
    
    /* Get the message id */
    if (OS_fileGetString(tmpbuf, sizeof(tmpbuf), hControlFile) == NULL)
    {
	/* We should have been able to read it! */
	OS_fileClose(hControlFile);
	return OS_RC_FileRead;
    }
    
    /* Skip past the header */
    SKIP(tmpbuf, "Message Id: ");
    
    /* If we got a trailing new-line... */
    p = pStart + strlen(pStart);
    
    if (p > tmpbuf && p[-1] == '\n')
    {
	/* ... strip it off. */
	*--p = '\0';
    }
    
    /* Copy the message id */
    strcpy(pControlFileData->messageId, pStart);

    /* Read the retry number */
    if (OS_fileGetString(tmpbuf, sizeof(tmpbuf), hControlFile) == NULL)
    {
	/* We should have been able to read it! */
	OS_fileClose(hControlFile);
	return OS_RC_FileRead;
    }
    
    /* Skip past the header */
    SKIP(tmpbuf, "Delivery Attempts: ");
	
	/* Get the number of previous retries */
    pControlFileData->retries = atoi(pStart);
	
    /* Read the next retry time */
    if (OS_fileGetString(tmpbuf, sizeof(tmpbuf), hControlFile) == NULL)
    {
	/* We should have been able to read it! */
	OS_fileClose(hControlFile);
	return OS_RC_FileRead;
    }
	
    /* Skip past the header */
    SKIP(tmpbuf, "Next Retry: ");
	
    /* Get the julian date.  Ignore human-readable part. */
    pControlFileData->nextRetryTime = atol(pStart);
	
    /* Initialize the recipient list head */
    QU_INIT(&pControlFileData->recipList);
    
    /* Read all of the recipient for whom we're responsible */
    while (OS_fileGetString(tmpbuf, sizeof(tmpbuf), hControlFile) != NULL)
    {
	/* Skip past the header */
	SKIP(tmpbuf, "To: ");
	
	/* If we got a trailing new-line... */
	p = pStart + strlen(pStart);
	
	if (p > pStart && p[-1] == '\n')
	{
	    /* ... strip it off. */
	    *--p = '\0';
	}
	
	/* Allocate a recipient structure with space for the name */
	if ((pRecip = OS_alloc(sizeof(MTSUA_Recipient) + strlen(pStart))) == NULL)
	{
	    OS_fileClose(hControlFile);
	    mtsua_freeControlFileData(pControlFileData);
	    return ResourceError;
	}
	
	/* Initialize the queue pointers */
	QU_INIT(pRecip);
	
	/* Copy the recipient name */
	strcpy(pRecip->recipient, pStart);

	/* Insert this recipient onto the recipient list */
	QU_INSERT(pRecip, &pControlFileData->recipList);
    }
    
    /* All done.  Close the file. */
    OS_fileClose(hControlFile);
    
    return Success;

#undef SKIP
}



ReturnCode
mtsua_writeControlFile(char * pTempControlFileName,
		       MTSUA_ControlFileData * pControlFileData)
{
    ReturnCode 		rc                = Success;
    char *		p                 = NULL;
    FILE *		hControlFile      = NULL;
    MTSUA_Recipient * 	pRecip            = 0;
    OS_ZuluDateTime 	retryTimeStruct;
    char		controlFileName[1024];
    
    /* Open the temporary control file */
    if ((hControlFile = OS_fileOpen(pTempControlFileName, "wt")) == NULL)
    {
	return OS_RC_FileOpen;
    }
    
    /* Determine the retry time, in date-struct format */
    if ((rc = OS_julianToDateStruct(pControlFileData->nextRetryTime,
				    &retryTimeStruct)) != Success)
    {
	OS_fileClose(hControlFile);
	return Fail;
    }
	
    /* Write out the control file data (other than recipient data) */
    OS_filePrintf(hControlFile,
		  "Data File Name: %s\n"
		  "Originator: %s\n"
		  "Message Id: %s\n"
		  "Delivery Attempts: %d\n"
		  "Next Retry: %lu (%02d/%02d/%02d %02d:%02d:%02d GMT)\n",
		  pControlFileData->dataFileName,
		  pControlFileData->originator,
		  pControlFileData->messageId,
		  pControlFileData->retries,
		  pControlFileData->nextRetryTime,
		  retryTimeStruct.year,
		  retryTimeStruct.month,
		  retryTimeStruct.day,
		  retryTimeStruct.hour,
		  retryTimeStruct.minute,
		  retryTimeStruct.second);
    
    
    /* Write out the list of recipients whom we're responsible for */
    for (pRecip = QU_FIRST(&pControlFileData->recipList);
	 ! QU_EQUAL(pRecip, &pControlFileData->recipList);
	 pRecip = QU_NEXT(pRecip))
    {
	fprintf(hControlFile, "To: %s\n", pRecip->recipient);
    }
    
    /* All done with this file. */
    OS_fileClose(hControlFile);
    
    /* Copy the temporary control file name */
    strcpy(controlFileName, pTempControlFileName);
    
    /* Create a new prototype for the real control file. */
    p = controlFileName + strlen(controlFileName) - 6;
    strcpy(p, "XXXXXX");
    p = strrchr (controlFileName, '/');
    if (p)
    {
	if (*++p == 'T')
	  *p = 'C';
	else
	  printf ("Expected 'T', got '%c'", *p);
    }
    else
    {
	printf ("Control file name does not contain a '/'\n");
    }
    
    /* Create a unique name for this file */
    if ((rc = OS_uniqueName(controlFileName)) != Success)
    {
	return rc;
    }
    
    /* Rename the temporary file to the real control file name */
    if ((rc = OS_moveFile(controlFileName,
			  pTempControlFileName)) != Success)
    {
	OS_deleteFile(pTempControlFileName);
	return rc;
    }
    
    /* Give 'em the new control file name.  It's the same length. */
    strcpy(pTempControlFileName, controlFileName);
    
    return Success;
}


void
mtsua_freeControlFileData(MTSUA_ControlFileData * pControlFileData)
{
    QU_FREE(&pControlFileData->recipList);
}

ReturnCode
mtsua_deleteControlAndDataFiles (char *                   pControlFileName,
				 MTSUA_ControlFileData *  pControlFileData)
{
    ReturnCode   rc       = Success;
    ReturnCode   result   = Success;

    if (pControlFileName == NULL) {
	result = Fail;
    } else {
      if ((rc = OS_deleteFile(pControlFileName)) != Success) {
	result = Fail;
	printf ("MTS could not delete control file (%s)\n", pControlFileName);
      }
    }

    if (!(pControlFileData && pControlFileData->dataFileName)) {
	result = Fail;
    } else {
      if ((rc = OS_deleteFile(pControlFileData->dataFileName)) != Success)
	{
	  result = Fail;
	  printf ("MTS could not delete data file (%s)\n", pControlFileData->dataFileName);
	}
    }
    return (result);
}
