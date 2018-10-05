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


#include "estd.h"	/* Type definitions: Int, Char, ...             */
#include "addr.h"	/* SAP struct definitions: ESRO_SapSel, ...     */
#include "du.h"	  	/* DU module  definitions:                      */
#include "eh.h"	  	/* Exeption Handler definitions:                */
#include "tm.h"	  	/* Trace Module definitions:                   	*/
#include "sch.h"	/* Scheduler Module definitions:               	*/
#include "config.h"
#include "target.h"	
#include "esro_cb.h"	

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: esroinit.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#define EROP_K_UdpSapSel 2002	/* ESROS Port Number (UDP SAP) */
#define EROP_SAPS        200	/* Max number of active SAPs */
#define EROP_INVOKES     200	/* Max number of concurrent invocations*/
#define NREFS 256  		/* Max number of reference numbers */
#define INVOKE_PDU_SIZE  1500	/* Max invoke PDU size (bytes) */

extern Int udpSapSel;		/* UDP SAP Selector */
extern Int nuOfSaps;		/* Max number of SAPs */
extern Int nuOfInvokes;		/* Max number of Invocations */
extern Int invokePduSize;	/* Invoke PDU size */

#ifdef UDP_PO_			/* UDP Point of Observation */
extern Bool UDP_noLogSw;	/* Logging control switch */

char eropEngErrLog[512] ="erop_err.log";	/* EROP Engine Error Log */
char eropEngOutLog[512] ="erop_out.log";	/* EROP Engine Output log */
#endif

extern UDP_init();		/* UDP init */
extern UDP_PO_init();		/* UDP Point of Observation initialization */
extern getConf();		/* Get ESROS Configuration */
extern EROP_init();		/* ESROS Engine initialization */


/*<
 * Function:    ESRO_CB_init
 *
 * Description: Initialize ESROS.
 *
 * Arguments:   None.
 *
 * Returns:     0 on successful completion, a negative error value on
 *              unsuccessful completion.
 *
>*/

#ifdef TM_ENABLED
LOCAL TM_ModuleCB *ESRO_modCB;		/* Trace Module Control Block */
#endif

PUBLIC ESRO_RetVal
ESRO_CB_init (char *pConfigFile)
{
    static Bool virgin = TRUE;
    
    if ( !virgin ) {
        EH_problem("WARNING: ESRO_CB_init() is called "
		   "more than once (harmless)\n");
    	return ( SUCCESS );
    }
    virgin = FALSE;

    if (pConfigFile == NULL) {
	EH_problem("ESRO_CB_init: Config file name not passed "
		   "to ESRO_CB_init\n");
	return (FAIL);
    }

#ifdef TM_ENABLED
    TM_INIT();

    if (TM_OPEN(ESRO_modCB, "ESRO_") == NULL) {
	EH_problem("ESRO_CB_init:      Error in TM_open ESRO_");
	return ( FAIL );
    }
    TM_TRACE((ESRO_modCB, TM_ENTER, 
	     "ESRO_CB_init(call back/one process):  ESRO_ opened\n"));

    TM_TRACE((ESRO_modCB, TM_ENTER, 
    	     "**** ESRO_CB_init for no user/provider shell *****\n"));
#endif

#ifdef UDP_PO_
    TM_TRACE((ESRO_modCB, TM_ENTER, 
    	     "**** ESRO_CB_init compiled <<<WITH UDP_PO>>> *****\n"));
    if (!UDP_noLogSw) {
        if (UDP_PO_init(eropEngErrLog, eropEngOutLog) == -2) { 
    	    TM_TRACE((ESRO_modCB, TM_ENTER, 
	             "WARNING: ESROS application didn't initialize "
	             "UDP_PO module.\n"));
        }
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "ESRO_CB_init: ===> Logging activated\n"));
    }
#else
    TM_TRACE((ESRO_modCB, TM_ENTER, 
    	     "**** ESRO_CB_init compiled <<<WITHOUT UDP_PO>>> *****\n"));
#endif

#ifndef MSDOS			 /* initialize scheduler */
    if (SCH_init(MAX_SCH_TASKS) ==  SCH_NOT_VIRGIN) { 
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "WARNING: ESROS application didn't initialize scheduler.\n"));
    }
#endif

    udpSapSel     = EROP_K_UdpSapSel;		/* UDP SAP Selector */
    nuOfSaps      = EROP_SAPS;			/* Max number of SAPS */
    nuOfInvokes   = EROP_INVOKES;		/* Max number of invocations */
    invokePduSize = INVOKE_PDU_SIZE;		/* Max invoke PDU Size */

    /* Initialize configuration module */
    if (CONFIG_init() == -2) { 
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "WARNING: ESROS application didn't initialize Config module.\n"));
    }
    getConf(pConfigFile);	/* get configuration parameters from file */

    /* Initialize Protocol Engine */
    if (EROP_init(udpSapSel, nuOfSaps, nuOfInvokes, invokePduSize) == FAIL) {	
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "%s: Engine initialization (EROP_init) failed\n", 
		 "ESRO_CB_init"));		/* improve: give error value */
	EH_problem("Engine initialization (EROP_init) failed\n");
	exit(13);
    }

    UDP_init(MAX_SAPS);   		/* initialize UDP module */

    TM_TRACE((ESRO_modCB, TM_ENTER, 
    	     "**** ESRO_CB_init compiled <<<WITH TM_ENABLED>>> *****\n"));

    return ( SUCCESS );

} /* ESRO_CB_init() */
 
