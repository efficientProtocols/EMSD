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
 * File name: sch.h (Scheduler)
 *
 * Description: Scheduler header file.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: sch.h,v 1.14 1998/01/31 01:59:05 mohsen Exp $
 */

#ifndef _SCH_H_	/*{*/
#define _SCH_H_

#include "estd.h"

#define SCH_PSEUDO_EVENT -1

/* return values */
#define SCH_INVALID_MAX_SCH_INFO -1
#define SCH_NOT_VIRGIN 		 13

#define TM_SCH_ERROR    (1 << 1)
#define TM_SCH_BLOCKED  (1 << 2)
#define TM_SCH_SELECT   (1 << 3)

typedef unsigned long int SCH_Event;

#ifdef LINT_ARGS /*{*/

extern SuccFail SCH_init(Int maxSchInfo);
extern Void SCH_term(Void);
#ifdef TM_ENABLED
extern Int SCH_submit(Void (*func)(), Void *, SCH_Event, String);
#else
extern Int SCH_submit(Void (*func)(), Void *, SCH_Event);
#endif
extern Int SCH_block(Void);
extern Int SCH_run(Void);
extern SuccFail SCH_taskDelete(SCH_Event event, Void (*func)(), Void *arg);
extern SuccFail SCH_allTaskDelete(SCH_Event event, Void (*func)(), Void *arg);

#else

extern SuccFail SCH_init();
extern Void SCH_term();
extern Int SCH_submit();
extern Int SCH_block();
extern Int SCH_run();
extern SuccFail SCH_taskDelete();
extern SuccFail SCH_allTaskDelete();

#endif


#endif
	
