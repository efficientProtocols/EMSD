/*
 * 
 * Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 *
 * This software is furnished under a license and use, duplication,
 * disclosure and all other uses are restricted to the rights specified
 * in the written license between the licensee and copyright holders.
 *
*/
/*+
 * File name: nm.h
 *
 * Description: Network Management (data structs, definitions, extern functions
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: nm.h,v 1.2 2002/10/08 19:04:43 mohsen Exp $
 */

#ifndef _NM_H_	/*{*/
#define _NM_H_

/*
 *  Network management definitions which are of interest
 *  for any of the seven OSI model layers.
 *
 */

/*
 *  Any item which a layer maintains which can be read or written
 *  by a Network Management Agent are contained in the following structure:
 */

#include "estd.h"

typedef struct Counter {
	long	count;			/* Current count */
	int 	name;			/* Counter identifier */
	int	event;			/* Threshold exceeded event name */
	struct Counter *threshold;	/* Pointer to threshold counter */
} Counter;

/*
 *  Each layer contains a Network Management Information
 *  data structure which is used by a Network Management Agent
 *  to access the counter data structure.
 */

struct nm_info {
	Counter	*counter;	/* Address of counter structure */
	int	mode;		/* Counter mode */
	};

/*  Counter mode  */

#define NM_READ		1	/* Counter can be read */
#define NM_WRITE	2	/* Counter can be written */

/*
 *  In each system, there is one data structure to map between
 *  layer identifiers and their associated network managment information
 *  data structures.
 */

struct nm_data {
	int	layer;		/* Layer identifier */
	int	sublayer;	/* Sublayer identifier */
	int	instance;	/* Instance number */
	struct nm_info *info;	/* Address of network management info *//* M001 */
	};

/*
 *  Structure to be used for reading the Network Management parameters.
 *  This structure enables one call to be made to read multiple parameters.
 */

struct ReadTab {
	int layer;		/* Layer identifier */
	int sublayer;		/* Sublayer identifier */
	int instance;		/* Instance number */
	int id;			/* Parameter id */
	long value;		/* Parameter value */
	};

/*
 *  Structure for use in event reporting.  The events are stored on
 *  a doubly-linked list, in the network management agent, with one
 *  such structure entry per event.
 */

struct nm_event {
	struct nm_event *next;	/* Next queued event */
	struct nm_event *prev;	/* Previous queued event */
	long ev_time;		/* Time-stamp for the event */
	int ev_id;		/* Event identifier */
	int layer_id;		/* Layer identifier */
	};

/*  Layer identifiers  */

#define NM_MGMT		0	/* Management */
#define NM_PHYSICAL	1	/* Physical */
#define NM_LINK		2	/* Link */
#define NM_NETWORK	3	/* Network */
#define NM_TRANSPORT	4	/* Transport */
#define NM_UDP  	4	/* Transport */
#define NM_SESSION	5	/* Session */
#define NM_PRESENTATION	6	/* Presentation */
#define NM_APPLICATION	7	/* Application */

#define NM_ESROS	7	/* Application */

/*  Sublayer identifiers  */

#define NM_MAC		0	/* MAC sublayer */
#define NM_LLC		1	/* LLC sublayer */


SuccFail NM_incCounter(int layer, register Counter *counter, int increment);
SuccFail NM_repEvent(int layer, int eventid);

#endif	/*}*/
