/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*+
 * File: fsmtrans.h
 *
 * Description:
 *	This File contains the definititions related to a transition diagram 
 *      represented by the Module Identifer  FSM_TRANSDIAG.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: fsmtrans.h,v 1.3 1996/02/04 00:11:33 mohsen Exp $
 */

#ifndef _FSM_TRANSDIAG_H
#define _FSM_TRANSDIAG_H

#include "tm.h"

EXTERN TM_ModuleCB *FSM_TRANSDIAG_modCB;

#define FSM_TRANSDIAG_TMEXEC	TM_BIT2
#define FSM_TRANSDIAG_TMGEN	TM_BIT3
#define FSM_TRANSDIAG_TMFUNC	TM_BIT4

typedef struct FSM_TransDiagram {
    QU_ELEMENT;
    FSM_State  * state;
    String	name;
} FSM_TransDiagram;

#ifdef LINT_ARGS /*{*/

Void FSM_TRANSDIAG_init (void);
SuccFail FSM_TRANSDIAG_load (FSM_Machine *machine, FSM_TransDiagram *transDiag);
SuccFail FSM_TRANSDIAG_resetMachine (FSM_Machine *machine);
extern Void *FSM_TRANSDIAG_create (String , FSM_State *);

#else

Void FSM_TRANSDIAG_init ();
SuccFail FSM_TRANSDIAG_load ();
SuccFail FSM_TRANSDIAG_resetMachine ();
extern Void *FSM_TRANSDIAG_create ();

#endif

#endif
