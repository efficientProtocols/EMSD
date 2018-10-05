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
 * File name: sp_shell.h
 *
 * Description:  Service Provider Shell Interface
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: sp_shell.h,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
 */

#ifndef _SP_SHELL_
#define _SP_SHELL_

#include "tm.h"
#include "queue.h"
#include "addr.h"

#include "target.h"

#define SP_BASE	0		/* Service Provider Shell QuId base */

/* OBSOLETE part: these values should go to include file of the 
	   corresponding layer */

#define DS_A_BASE     20	/* DataLink Action primitive Qu Elem Ids */
#define DS_E_BASE     140	/* DataLink Event primitive Qu Elem Ids */

#define NS_A_BASE     40	/* Network Action primitive Qu Elem Ids */
#define NS_E_BASE     160	/* Network Event primitive Qu Elem Ids */

#define TS_A_BASE     60	/* Transport Action primitive Qu Elem Ids */
#define TS_E_BASE     180	/* Transport Event primitive Qu Elem Ids */

#define LOPQ_A_BASE   80	/* ESROS Action primitive Qu Elem Ids */
#define LOPQ_E_BASE   200	/* ESROS Event primitive Qu Elem Ids */

#define BAD_LOPQ_ACTION    100
#define BAD_ACTION    BAD_LOPQ_ACTION  /* NOT YET backwards compatibility 
					  remove after integration of NMM_ module */
#define BAD_LOPQ_EVENT     220

#define NOPQ_A_BASE    100       /* Network Management Action primitive Qu Elem Ids */
#define NOPQ_E_BASE    220       /* Network Management Event primitive Qu Elem Ids */

#define BAD_NOPQ_ACTION    120
#define BAD_NOPQ_EVENT     240

/* end of OBSOLETE part */

#define SP_CHNLNAMELEN	64
 
/* Add a Primitive Queue */
typedef struct SP_QuAddReq {
    Char chnlName[SP_CHNLNAMELEN];
} SP_QuAddReq;
#define SP_QUADDREQ (SP_BASE+1)

typedef struct SP_QuAddCnf {
    Int status;
} SP_QuAddCnf;
#define SP_QUADDCNF (SP_BASE+2)

typedef union UnAction {
    SP_QuAddReq quAddReq;
    SP_QuAddCnf quAddCnf;
} UnAction;

typedef struct SP_Action {
    long mtype;			/* Sys V comaptability */
    int type;
    UnAction un;
} SP_Action;

#define TSZ  (sizeof(SP_Action) - sizeof(UnAction))
#define SP_getSize(type)  ((type==SP_QUADDREQ) ? (sizeof(SP_QuAddReq)+TSZ) :\
			  (type==SP_QUADDCNF) ? (sizeof(SP_QuAddCnf)+TSZ) :\
			  -1)
#undef TYPESIZE

#ifdef LINT_ARGS
extern PUBLIC SuccFail LOPS_init (void);
#else
extern PUBLIC SuccFail LOPS_init ();
#endif

#endif	/* _SP_SHELL_ */
