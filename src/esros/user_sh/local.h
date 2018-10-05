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
 * File name: local.h
 *
 * Description: local definitions of User shell (ESROS)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: local.h,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
 */

#ifndef _LOCAL_H_
#define _LOCAL_H_

#ifdef TM_ENABLED
LCL_XTRN TM_ModuleCB * ESRO_modCB;
#endif

#define ESRO_TRPRIM	TM_BIT1
#define ESRO_TREVENT	TM_BIT2

#define INACTIVE_STATE 0
#define ACTIVE_STATE 1

/* Information associated with a Receive Request */

/* Invoke Control Block */
typedef struct InvokeCB {
    struct InvokeCB *next;
    struct InvokeCB *prev;
    Int state;
    struct EsroSapCB *sapCB;
} InvokeCB;

/* Sap Control Block */
typedef struct EsroSapCB {
    ESRO_SapSel sapSel;
    USQ_PrimQuInfo *primQu;
    Int sapDesc;
    Int state;
    QU_Head	invokeQuHead;	/* Invokes Associated with this SAP */
    QU_Head	evQuHead;	/*  Events queued for this SAP */
} EsroSapCB;

LCL_XTRN  USQ_PrimQuInfo *SUS_primQu;

#endif	/* _LOCAL_H_ */
