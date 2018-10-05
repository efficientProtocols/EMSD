/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of Neda-EMSD. An implementation of RFC-2524.
 *
 *Neda-EMSD is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with Neda-EMSD; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/

ReturnCode
DUP_invokeIn (DU_View hView, ESRO_OperationValue operationValue, N_SapAddr *pRemoteNsap);

ReturnCode
DUP_invokeOut (DU_View hView, ESRO_OperationValue operationValue, N_SapAddr *pRemoteNsap);

/* TM Trace Flags */
#define	DUP_TRACE_ERROR   	(1 << 0)
#define	DUP_TRACE_WARNING   	(1 << 1)
#define	DUP_TRACE_DETAIL   	(1 << 2)
#define	DUP_TRACE_ACTIVITY	(1 << 3)
#define	DUP_TRACE_INIT		(1 << 4)
#define	DUP_TRACE_VALIDATION	(1 << 5)
#define	DUP_TRACE_PREDICATE	(1 << 6)
#define	DUP_TRACE_ADDRESS	(1 << 7)
#define	DUP_TRACE_STATE  	(1 << 8)
#define	DUP_TRACE_PDU		(1 << 9)
#define	DUP_TRACE_PROFILE	(1 << 10)



/*
 * Each NSAP gets one of these.
 */
typedef struct DUP_NsapEntry
{
    QU_ELEMENT;

    /* Unique operation identifier */
    OS_Uint32		        nsapAddr;

    OS_Uint32		        rejectCount;
    OS_Uint32		        acceptCount;

    OS_Uint8 			sequenceId;

    /* Operation-specific free function and parameter */
    QU_Head 			userFreeQHead;
} DUP_dupeSequence;



typedef struct DUP_Globals
{
    void *	    hTM;		/* Trace Module handle */

    QU_Head         seqList;            /* Dupe detect sequence numbers */

} DUP_Globals;

extern DUP_Globals	DUP_globals;
