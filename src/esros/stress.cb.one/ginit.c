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
#include "du.h"
#include "udp_po.h"
#include "esro.h"

Void UDP_init();
SuccFail EROP_init();
Void TMR_init();
Int SCH_init(int);
SuccFail TMR_startClockInterrupt(Int);

Void G_init(void);
Void G_sigIntr(Int unUsed);


#ifdef UDP_PO_
extern Bool UDP_noLogSw;

extern char eropEngOutLog[512];
extern char eropEngErrLog[512];
#endif

extern Int clockPeriod;

#ifdef MSDOS
volatile Int OS_isrActive;
#endif

PUBLIC DU_Pool *G_duMainPool;

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif


