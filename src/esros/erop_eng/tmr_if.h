/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of ESRO. An implementation of RFC-2188.
 *
 *ESRO is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *ESRO is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with ESRO; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
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
/*+
 * File name: tmr_if.h (Timer Interface)
 *
 * Description: Timer definition.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: tmr_if.h,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
 */

#ifndef _TMR_IF_H_	/*{*/
#define _TMR_IF_H_

#include "tmr.h"
#include "invoke.h"

typedef long TmrDuration;


typedef struct TmrQuHead {
	struct TmrData	*first;
	struct TmrData	*last;
} TmrQuHead;

#if 0
/* Where is the right place for this, tmr.h?? */
typedef Int SeqNu;
#define ALL_SEQ ((SeqNu) -1)
#endif

typedef struct TmrData {
    struct TmrData *next;
    struct TmrData *prev;
    struct InvokeInfo *invoke;
    Int name;
#if 0
    SeqNu seqNu;
#endif
    Int data;
} TmrData;

/* NOTYET, Needs ordering and cleanup */
typedef enum tmr_Name {
    erop_allTimer = -1,

    erop_invokePduRetranTimer =	1,
    erop_resultPduRetranTimer =	2,
    erop_lastTimer = 3,
#if 0
    EX_erop_lastTimer =	7,
#endif
    erop_inactivityTimer = 4,
    erop_refNuTimer = 5,
    erop_ackTimer = 6,
    erop_perfNoResponseTimer = 7
} tmr_Name;

LCL_XTRN LgUns erop_timeStamp;

#ifdef LINT_ARGS	/* Arg. checking enabled */

/*
extern Void erop_tmrSetValues(InvokeInfo *);
extern Void erop_tmrUpdate (InvokeInfo *, int);
extern Int  erop_timerRsp  (Ptr);
extern SuccFail erop_tmrCreate (InvokeInfo *, Int, LgInt, Int);
extern Void erop_tmrCancel (InvokeInfo *, Int);
*/
SuccFail erop_tmrCreate();
Void erop_tmrSetValues();
Void erop_tmrUpdate();
Void erop_tmrCancel();

#else

SuccFail erop_tmrCreate();
Void erop_tmrSetValues();
Void erop_tmrUpdate();
Void erop_tmrCancel();

#endif /* LINT_ARGS */

#endif	/*}*/
