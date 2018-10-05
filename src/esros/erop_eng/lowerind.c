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
 * File: lowerind.c
 *
 * Description: This file contains the interface event routines between
 *              this and the lower layer.
 *
 * Functions:   lower_dataInd(T_SapSel *remTsapSel,
 *  	                      N_SapAddr *remNsapAddr,
 *	                      T_SapSel *locTsapSel,
 *	                      N_SapAddr *locNsapAddr,
 *	                      DU_View data)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: lowerind.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "eh.h"
#include "eropdu.h"

#include "local.h"
#include "extfuncs.h"

extern Pdu erop_pdu;
extern InvokeInfo *erop_inPdu (DU_View bp); 


/*<
 * Function:    lower_dataInd
 *
 * Description: This function is performed from the lower layer to indicate 
 *              the arrival of a SDU.
 *
 * Arguments:   Transport SAP, network SAP, data unit.
 *
 * Returns:     	
 *
>*/

LOCAL Int
lower_dataInd(T_SapSel *remTsapSel,
	      N_SapAddr *remNsapAddr,
	      T_SapSel *locTsapSel,
	      N_SapAddr *locNsapAddr,
	      DU_View data)
{
    InvokeInfo *invoke;
    Int gotVal;

    /* Save network address of source and destination */

    erop_pdu.fromTsapSel  = remTsapSel;
    erop_pdu.fromNsapAddr = remNsapAddr;

    erop_pdu.toTsapSel    = locTsapSel;
    erop_pdu.toNsapAddr   = locNsapAddr;

    if ((invoke = erop_inPdu(data)) == (InvokeInfo *) NULL) {
	TM_TRACE((EROP_modCB, TM_ENTER, 
    		 "lower_dataInd: SAP not active or received PDU can "
		 "not be associated with operation\n"));

/* NOTYET: send failure PDU? */

	if (data) {
	    TM_TRACE((DU_modCB, DU_MALLOC, 
                     "lower_dataInd: DU_free: data=0x%lx\n", data));
	    DU_free(data);
	}
    	return (FAIL);
    }
	
    switch ( erop_pdu.pdutype ) {
    case INVOKE_PDU:
	gotVal = FSM_runMachine((Void *)invoke,  lsfsm_EvtPduInvoke);
	break;

    case RESULT_PDU:
    case ERROR_PDU:
	gotVal = FSM_runMachine((Void *)invoke,  lsfsm_EvtPduResult);
	break;

    case ACK_PDU:
 	gotVal = FSM_runMachine((Void *)invoke,  lsfsm_EvtPduAck);
	break;

    case FAILURE_PDU:
        gotVal = FSM_runMachine((Void *)invoke,  lsfsm_EvtPduFailure);
	break;

    default:
	EH_problem("lower_dataInd: Invalid PDU type");
	return (FAIL);
    }

    if (gotVal < 0) {
	TM_TRACE((EROP_modCB, TM_ENTER, 
                 "lower_dataInd: FSM_runMachine returned negative value: %d\n",
	         gotVal));

        if (erop_pdu.data != (DU_View) NULL) {

	    TM_TRACE((DU_modCB, DU_MALLOC, 
                     "lower_dataInd: DU_free: erop_pdu.data=0x%lx\n",
		     erop_pdu.data));

	    DU_free(erop_pdu.data);
	}
    }

    return (SUCCESS);

} /* lower_dataInd() */
