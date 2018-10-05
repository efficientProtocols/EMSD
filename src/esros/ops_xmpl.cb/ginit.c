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
 * File: ginit.c
 * 
 * Description: Global Module routines
 * 
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: ginit.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "oe.h"
#include "target.h"
#include "eh.h"
#include "tm.h"
#include "tmr.h"
#include "esro_cb.h"
#include "du.h"
#include "udp_po.h"
#include "sch.h"

Void G_init(void);
Void G_sigIntr(Int unUsed);

#ifdef UDP_PO_
extern Bool UDP_noLogSw;		/* Logging control switch */
extern char eropEngOutLog[256]; 	/* ESROS Engine Out   Log file */
extern char eropEngErrLog[256];		/* ESROS Engine Error Log file */
#endif

extern Int clockPeriod;			/* Clock Period */

#ifdef MSDOS
volatile Int OS_isrActive;
#endif

PUBLIC DU_Pool *G_duMainPool;		/* DU Module Main Pool */

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;		/* Global Module Trace descriptor */
#endif

extern char pConfigFile[512];		/* Configuration File name */


/*<
 * Function:    G_init
 *
 * Description: Initialize.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

Void
G_init(void)
{
    if (TM_OPEN(G_tmDesc, "G_") == NULL) {
	EH_problem("G_init: TM_open G_ failed");
    }

#ifdef MSDOS
    {
    extern void erop_force_msdos_link(void);
    erop_force_msdos_link();
    }
#endif
    
#ifndef WINDOWS
    signal(SIGINT, G_sigIntr);
#endif

    G_duMainPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS); /* build buf pool */

#ifdef UDP_PO_
    TM_TRACE((G_tmDesc, TM_ENTER, 
    	      "**** ops_xmpl.one.cb compiled <<<WITH UDP_PO>>> *****\n"));
    if (!UDP_noLogSw) {
    	UDP_PO_init (eropEngErrLog, eropEngOutLog);
    	TM_TRACE((G_tmDesc, TM_ENTER, 
		 "G_init: ===> Logging activated\n"));
    }
#else
    TM_TRACE((G_tmDesc, TM_ENTER, 
    	     "**** ops_xmpl.one.cb main compiled <<<WITHOUT UDP_PO>>> *****\n"));
#endif
    SCH_init(MAX_SCH_TASKS);			/* Initialize Scheduler */

    ESRO_CB_init(pConfigFile);			/* Initialize ESROS */

    TMR_init(NUMBER_OF_TIMERS, CLOCK_PERIOD);	/* Initialize Timer Module */
    
    (void) TM_VALIDATE();			/* Validate Trace Modules */

    TMR_startClockInterrupt(CLOCK_PERIOD);	/* Start Clock Interrupt */

} /* G_init(void) */

