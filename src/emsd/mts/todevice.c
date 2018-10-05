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
 * Copyright (C) 1995,1997  Neda Communications, Inc. All rights reserved.
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

static char *rcs = "$Id: todevice.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";


#include "estd.h"
#include "eh.h"
#include "mm.h"
#include "fsm.h"
#include "mtsua.h"
#include "mts.h"
#include "subpro.h"
#include "msgstore.h"

ReturnCode
mts_tryDelivery(void * hTimer, void * hUserData1, void * hUserData2)
{
    ReturnCode			rc                 = 0;
    OS_Uint32                   count              = 0;
    char *			pControlFileName   = hUserData1;
    MTS_Operation *		pOperation         = NULL;
    MTSUA_Recipient *		pRecip             = NULL;
    MTSUA_ControlFileData *	pControlFileData   = NULL;
    struct DQ_ControlData *     pMsg               = NULL;
    struct SUB_DeviceProfile *  pDev               = NULL;
    

    TM_TRACE((globals.hTM, MTS_TRACE_POLL, "mts_tryDelivery(): Start serving delivery queues"));

    /* for each recipient delivery queue */
    while (pMsg = msg_getMessage()) {
      TM_TRACE((globals.hTM, MTS_TRACE_MSG, "Delivering MID '%s', To: %lu",
		pMsg->messageId, pMsg->pRecipQueue->recipient));

      /* Allocate an operation structure */
      if ((rc = mts_allocOperation(&pOperation)) != Success)
	{
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Error allocating operation"));
	  MM_logMessage(globals.hMMLog, globals.tmpbuf,
			"MTS could not create operation structure; resource error\n");
	}

      /*** set up the control data ***/
      pOperation->pControlData = pMsg;

      pDev = &pMsg->pRecipQueue->deviceProfile;
      memcpy (&pOperation->nsap, &pDev->nsap, sizeof pOperation->nsap);

      /* check for valid msg */
      if (pDev->protocolVersionMajor < 1 || pDev->protocolVersionMajor > 100) {
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Protocol version invalid (%d.%d), default to v1.1",
		    pDev->protocolVersionMajor, pDev->protocolVersionMinor));
	  pDev->protocolVersionMajor = 1;
	  pDev->protocolVersionMinor = 1;
      }
      /* Preprocess the delivery */
      if ((rc = mts_preprocessDeliverRequest(pOperation)) != Success)
	{
	  TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "mts_tryDelivery(): Pre-processing failed; [%d]", rc));
	  msg_deleteMessage (pMsg);
	  mts_freeOperation(pOperation); /*** fnh ***/
	  return rc;
	}

      count++;
	    
      /* Generate an event to the finite state machine */
      FSM_generateEvent(pOperation->hFsm, MTS_FsmEvent_MessageDeliverRequest);
    }
    if (count > 0) {
      TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "mts_tryDelivery(): began %d deliveries", count));
    }
    return Success;
}
