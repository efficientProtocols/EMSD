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
 * File name: esros_cb.h  (Limited Size Remote Operation Services)
 *
 * Description:
 *    This header file defines the data structures needed by ESRO user
 *    for call back API. The NO_UPSHELL part of the file is the entry
 *    used when the application is linked with ESROS engine in one 
 *    process. That part of the file maps user_shell calls to engine calls.
 *    Include this file after other header files.
 *
 *    ESRO_ global module identifier identifies ESRO User related structures.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: esro_cb.h,v 1.2 2002/10/25 19:37:34 mohsen Exp $
 */

#ifndef _ESROS_H_
#define _ESROS_H_

#include "target.h"
#include "estd.h"	  /* Type definitions: Int, Char, ...                */
#include "addr.h"	  /* SAP struct definitions: ESRO_SapSel, ...        */
#include "du.h"	  /* SAP struct definitions: ESRO_SapSel, ...        */

typedef Int ESRO_RetVal;

typedef Int ESRO_SapDesc;

#ifndef NO_UPSHELL	/* No user/provider shell, i.e. appl/eng in one process */
typedef Int ESRO_InvokeId;
#else
typedef Void *ESRO_InvokeId;
#endif

typedef Int ESRO_OperationValue;        /* 0-63 */

typedef Int ESRO_FunctionalUnit; 

typedef Int ESRO_ErrorValue; 

typedef Void *ESRO_UserInvokeRef; 

/* Encoding type */
typedef enum ESRO_EncodingType {
    ESRO_EncodingBER	    = 0,	/* Basic Encoding Rules  */
    ESRO_EncodingPER        = 1,	/* Packed Encoding Rules */
    ESRO_EncodingReserved1  = 2,	/* Reserved 		 */
    ESRO_EncodingReserved2  = 3 	/* Reserved  		 */
} ESRO_EncodingType;

/* Failure value */
typedef enum ESRO_FailureValue {
    ESRO_FailureTransmission	  = 0,  /* Transmission failure    */
    ESRO_FailureLocResource	  = 1,	/* Out of local resources  */
    ESRO_FailureUserNotResponding = 2,	/* User not responding     */
    ESRO_FailureRemResource	  = 3   /* Out of remote resources */
} ESRO_FailureValue;

typedef enum ESRO_ParamValue {
    ESRO_PortNumber      = 0,	/* Port Number */
    ESRO_MaxSAPs         = 1,	/* Maximum Service Access Points   */
    ESRO_MaxInvokes      = 2,	/* Maximum no of invocations       */
    ESRO_MaxReference    = 3,	/* Maximum no of reference numbers */
    ESRO_PduSize         = 4,	/* PDU size			   */	
    ESRO_Acknowledgement = 5,	/* Acknowledgement delay	   */
    ESRO_Roundtrip       = 6,	/* Roundtrip time		   */
    ESRO_Retransmit      = 7,	/* Retransmission time		   */
    ESRO_MaxNSDULifeTime = 8,	/* Maximum NSDU life time	   */
    ESRO_RetransmitCount = 9	/* Retransmission Time		   */
} ESRO_ParamValue;

#define ESRO_2Way 2
#define ESRO_3Way 3

#define ESRODATASIZE 1500

#define	ESRO_MIN_SAP_NO	 0
#define	ESRO_MAX_SAP_NO 15

/* ----- Events, Confirmation and Indications -----*/

/* Invoke indication */
typedef struct ESRO_InvokeInd {
    ESRO_SapDesc        locSapDesc;
    ESRO_SapSel 	remESROSap;
    T_SapSel		remTsap;
    N_SapAddr		remNsap;
    ESRO_InvokeId	invokeId;
    ESRO_OperationValue	operationValue;
    ESRO_EncodingType	encodingType;
    Int 		len;
    Byte		data[ESRODATASIZE];
} ESRO_InvokeInd;

/* Result indication */
typedef struct ESRO_ResultInd {
    ESRO_InvokeId	invokeId;
    ESRO_UserInvokeRef	userInvokeRef;
    ESRO_EncodingType	encodingType;
    Int 		len;
    Byte		data[ESRODATASIZE];
} ESRO_ResultInd;

/* Error indication */
typedef struct ESRO_ErrorInd {
    ESRO_InvokeId	invokeId;
    ESRO_UserInvokeRef	userInvokeRef;
    ESRO_EncodingType	encodingType;
    ESRO_ErrorValue	errorValue;
    Int 		len;
    Byte		data[ESRODATASIZE];
} ESRO_ErrorInd;

/* Result confirm */
typedef struct ESRO_ResultCnf {
    ESRO_InvokeId	invokeId;
    ESRO_UserInvokeRef	userInvokeRef;
} ESRO_ResultCnf;

/* Error confirm */
typedef struct ESRO_ErrorCnf {
    ESRO_InvokeId	invokeId;
    ESRO_UserInvokeRef	userInvokeRef;
} ESRO_ErrorCnf;

/* Failure indication */
typedef struct ESRO_FailureInd {
    ESRO_InvokeId	invokeId;
    ESRO_UserInvokeRef	userInvokeRef;
    ESRO_FailureValue	failureValue;
} ESRO_FailureInd;

/* Event structure */
typedef struct ESRO_Event {
    int type;				/* Event type */
    union {
	ESRO_InvokeInd  invokeInd;	/* Invoke  indication   */
	ESRO_ResultInd  resultInd;	/* Result  indication   */
	ESRO_ErrorInd   errorInd;	/* Error   indication   */
	ESRO_ResultCnf  resultCnf;	/* Result  confirmation */
	ESRO_ErrorCnf   errorCnf;	/* Error   confirmation */
	ESRO_FailureInd failureInd;	/* Failure indication   */
    } un;
} ESRO_Event;

/* Event Codes */
#define  ESRO_E_BASE     200
#define  ESRO_INVOKEIND  (ESRO_E_BASE+0)
#define  ESRO_RESULTIND  (ESRO_E_BASE+1)
#define  ESRO_ERRORIND   (ESRO_E_BASE+2)
#define  ESRO_RESULTCNF  (ESRO_E_BASE+3)
#define  ESRO_ERRORCNF   (ESRO_E_BASE+4)
#define  ESRO_FAILUREIND (ESRO_E_BASE+5)

/* NOTYET */
#define  SDP_MTS_INVOKER_3WAY 		2
#define  SDP_DEVICE_PERFORMER_3WAY 	3
#define  SDP_DEVICE_INVOKER_3WAY 	4
#define  SDP_MTS_PERFORMER_3WAY 	5
#define  SDP_MTS_INVOKER_2WAY 		6
#define  SDP_DEVICE_PERFORMER_2WAY 	7
#define  SDP_DEVICE_INVOKER_2WAY 	8
#define  SDP_MTS_PERFORMER_2WAY 	9

#ifdef UDP_PO
extern char eropEngErrLog[512];
extern char eropEngOutLog[512];
#endif

/*----- definition of functions -----*/

#ifdef LINT_ARGS	/* argument checking is enabled */

#ifdef NO_UPSHELL	/* No user/provider shell, i.e. appl/eng in one process */
extern ESRO_RetVal ESRO_CB_init(char *); 
#else
extern ESRO_RetVal ESRO_CB_init(); 
#endif
extern ESRO_RetVal
ESRO_CB_sapBind(ESRO_SapDesc *sapDesc, /* OUTgoing */
	        ESRO_SapSel sapSel,
	        ESRO_FunctionalUnit functionalUnit,
	        int (*invokeInd) (ESRO_SapDesc		locSapDesc,
			          ESRO_SapSel 	   	remESROSap,
			          T_SapSel		*remTsap,
			          N_SapAddr	   	*remNsap,
				  ESRO_InvokeId     	invokeId,
			          ESRO_OperationValue 	opValue,
			          ESRO_EncodingType   	encodingType,
			          DU_View	   	parameter),
	        int (*resultInd) (ESRO_InvokeId   	invokeId,
				  ESRO_UserInvokeRef userInvokeRef,
			          ESRO_EncodingType 	encodingType,
			          DU_View		parameter),
	        int (*errorInd)  (ESRO_InvokeId   	invokeId,
				  ESRO_UserInvokeRef userInvokeRef,
			          ESRO_EncodingType	encodingType,
			          ESRO_ErrorValue	errorValue,
			          DU_View 		parameter),
	        int (*resultCnf) (ESRO_InvokeId     invokeId,
				  ESRO_UserInvokeRef userInvokeRef),
	        int (*errorCnf)  (ESRO_InvokeId     invokeId,
				  ESRO_UserInvokeRef userInvokeRef),
	        int (*failureInd)(ESRO_InvokeId     invokeId, 
				  ESRO_UserInvokeRef userInvokeRef,
				  ESRO_FailureValue failureValue));

extern ESRO_RetVal ESRO_CB_sapUnbind (ESRO_SapSel sapSel); 
extern ESRO_RetVal ESRO_CB_invokeReq (ESRO_InvokeId *invokeId, 
				   ESRO_UserInvokeRef userInvokeRef,
 	    			   ESRO_SapDesc locSapDesc, 
				   ESRO_SapSel remESROSap, 
				   T_SapSel *remTsap,
	    			   N_SapAddr *remNsap, 
				   ESRO_OperationValue opValue, 
	    			   ESRO_EncodingType encodingType, 
				   DU_View parameter);
extern ESRO_RetVal ESRO_CB_resultReq (ESRO_InvokeId invokeId, 
				   ESRO_UserInvokeRef userInvokeRef,
	    			   ESRO_EncodingType encodingType, 
				   DU_View parameter);
extern ESRO_RetVal ESRO_CB_errorReq  (ESRO_InvokeId  invokeId, 
				   ESRO_UserInvokeRef userInvokeRef,
	    			   ESRO_EncodingType encodingType, 
	    			   ESRO_ErrorValue   errorValue, 
				   DU_View parameter);
extern ESRO_RetVal ESRO_CB_getEvent  (ESRO_SapDesc sapDesc);

#else			/* argument checking is disabled */

extern ESRO_RetVal ESRO_CB_init      (); 
extern ESRO_RetVal ESRO_CB_sapBind   ();
extern ESRO_RetVal ESRO_CB_sapUnbind (); 
extern ESRO_RetVal ESRO_CB_invokeReq ();
extern ESRO_RetVal ESRO_CB_resultReq (); 
extern ESRO_RetVal ESRO_CB_errorReq  (); 
extern ESRO_RetVal ESRO_CB_getEvent  (); 

#endif /* LINT_ARGS */

#ifdef NO_UPSHELL  /* No user/provider shell, i.e. appl/eng in one process */

#define ESRO_CB_sapBind   EROP_sapBind
#define ESRO_CB_sapUnbind EROP_sapUnbind

#define ESRO_CB_invokeReq EROP_invokeReq
#define ESRO_CB_resultReq EROP_resultReq
#define ESRO_CB_errorReq  EROP_errorReq

extern ESRO_RetVal EROP_sapBind();
extern SuccFail    EROP_sapUnbind();
extern ESRO_RetVal EROP_invokeReq();
extern ESRO_RetVal EROP_resultReq();
extern ESRO_RetVal EROP_errorReq();

#ifdef LINT_ARGS	/* argument checking is enabled */
extern SuccFail UDP_PO_init(String, String);
extern SuccFail UDP_init (Int);
extern PUBLIC SuccFail EROP_init(Int udpSapSel, Int nuOfSaps, 
			 	 Int nuOfInvokes, Int invokePduSize);
#else			/* argument checking is disabled */
extern SuccFail UDP_PO_init();
extern SuccFail UDP_init ();
extern PUBLIC SuccFail EROP_init();
#endif /* LINT_ARGS */

#endif

#endif /* _ESROS_H_ */
