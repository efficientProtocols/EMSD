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
#include "mm.h"
#include "mts.h"

FORWARD static void
screenAlert(char * pDestinationName,
	    char * pApplicationEntityInstanceName,
	    char * pModuleName,
	    char * pObjectName,
	    char * pDescription,
	    MM_EventType eventType,
	    ...);




ReturnCode
mts_initModuleManagement(char * pErrorBuf)
{
    ReturnCode 	    rc;

    /* Initialize the Module Management module; define our entity */
    if ((rc = MM_entityInit("emsd-mts", NULL, &globals.hMMEntity)) != Success)
    {
	strcpy(pErrorBuf, "Could not initialize entity with MM module");
	return Fail;
    }
    
    /* Initialize the Module Management module; define our Main module */
    if ((rc = MM_registerModule(globals.hMMEntity,
				"main", &globals.hMMModule)) != Success)
    {
	strcpy(pErrorBuf, "Could not register Main with MM module");
	return Fail;
    }
    
    /* Create a Log object */
    if ((rc =
	 MM_registerManagableObject(globals.hMMModule,
				    MM_ManagableObjectType_Log,
				    "log",
				    "Message Log",
				    MM_AccessByName_ReadWrite,
				    0xffffffff,
				    NULL,
				    NULL,
				    &globals.hMMLog)) != Success)
    {
	strcpy(pErrorBuf, "Could not create managable object: Log\n");
	return Fail;
    }
    
    /* Register a destination for log messages */
    if ((rc =
	 MM_registerDestination(globals.hMMEntity,
				"screen",
				"Screen Log",
				MM_AccessByName_ReadWrite,
				0xffffffff,
				0xffffffff,
				screenAlert,
				&globals.hMMScreenAlert)) !=
	Success)
    {
	strcpy(pErrorBuf, "Could not register screen alert destination\n");
	return Fail;
    }

#ifdef notyet
    
    /* Create managable object: Deliver new message directory */
    if ((rc =
	 MM_registerManagableObject(globals.hMMModule,
				    MM_ManagableObjectType_String,
				    "deliver new",
				    "Deliver: New Message Directory",
				    MM_AccessByName_Read,
				    0x0,
				    NULL,
				    NULL,
				    &globals.deliver.hNewMessageDirectory)) !=
	Success)
    {
	strcpy(pErrorBuf,
	       "Could not create managable object: "
	       "Deliver: new message directory\n");
	return Fail;
    }
    
    /* Create managable object: Deliver queue directory */
    if ((rc =
	 MM_registerManagableObject(globals.hMMModule,
				    MM_ManagableObjectType_String,
				    "deliver new",
				    "Deliver: New Message Directory",
				    MM_AccessByName_Read,
				    0x0,
				    NULL,
				    NULL,
				    &globals.deliver.hQueueDirectory)) !=
	Success)
    {
	strcpy(pErrorBuf,
	       "Could not create managable object: "
	       "Deliver: queue directory\n");
	return Fail;
    }
    
    /* Create managable object: Submit new message directory */
    if ((rc =
	 MM_registerManagableObject(globals.hMMModule,
				    MM_ManagableObjectType_String,
				    "submit new",
				    "Submit: New Message Directory",
				    MM_AccessByName_Read,
				    0x0,
				    NULL,
				    NULL,
				    &globals.submit.hNewMessageDirectory)) !=
	Success)
    {
	strcpy(pErrorBuf,
	       "Could not create managable object: "
	       "Submit: new message directory\n");
	return Fail;
    }
    
    /* Create managable object: Submit queue directory */
    if ((rc =
	 MM_registerManagableObject(globals.hMMModule,
				    MM_ManagableObjectType_String,
				    "submit new",
				    "Submit: New Message Directory",
				    MM_AccessByName_Read,
				    0x0,
				    NULL,
				    NULL,
				    &globals.submit.hQueueDirectory)) !=
	Success)
    {
	strcpy(pErrorBuf,
	       "Could not create managable object: "
	       "Submit: queue directory\n");
	return Fail;
    }
    
    /* Create managable object: Local Host Name */
    if ((rc =
	 MM_registerManagableObject(globals.hMMModule,
				    MM_ManagableObjectType_String,
				    "mts host name",
				    "MTS Local Host Name",
				    MM_AccessByName_Read,
				    0x0,
				    NULL,
				    NULL,
				    &globals.hLocalHostName)) !=
	Success)
    {
	strcpy(pErrorBuf,
	       "Could not create managable object: "
	       "Deliver: new message directory\n");
	return Fail;
    }
    
    /* Create managable object: Local Host NSAP address */
    if ((rc =
	 MM_registerManagableObject(globals.hMMModule,
				    MM_ManagableObjectType_String,
				    "mts nsap",
				    "MTS Local NSAP Address",
				    MM_AccessByName_Read,
				    0x0,
				    NULL,
				    NULL,
				    &globals.hLocalNsapAddress)) !=
	Success)
    {
	strcpy(pErrorBuf,
	       "Could not create managable object: "
	       "Deliver: new message directory\n");
	return Fail;
    }
    
    return Success;
    
    /* revised structure */
    struct
    {
	void *		hNewMessageDirectory;
	void *		hQueueDirectory;
    } 		    deliver;
    struct
    {
	char *		hNewMessageDirectory;
	char *		hQueueDirectory;
    } 		    submit;
    void *	    hLocalHostName;
    void *	    hLocalNsapAddress;
    void *	    hUaTsap;
    void *	    hUaPerformerRsap;
    void *	    hMtsInvokerRsap;
    void *	    hMtsPerformerRsap;
    void *	    hSubscriberProfileFile;
    QU_Head	    emsdHosts;
    
#endif /* notyet */

    return Success;
}


static void
screenAlert(char * pDestinationName,
	    char * pApplicationEntityInstanceName,
	    char * pModuleName,
	    char * pObjectName,
	    char * pDescription,
	    MM_EventType eventType,
	    ...)
{
    ReturnCode		rc;
    va_list		pArgs;
    OS_Uint32		julianDate;
    char *		p;
    char *		pDateTime;

    va_start(pArgs, eventType);

    switch(eventType)
    {
    case MM_EventType_LogMessage:
	/* Get the current date and time string */
	if ((rc = OS_currentDateTime(NULL, &julianDate)) != Success)
	{
	    pDateTime = "<Unknown date/time>";
	}
	else
	{
	    pDateTime = OS_printableDateTime(&julianDate);
	}
	
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 *
	 *		- the log message string, as a "char *"
	 */
	p = va_arg(pArgs, char *);

	/* Print the date/time, followed, on the next line, with the log msg */
	printf("\n---\n%s%s\n", pDateTime + 4, p);
	break;
	
    default:
	/* Unsupported event type */
	printf("<MM Event Type %d (not currently supported)>\n", eventType);
	break;
    }

    va_end(pArgs);
}


