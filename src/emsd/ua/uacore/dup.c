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

/*
 * The sequence is added or stripped from PDUs with an operationValue within
 * a given range.  If an incoming PDU has a sequence value within the previous
 * one-half of the sequence cycle, the packet is dropped.
 */

#include "estd.h"
#include "du.h"
#include "addr.h"
#include "inetaddr.h"
#include "tm.h"
#include "eh.h"
#include "buf.h"
#include "fsm.h"
#include "esro_cb.h"
#include "emsd.h"
#include "tm.h"
#include "dup.h"

#define	NSAP_ADDR_TO_LONG(x)	((x)->addr[0] + ((x)->addr[1]<<8) + ((x)->addr[2]<<16) + ((x)->addr[3]<<24))


/*
 * Global Variables
 *
 * All global variables for DUP go inside of MTS_Globals
 */
DUP_Globals		DUP_globals;

/*
 * Check an incoming PDU for a duplicate detection byte, stripping
 * it out if one is present.
 *
 * used by mts/esrosevent.c for incoming events
 */
ReturnCode
DUP_invokeIn (DU_View hView, ESRO_OperationValue operationValue, N_SapAddr *pRemoteNsap)
{
    if (operationValue > 31 && operationValue < 128)
      {
	TM_TRACE((DUP_globals.hTM, DUP_TRACE_ACTIVITY, 
		 "Dupe detection octet stripped: %d", 0));

	DU_strip (hView, 1);
	return 1;
      }
    return 0;
}

#ifdef NOT_YET
ReturnCode
DUP_checkIn (OS_Uint8 sequence, OS_Uint32 nsapAddr)
{
    /* find entry for this NSAP */
    for (pElement = QU_FIRST(globals);
	 !QU_EQUAL (pElement, pHead);
	 pElement = QU_NEXT(pElement))
      {
	break;
      }
    /* add entry is not found */
    /* compare sequence to current sequence */

    return (0);
}
#endif /* NOT_YET */

/*
 * Insert a "duplicate detection" byte before the PDU if the
 * operation value is within a specific range
 */
/* used by mts/mtssubr before an invoke is issued */
ReturnCode
DUP_invokeOut (DU_View hView, ESRO_OperationValue operationValue, N_SapAddr *pRemoteNsap)
{
    static unsigned char  invokeCount = '\0';

    if (operationValue > 31 && operationValue < 128)
      {
	TM_TRACE((DUP_globals.hTM, DUP_TRACE_ACTIVITY, 
		 "Dupe detection octet added: %d", invokeCount));
	DU_prepend(hView, 1);
	OS_copy(DU_data(hView), &invokeCount, 1);
	return Success;
      }
    return Fail;
}

#ifdef NOTUSED
/* used by uad/emsd-uad */
static int
DUP_invokeIn (ESRO_Event *ep, N_SapAddr * pRemoteNsap)
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
#endif
