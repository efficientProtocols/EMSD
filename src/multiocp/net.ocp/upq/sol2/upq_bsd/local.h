/*
 * This file is part of the Open C Platform (OCP) Library. OCP is a
 * portable library for development of Data Communication Applications.
 *
 * Copyright (C) 1995 Neda Communications, Inc.
 *	Prepared by Mohsen Banan (mohsen@neda.com)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.  This library is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.  You should have received a copy of the GNU
 * Library General Public License along with this library; if not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
*/

/*+
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: local.h,v 1.9 1996/03/27 22:30:48 fletch Exp $
 */

#ifndef _LOCAL_H_	/*{*/
#define _LOCAL_H_

#include "estd.h"
#include "oe.h"
#include "tm.h"

LCL_XTRN TM_ModuleCB *IMQ_modCB;

#ifdef LINT_ARGS	/* Arg. checking enabled */

#include "psq.h"
#include "usq.h"
#include "sp_shell.h"

#include "extfuncs.h"

/* extern PUBLIC Bool tm_trace(); */
extern void bcopy();
extern char *mktemp(char *template);
extern write();
extern read();
#ifndef DJL
extern void bzero(char *sp, int len);
extern Void free(Void *ptr);
#endif
extern void *memset();
extern Void eh_unixProblem();

extern Int LOPS_userIn(LOPQ_Primitive *prim, Int len);

#else

#endif /* LINT_ARGS */

#endif	/*}*/
