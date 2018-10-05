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

/*
 * Author: Derrell Lipman
 * History:
 *
 */

/*
 * RCS Revision: $Id: tmr.h,v 1.4 1998/01/31 01:59:06 mohsen Exp $
 */

#ifndef __TMR_H__
#define	__TMR_H__

#include "estd.h"

/* Macro to convert seconds to milliseconds */
#define	TMR_SECONDS(n)	(((OS_Uint32) n) * 1000)


/* Types needed for backward compatibility */
typedef void *	    TMR_Desc;

void
TMR_init(OS_Uint16 numberOfTimers, OS_Uint16 millisecondsPerTick);


ReturnCode
TMR_start(OS_Uint32 milliseconds,
	  void * hUserData1,
	  void * hUserData2,
	  ReturnCode (* pfHandler)(void * hTimer,
				   void * hUserData1,
				   void * hUserData2),
	  void ** phTimer);


void
TMR_stop(void * hTimer);


ReturnCode
TMR_processQueue(OS_Boolean * pProcessedSomething);

void
TMR_startClockInterruptPlus(void (* pfHandler)(void));

SuccFail
TMR_startClockInterrupt(Int period);


void
TMR_stopClockInterrupt(void);


void
TMR_poll(void);


void *
TMR_create(LgInt milliseconds, Int (*pfHandler) (/* ??? */));


void
TMR_cancel(void * hTimer);


void
TMR_setLocalDataSize(OS_Uint16 size);


OS_Uint16
TMR_getLocalDataSize(void);


void *
TMR_getData(void * hTimer);


void *
TMR_getDesc(void * pLocalData);


OS_Uint32
TMR_diff(OS_Uint32 time1, OS_Uint32 time2);


#endif /* __TMR_H__ */
