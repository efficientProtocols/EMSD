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
 * File name: lops_qif.c  (Limited Operations Provider Shell Queue Interface)
 *
 * Description:  User/Provider Queue Interface
 *
 * Functions:
 *   LOPS_getEventSize(Int eventType)
 *   LOPS_allocEvent(Int eventType)
 *   LOPS_putEvent(Int sapDesc, LOPQ_Event *event)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: lops_qif.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include  "estd.h"
#include  "eh.h"
#include  "sf.h"
#include  "tm.h"
#include  "imq.h"
#include  "psq.h"
#include  "esro_sh.h"
#include  "erop.h"

#include  "local.h"
#include  "extfuncs.h"

#ifdef TM_ENABLED
TM_ModuleCB *LOPS_modCB;
#endif


/*<
 * Function:    LOPS_getEventSize
 *
 * Description: Get event size.
 *
 * Arguments:   Event type.
 *
 * Returns:     Event size if successful, -1 otherwise.
 *
>*/

Int
LOPS_getEventSize(Int eventType)
{
    Int eventSize;
    LOPQ_PStatus *pStatus;	/* Only used at compile time */
    LOPQ_Event   *event;	/* Only used at compile time */

    TM_TRACE((LOPS_modCB, TM_ENTER,
	    " LOPS_getEventSize: eventType=%d\n",  eventType));

    eventSize = 0;

#define SIZEOFEVENTTYPE (sizeof(*event) - sizeof(event->un))
    switch ( eventType ) {

    case LOPQ_INVOKE_IND :
	eventSize = sizeof(event->un.invokeInd) + SIZEOFEVENTTYPE;

        TM_TRACE((LOPS_modCB, TM_ENTER,
	         " LOPS_getEventSize: eventType=LOPQ_INVOKE_IND\n"));
	break;

    case LOPQ_RESULT_IND :
	eventSize = sizeof(event->un.resultInd) + SIZEOFEVENTTYPE;

        TM_TRACE((LOPS_modCB, TM_ENTER,
	         " LOPS_getEventSize: eventType=LOPQ_RESULT_IND\n"));
	break;

    case LOPQ_ERROR_IND :
	eventSize = sizeof(event->un.errorInd) + SIZEOFEVENTTYPE;

        TM_TRACE((LOPS_modCB, TM_ENTER,
	         " LOPS_getEventSize: eventType=LOPQ_ERROR_IND\n"));
	break;

    case LOPQ_RESULT_CNF :
	eventSize = sizeof(event->un.resultCnf) + SIZEOFEVENTTYPE;

        TM_TRACE((LOPS_modCB, TM_ENTER,
	         " LOPS_getEventSize: eventType=LOPQ_RESULT_CNF\n"));
	break;

    case LOPQ_ERROR_CNF :
	eventSize = sizeof(event->un.errorCnf) + SIZEOFEVENTTYPE;

        TM_TRACE((LOPS_modCB, TM_ENTER,
	         " LOPS_getEventSize: eventType=LOPQ_ERROR_CONF\n"));
	break;

    case LOPQ_FAILURE_IND :
	eventSize = sizeof(event->un.failureInd) + SIZEOFEVENTTYPE;

        TM_TRACE((LOPS_modCB, TM_ENTER,
	         " LOPS_getEventSize: eventType=LOPQ_FAILURE_IND\n"));
	break;


#define SIZEOFSTATUSTYPE (sizeof(*pStatus) - sizeof(pStatus->un))

    case LOPQ_SAP_BIND_STAT:
	eventSize = sizeof(pStatus->un.activateStat) + SIZEOFSTATUSTYPE;

        TM_TRACE((LOPS_modCB, TM_ENTER,
	         " LOPS_getEventSize: eventType=LOPQ_SAP_BIND_STAT\n"));
	break;

    case LOPQ_INVOKE_REQ_STAT:
	eventSize = sizeof(pStatus->un.invokeReqStat) + SIZEOFSTATUSTYPE;

        TM_TRACE((LOPS_modCB, TM_ENTER,
	         " LOPS_getEventSize: eventType=LOPQ_INVOKE_REQ_STAT\n"));
	break;

    default:
	EH_problem("LOPS_getEventSize: invalid event type");
	fprintf(stderr, "LOPS_getEventSize: eventType=%d\n", eventType);
	break;
    }

    return ( eventSize );

} /* getEventSize () */

    

/*<
 * Function:    LOPS_allocEvent
 *
 * Description: Allocate memory for event and initialize it.
 *
 * Arguments:   Event type.
 *
 * Returns:     Pointer to event struct.
 * 
>*/

PUBLIC LOPQ_Event *
LOPS_allocEvent(Int eventType)
{
    /* allocate an event and initialize it,
     * return a pointer to it.
     */
    Int eventSize;
    LOPQ_Event * event;

    if ( ! (eventSize = LOPS_getEventSize(eventType)) ) {
	EH_problem("LOPS_allocEvent: zero event size");
	return ( (LOPQ_Event *)NULL );
    }

    if ((event = (LOPQ_Event *) SF_memGet(eventSize)) == 0) {
	EH_problem("LOPS_allocEvent: SF_memGet failed");
	return ( (LOPQ_Event *)NULL );
    } 
    
    event->mtype = 0;
    event->type  = eventType;

    return ( event );

} /* LOPS_allocEvent() */


/*<
 * Function:    LOPS_putEvent
 *
 * Description: Put Event in the queue.
 *
 * Arguments:   SAP descriptor, event.
 *
 * Returns:     0 if successfule, a negative value if unsuccessful.
 * 
>*/

PUBLIC SuccFail
LOPS_putEvent(Int sapDesc, LOPQ_Event *event)
{
    Int eventSize;
    PSQ_PrimQuInfo *primQu;

    TM_TRACE((LOPS_modCB, TM_ENTER,
	    "LOPS_putEvent:     eventType=%d\n", event->type));

    if ( ! (eventSize = LOPS_getEventSize(event->type)) ) {
	EH_problem("LOPS_putEvent: zero event size");
        SF_memRelease(event);
	return (FAIL);
    }
    primQu = psSapCBMap[sapDesc].primQu;

    if (PSQ_putEvent(primQu, (unsigned char *)event, eventSize) == FAIL) {
        SF_memRelease(event);
	return (FAIL);
    }

    SF_memRelease(event);

    return (SUCCESS);

} /* LOPS_putEvent() */
