/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File name: layernm.C
 *
 * Description: Network management interface.
 *
 * Functions:
 *   SuccFail erop_nm_incr(int layer, register Counter *counter, int increment)
 *   SuccFail erop_nm_event(int layer, int eventid)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: layernm.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "local.h"
#include "estd.h"
#include "extfuncs.h"
#include "nm.h"

/*  EROP_ counters  */

Counter erop_pduRetranCounter;

Counter erop_completeOperationCounter;
Counter erop_protocolErrorsCounter;

Counter erop_pduSentCounter;
Counter erop_invokePduRetranCounter;
Counter erop_badAddrCounter;
Counter erop_opRefusedCounter;


#ifdef MSDOS
/* 
 * call this from ginit to force globals to link 
 */
void
erop_force_msdos_link(void)
{
}
#endif

/*
 *  Stubs when NM_ module is not linked in.
 */

#ifdef NM


/*<
 * Function:    erop_nm_incr
 *
 * Description: Increment a counter and check against the threshold
 *
 * Arguments:   Layer, counter, increment value
 *
 * Returns:     0 if successful, -1 if unsuccessful. 
 *
>*/

SuccFail erop_nm_incr(int layer, Counter *counter, int increment)
{
    counter->count += increment;
    if (counter->threshold && counter->count >= counter->threshold->count) {
	counter->count = 0;
	return erop_nm_event(layer, counter->event);
    }
    return (SUCCESS); 
}


/*<
 * Function:    erop_nm_event
 *
 * Description: Process an event
 *
 * Arguments:   Layer, event identifier
 *
 * Returns:     0 if successful, -1 if unsuccessful. 
 *
>*/

SuccFail erop_nm_event(int layer, int eventid)
{
    return (SUCCESS); 
}
#endif







