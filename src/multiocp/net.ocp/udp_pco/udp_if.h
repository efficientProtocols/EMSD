/*
 * Copyright (C) 1995,1999  Neda Communications, Inc. All rights reserved.
 *
 * This software is furnished under a license and use, duplication,
 * disclosure and all other uses are restricted to the rights specified
 * in the written license between the licensee and copyright holders.
 *
*/
/*+
 * File: udp_if.h
 *
 * Description: UDP Interface
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: udp_if.h,v 1.5 2002/10/25 19:37:54 mohsen Exp $
 */

#ifndef _UDP_IF_H_
#define _UDP_IF_H_

#include "addr.h"
#include "du.h"

typedef Void *UDP_SapDesc;
LCL_XTRN UDP_SapDesc erop_udpSapDesc;


#ifdef LINT_ARGS	/* Arg. checking enabled */

extern SuccFail UDP_init (Int);
extern SuccFail UDP_dataReq (UDP_SapDesc locSapDesc, T_SapSel *remTsapSel, 
			     N_SapAddr *remNsapAddr, DU_View udpSdu);
extern SuccFail UDP_poll (void);
extern SuccFail UDP_sapUnbind (T_SapSel *sapSel);
extern UDP_SapDesc UDP_sapBind (T_SapSel *sapSel, 
                                int (*dataInd) (T_SapSel *, N_SapAddr *, 
                                                T_SapSel *, N_SapAddr *, 
                                                DU_View));

#else
extern SuccFail UDP_init ();
extern SuccFail UDP_dataReq ();
extern SuccFail UDP_poll ();
extern SuccFail UDP_sapUnbind ();
extern UDP_SapDesc UDP_sapBind ();

#endif /* LINT_ARGS */

#endif







