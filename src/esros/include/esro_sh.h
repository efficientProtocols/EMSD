/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of ESRO. An implementation of RFC-2188.
 *
 *ESRO is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *ESRO is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with ESRO; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 *
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 *
*/
/*+
 * Description:
 *	This header file defines the Data Structures needed for 
 *  	communication between
 *	the ESRO User Shell.
 *	and
 *	the ESRO Provider Shell
 *
 * 	Module name is:
 *	LOPQ_
 *	Limited Operations Primitive Queue
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: esro_sh.h,v 1.2 2002/10/25 19:37:34 mohsen Exp $
 */

#ifndef _ESRO_SH_H_ /*{*/
#define _ESRO_SH_H_


#include "addr.h"
#include "esro.h"
#include "sp_shell.h"

#define LOPQ_A_BASE   80	/* ESROS Action primitive Qu Elem Ids */
#define LOPQ_E_BASE   200	/* ESROS Event primitive Qu Elem Ids */

typedef Int LOPS_InvokeId;

typedef struct LOPQ_SapActivate {
    ESRO_SapSel locSapSel;
    ESRO_FunctionalUnit functionalUnit;
    char	chnlName[SP_CHNLNAMELEN];
} LOPQ_SapActivate;

typedef struct LOPQ_SapDeActivate {
    ESRO_SapDesc locSapDesc;
} LOPQ_SapDeActivate;


typedef struct LOPQ_InvokeReq {
    LOPS_InvokeId userShellInvokeId;
    ESRO_UserInvokeRef userInvokeRef;
    ESRO_SapDesc locSapDesc;
    ESRO_SapSel  remESROSap;
    T_SapSel  remTsap;
    N_SapAddr remNsap;
    ESRO_OperationValue opValue;
    ESRO_EncodingType encodingType;
    Int parameterLen;
    Byte parameter[ESRODATASIZE];  /* NOTYET */
} LOPQ_InvokeReq;

typedef struct LOPQ_ResultReq {
    ESRO_SapDesc locSapDesc;
    ESRO_InvokeId invokeId;
    ESRO_UserInvokeRef userInvokeRef;
    ESRO_EncodingType encodingType;
    Int parameterLen;
    Byte parameter[ESRODATASIZE];
} LOPQ_ResultReq;

typedef struct LOPQ_ErrorReq {
    ESRO_SapDesc locSapDesc;
    ESRO_InvokeId invokeId;
    ESRO_UserInvokeRef userInvokeRef;
    ESRO_EncodingType encodingType;
    ESRO_ErrorValue errorValue;
    Int parameterLen;
    Byte parameter[ESRODATASIZE];
} LOPQ_ErrorReq;

typedef struct LOPQ_Primitive {
	long mtype;		/* System V Compatability */
	int type;		/* Event type */
	union {
	    LOPQ_InvokeReq	invokeReq;
	    LOPQ_ResultReq	resultReq;
	    LOPQ_ErrorReq	errorReq;

	    LOPQ_SapActivate    activate;
	    LOPQ_SapDeActivate  deActivate;
	} un;
} LOPQ_Primitive;


typedef struct LOPQ_SapActivateStat {
    ESRO_SapDesc locSapDesc;
} LOPQ_SapActivateStat;

typedef struct LOPQ_InvokeReqStat {
    ESRO_SapDesc locSapDesc;
    ESRO_InvokeId invokeId;
    ESRO_InvokeId userShellInvokeId;
    ESRO_UserInvokeRef userInvokeRef;
} LOPQ_InvokeReqStat;

typedef struct LOPQ_PStatus {
    long mtype;		/* System V compatability */
    int type;
    int status;		/* Primitive Status to be returned to Host */
    union {
	LOPQ_SapActivateStat activateStat;
	LOPQ_InvokeReqStat   invokeReqStat;
    } un;
} LOPQ_PStatus;

/* Events, Confirmation and Indications */
typedef ESRO_InvokeInd LOPQ_InvokeInd;

typedef ESRO_ResultInd LOPQ_ResultInd;

typedef ESRO_ErrorInd  LOPQ_ErrorInd;

typedef ESRO_ResultCnf LOPQ_ResultCnf;

typedef ESRO_ErrorCnf  LOPQ_ErrorCnf;

typedef ESRO_FailureInd LOPQ_FailureInd;

typedef struct LOPQ_Event {
    long mtype;		/* System V compatability */
    int type;		/* Event type */
    union {
	LOPQ_InvokeInd  invokeInd;
	LOPQ_ResultInd  resultInd;
	LOPQ_ErrorInd   errorInd;
	LOPQ_ResultCnf  resultCnf;
	LOPQ_ErrorCnf   errorCnf;
	LOPQ_FailureInd failureInd;
    } un;
} LOPQ_Event;

/* LOPQ_A_BASE and LOPQ_E_BASE are sp_shell.h */


/* Primitive Codes */
#define LOPQ_INVOKE_REQ	 (LOPQ_A_BASE+1)
#define LOPQ_RESULT_REQ	 (LOPQ_A_BASE+2)
#define LOPQ_ERROR_REQ	 (LOPQ_A_BASE+3)

#define LOPQ_SAP_BIND	 (LOPQ_A_BASE+8)
#define LOPQ_SAP_UNBIND  (LOPQ_A_BASE+9)

/* Event Codes */
#define LOPQ_INVOKE_IND	 (LOPQ_E_BASE+0)
#define LOPQ_RESULT_IND	 (LOPQ_E_BASE+1)
#define LOPQ_ERROR_IND	 (LOPQ_E_BASE+2)
#define LOPQ_RESULT_CNF	 (LOPQ_E_BASE+3)
#define LOPQ_ERROR_CNF	 (LOPQ_E_BASE+4)
#define LOPQ_FAILURE_IND (LOPQ_E_BASE+5)

/* Response Codes */
#define LOPQ_SAP_BIND_STAT	(LOPQ_E_BASE+10)
#define LOPQ_INVOKE_REQ_STAT	(LOPQ_E_BASE+11)

/* Common Configuration Parameters,
 * If you change one make sure that both sides are recompiled.
 */
#define	LOPQ_SAPS	64	/* SAPS */
#define	LOPQ_INVOKE_CBS	64	/* Invoke Control Blocks */

#endif	/*}*/
