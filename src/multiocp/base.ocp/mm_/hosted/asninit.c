/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*
 * Author: Derrell Lipman
 * History:
 *
 */

/*
 * Module Management Agent functionality
 */


#include "estd.h"
#include "rc.h"
#include "queue.h"
#include "tmr.h"
#include "buf.h"
#include "mm.h"
#include "mmlocal.h"


Protocols mm_protocols[] =
{
    {
	NULL,
	NULL
    },
    {
				/* Get Value */
	MM_compileGetValueRequest,
	MM_compileGetValueResult
    },
    {
				/* Set Value */
	MM_compileSetValueRequest,
	NULL
    },
    {
				/* Get Threshold */
	MM_compileGetThresholdRequest,
	MM_compileGetThresholdResult
    },
    {
				/* Set Threshold */
	MM_compileSetThresholdRequest,
	NULL
    },
    {
				/* Start Timer */
	MM_compileStartTimerRequest,
	NULL
    },
    {
				/* Stop Timer */
	MM_compileStopTimerRequest,
	NULL
    },
    {
				/* Get Application Entity List */
	NULL,
	MM_compileGetApplicationEntityListResult
    },
    {
				/* Get Module List */
	MM_compileGetModuleListRequest,
	MM_compileGetModuleListResult
    },
    {
				/* Get Managable Object List */
	MM_compileGetManagableObjectListRequest,
	MM_compileGetManagableObjectListResult
    },
    {
				/* Get Managable Object Information */
	MM_compileGetManagableObjectInfoRequest,
	MM_compileGetManagableObjectInfoResult
    },
    {
				/* Get Managable Object Notify Mask */
	MM_compileGetManagableObjectNotifyMaskRequest,
	MM_compileGetManagableObjectNotifyMaskResult
    },
    {
				/* Set Managable Object Notify Mask */
	MM_compileSetManagableObjectNotifyMaskRequest,
	NULL
    },
    {
				/* Get Destination List */
	MM_compileGetDestinationListRequest,
	MM_compileGetDestinationListResult
    },
    {
				/* Get Destination Masks */
	MM_compileGetDestinationMasksRequest,
	MM_compileGetDestinationMasksResult
    },
    {
				/* Set Destination Masks */
	MM_compileSetDestinationMasksRequest,
	NULL
    }
};


EventProtocols mm_eventProtocols[] =
{
    {
	NULL
    },
    {
				/* MaxThresholdExceededSigned */
	MM_compileThresholdEvent
    },
    {
				/* MaxThresholdExceededUnsigned */
	MM_compileThresholdEvent
    },
    {
				/* MinThresholdExceededSigned */
	MM_compileThresholdEvent
    },
    {
				/* MinThresholdExceededUnsigned */
	MM_compileThresholdEvent
    },
    {
				/* MaxThresholdReenteredSigned */
	MM_compileThresholdEvent
    },
    {
				/* MaxThresholdReenteredUnsigned */
	MM_compileThresholdEvent
    },
    {
				/* MinThresholdReenteredSigned */
	MM_compileThresholdEvent
    },
    {
				/* MinThresholdReenteredUnsigned */
	MM_compileThresholdEvent
    },
    {
				/* TimerExpired */
	MM_compileTimerExpiredEvent
    },
    {
				/* LogMessage */
	MM_compileLogMessageEvent
    },
    {
				/* ValueChangedSigned */
	MM_compileValueChangedEvent
    },
    {
				/* ValueChangedUnsigned */
	MM_compileValueChangedEvent
    },
    {
				/* ValueChangedString */
	MM_compileValueChangedEvent
    },
    {
				/* ManagableObjectChanged */
	MM_compileManagableObjectChangedEvent
    }
};


ReturnCode
mm_asnInit(void)
{
    int				i;
    ReturnCode 			rc;
    MM_Protocol			mmProtocol;
					     
    /* Initialize the ASN Module */
    if ((rc = ASN_init()) != Success)
    {
	return FAIL_RC(rc, ("MM_mgrInit(): initialize ASN module\n"));
    }

    /* Create each of the ASN Compile trees */
    for (i=0; i<DIMOF(mm_protocols); i++)
    {
	QU_INIT(&mm_protocols[i].requestQ);

	/* If there's a Request PDU for this operation... */
	if (mm_protocols[i].pfCompReq != NULL)
	{
	    /* ... compile it. */
	    if ((rc =
		 (* mm_protocols[i].pfCompReq)((unsigned char *) &mmProtocol,
					       NULL, &mmProtocol,
					       0,
					       &mm_protocols[i].requestQ)) !=
		Success)
	    {
		return Fail;
	    }
	}

	QU_INIT(&mm_protocols[i].resultQ);

	/* If there's a Result PDU for this operation... */
	if (mm_protocols[i].pfCompRes != NULL)
	{
	    /* ... compile it. */
	    if ((rc =
		 (* mm_protocols[i].pfCompRes)((unsigned char *) &mmProtocol,
					       NULL, &mmProtocol,
					       0,
					       &mm_protocols[i].resultQ)) !=
		Success)
	    {
		return Fail;
	    }
	}

    }
	    
    /* Create each of the Event ASN Compile trees */
    for (i=0; i<DIMOF(mm_eventProtocols); i++)
    {
	QU_INIT(&mm_eventProtocols[i].eventQ);

	/* If there's an Event PDU for this operation... */
	if (mm_eventProtocols[i].pfComp != NULL)
	{
	    /* ... compile it. */
	    if ((rc = (* mm_eventProtocols[i].
		       pfComp)((unsigned char *) &mmProtocol,
			       NULL, &mmProtocol,
			       0,
			       &mm_eventProtocols[i].eventQ)) !=
		Success)
	    {
		return Fail;
	    }
	}
    }
    
    return Success;
}
