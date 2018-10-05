/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File name: heartbea.c
 *
 * Description: Heart beat function of scheduler
 *
 * Functions:
 *   G_heartBeat(void)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: heartbea.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "sch.h"

extern Int      G_heartBeat(void);


/*<
 * Function:    G_heartBeat
 *
 * Description: Heart Beat for the stack.
 *
 * Arguments:   None.
 *
 * Returns:     0 on successful completion, 
 *		exits program on unsuccessful completion.
 *
>*/

SuccFail
G_heartBeat(void)
{
    if (SCH_block() < 0) {				/* Scheduler block */
        EH_fatal("main: SCH_block returned negative value");
    }

    SCH_run();						/* Scheduler run */

    return (SUCCESS);
}
