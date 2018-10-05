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
 *  This software is furnished under a license and use duplication,
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

#include "estd.h"
#include "target.h"
#include "getopt.h"
#include "config.h"
#include "esro.h"
#include "tm.h"
#include "eh.h"

extern int udpSapSel;
extern int nuOfSaps;
extern int nuOfInvokes;
extern int invokePduSize;
extern int retransmitCount;

extern struct TimerValue {
    Int	    acknowledgment;
    Int	    roundtrip;
    Int	    retransmit;
    Int	    maxNSDULifeTime;
} timerValue; 

char errbuf[1024];

#if 0
extern ParamBoundary paramBoundary[]; 
#endif

#define FIRST_PARAM 0		/* move it and change it */
#define LAST_PARAM  10 		/* move it and change it */

Int
ESRO_setConf(Int param, Int value)
{
    if (param > LAST_PARAM  ||  param < FIRST_PARAM) {
	return -1;  /* INVALID_PARAM */
    }

#if 0
    if (value < paramBoundary[param].min  ||
	value > paramBoundary[param].max) {
	return -2;  /* INVALID_VALUE */
    }
#endif
	    
    switch (param) {
	case ESRO_PortNumber:
	    udpSapSel = value;
	    break;
	case ESRO_MaxSAPs:
	    nuOfSaps = value;
	    break;
	case ESRO_MaxInvokes:
	    nuOfInvokes = value;
	    break;
/*	case ESRO_MaxReference: */ /* Not yet */
	case ESRO_PduSize:
	    invokePduSize = value;
	    break;
	case ESRO_Acknowledgement:
	    timerValue.acknowledgment = value;
	    break;
	case ESRO_Roundtrip:
	    timerValue.roundtrip = value;
	    break;
	case ESRO_Retransmit:
	    timerValue.retransmit = value;
	    break;
	case ESRO_MaxNSDULifeTime:
	    timerValue.maxNSDULifeTime = value;
	    break;
	case ESRO_RetransmitCount:
	    retransmitCount = value;
    }

    return 0;
}

Int
ESRO_getConf(Int param, Int *value)
{
    if (param > LAST_PARAM  ||  param < FIRST_PARAM) {
	return -1;  /* INVALID_PARAM */
    }

    switch (param) {
	case ESRO_PortNumber:
	    *value = udpSapSel;
	     break;
	case ESRO_MaxSAPs:
	    *value = nuOfSaps;
	     break;
	case ESRO_MaxInvokes:
	    *value = nuOfInvokes;
	     break;
/*	case ESRO_MaxReference: NOT YET
	    *value = ; */
	case ESRO_PduSize:
	    *value = invokePduSize;
	     break;
	case ESRO_Acknowledgement:
	    *value = timerValue.acknowledgment;
	     break;
	case ESRO_Roundtrip:
	    *value = timerValue.roundtrip;
	     break;
	case ESRO_Retransmit:
	    *value = timerValue.retransmit;
	     break;
	case ESRO_MaxNSDULifeTime:
	    *value = timerValue.maxNSDULifeTime;
	     break;
	case ESRO_RetransmitCount:
	    *value = retransmitCount;
	     break;
    }

    return 0;
}



#ifdef TEST
main()
{
    setConf();
}
#endif
