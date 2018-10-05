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
 * SCCS Revision: @(#)tmr.h	1.3    Released: 1/31/88
 */

#ifndef _TMR_H_	/*{*/
#define _TMR_H_

typedef LgUns TMR_MSecs;    /* Millisecodes */
extern LgUns tmr_freeCnt;
#define TMR_getFreeCnt() (tmr_freeCnt)

typedef Ptr TMR_Desc;

#ifdef LINT_ARGS	/* Arg. checking enabled */

void TMR_init(int , int);
void TMR_clock(void);
void TMR_poll(void);
char  *TMR_create(long ,int ( *)());
void TMR_cancel(char  *);
char  *TMR_getData(char  *);
TMR_Desc TMR_getDesc(Ptr );
long TMR_diff(unsigned long ,unsigned long );

/* Non Portable Timer Interface */
SuccFail TMR_startClockInterrupt(Int);
Void TMR_stopClockInterrupt();

#else

extern  Void TMR_init();
extern Void TMR_clock();
extern Void TMR_poll();
extern TMR_Desc TMR_create();
extern Void TMR_cancel();
extern Ptr TMR_getData();
extern TMR_Desc TMR_getDesc();
extern LgInt TMR_diff();

/* Non Portable Timer Interface */
SuccFail TMR_startClockInterrupt();
Void TMR_stopClockInterrupt();

#endif /* LINT_ARGS */


#endif	/*}*/
