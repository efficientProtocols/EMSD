/*
 *  
 *  Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
 * 
 */

/*+
 * File name: nm.c
 *
 * Description: Network Management functions.
 *
 * Functions:
 *   NM_incCounter(int layer, register Counter *counter, int increment)
 *   NM_repEvent(int layer, int eventid)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: nm.c,v 1.3 1999/09/30 00:05:11 mohsen Exp $";
#endif /*}*/

#include "estd.h"  /* or local.h? */
#include "nm.h"

extern SuccFail NM_repEvent(int layer, int eventid);


/*<
 * Function:    NM_incCounter
 *
 * Description: Add the input argument "increment" to a counter.
 *              If a threshold is specified and the counter value exceeds
 *              that value, generate an network management event.
 *
 * Arguments:   Layer, pointer to counter, increment value.
 *
 * Returns:     0 on successful execution, -1 on unsuccessful completion.
 * 
>*/

SuccFail 
NM_incCounter(int layer, register Counter *counter, int increment)
{
    counter->count += increment;
    if (counter->threshold && counter->count >= counter->threshold->count) {
	counter->count = 0;
	return NM_repEvent(layer, counter->event);
    }
    return (SUCCESS);
}


/*<
 * Function:    NM_repEvent
 *
 * Description: Process an event.
 *
 * Arguments:   Layer, event ID.
 *
 * Returns:     0 on successful execution, other than 0 on unsuccessful.
 * 
>*/

SuccFail
NM_repEvent(int layer, int eventid)
{
    /* NOTYET */
    return (SUCCESS);
}
