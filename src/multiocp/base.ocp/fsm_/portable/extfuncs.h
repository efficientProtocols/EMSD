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

#include "fsm.h"

#ifdef LINT_ARGS	/* Arg. checking enabled */

extern PUBLIC FSM_TransDiagram *fsmex_transDiag (void);
extern Int tr_badEventIgnore(Void *machine, Void *userData, FSM_EventId evtId);
extern Int tr_idleTo1(Void *, Void *userData, FSM_EventId );
extern Int tr_1ToIdle(Void *, Void *userData, FSM_EventId );
extern Int tr_1To2(Void *, Void *userData, FSM_EventId );
extern Int tr_2ToIdle(Void *, Void *userData, FSM_EventId );
extern Int tr_1To3(Void *, Void *userData, FSM_EventId );
extern Int tr_3ToIdle(Void *, Void *userData, FSM_EventId );

#else

extern PUBLIC FSM_TransDiagram *erop_CLPerformerTransDiag ();
extern Int tr_badEventIgnore();
extern Int tr_idleTo1();
extern Int tr_1ToIdle();
extern Int tr_1To2();
extern Int tr_2ToIdle();

#endif
