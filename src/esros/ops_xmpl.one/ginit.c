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
 * Description: Global module routines
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
#include "esro.h"
#include "du.h"
#include "udp_po.h"

Void UDP_init();
Void LOPS_init();
SuccFail EROP_init();
Void TMR_init();
Int SCH_init(int);
SuccFail TMR_startClockInterrupt(Int);

Void G_init(void);
Void G_sigIntr(Int unUsed);

#undef UDP_PO

#ifdef UDP_PO_
extern Bool UDP_noLogSw;

char eropEngOutLog[256] = "erop_out.log"; 
char eropEngErrLog[256] = "erop_err.log"; 
#endif

extern Int clockPeriod;

#ifdef MSDOS
volatile Int OS_isrActive;
#endif

PUBLIC DU_Pool *G_duMainPool;

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif

extern Int udpSapSel;
extern Int nuOfSaps;
extern Int nuOfInvokes;
extern Int invokePduSize;

extern char pConfigFile[512];


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
#ifdef TM_ENABLED
    if (! TM_OPEN(G_tmDesc, "G_")) {
	EH_problem("G_init: TM_open G_ failed");
    }
#endif

#if defined(MSDOS) || defined(WINDOWS)
    {
    extern void erop_force_msdos_link(void);
    erop_force_msdos_link();
    }
#endif
    
#ifndef MSDOS
    signal(SIGINT, G_sigIntr);
#endif

    G_duMainPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS); /* build buf pool */

#ifdef UDP_PO_
    fprintf(stderr, "\n**** ops_xmpl.one compiled <<<WITH UDP_PO>>> *****\n");
    if (!UDP_noLogSw) {
    	UDP_PO_init (eropEngErrLog, eropEngOutLog);
	fprintf(stdout, "===> Logging activated\n");
    }
#else
    fprintf(stderr, "\n**** ops_xmpl.one main compiled <<<WITHOUT UDP_PO>>> *****\n");
#endif

    SCH_init(MAX_SCH_TASKS);
    UDP_init(MAX_SAPS);

    if (EROP_init(udpSapSel, nuOfSaps, nuOfInvokes, invokePduSize) == FAIL) {/* Protocol Engine */
	fprintf (stderr, "%s: Engine initialization (EROP_init) failed\n", 
		 "G_init");		/* improve: give error value */
 	exit(13);
    }

    LOPS_init();
    TMR_init(NUMBER_OF_TIMERS, CLOCK_PERIOD);
    ESRO_init(pConfigFile);

#ifdef TM_ENABLED
	TM_validate();
#endif

    TMR_startClockInterrupt(CLOCK_PERIOD);
}

