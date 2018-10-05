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
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)tmr_start.c	1.1    Released: 2/4/88";
#endif /*}*/

#include <stdio.h>
#include "estd.h"
#include "target.h"
#include "eh.h"
#include "tm.h"
#include "tmr.h"
#include "dossup.h"

LOCAL int (*origClockIntr)() = (int(*)())0;


/*<
 * Function:
 * Description:
 * May be current time should also be passed so that 
 * time of the day can be computed from freeCnt.
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC SuccFail
TMR_startClockInterrupt(clockPeriod)
Int clockPeriod;
{
    extern int myClockIntr();
    
    if (clockPeriod != 54) {
	/* What do you expect me to do, reprogram the timer chip */
	EH_problem("Bad clock period");
	return ( FAIL );
    }

    origClockIntr = DOS_getInterruptVector(0x1c);
#if 0
    printf("origClockIntr=%s\n", TM_prAddr(origClockIntr));
    printf("myClockIntr=%s\n", TM_prAddr(myClockIntr));
#endif
    DOS_setInterruptVector(0x1c, myClockIntr);
    return ( SUCCESS );
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC Void
TMR_stopClockInterrupt()
{
    DOS_setInterruptVector(0x1c, origClockIntr);
}
