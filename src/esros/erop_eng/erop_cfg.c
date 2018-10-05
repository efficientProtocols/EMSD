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
 * File: eropcfg.c
 *
 * Description: This module contains all initialization routines.
 *              EROP_init: Initialize protocol engine.
 *
 * Functions:
 *   EROP_init(Int udpSapSel, Int nuOfSaps, Int nuOfInvokes, Int invokePduSize)
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: erop_cfg.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "nm.h"
#include "layernm.h"


#include "tm.h"
#include "inetaddr.h"
#include "erop_cfg.h"
#include "udp_if.h"
#include "fsm.h"
#include "fsmtrans.h"

#include "local.h"
#include "extfuncs.h"
#include "target.h"

extern Void emptyFunctionHandler(InvokeInfo * pInvokeInfo);

/*----- Global Var Definition -----*/

Int udpSapSel = EROP_K_UdpSapSel;

LOCAL TM_ModuleCB *EROP_modCB;

Int nuOfSaps    = EROP_SAPS;
Int nuOfInvokes = EROP_INVOKES;
Int invokePduSize = INVOKE_PDU_SIZE;

UDP_SapDesc erop_udpSapDesc;

#ifdef AUTHENTICATE_DOMAIN
N_SapAddr authorizedDomains[] = {
  {4, {198, 62, 92, 0}}, 
  {0, {0, 0, 0, 0}}
};
#endif



/*<
 * Function:    EROP_init
 *
 * Description: Initialize protocol engine
 *
 * Arguments:   None.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
>*/

PUBLIC SuccFail
EROP_init(Int udpSapSel, Int nuOfSaps, Int nuOfInvokes, Int invokePduSize)
{
    static Bool virgin = TRUE;
    
    if ( !virgin ) {
	return 0;
    }
    virgin = FALSE;

#ifdef TM_ENABLED
    TM_INIT();

    if (TM_OPEN(EROP_modCB, "EROP_") == NULL) {
    	EH_problem("EROP_init: TM_open EROP_ failed");
	return ( FAIL );
    }
#endif

    erop_invokeInit(nuOfInvokes);
    erop_sapInit(nuOfSaps);

    FSM_TRANSDIAG_init();
    FSM_init();

    UDP_init(MAX_SAPS);

    /* Make all local reference numbers assignable  */
    erop_refInit(NREFS);

#ifdef FUTURE
    erop_completeOperationCounter.count = 0;	
#endif

    {
	static T_SapSel locTsapSel;

	INET_portNuToTsapSel(udpSapSel, &locTsapSel);

	if ((erop_udpSapDesc = UDP_sapBind(&locTsapSel, lower_dataInd)) 
	    == NULL) {
	    return (FAIL);
	}
    }

    erop_freeLopsInvoke = emptyFunctionHandler;

    return ( SUCCESS );
}


/*<
 * Function:    emptyFunctionHandler
 *
 * Description: Empty Function handler
 *
 * Arguments:	Invoke info structure.
 *
 * Returns: 	None.
 *
>*/

Void
emptyFunctionHandler(InvokeInfo * pInvokeInfo)
{
#if 0
    TM_TRACE((EROP_modCB, TM_ENTER, 
    	     "WARNING: Function handler is empty (Don't worry)\n"));
#endif
}

#ifdef AUTHENTICATE_DOMAIN
 
/*<
 * Function:    erop_authenticateIP
 *
 * Description: Authenticate the IP address
 *
 * Arguments:	IP address
 *
 * Returns: 	0 if authenticated, -1 otherwise.
 *
>*/

SuccFail
erop_authenticateIP(N_SapAddr *remNsap) 
{
    extern N_SapAddr authorizedDomains[];
    int i;

    for (i = 0; authorizedDomains[i].len; i++) {
    	if (!SAP_nSapDomainCmp(remNsap, &authorizedDomains[i])) {
	    break;
    	}
    }

    if (!authorizedDomains[i].len) {
	EH_fatal("\nProgram is talking to unauthorized network address\n");
	return -1;
    }

    return 0;

} /* erop_authenticateIP() */
#endif
